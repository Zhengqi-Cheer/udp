#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<linux/if_ether.h>

#define server_port 88
#define server_ip "192.168.0.128"

int  main ()
{
//创建套接字client_sock
	int client_sock;
	if( (client_sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror ("sock error");
		return -1;
	
	}
//设置address
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	int addr_len = sizeof(server_addr);	
//收发准备

	int send,recv;
	char send_buf[256];
	char recv_buf[256];
	//发	
	printf("发送：");
	scanf("%s",send_buf);

	send = sendto(client_sock,send_buf,sizeof(send_buf),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
	if(send < 0)
	{
		perror("发送未完成");
		return -1;
	
	}
	else 
		printf("发送完成");
	//收
	recv = recvfrom(client_sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&server_addr, &addr_len);       
	if(recv < 0)  
	{  
	    	perror("recvfrom error:");  
		return -1;  
	}
	printf("接收到%d bytes；%s\n",recv,recv_buf);
	close (client_sock);


}


