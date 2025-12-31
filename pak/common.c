#include "common.h"

uint32_t get_file_len(FILE* restrict f) // 文件偏移表中，用uint32_t表示偏移。意味着，不仅每个文件体积要<= 4 GiB，且所有文件的体积和也要<= 4 GiB。
{
	uint32_t posBackup = ftell(f), _len;
	fseek(f, 0, 2);
	_len = ftell(f);
	fseek(f, posBackup, 0);

	return _len;
}

uint32_t get_file_lines(FILE* restrict f)
{
	uint32_t posBackup = ftell(f), LFCount = 0;

	fseek(f, 0, 0);

	int c;
	while ((c = fgetc(f)) != EOF) {
		if (c == '\n') {
			LFCount += 1;
		}
	}

	fseek(f, posBackup, 0);
	return LFCount + 1; // 预防一些文本文件末尾没'\n'的情况。所以这不是一款严谨的求行数函数
}

void str_rstrip(char* restrict s, const size_t s_len)
{
	char* ps = s + s_len - 1; // 定位到'\0'前1个字符

	while (strchr("\n\t ", *ps)) {
		*ps = '\0';
		ps -= 1;
	}
}

int strcasecmp(const char* s1, const char* s2) // 不计大小写地比较两个字符串的大小
{
	char c1, c2;
	while (*s1 and *s2) {
		c1 = tolower(*s1);
		c2 = tolower(*s2);

		if (c1 > c2) {
			return 1;
		} else if (c1 < c2) {
			return -1;
		}

		s1 += 1;
		s2 += 1;
	}

	c1 = tolower(*s1);
	c2 = tolower(*s2);
	if (c1 > c2) { // 同时避免空字符串的情况
		return 1;
	} else if (c1 < c2) {
		return -1;
	} else {
		return 0;
	}
}

bool feof_(FILE* restrict inFile) // 是否到达文件结尾
{
	int cTest = fgetc(inFile); // 试取字

	if (cTest != EOF) {
		ungetc(cTest, inFile); // 若不是则放回流中，甚至可以是stdin
		return false;
	} else {
		return true;
	}
}
