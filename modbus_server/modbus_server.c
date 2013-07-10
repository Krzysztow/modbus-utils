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

#include "mbu-common.h"

const char DebugOpt[]   = "debug";
const char TcpOptVal[]  = "tcp";
const char RtuOptVal[]  = "rtu";
const char DiscreteInputsNo[] = "di";
const char CoilsNo[] = "co";
const char InputRegistersNo[] = "ir";
const char HoldingRegistersNo[] = "hr";

void printUsage(const char progName[]) {
    printf("%s [--%s] -m{tcp|rtu}\n\t" \
           "--%s<discrete-inputs-no>=100 --%s<coils-no>=100 --%s<input-registers-no>=100 --%s<holding-registers-no>=100\n\t" \
           "[{rtu-params|tcp-params}]", progName, DebugOpt, DiscreteInputsNo, CoilsNo, InputRegistersNo, HoldingRegistersNo);
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

    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;

    BackendParams *backend = 0;
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

        c = getopt_long(argc, argv, "b:d:m:s:p:",
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
                if (0 == ok  || coilsNo < 0) {
                    printf("Cannot set discrete coils no from %s", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == strcmp(long_options[option_index].name, InputRegistersNo)) {
                irNo = getInt(optarg, &ok);
                if (0 == ok  || irNo < 0) {
                    printf("Cannot set input registers no from %s", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == strcmp(long_options[option_index].name, HoldingRegistersNo)) {
                hrNo = getInt(optarg, &ok);
                if (0 == ok || hrNo) {
                    printf("Cannot set holding registers no from %s", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }

            break;

        case 'm':
            if (0 == strcmp(optarg, TcpOptVal)) {
                backend = initTcpParams((TcpBackend*)malloc(sizeof(TcpBackend)));
            }
            else if (0 == strcmp(optarg, RtuOptVal))
                backend = initRtuParams((RtuBackend*)malloc(sizeof(RtuBackend)));
            else {
                printf("Unrecognized connection type %s\n\n", optarg);
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;

            //tcp/rtu params
        case 'p': {
            if (Tcp == backend->type) {
                TcpBackend *tcpP = (TcpBackend*)backend;
                tcpP->port = getInt(optarg, &ok);
                if (0 == ok) {
                    printf("Port parameter %s is not integer!\n\n", optarg);
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (Rtu == backend->type) {
                if (0 == setRtuParam((RtuBackend*)backend, c, optarg))
                    exit(EXIT_FAILURE);
            }
            else {
                printf("Port parameter %s specified for non TCP or RTU conn type!\n\n", optarg);
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
            break;
            //rtu params
        case 'b':
        case 'd':
        case 's':
            if (Rtu == backend->type) {
                if (0 == setRtuParam((RtuBackend*)backend, c, optarg)) {
                    exit(EXIT_FAILURE);
                }
            }
            else {
                printf("Port parameter %s specified for non RTU conn type!\n\n", optarg);
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case '?':
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
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

    if (Tcp == backend->type) {
        TcpBackend *tcp = (TcpBackend*)backend;
        ctx = modbus_new_tcp(tcp->ip, tcp->port);
    }
    else if (Rtu == backend->type) {
        RtuBackend *rtu = (RtuBackend*)backend;
        ctx = modbus_new_rtu(rtu->devName, rtu->baud, rtu->parity, rtu->dataBits, rtu->stopBits);
    }
    modbus_set_debug(ctx, debug);

    uint8_t query[(Tcp == backend->type) ? MODBUS_TCP_MAX_ADU_LENGTH : MODBUS_RTU_MAX_ADU_LENGTH];

    for(;;) {

        if (Tcp == backend->type) {
            s = modbus_tcp_listen(ctx, 1);
            if (-1 == s) {
                printf("Listen returned %d (%s)\n", s, modbus_strerror(errno));
                break;
            }
            modbus_tcp_accept(ctx, &s);
        }

        for (;;) {
            int rc;

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

        if (Tcp == backend->type) {
            if (s != -1) {
                close(s);
                s = 0;
            }
        }
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
