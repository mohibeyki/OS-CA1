#include "../Common/Consts/Consts.h"
#include "../Common/Utilities/IOUtil.h"
#include "../Common/Utilities/FileUtil.h"
#include "../Common/Utilities/StringUtil.h"

int sockfd = -1;

int init();

int do_connect(char* line) {
	int locIndex = 0;
	char sIP[BUFFER_SIZE];
	memset(sIP, 0, BUFFER_SIZE);
	locIndex = nextTokenDelimiter(line, sIP, 0, ':');
	char sPORT[BUFFER_SIZE];
	memset(sPORT, 0, BUFFER_SIZE);
	nextToken(line, sPORT, locIndex);
	return init(sIP, sPORT);
}

int do_command(char* line) {
	int index = 0;
	char next[BUFFER_SIZE];
	memset(next, 0, BUFFER_SIZE);
	index = nextToken(line, next, index);
	strcpy(line, line + index);
	index = 0;
	int k = -1;
	print("next token is '%s'\n", next);
	if (strcmp(next, "quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "connect") == 0) {
		k = do_connect(line);
	}
	else if (strcmp(next, "get-clients-list") == 0)
		k = 1; // send "get-clients-list"
	else if (strcmp(next, "share") == 0)
		k = 2; // share
	else if (strcmp(next, "get-files-list") == 0)
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
		memset(recv_buf, 0, sizeof(recv_buf));
		len = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
		print("  received '%s'\n", recv_buf);
	}
	else if (strcmp(next, "dc") == 0)
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
		writeErr("Error: Use ./client clientName directory\n");
		return -1;
	}

	int len, rc, k;
	char buffer[BUFFER_SIZE];
	readLine(buffer);

	k = do_command(buffer);
	while (k >= 0) {
		print("Code %d returned!\n", k);
		print("Command me:\n");
		readLine(buffer);
		k = do_command(buffer);
	}
	print("Closing socket and ending\n");
	close(sockfd);
}
