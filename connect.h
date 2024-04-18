#ifndef CONNECT_H
#define CONNECT_H

int mqtt_init(char ssid[], char password[], char ip[]);
int mqtt_loop();

int mqtt_test(char ssid[], char password[], char ip[]);

#endif
