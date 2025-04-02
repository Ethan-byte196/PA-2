#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length);
void log_event(const char *event);
void log_command(const char *command, const char *name, uint32_t salary);

#endif
