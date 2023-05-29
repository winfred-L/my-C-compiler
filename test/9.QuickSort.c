extern void printi(int val);
extern void println();
extern int geti();

int main() {
    int n;
    int arr[100];
    int stack[100];  // 用栈模拟递归
    int left, right, top, temp;
    n = geti();
    for (int i = 0; i < n; i = i + 1) {
        arr[i] = geti();
    }
    for (int i = 0; i < n; i = i + 1) {
        printi(arr[i]);
    }
    println();
    // 初始化栈
    top = -1;
    left = 0;
    right = n - 1;
    top = top + 1;
    stack[top] = left;
    top = top + 1;
    stack[top] = right;

    while (top >= 0) {
        // 取出栈顶元素
        right = stack[top];
        top = top - 1;
        left = stack[top];
        top = top - 1;
        // partition
        int pivot = arr[right];
        int i = left - 1;
        for (int j = left; j < right; j = j + 1) {
            if (arr[j] <= pivot) {
                i = i + 1;
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        temp = arr[i + 1];
        arr[i + 1] = arr[right];
        arr[right] = temp;
        int pivotIndex = i + 1;
        // 再次将子区间压入栈
        if (pivotIndex - 1 > left) {
            top = top + 1;
            stack[top] = left;
            top = top + 1;
            stack[top] = pivotIndex - 1;
        }
        if (pivotIndex + 1 < right) {
            top = top + 1;
            stack[top] = pivotIndex + 1;
            top = top + 1;
            stack[top] = right;
        }
    }
    for (int i = 0; i < n; i = i + 1) {
        printi(arr[i]);
    }
    println();
    return 0;
}