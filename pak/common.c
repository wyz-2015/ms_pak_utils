#include "common.h"

uint32_t get_file_len(FILE* restrict f) // 文件偏移表中，用uint32_t表示偏移。意味着，不仅每个文件体积要<= 4 GiB，且所有文件的体积和也要<= 4 GiB。
{
	uint32_t posBackup = ftell(f), _len;
	fseek(f, 0, 2);
	_len = ftell(f);
	fseek(f, posBackup, 0);

	return _len;
}
