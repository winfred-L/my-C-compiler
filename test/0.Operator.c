extern void printi(int val);
extern void printd(double val);
extern void printch(char val);
extern void println();

int add(int a, int b)
{
    return a + b;
}

int main()
{
    int a = -2;    
    int b = 2;
    char c = 'a';    
    int d = add(a, b); 
    double e = 3.0;
    //单目运算
    int unaryMinus = -b;
    //逻辑运算
    int logicOR = a || b;
    int logicAnd = a && b;
    int logicNot = !(a >= b);
    //比较运算
    int logicGT = (a > b);
    int logicLE = (a <= b);
    //算术运算
    int add = a + b;
    int sub = a - b;
    int mul = a * b;
    int div = a / b;
    int mod = a % b;
    
    printi(unaryMinus);
    printi(logicOR);
    printi(logicAnd);
    printi(logicNot);
    printi(logicGT);
    printi(logicLE);
    printi(add);
    printi(sub);
    printi(mul);
    printi(div);
    printi(mod);

    println();
    return 0;
}
