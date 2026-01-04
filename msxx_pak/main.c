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
	{ "extract", 'x', NULL, OPTION_ARG_OPTIONAL, "从归档中解出文件" },
	{ "list", 't', NULL, OPTION_ARG_OPTIONAL, "列出归档内容" },
	{ "verbose", 'v', NULL, OPTION_ARG_OPTIONAL, "显示详细信息" },
	{ "files-from", 'T', "FILE", OPTION_ARG_OPTIONAL, "从FILE中获取文件名来解包文件 / 读取本程序-t输出格式的外部文件表FILE，据此创建PAK包。文件表中只有subDir和subID的值是必须正确的" },
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

static struct argp argp = { options, parse_opt, NULL, "一款支持 解包、罗列、打包 Metal Slug XX (PSP)中出现的.pak特有打包格式文件 的工具。" };

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
		.fileListPath = NULL
	};

	if (argc == 1) {
		argp_help(&argp, stdout, ARGP_HELP_USAGE | ARGP_HELP_LONG, NULL);
		error(EINVAL, EINVAL, "程序未接收到任何参数……");
	}

	argp_parse(&argp, argc, argv, 0, 0, &args);

	// 处理-T / --files-from
	uint32_t lines, lines_real;
	char **itemDeque_fromFile = NULL, *tempStr = NULL;

	if (args.fileListPath) {
		FILE* fileListFile = fopen(args.fileListPath, "rt");
		if (not fileListFile) {
			error(ENOENT, ENOENT, "%s：无法文本模式打开文件%s的指针%p", __func__, args.fileListPath, fileListFile);
		}

		lines = get_file_lines(fileListFile);
		lines_real = 0;
		/* 作用域问题。出了这个大括号，两个数组变量就不再有效。此前带问题的程序能正常执行，简直就是奇迹
		char itemDeque_fromFile[lines][FILEPATH_LEN_MAX], tempStr[FILEPATH_LEN_MAX];
		memset(itemDeque_fromFile, 0, lines * FILEPATH_LEN_MAX * sizeof(char));
		*/
		tempStr = (char*)malloc(FILEPATH_LEN_MAX * sizeof(char));
		if (not tempStr) {
			error(ENOMEM, ENOMEM, "%s：为tempStr(%p)malloc失败", __func__, tempStr);
		}
		memset(tempStr, 0, FILEPATH_LEN_MAX * sizeof(char));

		itemDeque_fromFile = (char**)malloc(lines * sizeof(char*));
		if (not itemDeque_fromFile) {
			error(ENOMEM, ENOMEM, "%s：为itemDeque_fromfile(%p)malloc失败", __func__, itemDeque_fromFile);
		}
		for (uint32_t i = 0; i < lines; i += 1) {
			itemDeque_fromFile[i] = (char*)malloc(FILEPATH_LEN_MAX * sizeof(char));
			if (not itemDeque_fromFile[i]) {
				error(ENOMEM, ENOMEM, "%s：为itemDeque_fromfile[%u](%p)malloc失败", __func__, i, itemDeque_fromFile);
			}

			memset(itemDeque_fromFile[i], '\0', FILEPATH_LEN_MAX * sizeof(char));
		}

		// 上面都是繁琐的申请内存，下面是读取

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
		// puts("由于文件目录条目(pak.h: PAK_Item)中，前2个uint32_t数的意义尚未摸清，目前不能实现打包逻辑");
		archive(&args);
		break;
	}
	default: {
		error(EINVAL, EINVAL, "-t -c -x 必须指定其中之一");
		break;
	}
	}

	if (tempStr) {
		free(tempStr);
		tempStr = NULL;
	}
	if (itemDeque_fromFile) {
		for (uint32_t l = 0; l < lines; l += 1) {
			if (itemDeque_fromFile[l]) {
				free(itemDeque_fromFile[l]);
				itemDeque_fromFile[l] = NULL;
			}
		}
		free(itemDeque_fromFile);
		itemDeque_fromFile = NULL;
	}
	if (itemList) {
		Deque_clear(itemList);
		free(itemList);
		itemList = NULL;
	}

	return 0;
}
