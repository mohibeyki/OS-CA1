#include "../Common/Consts/Consts.h"
#include "../Common/Utilities/IOUtil.h"
#include "../Common/Utilities/FileUtil.h"
#include "../Common/Utilities/StringUtil.h"

int sockfd = -1;
int MY_NUMBER = -1;
char MY_NAME[BUFFER_SIZE] = "";
char MY_PATH[BUFFER_SIZE] = "";

int init();

int do_connect(char* line) {
	int locIndex = 0;
	char sIP[BUFFER_SIZE];
	memset(sIP, 0, BUFFER_SIZE);
	locIndex = nextTokenDelimiter(line, sIP, 0, ':');
	char sPORT[BUFFER_SIZE];
	memset(sPORT, 0, BUFFER_SIZE);
	nextToken(line, sPORT, locIndex);
	int rc = init(sIP, sPORT);
	return rc;
}

int do_send(char* line) {
	int len = send(sockfd, line, strlen(line) + 1, 0);
	if (len != strlen(line) + 1) {
		writeErr("Error is send");
		return -1;
	}
	return 0;
}

int do_receive(char* output) {
	memset(output, 0, sizeof(char) * BUFFER_SIZE);
	return recv(sockfd, output, sizeof(char) * BUFFER_SIZE, 0);
}

int send_file(char send_buf[BUFFER_SIZE], int index, char next[BUFFER_SIZE],
		char recv_buf[BUFFER_SIZE], char* line) {
	strcat(send_buf, "share ");
	index = nextToken(line, next, index);
	int myFD = getROFile(next);
	if (myFD == -1) {
		println("This file does not exists");
		return -1;
	}

	char fileName[BUFFER_SIZE] = "";
	char passWord[BUFFER_SIZE] = "";

	index = nextToken(line, passWord, index);
	getFileName(next, fileName);
	strcat(send_buf, fileName);
	int k = do_send(send_buf);
	if (k >= 0) {
		k = do_receive(recv_buf);
		if (strcmp("Privilege granted", recv_buf) == 0) {
			while (fEndOfFile(myFD) == 0) {
				fReadSome(myFD, send_buf, BUFFER_SIZE - 5);
				k = do_send(send_buf);
				if (k < 0) {
					println("Something's wrong");
					return -1;
				}
				k = do_receive(recv_buf);
				if (k >= 0 && strcmp("OK!", recv_buf) != 0) {
					println("interrupt from server");
					return -1;
				}
			}
			k = do_send("BAZINGA");
			if (k < 0) {
				println("I Can't send BAZINGA");
				return -1;
			}
			k = do_receive(recv_buf);
			if (strcmp("give me Password", recv_buf) != 0) {
				println("problem with finalize");
				return -1;
			}
			k = do_send(passWord);
			if (k < 0) {
				println("I Can't send Password");
				return -1;
			}
			k = do_receive(recv_buf);

			if (strcmp("Complete", recv_buf) != 0) {
				println("problem with sending password");
				return -1;
			}
		}
	}
}

int do_command(char* line) {
	int index = 0;
	char next[BUFFER_SIZE] = "";
	char send_buf[BUFFER_SIZE] = "";
	char recv_buf[BUFFER_SIZE] = "";
	memset(next, 0, BUFFER_SIZE);
	index = nextToken(line, next, index);
	int k = 100;
	println("next token is '%s'", next);
	if (strcmp(next, "quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "connect") == 0) {
		k = do_connect(line);
		if (k != -1) {
			do_send(MY_NAME);
			int len = do_receive(recv_buf);
			MY_NUMBER = atoi(recv_buf);
			println("My Number is '%d'", MY_NUMBER);
		}
	} else if (strcmp(next, "get-clients-list") == 0) {
		k = do_send(next);
		if (k >= 0) {
			k = do_receive(recv_buf);
			if (k >= 0) {
				println("----------currently available clients-----------");
				println("%s", recv_buf);
			}
		}
		k = 1;
	} else if (strcmp(next, "share") == 0) {
		send_file(send_buf, index, next, recv_buf, line);
		k = 2; // share
	} else if (strcmp(next, "get-files-list") == 0)
		k = 3; // share
	else if (strcmp(next, "get") == 0)
		k = 4; // share
	else if (strcmp(next, "remove") == 0)
		k = 5; // share
	else if (strcmp(next, "rename") == 0)
		k = 6; // share
	else if (strcmp(next, "msg") == 0) {
		k = 7; // msg
		int len = send(sockfd, line, strlen(line) + 1, 0);
		if (len != strlen(line) + 1) {
			writeErr("Error is send\n");
			return -1;
		}

		char recv_buf[BUFFER_SIZE];
		len = do_receive(recv_buf);
		print("  received '%s'\n", recv_buf);
	} else if (strcmp(next, "dc") == 0)
		k = 8; // share
	return k;
}

int init(char* SERVER_IP, char* SERVER_PORT) {
	if (sockfd > 0)
		return 1;
	int len, rc;
	struct sockaddr_in addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		writeErr("Error: Socket cannot be established");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
	addr.sin_port = htons(atoi(SERVER_PORT));

	rc = connect(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

	if (rc < 0) {
		writeErr("Error: Unable to connect");
		close(sockfd);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		writeErr("Error: Use ./client clientName directory");
		return -1;
	}

	strcpy(MY_NAME, argv[1]);
	strcpy(MY_PATH, argv[2]);

	int 		len, rc, status, max_sd, IS_ALIVE = 1;
	register	int i;
	char		buffer[BUFFER_SIZE];
	fd_set		file_descriptors;

	max_sd = 0;
	FD_ZERO(&file_descriptors);
	FD_SET(STDIN, &file_descriptors);

	do {
		println("Waiting on select()");
		rc = select(max_sd + 1, &file_descriptors, NULL, NULL, NULL);

		if (rc < 0) {
			writeErr("  select() failed");
			break;
		}

		if (rc == 0) {
			writeErr("  select() timed out.  End program.");
			break;
		}

		int desc_ready = rc;
		for (i = 0; i <= max_sd  &&  desc_ready > 0 && IS_ALIVE; ++i) {
			if (FD_ISSET(i, &file_descriptors)) {
				desc_ready -= 1;

				if (i == STDIN) {
					memset(buffer, 0, BUFFER_SIZE);
					readLine(buffer);
					status = do_command(buffer);
					if (status == -1)
						IS_ALIVE = FALSE;
				} else {
					// Socket is hot, should now recv a message from server

					break;
				}
			}
		}
	} while (IS_ALIVE);
	if (sockfd > 0)
		close(sockfd);
}
