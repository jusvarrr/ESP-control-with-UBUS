#ifndef UBUS_SERIAL_ESP_H
#define UBUS_SERIAL_ESP_H

#include "send_esp_json.h"

int ubus_run(struct esp_port ***selected_ports);
void cleanup_ubus_context();

#endif