# ms\_pak\_utils

一组针对Metal Slug 3D、6、7、XX资源文件中几种特定“PAK”打包格式的解包、打包工具，及其中出现的一种特有的被称为“PKLZ”的压缩文件的压缩与解压缩工具。

命令行工具，交互操作设计上有意模仿`GNU tar`、`xz`或`zstd`。

OOP写法，日后若有需要，或许还可以据此改为库，扩展别的功能？

## 致谢

礼赞Bilibili UP主：[aikika](https://space.bilibili.com/3405001/)

本仓库各程序的参考资料：

* [PS2《合金弹头3D》汉化发布](https://www.bilibili.com/video/BV1RqqsB9E9r/?share_source=copy_web&vd_source=311fbce1b59d9cdaba8d32f9d2e5bad9)
* [PS2《合金弹头3D》汉化笔记](https://www.bilibili.com/read/cv44077327/)
* [Github repository: Ailyth99/RetroGameLocalization/](https://github.com/Ailyth99/RetroGameLocalization/)

## 工具使用

### pak

一开始只知道这套方法能处理MS3D的PAK文件，后面发现MS6、7中也存在此类文件。程序二进制名保留“ms3d_”前缀，是“以MS3D为代表”之意。

```
Usage: /path2bin/ms3d_pak [-ctvx] [-C[DIR]] [-f[ARCHIVE]] [-m[str]] [-p[str]] [-T[FILE]]
            [--create] [--directory[=DIR]] [--file[=ARCHIVE]] [--magic[=str]]
            [--prefix[=str]] [--list] [--files-from[=FILE]] [--verbose]
            [--extract]

  -c, --create               创建一个新归档
  -C, --directory[=DIR]      改变至目录DIR
  -f, --file[=ARCHIVE]       操作目标PAK ARCHIVE文件
  -m, --magic[=str]
                             如果是新建PAK归档包，则规定文件头的魔术字，限"DATA"
                             "MENU" "FONT" "STRD"，默认"DATA"。
  -p, --prefix[=str]         由于这种PAK包格式连原文件名都不保留，输出的文件名只能以其在包中的相对偏移代替。使用此选项可以在输出文件名的“偏移值”之前加上一个前缀，便于标识
  -t, --list                 列出归档内容
  -T, --files-from[=FILE]    从FILE中获取文件名来解压或创建文件
  -v, --verbose              显示详细信息
  -x, --extract              从归档中解出文件
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

例子：(假设调用程序的指令是`./ms3d_pak`)

```
# 在当前目录下创建a.pak，内容为./dir_a/下的所有文件、文件列表./b.txt中列文件，并显示过程。(文件包中各文件有为对其到16 Byte而补0。)
$ ./ms3d_pak -cvf./a.pak ./dir_a/* --files-from=./b.txt

# 解包./pl01.pak，并将解出的文件放到目录./pl01/下
$ ./ms3d_pak -xf./pl01.pak -C./pl01

# 不解包，查看./pl01.pak中的各文件的序号、所在偏移、文件长
$ ./ms3d_pak --list -f./pl01.pak

# 解包，但只解出其中序号0、7、2、1的文件。同时为所有输出的文件的名字追加一个“pl01-”前缀
$ ./ms3d_pak -xf./pl01.pak 0 7 2 1 --prefix=pl01-
```

### msxx\_pak

```
Usage: /path2bin/msxx_pak [-ctvx] [-C[DIR]] [-f[ARCHIVE]] [-T[FILE]] [--create]
            [--directory[=DIR]] [--file[=ARCHIVE]] [--list]
            [--files-from[=FILE]] [--verbose] [--extract]

  -c, --create               创建一个新归档
  -C, --directory[=DIR]      改变至目录DIR
  -f, --file[=ARCHIVE]       操作目标PAK ARCHIVE文件
  -t, --list                 列出归档内容
  -T, --files-from[=FILE]
                             从FILE中获取文件名来解压或创建文件
  -v, --verbose              显示详细信息
  -x, --extract              从归档中解出文件
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

例：解包`COMMON.PAK`，修改文件后重新打包。(假设调用程序的指令为`./msxx_pak`，`COMMON.PAK`已在当前目录下)

* 解包并生成列表文件：

```
$ mkdir ./COMMON/
$ ./msxx_pak -xvf./COMMON.PAK -C./COMMON
$ ./msxx_pak -tf./COMMON.PAK > ./COMMON.PAK.txt # 这样生成列表文件
```

* 修改完成后：

```
$ mv ./COMMON.PAK ./COMMON.PAK.bak # 为新生成的COMMON.PAK让位
$ ./msxx_pak -cvf./COMMON.PAK -C./COMMON --files-from=./COMMON.PAK.txt # 若仅仅是新增文件，那此文件完全可以不用修改。文件列表中仅有subDir与subID两项是重要的，指定其在包内的subDir与subID的
```

### pklz

```
Usage: /path2bin/pklz [-dvz] [-o FILE] [--decompress] [--out-file=FILE] [--verbose]
            [--compress]

  -d, --decompress
                             解压缩。未指定传入文件时，会从stdin读取数据
  -o, --out-file=FILE        指定输出文件为FILE(必须)
  -v, --verbose              显示详细信息
  -z, --compress
                             压缩。未指定传入文件时，会从stdin读取数据
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

例子：(假设调用程序的指令是`./pklz`)

```
# 解压./sl02.pklz，将解压到当前目录下”sl02.dff“文件中
$ ./pklz -d ./sl02.dff.pklz

# 从./CCC.tar.xz中解出ccc文件，通过管道输入到pklz程序中，解压，并输出到./ccc.decompressed文件中。显示信息
$ tar -xf ./CCC.tar.xz ccc --to-stdout | ./pklz -d -o./ccc.decompressed -v

# 将./ddd.txt制成./d/dd.pklz文件(实际上完全未压缩，因为压缩程序没有实现)
$ ./pklz -z ./ddd.txt -o./d/dd.pklz
```

## 编译

Linux下大概率直接`make`就行了。

Windows下，需要`Cygwin`或者`MSYS2`。不过我没有测试编译的条件，没有装有上述环境的Windows电脑……

## 许可证

LGPL >= 3

----

—— wyz\_2015
