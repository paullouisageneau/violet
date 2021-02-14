
#include "options.h"

#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>

#define VIOLET_OPTIONS_COUNT 6

void violet_options_init(violet_options_t *vopts) {
	memset(vopts, 0, sizeof(*vopts));
	vopts->log_level = JUICE_LOG_LEVEL_NONE;
	vopts->port = 3478;
}

void violet_options_destroy(violet_options_t *vopts) {
	for(int i = 0; i < vopts->credentials_count; ++i) {
		juice_server_credentials_t *credentials = vopts->credentials + i;
		free((char*)credentials->username);
		free((char*)credentials->password);
	}

	free(vopts->credentials);
	vopts->credentials = NULL;
	vopts->credentials_count = 0;
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

	vopts->port = (uint16_t)p;
	return 0;
}

static int on_credentials(violet_options_t *vopts, const char *arg) {
	const char *s = strchr(arg, ':');
	if (!s)
		return -1;

	const char *username = arg;
	const char *password = s + 1;
	int username_len = s - arg;
	int password_len = strlen(s);

	char *username_copy = malloc(username_len+1);
	char *password_copy = malloc(password_len+1);
	vopts->credentials = realloc(vopts->credentials, (vopts->credentials_count+1) * sizeof(juice_server_credentials_t));
	if(!username_copy || !password_copy || !vopts->credentials) {
		fprintf(stderr, "Memory allocation for credentials failed\n");
		free(username_copy);
		free(password_copy);
		free(vopts->credentials);
		vopts->credentials_count = 0;
		return -1;
	}

	memcpy(username_copy, username, username_len);
	username_copy[username_len] = '\0';
	memcpy(password_copy, password, password_len);
	password_copy[password_len] = '\0';

	juice_server_credentials_t *credentials = vopts->credentials + vopts->credentials_count;
	memset(credentials, 0, sizeof(*credentials));
	credentials->username = username_copy;
	credentials->password = password_copy;
	++vopts->credentials_count;

	return 0;
}

static int on_quota(violet_options_t *vopts, const char *arg) {
	int n = atoi(arg);
	if (n <= 0)
		return -1;

	if(vopts->credentials_count == 0)
		return -1;

	vopts->credentials[vopts->credentials_count-1].allocations_quota = n;
	return 0;
}

static int on_max(violet_options_t *vopts, const char *arg) {
	int n = atoi(arg);
	if (n <= 0)
		return -1;

	vopts->max_allocations = n;
	return 0;
}

typedef struct violet_option_entry {
	char short_name;
	const char *long_name;
	const char *arg_name;
	const char *description;
	int (*callback)(violet_options_t *violet_options, const char *value);
} violet_option_entry_t;

static const violet_option_entry_t violet_options_map[VIOLET_OPTIONS_COUNT] = {
    {'h', "help", NULL, "Display this message", on_help},
    {'d', "debug", NULL, "Enable debug mode (default off)", on_debug},
    {'p', "port", "PORT", "UDP port to listen on (default 3478)", on_port},
    {'c', "credentials", "USER:PASSWORD", "Add TURN credentials", on_credentials},
    {'q', "quota", "ALLOCATIONS", "Set an allocations quota for the last credentials", on_quota},
	{'m', "max", "ALLOCATIONS", "Set the maximum number of allocations", on_max}
};

static const char *program_name = NULL;

static int on_help(violet_options_t *vopts, const char *arg) {
	(void)vopts;
	(void)arg;
	printf("Usage: %s [options]\n\n", program_name ? program_name : "violet");

	for (int i = 0; i < VIOLET_OPTIONS_COUNT; ++i) {
		const violet_option_entry_t *entry = violet_options_map + i;
		printf("  %c%c%s%s%c%s\t%s\n", entry->short_name ? '-' : ' ',
		       entry->short_name ? entry->short_name : ' ', entry->long_name ? ", --" : " ",
		       entry->long_name ? entry->long_name : "",
		       entry->long_name && entry->arg_name ? '=' : ' ',
		       entry->arg_name ? entry->arg_name : "\t",
		       entry->description ? entry->description : "");
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
