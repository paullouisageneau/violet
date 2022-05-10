/*
 * Copyright (c) 2021 Paul-Louis Ageneau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "daemon.h"
#include "options.h"
#include "utils.h"

#include <juice/juice.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static FILE *log_file = NULL;

static void signal_handler(int sig) { (void)sig; }

static void log_handler(juice_log_level_t level, const char *message) {
	FILE *file = log_file ? log_file : stdout;
	time_t t = time(NULL);
	struct tm lt;
	char buffer[32];
	if (!localtime_r(&t, &lt) || strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", &lt) == 0)
		buffer[0] = '\0';
	fprintf(file, "%s %-7s %s\n", buffer, log_level_to_string(level), message);
	fflush(file);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, signal_handler);

	violet_options_t vopts;
	violet_options_init(&vopts);

	if (violet_options_from_arg(argc, argv, &vopts) < 0) {
		goto error;
	}

	if (vopts.daemon) {
		int pid = violet_fork_daemon();
		if (pid < 0) {
			fprintf(stderr, "Fork as daemon failed\n");
			goto error;
		}

		if (pid > 0) { // parent
			printf("Daemon forked as pid %d\n", pid);
			violet_options_destroy(&vopts);
			return EXIT_SUCCESS;
		}
	}

	if (vopts.log_filename) {
		log_file = fopen(vopts.log_filename, "a");
		if (!log_file) {
			fprintf(stderr, "Log file opening failed\n");
			goto error;
		}
	}

	juice_set_log_handler(log_handler);
	juice_set_log_level(vopts.log_level);

	juice_server_t *server = juice_server_create(&vopts.config);
	if (!server) {
		fprintf(stderr, "Server initialization failed\n");
		goto error;
	}

	pause();

	juice_server_destroy(server);

	if (log_file)
		fclose(log_file);

	violet_options_destroy(&vopts);
	return EXIT_SUCCESS;

error:
	if (log_file)
		fclose(log_file);

	violet_options_destroy(&vopts);
	return EXIT_FAILURE;
}

