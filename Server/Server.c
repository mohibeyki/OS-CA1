#include "../Common/Consts/Consts.h"

#define SERVER_PORT  12345

int main (int argc, char *argv[]) {
	int    i, len, rc, on = 1;
	int    listen_sd, max_sd, new_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;
	char   buffer[BUFFER_SIZE];
	struct sockaddr_in	addr;
	fd_set master_set, working_set;

	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		writeErr("socket() failed");
		return -1;
	}

	rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
			(char *)&on, sizeof(on));
	if (rc < 0) {
		writeErr("setsockopt() failed");
		close(listen_sd);
		return -1;
	}

	rc = ioctl(listen_sd, FIONBIO, (char *)&on);
	if (rc < 0) {
		writeErr("ioctl() failed");
		close(listen_sd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(SERVER_PORT);
	rc = bind(listen_sd,
			(struct sockaddr *)&addr, sizeof(addr));
	if (rc < 0) {
		writeErr("bind() failed");
		close(listen_sd);
		return -1;
	}

	rc = listen(listen_sd, 5);
	if (rc < 0) {
		writeErr("listen() failed");
		close(listen_sd);
		return -1;
	}

	FD_ZERO(&master_set);
	println("listening");
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);

	do {
		memcpy(&working_set, &master_set, sizeof(master_set));
		println("waiting for listen...");
		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);

		if (rc < 0) {
			writeErr("  select() failed");
			break;
		}

		if (rc == 0) {
			writeErr("  select() timed out.  End program.");
			break;
		}
		
		desc_ready = rc;
		for (i=0; i <= max_sd  &&  desc_ready > 0; ++i) {
			if (FD_ISSET(i, &working_set)) {
				desc_ready -= 1;

				if (i == listen_sd) {
					print("  Listening socket is readable\n");
					do {
						new_sd = accept(listen_sd, NULL, NULL);
						if (new_sd < 0) {
							if (errno != EWOULDBLOCK) {
								writeErr("  accept() failed");
								end_server = TRUE;
							}
							break;
						}

						print("  New incoming connection - %d\n", new_sd);
						FD_SET(new_sd, &master_set);
						if (new_sd > max_sd)
							max_sd = new_sd;

					} while (new_sd != -1);
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

					rc = send(i, buffer, len, 0);
					if (rc < 0) {
						writeErr("  send() failed");
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
				}
			}
		}

	} while (end_server == FALSE);

	for (i = 0; i <= max_sd; ++i) {
		if (FD_ISSET(i, &master_set))
			close(i);
	}
}
