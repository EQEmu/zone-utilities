#ifndef EQEMU_COMMON_EQG_TERRAIN_H
#define EQEMU_COMMON_EQG_TERRAIN_H

#include "eqg_invis_wall.h"
#include "eqg_model_loader.h"
#include "eqg_region.h"
#include "eqg_terrain_tile.h"
#include "eqg_water_sheet.h"
#include "placeable_group.h"
#include <map>
#include <memory>

namespace EQEmu {

    namespace EQG {

        class Terrain {
        public:
            struct ZoneOptions {
                std::string name;
                int32_t min_lng;
                int32_t max_lng;
                int32_t min_lat;
                int32_t max_lat;
                float min_extents[3];
                float max_extents[3];
                float units_per_vert;
                uint32_t quads_per_tile;
                uint32_t cover_map_input_size;
                uint32_t layer_map_input_size;
                float base_water_level;
            };

            Terrain() {}
            ~Terrain() {}

            void AddTile(std::shared_ptr<TerrainTile> t) { tiles.push_back(t); }
            void SetQuadsPerTile(uint32_t value) { quads_per_tile = value; }
            void SetUnitsPerVertex(float value) { units_per_vertex = value; }
            void AddWaterSheet(std::shared_ptr<WaterSheet> s) { water_sheets.push_back(s); }
            void AddInvisWall(std::shared_ptr<InvisWall> w) { invis_walls.push_back(w); }
            void AddPlaceableGroup(std::shared_ptr<PlaceableGroup> p) { placeable_groups.push_back(p); }
            void AddRegion(std::shared_ptr<EQG::Region> r) { regions.push_back(r); }
            void SetOpts(ZoneOptions opt) { opts = opt; }

            std::vector<std::shared_ptr<TerrainTile>>& GetTiles() { return tiles; }
            uint32_t GetQuadsPerTile() { return quads_per_tile; }
            float GetUnitsPerVertex() { return units_per_vertex; }
            std::vector<std::shared_ptr<WaterSheet>>& GetWaterSheets() { return water_sheets; }
            std::vector<std::shared_ptr<InvisWall>>& GetInvisWalls() { return invis_walls; }
            std::vector<std::shared_ptr<PlaceableGroup>>& GetPlaceableGroups() { return placeable_groups; }
            std::map<std::string, std::shared_ptr<EQG::Geometry>>& GetModels() { return models; }
            std::vector<std::shared_ptr<EQG::Region>>& GetRegions() { return regions; }
            ZoneOptions& GetOpts() { return opts; }

        private:
            std::vector<std::shared_ptr<TerrainTile>> tiles;
            uint32_t quads_per_tile;
            float units_per_vertex;

            std::vector<std::shared_ptr<WaterSheet>> water_sheets;
            std::vector<std::shared_ptr<InvisWall>> invis_walls;
            std::vector<std::shared_ptr<PlaceableGroup>> placeable_groups;
            std::map<std::string, std::shared_ptr<EQG::Geometry>> models;
            std::vector<std::shared_ptr<EQG::Region>> regions;
            ZoneOptions opts;
        };

    }    // namespace EQG

}    // namespace EQEmu

#endif
