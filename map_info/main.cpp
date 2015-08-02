#include <map>
#include <mutex>
#include <thread>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "lib/libwebsockets.h"
#include "string_util.h"
#include "zone_map.h"
#include "water_map.h"

libwebsocket_context *context = nullptr;
bool run = true;
std::mutex map_mutex;
std::map<std::string, bool> map_loaded;
std::map<std::string, std::shared_ptr<ZoneMap>> map_ptrs;
std::map<std::string, std::shared_ptr<WaterMap>> water_map_ptrs;

void catch_signal(int sig_num) {
	run = false;

	if(context)
		libwebsocket_cancel_service(context);
}

struct http_session_info {
	unsigned char buffer[1024];
	unsigned int sz;
};

int callback_http(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {
	static unsigned char buffer[4096];
	struct http_session_info *session = (struct http_session_info*)user;

	switch(reason) {
	case LWS_CALLBACK_HTTP: {
		auto args = EQEmu::SplitString(std::string((const char*)in + 1), '/');
		std::string command;
		if(args.size() > 0) {
			command = args[0];
			if(command.compare("info") == 0) {
				if(args.size() < 5) {
					libwebsockets_return_http_status(context, wsi, HTTP_STATUS_NOT_FOUND, NULL);
					return -1;
				}

				std::string zone = args[1];
				double x = stof(args[2]);
				double y = stof(args[3]);
				double z = stof(args[4]);

				//find map info here
				std::shared_ptr<ZoneMap> m;
				std::shared_ptr<WaterMap> wm;
				std::string status = "ok";
				map_mutex.lock();
				if(map_loaded.count(zone) == 0) {
					status = "map not loaded";
				} else {
					m = map_ptrs[zone];
					wm = water_map_ptrs[zone];
				}

				double best_z = BEST_Z_INVALID;
				if(m) {
					glm::vec3 v(x, y, z);
					best_z = m->FindBestZ(v, nullptr, nullptr);
				}

				std::string area = "unknown";
				if(wm) {
					WaterRegionType region = wm->ReturnRegionType(x, y, z);
					if(region == RegionTypeUntagged) {
						area = "untagged";
					}
					else if(region == RegionTypeNormal) {
						area = "normal";
					}
					else if(region == RegionTypeWater) {
						area = "water";
					}
					else if(region == RegionTypeLava) {
						area = "lava";
					}
					else if(region == RegionTypeZoneLine) {
						area = "zone line";
					}
					else if(region == RegionTypePVP) {
						area = "pvp";
					}
					else if(region == RegionTypeSlime) {
						area = "slime";
					}
					else if(region == RegionTypeIce) {
						area = "ice";
					}
					else if(region == RegionTypeVWater) {
						area = "vwater";
					}
				}

				map_mutex.unlock();

				unsigned char *p = session->buffer;
				p += sprintf((char*)p, "{status: '%s', best_z: %f, area: '%s'}", status.c_str(), best_z, area.c_str());
				session->sz = p - session->buffer;
			}
			else if(command.compare("load") == 0) {
				if(args.size() < 2) {
					libwebsockets_return_http_status(context, wsi, HTTP_STATUS_NOT_FOUND, NULL);
					return -1;
				}

				std::string zone = args[1];

				if(zone.length() > 64) {
					libwebsockets_return_http_status(context, wsi, HTTP_STATUS_BAD_REQUEST, NULL);
					return -1;
				}

				map_mutex.lock();
				std::thread t([](const std::string &z) {
					std::shared_ptr<ZoneMap> m(ZoneMap::LoadMapFile(z));
					std::shared_ptr<WaterMap> wm(WaterMap::LoadWaterMapfile(z));
					map_mutex.lock();
					map_loaded[z] = true;
					map_ptrs[z] = m;
					water_map_ptrs[z] = wm;
					map_mutex.unlock();
				}, zone);
				t.detach();
				map_mutex.unlock();

				unsigned char *p = session->buffer;
				p += sprintf((char*)p, "{status: 'loading'}");
				session->sz = p - session->buffer;
			}
			else if(command.compare("unload") == 0) {
				if(args.size() < 2) {
					libwebsockets_return_http_status(context, wsi, HTTP_STATUS_NOT_FOUND, NULL);
					return -1;
				}

				std::string zone = args[1];

				map_mutex.lock();
				map_loaded.erase(zone);
				map_ptrs.erase(zone);
				water_map_ptrs.erase(zone);
				map_mutex.unlock();

				if(zone.length() > 64) {
					libwebsockets_return_http_status(context, wsi, HTTP_STATUS_BAD_REQUEST, NULL);
					return -1;
				}

				unsigned char *p = session->buffer;
				p += sprintf((char*)p, "{status: 'unloaded'}");
				session->sz = p - session->buffer;
			}
		} else {
			libwebsockets_return_http_status(context, wsi, HTTP_STATUS_NOT_FOUND, NULL);
			return -1;
		}

		if(session->sz > 0) {
			unsigned char *p = buffer;
			p += sprintf((char *)p,
				"HTTP/1.0 200 OK\x0d\x0a"
				"Server: map_info\x0d\x0a"
				"Content-Type: application/json\x0d\x0a"
				"Content-Length: %u\x0d\x0a\x0d\x0a",
				session->sz);

			libwebsocket_write(wsi, buffer, p - buffer, LWS_WRITE_HTTP);
			libwebsocket_callback_on_writable(context, wsi);
		} else {
			libwebsockets_return_http_status(context, wsi, HTTP_STATUS_NOT_FOUND, NULL);
			return -1;
		}
		break;
	}
	case LWS_CALLBACK_HTTP_BODY_COMPLETION:
		libwebsockets_return_http_status(context, wsi, HTTP_STATUS_OK, NULL);
		return -1;
	case LWS_CALLBACK_HTTP_WRITEABLE:
		libwebsocket_write(wsi, session->buffer, session->sz, LWS_WRITE_HTTP);
		return -1;
	default:
		break;
	};
	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	{ "http-only", callback_http, sizeof(http_session_info), 0, },
	{ nullptr, nullptr, 0, 0 }
};

int main(int argc, char **argv) {
	if(signal(SIGINT, catch_signal) == SIG_ERR)	{
		return 1;
	}

	if(signal(SIGTERM, catch_signal) == SIG_ERR)	{
		return 1;
	}

	lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = EQEMU_MAP_INFO_PORT;
	info.protocols = protocols;
	info.extensions = nullptr;
	info.gid = -1;
	info.uid = -1;
	context = libwebsocket_create_context(&info);
	if(context == nullptr) {
		return 0;
	}

	while(run) {
		libwebsocket_service(context, 5);
	}

	libwebsocket_context_destroy(context);

	return 0;
}
