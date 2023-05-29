extern void printi(int val);
extern void println();
extern int geti();


int main(){
          int nums[100];
          int n;
          n = geti();
          for(int i = 0; i < n; i = i + 1){
                    nums[i] = geti();
          }
          for(int i = 0; i < n; i = i + 1){
            if((nums[i] >= 1) && (nums[i] <= n) && (i != nums[i] - 1)){
                int t = nums[i] - 1;
                if(nums[i] != nums[t]){
                    int temp = nums[i];
                    nums[i] = nums[t];
                    nums[t] = temp;
                    i = i - 1;
                }    
            }
        }
        int flag = 0;
        for(int i = 0; i < n; i = i + 1){
            if(nums[i] != (i + 1)){
                    printi(i + 1);
                    flag = 1;
                    break;
            } 
        }
        if(flag == 0) printi(n + 1);
        println();
        return 0;
}