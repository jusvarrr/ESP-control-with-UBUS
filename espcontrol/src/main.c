//#include "../include/ubus_serial_esp.h"
#include "../include/esp_control.h"
#include "../include/ubus_serial_esp.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

char buff[100];
struct esp_port **selected_ports = NULL;

void cleanup() {
    cleanup_ubus_context();
    uloop_done();
    port_list_cleanup(&selected_ports);
}

void sig_handler(int signum){
    cleanup();
    exit(1);
}

void set_signal_handlers(){
    signal(SIGINT,sig_handler);
    signal(SIGHUP,sig_handler);
    signal(SIGQUIT,sig_handler);
    signal(SIGILL,sig_handler);
    signal(SIGTRAP,sig_handler);
    signal(SIGABRT,sig_handler);
    signal(SIGFPE,sig_handler);
    signal(SIGTERM,sig_handler);
}

int main(int argc, char **argv) {
    set_signal_handlers();

    ubus_run(&selected_ports);

    cleanup();

    exit(0);
}