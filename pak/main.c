#include "args.h"
#include "deque.h"
#include "reader.h"
#include "writer.h"
#include <argp.h>

Deque* itemList = NULL;

static const struct argp_option options[] = {
	{ "directory", 'C', "DIR", OPTION_ARG_OPTIONAL, "改变至目录DIR" },
	{ "file", 'f', "ARCHIVE", OPTION_ARG_OPTIONAL, "操作目标PAK ARCHIVE文件" },
	{ "create", 'c', NULL, OPTION_ARG_OPTIONAL, "创建一个新归档" },
	{ "extract", 'x', 0, OPTION_ARG_OPTIONAL, "从归档中解出文件" },
	{ "list", 't', NULL, OPTION_ARG_OPTIONAL, "列出归档内容" },
	{ "verbose", 'v', 0, OPTION_ARG_OPTIONAL, "显示详细信息" },
	{ "prefix", 'p', "str", OPTION_ARG_OPTIONAL, "由于这种PAK包格式连原文件名都不保留，输出的文件名只能以其在包中的相对偏移代替。使用此选项可以在输出文件名的“偏移值”之前加上一个前缀，便于标识" },
	{ "magic", 'm', "str", OPTION_ARG_OPTIONAL, "如果是新建PAK归档包，则规定文件头的魔术字，限\"DATA\" \"MENU\" \"FONT\" \"STRD\"，默认\"DATA\"。" },
	{ "files-from", 'T', "FILE", OPTION_ARG_OPTIONAL, "从FILE中获取文件名来解压或创建文件" },
	// { 0, '\0', "str", 0, "传入文件路径" }
	{ 0 }
};

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
	Args* args = state->input;

	switch (key) {
	case 'C': {
		if (arg) {
			args->dir = arg;
		}
		break;
	}
	case 'f': {
		if (arg == NULL) {
			error(EINVAL, EINVAL, "读取-f / --file的参数“%s”失败", arg);
		}
		args->filePath = arg;
		break;
	}
	case 'c': {
		if (args->mode != '\0') {
			error(EINVAL, EINVAL, "-t -c -x 选项不可共存");
		}
		args->mode = 'c';
		break;
	}
	case 'x': {
		if (args->mode != '\0') {
			error(EINVAL, EINVAL, "-t -c -x 选项不可共存");
		}
		args->mode = 'x';
		break;
	}
	case 't': {
		if (args->mode != '\0') {
			error(EINVAL, EINVAL, "-t -c -x 选项不可共存");
		}
		args->mode = 't';
		break;
	}
	case 'v': {
		args->verbose = true;
		break;
	}
	case 'p': {
		if (arg) {
			args->prefix = arg;
		}
		break;
	}
	case 'm': {
		if (strlen(arg) == 4 and strstr("DATA MENU FONT STRD", arg)) { // 字数匹配，字串匹配
			args->magicStr = arg;
		} else {
			error(EINVAL, EINVAL, "魔术字并非\"DATA\" \"MENU\" \"FONT\" \"STRD\"其中之一");
		}
	}
	case 'T': {
		args->fileListPath = arg;
		break;
	}
	case ARGP_KEY_ARG: {
		args->extractAll = false;
		Deque_append(itemList, arg);
		break;
	}
	}
	return 0;
}

static struct argp argp = { options, parse_opt, NULL, "一款支持 解包、罗列、打包 Metal Slug 3D、6、7中出现的.pak特有打包格式文件 的工具。" };

int main(int argc, char** argv)
{
	itemList = (Deque*)malloc(sizeof(Deque));
	if (not itemList) {
		error(ENOMEM, ENOMEM, "%s：为itemList(%p)malloc失败。", __func__, itemList);
	}
	Deque_init(itemList);

	// Args args = { NULL, 44100, 2, false, 2, 10000, false, 0.5, 0, 0, false };
	Args args = {
		.dir = ".", // 留一个点号，表示当前目录，否则保存目录会变成默认"/"
		.extractAll = true,
		.filePath = NULL,
		.itemDeque = itemList,
		.mode = '\0',
		.verbose = false,
		.prefix = "",
		.magicStr = "DATA",
		.fileListPath = NULL
	};

	if (argc == 1) {
		argp_help(&argp, stdout, ARGP_HELP_USAGE | ARGP_HELP_LONG, NULL);
		error(EINVAL, EINVAL, "程序未接收到任何参数……");
	}

	argp_parse(&argp, argc, argv, 0, 0, &args);

	if (args.fileListPath) { // 处理-T / --files-from
				 // Warning: 很奇怪，这段代码用clang(18.1.3 (1ubuntu1), x86_64-pc-linux-gnu)编译会出BUG
		FILE* fileListFile = fopen(args.fileListPath, "rt");
		if (not fileListFile) {
			error(ENOENT, ENOENT, "%s：无法文本模式打开文件%s的指针%p", __func__, args.fileListPath, fileListFile);
		}

		uint32_t lines = get_file_lines(fileListFile), lines_real = 0;
		char itemDeque_fromFile[lines][FILEPATH_LEN_MAX], tempStr[FILEPATH_LEN_MAX];
		memset(itemDeque_fromFile, 0, lines * FILEPATH_LEN_MAX * sizeof(char));
		memset(tempStr, 0, FILEPATH_LEN_MAX * sizeof(char));

		while (fgets(tempStr, FILEPATH_LEN_MAX, fileListFile)) {
			str_rstrip(tempStr, strlen(tempStr));

			if (*tempStr) { // rstrip()后不是空字符串
				strcpy(itemDeque_fromFile[lines_real], tempStr);
				lines_real += 1;
			}
		}

		for (uint32_t l = 0; l < lines_real; l += 1) {
			Deque_append(itemList, itemDeque_fromFile[l]);
		}

		fclose(fileListFile);
	}

	switch (args.mode) {
	case 'x': {
		extract(&args);
		break;
	}
	case 't': {
		list(&args);
		break;
	}
	case 'c': {
		// puts("尚未支持"); // TODO
		archive(&args);
		break;
	}
	default: {
		error(EINVAL, EINVAL, "-t -c -x 必须指定其中之一");
	}
	}

	return 0;
}
