extern void printi(int val);
extern void printd(double val);
extern void printch(char val);
extern void println();

int fun1(int c){
          if((c == 1) || (c == 2)) return 1;
          else return fun1(c - 1) + fun1(c - 2);
}

int fun2(int c, int d){
          if(c == 0 && d == 0) return 0;
          if(c > d) {
                    return fun2(c - 1, d) + 1;
          }
          else return fun2(c, d - 1) + 2;
}

int main(){
          printi(fun1(10));
          printi(fun2(10, 4));
          println();
          return 0;
}