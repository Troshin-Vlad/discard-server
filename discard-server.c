#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLR_ERR "\033[31m"
#define CLR_OK	"\033[32m"
#define CLR_DEF	"\033[0m"

int sock_d;
int connect_d;

void error(const char*);
void closed_socket();
void bind_socket(int,struct sockaddr_in);
int open_socket();
int connecting(int,struct sockaddr_storage*);
void server_shutdown(int);

int main(int argc, char *argv[]){

	if(signal(SIGINT, server_shutdown) == SIG_ERR)
		error("cat't catch signal(SIGINT)");

	sock_d = open_socket();

	int pid;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind_socket(sock_d, addr);

	if(listen(sock_d, 10) == -1)
		error("setup queue listen on socket");

	struct sockaddr_storage client;

	while(1){

		connect_d = connecting(sock_d, &client);

		if( (pid = fork()) == 0 ){

			while(1){
				shutdown(connect_d, SHUT_RD);
			}

			exit(0);
		}
		close(connect_d);
	}

	closed_socket();
	return 0;
}

void server_shutdown(int sig){
	closed_socket();
	exit(0);
}

int connecting(int sock,struct sockaddr_storage *client){

	unsigned int size = sizeof(client);
	int con = accept(sock, (struct sockaddr*)&client, &size);
	if(con == -1)
		error("connecting");

	return con;
}

void closed_socket(){
	if(sock_d)
		close(sock_d);
	if(connect_d)
		close(connect_d);
}

void bind_socket(int sock, struct sockaddr_in addr){
	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		error("bind socket with port");
}

int open_socket(){
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock_d == -1)
		error("open socket");

	int reuse = -1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) == -1)
		error("setup option reuse port");

	return sock;
}

void error(const char *msg){
	fprintf(stderr, "[%serror%s]: %s\n", CLR_ERR, CLR_DEF, msg);
	exit(0);
}