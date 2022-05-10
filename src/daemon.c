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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void signal_handler(int sig) { (void)sig; }

// Implements the double-fork technique to create daemon
// Returns 0 for the daemon and the pid for the parent, or -1 on error
int violet_fork_daemon() {
	int fd[2];
	if (pipe(fd) != 0)
		return -1;

	int pipe_out = fd[0];
	int pipe_in = fd[1];

	// First fork
	pid_t pid = fork();
	if (pid < 0)
		return -1;

	if (pid > 0) { // if parent
		close(pipe_in);

		// Get pid from pipe
		char buf[32];
		int len = read(pipe_out, buf, 32);
		close(pipe_out);
		if (len <= 0)
			return -1;

		buf[len] = '\0';
		if (sscanf(buf, "%d\n", &pid) != 1)
			return -1;

		return pid;
	}

	close(pipe_out);

	// Set child as session leader
	if (setsid() < 0) {
		close(pipe_in);
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, signal_handler);
	signal(SIGHUP, signal_handler);

	// Second fork
	pid = fork();
	if (pid < 0) {
		close(pipe_in);
		exit(EXIT_FAILURE);
	}

	if (pid > 0) { // if parent
		close(pipe_in);
		exit(EXIT_SUCCESS);
	}

	pid = getpid();

	// Put pid to pipe
	char buf[32];
	int len = snprintf(buf, 32, "%d\n", pid);
	if (len <= 0 || write(pipe_in, buf, len) <= 0) {
		close(pipe_in);
		exit(EXIT_FAILURE);
	}

	close(pipe_in);
	return 0;
}
