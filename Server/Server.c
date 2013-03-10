#include "../Common/Consts/Consts.h"
#include "../Common/Utilities/StringUtil.h"
#include "../Common/Objects/DataObjects.h"
#include "../Common/Utilities/FileUtil.h"

#define SERVER_PORT  12345

Client clients[CLIENT_SIZE];
int clientCount;

int init() {
	clientCount = 0;
	int i = 0;
	for (; i < CLIENT_SIZE; i++)
		resetClient(&clients[i]);
	int listen_sd, on = 1;
	struct sockaddr_in addr;
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		writeErr("socket() failed");
		return -1;
	}

	int rc2 = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
			sizeof(on));
	if (rc2 < 0) {
		writeErr("setsockopt() failed");
		close(listen_sd);
		return -1;
	}

	rc2 = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc2 < 0) {
		writeErr("ioctl() failed");
		close(listen_sd);
		return -1;
	}
	println("AHA1");
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY );
	addr.sin_port = htons(SERVER_PORT);
	rc2 = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
	if (rc2 < 0) {
		println("bind() failed");
		close(listen_sd);
		return -1;
	}
	println("AHA2");

	rc2 = listen(listen_sd, 5);
	if (rc2 < 0) {
		writeErr("listen() failed");
		close(listen_sd);
		return -1;
	}
	return listen_sd;
}

int isRCValid(int rc) {
	if (rc < 0)
		writeErr("  select() failed");
	if (rc == 0)
		writeErr("  select() timed out.  End program.");
	return rc;
}

int do_send(char* line, int sockfd) {
	int len = send(sockfd, line, strlen(line) + 1, 0);
	if (len != strlen(line) + 1) {
		writeErr("Error is send");
		return -1;
	}
	return 0;
}

int do_recieve(char* output, int sockfd) {
	memset(output, 0, sizeof(char) * BUFFER_SIZE);
	return recv(sockfd, output, sizeof(char) * BUFFER_SIZE, 0);
}

int do_command(char* line, int num) {
	int index = 0;
	char next[BUFFER_SIZE];
	char send_buf[BUFFER_SIZE] = "";
	char recv_buf[BUFFER_SIZE] = "";
	memset(next, 0, BUFFER_SIZE);
	index = nextToken(line, next, index);
	strcpy(line, line + index);
	index = 0;
	int k = -1;
	println("next token is '%s'", next);
	if (strcmp(next, "quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "get-clients-list") == 0) {
		char message[BUFFER_SIZE] = "";
		int q = 0;
		for (; q < clientCount; q++) {
			if (clients[q].fd != -1) {
				strcat(message, "cli");
				char tmpInt[BUFFER_SIZE] = "";
				convertIntToString(clients[q].id, tmpInt);
				strcat(message, tmpInt);
				strcat(message, ":");
				strcat(message, clients[q].name);
				strcat(message, "\n");
			}
		}
		do_send(message, clients[num].fd);
		k = 0; // send "get-clients-list"
	} else if (strcmp(next, "share") == 0) {
		char completeName[BUFFER_SIZE] = "__shared__";
		char fileName[BUFFER_SIZE];
		index = nextToken(line, fileName, index);
		strcat(completeName, fileName);
		int myFD = getWRFile(completeName);
		if( myFD < 0) {
			println("I can't make this file %s",recv_buf);
			do_send("Problem:", clients[num].fd);
			return 1;
		}
		do_send("Privilege granted",clients[num].fd);
		do_recieve(recv_buf, clients[num].fd);
		while(strcmp("BAZINGA", recv_buf) != 0) {
			fprint(myFD, recv_buf);
			do_send("OK!", clients[num].fd);
			do_recieve(recv_buf, clients[num].fd);
		}
		do_send("give me Password", clients[num].fd);
		do_recieve(recv_buf, clients[num].fd);
		println("Password: %s", recv_buf);
		do_send("Complete", clients[num].fd);
		println("Receiving file \"%s\" was completed", fileName);
		int q = 0;
		char message[BUFFER_SIZE] = "";
		for (; q < clientCount; q++) {
			if (clients[q].fd != -1) {
				strcat(message, "cli");
				char tmpInt[BUFFER_SIZE] = "";
				convertIntToString(clients[q].id, tmpInt);
				strcat(message, tmpInt);
				strcat(message, ":");
				strcat(message, clients[q].name);
				strcat(message, "\n");
			}
		}
		print(message);
		close(myFD);

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
//		int len = send(sockfd, line, strlen(line) + 1, 0);
//		if (len != strlen(line) + 1) {
//			writeErr("Error is send\n");
//			return -1;
//		}
//
//		char recv_buf[BUFFER_SIZE];
//		len = do_recieve(recv_buf);
//		print("  received '%s'\n", recv_buf);
	} else if (strcmp(next, "dc") == 0)
		k = 8; // share
	return k;
}

int getNum(int fd) {
	int q = 0;
	for (; q < clientCount; q++)
		if (clients[q].fd == fd)
			return q;
	return -1;
}

int main(int argc, char *argv[]) {

	int i, len, rc;
	int desc_ready, end_server = FALSE;
	int close_conn;
	char buffer[BUFFER_SIZE];
	fd_set master_set, working_set;
	int max_sd, new_sd;

	int listen_sd = init();
	if (listen_sd == -1)
		return -1;
	println("list");
	FD_ZERO(&master_set);
	println("listening");
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);
	do {
		memcpy(&working_set, &master_set, sizeof(master_set));
		println("waiting for listen...");
		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL );
		if (isRCValid(rc) <= 0)
			break;
		desc_ready = rc;
		for (i = 0; i <= max_sd && desc_ready > 0; ++i) {
			if (FD_ISSET(i, &working_set)) {
				desc_ready--;
				if (i == listen_sd) {
					print("  Listening socket is readable\n");
					//DO_ACCEPT
					/**********************/
					new_sd = accept(listen_sd, NULL, NULL );
					if (new_sd < 0) {
						if (errno != EWOULDBLOCK) {
							writeErr("  accept() failed");
							end_server = TRUE;
						}
						break;
					}
					print("  New incoming connection: %d\n", new_sd);
					rc = recv(new_sd, buffer, sizeof(buffer), 0);
					int q = 0;
					int flag = 0;
					for (; q < clientCount; q++) {
						if (clients[q].fd == -1) {
							flag = 1;
							clients[q].fd = new_sd;
							strcpy(clients[q].name, buffer);
							clients[q].id = q;
							println("cli#%d:%s", q, buffer);
							break;
						}
					}
					if (flag == 0) {
						q = clientCount;
						clients[q].fd = new_sd;
						strcpy(clients[q].name, buffer);
						clients[q].id = q;
						println("cli#%d:%s", q, buffer);
						clientCount++;
					}
					len = convertIntToString(q, buffer);
					rc = send(new_sd, buffer, len + 1, 0);
					println("It has been sent");
					/**********************/
					//END DO_ACCEPT
					FD_SET(new_sd, &master_set);
					if (new_sd > max_sd)
						max_sd = new_sd;

				} else {
					print("  Descriptor %d is readable\n", i);
					close_conn = FALSE;
					rc = recv(i, buffer, sizeof(buffer), 0);
					if (rc < 0)
						writeErr("  recv() failed");

					if (rc == 0) {
						print("  Connection closed\n");

						close_conn = TRUE;
					}

					len = rc;
					print("  received : \"%s\"\n", buffer);
					do_command(buffer, getNum(i));

					if (close_conn) {
						int q = 0;
						int flag = 0;
						for (; q < clientCount; q++) {
							if (clients[q].fd == i) {
								resetClient(&clients[q]);
								break;
							}
						}
						close(i);
						FD_CLR(i, &master_set);
						if (i == max_sd) {
							while (FD_ISSET(max_sd, &master_set) == FALSE)
								max_sd -= 1;
						}
					}
				}
			}
		}
	} while (end_server == FALSE);

	for (i = 0; i <= max_sd; ++i) {
		if (FD_ISSET(i, &master_set))
			close(i);
	}
}
