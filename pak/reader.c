#include "reader.h"

/*
off_t get_file_len(FILE* f)
{
	off_t posBackup = ftell(f), _len;
	fseek(f, 0, 2);
	_len = ftell(f);
	fseek(f, posBackup, 0);

	return _len;
}

#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))
*/

bool PAKReader_file_check(PAKReader* restrict preader) // 文件头与偏移表检查
{
	char magicStr[5] = { 0 };
	memcpy(magicStr, preader->header->magicStr, 4);

	if (not strstr("DATA MENU FONT STRD", magicStr)) {
		return false;
	}

	long long int fileLen_real = get_file_len(preader->pak), fileLen = preader->fileOffsetTable[preader->fileOffsetTableCount - 1];
	if (not(abs(fileLen - fileLen_real) < 4 * (long long int)sizeof(uint32_t))) { // 一般fileLen > fileLen_real
		return false;
	}

	return true;
}

void PAKReader_init(PAKReader* restrict preader, const Args* restrict args)
{
	preader->args = args;

	preader->pak = fopen(args->filePath, "rb");
	if (not preader->pak) {
		error(ENOENT, ENOENT, "%s：无法打开preader->pak(%s)文件指针(%p)", __func__, args->filePath, preader->pak);
	}

	// 读头
	preader->header = (PAK_Header*)malloc(sizeof(PAK_Header));
	if (not preader->header) {
		error(ENOMEM, ENOMEM, "%s：为preader->header(%p)malloc失败", __func__, preader->header);
	}

	fread(preader->header, sizeof(PAK_Header), 1, preader->pak);

	// 读文件偏移表
	preader->fileOffsetTableCount = preader->header->fileCount + 1;
	preader->fileOffsetTable = (uint32_t*)malloc(preader->fileOffsetTableCount * sizeof(uint32_t));
	if (not preader->fileOffsetTable) {
		error(ENOMEM, ENOMEM, "%s：为preader->fileOffsetTable(%p)malloc失败", __func__, preader->fileOffsetTable);
	}

	fread(preader->fileOffsetTable, sizeof(uint32_t), preader->fileOffsetTableCount, preader->pak);

	// 文件检查
	if (not PAKReader_file_check(preader)) {
		error(EPERM, EPERM, "%s：传入的文件%s(%p)疑似不是MS3D、6、7 PAK文件，至少不符合文件头或偏移表的规定", __func__, preader->args->filePath, preader->pak);
	}

	// 记录文件数据。本来在初始化时就计算，但为了MS7的复杂情况，将功能分离出去。

	// 为7代准备的，MS7的偏移表很混乱，有值为0却计入文件数的情况
	preader->fileCount_real = 0;
	for (uint32_t i = 0; i < preader->header->fileCount; i += 1) {
		if (preader->fileOffsetTable[i]) {
			preader->fileCount_real += 1;
		}
	}

	preader->fileArray = (PAK_File**)malloc(preader->fileCount_real * sizeof(PAK_File*));
	if (not preader->fileArray) {
		error(ENOMEM, ENOMEM, "%s：为preader->fileArray(%p)malloc失败", __func__, preader->fileArray);
	}
	// for (uint32_t i = 0; i < preader->header->fileCount; i += 1) {
	for (uint32_t i = 0; i < preader->fileCount_real; i += 1) {
		preader->fileArray[i] = (PAK_File*)malloc(sizeof(PAK_File));
		if (not preader->fileArray[i]) {
			error(ENOMEM, ENOMEM, "%s：为preader->fileArray[%u](%p)malloc失败", __func__, i, preader->fileArray[i]);
		}

		preader->fileArray[i]->content = NULL;
		preader->fileArray[i]->bufferSize = 0;
		preader->fileArray[i]->length = 0;
		preader->fileArray[i]->relativeOffset = 0;
		/*
		preader->fileArray[i]->relativeOffset = preader->fileOffsetTable[i];
		if (i < preader->header->fileCount - 1) { // 文件长 = 下一文件偏移 - 此文件偏移
			preader->fileArray[i]->length = preader->fileOffsetTable[i + 1] - preader->fileOffsetTable[i];
			preader->fileArray[i]->bufferSize = preader->fileArray[i]->length; // 缓冲区长 = 文件长
		} else {								   // 最后一个文件需要额外处理：偏移表中PAK文件总长 或 略大于 实际长。但最后输出的文件长还是按偏移表中数据定
			preader->fileArray[i]->length = min(get_file_len(preader->pak), preader->fileOffsetTable[preader->fileOffsetTableCount - 1]) - preader->fileOffsetTable[i];
			preader->fileArray[i]->bufferSize = max(get_file_len(preader->pak), preader->fileOffsetTable[preader->fileOffsetTableCount - 1]) - preader->fileOffsetTable[i];
		}
		*/
	}
}

void PAKReader_calc_offset_len(PAKReader* restrict preader) // 新的、分离出来的，获取偏移表中真正有效偏移值，计算各文件的实际长度的函数
{
	uint32_t i_array = 0, i;
	for (i = 0; i < preader->header->fileCount; i += 1) {
		if (preader->fileOffsetTable[i]) {
			preader->fileArray[i_array]->relativeOffset = preader->fileOffsetTable[i];
			i_array += 1;
		}
	}

	for (i_array = 0; i_array < preader->fileCount_real - 1; i_array += 1) {
		preader->fileArray[i_array]->length = preader->fileArray[i_array + 1]->relativeOffset - preader->fileArray[i_array]->relativeOffset;
		preader->fileArray[i_array]->bufferSize = preader->fileArray[i_array]->length;
	}

	uint32_t fileLen = preader->fileOffsetTable[preader->fileOffsetTableCount - 1], fileLen_real = get_file_len(preader->pak);
	preader->fileArray[preader->fileCount_real - 1]->length = min(fileLen_real, fileLen) - preader->fileArray[preader->fileCount_real - 1]->relativeOffset;
	preader->fileArray[preader->fileCount_real - 1]->bufferSize = max(fileLen_real, fileLen) - preader->fileArray[preader->fileCount_real - 1]->relativeOffset;
}

void PAKReader_clear(PAKReader* restrict preader)
{
	if (preader->fileOffsetTable) {
		free(preader->fileOffsetTable);
		preader->fileOffsetTable = NULL;
	}
	if (preader->fileArray) {
		for (uint32_t i = 0; i < preader->fileCount_real; i += 1) {
			if (preader->fileArray[i]->content) {
				free(preader->fileArray[i]->content);
				preader->fileArray[i]->content = NULL;
			}
			free(preader->fileArray[i]);
			preader->fileArray[i] = NULL;
		}

		free(preader->fileArray);
		preader->fileArray = NULL;
	}
	if (preader->pak) {
		fclose(preader->pak);
		preader->pak = NULL;
	}
}

void PAKReader_read_content(PAKReader* restrict preader, const uint32_t fileIndex) // 读取文件内容。只有要进行提取操作时才读取。建立目录还是必要的。
{
	/*
	uint32_t bufferLen = (fileIndex == preader->header->fileCount - 1)
	    ? (preader->fileOffsetTable[preader->fileOffsetTableCount - 1] - preader->fileOffsetTable[preader->fileOffsetTableCount - 2])
	    : (preader->fileArray[fileIndex]->length); // 还是最后一个文件的问题
	*/

	void* content = malloc(preader->fileArray[fileIndex]->bufferSize);
	if (not content) {
		error(ENOMEM, ENOMEM, "%s：为content(%p)malloc失败", __func__, content);
	}
	memset(content, 0, preader->fileArray[fileIndex]->bufferSize); // 全部先填0

	fseek(preader->pak, preader->fileArray[fileIndex]->relativeOffset, 0);
	fread(content, preader->fileArray[fileIndex]->length, 1, preader->pak);

	preader->fileArray[fileIndex]->content = content;
}

void PAKReader_copy_content(PAKReader* restrict preader, const uint32_t fileIndex, const char* outFilePath)
{
	if (preader->fileArray[fileIndex]->content == NULL) {
		PAKReader_read_content(preader, fileIndex);
	}

	FILE* binFile = fopen(outFilePath, "wb");
	if (not binFile) {
		error(EINVAL, EINVAL, "%s：无法打开文件%s的指针%p", __func__, outFilePath, binFile);
	}
	// fwrite(preader.fileArray[i]->content, preader.fileArray[i]->length, 1, binFile);
	fwrite(preader->fileArray[fileIndex]->content, preader->fileArray[fileIndex]->bufferSize, 1, binFile); // 针对最后一个文件，还是要把文件长和缓冲区长分开。已解决
	fclose(binFile);
}

void extract(Args* restrict args)
{
	PAKReader preader;
	PAKReader_init(&preader, args);

	PAKReader_calc_offset_len(&preader);

	char outFilePath[FILEPATH_LEN_MAX];

	if (args->extractAll) {
		for (uint32_t i = 0; i < preader.fileCount_real; i += 1) {
			sprintf(outFilePath, "%s/%s0x%08x", args->dir, args->prefix, preader.fileArray[i]->relativeOffset);
			if (args->verbose) {
				puts(outFilePath);
			}

			PAKReader_copy_content(&preader, i, outFilePath);
		}
	} else {
		Deque_Node* currentNode = args->itemDeque->head;
		uint32_t fileIndex = 0;
		while (currentNode) {
			if (sscanf(currentNode->value, "%u", &fileIndex) != 1) {
				error(EINVAL, EINVAL, "%s：无法读取参数%s为uint32_t数字", __func__, currentNode->value);
			}

			sprintf(outFilePath, "%s/%s0x%08x", args->dir, args->prefix, preader.fileArray[fileIndex]->relativeOffset);
			if (args->verbose) {
				puts(outFilePath);
			}

			PAKReader_copy_content(&preader, fileIndex, outFilePath);

			currentNode = currentNode->next;
		}
	}

	PAKReader_clear(&preader);
}

void list(Args* restrict args)
{
	PAKReader preader;
	PAKReader_init(&preader, args);

	PAKReader_calc_offset_len(&preader);

	printf("实际文件数：%u\n", preader.fileCount_real);

	puts("No.\tOffset(hex)\tLength(hex)\tLength(KiB)\n====================================================");
	for (uint32_t i = 0; i < preader.fileCount_real; i += 1) {
		printf("%u\t0x%08x\t0x%08x\t%lf\n",
		    i,
		    preader.fileArray[i]->relativeOffset,
		    preader.fileArray[i]->length,
		    B2KB(preader.fileArray[i]->length));
	}

	PAKReader_clear(&preader);
}
