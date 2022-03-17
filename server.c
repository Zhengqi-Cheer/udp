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


#define server_port  88
#define server_IP 192.168.0.128

#define fd_num 64
#define BUFFER_SIZE 1024

int cheat (int sock_fd,struct sockaddr_in addr_client);
int recv_file(int recv_sock,struct sockaddr_in addr_client);


int main ()
{
	
	int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd < 0 )
	{	
		perror("sock_fd error\n");
		return -1;
	}
	else 
		printf("sock_fd ok\n");
	struct sockaddr_in addr_server;
	struct sockaddr_in addr_client;
//套接字和结构体addr_server绑定
	int len;
	memset (&addr_server,0,sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(server_port);
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	int addr_len = sizeof(addr_server);

	if(bind(sock_fd,(struct sockaddr *)&addr_server,sizeof(addr_server)) < 0)
	{
		perror("sock_fd bind error\n");
		return -1;
	}
	else
		printf("sock_fd bind ok\n");
/*
//文件套接字和结构体绑定
	int recvfile_sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd < 0 )
	{	
		perror("recvfile_sock error\n");
		return -1;
	}
	else 
		printf("recvfile_sock ok\n");
	struct sockaddr_in addr_serverfile;
	memset (&addr_server,0,sizeof(addr_serverfile));
	addr_serverfile.sin_family = AF_INET;
	addr_serverfile.sin_port = htons(server_port);
	addr_serverfile.sin_addr.s_addr = htonl(INADDR_ANY);
	//int addr_len = sizeof(addr_serverfile);
	if(bind(recvfile_sock,(struct sockaddr *)&addr_serverfile,sizeof(addr_serverfile)) < 0)
	{
		perror("recvfile_sock bind error\n");
		return -1;
	}
	else
		printf("recvfile_sock bind ok\n");
*/

	
	
//接收

	int array[fd_num] = {-1};	//描述符集合
	
	fd_set read_set;		//	
	fd_set write_set;
	FD_ZERO(&read_set);	//清空集合
	FD_ZERO(&write_set);	//清空集合
	
	FD_SET(sock_fd,&read_set);//

	for(int i = 0;i<fd_num;++i) //对 array[]进行初始化
	{
		array[i]=-1;
	}
	
	array[0]= sock_fd;

	struct timeval timeout = {1,0};//超时时间
	
	printf("服务器准备就绪\n");
	
	while(1)
	{
		int MAX_FD=-1;
		for (int i = 0; i < fd_num; ++i) //取最大值
			{
			if(array[i]>0)
				FD_SET(array[i],&read_set);
				FD_SET(array[i],&write_set);	
				MAX_FD = MAX_FD > array[i] ? MAX_FD : array[i];
			}
		int result = select(MAX_FD+1,&read_set,&write_set,NULL,NULL);
		switch(result)
		{
			case -1 :
				perror("select 错误：");
				break;
			case 0 :
				printf("select timeout...\n");
				break;
			default:
			{
				//检测那个描述符有变化
				int i=0;
				for (i = 0; i < fd_num; ++i)
					{
					if (array[i]>0&&FD_ISSET(array[i],&read_set)&&FD_ISSET(array[i],&write_set))
						{
							recv_file(array[i],addr_client);
							
						}
					else if (array[i]>0 && FD_ISSET(array[i],&write_set))
						{
						cheat (array[i],addr_client);

						}
					}

			}
		
		

		}

	}


	close(sock_fd);
	return 0;	
}

int cheat (int sock_fd,struct sockaddr_in addr_client)
{
	
	char recv_buf[256];
	char send_buf[256];
	int recv,send;
	
	int len = sizeof(addr_client);
	//收
		recv = recvfrom(sock_fd,recv_buf,sizeof(recv_buf),0,(struct sockaddr *) &addr_client,&len);
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
			
		send = sendto(sock_fd,send_buf,sizeof(send_buf),0,(struct sockaddr* )& addr_client,len);
			
		if(send < 0)
		{
			perror("send error");
			return -1;
		
		}


}


int recv_file(int recv_sock,struct sockaddr_in addr_client)
{
	
	int addr_len = sizeof(addr_client);
	char filename[100];
	char filepath[100];

	char buffer[BUFFER_SIZE] ;//开文件缓存空间
	bzero(buffer,BUFFER_SIZE);

	FILE *fp;
	
	printf("等待中。。。\n");

	memset(filename,'\0',sizeof(filename));
	memset(filepath,'\0',sizeof(filepath));

//接收地址和名字
	int recv_n;
	recv_n =recvfrom(recv_sock,filepath,100,0,(struct sockaddr *) &addr_client,&addr_len);
	if(recv_n < 0)
		{
		perror ("recv error\n");
		return -1;
	}
	else 
	{
	printf("recv ok\n");
	printf("filepath:%s\n",filepath);//打印路径
	
	int i=0,k=0;  
		for(i=strlen(filepath);i>=0;i--)  
		{  
			if(filepath[i]!='/')	  
			{  
				k++;  
			}  
			else   
				break;	  
		}  
	strcpy(filename,filepath+(strlen(filepath)-k)+1);
	}
	printf("文件名字filename:%s\n",filename);
	char server_filepath[100]= "/home/zhengquan/share/git/udp/";
	
	strncat(server_filepath,filename,strlen(filename));

	fp = fopen(filename,"w+");
	if(fp == NULL)
	{
		perror("file open error");
		return -2;
	}
	else
	{	printf("open file ok");
		int times = 1;
		int fileTrans;
		int writelength;
	//收数据
		while (fileTrans = recvfrom(recv_sock,buffer,BUFFER_SIZE,0,(struct sockaddr *)&addr_client,&addr_len))
		{

			
			times++;
			writelength = fwrite(buffer,sizeof(char),fileTrans,fp);
			if(writelength <= 0)
				printf("没有就收到数据\n");
			else
			{
				printf("writelength:%d\n",writelength);
				if (fileTrans < BUFFER_SIZE)
				{
					printf("接收完成\n");
					break;
				}
				
			}	
				printf("写入成功\n");
		}
		printf("recv finished!\n");
	}
		
	fclose(fp);


}




