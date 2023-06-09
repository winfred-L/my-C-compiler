## 语言特性

编译器所分析的语法基本参照C语言设计，具体如下：

### 数据类型

支持`bool`,`char`,`int`,`double`,`void`五种基本类型，同时支持建立在这几种类型上的一维数组和指针类型。常量值中，`true`和`false`是`bool`类型，单引号引起的字符是`char`类型，整数字面量是`int`类型，浮点数字面量是`double`类型。

变量声明如`int a;`或`char a = 'a';`。变量赋值使用`=`号。赋值时会根据被赋值的变量类型进行隐式类型转换。不允许同作用域中重复定义变量。

支持基于以上基本类型的指针和一位数组，支持取地址和指针解引用。指针声明和初始化暂时不能同时进行，需要先声明再赋值。

### 运算符

双目运算中，支持算术运算`+`, `-`, `*`, `/`, `%`, 逻辑运算`||`, `&&`, 比较运算`>`, `>=`, `<`, `<=`, `==`, `!=`。单目运算中，支持负号取反`-`和逻辑非`!`。算数运算会将结构值取更大的数据类型，这里`bool < char < int < double`。void类型不允许参与运算。除法`/`和取余`%`只支持整数间运算。

### 控制语句

支持`if`, `for`, `while`,`continue`, `break`关键字。`if`, `for`, `while`的主体可以是花括号括起的语句块，也可以是没有花括号的单语句。`continue`, `break`只能在`for`, `while`结构中使用。允许各结构相互嵌套。

多个if结合的else if也是合法的：

```c
if(...){...}
else if(...){...}
else{...}
```

### 函数

函数声明处必须同时给出定义，语法同C语言，除非使用`extern`关键字调用本文件外的函数。不支持可变长度参数。必须有返回`return`语句，如返回类型为void则使用`return;`。

```c
void f(double a){...; return;}
extern void printi(int val);
```

### 变量作用域

用花括号`{}`括起的语句块，函数体，`if`, `for`, `while`都具有内部变量作用域的效果，可以覆盖外部同名的变量，出了相应作用域则无法访问。

### IO输入输出

提供基本类型的包装函数。比如打印一个整数的`void printi(int val)`函数，获得一个浮点数的`double getd()`函数。如需使用，只要使用`extern`关键字提前声明即可。

###  单文件编译及main函数

我们的编译器只能编译单个文件，所有代码必须位于一个文件中。

同时，我们的编译器要求存在`int main()`函数作为程序入口，且必须在所给定的这个单文件中定义。

### 注释

单行注释由`//`开始，多行注释包含在`/*`与`*/`之间。