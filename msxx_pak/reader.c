#include "reader.h"
#include <sys/stat.h>

bool PAKReader_file_check(PAKReader* restrict preader) // 检查文件
{
	const PAK_Item* lastItem = preader->itemTable[preader->itemCount - 1];
	uint32_t fileLen = lastItem->relativeOffset + lastItem->fileLength, fileLen_real = get_file_len(preader->pak);

	if (abs(fileLen - fileLen_real) < sizeof(uint32_t)) {
		return true;
	} else {
		return false;
	}
}

void PAKReader_init(PAKReader* restrict preader, const Args* args)
{
	preader->args = args;

	preader->pak = fopen(preader->args->filePath, "rb");
	if (not preader->pak) {
		error(ENOENT, ENOENT, "%s：无法打开preader->pak(%s)文件指针(%p)", __func__, args->filePath, preader->pak);
	}

	// 读头
	preader->header = (PAK_Header*)malloc(sizeof(PAK_Header));
	if (not preader->header) {
		error(ENOMEM, ENOMEM, "%s：为preader->header(%p)malloc失败", __func__, preader->header);
	}

	fread(preader->header, sizeof(PAK_Header), 1, preader->pak);

	preader->itemCount = preader->header->fileCount; // MSXX的PAK更好一点，这个数是可以相信的
	preader->itemCount_real = preader->itemCount;

	// 读文件目录
	preader->itemTable = (PAK_Item**)malloc(sizeof(PAK_Item*) * preader->itemCount_real);
	if (not preader->itemTable) {
		error(ENOMEM, ENOMEM, "%s：为preader->tiemTable(%p)malloc失败", __func__, preader->itemTable);
	}
	for (uint32_t i = 0; i < preader->itemCount_real; i += 1) {
		preader->itemTable[i] = (PAK_Item*)malloc(sizeof(PAK_Item));
		if (not preader->itemTable[i]) {
			error(ENOMEM, ENOMEM, "%s：为preader->tiemTable[%u](%p)malloc失败", __func__, i, preader->itemTable[i]);
		}
		fread(preader->itemTable[i], sizeof(PAK_Item), 1, preader->pak);
	}

	// 检查文件
	if (not PAKReader_file_check(preader)) {
		error(EPERM, EPERM, "%s：传入的文件%s(%p)疑似不是MSXX PAK文件，至少不符合文件头或文件目录规定", __func__, preader->args->filePath, preader->pak);
	}

	// 初始化文件数组
	preader->fileArray = (PAK_File**)malloc(sizeof(PAK_File*) * preader->itemCount_real);
	if (not preader->fileArray) {
		error(ENOMEM, ENOMEM, "%s：为preader->fileArray(%p)malloc失败", __func__, preader->fileArray);
	}
	for (uint32_t i = 0; i < preader->itemCount_real; i += 1) {
		preader->fileArray[i] = (PAK_File*)malloc(sizeof(PAK_File));
		if (not preader->fileArray[i]) {
			error(ENOMEM, ENOMEM, "%s：为preader->fileArray[%u](%p)malloc失败", __func__, i, preader->fileArray[i]);
		}

		preader->fileArray[i]->content = NULL;
		preader->fileArray[i]->relativeOffset = 0;
		preader->fileArray[i]->fileLength = 0;
		preader->fileArray[i]->bufferSize = 0;
		preader->fileArray[i]->subID = 0;
		preader->fileArray[i]->subDir = 0; // 是数字，不是字符串
	}
}

void PAKReader_read_metadata(PAKReader* restrict preader) // 读取PAK_File中的元数据
{
	for (uint32_t i = 0; i < preader->itemCount_real; i += 1) {
		preader->fileArray[i]->relativeOffset = preader->itemTable[i]->relativeOffset;
		preader->fileArray[i]->fileLength = preader->itemTable[i]->fileLength;
		preader->fileArray[i]->bufferSize = preader->fileArray[i]->fileLength;
		preader->fileArray[i]->subDir = preader->itemTable[i]->subDir;
		preader->fileArray[i]->subID = preader->itemTable[i]->subID;
	}
}

void PAKReader_read_content(PAKReader* restrict preader, const uint32_t fileIndex) // 读取PAK_File中的内容数据
{
	uint32_t i = fileIndex;
	preader->fileArray[i]->content = malloc(preader->fileArray[i]->bufferSize);
	if (not preader->fileArray[i]->content) {
		error(ENOMEM, ENOMEM, "%s：为preader->fileArray[%u]->content(%p)malloc失败", __func__, i, preader->fileArray[i]->content);
	}

	fseek(preader->pak, preader->fileArray[i]->relativeOffset, 0);
	fread(preader->fileArray[i]->content, preader->fileArray[i]->bufferSize, 1, preader->pak);
}

void PAKReader_copy_content(PAKReader* restrict preader, const uint32_t fileIndex)
{
	char dir[FILEPATH_LEN_MAX], outFilePath[FILEPATH_LEN_MAX];
	memset(dir, '\0', FILEPATH_LEN_MAX * sizeof(char));
	memset(outFilePath, '\0', FILEPATH_LEN_MAX * sizeof(char));

	struct stat sb;

	if (not(stat(preader->args->dir, &sb) == 0 and S_ISDIR(sb.st_mode))) { // 判定-C指定的目录是否存在
		error(ENOENT, ENOENT, "%s：输出目录%s不存在", __func__, preader->args->dir);
	}

	sprintf(dir, "%s/0x%x", preader->args->dir, preader->fileArray[fileIndex]->subDir);

	if (not(stat(dir, &sb) == 0 and S_ISDIR(sb.st_mode))) { // 创建以subDir为名的子目录
		if (mkdir(dir, 0755) != 0) {
			error(EPERM, EPERM, "%s：目录%s创建失败", __func__, dir);
		}
	}

	sprintf(outFilePath, "%s/0x%x", dir, preader->fileArray[fileIndex]->subID);
	if (preader->args->verbose) {
		printf("0x%x/0x%x\n", preader->fileArray[fileIndex]->subDir, preader->fileArray[fileIndex]->subID);
	}

	FILE* outFile = fopen(outFilePath, "wb");
	if (not outFile) {
		error(ENOENT, ENOENT, "%s：打开文件%s的指针%p失败", __func__, outFilePath, outFile);
	}

	fwrite(preader->fileArray[fileIndex]->content, preader->fileArray[fileIndex]->bufferSize, 1, outFile);

	fclose(outFile);
	outFile = NULL;
}

void PAKReader_clear(PAKReader* restrict preader)
{
	if (preader->header) {
		free(preader->header);
		preader->header = NULL;
	}

	if (preader->itemTable) {
		for (uint32_t i = 0; i < preader->itemCount_real; i += 1) {
			if (preader->itemTable[i]) {
				free(preader->itemTable[i]);
				preader->itemTable[i] = NULL;
			}
		}
		free(preader->itemTable);
		preader->itemTable = NULL;
	}

	if (preader->fileArray) {
		for (uint32_t i = 0; i < preader->itemCount_real; i += 1) {
			if (preader->fileArray[i]) {
				if (preader->fileArray[i]->content) {
					free(preader->fileArray[i]->content);
					preader->fileArray[i]->content = NULL;
				}
				free(preader->fileArray[i]);
				preader->fileArray[i] = NULL;
			}
		}
		preader->fileArray = NULL;
	}
}

/* ********************************** */

void extract(const Args* restrict args)
{
	PAKReader preader;
	PAKReader_init(&preader, args);

	if (args->extractAll) {
		for (uint32_t i = 0; i < preader.itemCount; i += 1) {
			PAKReader_read_metadata(&preader);
			PAKReader_read_content(&preader, i);
			PAKReader_copy_content(&preader, i);
		}
	} else {
		Deque_Node* item = args->itemDeque->head;
		uint32_t itemNo;
		while (item) {
			if (sscanf(item->value, "%u", &itemNo) != 1) {
				error(EINVAL, EINVAL, "%s：“%s”无法读取为一个uint32_t数，无法作为文件序号", __func__, item->value);
			}
			PAKReader_read_metadata(&preader);
			PAKReader_read_content(&preader, itemNo);
			PAKReader_copy_content(&preader, itemNo);

			item = item->next;
		}
	}

	PAKReader_clear(&preader);
}

void list(const Args* restrict args)
{
	PAKReader preader;
	PAKReader_init(&preader, args);

	PAKReader_read_metadata(&preader);

	printf("总文件数：%u\n", preader.itemCount);

	char s1[15], s2[15], s3[10], s4[10];
	// puts("No.\tOffset(hex)\tLength(hex)\n=====================================");
	printf("%-5s%-10s%-10s%-15s%-20s%-15s\n", "No.", "subDir", "subID", "Offset(hex)", "Length(hex, Byte)", "Length(KiB)");
	puts("========================================================================");
	for (uint32_t i = 0; i < preader.itemCount; i += 1) {
		sprintf(s1, "0x%08x", preader.fileArray[i]->relativeOffset);
		sprintf(s2, "0x%08x", preader.fileArray[i]->fileLength);
		sprintf(s3, "0x%x", preader.fileArray[i]->subDir);
		sprintf(s4, "0x%x", preader.fileArray[i]->subID);
		printf("%-5u%-10s%-10s%-15s%-20s%-15lf\n",
		    i,
		    s3,
		    s4,
		    s1,
		    s2,
		    B2KB(preader.fileArray[i]->fileLength));
	}

	PAKReader_clear(&preader);
}
