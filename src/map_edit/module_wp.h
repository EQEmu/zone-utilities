#pragma once

#include "module.h"
#include "scene.h"
#include <vector>

struct WPNode {
    uint32_t id;
    float x;
    float y;
    float z;
    float best_z;
};

struct WPEdge {
    uint32_t from;
    uint32_t to;
    float distance;
    int8_t teleport;
    int32_t door_id;
};

class ModuleWP : public Module, public SceneHotkeyListener {
public:
    ModuleWP();
    virtual ~ModuleWP();
    virtual const char* GetName() { return "WP"; };
    virtual void OnLoad(Scene* s);
    virtual void OnShutdown();
    virtual void OnDrawMenu();
    virtual void OnDrawUI();
    virtual void OnDrawOptions();
    virtual void OnSceneLoad(const char* zone_name);
    virtual void OnSuspend();
    virtual void OnResume();
    virtual bool HasWork();
    virtual bool CanSave();
    virtual void Save();
    virtual void OnHotkey(int ident);
    virtual void OnClick(int mouse_button, const glm::vec3* collide_hit, const glm::vec3* select_hit, Entity* selected);
    virtual void Tick(float delta_time) {}

private:
    void LoadPath();
    void LoadV2(FILE* f, uint32_t nodes);
    void LoadV3(FILE* f, uint32_t nodes);
    void BuildVisualGraph(bool rebuild = false);
    int32_t GetSelectedNode(const glm::vec3& loc);
    void Connect(int selected, int current);
    void ConnectNodeToNode(int a, int b);
    void Disconnect(int selected, int current);
    void DisconnectNodeToNode(int a, int b);
    void Create(const glm::vec3& loc);
    void Delete();
    void RecalcDistancesForNode(int id);

    Scene* m_scene;
    std::vector<WPNode> m_nodes;
    std::vector<WPEdge> m_edges;
    std::unique_ptr<DynamicGeometry> m_nodes_renderable;
    std::unique_ptr<DynamicGeometry> m_edges_renderable;
    std::unique_ptr<DynamicGeometry> m_selected_renderable;
    int32_t m_selected_node;
    bool m_dirty;

    int32_t m_current_door_id;
    bool m_current_teleport;
    bool m_current_bidirectional;
};
