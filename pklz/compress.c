#include "compress.h"

void Comper_init(Comper* restrict cer, const Args* restrict args)
{
	cer->args = args;

	if (cer->args->inFilePath) {
		cer->inFile = fopen(cer->args->inFilePath, "rb");
		if (not cer->inFile) {
			error(ENOENT, ENOENT, "%s：无法打开文件%s的指针(%p)", __func__, cer->args->inFilePath, cer->inFile);
		}
	} else {
		cer->inFile = stdin;
	}

	cer->header = (PKLZ_Header*)malloc(sizeof(PKLZ_Header));
	if (not cer->header) {
		error(ENOMEM, ENOMEM, "%s：为cer->header(%p)malloc失败", __func__, cer->header);
	}
	memset(cer->header, 0, sizeof(PKLZ_Header));
	strcpy(cer->header->magicStr, "PK");
	cer->fileSize = sizeof(PKLZ_Header);

	cer->type = 0x00; // 没有LZSS压缩算法，只有直接拷贝并加头
	cer->header->type = cer->type;
	cer->decompSize_real = 0;
	cer->decompSize = 0;
}

void Comper_outFile_select(Comper* restrict cer) // 输出文件选择？没得选
{

	if (cer->args->outFilePath) {
		cer->outFile = fopen(cer->args->outFilePath, "wb");
		if (not cer->outFile) {
			error(ENOENT, ENOENT, "%s：无法打开文件%s的指针(%p)", __func__, cer->args->outFilePath, cer->outFile);
		}
	} else {
		if (cer->args->inFilePath) {
			char outFilePath[FILEPATH_LEN_MAX];
			sprintf(outFilePath, "%s.pklz", cer->args->inFilePath); // 扩展名倒是简单，默认追加“.pklz”

			cer->outFile = fopen(outFilePath, "wb"); // 同样地，能顺写，也要能回写

			if (not cer->outFile) {
				error(ENOENT, ENOENT, "%s：无法打开文件%s的指针(%p)", __func__, outFilePath, cer->outFile);
			}
		} else {
			error(EINVAL, EINVAL, "%s：若是从stdin读取数据，则必须指定一个输出文件(-o)", __func__);
		}
	}
}

void Comper_raw_copy(Comper* restrict cer) // 没有LZSS压缩算法，只有直接拷贝并加头
{
	fwrite(cer->header, sizeof(PKLZ_Header), 1, cer->outFile);

	int byte;
	while ((byte = fgetc(cer->inFile)) != EOF) {
		fputc(byte, cer->outFile);
		cer->decompSize_real += 1;
	}

	// cer->decompSize_real = ftell(cer->inFile); // inFile必然已到末尾 -> 不行，若是走stdin，返回的值是0xffffffff
	cer->decompSize = cer->decompSize_real;
	cer->fileSize += cer->decompSize_real;

	// 填写头中的解压后数据
	fseek(cer->outFile, (3 * sizeof(char) + 1 * sizeof(uint8_t)), 0);
	fwrite(&cer->decompSize_real, sizeof(uint32_t), 1, cer->outFile);
	fseek(cer->outFile, 0, 2);
}

void compress(const Args* restrict args)
{
	Comper cer;
	Comper_init(&cer, args);

	Comper_outFile_select(&cer);

	Comper_raw_copy(&cer);

	if (args->verbose) {
		printf("压缩前文件大小：%u Byte = %lf KiB；压缩后(实际上只是加头)文件大小：%u Byte = %lf KiB\n", cer.decompSize_real, B2KB(cer.decompSize_real), cer.fileSize, B2KB(cer.fileSize));
	}

	Comper_clear(&cer);
}
