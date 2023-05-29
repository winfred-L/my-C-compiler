extern void printi(int val);
extern void printd(double val);
extern void printch(char val);
extern void println();

int fun1(int c){
          return c;
}

char fun2(int c, char b){
          return c + b;
}

int main(){
          printi(fun1(22));
          printch(fun2(2, 'c'));
          println();
          return 0;
}