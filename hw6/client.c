#include <stdlib.h>         // exit
#include <stdio.h>          // printf, fprintf, stderr
#include <unistd.h>         // read
#include <string.h>         // memset
//#include <strings.h>        // bzero
#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM
#include <arpa/inet.h>      // inet_pton
#include <netinet/in.h>     // servaddr
#include <sys/select.h>

#define	MAXLINE		4096	/* max text line length */
#define	MAX_MSG_LEN	1024	/* buffer size for reads and writes */
#define PORT 9999
#define CLIENT_NAME "Client"

int main() {
	int sockfd, n;
    //char recvline[MAXLINE + 1];
    struct sockaddr_in	servaddr;
    char * ip = "127.0.0.1";

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for: %s\n", ip);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    fd_set readfds;

	FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    FD_SET(0, &readfds); 

    //char buff[MAX_MSG_LEN];
    for (;;) {
        select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        for (int fd = 0; fd < FD_SETSIZE; fd++) {
        	if (FD_ISSET(fd, &readfds)) {
        		printf("fd:%d\n", fd);
        		if (fd > 0) { //read from server
					char buffer[MAX_MSG_LEN + 1];
					memset(buffer, 0, sizeof(buffer));
					if (read(fd, buffer, MAX_MSG_LEN) < 0) {
						perror("Error: Failed to read from server"); // implies server might be dead or terminating us
						FD_CLR(fd, &readfds);
						close(fd);
						exit(1);
					}
					printf("%s", buffer);
				}

				else if (fd == 0) { //write to server
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
					if (write(sockfd, buffer, strlen(buffer)) < 0) {
						perror("Error: Failed to write to server"); 
					}
				}


        	}

        }
    }

    close(sockfd);
    exit(0);
}



