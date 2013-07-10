#ifndef MBU_COMMON_H
#define MBU_COMMON_H

#include <string.h>

typedef enum {
    None,
    Tcp,
    Rtu
} ConnType;

int getInt(const char str[], int *ok) {
    int value;
    int ret = sscanf(str, "0x%x", &value);
    if (0 >= ret) {//couldn't convert from hex, try dec
        ret = sscanf(str, "%d", &value);
    }

    if (0 != ok) {
        *ok = (0 < ret);
    }

    return value;
}

typedef struct {
    ConnType type;
} BackendParams;

typedef struct {
    ConnType type;
    char devName[32];
    int baud;
    int dataBits;
    int stopBits;
    char parity;
} RtuBackend;

BackendParams *initRtuParams(RtuBackend *rtuParams) {
    rtuParams->type = Rtu;
    strcpy(rtuParams->devName, "");
    rtuParams->baud = 9600;
    rtuParams->dataBits = 8;
    rtuParams->stopBits = 1;
    rtuParams->parity = 'e';

    return (BackendParams*)rtuParams;
}

int setRtuParam(RtuBackend* rtuParams, char c, char *value) {
    int ok = 1;

    switch (c) {
    case 'b': {
        rtuParams->baud = getInt(value, &ok);
        if (0 != ok) {
            printf("Baudrate is invalid %s", value);
            ok = 0;
        }
    }
        break;
    case 'd': {
        int db = getInt(value, &ok);
        if (0 == ok || (7 != db && 8 != db)) {
            printf("Data bits incorrect (%s)", value);
            ok = 0;
        }
        else
            rtuParams->dataBits = db;
    }
        break;
    case 's': {
        int sb = getInt(value, &ok);
        if (0 == ok || (1 != sb && 2 != sb)) {
            printf("Stop bits incorrect (%s)", value);
            ok = 0;
        }
        else
            rtuParams->stopBits = sb;
    }
        break;
    default:
        printf("Unknown rtu param (%c: %s)\n\n", c, value);
        ok = 0;
    }

    return ok;
}

typedef struct {
    ConnType type;
    char ip[32];
    int port;
} TcpBackend;

BackendParams *initTcpParams(TcpBackend *tcpParams) {
    tcpParams->type = Tcp;
    strcpy(tcpParams->ip, "0.0.0.0");
    tcpParams->port = 502;

    return (BackendParams*)tcpParams;
}

#endif //MBU_COMMON_H
