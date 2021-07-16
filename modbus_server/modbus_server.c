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

/*
 * The file is strongly based upon libmodbus/tests/random-test-server.c of libmodbus library
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <getopt.h>

#include <modbus.h>
#include <errno.h>
#include <signal.h>

#include "mbu-common.h"

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define NB_CONNECTION    10

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;

static int server_socket = -1;

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

const char DebugOpt[]   = "debug";
const char TcpOptVal[]  = "tcp";
const char RtuOptVal[]  = "rtu";
const char DiscreteInputsNo[] = "di";
const char CoilsNo[] = "co";
const char InputRegistersNo[] = "ir";
const char HoldingRegistersNo[] = "hr";

void printUsage(const char progName[]) {
    printf("%s [--%s] -m{tcp|rtu}\n\t" \
           "[-a<slave-addr=1>] --%s<discrete-inputs-no>=100 --%s<coils-no>=100 --%s<input-registers-no>=100 --%s<holding-registers-no>=100\n\t" \
           "[{rtu-params|tcp-params}]\n", progName, DebugOpt, DiscreteInputsNo, CoilsNo, InputRegistersNo, HoldingRegistersNo);           
    printf("rtu-params:\n" \
           "\tb<baud-rate>=9600\n" \
           "\td{7|8}<data-bits>=8\n" \
           "\ts{1|2}<stop-bits>=1\n" \
           "\tp{none|even|odd}=even\n");
    printf("tcp-params:\n" \
           "\tp<port>=502\n");
}

int main(int argc, char **argv)
{
    int c;
    int ok;
    int rc;

    BackendParams *backend = 0;
    int slaveAddr = 1;
    int debug = 0;
    int diNo = 100;
    int coilsNo = 100;
    int irNo = 100;
    int hrNo = 100;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {DebugOpt,  no_argument, 0,  0},
            {DiscreteInputsNo, required_argument, 0, 0},
            {CoilsNo, required_argument, 0, 0},
            {InputRegistersNo, required_argument, 0, 0},
            {HoldingRegistersNo, required_argument, 0, 0},
            {0, 0,  0,  0}
        };

        c = getopt_long(argc, argv, "a:b:d:m:s:p:",
                        long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            if (0 == strcmp(long_options[option_index].name, DebugOpt)) {
                debug = 1;
            }
            else if (0 == strcmp(long_options[option_index].name, DiscreteInputsNo)) {
                diNo = getInt(optarg, &ok);
                if (0 == ok || diNo < 0) {
                    printf("Cannot set discrete inputs no from %s", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == strcmp(long_options[option_index].name, CoilsNo)) {
                coilsNo = getInt(optarg, &ok);
                if (0 == ok || coilsNo < 0) {                
                    printf("Cannot set discrete coils no from %s", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == strcmp(long_options[option_index].name, InputRegistersNo)) {
                irNo = getInt(optarg, &ok);
                if (0 == ok || irNo < 0) {
                    printf("Cannot set input registers no from %s", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == strcmp(long_options[option_index].name, HoldingRegistersNo)) {
                hrNo = getInt(optarg, &ok);
                if (0 == ok || hrNo < 0) {
                    printf("Cannot set holding registers no from %s\n", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }

            break;

        case 'a': {
            slaveAddr = getInt(optarg, &ok);
            if (0 == ok) {
                printf("Slave address (%s) is not integer!\n\n", optarg);
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
            break;

        case 'm':
            if (0 == strcmp(optarg, TcpOptVal)) {
                backend = createTcpBackend();
            }
            else if (0 == strcmp(optarg, RtuOptVal))
                backend = createRtuBackend();
            else {
                printf("Unrecognized connection type %s\n\n", optarg);
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;

            //tcp/rtu params
        case 'p':
        case 'b':
        case 'd':
        case 's':
            if (0 == backend) {
                printf("Connection type (-m switch) has to be set before its params are provided!\n");
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
            else {
                if (0 == backend->setParam(backend, c, optarg)) {
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case '?':
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (0 == backend) {
        printf("No connection type was specified!\n");
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (1 == argc - optind) {
        if (Rtu == backend->type) {
            RtuBackend *rtuP = (RtuBackend*)backend;
            strcpy(rtuP->devName, argv[optind]);
        }
        else if (Tcp == backend->type) {
            TcpBackend *tcpP = (TcpBackend*)backend;
            strcpy(tcpP->ip, argv[optind]);
        }
    }
    else {
        printf("Expecting only serialport|ip as free parameter!\n");
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    //prepare mapping
    mb_mapping = modbus_mapping_new(coilsNo, diNo, hrNo, irNo);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("Ranges: \n \tCoils: 0-0x%04x\n\tDigital inputs: 0-0x%04x\n\tHolding registers: 0-0x%04x\n\tInput registers: 0-0x%04x\n",
               coilsNo, diNo, hrNo, irNo);

    if (0 == backend) {
        printf("No backend has been specified!\n");
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    ctx = backend->createCtxt(backend);
    modbus_set_debug(ctx, debug);
    modbus_set_slave(ctx, slaveAddr);

    if (Rtu == backend->type) {

        for(;;) {

            if (0 == backend->listenForConnection(backend, ctx)) {
                break;
            }

            for (;;) {
                uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];

                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                    /* rc is the query size */
                    modbus_reply(ctx, query, rc, mb_mapping);
                } else if (rc == -1) {
                    /* Connection closed by the client or error */
                    break;
                }
            }
            printf("Client disconnected: %s\n", modbus_strerror(errno));

            backend->closeConnection(backend);
        }

    }
    else if (Tcp == backend->type) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int master_socket;
        fd_set refset;
        fd_set rdset;
        /* Maximum file descriptor number */
        int fdmax;

        server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
        if (server_socket == -1) {
            fprintf(stderr, "Unable to listen TCP connection\n");
            modbus_free(ctx);
            return -1;
        }

        signal(SIGINT, close_sigint);

        /* Clear the reference set of socket */
        FD_ZERO(&refset);
        /* Add the server socket */
        FD_SET(server_socket, &refset);

        /* Keep track of the max file descriptor */
        fdmax = server_socket;

        for (;;) {
            rdset = refset;
            if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
                perror("Server select() failure.");
                close_sigint(1);
            }

            /* Run through the existing connections looking for data to be
             * read */
            for (master_socket = 0; master_socket <= fdmax; master_socket++) {

                if (!FD_ISSET(master_socket, &rdset)) {
                    continue;
                }

                if (master_socket == server_socket) {
                    /* A client is asking a new connection */
                    socklen_t addrlen;
                    struct sockaddr_in clientaddr;
                    int newfd;

                    /* Handle new connections */
                    addrlen = sizeof(clientaddr);
                    memset(&clientaddr, 0, sizeof(clientaddr));
                    newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
                    if (newfd == -1) {
                        perror("Server accept() error");
                    } else {
                        FD_SET(newfd, &refset);

                        if (newfd > fdmax) {
                            /* Keep track of the maximum */
                            fdmax = newfd;
                        }
                        printf("New connection from %s:%d on socket %d\n",
                            inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                    }
                } else {
                    modbus_set_socket(ctx, master_socket);
                    rc = modbus_receive(ctx, query);
                    if (rc > 0) {
                        modbus_reply(ctx, query, rc, mb_mapping);
                    } else if (rc == -1) {
                        /* This example server in ended on connection closing or
                         * any errors. */
                        printf("Connection closed on socket %d\n", master_socket);
                        close(master_socket);

                        /* Remove from reference set */
                        FD_CLR(master_socket, &refset);

                        if (master_socket == fdmax) {
                            fdmax--;
                        }
                    }
                }
            }
        }
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    backend->del(backend);

    return 0;
}
