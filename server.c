#
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define server_port  6262
#define server_IP 192.168.0.128

int main ()
{
	
	int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd < 0 )
	{	
		perror("sock_fd error");
		return -1;
	}
	
	struct sockaddr_in addr_server;
//套接字和结构体addr_server绑定
	int len;
	memset (&addr_server,0,sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(server_port);

	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock_fd,(struct sockaddr *)&addr_server,sizeof(addr_server)) < 0)
	{
		perror("bind error");
		return -1;
	
	}
//接收
	char recv_buf[256];
	char send_buf[256];
	int recv,send;

	struct sockaddr_in addr_client;

	while(1)
	{
		printf("服务器准备就绪\n");
//收
		recv = recvfrom(sock_fd,recv_buf,sizeof(recv_buf),0,
				(struct sockaddr *) &addr_client,(socklen_t *) &len);
		if(recv < 0)
		{
			perror("recvfrom error");
			return -1;
		}
		
		recv_buf[recv] = '\0';
		printf("客户端发送%d bytes：%s\n",recv,recv_buf);
	
		
//发	
		printf("发送:");
		scanf("%s",send_buf);
		
		send = sendto(sock_fd,send_buf,sizeof(send_buf),0,
				(struct sockaddr* )& addr_client,len);
		
		if(send < 0)
		{
			perror("send error");
			return -1;
		
		}
	}
	close(sock_fd);
	return 0;	
}



