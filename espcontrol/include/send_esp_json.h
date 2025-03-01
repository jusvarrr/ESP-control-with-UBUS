#ifndef UBUS_SEND_JSON_H
#define UBUS_SEND_JSON_H

#include "esp_control.h"
#include <libubox/blobmsg_json.h>
#include <libubus.h>

void list_ports_format(struct esp_port **selected_ports, struct blob_buf *reply);

void format_json_error(struct ubus_request_data *req, struct blob_buf *reply, struct ubus_context *ctx);

#endif