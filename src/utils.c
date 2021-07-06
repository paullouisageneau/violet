/**
 * Copyright (c) 2021 Paul-Louis Ageneau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

#include <strings.h>

const char *log_level_to_string(juice_log_level_t level) {
	switch (level) {
	case JUICE_LOG_LEVEL_NONE:
		return "NONE";
	case JUICE_LOG_LEVEL_FATAL:
		return "FATAL";
	case JUICE_LOG_LEVEL_ERROR:
		return "ERROR";
	case JUICE_LOG_LEVEL_WARN:
		return "WARN";
	case JUICE_LOG_LEVEL_INFO:
		return "INFO";
	case JUICE_LOG_LEVEL_DEBUG:
		return "DEBUG";
	default:
		return "VERBOSE";
	}
}

juice_log_level_t string_to_log_level(const char *str) {
	if(strcasecmp(str, "FATAL") == 0)
		return JUICE_LOG_LEVEL_FATAL;

	if(strcasecmp(str, "ERROR") == 0)
		return JUICE_LOG_LEVEL_ERROR;

	if(strcasecmp(str, "WARN") == 0 || strcasecmp(str, "WARNING") == 0)
		return JUICE_LOG_LEVEL_WARN;

	if(strcasecmp(str, "INFO") == 0)
		return JUICE_LOG_LEVEL_INFO;

	if(strcasecmp(str, "DEBUG") == 0)
		return JUICE_LOG_LEVEL_DEBUG;

	if(strcasecmp(str, "VERBOSE") == 0)
		return JUICE_LOG_LEVEL_VERBOSE;

	return JUICE_LOG_LEVEL_NONE;
}

