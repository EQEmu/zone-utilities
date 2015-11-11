#include "nav_mesh_model.h"
#include "shader.h"

NavMeshModel::NavMeshModel() {
}

NavMeshModel::~NavMeshModel() {
}

void NavMeshModel::Draw() {
	ShaderProgram shader = ShaderProgram::Current();
	ShaderUniform tint = shader.GetUniformLocation("Tint");

	glDepthMask(GL_FALSE);

	glm::vec4 current_tint = m_triangles.GetTint();
	tint.SetValuePtr4(1, &current_tint[0]);
	m_triangles.Draw();

	current_tint = m_lines.GetTint();
	tint.SetValuePtr4(1, &current_tint[0]);
	m_lines.Draw();

	current_tint = m_points.GetTint();
	tint.SetValuePtr4(1, &current_tint[0]);
	m_points.Draw();

	glDepthMask(GL_TRUE);
}

void NavMeshModel::Clear()
{
	m_triangles.Clear();
	m_lines.Clear();
	m_points.Clear();
}

void NavMeshModel::Update()
{
	m_points.Update();
	m_points.SetDrawType(GL_POINTS);
	
	m_lines.Update();
	m_lines.SetDrawType(GL_LINES);
	
	m_triangles.Update();
	m_triangles.SetDrawType(GL_TRIANGLES);
}


