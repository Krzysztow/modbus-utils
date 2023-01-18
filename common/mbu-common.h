/*
*  MIT License

*  Copyright (c) 2013  Krzysztow

*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:

*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.

*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*/

#ifndef MBU_COMMON_H
#define MBU_COMMON_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>

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

    void (*del)(void *backend);

    //common client/server functions
    int (*setParam)(void *backend, char c, char *value);
    modbus_t *(*createCtxt)(void *backend);

    //server functions
    int (*listenForConnection)(void *backend, modbus_t *ctx);
    void (*closeConnection)(void *backend);
} BackendParams;

typedef struct {
    BackendParams base;
    char devName[32];
    int baud;
    int dataBits;
    int stopBits;
    char parity;
} RtuBackend;

int setRtuParam(void *backend, char c, char *value) {
    RtuBackend *rtuParams = (RtuBackend*)backend;
    int ok = 1;

    switch (c) {
    case 'b': {
        rtuParams->baud = getInt(value, &ok);
        if (0 == ok) {
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
    case 'p': {
        if (0 == strcmp(value, "none")) {
            rtuParams->parity = 'N';
        }
        else if (0 == strcmp(value, "even")) {
            rtuParams->parity = 'E';
        }
        else if (0 == strcmp(value, "odd")) {
            rtuParams->parity = 'O';
        }
        else {
            printf("Unrecognized parity (%s)", value);
            ok = 0;
        }
    }
        break;
    default:
        printf("Unknown rtu param (%c: %s)\n\n", c, value);
        ok = 0;
    }

    return ok;
}

modbus_t *createRtuCtxt(void *backend) {
    RtuBackend *rtu = (RtuBackend*)backend;
    modbus_t *ctx = modbus_new_rtu(rtu->devName, rtu->baud, rtu->parity, rtu->dataBits, rtu->stopBits);

    return ctx;
}

void delRtu(void *backend) {
    RtuBackend *rtu = (RtuBackend*)backend;
    free(rtu);
}

int listenForRtuConnection(void *backend, modbus_t *ctx) {
    (void)backend;
    (void)ctx;

    printf("Connecting...\r\n");
    return (0 == modbus_connect(ctx));
}
void closeRtuConnection(void *backend) {(void)backend;}

BackendParams *createRtuBackend() {
    RtuBackend *rtu = (RtuBackend*)malloc(sizeof(RtuBackend));
    rtu->base.type = Rtu;
    rtu->base.setParam = &setRtuParam;
    rtu->base.createCtxt = &createRtuCtxt;
    rtu->base.listenForConnection = &listenForRtuConnection;
    rtu->base.closeConnection = &closeRtuConnection;
    rtu->base.del = &delRtu;

    strcpy(rtu->devName, "");
    rtu->baud = 9600;
    rtu->dataBits = 8;
    rtu->stopBits = 1;
    rtu->parity = 'E';

    return (BackendParams*)rtu;
}

typedef struct {
    BackendParams base;
    char ip[32];
    int port;

    int clientSocket;
} TcpBackend;

int setTcpParam(void* backend, char c, char *value) {
    TcpBackend *tcp = (TcpBackend*)backend;

    int ok = 1;

    switch (c) {

    case 'p': {
        tcp->port = getInt(optarg, &ok);
        if (0 == ok) {
            printf("Port parameter %s is not integer!\n\n", optarg);
        }
    }
        break;

    default:
        printf("Unknown tcp param (%c: %s)\n\n", c, value);
        ok = 0;
    }

    return ok;
}

modbus_t *createTcpCtxt(void *backend) {
    TcpBackend *tcp = (TcpBackend*)backend;
    modbus_t *ctx = modbus_new_tcp(tcp->ip, tcp->port);

    return ctx;
}

void delTcp(void *backend) {
    TcpBackend *tcp = (TcpBackend*)backend;
    free(tcp);
}

int listenForTcpConnection(void *backend, modbus_t *ctx) {
    TcpBackend *tcp = (TcpBackend*)backend;
    tcp->clientSocket = modbus_tcp_listen(ctx, 1);
    if (-1 == tcp->clientSocket) {
        printf("Listen returned %d (%s)\n", tcp->clientSocket, modbus_strerror(errno));
        return 0;
    }
    modbus_tcp_accept(ctx, &(tcp->clientSocket));
    return 1;
}

void closeTcpConnection(void *backend) {
    TcpBackend *tcp = (TcpBackend*)backend;
    if (tcp->clientSocket != -1) {
        close(tcp->clientSocket);
        tcp->clientSocket = -1;
    }
}

BackendParams *createTcpBackend() {
    TcpBackend *tcp = (TcpBackend*)malloc(sizeof(TcpBackend));
    tcp->clientSocket =  -1;
    tcp->base.setParam = &setTcpParam;
    tcp->base.createCtxt = &createTcpCtxt;
    tcp->base.del = &delTcp;
    tcp->base.listenForConnection = &listenForTcpConnection;
    tcp->base.closeConnection = &closeTcpConnection;

    tcp->base.type = Tcp;
    strcpy(tcp->ip, "0.0.0.0");
    tcp->port = 502;

    return (BackendParams*)tcp;
}

#endif //MBU_COMMON_H
