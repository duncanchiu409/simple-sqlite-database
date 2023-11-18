#include<stdlib.h>
#include<stdio.h>

int main(){
    int arr[4] = {1,2,3,4};
    int item = 1;
    int i = 0;
    for(i = 3; i >= 1; i--){
        if(item == arr[i]){
            printf("Found");
        }
    }
    printf("%d\n", i);
    return 0;
}