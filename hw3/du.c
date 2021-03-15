/* Author: Ruiqi(Rachel) Tao
	Purpose of the document: Unix HW3 Implement the utility du with no flags
	To compile, simply run "make".
	To run the program, use "./du" or "./du [dirname]"
	Known bugs: When run ./du on some directory, sometimes there's a malloc error. I'm not sure what is causing the error(might be path length, but I couldn't find the cause). 
				The running result is identical to the du utility on my Ubuntu. When I ran du on MacOs, the blocksize seems to be half as big and empty dir doesn't seems to take blocks. 

*/

#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include <math.h>

#define BLOCKSIZE 1024

int fileExist(int *inode_arr, int count, int inode_num){
	for(int i = 0; i < count; i++){
		if(inode_arr[i] == inode_num) return 0;
	}
	return -1;
}

//
int listDir(char *currdir, int *inode_arr, int count){
	struct dirent *direntp;
	struct stat statBuf;
	int size = 0;
	DIR *dirp = opendir(currdir);
	//if the given directory is a file, then return the file size directly
	lstat(currdir, &statBuf);
	if (!S_ISDIR(statBuf.st_mode)){
		if (S_ISLNK(statBuf.st_mode)) return 0;
		struct stat info;
		stat(currdir, &info);
		//
		if(fileExist(inode_arr, count, info.st_ino) == 0) return 0;
		else{
			int arr_size = sizeof(*inode_arr)/sizeof(int);
			if(arr_size == count){
				int *temp_arr = malloc(count * 2 * sizeof(int));
				inode_arr = temp_arr;
			}
			inode_arr[count] = info.st_ino;
			count += 1;
		}
		//
		size += (int)ceil((double)info.st_size / 4096.0)* 4096 / BLOCKSIZE;
	    return size;
	}
	size += 4; //if it's a directory, add 4 to size
	//iterate through each item in the directory
	while ((direntp = readdir(dirp)) != NULL){

		if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0){
			char *temp = malloc(strlen(currdir)+strlen(direntp->d_name)+1); //allocate memory for new directory name
			strcat(temp, currdir);
			strcat(temp, "/");
			strcat(temp, direntp->d_name); //add new directory to the currrent directory name
			if(lstat(temp, &statBuf) == -1) {
			}
			else{
			}
			if (S_ISDIR(statBuf.st_mode)){
				int dirSize = listDir(temp, inode_arr, count);
				size += dirSize;
				printf("%d\t%s\n", dirSize, temp);

			}
			else if (!S_ISLNK(statBuf.st_mode)) {
				struct stat info;
				stat(temp, &info);
				if(fileExist(inode_arr, count, info.st_ino) == 0) return 0;
				else{
					int arr_size = sizeof(*inode_arr)/sizeof(int);
					if(arr_size == count){
						int *temp_arr = malloc(count * 2 * sizeof(int));
						inode_arr = temp_arr;
					}
					inode_arr[count] = info.st_ino;
					count += 1;
		}
				size += (int)ceil((double)info.st_size / 4096.0)* 4096 / BLOCKSIZE; //calculate block number based on file size and block size
			}
		}
	}
	return size;
}


int main(int argc, char *argv[]){
	int dirSize = 0;
	int *inode_arr = malloc(10 * sizeof(int)); //create an array for storing inode number
	int count = 0;
	if (argc == 1) {
		printf("%d\t%s\n", listDir(".", inode_arr, count), ".");
	}
	else {
		printf("%d\t%s\n", listDir(argv[1], inode_arr, count), argv[1]);
	}

    return 0;
}

