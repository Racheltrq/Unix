/*
	Only support command with one >, < or >>. 
*/

#include <stdio.h>   // printf, fgets
#include <string.h>  // strlen
#include <unistd.h>  // execlp
#include <stdbool.h> // true
#include <sys/wait.h>// wait
#include <stdlib.h>  //exit
#include <fcntl.h>


#define MAX_ARG 20


int parse_arg_to_builtin(char **arg, int arg_count){
	if (strcmp(strtok(arg[0], "\n"), "cd") == 0) {
		char error_msg[strlen(strtok(arg[arg_count-1], "\n")) + 3];
		strcat(error_msg, "cd: ");
		strcat(error_msg, strtok(arg[arg_count-1], "\n"));
		if (chdir(strtok(arg[arg_count-1], "\n")) == -1) perror(error_msg);
		return 0;
	}

	if (strcmp(strtok(arg[0], "\n"), "exit") == 0) {
		exit(0);
	}
	return 1; //not built in command
}

void redirect_io(char *token, char *file, char **arg) {
	if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0) {
		//printf("in > >>\n");
		int fd;
		if (strcmp(token, ">") == 0) fd = open(strtok(file, "\n"), O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (strcmp(token, ">>") == 0) fd = open(strtok(file, "\n"), O_WRONLY | O_CREAT | O_APPEND, 0666);
		if (fd < 0) {
			fprintf(stderr, "Could not open %s.\n", file);
			exit(1);
		}
		if(dup2(fd, 1) < 0) perror("dup output to fd failed");
		close(fd);

	}
	if (strcmp(token, "<") == 0) {
		//printf("in < \n");
		int fd;
		fd = open(strtok(file, "\n"), O_RDONLY);
		printf("%d\n", fd);
		if (fd < 0) {
			fprintf(stderr, "Could not open %s.\n", file);
			exit(1);
		}
		if(dup2(fd, 0) < 0) perror("dup output to fd failed");
		close(fd);
	}
}

int main() 
{		
	while(true) {
		printf(":^) ");
		char command[80];
		fgets(command, 80, stdin);

		char *arg[MAX_ARG];
		char *token = strtok(command, " ");
		int arg_count = 0;
		bool IO_redirection = false;
		char *redirection_token;
		char *file;
		while (token != NULL) {
				if (strcmp(token, ">") == 0 || strcmp(token, "<") == 0 || strcmp(token, ">>") == 0) {
					IO_redirection = true;
					redirection_token = token;
					file = strtok(NULL, " ");
					break;
				}
				else{
				arg[arg_count] = token;
				token = strtok(NULL, " ");
				arg_count++;
				}
		}
		if (parse_arg_to_builtin(arg, arg_count) == 0) continue; 



		arg[arg_count] = NULL; //add Null terminator at the end of the arg list

		pid_t fork_pid = fork();
		if (fork_pid > 0) wait(NULL);
		if (fork_pid == 0) {
				if (IO_redirection == true) {
					redirect_io(redirection_token, file, arg);
					strtok(arg[arg_count-1], "\n");

				}
				strtok(arg[arg_count-1], "\n");
				execvp(*arg, arg);
				perror("Failed to exec");
		}

	}
}
