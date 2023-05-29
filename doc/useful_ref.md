### flex+bison+llvm 流程介绍

[flex+bison+llvm简易学习心得](https://xymeow.github.io/post/flex+bison+llvm%E7%AE%80%E6%98%93%E5%AD%A6%E4%B9%A0%E5%BF%83%E5%BE%97/)

[llvm相关工具链及使用](https://buaa-se-compiling.github.io/miniSysY-tutorial/pre/llvm_tool_chain.html)
(该文档中的参考文献中有很多资料)

* `clang -S -emit-llvm main.c -o main.ll -O0`: 生成 llvm ir（不开优化）

### flex与bison 教程

[手把手教程-lex与yacc/flex与bison入门](https://blog.csdn.net/weixin_44007632/article/details/108666375)

### llvm IR 教程

[llvm IR 入门指南](https://zhuanlan.zhihu.com/c_1267851596689457152)

[官网 LLVM Language Reference Manual](https://llvm.org/docs/LangRef.html)（仅作最后参考）

### 代码样例

[使用FLEX BISON和LLVM编写自己的编译器](https://coolshell.cn/articles/1547.html)
[(配套github代码)](https://github.com/lsegal/my_toy_compiler)

[llvm官方tutorial](https://llvm.org/docs/tutorial/index.html)
[(14.0.0版)](https://releases.llvm.org/14.0.0/docs/tutorial/index.html)

[类c编译器项目](https://github.com/YJJfish/C-Compiler)

### llvm C++ api

[官网 LLVM Programmer’s Manual](https://llvm.org/docs/ProgrammersManual.html)
[(中文)](https://blog.csdn.net/qq_23599965/article/details/88538590)
（重点在`The Core LLVM Class Hierarchy Reference`一节）