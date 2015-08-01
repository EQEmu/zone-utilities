#include "navigation.h"
#include "imgui.h"
#include "oriented_bounding_box.h"
#define _USE_MATH_DEFINES
#include <math.h>

void Navigation::CreateNavMesh(Model *m) {
	if(!m) {
		return;
	}

	std::unique_ptr<Model> model(m->Flip());
	NavWorkStatus status = GetStatus();
	if(!(status == NavWorkNone || status == NavWorkNeedsCompile)) {
		return;
	}

	SetStatus(NavWorkNavMeshCreateHeightField);

	rcHeightfield* m_solid = nullptr;
	rcCompactHeightfield* m_chf = nullptr;
	rcContourSet* m_cset = nullptr;
	rcPolyMesh* m_pmesh = nullptr;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh = nullptr;
	rcContext m_ctx;

	dtFreeNavMesh(m_nav_mesh_data.m_navMesh);
	m_nav_mesh_data.m_navMesh = nullptr;

	const float* bmin = (float*)&model->GetAABBMin();
	const float* bmax = (float*)&model->GetAABBMax();
	const float* verts = (float*)model->GetPositions().data();
	const int nverts = (int)model->GetPositions().size();
	const int* tris = (int*)model->GetIndicies().data();
	const int ntris = (int)model->GetIndicies().size() / 3;

	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = m_nav_mesh_data.m_cellSize;
	m_cfg.ch = m_nav_mesh_data.m_cellHeight;
	m_cfg.walkableSlopeAngle = m_nav_mesh_data.m_agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_nav_mesh_data.m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_nav_mesh_data.m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_nav_mesh_data.m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_nav_mesh_data.m_edgeMaxLen / m_nav_mesh_data.m_cellSize);
	m_cfg.maxSimplificationError = m_nav_mesh_data.m_edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_nav_mesh_data.m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_nav_mesh_data.m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_nav_mesh_data.m_vertsPerPoly;
	m_cfg.detailSampleDist = m_nav_mesh_data.m_detailSampleDist < 0.9f ? 0 : m_nav_mesh_data.m_cellSize * m_nav_mesh_data.m_detailSampleDist;
	m_cfg.detailSampleMaxError = m_nav_mesh_data.m_cellHeight * m_nav_mesh_data.m_detailSampleMaxError;

	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	m_solid = rcAllocHeightfield();
	if(!m_solid)
	{
		return;
	}

	if(!rcCreateHeightfield(&m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		rcFreeHeightField(m_solid);
		SetStatus(NavWorkNeedsCompile);
		return;
	}

	unsigned char *m_triareas = new unsigned char[ntris];
	if(!m_triareas)
	{
		rcFreeHeightField(m_solid);
		SetStatus(NavWorkNeedsCompile);
		return;
	}

	SetStatus(NavWorkNavMeshMarkTriangles);
	rcMarkWalkableTriangles(&m_ctx, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	rcRasterizeTriangles(&m_ctx, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb);

	delete[] m_triareas;

	SetStatus(NavWorkNavMeshFilterObstacles);
	rcFilterLowHangingWalkableObstacles(&m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(&m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(&m_ctx, m_cfg.walkableHeight, *m_solid);

	SetStatus(NavWorkNavMeshBuildCompactHightField);
	m_chf = rcAllocCompactHeightfield();
	if(!m_chf)
	{
		return;
	}

	if(!rcBuildCompactHeightfield(&m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		rcFreeHeightField(m_solid);
		rcFreeCompactHeightfield(m_chf);
		SetStatus(NavWorkNeedsCompile);
		return;
	}

	rcFreeHeightField(m_solid);

	SetStatus(NavWorkNavMeshErodeWalkableAreas);
	if(!rcErodeWalkableArea(&m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		rcFreeCompactHeightfield(m_chf);
		SetStatus(NavWorkNeedsCompile);
		return;
	}

	SetStatus(NavWorkNavMeshPartition);
	if(m_nav_mesh_data.m_partitionType == NAVMESH_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if(!rcBuildDistanceField(&m_ctx, *m_chf))
		{
			rcFreeCompactHeightfield(m_chf);
			SetStatus(NavWorkNeedsCompile);
			return;
		}

		// Partition the walkable surface into simple regions without holes.
		if(!rcBuildRegions(&m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(m_chf);
			SetStatus(NavWorkNeedsCompile);
			return;
		}
	}
	else if(m_nav_mesh_data.m_partitionType == NAVMESH_PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if(!rcBuildRegionsMonotone(&m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(m_chf);
			SetStatus(NavWorkNeedsCompile);
			return;
		}
	}
	else
	{
		// Partition the walkable surface into simple regions without holes.
		if(!rcBuildLayerRegions(&m_ctx, *m_chf, 0, m_cfg.minRegionArea))
		{
			rcFreeCompactHeightfield(m_chf);
			SetStatus(NavWorkNeedsCompile);
			return;
		}
	}

	SetStatus(NavWorkNavMeshBuildContours);
	m_cset = rcAllocContourSet();
	if(!m_cset) {
		rcFreeCompactHeightfield(m_chf);
		SetStatus(NavWorkNeedsCompile);
		printf("Alloc contours fail\n");
		return;
	}

	if(!rcBuildContours(&m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		rcFreeCompactHeightfield(m_chf);
		rcFreeContourSet(m_cset);
		SetStatus(NavWorkNeedsCompile);
		printf("Build contours fail\n");
		return;
	}

	SetStatus(NavWorkNavMeshBuildPolyMesh);
	m_pmesh = rcAllocPolyMesh();
	if(!m_pmesh)
	{
		rcFreeCompactHeightfield(m_chf);
		rcFreeContourSet(m_cset);
		SetStatus(NavWorkNeedsCompile);
		printf("Alloc polymesh fail\n");
		return;
	}
	if(!rcBuildPolyMesh(&m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		rcFreeCompactHeightfield(m_chf);
		rcFreeContourSet(m_cset);
		rcFreePolyMesh(m_pmesh);
		SetStatus(NavWorkNeedsCompile);
		printf("Build poly mesh fail\n");
		return;
	}

	SetStatus(NavWorkNavMeshBuildPolyMeshDetail);
	m_dmesh = rcAllocPolyMeshDetail();
	if(!m_dmesh)
	{
		rcFreeCompactHeightfield(m_chf);
		rcFreeContourSet(m_cset);
		SetStatus(NavWorkNeedsCompile);
		printf("Alloc polymesh detail fail\n");
		return;
	}

	if(!rcBuildPolyMeshDetail(&m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		rcFreeCompactHeightfield(m_chf);
		rcFreeContourSet(m_cset);
		rcFreePolyMesh(m_pmesh);
		SetStatus(NavWorkNeedsCompile);

		printf("Build poly mesh detail fail\n");
		return;
	}

	rcFreeCompactHeightfield(m_chf);
	rcFreeContourSet(m_cset);

	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = m_pmesh->verts;
	params.vertCount = m_pmesh->nverts;
	params.polys = m_pmesh->polys;
	params.polyAreas = m_pmesh->areas;
	params.polyFlags = m_pmesh->flags;
	params.polyCount = m_pmesh->npolys;
	params.nvp = m_pmesh->nvp;
	params.detailMeshes = m_dmesh->meshes;
	params.detailVerts = m_dmesh->verts;
	params.detailVertsCount = m_dmesh->nverts;
	params.detailTris = m_dmesh->tris;
	params.detailTriCount = m_dmesh->ntris;
	params.offMeshConVerts = nullptr;
	params.offMeshConRad = nullptr;
	params.offMeshConDir = nullptr;
	params.offMeshConAreas = nullptr;
	params.offMeshConFlags = nullptr;
	params.offMeshConUserID = nullptr;
	params.offMeshConCount = 0;
	params.walkableHeight = m_nav_mesh_data.m_agentHeight;
	params.walkableRadius = m_nav_mesh_data.m_agentRadius;
	params.walkableClimb = m_nav_mesh_data.m_agentMaxClimb;
	rcVcopy(params.bmin, m_pmesh->bmin);
	rcVcopy(params.bmax, m_pmesh->bmax);
	params.cs = m_cfg.cs;
	params.ch = m_cfg.ch;
	params.buildBvTree = true;

	SetStatus(NavWorkNavMeshConvertToDetourNM);
	unsigned char *navData = nullptr;
	int navDataSize = 0;
	if(!dtCreateNavMeshData(&params, &navData, &navDataSize))
	{
		rcFreePolyMesh(m_pmesh);
		SetStatus(NavWorkNeedsCompile);
		printf("Create detour mesh fail\n");
		return;
	}

	m_nav_mesh_data.m_navMesh = dtAllocNavMesh();
	if(!m_nav_mesh_data.m_navMesh)
	{
		dtFree(navData);
		rcFreePolyMesh(m_pmesh);
		SetStatus(NavWorkNeedsCompile);
		return;
	}

	dtStatus d_status = m_nav_mesh_data.m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if(dtStatusFailed(d_status))
	{
		dtFree(navData);
		rcFreePolyMesh(m_pmesh);
		SetStatus(NavWorkNeedsCompile);
		printf("Detour mesh init fail\n");
		return;
	}

	rcFreePolyMesh(m_pmesh);

	SetStatus(NavWorkNeedsCompile);
}

void Navigation::BuildNavigationModel() {
	if(GetStatus() != NavWorkNeedsCompile) {
		return;
	}

	SetStatus(NavWorkNone);
	m_nav_nodes_model.reset(new Model());
	m_nav_mesh_model_lines.reset(new Model());
	m_nav_mesh_model_points.reset(new Model());
	m_nav_mesh_model_tris.reset(new Model());

	BuildNodeModel();

	BuildNavMeshModel();

	m_nav_nodes_model->Compile();
	m_nav_mesh_model_lines->Compile();
	m_nav_mesh_model_points->Compile();
	m_nav_mesh_model_tris->Compile();
}

void Navigation::BuildNodeModel() {
	if(!z_model) {
		return;
	}

	auto &positions = m_nav_nodes_model->GetPositions();
	auto &indicies = m_nav_nodes_model->GetIndicies();

	for(auto &ent : m_nodes) {
		float extent = 1.0f;
		float scale = 0.4f;

		glm::vec4 v1(-extent, extent, -extent, 1.0f);
		glm::vec4 v2(-extent, extent, extent, 1.0f);
		glm::vec4 v3(extent, extent, extent, 1.0f);
		glm::vec4 v4(extent, extent, -extent, 1.0f);
		glm::vec4 v5(-extent, -extent, -extent, 1.0f);
		glm::vec4 v6(-extent, -extent, extent, 1.0f);
		glm::vec4 v7(extent, -extent, extent, 1.0f);
		glm::vec4 v8(extent, -extent, -extent, 1.0f);

		glm::mat4 transformation = CreateRotateMatrix(0.0f, 0.0f, 0.0f);
		transformation = CreateScaleMatrix(scale, scale, scale) * transformation;
		transformation = CreateTranslateMatrix(ent->y, ent->x, ent->z) * transformation;

		v1 = transformation * v1;
		v2 = transformation * v2;
		v3 = transformation * v3;
		v4 = transformation * v4;
		v5 = transformation * v5;
		v6 = transformation * v6;
		v7 = transformation * v7;
		v8 = transformation * v8;

		uint32_t current_index = (uint32_t)positions.size();
		positions.push_back(glm::vec3(v1.y, v1.x, v1.z));
		positions.push_back(glm::vec3(v2.y, v2.x, v2.z));
		positions.push_back(glm::vec3(v3.y, v3.x, v3.z));
		positions.push_back(glm::vec3(v4.y, v4.x, v4.z));
		positions.push_back(glm::vec3(v5.y, v5.x, v5.z));
		positions.push_back(glm::vec3(v6.y, v6.x, v6.z));
		positions.push_back(glm::vec3(v7.y, v7.x, v7.z));
		positions.push_back(glm::vec3(v8.y, v8.x, v8.z));

		//top
		indicies.push_back(current_index + 0);
		indicies.push_back(current_index + 1);
		indicies.push_back(current_index + 2);
		indicies.push_back(current_index + 2);
		indicies.push_back(current_index + 3);
		indicies.push_back(current_index + 0);

		//back
		indicies.push_back(current_index + 1);
		indicies.push_back(current_index + 2);
		indicies.push_back(current_index + 6);
		indicies.push_back(current_index + 6);
		indicies.push_back(current_index + 5);
		indicies.push_back(current_index + 1);

		//bottom
		indicies.push_back(current_index + 4);
		indicies.push_back(current_index + 5);
		indicies.push_back(current_index + 6);
		indicies.push_back(current_index + 6);
		indicies.push_back(current_index + 7);
		indicies.push_back(current_index + 4);

		//front
		indicies.push_back(current_index + 0);
		indicies.push_back(current_index + 3);
		indicies.push_back(current_index + 7);
		indicies.push_back(current_index + 7);
		indicies.push_back(current_index + 4);
		indicies.push_back(current_index + 0);

		//left
		indicies.push_back(current_index + 0);
		indicies.push_back(current_index + 1);
		indicies.push_back(current_index + 5);
		indicies.push_back(current_index + 5);
		indicies.push_back(current_index + 4);
		indicies.push_back(current_index + 0);

		//right
		indicies.push_back(current_index + 3);
		indicies.push_back(current_index + 2);
		indicies.push_back(current_index + 6);
		indicies.push_back(current_index + 6);
		indicies.push_back(current_index + 7);
		indicies.push_back(current_index + 3);
	}
}

void Navigation::BuildNavMeshModel() {
	if(!m_nav_mesh_data.m_navMesh) {
		return;
	}

	NavigationDebugDraw dd;
	dd.lines = m_nav_mesh_model_lines.get();
	dd.points = m_nav_mesh_model_points.get();
	dd.tris = m_nav_mesh_model_tris.get();
	duDebugDrawNavMesh(&dd, *m_nav_mesh_data.m_navMesh, 0xffu);
}

PathNode *Navigation::AttemptToAddWaterNode(float x, float y, float z) {
	return nullptr;
}

void Navigation::SetStatus(NavWorkStatus status) {
	m_work_lock.lock();
	m_work_status = status;
	m_work_lock.unlock();
}

NavWorkStatus Navigation::GetStatus() {
	NavWorkStatus ret;
	m_work_lock.lock();
	ret = m_work_status;
	m_work_lock.unlock();
	return ret;
}

void Navigation::Draw(ShaderUniform *tint, bool wire) {
	if(!m_nav_nodes_model) {
		return;
	}

	if(!m_nav_mesh_model_tris) {
		return;
	}
	
	if(!wire) {
		glm::vec4 tnt(0.5f, 1.0f, 0.7f, 1.0f);
		tint->SetValuePtr4(1, &tnt[0]);
	}

	m_nav_nodes_model->Draw();

	if(!wire) {
		m_nav_mesh_model_tris->Draw();

		glLineWidth(1.0f);

		glm::vec4 tnt(0.0f, 0.0f, 0.0f, 0.0f);
		tint->SetValuePtr4(1, &tnt[0]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		m_nav_mesh_model_lines->Draw(GL_LINES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		m_nav_mesh_model_points->Draw(GL_POINTS);

		glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Navigation::RenderGUI() {
	NavWorkStatus status = GetStatus();

	if(status == NavWorkNeedsCompile) {
		BuildNavigationModel();
	}

	ImGui::Begin("Navigation");
	switch(status) {
	case NavWorkNavMeshCreateHeightField:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Creating Navmesh height field.");
		break;
	case NavWorkNavMeshMarkTriangles:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Marking Navmesh triangles.");
		break;
	case NavWorkNavMeshFilterObstacles:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Filtering Navmesh triangles based on obstacles.");
		break;
	case NavWorkNavMeshBuildCompactHightField:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Building Navmesh compact height field.");
		break;
	case NavWorkNavMeshErodeWalkableAreas:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Eroding walkable areas.");
		break;
	case NavWorkNavMeshPartition:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Partitioning Navmesh.");
		break;
	case NavWorkNavMeshBuildContours:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Building Navmesh contours.");
		break;
	case NavWorkNavMeshBuildPolyMesh:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Building polygon Navmesh.");
		break;
	case NavWorkNavMeshBuildPolyMeshDetail:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Building detail polygon Navmesh.");
		break;
	case NavWorkNavMeshConvertToDetourNM:
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Converting Recast Navmesh to Detour Navmesh");
		break;
	default:
	{
		if(ImGui::Button("Calculate Navmesh")) {
		   std::thread t(&Navigation::CreateNavMesh, this, z_model);
		   t.detach();
		}
	}
	}
	ImGui::End();
}

void NavigationDebugDraw::begin(duDebugDrawPrimitives prim, float size) {
	verts_in_use = 0;
	switch(prim)
	{
	case DU_DRAW_POINTS:
		mode = 1;
	break;
	case DU_DRAW_LINES:
		mode = 2;
	break;
	case DU_DRAW_TRIS:
		mode = 3;
	break;
	case DU_DRAW_QUADS:
		mode = 4;
	break;
	};
}

void NavigationDebugDraw::vertex(const float* pos, unsigned int color) {
	verts[verts_in_use++] = glm::vec3(pos[0], pos[2], pos[1]);
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color) {
	verts[verts_in_use++] = glm::vec3(x, z, y);
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float* pos, unsigned int color, const float* uv) {
	verts[verts_in_use++] = glm::vec3(pos[0], pos[2], pos[1]);
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) {
	verts[verts_in_use++] = glm::vec3(x, z, y);
	CreatePrimitive();
}

void NavigationDebugDraw::CreatePrimitive() {
	if(mode == 0 || verts_in_use != mode) {
		return;
	}

	switch(mode) {
	case 1:
	{
		unsigned int index = (unsigned int)points->GetPositions().size();
		points->GetPositions().push_back(verts[0]);
		points->GetIndicies().push_back(index);
	}
		break;
	case 2:
	{
		unsigned int index = (unsigned int)lines->GetPositions().size();
		lines->GetPositions().push_back(verts[0]);
		lines->GetPositions().push_back(verts[1]);
		lines->GetIndicies().push_back(index);
		lines->GetIndicies().push_back(index + 1);
	}
		break;
	case 3:
	{
		unsigned int index = (unsigned int)tris->GetPositions().size();
		tris->GetPositions().push_back(verts[0]);
		tris->GetPositions().push_back(verts[1]);
		tris->GetPositions().push_back(verts[2]);
		tris->GetIndicies().push_back(index);
		tris->GetIndicies().push_back(index + 1);
		tris->GetIndicies().push_back(index + 2);
	}
		break; //2 3 0
	case 4:
	{
		unsigned int index = (unsigned int)tris->GetPositions().size();
		tris->GetPositions().push_back(verts[0]);
		tris->GetPositions().push_back(verts[1]);
		tris->GetPositions().push_back(verts[2]);
		tris->GetPositions().push_back(verts[3]);
		tris->GetIndicies().push_back(index);
		tris->GetIndicies().push_back(index + 1);
		tris->GetIndicies().push_back(index + 2);
		tris->GetIndicies().push_back(index + 2);
		tris->GetIndicies().push_back(index + 3);
		tris->GetIndicies().push_back(index);
	}
		break;
	}

	verts_in_use = 0;
}
