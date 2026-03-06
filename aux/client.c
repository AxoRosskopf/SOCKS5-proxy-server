#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RCV_BUF_SIZE 32

int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in serverAddress;
	unsigned short serverPort;
	char *serverIP;
	char *content;
	char contentBuffer[RCV_BUF_SIZE];
	unsigned int lenghtContent;
	int bytesRcvd, totalBytesRcvd;

	if((argc < 3)||(argc > 4)){
		fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
		exit(1);
	}

	serverIP = argv[1];
	content = argv[2];

	if(argc == 4){
		serverPort = atoi(argv[3]);
	} else {
		serverPort = 7;
	}

	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))< 0){
		printf("%s\n", "Error instantiating socket function");
		exit(1);
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family 		= AF_INET;
	serverAddress.sin_addr.s_addr 	= inet_addr(serverIP);
	serverAddress.sin_port 			= htons(serverPort);

	if(connect(sock, (struct sockaddr *)&serverAddress,sizeof(serverAddress)) < 0){
		printf("%s\n", "Error instantiating connect function");
		close(sock);
		exit(1);
	}

	lenghtContent = strlen(content);

	if(send(sock, content, lenghtContent, 0) != lenghtContent){
		printf("%s\n","send() sent a different number of bytes than expected");
	}

	totalBytesRcvd = 0;
	printf("Received: ");
	while(totalBytesRcvd < lenghtContent){
		if((bytesRcvd = recv(sock, contentBuffer, RCV_BUF_SIZE - 1,0)) <= 0 ){
			printf("%s\n","recv() failed or connection closed prematurely");
			break;
		}
		totalBytesRcvd += bytesRcvd;
		contentBuffer[bytesRcvd] = '\0';
		printf("%s",contentBuffer);
	}

	printf("\n");
	close(sock);
	exit(0);
}
