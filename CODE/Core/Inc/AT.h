#ifndef UNTITLED_AT_H
#define UNTITLED_AT_H

#endif //UNTITLED_AT_H

int TransmitCommand(const char* command, const char* response);
int TransmitCommandOK(const char* command);

void ATInit();

void WifiInit();

void MQTTInit();