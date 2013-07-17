/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "modbus.h"
#include "modbus-private.h"
#include "modbus-tcp-private.h"

#include <pthread.h>

modbus_t *modbus_clone_tcp(modbus_t* ctx)
{	
	modbus_t* new_ctx = (modbus_t *) malloc(sizeof(modbus_t));
	memcpy(new_ctx, ctx, sizeof(modbus_t));
	new_ctx->backend_data = (modbus_tcp_t *) malloc(sizeof(modbus_tcp_t));
	memcpy(new_ctx->backend_data, ctx->backend_data, sizeof(modbus_tcp_t));

	//modbus_tcp_t *mb_tcp = (modbus_tcp_t*)ctx->backend_data;
	printf("Socket copied %d, should be %d\n", new_ctx->s, ctx->s);
	ctx->s = 0;

	return new_ctx;
}


typedef struct {
	modbus_t *ctxt;
	modbus_mapping_t *mm;
	int id;
} SA;

void *serveClient(void *threadarg)
{
	SA *a;
	a = (SA *) threadarg;
	modbus_t *ctx = a->ctxt;
	modbus_mapping_t *mb_mapping = a->mm;	
	int id = a->id;
	printf("(%d) Serving... %p\n", id, ctx);
	for (;;) {
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
		int rc;
		printf("(%d) receiving...\n", id);
		rc = modbus_receive(ctx, query);
		if (rc > 0) {
		    /* rc is the query size */
		    modbus_reply(ctx, query, rc, mb_mapping);
		} else if (rc == -1) {
			printf("(%d) Disconnected %s\n", id, modbus_strerror(errno));
		    /* Connection closed by the client or error */
		    break;
		}
	}

	//clean-up
	free(ctx);
	free(a);
}

int main(void)
{
	int s = -1;
	modbus_t *ctx;
	modbus_mapping_t *mb_mapping;
	int rc = -1;
	pthread_t tId;

	pthread_attr_t attr;
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ctx = modbus_new_tcp("127.0.0.1", 1502);
	modbus_set_debug(ctx, TRUE);

	mb_mapping = modbus_mapping_new(500, 500, 500, 500);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
			modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	s = modbus_tcp_listen(ctx, 1);

	int id = 0;
	for (;;) {
		id++;
		printf("Waiting for another connection...\n\n");
		modbus_tcp_accept(ctx, &s);

		modbus_t *new_ctx = modbus_clone_tcp(ctx);

		SA *sa = (SA*)malloc(sizeof(SA));
		sa->ctxt = new_ctx;
		sa->mm = mb_mapping;
		sa->id = id;

		rc = pthread_create(&tId, &attr, serveClient, (void *)sa); 
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	if (s != -1) {
		close(s);
	}	
	modbus_mapping_free(mb_mapping);
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}
