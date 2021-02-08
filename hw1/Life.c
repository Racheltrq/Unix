/* Author: Ruiqi(Rachel) Tao
	Purpose of the document: Unix HW1 Game Of Life
	Default input filename: life.txt
	Compiled with std=c99
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <errno.h>

extern int errno;


void generate(int rows, int columns, int **arr){
	//create a new 2D array
	int **next_gen = (int**)malloc(sizeof(int*)*rows);
	for(int i=0; i<rows; i++)
        *(next_gen+i) = (int*)malloc(sizeof(int)*columns);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			next_gen[i][j] = '-';
		}
	}

	//Calculate number of neighbours for each cell for the current generation
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			int neighbours = 0;
			if(i > 0 && arr[i - 1][j] == '*') neighbours++; //check if there's a neighbours to the North
			if(i < rows - 1 && arr[i + 1][j] == '*') neighbours++; //South
			if(j > 0 && arr[i][j - 1] == '*') neighbours++; //West
			if(j < columns - 1 && arr[i][j + 1] == '*') neighbours++; //East
			if((i > 0 & j > 0) && arr[i - 1][j - 1] == '*') neighbours++; //Northwest
			if((i > 0 & j < columns - 1) && arr[i - 1][j + 1] == '*') neighbours++; //Southwest
			if((i < rows - 1 & j > 0) && arr[i + 1][j - 1] == '*') neighbours++; //Northeast
			if((i < rows - 1 & j < columns - 1) && arr[i + 1][j + 1] == '*') neighbours++; //Southeast
			if(arr[i][j] == '*' && (neighbours == 2 || neighbours == 3)) next_gen[i][j] = '*';
			if(arr[i][j] == '-' && neighbours == 3) next_gen[i][j] = '*';
		}
	}

	//copy the new generation back
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			arr[i][j] = next_gen[i][j];
		}
	}

	free(next_gen);
	return;
}


int main(int argc, char *argv[]){
	int rows, columns, generations;
	rows = columns = generations = 10;
	char filename[20];
	strcpy(filename, "life.txt");
	//set default values for different number of arguments passed into the program
	if(argc == 2){
		rows = atoi(argv[1]);
	}
	else if(argc == 3){
		rows = atoi(argv[1]);
		columns = atoi(argv[2]);
	}
	else if (argc == 4){
		rows = atoi(argv[1]);
		columns = atoi(argv[2]);
		strcpy(filename, argv[3]);
	}
	else if (argc == 5){
		rows = atoi(argv[1]);
		columns = atoi(argv[2]);
		strcpy(filename, argv[3]);
		generations = atoi(argv[4]);
	}
	else if (argc > 5) {
		printf("Too many arguments...\n");
		exit(EXIT_FAILURE);
	}

	/* Read from input file */
	FILE *fp;
	char *buff;
	fp = fopen(filename, "r");
	if (fp == NULL){
		int errnum = errno;
		fprintf(stderr, "Value of errno: %d\n", errno);
		perror("Error printed by perror");
		fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
		exit(EXIT_FAILURE);
	}
	size_t len = 0;
	ssize_t read;
	//int map[rows][columns];
	int count_row = 0;

	//create a dynamically sized 2D array
	int **map = (int**)malloc(sizeof(int*)*rows);
	for(int i=0; i<rows; i++)
        *(map+i) = (int*)malloc(sizeof(int)*columns);

    //set all cells to dead
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			map[i][j] = '-';
		}
	}

	//change some cells to alive based on input file
	while ((read = getline(&buff, &len, fp)) != -1) {
		for(int i = 0; i < strlen(buff)-1; i++){
			if(buff[i] == '*') {
				map[count_row][i] = '*';
			}
		}
		count_row++;
	}

	fclose(fp);
	free(buff);

	fp = fopen("output.txt", "w");
	int count = 0;
	
	//generate a set a number of epoches
	while(count <= generations){
		char buffer [12];
		sprintf(buffer, "%d", count);
		char *target = malloc(100);
		char *s1 = "Generation ";
		char *s2 = buffer;
		char *s3 = ":\n";
		strcat(target, s1);
		strcat(target, s2);
		strcat(target, s3);
		fputs(target, fp);
		
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns; j++) {
				fputc(map[i][j], fp);
			}
			fputs("\n", fp);
		}
		fputs("================================\n", fp);
		
		generate(rows, columns, map);
		count++;
	}
	fclose(fp);
	free(map);
	return 0;



}

