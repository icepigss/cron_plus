#include "ps_cron.h"

void
build_tcp_server(int pfd[])
{
    int sock; 
    struct sockaddr_in server_socket;
    struct sockaddr_in socket_in;


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        _exit(PS_FAILURE);
    }
    bzero(&server_socket, sizeof(server_socket));
    server_socket.sin_family=AF_INET;
    server_socket.sin_addr.s_addr=htonl(INADDR_ANY);
    server_socket.sin_port=htons(PS_PORT);
    if (bind(sock, (struct sockaddr*)&server_socket, sizeof(struct sockaddr_in)) < 0) {
        close(sock);
        _exit(PS_FAILURE);
    }
    if (listen(sock, PS_BACKLOG) < 0) {
        close(sock);
        _exit(PS_FAILURE);
    }
    while (PS_TRUE) {
        socklen_t len = 0;
        char buf[PS_MAXLINE];
        char buf_ip[INET_ADDRSTRLEN];
        int client_sock = accept(sock, (struct sockaddr*)&socket_in, &len);

        if (client_sock < 0) {
            _exit(PS_FAILURE);
        }
		if (DebugFlags == 1) {
			fprintf(stdout, "new request %d \n", client_sock);
		}

        memset(buf_ip, '\0', sizeof(buf_ip));
        inet_ntop(AF_INET, &socket_in.sin_addr, buf_ip, sizeof(buf_ip));


        memset(buf, '\0', sizeof(buf));
        read(client_sock, buf, sizeof(buf));
        write(pfd[1], buf, PS_MAXLINE);
        memset(buf, '\0', sizeof(buf));
        write(client_sock, PS_RET_OK, strlen(PS_RET_OK)+1);
		close(client_sock);
    }
    close(sock);
}
