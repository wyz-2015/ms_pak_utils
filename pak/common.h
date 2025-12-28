#ifndef _COMMON_H
#define _COMMON_H

#include <errno.h>
#include <error.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILEPATH_LEN_MAX 1024

#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

extern uint32_t get_file_len(FILE* f);

#endif
