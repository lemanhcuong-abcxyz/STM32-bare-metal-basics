
#include "gps.h"
#include <string.h>

#define NMEA_SIZE 128

static char nmea[NMEA_SIZE];
static uint32_t nmea_index = 0;
static GPS_Data gps_data = {0, 0, 0};

static int HexValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int CheckChecksum(char *line)
{
    if (line[0] != '$') return 0;

    char *star = strchr(line, '*');

    if (star == 0 || star[1] == '\0' || star[2] == '\0')
        return 0;

    int high = HexValue(star[1]);
    int low = HexValue(star[2]);

    if (high < 0 || low < 0) return 0;

    uint8_t checksum = 0;

    for (char *p = line + 1; p < star; p++)
        checksum ^= (uint8_t)*p;

    if (checksum != (uint8_t)((high << 4) | low))
        return 0;

    *star = '\0';
    return 1;
}

static int ParseDecimal6(const char *s, uint32_t *result)
{
    uint32_t whole = 0;
    uint32_t fraction = 0;
    uint32_t scale = 100000U;
    int digits = 0;

    if (*s == '\0') return 0;

    while (*s >= '0' && *s <= '9') {
        whole = whole * 10U + (uint32_t)(*s - '0');
        s++;
        digits++;
    }

    if (digits == 0) return 0;

    if (*s == '.') {
        s++;

        while (*s >= '0' && *s <= '9') {
            if (scale > 0) {
                fraction += (uint32_t)(*s - '0') * scale;
                scale /= 10U;
            }
            s++;
        }
    }

    if (*s != '\0') return 0;

    *result = whole * 1000000U + fraction;
    return 1;
}

static int ParseCoordinate(
    const char *field,
    const char *hemisphere,
    int32_t *coordinate)
{
    uint32_t raw;

    if (!ParseDecimal6(field, &raw)) return 0;

    uint32_t degrees = raw / 100000000U;
    uint32_t minutes = raw - degrees * 100000000U;

    if (minutes >= 60000000U) return 0;
    if (hemisphere[0] == '\0' || hemisphere[1] != '\0')
        return 0;

    uint32_t decimal_degrees =
        degrees * 1000000U + minutes / 60U;

    if (hemisphere[0] == 'S' || hemisphere[0] == 'W') {
        *coordinate = -(int32_t)decimal_degrees;
    } else if (hemisphere[0] == 'N' || hemisphere[0] == 'E') {
        *coordinate = (int32_t)decimal_degrees;
    } else {
        return 0;
    }

    return 1;
}

static void ParseRMC(char *line)
{
    if (!CheckChecksum(line)) return;

    char *fields[12];
    uint32_t count = 1;
    fields[0] = line;

    for (char *p = line; *p && count < 12; p++) {
        if (*p == ',') {
            *p = '\0';
            fields[count++] = p + 1;
        }
    }

    if (count < 7) return;

    if (strcmp(fields[0], "$GNRMC") != 0 &&
        strcmp(fields[0], "$GPRMC") != 0)
        return;

    if (fields[2][0] != 'A' || fields[2][1] != '\0') {
        gps_data.fix = 0;
        return;
    }

    int32_t latitude;
    int32_t longitude;

    if (!ParseCoordinate(fields[3], fields[4], &latitude))
        return;

    if (!ParseCoordinate(fields[5], fields[6], &longitude))
        return;

    if (latitude < -90000000 || latitude > 90000000)
        return;

    if (longitude < -180000000 || longitude > 180000000)
        return;

    gps_data.latitude = latitude;
    gps_data.longitude = longitude;
    gps_data.fix = 1;
}

void GPS_Init(void)
{
    nmea_index = 0;
    gps_data.latitude = 0;
    gps_data.longitude = 0;
    gps_data.fix = 0;
}

void GPS_ProcessByte(char c)
{
    if (c == '$') {
        nmea_index = 0;
        nmea[nmea_index++] = c;
        return;
    }

    if (nmea_index == 0) return;

    if (c == '\n' || c == '\r') {
        if (nmea_index > 1) {
            nmea[nmea_index] = '\0';
            ParseRMC(nmea);
        }

        nmea_index = 0;
        return;
    }

    if (nmea_index < NMEA_SIZE - 1) {
        nmea[nmea_index++] = c;
    } else {
        nmea_index = 0;
    }
}

GPS_Data GPS_GetData(void)
{
    return gps_data;
}
