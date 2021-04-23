#include "module_volume.h"
#include "log_macros.h"
#include <algorithm>

#include <core/config.h>
#include <event/background_task.h>
#include <gtc/matrix_transform.hpp>

const int HotkeyDel = 58;

ModuleVolume::ModuleVolume() {
    m_work_pending = 0;
    m_selected = -1;
    m_render_volume = true;
}

ModuleVolume::~ModuleVolume() {
}

void ModuleVolume::OnLoad(Scene* s) {
    m_scene = s;
    m_scene->RegisterHotkey(this, HotkeyDel, GLFW_KEY_DELETE, false, false, false);
}

void ModuleVolume::OnShutdown() {
    m_scene->UnregisterEntitiesByModule(this);
}

void ModuleVolume::OnDrawMenu() {
}

void ModuleVolume::OnDrawUI() {
    ImGui::Begin("Volume");

    ImGui::Text("LMB to select a region");
    ImGui::Text("Shift LMB to place a new region");
    ImGui::Text("DEL to delete a selected region");
    ImGui::Text("Arrow keys to move a selected region");
    ImGui::Text("PgUp/PgDown to adjust region up and down");
    ImGui::Text("[ & ] to rotate a selected region");
    ImGui::Text("l to expand & , to shrink a region on the x axis");
    ImGui::Text("; to expand & . to shrink a region on the y axis");
    ImGui::Text("' to expand & / to shrink a region on the z axis");
    if(m_selected >= 0) {
        ImGui::Separator();

        auto& region = m_regions[m_selected];

        ImGui::Text("Selected Region: %d", m_selected);

        bool needs_update = false;

        const char* region_identifiers[] = {"Normal",
                                            "Water",
                                            "Lava",
                                            "ZoneLine",
                                            "PVP",
                                            "Slime",
                                            "Ice",
                                            "V Water",
                                            "Generic Area",
                                            "Prefer Pathing",
                                            "Disable NavigationMesh"};
        if(ImGui::Combo("Area type", (int*)&region.area_type, region_identifiers, 11)) {
            m_modified = true;
        }

        if(ImGui::DragFloat("X", &region.pos.x)) {
            m_modified = true;
            needs_update = true;
        }

        if(ImGui::DragFloat("Y", &region.pos.y)) {
            m_modified = true;
            needs_update = true;
        }

        if(ImGui::DragFloat("Z", &region.pos.z)) {
            m_modified = true;
            needs_update = true;
        }

        ImGui::Separator();

        if(ImGui::DragFloat("X rot", &region.rot.x)) {
            m_modified = true;
            needs_update = true;
        }

        if(ImGui::DragFloat("Y rot", &region.rot.y)) {
            m_modified = true;
            needs_update = true;
        }

        if(ImGui::DragFloat("Z rot", &region.rot.z)) {
            m_modified = true;
            needs_update = true;
        }

        ImGui::Separator();

        if(ImGui::DragFloat("X ext", &region.extents.x)) {
            m_modified = true;
            needs_update = true;
        }

        if(ImGui::DragFloat("Y ext", &region.extents.y)) {
            m_modified = true;
            needs_update = true;
        }

        if(ImGui::DragFloat("Z ext", &region.extents.z)) {
            m_modified = true;
            needs_update = true;
        }

        if(needs_update) {
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    // ImGui::Separator();
    // if (m_work_pending == 0) {
    //	if (ImGui::Button("Build From Watermap (slow)")) {
    //		BuildFromWatermap(glm::vec3(0));
    //	}
    //}
    // else {
    //	ImGui::Text("Jobs in progress: %i", m_work_pending);
    //}

    ImGui::End();
}

void ModuleVolume::OnDrawOptions() {
    if(ImGui::Checkbox("Render Volumes", &m_render_volume)) {
        if(m_render_volume) {
            for(auto& vol : m_volumes) {
                m_scene->RegisterEntity(this, vol.get(), true);
            }
        } else {
            for(auto& vol : m_volumes) {
                m_scene->UnregisterEntity(this, vol.get());
            }
        }
    }
}

void ModuleVolume::OnSceneLoad(const char* zone_name) {
    m_modified = false;
    m_selected = -1;
    if(!LoadVolumes(eqemu::core::config::instance().get_path("volume", "maps/volume") + "/")) {
        LoadVolumes(eqemu::core::config::instance().get_path("water", "maps/water") + "/");
    }

    BuildVolumeEntities();
}

void ModuleVolume::OnSuspend() {
}

void ModuleVolume::OnResume() {
}

bool ModuleVolume::HasWork() {
    return m_work_pending > 0;
}

bool ModuleVolume::CanSave() {
    return m_modified;
}

void ModuleVolume::Save() {
    if(CanSave()) {
        std::string filename =
            eqemu::core::config::instance().get_path("volume", "maps/volume") + "/" + m_scene->GetZoneName() + ".wtr";
        FILE* f = fopen(filename.c_str(), "wb");
        if(f) {
            fwrite("EQEMUWATER", 10, 1, f);

            uint32_t version = 2;
            fwrite(&version, sizeof(uint32_t), 1, f);

            uint32_t region_count = (uint32_t)m_regions.size();
            fwrite(&region_count, sizeof(uint32_t), 1, f);
            for(auto& region : m_regions) {
                uint32_t region_type = (uint32_t)region.area_type;
                fwrite(&region_type, sizeof(uint32_t), 1, f);
                fwrite(&region.pos.x, sizeof(float), 1, f);
                fwrite(&region.pos.y, sizeof(float), 1, f);
                fwrite(&region.pos.z, sizeof(float), 1, f);
                fwrite(&region.rot.x, sizeof(float), 1, f);
                fwrite(&region.rot.y, sizeof(float), 1, f);
                fwrite(&region.rot.z, sizeof(float), 1, f);
                fwrite(&region.scale.x, sizeof(float), 1, f);
                fwrite(&region.scale.y, sizeof(float), 1, f);
                fwrite(&region.scale.z, sizeof(float), 1, f);
                fwrite(&region.extents.x, sizeof(float), 1, f);
                fwrite(&region.extents.y, sizeof(float), 1, f);
                fwrite(&region.extents.z, sizeof(float), 1, f);
            }
            fclose(f);
            m_modified = false;

            WaterMap* wm = WaterMap::LoadWaterMapfile(
                eqemu::core::config::instance().get_path("volume", "maps/volume") + "/", m_scene->GetZoneName());
            if(wm) {
                m_scene->GetZonePhysics()->SetWaterMap(wm);
            }
        }
    }
}

void ModuleVolume::OnHotkey(int ident) {
    switch(ident) {
        case HotkeyDel:
            if(m_selected != -1) {
                m_regions.erase(m_regions.begin() + m_selected);
                m_selected = -1;
                BuildVolumeEntities();
            }
            break;
        default: break;
    }
}

void ModuleVolume::OnClick(int mouse_button,
                           const glm::vec3* collide_hit,
                           const glm::vec3* select_hit,
                           Entity* selected) {
    auto& io = ImGui::GetIO();
    if(mouse_button == GLFW_MOUSE_BUTTON_1 && !io.KeyShift && !io.KeyCtrl && selected) {
        int i = 0;
        for(auto& volume : m_volumes) {
            if(volume.get() == selected) {
                if(m_selected >= 0) {
                    auto& c = m_volumes[m_selected];
                    c->SetTint(glm::vec4(0.0, 0.0, 1.0, 0.5));
                }

                m_selected = i;
                volume->SetTint(glm::vec4(0.0, 0.0, 0.8, 0.5));
                return;
            }

            i++;
        }
    } else if(mouse_button == GLFW_MOUSE_BUTTON_1 && io.KeyShift && collide_hit) {
        Region t;
        t.area_type = RegionTypeWater;
        t.pos = glm::vec3(collide_hit->z, collide_hit->x, collide_hit->y);
        t.rot = glm::vec3(0.0f);
        t.scale = glm::vec3(1.0f);
        t.extents = glm::vec3(10.0f);
        t.obb = eqemu::math::oriented_bounding_box(t.pos, t.rot, t.scale, t.extents);

        m_regions.push_back(t);
        m_selected = m_regions.size() - 1;

        BuildVolumeEntities();

        m_modified = true;
    }
}

void ModuleVolume::Tick(float delta_time) {
    auto& io = ImGui::GetIO();
    auto window = m_scene->GetWindow();

    float speed = 20.0f;
    if(!io.WantCaptureKeyboard && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                   glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
        speed *= 6.0f;
    }

    // Translate X
    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];
            auto& trans = region.obb.get_transformation();
            auto new_trans = glm::translate(trans, glm::vec3(-speed * delta_time, 0.0f, 0.0f));
            auto new_pos = new_trans[3];
            region.pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];
            auto& trans = region.obb.get_transformation();
            auto new_trans = glm::translate(trans, glm::vec3(speed * delta_time, 0.0f, 0.0f));
            auto new_pos = new_trans[3];
            region.pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    // Translate Y
    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];
            auto& trans = region.obb.get_transformation();
            auto new_trans = glm::translate(trans, glm::vec3(0.0f, -speed * delta_time, 0.0f));
            auto new_pos = new_trans[3];
            region.pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];
            auto& trans = region.obb.get_transformation();
            auto new_trans = glm::translate(trans, glm::vec3(0.0f, speed * delta_time, 0.0f));
            auto new_pos = new_trans[3];
            region.pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    // Translate Z
    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];
            auto& trans = region.obb.get_transformation();
            auto new_trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, -speed * delta_time));
            auto new_pos = new_trans[3];
            region.pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];
            auto& trans = region.obb.get_transformation();
            auto new_trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, speed * delta_time));
            auto new_pos = new_trans[3];
            region.pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    // Rotate around Z
    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.rot.z -= speed * 0.25f;
            if(region.rot.z < -90.0f) {
                region.rot.z += 180.0f;
            }

            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.rot.z += speed * 0.25f;
            if(region.rot.z > 90.0f) {
                region.rot.z -= 180.0f;
            }

            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    // Expand
    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.extents.x += speed * 0.01f;
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.extents.y += speed * 0.01f;
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_APOSTROPHE) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.extents.z += speed * 0.01f;
            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    // Shrink
    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.extents.x -= speed * 0.01f;
            if(region.extents.x < 0.1f) {
                region.extents.x = 0.1f;
            }

            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.extents.y -= speed * 0.01f;
            if(region.extents.y < 0.1f) {
                region.extents.y = 0.1f;
            }

            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }

    if(!io.WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS) {
        if(m_selected != -1) {
            auto& region = m_regions[m_selected];

            region.extents.z -= speed * 0.01f;
            if(region.extents.z < 0.1f) {
                region.extents.z = 0.1f;
            }

            region.obb = eqemu::math::oriented_bounding_box(region.pos, region.rot, region.scale, region.extents);
            BuildVolumeEntities();
        }
    }
}

bool ModuleVolume::LoadVolumes(const std::string& dir) {
    m_regions.clear();
    std::string filename = dir + m_scene->GetZoneName() + ".wtr";
    FILE* f = fopen(filename.c_str(), "rb");
    if(f) {
        char magic[10];
        uint32_t version;
        if(fread(magic, 10, 1, f) != 1) {
            fclose(f);
            return false;
        }

        if(strncmp(magic, "EQEMUWATER", 10)) {
            fclose(f);
            return false;
        }

        if(fread(&version, sizeof(version), 1, f) != 1) {
            fclose(f);
            return false;
        }

        if(version == 2) {
            uint32_t region_count;
            if(fread(&region_count, sizeof(region_count), 1, f) != 1) {
                fclose(f);
                return false;
            }

            for(uint32_t i = 0; i < region_count; ++i) {
                uint32_t region_type;
                float x;
                float y;
                float z;
                float x_rot;
                float y_rot;
                float z_rot;
                float x_scale;
                float y_scale;
                float z_scale;
                float x_extent;
                float y_extent;
                float z_extent;

                if(fread(&region_type, sizeof(region_type), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&x, sizeof(x), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&y, sizeof(y), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&z, sizeof(z), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&x_rot, sizeof(x_rot), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&y_rot, sizeof(y_rot), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&z_rot, sizeof(z_rot), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&x_scale, sizeof(x_scale), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&y_scale, sizeof(y_scale), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&z_scale, sizeof(z_scale), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&x_extent, sizeof(x_extent), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&y_extent, sizeof(y_extent), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                if(fread(&z_extent, sizeof(z_extent), 1, f) != 1) {
                    fclose(f);
                    return false;
                }

                Region r;
                r.area_type = region_type;
                r.pos = glm::vec3(x, y, z);
                r.rot = glm::vec3(x_rot, y_rot, z_rot);
                r.scale = glm::vec3(x_scale, y_scale, z_scale);
                r.extents = glm::vec3(x_extent, y_extent, z_extent);
                r.obb = eqemu::math::oriented_bounding_box(r.pos, r.rot, r.scale, r.extents);
                m_regions.push_back(r);
            }

            fclose(f);
            return true;
        } else {
            fclose(f);
            return false;
        }
    }

    return false;
}

void ModuleVolume::BuildVolumeEntities() {
    for(auto& ent : m_volumes) {
        m_scene->UnregisterEntity(this, ent.get());
    }

    m_volumes.clear();

    int i = 0;
    for(auto& region : m_regions) {
        DynamicGeometry* g = new DynamicGeometry();
        auto color = glm::vec3(1.0f, 1.0f, 1.0f);

        float min_x = region.obb.get_min_x();
        float min_y = region.obb.get_min_y();
        float min_z = region.obb.get_min_z();
        float max_x = region.obb.get_max_x();
        float max_y = region.obb.get_max_y();
        float max_z = region.obb.get_max_z();

        glm::vec4 v1(min_x, max_y, min_z, 1.0f);
        glm::vec4 v2(min_x, max_y, max_z, 1.0f);
        glm::vec4 v3(max_x, max_y, max_z, 1.0f);
        glm::vec4 v4(max_x, max_y, min_z, 1.0f);
        glm::vec4 v5(min_x, min_y, min_z, 1.0f);
        glm::vec4 v6(min_x, min_y, max_z, 1.0f);
        glm::vec4 v7(max_x, min_y, max_z, 1.0f);
        glm::vec4 v8(max_x, min_y, min_z, 1.0f);

        v1 = region.obb.get_transformation() * v1;
        v2 = region.obb.get_transformation() * v2;
        v3 = region.obb.get_transformation() * v3;
        v4 = region.obb.get_transformation() * v4;
        v5 = region.obb.get_transformation() * v5;
        v6 = region.obb.get_transformation() * v6;
        v7 = region.obb.get_transformation() * v7;
        v8 = region.obb.get_transformation() * v8;

        auto& inds = g->GetInds();
        auto& verts = g->GetVerts();
        auto& colors = g->GetVertColors();

        verts.push_back(glm::vec3(v1.y, v1.z, v1.x));
        verts.push_back(glm::vec3(v2.y, v2.z, v2.x));
        verts.push_back(glm::vec3(v3.y, v3.z, v3.x));
        verts.push_back(glm::vec3(v4.y, v4.z, v4.x));
        verts.push_back(glm::vec3(v5.y, v5.z, v5.x));
        verts.push_back(glm::vec3(v6.y, v6.z, v6.x));
        verts.push_back(glm::vec3(v7.y, v7.z, v7.x));
        verts.push_back(glm::vec3(v8.y, v8.z, v8.x));

        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);

        // top
        inds.push_back(0);
        inds.push_back(1);
        inds.push_back(2);
        inds.push_back(2);
        inds.push_back(3);
        inds.push_back(0);

        inds.push_back(2);
        inds.push_back(1);
        inds.push_back(0);
        inds.push_back(0);
        inds.push_back(3);
        inds.push_back(2);

        // back
        inds.push_back(1);
        inds.push_back(2);
        inds.push_back(6);
        inds.push_back(6);
        inds.push_back(5);
        inds.push_back(1);

        inds.push_back(6);
        inds.push_back(2);
        inds.push_back(1);
        inds.push_back(1);
        inds.push_back(5);
        inds.push_back(6);

        // bottom
        inds.push_back(4);
        inds.push_back(5);
        inds.push_back(6);
        inds.push_back(6);
        inds.push_back(7);
        inds.push_back(4);

        inds.push_back(6);
        inds.push_back(5);
        inds.push_back(4);
        inds.push_back(4);
        inds.push_back(7);
        inds.push_back(6);

        // front
        inds.push_back(0);
        inds.push_back(3);
        inds.push_back(7);
        inds.push_back(7);
        inds.push_back(4);
        inds.push_back(0);

        inds.push_back(7);
        inds.push_back(3);
        inds.push_back(0);
        inds.push_back(0);
        inds.push_back(4);
        inds.push_back(7);

        // left
        inds.push_back(0);
        inds.push_back(1);
        inds.push_back(5);
        inds.push_back(5);
        inds.push_back(4);
        inds.push_back(0);

        inds.push_back(5);
        inds.push_back(1);
        inds.push_back(0);
        inds.push_back(0);
        inds.push_back(4);
        inds.push_back(5);

        // right
        inds.push_back(3);
        inds.push_back(2);
        inds.push_back(6);
        inds.push_back(6);
        inds.push_back(7);
        inds.push_back(3);

        inds.push_back(6);
        inds.push_back(2);
        inds.push_back(3);
        inds.push_back(3);
        inds.push_back(7);
        inds.push_back(6);

        g->SetBlend(true);
        if(i == m_selected) {
            g->SetTint(glm::vec4(0.0, 0.8, 0.0, 0.5));
        } else {
            g->SetTint(glm::vec4(0.0, 0.0, 0.8, 0.5));
        }

        g->Update();
        m_volumes.push_back(std::unique_ptr<DynamicGeometry>(g));

        ++i;
    }

    for(auto& vol : m_volumes) {
        m_scene->RegisterEntity(this, vol.get(), true);
    }
}

struct WaterMapTile {
    glm::vec2 min;
    glm::vec2 max;
};

struct RegionSpan {
    WaterRegionType region_type;
    double start;
    double end;
};

void ModuleVolume::BuildFromWatermap(const glm::vec3& pos) {
    auto physics = m_scene->GetZonePhysics();
    if(!physics) {
        return;
    }

    if(m_work_pending != 0) {
        return;
    }

    auto& min = m_scene->GetBoundingBoxMin();
    auto& max = m_scene->GetBoundingBoxMax();

    auto steps_x = (int)ceilf((max.x - min.x) / 256.0f);
    auto steps_y = (int)ceilf((max.y - min.y) / 256.0f);

    // chunk zone into tiles
    for(auto i = 0; i < steps_x; ++i) {
        for(auto j = 0; j < steps_y; ++j) {
            glm::vec2 minv(min.x + (256.0f * i), min.y + (256.0f * j));
            glm::vec2 maxv(min.x + (256.0f * (i + 1)), min.y + (256.0f * (j + 1)));

            if(maxv.x > max.x) {
                maxv.x = max.x;
            }

            if(maxv.y > max.y) {
                maxv.y = max.y;
            }

            eqLogMessage(LogInfo, "Create Tile (%.2f, %.2f) (%.2f, %.2f)", minv.x, minv.y, maxv.x, maxv.y);
            m_work_pending++;

            // EQ::BackgroundTask task(
            //    []() {
            //
            //    },
            //    [this]() { m_work_pending--; });
        }
    }
}
