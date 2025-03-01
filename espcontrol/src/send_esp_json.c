#include "../include/send_esp_json.h"

void list_ports_format(struct esp_port **selected_ports, struct blob_buf *reply) {

    blob_buf_init(reply, 0);
    char hex_buffer[16];

    void *devices_array = blobmsg_open_array(reply, "devices");

    if (selected_ports == NULL || selected_ports[0] == NULL) {
        blobmsg_close_array(reply, devices_array);
    } else {
        for (int i = 0; selected_ports[i] != NULL; i++) {
            struct esp_port *current_port = selected_ports[i];

            void *device_object = blobmsg_open_table(reply, NULL);

            snprintf(hex_buffer, sizeof(hex_buffer), "0x%04X", current_port->usb_vid);
            blobmsg_add_string(reply, "vendor_id", hex_buffer);
            snprintf(hex_buffer, sizeof(hex_buffer), "0x%04X", current_port->usb_pid);
            blobmsg_add_string(reply, "product_id", hex_buffer);
            blobmsg_add_string(reply, "port", current_port->port);

            blobmsg_close_table(reply, device_object);
        }

        blobmsg_close_array(reply, devices_array);

    }
}

void format_json_error(struct ubus_request_data *req, struct blob_buf *reply, struct ubus_context *ctx) {

    char error_reply[] = "{\"rc\": -10015,	\"msg\": \"Incorrect port name.\"}";

    blob_buf_init(reply, 0);
    blobmsg_add_json_from_string(reply, error_reply);
    ubus_send_reply(ctx, req, reply->head);
    blob_buf_free(reply);
}