#include "azone_map.h"

#include <boost/program_options.hpp>
#include <entt/entt.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string/transform.h>

#include <cstdlib>
#include <iostream>

namespace prog = boost::program_options;

void setup_logger(bool verbose) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("azone.log", true);
    auto logger = std::shared_ptr<spdlog::logger>(new spdlog::logger("azone", {console_sink, file_sink}));

    if(verbose) {
        logger->set_level(spdlog::level::trace);
    }

    entt::service_locator<spdlog::logger>::set(logger);
}

void print_usage(const prog::options_description& desc) {
    std::cout << "Usage: azone [options] zone...\n";
    std::cout << desc << "\n";
}

int main(int argc, char** argv) {
    prog::options_description desc("options");
    auto options = desc.add_options();
    options("help,H", "display help message");
    options("verbose,V", "use verbose logging");
    options("include-collide-mat,I", "include the invis collide materials in the map geometry");
    options("zones,Z", prog::value<std::vector<std::string>>(), "zones to process");

    prog::positional_options_description pos;
    pos.add("zones", -1);
    std::shared_ptr<spdlog::logger> logger;

    try {
        prog::variables_map vm;
        prog::store(prog::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
        prog::notify(vm);

        if(vm.count("help")) {
            print_usage(desc);
            return EXIT_FAILURE;
        }

        if(vm.count("verbose")) {
            setup_logger(true);
        } else {
            setup_logger(false);
        }

        logger = entt::service_locator<spdlog::logger>::get().lock();

        std::vector<std::string> zones;
        if(vm.count("zones")) {
            zones = vm["zones"].as<std::vector<std::string>>();
        }

        if(zones.empty()) {
            logger->critical("no input zones");
            return EXIT_FAILURE;
        }

        bool ignore_collide_tex = true;
        if(vm.count("include-collide-tex")) {
            ignore_collide_tex = false;
        }

        for(auto& zone : zones) {
            eqemu::azone::map m;

            logger->info("building map for {0}", zone);
            if(!m.build(zone, ignore_collide_tex)) {
                logger->warn("failed to build map for {0}", zone);
            } else {
                if(!m.write(zone + ".map")) {
                    logger->warn("failed to write map for {0}", zone);
                } else {
                    logger->info("wrote map for {0}", zone);
                }
            }
        }
    } catch(std::exception& ex) {
        if(logger) {
            logger->critical("error running azone: {0}", ex.what());
        }

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
