#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "util.h"

#define BUFFER_SIZE 128

main (int argc, char *argv[]) {

	if (argc != 3) {
		writeErr("Error: Use ./client clientName directory\n");
		exit(1);
	}

	int	len, rc;
	int	sockfd;
	char*	send_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE);
	char*	recv_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE);
	struct	sockaddr_in	addr;
	char* 	DIR_ADDR = argv[1];
	char*	CLIENT_NAME = argv[2];
	char* SERVER_IP = (char*) malloc(sizeof(char));
	char* SERVER_PORT = (char*) malloc(sizeof(char));

	writeLine("Command me:\n");
	char* cmd = readLine();

	while (strcmp(cmd, "quit") != 0) {

		int index = indexOf(cmd, ' ');
		if (index == -1) {
			writeErr("Error: illigal command");
			exit(1);
		}

		char* address = (char*) malloc(sizeof(char));
		strcpy(address, cmd + index + 1);

		int colonIndex = indexOf(cmd + index + 1, ':');

		strncpy(SERVER_IP, cmd + index + 1, colonIndex);
		strcpy(SERVER_PORT, cmd + index + 1 + colonIndex + 1);

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

		while(strcmp(send_buf, "quit") != 0) {

			writeLine("Enter message to be sent:\n");
			send_buf = readLine();

			len = send(sockfd, send_buf, strlen(send_buf) + 1, 0);
			if (len != strlen(send_buf) + 1) {
				writeErr("send");
				close(sockfd);
				exit(-1);
			}

			len = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
			if (len != strlen(send_buf) + 1) {
				writeErr("Error: Size mismatch");
				close(sockfd);
				exit(-1);
			}
		}
		close(sockfd);
		writeLine("Command me:\n");
		cmd = readLine();
	}
}
