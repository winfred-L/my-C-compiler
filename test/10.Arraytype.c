extern void printi(int val);
extern void println();

int main() {
    int arr[100];
    arr[0] = 1;
    arr[1] = 1; 

    for(int i = 2; i < 20; i = i + 1){
          arr[i] = arr[i - 1] + arr[i - 2];
          printi(arr[i]);
    }
    println();
    return 0;
}