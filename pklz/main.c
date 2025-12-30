#include "args.h"
#include "compress.h"
#include "decompress.h"
#include <argp.h>

static const struct argp_option options[] = {
	{ "compress", 'z', NULL, OPTION_ARG_OPTIONAL, "压缩。未指定传入文件时，会从stdin读取数据" },
	{ "decompress", 'd', NULL, OPTION_ARG_OPTIONAL, "解压缩。未指定传入文件时，会从stdin读取数据" },
	// { "stdout", 'c', NULL, OPTION_ARG_OPTIONAL, "向标准输出写入" },
	{ "verbose", 'v', NULL, OPTION_ARG_OPTIONAL, "显示详细信息" },
	{ "out-file", 'o', "FILE", 0, "指定输出文件为FILE(必须)" },
	{ 0 }
};

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
	Args* args = state->input;

	switch (key) {
	/*
	case 'c': {
		args->toStdout = true;
		break;
	}
	*/
	case 'z': {
		if (args->mode != '\0') {
			error(EINVAL, EINVAL, "-z -d 选项不可共存");
		}
		args->mode = 'z';
		break;
	}
	case 'd': {
		if (args->mode != '\0') {
			error(EINVAL, EINVAL, "-z -d 选项不可共存");
		}
		args->mode = 'd';
		break;
	}
	case 'v': {
		args->verbose = true;
		break;
	}
	case ARGP_KEY_ARG: {
		args->inFilePath = arg;
		break;
	}
	}
	return 0;
}

static struct argp argp = { options, parse_opt, NULL, "一款支持 压缩(并非)、解压缩 Metal Slug 3D、7 中出现的被称为“PKLZ”的特有单文件压缩格式文件 的工具。" };

int main(int argc, char** argv)
{
	Args args = {
		.mode = '\0',
		.verbose = false,
		.inFilePath = NULL,
		.outFilePath = NULL
	};

	if (argc == 1) {
		argp_help(&argp, stdout, ARGP_HELP_USAGE | ARGP_HELP_LONG, NULL);
		error(EINVAL, EINVAL, "程序未接收到任何参数……");
	}

	argp_parse(&argp, argc, argv, 0, 0, &args);

	switch (args.mode) {
	case 'z': {
		compress(&args);
		break;
	}
	case 'd': {
		decompress(&args);
		break;
	}
	default: {
		error(EINVAL, EINVAL, "-z -d 必须指定其中之一");
		break;
	}
	}

	return 0;
}
