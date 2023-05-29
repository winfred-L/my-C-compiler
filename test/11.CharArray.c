extern void printch(char val);
extern void printi(int val);
extern void println();

int main() {
    char c[100];
    c[0] = 'a';
    c[1] = 'b';
    int id = 1;
    while (c[id - 1] + 1 < 'z')
    {
          c[id] = c[id - 1] + 2;
          id = id + 1;
    }
    id = 0;
    while ((c[id] >= 'a') && (c[id] <= 'z'))
    {
          printch(c[id]);
          id = id + 1;
    }
    println();
    return 0;
}