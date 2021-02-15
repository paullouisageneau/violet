
#include <juice/juice.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct violet_options {
	juice_log_level_t log_level;
	juice_server_config_t config;
	bool stun_only;
} violet_options_t;

void violet_options_init(violet_options_t *vopts);
void violet_options_destroy(violet_options_t *vopts);
int violet_options_from_file(FILE *file, violet_options_t *vopts);
int violet_options_from_arg(int argc, char *argv[], violet_options_t *vopts);
