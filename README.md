# my-C-compiler

ZJU编译原理大程2023

## 运行环境

操作系统：Linux Ubuntu 22.04.1 LTS

软件依赖：
* flex 2.6.4
* bison 3.8.2
* GNU Make 4.3
* llvm 14.0.0
* clang 14.0.0

**注意：输入的源文件使用LF行尾格式，CRLF会有token识别问题**

```bash
# 安装命令
sudo apt update
sudo apt upgrade
sudo apt install flex  # flex -V
sudo apt install bison # bison -V
sudo apt install make
sudo apt install llvm  # llvm-config --version
sudo apt install clang # clang -v
```

## 运行方式

```bash
cd src
# 编译
make all
# 自动测试
make test FILE=test.c
# 链接IO函数(主要是用于输入)
make link OBJFILE=a.o
```

## 工程结构

```
.
├── README.md
├── doc
│   └── useful_ref.md   # 参考链接
├── src
│   ├── Makefile
│   ├── codegen.cpp     # IR代码生成
│   ├── codegen.h
│   ├── main.cpp        # 顶层
│   ├── native.cpp      # 包装好的IO函数
│   ├── node.h          # AST节点定义
│   ├── parser.y        # 语法分析
│   ├── tokens.l        # 词法分析
│   ├── utils.cpp       # 工具函数
│   └── utils.h
└── test
    └── ...             # 测试用例
```

## 参考链接

[useful_ref](./doc/useful_ref.md)
