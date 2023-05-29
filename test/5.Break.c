extern void printi(int val);
extern void printd(double val);
extern void printch(char val);
extern void println();

int main(){
          int i = 0;
          while (i < 10)
          {
                    if (i == 7) break;
                    else printch('a' + i);
                    i = i + 1;
          }
          for(int i = 0; i < 10; i = i + 1){
                    if (i == 7) break;
                    else printch('a' + i);    
          }
          println();
          return 0;
}