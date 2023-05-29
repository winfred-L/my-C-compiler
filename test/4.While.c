extern void printi(int val);
extern void printd(double val);
extern void printch(char val);
extern void println();

int main(){
          int i = 0;
          while (i < 10)
          {
                    if(i == 3){
                              i = i + 1;
                              continue;
                    } 
                    else if (i == 7) break;
                    else printch('a' + i);
                    /* code */
                    i = i + 1;
          }
          println();
          return 0;
}