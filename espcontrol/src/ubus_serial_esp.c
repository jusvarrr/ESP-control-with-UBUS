#include "../include/ubus_serial_esp.h"

static struct ubus_context *ctx = NULL;

enum {
    PORT_NAME,
    PIN,
    SENSOR,
    MODEL,
    __PARAMS_MAX
};

static int pin_on(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg);
static int pin_off(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg);
static int ports_list(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg);
static int pin_get(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg);

static const struct blobmsg_policy on_off_policy[] = {
    [PORT_NAME] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
};
static const struct blobmsg_policy get_policy[] = {
    [PORT_NAME] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
    [SENSOR] = { .name = "sensor", .type = BLOBMSG_TYPE_STRING },
    [MODEL] = { .name = "model", .type = BLOBMSG_TYPE_STRING },
};

static const struct ubus_method esp_control_methods[] = {
    UBUS_METHOD("on", pin_on, on_off_policy),
    UBUS_METHOD("off", pin_off, on_off_policy),
    UBUS_METHOD("get", pin_get, get_policy),
    UBUS_METHOD_NOARG("devices", ports_list)
};

static struct ubus_object_type esp_control_type = 
    UBUS_OBJECT_TYPE("esp", esp_control_methods);

static struct ubus_object esp_control_obj = {
    .name = "esp",
    .type = &esp_control_type,
    .methods = esp_control_methods,
    .n_methods = ARRAY_SIZE(esp_control_methods),
};


static int pin_on(struct ubus_context *ctx, struct ubus_object *obj,
 struct ubus_request_data *req, const char *method, struct blob_attr *msg) {

    int err = SP_OK;

    char data[65];
    char buff[100] = {0};

    struct blob_buf reply = {};
    struct blob_attr *tb[__PARAMS_MAX];


    blobmsg_parse(on_off_policy, ARRAY_SIZE(on_off_policy), tb, blob_data(msg), blob_len(msg));
    if (!tb[PORT_NAME] || !tb[PIN])
        return UBUS_STATUS_INVALID_ARGUMENT;
    
    char *port_name = blobmsg_get_string(tb[PORT_NAME]);
    int pin = blobmsg_get_u32(tb[PIN]);
    
    snprintf(data, sizeof(char)*65, "{\"action\": \"on\", \"pin\": %d}", pin);

    if ((err = send_command(port_name, data)) == (-1) || !is_esp(port_name)) {
        format_json_error(req, &reply, ctx);
        return err;
    } else if (err != SP_OK)
        return err;
        
    if ((err = get_response(port_name, buff, 65)) != SP_OK) {
        return err;
    }

    blob_buf_init(&reply, 0);
    blobmsg_add_json_from_string(&reply, buff);
    ubus_send_reply(ctx, req, reply.head);
    blob_buf_free(&reply);

    return UBUS_STATUS_OK;

}

static int pin_off(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg) {

    int err = SP_OK;
    char data[65];
    char buff[100] = {0};
    struct blob_buf reply = {};
    struct blob_attr *tb[__PARAMS_MAX];

    blobmsg_parse(on_off_policy, ARRAY_SIZE(on_off_policy), tb, blob_data(msg), blob_len(msg));

    if (!tb[PORT_NAME] || !tb[PIN]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    char *port_name = blobmsg_get_string(tb[PORT_NAME]);
    int pin = blobmsg_get_u32(tb[PIN]);

    snprintf(data, sizeof(char)*65, "{\"action\": \"off\", \"pin\": %d}", pin);

    if ((err = send_command(port_name, data)) == (-1) || !is_esp(port_name)) {
        format_json_error(req, &reply, ctx);
        return err;
    } else if (err != SP_OK)
        return err;

    if ((err = get_response(port_name, buff, 65)) != SP_OK) {
        return err;
    }
    blob_buf_init(&reply, 0);

    blobmsg_add_json_from_string(&reply, buff);
    
    ubus_send_reply(ctx, req, reply.head);
    blob_buf_free(&reply);
    return UBUS_STATUS_OK;

}

static int ports_list(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg) {

    int err = SP_OK;
    struct esp_port **selected_ports = NULL;
    struct blob_buf reply_buffer = {};
    
    int result = find_esp_devices(&selected_ports);
    
    list_ports_format(selected_ports, &reply_buffer);

    port_list_cleanup(&selected_ports);
    
    ubus_send_reply(ctx, req, reply_buffer.head);
    blob_buf_free(&reply_buffer);
    
    return UBUS_STATUS_OK;
}

static int pin_get(struct ubus_context *ctx, struct ubus_object *obj,
    struct ubus_request_data *req, const char *method, struct blob_attr *msg) {

    int err = SP_OK;
    char data[256];
    char port_name[51] = {0};
    char sensor[51] = {0};
    char model[51] = {0};
    char buff[256] = {0};

    struct blob_attr *tb[__PARAMS_MAX];

    struct blob_buf reply = {};

    blobmsg_parse(get_policy, ARRAY_SIZE(get_policy), tb, blob_data(msg), blob_len(msg));

    if (!tb[PORT_NAME] || !tb[PIN] || !tb[SENSOR] || !tb[MODEL]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    strncpy(port_name, blobmsg_get_string(tb[PORT_NAME]), 50);
    port_name[51] = '\0';

    strncpy(sensor, blobmsg_get_string(tb[SENSOR]), 50);
    sensor[51] = '\0';

    strncpy(model, blobmsg_get_string(tb[MODEL]), 50);
    model[51] = '\0';

    int pin = blobmsg_get_u32(tb[PIN]);

    snprintf(data, 256, "{\"action\": \"get\", \"sensor\": \"%s\", \"pin\": %d, \"model\": \"%s\"}", sensor, pin, model);

    if ((err = send_command(port_name, data)) == (-1) || !is_esp(port_name)) {
        format_json_error(req, &reply, ctx);
        return err;
    } else if (err != SP_OK)
        return err;

    if ((err = get_response(port_name, buff, 256)) != SP_OK) {
        return err;
    }

    blob_buf_init(&reply, 0);

    blobmsg_add_json_from_string(&reply, buff);
    
    ubus_send_reply(ctx, req, reply.head);
    blob_buf_free(&reply);

    return UBUS_STATUS_OK;
}

int ubus_run(struct esp_port ***selected_ports) {

    uloop_init();
    ctx = ubus_connect(NULL);

    if (!ctx) {
        fprintf(stderr, "Failed to connect to UBUS\n");
        return -1;
    }

    ubus_add_uloop(ctx);

    if (ubus_add_object(ctx, &esp_control_obj)) {
        printf("Failed to add UBUS object\n");
        port_list_cleanup(selected_ports);
        return -1;
    }


    printf("UBUS object registered successfully. Listening for commands...\n");

    uloop_run();

}

void cleanup_ubus_context() {
    if (ctx) {
        ubus_free(ctx);
        ctx = NULL;
    }
}