#include "writer.h"

uint32_t find_n_block_size(const uint32_t size)
{
	uint32_t size2 = size;
	while (size2 % BLOCK_SIZE != 0) {
		size2 += 1;
	}

	return size2;
}

void PAKWriter_calc_itemCount_real(PAKWriter* restrict pwriter)
{
	Deque_Node* node = pwriter->args->itemDeque->head;
	uint32_t no, subDir, subID, offset, length,
	    count = 0;
	double lengthKiB;

	while (node) {
		// printf("%d\n", sscanf(node->value, "%u%x%x%x%x%lf", &no, &subDir, &subID, &offset, &length, &lengthKiB));
		// puts(node->value);
		if (sscanf(node->value, "%u%x%x%x%x%lf", &no, &subDir, &subID, &offset, &length, &lengthKiB) == 6) {
			count += 1;
		}

		node = node->next;
	}

	// printf("%u\n", count);
	pwriter->itemCount_real = count;
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

	memset(pwriter->header, 0, sizeof(PAK_Header)); // 先全部填0，后面类似
	pwriter->itemCount = args->itemDeque->__len__;	// 由于使用根据文件列表导入方法，deque的项数 >= 实际文件数
	PAKWriter_calc_itemCount_real(pwriter);		// 计算真正的文件项数

	// 初始化文件偏移表
	pwriter->itemTable = (PAK_Item**)malloc(pwriter->itemCount_real * sizeof(PAK_Item*));
	if (not pwriter->itemTable) {
		error(ENOMEM, ENOMEM, "%s：为pwriter->fileOffsetTable(%p)malloc失败", __func__, pwriter->itemTable);
	}
	for (uint32_t i = 0; i < pwriter->itemCount_real; i += 1) {
		pwriter->itemTable[i] = (PAK_Item*)malloc(sizeof(PAK_Item));
		if (not pwriter->itemTable[i]) {
			error(ENOMEM, ENOMEM, "%s：为pwriter->fileOffsetTable[%u](%p)malloc失败", __func__, i, pwriter->itemTable[i]);
		}
		memset(pwriter->itemTable[i], 0, sizeof(PAK_Item));
	}

	// 初始化文件数组，一切留空
	pwriter->fileArray = (PAK_File**)malloc(pwriter->itemCount_real * sizeof(PAK_File*));
	if (not pwriter->fileArray) {
		error(ENOMEM, ENOMEM, "%s：为pwriter->fileArray(%p)malloc失败", __func__, pwriter->fileArray);
	}

	for (uint32_t i = 0; i < pwriter->itemCount_real; i += 1) {
		pwriter->fileArray[i] = (PAK_File*)malloc(sizeof(PAK_File));
		if (not pwriter->fileArray[i]) {
			error(ENOMEM, ENOMEM, "%s：为pwriter->fileArray[%u](%p)malloc失败", __func__, i, pwriter->fileArray[i]);
		}

		memset(pwriter->fileArray[i], 0, sizeof(PAK_File));
	}
}

void PAKWriter_read(PAKWriter* restrict pwriter) // 读取待写入文件的内容
{
	Deque_Node* item = pwriter->args->itemDeque->head;
	PAK_File** pf = pwriter->fileArray;
	FILE* inFile;
	char inFilePath[FILEPATH_LEN_MAX];

	uint32_t no, subDir, subID, offset, length,
	    count = 0;
	double lengthKiB;

	while (item) {
		if (sscanf(item->value, "%u%x%x%x%x%lf", &no, &subDir, &subID, &offset, &length, &lengthKiB) != 6) { // 连读6个数，成功才可登记。实际有用的数只有subDir和subID
			item = item->next;
			continue;
		}

		if (pwriter->args->verbose) {
			puts(item->value);
		}

		sprintf(inFilePath, "%s/0x%x/0x%x", pwriter->args->dir, subDir, subID);

		inFile = fopen(inFilePath, "rb");
		if (not inFile) {
			error(ENOENT, ENOENT, "%s：无法打开文件%s的指针%p", __func__, item->value, inFile);
		}

		(*pf)->fileLength = get_file_len(inFile);
		(*pf)->bufferSize = find_n_block_size((*pf)->fileLength);
		(*pf)->subDir = subDir;
		(*pf)->subID = subID;

		(*pf)->content = malloc((*pf)->bufferSize);
		if (not(*pf)->content) {
			error(ENOMEM, ENOMEM, "%s：为(*pf)->content(%p)malloc失败", __func__, (*pf)->content);
		}
		memset((*pf)->content, 0, (*pf)->bufferSize); // 补0

		fseek(inFile, 0, 0);
		fread((*pf)->content, (*pf)->fileLength, 1, inFile);

		fclose(inFile);
		inFile = NULL;

		item = item->next;
		pf += 1;
	}
	// 计算文件偏移表
	pwriter->fileArray[0]->relativeOffset = sizeof(PAK_Header) + sizeof(PAK_Item) * pwriter->itemCount_real; // 但是第0个文件的相对偏移要先算好，作为后续计算的起点

	for (uint32_t i = 1; i < pwriter->itemCount_real; i += 1) {
		pwriter->fileArray[i]->relativeOffset = pwriter->fileArray[i - 1]->relativeOffset + pwriter->fileArray[i - 1]->bufferSize;
	}
}

void PAKWriter_build_pakFile(PAKWriter* restrict pwriter) // 构建PAK文件
{
	fseek(pwriter->pak, 0, 0);

	pwriter->header->fileCount = pwriter->itemCount_real;
	fwrite(pwriter->header, sizeof(PAK_Header), 1, pwriter->pak);

	for (uint32_t i = 0; i < pwriter->itemCount_real; i += 1) {
		if (i < pwriter->itemCount_real) {
			pwriter->itemTable[i]->fileLength = pwriter->fileArray[i]->fileLength;
			pwriter->itemTable[i]->relativeOffset = pwriter->fileArray[i]->relativeOffset;
			pwriter->itemTable[i]->subID = pwriter->fileArray[i]->subID;
			pwriter->itemTable[i]->subDir = pwriter->fileArray[i]->subDir;
		}

		fwrite(pwriter->itemTable[i], sizeof(PAK_Item), 1, pwriter->pak);
	}

	for (uint32_t i = 0; i < pwriter->itemCount_real; i += 1) {
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

	PAKWriter_build_pakFile(&pwriter);

	PAKWriter_clear(&pwriter);
}
