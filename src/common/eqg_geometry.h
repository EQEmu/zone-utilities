#ifndef EQEMU_EQG_GEOMETRY_H
#define EQEMU_EQG_GEOMETRY_H

#include "eqg_material.h"

#include <glm.hpp>
#include <vector>

namespace EQEmu {

    namespace EQG {

        class Geometry {
        public:
            struct Vertex {
                glm::vec3 pos;
                glm::vec2 tex;
                glm::vec3 nor;
                uint32_t col;
            };

            struct Polygon {
                uint32_t flags;
                uint32_t verts[3];
                int32_t material;
            };

            Geometry() {}
            ~Geometry() {}

            void AddMaterial(EQG::Material& mat) { mats.push_back(mat); }
            void AddVertex(Vertex& v) { verts.push_back(v); }
            void AddPolygon(Polygon& p) { polys.push_back(p); }
            void SetName(std::string nname) { name = nname; }

            std::string GetMaterialName(int32_t idx) {
                if(idx < 0 || idx >= (int32_t)mats.size()) {
                    return "";
                }
                return mats[idx].GetName();
            }
            std::vector<EQG::Material>& GetMaterials() { return mats; }
            std::vector<Vertex>& GetVertices() { return verts; }
            std::vector<Polygon>& GetPolygons() { return polys; }
            std::string& GetName() { return name; }

        private:
            std::vector<EQG::Material> mats;
            std::vector<Vertex> verts;
            std::vector<Polygon> polys;
            std::string name;
        };

    }    // namespace EQG

}    // namespace EQEmu

#endif
