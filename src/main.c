/**
 * Copyright (c) 2021 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "options.h"

#include <juice/juice.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void signal_handler(int sig) {
	(void)sig;
}

int main(int argc, char *argv[]) {
	signal(SIGINT, signal_handler);

	violet_options_t vopts;
	violet_options_init(&vopts);
	if(violet_options_from_arg(argc, argv, &vopts) < 0) {
		violet_options_destroy(&vopts);
		return EXIT_FAILURE;
	}

	juice_set_log_level(vopts.log_level);

	juice_server_config_t config;
	memset(&config, 0, sizeof(config));
	config.credentials = vopts.credentials;
	config.credentials_count = vopts.credentials_count;
	config.max_allocations = vopts.max_allocations;

	juice_server_t *server = juice_server_create(vopts.port, &config);
	if (!server) {
		fprintf(stderr, "Server initialization failed.\n");
		violet_options_destroy(&vopts);
		return EXIT_FAILURE;
	}

	pause();

	juice_server_destroy(server);
	violet_options_destroy(&vopts);
	return EXIT_SUCCESS;
}
