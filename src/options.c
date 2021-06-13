
#include "options.h"

#include <ctype.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static char *alloc_string_copy(const char *src, size_t max) {
	if (!src)
		return NULL;

	size_t len = strlen(src);
	if (len > max)
		len = max;

	char *copy = malloc(len + 1);
	if (!copy)
		return NULL;

	memcpy(copy, src, len);
	copy[len] = '\0';
	return copy;
}

void violet_options_init(violet_options_t *vopts) {
	memset(vopts, 0, sizeof(*vopts));
	vopts->log_level = JUICE_LOG_LEVEL_FATAL;
	vopts->stun_only = false;
	vopts->config.port = 3478;
}

void violet_options_destroy(violet_options_t *vopts) {
	for (int i = 0; i < vopts->config.credentials_count; ++i) {
		juice_server_credentials_t *credentials = vopts->config.credentials + i;
		free((char *)credentials->username);
		free((char *)credentials->password);
	}

	free((char *)vopts->config.bind_address);
	vopts->config.bind_address = NULL;

	free((char *)vopts->config.external_address);
	vopts->config.external_address = NULL;

	free(vopts->config.credentials);
	vopts->config.credentials = NULL;
	vopts->config.credentials_count = 0;
}

static int on_help(violet_options_t *vopts, const char *arg);

static int on_debug(violet_options_t *vopts, const char *arg) {
	(void)arg;
	vopts->log_level = JUICE_LOG_LEVEL_VERBOSE;
	return 0;
}

static int on_port(violet_options_t *vopts, const char *arg) {
	int p = atoi(arg);
	if (p <= 0)
		return -1;

	vopts->config.port = (uint16_t)p;
	return 0;
}

static int on_range(violet_options_t *vopts, const char *arg) {
	uint16_t range_begin = 0;
	uint16_t range_end = 0;
	if(sscanf(arg, "%hu:%hu", &range_begin, &range_end) != 2)
		return -1;

	if(range_end < range_begin)
		return -1;

	vopts->config.relay_port_range_begin = range_begin;
	vopts->config.relay_port_range_end = range_end;
	return 0;
}

static int on_bind(violet_options_t *vopts, const char *arg) {
	if (*arg == '\0')
		return -1;

	vopts->config.bind_address = alloc_string_copy(arg, SIZE_MAX);
	return 0;
}

static int on_external(violet_options_t *vopts, const char *arg) {
	if (*arg == '\0')
		return -1;

	vopts->config.external_address = alloc_string_copy(arg, SIZE_MAX);
	return 0;
}

static int on_credentials(violet_options_t *vopts, const char *arg) {
	const char *s = strchr(arg, ':');
	if (!s)
		return -1;

	if(vopts->stun_only)
		return 0;

	char *username = alloc_string_copy(arg, s - arg);
	char *password = alloc_string_copy(s + 1, SIZE_MAX);
	vopts->config.credentials =
	    realloc(vopts->config.credentials,
	            (vopts->config.credentials_count + 1) * sizeof(juice_server_credentials_t));
	if (!username || !password || !vopts->config.credentials) {
		fprintf(stderr, "Memory allocation for credentials failed\n");
		free(username);
		free(password);
		free(vopts->config.credentials);
		vopts->config.credentials_count = 0;
		return -1;
	}

	juice_server_credentials_t *credentials =
	    vopts->config.credentials + vopts->config.credentials_count;
	memset(credentials, 0, sizeof(*credentials));
	credentials->username = username;
	credentials->password = password;
	++vopts->config.credentials_count;

	return 0;
}

static int on_quota(violet_options_t *vopts, const char *arg) {
	int n = atoi(arg);
	if (n <= 0)
		return -1;

	if(vopts->stun_only)
		return 0;

	if (vopts->config.credentials_count == 0)
		return -1;

	vopts->config.credentials[vopts->config.credentials_count - 1].allocations_quota = n;
	return 0;
}

static int on_max(violet_options_t *vopts, const char *arg) {
	int n = atoi(arg);
	if (n <= 0)
		return -1;

	if(vopts->stun_only)
		return 0;

	vopts->config.max_allocations = n;
	return 0;
}

static int on_stun_only(violet_options_t *vopts, const char *arg) {
	(void)arg;
	vopts->stun_only = true;

	free(vopts->config.credentials);
	vopts->config.credentials = NULL;
	vopts->config.credentials_count = 0;
	vopts->config.max_allocations = 0;
	return 0;
}

typedef struct violet_option_entry {
	char short_name;
	const char *long_name;
	const char *arg_name;
	const char *description;
	int (*callback)(violet_options_t *violet_options, const char *value);
} violet_option_entry_t;

#define VIOLET_OPTIONS_COUNT 10
#define HELP_DESCRIPTION_OFFSET 24

static const violet_option_entry_t violet_options_map[VIOLET_OPTIONS_COUNT] = {
    {'h', "help", NULL, "Display this message", on_help},
    {'d', "debug", NULL, "Enable debug mode (default disabled)", on_debug},
    {'p', "port", "PORT", "UDP port to listen on (default 3478)", on_port},
    {'r', "range", "BEGIN:END", "UDP port range for relay (default automatic)", on_range},
    {'b', "bind", "ADDRESS", "Bind only on ADDRESS (default any address)", on_bind},
    {'e', "external", "ADDRESS", "Avertise relay on ADDRESS (default local address)", on_external},
    {'c', "credentials", "USER:PASS", "Add TURN credentials (may be called multiple times)", on_credentials},
    {'q', "quota", "ALLOCATIONS", "Set an allocations quota for the last credentials", on_quota},
    {'m', "max", "ALLOCATIONS", "Set the maximum number of allocations", on_max},
    {'s', "stun-only", NULL, "Disable TURN support", on_stun_only}};

static const char *program_name = NULL;

static int on_help(violet_options_t *vopts, const char *arg) {
	(void)vopts;
	(void)arg;
	printf("Usage: %s [options]\n\n", program_name ? program_name : "violet");

	for (int i = 0; i < VIOLET_OPTIONS_COUNT; ++i) {
		const violet_option_entry_t *entry = violet_options_map + i;
		int ret = printf("  %c%c%s%s%c%s", entry->short_name ? '-' : ' ',
		                 entry->short_name ? entry->short_name : ' ',
		                 entry->long_name ? ", --" : " ", entry->long_name ? entry->long_name : "",
		                 entry->long_name && entry->arg_name ? '=' : ' ',
		                 entry->arg_name ? entry->arg_name : "");

		if (entry->description) {
			int offset = ret >= 0 ? HELP_DESCRIPTION_OFFSET - ret : 0;
			while (offset-- > 0)
				printf(" ");

			printf("\t%s\n", entry->description);
		}
	}

	printf("\n");
	violet_options_destroy(vopts);
	exit(EXIT_SUCCESS);
}

int violet_options_from_file(FILE *file, violet_options_t *vopts) {
	// TODO
	(void)file;
	(void)vopts;
	return -1;
}

int violet_options_from_arg(int argc, char *argv[], violet_options_t *vopts) {
	program_name = argv[0];

	char short_options[VIOLET_OPTIONS_COUNT * 2 + 2];
	memset(short_options, 0, sizeof(short_options));

	char *so = short_options;
	*so++ = ':';
	for (int i = 0; i < VIOLET_OPTIONS_COUNT; ++i) {
		const violet_option_entry_t *entry = violet_options_map + i;
		if (entry->short_name) {
			*so++ = entry->short_name;
			if (entry->arg_name)
				*so++ = ':';
		}
	}

	struct option long_options[VIOLET_OPTIONS_COUNT + 1];
	memset(long_options, 0, sizeof(long_options));

	struct option *lo = long_options;
	for (int i = 0; i < VIOLET_OPTIONS_COUNT; ++i) {
		const violet_option_entry_t *entry = violet_options_map + i;
		if (entry->long_name) {
			lo->name = entry->long_name;
			lo->has_arg = optional_argument; // we are going to check ourselves
			++lo;
		}
	}

	opterr = false;
	while (true) {
		int index = 0;
		int c = getopt_long(argc, argv, short_options, long_options, &index);
		if (c == -1)
			break;

		if (c == 0) {
			lo = long_options + index;
			for (int i = 0; i < VIOLET_OPTIONS_COUNT; ++i) {
				const violet_option_entry_t *entry = violet_options_map + i;
				if (entry->long_name == lo->name) {
					if (entry->arg_name && !optarg) {
						fprintf(stderr, "Option --%s requires an argument.\n", entry->long_name);
						return -1;
					}
					if (!entry->arg_name && optarg) {
						fprintf(stderr, "Option --%s does not expect an argument.\n",
						        entry->long_name);
						return -1;
					}
					if (entry->callback && entry->callback(vopts, optarg) < 0) {
						fprintf(stderr, "Option --%s cannot be set%s%s.\n", entry->long_name,
						        optarg ? " to value " : "", optarg ? optarg : "");
						return -1;
					}
					break;
				}
			}
			continue;
		}

		switch (c) {
		case ':':
			fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			return -1;
			break;

		case '?':
			if (optopt == 0)
				fprintf(stderr, "Unknown option %s.\n", argv[optind - 1]);
			else
				fprintf(stderr, "Unknown option -%c.\n", optopt);

			return -1;
			break;

		default:
			for (int i = 0; i < VIOLET_OPTIONS_COUNT; ++i) {
				const violet_option_entry_t *entry = violet_options_map + i;
				if (entry->short_name == c) {
					if (entry->callback && entry->callback(vopts, optarg) < 0) {
						fprintf(stderr, "Option -%c cannot be set%s%s.\n", entry->short_name,
						        optarg ? " to value " : "", optarg ? optarg : "");
						return -1;
					}
					break;
				}
			}
			break;
		}
	}

	if (optind < argc) {
		fprintf(stderr, "Unexpected non-option argument \"%s\"\n", argv[optind]);
		return -1;
	}

	return 0;
}
