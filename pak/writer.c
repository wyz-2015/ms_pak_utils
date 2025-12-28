#include "writer.h"

uint32_t find_c1o(const uint32_t c1)
{
	uint32_t c1o = c1;
	while (c1o % 4 != 0) {
		c1o += 1;
	}

	return c1o;
}

uint32_t find_n_block_size(const uint32_t size)
{
	uint32_t size2 = size;
	while (size2 % BLOCK_SIZE != 0) {
		size2 += 1;
	}

	return size2;
}

void PAKWriter_init(PAKWriter* restrict pwriter, const Args* restrict args)
{
	pwriter->args = args;

	pwriter->pak = fopen(pwriter->args->filePath, "wb");
	if (not pwriter->pak) {
		error(ENOENT, ENOENT, "%s：打开文件%s的指针(%p)失败", __func__, pwriter->args->filePath, pwriter->pak);
	}

	// 初始化头
	pwriter->header = (PAK_Header*)malloc(sizeof(PAK_Header));
	if (not pwriter->header) {
		error(ENOMEM, ENOMEM, "%s：为pwriter->header(%p)malloc失败", __func__, pwriter->header);
	}

	memset(pwriter->header, 0, sizeof(PAK_Header));	       // 先全部填0，后面类似
	pwriter->header->fileCount = args->itemDeque->__len__; // 文件数即传入的文件deque的项数
	memcpy(pwriter->header->magicStr, args->magicStr, 4);  // 写Magic数

	// 初始化文件偏移表
	uint32_t c1 = args->itemDeque->__len__ + 1; //(c + 1)项

	pwriter->fileOffsetTableCount = find_c1o(c1); // 想让文件能4 Byte对齐，文件内容总能从0xnnnnnnn0处开始。据此计算(c + 1 + o)
	uint32_t fileOffsetTableSize = pwriter->fileOffsetTableCount * sizeof(uint32_t);
	pwriter->fileOffsetTable = (uint32_t*)malloc(fileOffsetTableSize);
	if (not pwriter->fileOffsetTable) {
		error(ENOMEM, ENOMEM, "%s：为pwriter->fileOffsetTable(%p)malloc失败", __func__, pwriter->fileOffsetTable);
	}
	memset(pwriter->fileOffsetTable, 0, fileOffsetTableSize);

	// 初始化文件数组，一切留空
	uint32_t fileArraySize = pwriter->header->fileCount * sizeof(PAK_File*);
	pwriter->fileArray = (PAK_File**)malloc(fileArraySize);
	if (not pwriter->fileArray) {
		error(ENOMEM, ENOMEM, "%s：为pwriter->fileArray(%p)malloc失败", __func__, pwriter->fileArray);
	}

	for (uint32_t i = 0; i < pwriter->header->fileCount; i += 1) {
		pwriter->fileArray[i] = (PAK_File*)malloc(sizeof(PAK_File));
		if (not pwriter->fileArray[i]) {
			error(ENOMEM, ENOMEM, "%s：为pwriter->fileArray[i](%p)malloc失败", __func__, pwriter->fileArray[i]);
		}

		memset(pwriter->fileArray[i], 0, sizeof(PAK_File));
	}

	pwriter->fileArray[0]->relativeOffset = sizeof(PAK_Header) + fileOffsetTableSize; // 但是第0个文件的相对偏移要先算好，作为后续计算的起点
	pwriter->fileOffsetTable[0] = pwriter->fileArray[0]->relativeOffset;
}

void PAKWriter_read(PAKWriter* restrict pwriter) // 读取待写入文件的内容
{
	Deque_Node* item = pwriter->args->itemDeque->head;
	PAK_File** pf = pwriter->fileArray;
	FILE* inFile;
	while (item) {
		if (pwriter->args->verbose) {
			puts(item->value);
		}

		inFile = fopen(item->value, "rb");
		if (not inFile) {
			error(ENOENT, ENOENT, "%s：无法打开文件%s的指针%p", __func__, item->value, inFile);
		}

		(*pf)->length = get_file_len(inFile);
		(*pf)->bufferSize = find_n_block_size((*pf)->length);

		(*pf)->content = malloc((*pf)->bufferSize);
		if (not(*pf)->content) {
			error(ENOMEM, ENOMEM, "%s：为(*pf)->content(%p)malloc失败", __func__, (*pf)->content);
		}
		memset((*pf)->content, 0, (*pf)->bufferSize); // 补0

		fseek(inFile, 0, 0);
		fread((*pf)->content, (*pf)->length, 1, inFile);

		fclose(inFile);
		inFile = NULL;

		item = item->next;
		pf += 1;
	}
}

void PAKWriter_clac_offset(PAKWriter* restrict pwriter) // 计算文件偏移表
{
	for (uint32_t i = 1; i < pwriter->header->fileCount; i += 1) {
		pwriter->fileArray[i]->relativeOffset = pwriter->fileArray[i - 1]->relativeOffset + pwriter->fileArray[i - 1]->bufferSize;
		pwriter->fileOffsetTable[i] = pwriter->fileArray[i]->relativeOffset;
	}

	pwriter->fileOffsetTable[pwriter->header->fileCount - 1 + 1] = pwriter->fileArray[pwriter->header->fileCount - 1]->relativeOffset + pwriter->fileArray[pwriter->header->fileCount - 1]->bufferSize; // 偏移表最后一项，文件总长
}

void PAKWriter_build_pakFile(PAKWriter* restrict pwriter) // 构建PAK文件
{
	fseek(pwriter->pak, 0, 0);

	fwrite(pwriter->header, sizeof(PAK_Header), 1, pwriter->pak);
	fwrite(pwriter->fileOffsetTable, pwriter->fileOffsetTableCount * sizeof(uint32_t), 1, pwriter->pak);

	for (uint32_t i = 0; i < pwriter->header->fileCount; i += 1) {
		fwrite(pwriter->fileArray[i]->content, pwriter->fileArray[i]->bufferSize, 1, pwriter->pak);
	}
}

// clear函数倒是完全一致的，所以直接作为一个别名
// #define PAKWriter_clear(pwriter) PAKReader_clear((pwriter))

void archive(const Args* restrict args)
{
	PAKWriter pwriter;
	PAKWriter_init(&pwriter, args);

	PAKWriter_read(&pwriter);
	PAKWriter_clac_offset(&pwriter);

	PAKWriter_build_pakFile(&pwriter);

	PAKWriter_clear(&pwriter);
}
