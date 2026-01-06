#ifndef _COMMON_H
#define _COMMON_H

#include <ctype.h>
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
#define abs(a) (((int)(a)) >= 0 ? ((int)(a)) : (-((int)(a))))

#define B2KB(x) ((double)(x) / 1024)

extern uint32_t get_file_len(FILE* __restrict f);
extern uint32_t get_file_lines(FILE* __restrict f);
extern void str_rstrip(char* __restrict s, const size_t s_len);
extern int strcasecmp(const char* s1, const char* s2); // 不计大小写地比较两个字符串的大小
extern bool feof_(FILE* __restrict inFile);	       // 是否到达文件结尾

#endif
