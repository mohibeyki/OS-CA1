#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "util.h"

#define BUFFER_SIZE 128
#define TRUE 1
#define FALSE 0

main (int argc, char *argv[]) {

	// if (argc != 3) {
	// 	writeErr("Error: Use ./client clientName directory\n");
	// 	exit(1);
	// }

	int	len, rc;
	int	sockfd;
	char*	recv_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE);
	struct	sockaddr_in	addr;
	char* 	DIR_ADDR = argv[1];
	char*	CLIENT_NAME = argv[2];
	char*	SERVER_IP = (char*) malloc(sizeof(char));
	char*	SERVER_PORT = (char*) malloc(sizeof(char));
	char*	buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
	fd_set	master_set, working_set;
	int		max_sd, i, IS_ALIVE = TRUE;

	writeLine("Command me:\n");
	buffer = readLine();
	
	FD_ZERO(&master_set);
	max_sd = 0;
	FD_SET(0, &master_set);

	do {
		memcpy(&working_set, &master_set, sizeof(master_set));

		printf("Waiting on select()...\n");
		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);

		if (rc < 0) {
			perror("  select() failed");
			break;
		}

		if (rc == 0) {
			printf("  select() timed out.  End program.\n");
			break;
		}

		int desc_ready = rc;
		for (i = 0; i <= max_sd  &&  desc_ready > 0 && IS_ALIVE; ++i) {
			if (FD_ISSET(i, &working_set)) {
				desc_ready -= 1;

				if (i == 0) {
					memset(buffer, 0, BUFFER_SIZE);
					scanf("%s", buffer);
					if (strcmp(buffer, "quit") == 0) {
						IS_ALIVE = FALSE;
						break;
					}
				} else {
					break;
					/*
					   printf("  Server socket is readable\n", i);
					   close_conn = FALSE;
					   rc = recv(i, buffer, sizeof(buffer), 0);
					   if (rc < 0)
					   perror("  recv() failed");

					   if (rc == 0) {
					   printf("  Connection closed\n");
					   close_conn = TRUE;
					   }

					   len = rc;
					   printf("  received : \"%s\"\n", buffer);

					   rc = send(i, buffer, len, 0);
					   if (rc < 0) {
					   perror("  send() failed");
					   close_conn = TRUE;
					   }

					   if (close_conn) {
					   close(i);
					   FD_CLR(i, &master_set);
					   if (i == max_sd) {
					   while (FD_ISSET(max_sd, &master_set) == FALSE)
					   max_sd -= 1;
					   }
					   }
					 */
				}
			}
		}

	} while (IS_ALIVE);

	/*while (strcmp(buffer, "quit") != 0) {

		int index = indexOf(buffer, ' ');
		if (index == -1) {
			writeErr("Error: illigal command");
			exit(1);
		}

		char* address = (char*) malloc(sizeof(char));
		strcpy(address, buffer + index + 1);

		int colonIndex = indexOf(buffer + index + 1, ':');

		strncpy(SERVER_IP, buffer + index + 1, colonIndex);
		strcpy(SERVER_PORT, buffer + index + 1 + colonIndex + 1);

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)	{
			writeErr("Error: Socket cannot be established");
			exit(-1);
		}

		memset(&addr, 0, sizeof(addr));
		addr.sin_family      = AF_INET;
		inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
		addr.sin_port        = htons(atoi(SERVER_PORT));

		rc = connect(sockfd,
				(struct sockaddr *)&addr,
				sizeof(struct sockaddr_in));

		if (rc < 0) {
			writeErr("Error: Unable to connect");
			close(sockfd);
			exit(-1);
		}
		
		while(strcmp(buffer, "quit") != 0) {

			writeLine("Enter message to be sent:\n");
			buffer = readLine();

			if (strcmp(buffer, "quit") == 0)
				break;

			len = send(sockfd, buffer, strlen(buffer) + 1, 0);
			if (len != strlen(buffer) + 1) {
				writeErr("send");
				close(sockfd);
				exit(-1);
			}

			len = recv(sockfd, recv_buf, sizeof(char) * BUFFER_SIZE, 0);
			writeLine("Server responded with: \"");
			writeLine(recv_buf);
			writeLine("\" \n");
			writeLine(itoa(len));
			writeLine("\n");
		}
		close(sockfd);
		if (strcmp(buffer, "quit") != 0) {
			writeLine("Command me:\n");
			buffer = readLine();
		}
	}*/
}

