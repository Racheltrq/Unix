/*
	Only support command with a single >, <, >>, or |
*/

#define _POSIX_SOURCE

#include <stdio.h>   // printf, fgets
#include <string.h>  // strlen
#include <unistd.h>  // execlp
#include <stdbool.h> // true
#include <sys/wait.h>// wait
#include <stdlib.h>  //exit
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>


#define MAX_ARG 20

static volatile sig_atomic_t jump_active = 0;
static sigjmp_buf env;

int pipe_fd[2]; //for pipe

void parse_commands(char *arg[], char command[],int * arg_count, bool * IO_redirection, char * redirection_token, char * file, int * isPipe) {
	
	char *token = strtok(command, " ");

	while (token != NULL) {
		//printf("hello token %s", token);
		if (strcmp(token, ">") == 0 || strcmp(token, "<") == 0 || strcmp(token, ">>") == 0) {
			*IO_redirection = true;
			*redirection_token = *token;
			file = strtok(NULL, " ");
			break;
		}
		else if (strcmp(token, "|") == 0){
			*isPipe = 1;
			*redirection_token = *token;
			break;
		}
		else{
			arg[*arg_count] = token;
			token = strtok(NULL, " ");
			*arg_count+=1;
		}
	}
}

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

void redirect_io(char *token, char *file) {
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
	if (strcmp(token, "|") == 0) {
		//printf("creating pipe...\n");
		if (pipe(pipe_fd) < 0){
			perror("create pipe failed");
		}
	}
}

void handler(int signum){
	if (!jump_active) {
        return;
    }
    siglongjmp(env, 3);
}



int main() 
{		
	while(true) {
		printf(":^) ");

		char *arg[MAX_ARG];
		char *arg_pipe[MAX_ARG];
		char command[80];
		int arg_count = 0;
		int arg_count_pipe = 0;
		bool IO_redirection = false;
		char *redirection_token;
		char *file;
		int isPipe = 0;

		char command_copy[80];
		fgets(command, 80, stdin);
		strcpy(command_copy, command);

		if (strcmp(command, "\n") == 0) {
			//printf("no input\n");
			continue;
		}

		struct sigaction act;
	    act.sa_handler = handler;
	    sigemptyset(&act.sa_mask);
	    act.sa_flags = SA_RESTART;
	    sigaction(SIGINT, &act, NULL);
	    sigaction(SIGQUIT, &act, NULL);

    	

		
		parse_commands(arg, command, &arg_count, &IO_redirection, redirection_token, file, &isPipe);

		//printf("isPipe:%d\n", isPipe);

		
		//printf("ARG COUNT: %d\n", arg_count);
		/*
		for(int i=0; i < arg_count; i++){
			//printf("%d...", i);
			if(arg[i] != NULL){
				arg[i][strcspn(arg[i], "\n")] = 0;
				printf("%s\n", arg[i]);
			}
		}
		*/
		

		if (isPipe != 1){
			if (parse_arg_to_builtin(arg, arg_count) == 0) continue; 
		}
		else {
			char *token = strtok(command_copy, "|");
			token = strtok(NULL, "|");
			parse_commands(arg_pipe, token, &arg_count_pipe, &IO_redirection, redirection_token, file, &isPipe);
		}

		for(int i=0; i < arg_count_pipe; i++){
			//printf("%d...", i);
			if(arg_pipe[i] != NULL){
				arg_pipe[i][strcspn(arg_pipe[i], "\n")] = 0;
				//printf("TEST:%s\n", arg_pipe[i]);
			}
		}

		arg[arg_count] = NULL; //add Null terminator at the end of the arg list
		arg_pipe[arg_count_pipe] = NULL;
		//printf("IO_redirection:%d\n", IO_redirection);
		pid_t fork_pid = fork();
		if (fork_pid > 0) wait(NULL);
		if (fork_pid == 0) {
			if (sigsetjmp(env, 1) == 3) {
	            continue;
	        }
	        jump_active = 1;
	        /* test signal by creating an infinite loop
			while(1){
	    		printf("hello\n");
	    		sleep(1);
	    	}
	    	*/
			if (IO_redirection == 0) {
				//printf("redirection_token:%s\n", redirection_token);
				redirect_io(redirection_token, file);
				strtok(arg[arg_count-1], "\n");

			}
			if (isPipe != 1) {
				//printf("executing command!\n");
				execvp(*arg, arg);
				perror("Failed to exec");
			}
			else {
				//printf("DEBUG TEST:\n");
				//printf("pipefd:%d %d\n", pipe_fd[0], pipe_fd[1]);
				int saved_stdout;
				saved_stdout = dup(1);
				if(dup2(pipe_fd[1], 1) < 0) perror("dup output to pipe_fd[1] failed");
				if(dup2(pipe_fd[0], 0) < 0) perror("dup input to pipe_fd[0] failed");
				
				//close(pipe_fd[1]);
				int wstatus;
				fork_pid = fork();
				//printf("fork_pid:%d\n", fork_pid);
				if (fork_pid > 0) {
					//printf("In parent\n");

					wait(&wstatus);
					dup2(saved_stdout, 1);
					close(pipe_fd[1]);
					//printf("After wait:\n");
					
					
					/*
					for(int i=0; i < arg_count; i++){
						//printf("%d...", i);
						if(arg[i] != NULL){
							arg[i][strcspn(arg[i], "\n")] = 0;
							printf("%s\n", arg[i]);
						}
					}
					*/
					//dup2(1, pipe_fd[1]);
					//sleep(5);
					execvp(*arg_pipe, arg_pipe);
					
					perror("Failed to exec");
				}
					
				if (fork_pid == 0) {
					execvp(*arg, arg);
					perror("Failed to exec");
				}
				if (fork_pid < 0) printf("negative\n");
				
			}
		}

	}
}
