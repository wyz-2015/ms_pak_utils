#include "decompress.h"
#include <sys/stat.h>

// 魔术字为"PK"(带'\0')，且dec->type为0、2其中之一
#define Decomper_inFile_check(dec) (strcmp((dec)->header->magicStr, "PK") == 0 and strchr("\x02\x00", (dec)->header->type))

void Decomper_init(Decomper* restrict dec, const Args* restrict args)
{
	dec->args = args;

	// 打开文件
	if (args->inFilePath) {
		dec->inFile = fopen(args->inFilePath, "rb");
		if (not dec->inFile) {
			error(ENOENT, ENOENT, "%s：打开文件%s的指针(%p)失败", __func__, args->inFilePath, dec->inFile);
		}
	} else { // 若无传入文件的路径，则从stdin读入
		dec->inFile = stdin;
	}

	// 读头
	dec->header = (PKLZ_Header*)malloc(sizeof(PKLZ_Header));
	if (not dec->header) {
		error(ENOMEM, ENOMEM, "%s：为dec->header(%p)malloc失败", __func__, dec->header);
	}

	fread(dec->header, sizeof(PKLZ_Header), 1, dec->inFile); // 读过头后，若文件正确，指针已经落在正文数据起始处

	if (not Decomper_inFile_check(dec)) { // 检查头
		error(EPERM, EPERM, "%s：传入的文件%s(%p)疑似不是PKLZ文件，至少不符合文件头规定", __func__, args->inFilePath, dec->inFile);
	}

	if (feof_(dec->inFile)) { // 检查是否只有头
		error(EPERM, EPERM, "%s：这个文件%s(%p)只有文件头，却没有内容", __func__, dec->args->inFilePath, dec->inFile);
	}

	dec->decompSize = dec->header->decompSize;
	dec->decompSize_real = 0;
}

void Decomper_clear(Decomper* restrict dec)
{
	if (dec->inFile != NULL and dec->inFile != stdin) {
		fclose(dec->inFile);
		dec->inFile = NULL;
	}

	if (dec->header) {
		free(dec->header);
		dec->header = NULL;
	}

	if (dec->outFile != NULL /*and dec->outFile != stdout*/) {
		fclose(dec->outFile);
		dec->outFile = NULL;
	}
}

void Decomper_outFile_select(Decomper* restrict dec)
{
	if (dec->args->outFilePath) {				     // 有指定输出文件
		dec->outFile = fopen(dec->args->outFilePath, "wb+"); // 算法有回读部分，所以必须 新写+读 模式
		if (not dec->outFile) {
			error(ENOENT, ENOENT, "%s：无法打开文件%s的指针%p", __func__, dec->args->outFilePath, dec->outFile);
		}
	} else if (dec->args->outFilePath) { // 指定输出到stdout
		// dec->outFile = stdout;
		error(EPERM, EPERM, "%s：受LZSS算法实现中，有“回读”行为的限制：stdout作为只写流无法做到此操作。这里的实现不是buffer解压到buffer，而是两个FILE*一个读一个写。", __func__);
	} else { // 若传入文件以“.pklz”为扩展名(不计大小写)，则自动指定输出到去掉扩展名的文件中
		const char *sufferix = ".pklz",
			   *inFilePath_sufferix = dec->args->inFilePath + strlen(dec->args->inFilePath) - strlen(sufferix),
			   *warningMessage = "程序仅支持遇到传入文件名为“xxx.pklz”时自动指定输出文件名为“xxx”。";
		if (strcasecmp(sufferix, inFilePath_sufferix) == 0) { // 检查传入文件是否“.pklz”为扩展名
			char outFilePath[FILEPATH_LEN_MAX];
			strncpy(outFilePath, dec->args->inFilePath, strlen(dec->args->inFilePath) - strlen(sufferix));

			struct stat sb;
			if (stat(outFilePath, &sb) == 0) {
				error(EEXIST, EEXIST, "%s：%s现名为%s的文件已经存在，为防误覆盖文件，自动命名模式下有此阻止措施。", __func__, warningMessage, outFilePath);
			}

			dec->outFile = fopen(outFilePath, "wb");
			if (not dec->outFile) {
				error(ENOENT, ENOENT, "%s：无法打开文件%s的指针%p", __func__, outFilePath, dec->outFile);
			}
		} else {
			error(EPERM, EPERM, "%s：%s请用-o参数额外指定输出文件(不阻止覆盖)，或用-c参数指定输出到stdout。", __func__, warningMessage);
		}
	}
}

bool next_step(Decomper* restrict dec, uint32_t* restrict bitCount, uint8_t* restrict controlByte) // 返回值是信号，输入文件到末尾的信号。解压缩函数收到信号要break
{
	if (*bitCount >= 8) {
		if (feof_(dec->inFile)) {
			return true;
		}
		*controlByte = fgetc(dec->inFile); // 读一字前必先检查是否结尾，小心异常(评论)
		*bitCount = 0;
	}

	return false;
}

// b的左起第pos位(0数起，pos范围[0, 8))是什么
#define get_bit(b, pos) (((b) >> (7 - (pos))) & 0b00000001)

void Decomper_decompress(Decomper* restrict dec) // 处理0x02型，有压缩数据
{
	uint8_t controlByte = fgetc(dec->inFile);
	uint32_t bitCount = 0;

	while (feof_(dec->inFile)) { // ftell(f) < fileLen对于输入文件是stdin时不奏效
		if (next_step(dec, &bitCount, &controlByte)) {
			break;
		}

		uint8_t actionSymbol = get_bit(controlByte, bitCount);
		bitCount += 1;

		if (actionSymbol == 1) { // 操作位，1则复制1 Byte
			if (feof_(dec->inFile)) {
				break;
			}

			fputc(fgetc(dec->inFile), dec->outFile);
		} else {
			if (next_step(dec, &bitCount, &controlByte)) {
				break;
			}

			uint8_t typeSymbol = get_bit(controlByte, bitCount);
			bitCount += 1;

			int offset, length;

			if (typeSymbol == 0) { // 引用类型位。0则短引用，1则长引用。最后算出来拷贝字典起点和长度
				int val2bit = 0;
				for (int k = 0; k < 2; k += 1) {
					if (next_step(dec, &bitCount, &controlByte)) {
						break;
					}

					uint8_t b = get_bit(controlByte, bitCount); // 没明白这个值是干什么用的
					bitCount += 1;

					val2bit = (val2bit << 1) | (int)b;
				}

				if (feof_(dec->inFile)) {
					break;
				}
				offset = (int)fgetc(dec->inFile) - 0x100;
				length = val2bit + 2;

			} else {
				int b1 = fgetc(dec->inFile), b2 = fgetc(dec->inFile);
				if (b1 == EOF or b2 == EOF) {
					break;
				}

				int combined = (b1 << 8) | b2, offsetRaw = (combined >> 5);
				offset = offsetRaw - 0x800;

				int lengthRaw = b2 & 0b00011111;
				if (lengthRaw == 0) {
					if (feof_(dec->inFile)) {
						break;
					}
					length = (int)fgetc(dec->inFile) + 1;
				} else {
					length = lengthRaw + 2;
				}
			}

			int start = ftell(dec->outFile) + offset; // offset很可能是个负数
			if (start < 0) {
				start = 0;
			}

			uint8_t word2Copy[length];
			memset(word2Copy, 0, length * sizeof(uint8_t));

			if (start >= ftell(dec->outFile)) { // 若算出来的起始点已经在当前outFile末尾及之后了，直接写填0的数组即可
				fseek(dec->outFile, start, 0);
				fread(word2Copy, length, 1, dec->outFile);
			}

			fseek(dec->outFile, 0, 2);
			fwrite(word2Copy, length, 1, dec->outFile);
		}
	}
	dec->decompSize_real = get_file_len(dec->outFile);
}

void Decomper_raw_copy(Decomper* restrict dec)
{
	int8_t byte;
	while ((byte = fgetc(dec->inFile)) != EOF) { // 未压缩类型，直接拷个爽~
		fputc(byte, dec->outFile);
	}
	dec->decompSize_real = get_file_len(dec->outFile);
}

void decompress(const Args* restrict args)
{
	Decomper dec;
	Decomper_init(&dec, args);
	Decomper_outFile_select(&dec);

	switch (dec.type) {
	case 0x00: {
		Decomper_raw_copy(&dec);
		break;
	}
	case 0x02: {
		Decomper_decompress(&dec);
		break;
	}
	}

	if (args->verbose) {
		printf("理应解压数据：%u Byte = %lf KiB；实际解压：%u Byte = %lf KiB。\n", dec.decompSize, B2KB(dec.decompSize), dec.decompSize_real, B2KB(dec.decompSize_real));
	}

	Decomper_clear(&dec);
}
