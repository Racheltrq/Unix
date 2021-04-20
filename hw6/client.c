#include <stdlib.h>         // exit
#include <stdio.h>          // printf, fprintf, stderr
#include <unistd.h>         // read
#include <string.h>         // memset
//#include <strings.h>        // bzero
#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM
#include <arpa/inet.h>      // inet_pton
#include <netinet/in.h>     // servaddr
#include <sys/select.h>
#include <ctype.h>		 //isdigit

#define	MAXLINE		4096	/* max text line length */
#define	MAX_MSG_LEN	1024	/* buffer size for reads and writes */
#define PORT_NUM 9998
#define CLIENT_NAME "Client"

void parse_args(int argc, char** argv, int* port_num, char** client_name, char* ip, int* nameSet);
int is_num(char* string);

int main(int argc, char** argv) {
	int sockfd, n;
    //char recvline[MAXLINE + 1];
    struct sockaddr_in	servaddr;
    char * ip = "127.0.0.1";
    int port_num = PORT_NUM;
    char* client_name = CLIENT_NAME;

    int nameSet = 0;
    parse_args(argc, argv, &port_num, &client_name, ip, &nameSet);
	//printf("client_name:%s port_num:%d\n", client_name, port_num);
    if (nameSet != 1) {
    	printf("Please give the client a name.\n");
    	exit(1);
    }

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(port_num);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for: %s\n", ip);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    fd_set myfds;
    fd_set tmpfds;
	FD_ZERO(&myfds);
    FD_SET(sockfd, &myfds);
    FD_SET(0, &myfds); 

    //char buff[MAX_MSG_LEN];
    for (;;) {
    	tmpfds = myfds;
        select(FD_SETSIZE, &tmpfds, NULL, NULL, NULL);
        for (int fd = 0; fd < FD_SETSIZE; fd++) {
        	if (FD_ISSET(fd, &tmpfds)) {
        		//printf("fd:%d\n", fd);
        		if (fd > 0) { //read from server
        			//printf("reading from server...\n");
					char buffer[MAX_MSG_LEN + 1];
					memset(buffer, 0, sizeof(buffer));
					if (read(fd, buffer, MAX_MSG_LEN) < 0) {
						perror("Error: Failed to read from server"); // implies server might be dead or terminating us
						FD_CLR(fd, &myfds);
						close(fd);
						exit(1);
					}
					printf("%s", buffer);
				}

				else if (fd == 0) { //write to server
					//printf("writing to server...\n");
					char buffer[MAX_MSG_LEN + 1];
					memset(buffer, 0, sizeof(buffer));
					if (fgets(buffer, MAX_MSG_LEN, stdin) == NULL) {
						perror("Error: Failed to read stdin");
						exit(1);
					}
					if (!strcmp(buffer, "exit\n")) {
						puts("Bye");
						exit(0);
					}
					printf("client_name:%s port_num:%d\n", client_name, port_num);
					char msg[MAX_MSG_LEN + 8];
					strcpy(msg, client_name);
					strcat(msg, ":");
					strcat(msg, buffer);
					if (write(sockfd, msg, strlen(msg)) < 0) {
						perror("Error: Failed to write to server"); 
					}
					printf("%s", msg);
				}


        	}

        }
    }

    close(sockfd);
    exit(0);
}

int is_num(char* string) {
	for (int i = 0; i < strlen(string); i++)
		if (isdigit(string[i]) == 0) return 0;
    return 1;
}

void parse_args(int argc, char** argv, int* port_num, char** client_name, char* ip, int* nameSet) {
	if (argc < 3) {
		printf("Usage: ./client [-n name] [-p port] [-H hostip]\n");
		exit(1);
	}

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-p")) {
			if(is_num(argv[i+1])) *port_num = atoi(argv[i+1]);;
			//printf("lolnum client_name:%s port_num:%d\n", *client_name, *port_num);
		}
		if (!strcmp(argv[i], "-n")) {
			*client_name = argv[i+1];
			//printf("lolnam client_name:%s port_num:%d\n", *client_name, *port_num);
			*nameSet = 1;
		}
		if (!strcmp(argv[i], "-H")) {
			ip = argv[i+1];
		}
	}
	//printf("lollol client_name:%s port_num:%d\n", *client_name, *port_num);
}



