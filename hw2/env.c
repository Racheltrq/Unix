/* Author: Ruiqi(Rachel) Tao
	Purpose of the document: Unix HW2 Implement the utility env
	Compiled with std=c99
	Does not work when executing "./env env" or "./env -i env" becuase env is not in /bin directory
*/


#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

extern char **environ;

int main(int argc, char *argv[]){
	//no argument provided, show current environment variables
	if (argc == 1){
		char **var;
		for (var=environ; *var != NULL; var++){
			puts(*var);
		}
		return 0;
	}

	//cound number of environment variable assigned
	int count_var = 0;
	int start = 1;
	if (strcmp(argv[1],"-i") == 0) start = 2;
	for(int i = start; i < argc; i++){
		if (strstr(argv[i], "=") != NULL){
			count_var += 1;
		}
		else{
			break;
		}
	}
	//printf("count_var:%d\n", count_var);
	//printf("commands start from index %d if there's any\n", start + count_var);




	if (argc > 1){
		//if start with variable assignment
		char **environ_new;
		if (strcmp(argv[1], "-i") == 0){
			char **temp = malloc((count_var + 1) * sizeof(char*));
			for(int i = start; i < start + count_var; i++){
				temp[i - start] = argv[i];
			}
			environ_new = temp;
		}
		else{ //combine the original environment variable list with the new arguments
			int ind = 0;
			while(environ[ind] != NULL){ //count the number of existing variables 
				ind++;
			}
			char **temp = malloc((count_var + ind + 1) * sizeof(char*)); //create a new array for variables
			int temp_ind = 0;

			for(int i = 0; i < ind; i++){ //copy the variables over
				temp[i] = environ[i];
				temp_ind++;
			}
			for (int i = start; i < start + count_var; i++){ // check if the arguments should replace the orginal ones or add to the orginal ones
				int exist = 0;
				char copy_argv[strlen(argv[i])+1];
				strcpy(copy_argv, argv[i]);
				char *var_arg = strtok(copy_argv, "=");
				for (int j = 0; j < ind; j++){
					char copy_environ[strlen(environ[j])+1];
					strcpy(copy_environ, environ[j]);
					char *var = strtok(copy_environ, "=");
					if (strcmp(var, var_arg) == 0){
						exist = 1;
						temp[j] = argv[i];
					}
				}
				if (exist == 0){
					temp[temp_ind] = argv[i];
					temp_ind++;
				}
			}
			environ_new = temp;
		}

		int count_comm = argc - count_var - 1;
		if (strcmp(argv[1], "-i") == 0) count_comm--;

		if (count_comm == 0){
			char **var;
			for (var=environ_new; *var != NULL; var++){
				puts(*var);
			}
			return 0;
		}

		//create an array that stores commands
		char **arr = malloc((count_comm + 1) * sizeof(char*));
		for(int i = start + count_var; i < argc; i++){
			arr[i-start-count_var] = (char*)malloc(strlen(argv[i]) + 1);
			arr[i-start-count_var] = argv[i];
		}
		char dst[255] = "/bin/";
		strcat(dst, arr[0]);
		execve(dst, arr, environ_new); //execute
		free(arr);
	}
	
	
	return 0;
	}

