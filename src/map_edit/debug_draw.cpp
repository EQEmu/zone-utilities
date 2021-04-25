#include "debug_draw.h"
#include "shader.h"

DebugDraw::DebugDraw() {
    m_use_depth = true;
}

DebugDraw::DebugDraw(bool depth) {
    m_use_depth = depth;
}

DebugDraw::~DebugDraw() {
}

void DebugDraw::Draw() {
    ShaderProgram shader = ShaderProgram::Current();
    ShaderUniform tint = shader.GetUniformLocation("Tint");

    glm::vec4 current_tint = m_triangles.GetTint();
    tint.SetValuePtr4(1, &current_tint[0]);
    m_triangles.Draw();

    current_tint = m_lines.GetTint();
    tint.SetValuePtr4(1, &current_tint[0]);
    m_lines.Draw();

    current_tint = m_points.GetTint();
    tint.SetValuePtr4(1, &current_tint[0]);
    m_points.Draw();
}

void DebugDraw::Clear() {
    m_triangles.Clear();
    m_lines.Clear();
    m_points.Clear();
}

void DebugDraw::Update() {
    m_points.Update();
    m_points.SetDrawType(GL_POINTS);
    m_points.SetBlend(true);

    m_lines.Update();
    m_lines.SetDrawType(GL_LINES);
    m_lines.SetBlend(true);

    m_triangles.Update();
    m_triangles.SetDrawType(GL_TRIANGLES);
    m_triangles.SetBlend(true);

    if(!m_use_depth) {
        m_points.SetDepthWriteEnabled(false);
        m_lines.SetDepthWriteEnabled(false);
        m_triangles.SetDepthWriteEnabled(false);
    }
}
