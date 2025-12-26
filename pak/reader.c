#include "reader.h"
#include "args.h"

off_t get_file_len(FILE* f)
{
	off_t posBackup = ftell(f), _len;
	fseek(f, 0, 2);
	_len = ftell(f);
	fseek(f, posBackup, 0);

	return _len;
}

#define min(a, b) ((a) <= (b) ? (a) : (b))

void PAKReader_init(PAKReader* restrict preader, const char* inFilePath)
{
	preader->pak = fopen(inFilePath, "rb");
	if (not preader->pak) {
		error(ENOENT, ENOENT, "%s：无法打开preader->pak(%s)文件指针(%p)", __func__, inFilePath, preader->pak);
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

	// 记录文件数据
	preader->fileArray = (PAK_File**)malloc(preader->header->fileCount * sizeof(PAK_File*));
	if (not preader->fileArray) {
		error(ENOMEM, ENOMEM, "%s：为preader->fileArray(%p)malloc失败", __func__, preader->fileArray);
	}
	for (uint32_t i = 0; i < preader->header->fileCount; i += 1) {
		preader->fileArray[i] = (PAK_File*)malloc(sizeof(PAK_File));
		if (not preader->fileArray[i]) {
			error(ENOMEM, ENOMEM, "%s：为preader->fileArray[%u](%p)malloc失败", __func__, i, preader->fileArray[i]);
		}

		preader->fileArray[i]->content = NULL;
		preader->fileArray[i]->relativeOffset = preader->fileOffsetTable[i];
		if (i < preader->header->fileCount - 1) { // 文件长 = 下一文件偏移 - 此文件偏移
			preader->fileArray[i]->length = preader->fileOffsetTable[i + 1] - preader->fileOffsetTable[i];
		} else { // 最后一个文件需要额外处理：偏移表中PAK文件总长 或 略大于 实际长。但最后输出的文件长还是按偏移表中数据定
			preader->fileArray[i]->length = min(get_file_len(preader->pak), preader->fileOffsetTable[preader->fileOffsetTableCount - 1]) - preader->fileOffsetTable[i];
		}
	}
}

void PAKReader_clear(PAKReader* restrict preader)
{
	if (preader->fileOffsetTable) {
		free(preader->fileOffsetTable);
		preader->fileOffsetTable = NULL;
	}
	if (preader->fileArray) {
		for (uint32_t i = 0; i < preader->header->fileCount; i += 1) {
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
	uint32_t bufferLen = (fileIndex == preader->header->fileCount - 1)
	    ? (preader->fileOffsetTable[preader->fileOffsetTableCount - 1] - preader->fileOffsetTable[preader->fileOffsetTableCount - 2])
	    : (preader->fileArray[fileIndex]->length); // 还是最后一个文件的问题

	void* content = malloc(preader->fileArray[fileIndex]->length);
	if (not content) {
		error(ENOMEM, ENOMEM, "%s：为content(%p)malloc失败", __func__, content);
	}
	memset(content, 0, bufferLen); // 全部先填0

	fseek(preader->pak, preader->fileArray[fileIndex]->relativeOffset, 0);
	fread(content, preader->fileArray[fileIndex]->length, 1, preader->pak);

	preader->fileArray[fileIndex]->content = content;
}

void extract(Args* restrict args)
{
	PAKReader preader;
	PAKReader_init(&preader, args->filePath);

	char outFilePath[1024];

	if (args->extractAll) {
		for (uint32_t i = 0; i < preader.header->fileCount; i += 1) {
			PAKReader_read_content(&preader, i);

			sprintf(outFilePath, "%s/0x%08x", args->dir, preader.fileArray[i]->relativeOffset);
			if (args->verbose) {
				puts(outFilePath);
			}

			FILE* binFile = fopen(outFilePath, "wb");
			if (not binFile) {
				error(EINVAL, EINVAL, "%s：无法打开文件%s的指针%p", __func__, outFilePath, binFile);
			}
			fwrite(preader.fileArray[i]->content, preader.fileArray[i]->length, 1, binFile);
			// fwrite(preader.fileArray[i]->content, preader.fileArray[i]->bufferSize, 1, binFile); //TODO: 针对最后一个文件，还是要把文件长和缓冲区长分开
			fclose(binFile);
		}
	} else {
		// TODO
	}
}
