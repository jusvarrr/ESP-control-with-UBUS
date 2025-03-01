#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/esp_control.h"

int port_close(struct sp_port *port) {
    int err = SP_OK;
    if ((err = sp_close(port)) != SP_OK)
        return err;
    if (port != NULL)
        sp_free_port(port);
    return err;
}

int check(enum sp_return result)
{
    char *error_message;
    switch (result) {
    case SP_ERR_ARG:
            printf("Error: Invalid argument.\n");
    case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            printf("Error: Failed: %s\n", error_message);
            sp_free_error_message(error_message);
    case SP_ERR_SUPP:
            printf("Error: Not supported.\n");
    case SP_ERR_MEM:
            printf("Error: Couldn't allocate memory.\n");
    }
    return result;
}

struct esp_port read_port_info(struct sp_port *port) {

    struct esp_port port_info = {0};
    if (port == NULL) {
        return port_info;
    }

    port_info.transport = sp_get_port_transport(port);

    const char *name = sp_get_port_name(port);
    if (name != NULL) {
        strncpy(port_info.port, name, sizeof(port_info.port) - 1);
        port_info.port[sizeof(port_info.port) - 1] = '\0';
    } else {
        port_info.port[0] = '\0';
    }

    const char *product = sp_get_port_usb_product(port);
    if (product != NULL) {
        strncpy(port_info.product, product, sizeof(port_info.product) - 1);
        port_info.product[sizeof(port_info.product) - 1] = '\0';
    } else {
        port_info.product[0] = '\0';
    }

    sp_get_port_usb_vid_pid(port, &port_info.usb_vid, &port_info.usb_pid);

    return port_info;
}

static int port_open(struct sp_port *port){
    int err = 0;
    if ((err = sp_open(port, SP_MODE_READ_WRITE)) != SP_OK) {
        if (port != NULL)
            sp_free_port(port);
    }
    return err;
}

static int port_config(struct sp_port *port){
    int err = 0;
    struct sp_port_config *config = NULL;
    int baudrate = 0, bits = 0, stopbits = 0;

    if (sp_set_baudrate(port, 9600) != SP_OK ||
        sp_set_bits(port, 8) != SP_OK ||
        sp_set_parity(port, SP_PARITY_NONE) != SP_OK ||
        sp_set_stopbits(port, 1) != SP_OK ||
        sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE) != SP_OK) {
        fprintf(stderr, "Failed to set baud rate for port: %s\n", sp_get_port_name(port));
        port_close(port);
        return -1;
    }
    
    if ((err = sp_new_config(&config)) != SP_OK) {
        fprintf(stderr, "Failed to create new port config\n");
        port_close(port);
        return err;
    }

    err = sp_get_config(port, config);
    
    check(sp_get_config_baudrate(config, &baudrate));
    check(sp_get_config_bits(config, &bits));
    check(sp_get_config_stopbits(config, &stopbits));
    sp_free_config(config);
    return err;
}

static int port_write(struct sp_port *port, char *command_line) {
    int err = 0;
    int size = strlen(command_line);
    unsigned int timeout = 1000;
    err = sp_blocking_write(port, command_line, size, timeout);
    if (err != size) {
        port_close(port);
        return err;
    }
    return SP_OK;
}

static int port_read(struct sp_port *port, char *read_buf, int size) {
    int len = 0;

    if ((len = sp_blocking_read(port, read_buf, size, 4000)) != 0) {
        read_buf[len] = '\0';
        return SP_OK;
    }
    port_close(port);
    return -1;
}

int send_command(char *name, char *command_line) {
    int err = 0;

    struct sp_port *port = NULL;
    if (sp_get_port_by_name(name, &port) != SP_OK)
        return -1;

    if ((err = port_open(port)) != SP_OK)
        return err;

    if ((err = port_config(port)) != SP_OK)
        return err;

    if ((err = port_write(port, command_line)) != SP_OK)
        return err;

    if ((err = sp_close(port)) != SP_OK)
        return err;

    sp_free_port(port);
    return SP_OK;
}

int get_response(char *name, char *line, int size) {
    int err = 0;

    struct sp_port *port = NULL;
    if (sp_get_port_by_name(name, &port) != SP_OK)
        return -1;

    if ((err = port_open(port)) != SP_OK)
        return err;

    if ((err = port_config(port)) != SP_OK)
        return err;

    if ((err = port_read(port, line, size)) != SP_OK)
        return err;

    if ((err = sp_close(port)) != SP_OK)
        return err;

    sp_free_port(port);
    return SP_OK;
}

int is_esp_by_struct(struct esp_port *esp_port) {
    if (esp_port->transport == SP_TRANSPORT_USB && 
        esp_port->usb_vid == 4292 && esp_port->usb_pid == 60000)
        return 1;
    else
        return 0;
}

int is_esp(char *port_name) {
    struct sp_port *port = NULL;
    struct esp_port p_info = {0};
    int is_esp = 0;

    if (sp_get_port_by_name(port_name, &port) != SP_OK) {
        fprintf(stderr, "Failed to get port: %s\n", port_name);
        return is_esp;
    }

    p_info = read_port_info(port);
    is_esp = is_esp_by_struct(&p_info);

    sp_free_port(port);
    return is_esp;
}

int find_esp_devices(struct esp_port ***selected_ports) {
    struct sp_port **port_list;
    *selected_ports = NULL;
    int selected_count = 0;
    struct esp_port p_info = {0};
    enum sp_return result = sp_list_ports(&port_list);

    if (result != SP_OK) {
        printf("sp_list_ports() failed!\n");
        return -1;
    }

    for (int i = 0; port_list[i] != NULL; i++) {
        struct sp_port *port = port_list[i];
        p_info = read_port_info(port);
        if (is_esp_by_struct(&p_info)) {

            struct esp_port **tmp = realloc(*selected_ports, sizeof(struct esp_port*) * (selected_count + 2));
            if (tmp == NULL) {
                free(*selected_ports);
                sp_free_port_list(port_list);
                return -2;
            }

            *selected_ports = tmp;
            struct esp_port *new_port = malloc(sizeof(struct esp_port));
            if (new_port == NULL) {
                free(*selected_ports);
                sp_free_port_list(port_list);
                return -3;
            }

            *new_port = p_info;
            (*selected_ports)[selected_count++] = new_port;
            (*selected_ports)[selected_count] = NULL; 

        }
    }

    sp_free_port_list(port_list);

    return 0;
}

void port_list_cleanup(struct esp_port ***selected_ports) {
    if (*selected_ports != NULL ) {
        for (int i = 0; (*selected_ports)[i] != NULL; i++) {
            free((*selected_ports)[i]);
        }
        free(*selected_ports);
        *selected_ports = NULL;
    }
}