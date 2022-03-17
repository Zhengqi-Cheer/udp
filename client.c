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


#define fd_num 64
#define BUFFER_SIZE 1024

int send_file(int file_sock,struct sockaddr_in server_addr,char flie_path[100]);
int   cheat (int client_sock,struct sockaddr_in server_addr);



int  main ()
{
//创建聊天套接字client_sock
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

//创建文件套接字
	int file_sock;
	if( (file_sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror ("flie_sock error");
		return -1;
	
	}
	else printf("flie_sock ok\n");


		//cheat(client_sock,server_addr);//聊天
	//send_file(file_sock,server_addr);//传文件
	
//收发准备
	int array[fd_num] = {-1};	//描述符集合
		
		fd_set read_set;		//	
		fd_set write_set;
		FD_ZERO(&read_set); //清空集合
		FD_ZERO(&write_set);	//清空集合
		
		FD_SET(client_sock,&read_set);//
	
		for(int i = 0;i<fd_num;++i) //对 array[]进行初始化
		{
			array[i]=-1;
		}
		
		array[0]= client_sock;
		array[1]= file_sock;
	
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
							char buff[100];
							scanf("%s",buff);
							if(buff[0] == '/')	
								send_file(array[i],server_addr, buff);
							else	
								cheat (array[i],server_addr);
							}
						/*
						else if (array[i]>0 && FD_ISSET(array[i],&write_set))
							{
							cheat (array[i],server_addr);
	
							}
							*/
						}
	
				}
			
			
	
			}
	
		}





}



int send_file(int file_sock,struct sockaddr_in server_addr,char flie_path[100])
{
	//char flie_path[100];// ="/home/zehngquan/share/git/udp/1.txt";
	int addr_len = sizeof(server_addr); 

	FILE *fp;
	char *buffer;

	buffer = malloc (sizeof(char)*BUFFER_SIZE);
	bzero(buffer,BUFFER_SIZE);

	//printf("输入要发送的文件地址：");//发送的文件地址
	//scanf("%s",flie_path);

	fp = fopen(flie_path,"r");//只读
	if(fp == NULL)
	{
		perror("open file error\n");
		return -1;
	}
	else 
		
		printf("open file ok !\n");	

//发送
	int file_n = sendto(file_sock,flie_path,strlen(flie_path),0,(struct sockaddr *)&server_addr,addr_len) ;
	if(file_n < 0)
	{
		perror("send file_path error\n");
		return -2;
	}
	else 
	printf("sending file_path ok >>>>>>>>>>>>\n");

	printf("开始发送文件数据\n");

	int times = 1;
	int fileTrans;
	while ((fileTrans = fread(buffer,sizeof(char),BUFFER_SIZE,fp)) > 0 )
	{
		times++;
		printf("发送的数据是：%s",buffer);
		if((sendto(file_sock,buffer,fileTrans,0,(struct sockaddr *)&server_addr,addr_len)) <0 )
		{
			perror("send date error\n");
			return -3;
		}
		else 
		printf("send date ok,num%d\n",times);
		
	}
	fclose(fp);//关闭文件
	return 0;
}

int  cheat (int client_sock,struct sockaddr_in server_addr)
{
	int send,recv;
	int addr_len = sizeof(server_addr);	
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



