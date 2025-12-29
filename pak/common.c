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

	char c;
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
