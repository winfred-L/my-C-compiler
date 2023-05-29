#include <cstdio>

extern "C"
void printi(int val)
{
    printf("%d ", val);
}

extern "C"
void println()
{
    printf("\n");
}



extern "C"
void printd(double val)
{
    printf("%lf\n", val);
}

extern "C"
void printch(char val)
{
    printf("%c\n", val);
}

extern "C"
int geti()
{
    int ret = 0;
    scanf("%d",&ret);
    return ret;
}

extern "C"
double getd()
{
    double ret = 0;
    scanf("%lf",&ret);
    return ret;
}

extern "C"
char getch()
{
    char ret = 0;
    scanf("%c",&ret);
    return ret;
}
