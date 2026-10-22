#ifndef GPS_H
#define GPS_H

#include <stdint.h>

typedef struct {
    int32_t latitude;
    int32_t longitude;
    uint8_t fix;
} GPS_Data;

void GPS_Init(void);
void GPS_ProcessByte(char c);
GPS_Data GPS_GetData(void);

#endif
