#ifndef ESP_CONTROL_H
#define ESP_CONTROL_H

#include <libserialport.h>

struct esp_port {
    char product[256];
    char port[256];
    int usb_vid;
    int usb_pid;
    enum sp_transport transport;
};

struct esp_port read_port_info(struct sp_port *port);
int find_esp_devices(struct esp_port ***selected_ports);
int send_command(char *name, char *command_line);
int get_response(char *port, char *line, int size);
int is_esp(char *port_name);
void port_list_cleanup();

#endif