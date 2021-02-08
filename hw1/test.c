
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void helper(int ar[]){
	ar[0] = 0;
	return;
}

int main(){
	int arr1[] = {1, 2, 3};
	helper(arr1);

	for (int i = 0; i < 3; i++){
		printf("%d", arr1[i]);
	}

	printf("\n");
	return 0;
}


