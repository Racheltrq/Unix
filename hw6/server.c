#include <stdio.h>       // perror, snprintf
#include <unistd.h>      // close, write
#include <string.h>      // strlen
#include <strings.h>     // bzero
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM,
                         // bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <pthread.h>
#include <stdlib.h>      // exit
#include <ctype.h>		 //isdigit

#define PORT_NUM 9998
#define	LISTENQ	100
#define MAX_MSG_LEN 1024
#define SERVER_NAME "Server"


void parse_args(int argc, char** argv, int* port_num, char** server_name, int* nameSet);
int is_num(char* string);
void read_from_client(int connfd);
void send_from_server(int connfd);
void create_chat(int connfd);

int main(int argc, char** argv) {
	int					listenfd, connfd;
    struct sockaddr_in  servaddr;
    int port_num = PORT_NUM;
    char* server_name = SERVER_NAME;

    int nameSet = 0;
    parse_args(argc, argv, &port_num, &server_name, &nameSet);
    //printf("server_name:%s port_num:%d\n", server_name, port_num);
    if (nameSet != 1) {
    	printf("Please give the server a name.\n");
    	exit(1);
    }
    

	listenfd = socket(AF_INET, SOCK_STREAM, 0); //IPv4 TCP
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port_num);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	for (;;) {
		while((connfd = accept(listenfd, NULL, NULL)) > 0) {
			printf("A client just connected to the server!\n");

			fd_set myfds;
			fd_set newfds;
			FD_ZERO(&myfds);
		    FD_SET(connfd, &myfds);
		    FD_SET(0, &myfds); 

	    //char buff[MAX_MSG_LEN];
	    for (;;) {
	    	newfds = myfds;
	        select(FD_SETSIZE, &newfds, NULL, NULL, NULL);
	        for (int fd = 0; fd < FD_SETSIZE; fd++) {
	        	if (FD_ISSET(fd, &newfds)) {
	        		//printf("fd:%d\n", fd);
	        		if (fd > 0) { //read from server
	        			//printf("reading from client...\n");
						char buffer[MAX_MSG_LEN + 1];
						memset(buffer, 0, sizeof(buffer));
						if (read(fd, buffer, MAX_MSG_LEN) < 0) {
							perror("Error: Failed to read from server");
							FD_CLR(fd, &myfds);
							close(fd);
							exit(1);
						}
						printf("%s", buffer);
					}

					else if (fd == 0) { //write to server
						//printf("writing to client...\n");
						char buffer[MAX_MSG_LEN + 1];
						memset(buffer, 0, sizeof(buffer));
						if (fgets(buffer, MAX_MSG_LEN, stdin) == NULL) {
							perror("Error: Failed to read stdin");
							exit(1);
						}
						if (!strcmp(buffer, "exit\n")) {
							puts("Disconnecting...");
							exit(0);
						}
						char msg[MAX_MSG_LEN + 8];
						strcpy(msg, server_name);
						strcat(msg, ":");
						strcat(msg, buffer);
						if (write(connfd, msg, strlen(msg)) < 0) {
							perror("Error: Failed to write to server"); 
						}
						printf("%s", msg);
					}
	        	}
		    }
	    }

			
		}
		close(connfd);
	}
}

int is_num(char* string) {
	for (int i = 0; i < strlen(string); i++)
		if (isdigit(string[i]) == 0)
		return 0;
    return 1;
}

void parse_args(int argc, char** argv, int* port_num, char** server_name, int* nameSet) {
	//printf("argc:%d\n", argc);
	
	if (argc < 3) {
		printf("Usage: ./server [-n name] [-p port]\n");
		exit(1);
	}
	for (int i = 1; i < argc; i++) {
		if ((!strcmp(argv[i], "-p")) && (i < argc - 1)) {
			if(is_num(argv[i+1])) *port_num = atoi(argv[i+1]);;
			//printf("lolnum client_name:%s port_num:%d\n", *client_name, *port_num);
		}
		if ((!strcmp(argv[i], "-n") && (i < argc - 1))) {
			*server_name = argv[i+1];
			//printf("lolnam client_name:%s port_num:%d\n", *client_name, *port_num);
			*nameSet = 1;
		}
	}
}

/*
void read_from_client(int connfd) {
	char buff[MAX_MSG_LEN];
	while(1) {
		bzero(buff, MAX_MSG_LEN);
		read(connfd, buff, sizeof(buff));

		if (buff[0] != '\0') printf("Read from client: %s\n", buff);
	}
}

void send_from_server(int connfd) {
	printf("connfd:%d\n", connfd);
	char buff[MAX_MSG_LEN];
	while(1) {
		bzero(buff, MAX_MSG_LEN);
		int input_count = 0;
		//int fd_new = 4;
		//dup2(fd_new, 1);
		printf("debug\n");
		while ((buff[input_count++] = getchar()) != '\n');
		if (strcmp("exit", buff) == 0) break;
		char msg[MAX_MSG_LEN + 8] = "Server:";
		strcat(msg, buff);
		write(connfd, msg, strlen(msg));
		printf("%s\n", msg);
	}
}
*/





