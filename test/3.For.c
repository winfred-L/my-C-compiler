extern void printi(int val);
extern void printd(double val);
extern void printch(char val);
extern void println();

int main(){
          for(int i = 0; i < 10; i = i + 1){
                    if(i == 3) continue;
                    else if (i == 7) break;
                    else printch('a' + i);    
          }
          println();
          return 0;
}