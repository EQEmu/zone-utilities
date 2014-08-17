#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "lib/libwebsockets.h"

libwebsocket_context *context = nullptr;
bool run = true;

void catch_signal(int sig_num) {
	run = false;

	if(context)
		libwebsocket_cancel_service(context);
}

int callback_http(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {
	switch(reason) {
	case LWS_CALLBACK_HTTP:
		libwebsockets_return_http_status(context, wsi, HTTP_STATUS_FORBIDDEN, NULL);
		break;
	case LWS_CALLBACK_HTTP_BODY_COMPLETION:
		libwebsockets_return_http_status(context, wsi, HTTP_STATUS_OK, NULL);
		return -1;
	default:
		break;
	};
	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	{ "http-only", callback_http, 0, 0, },
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
	info.port = 9082;
	info.protocols = protocols;
	info.extensions = nullptr;
	info.gid = -1;
	info.uid = -1;
	context = libwebsocket_create_context(&info);
	if(context == nullptr) {
		return 0;
	}

	while(run) {
		libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
		libwebsocket_service(context, 5);
	}

	libwebsocket_context_destroy(context);

	return 0;
}
