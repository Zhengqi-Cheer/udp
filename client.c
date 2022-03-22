#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>#include <sys/stat.h>
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
#include <unistd.h>

#define server_port  88
#define server_IP "192.168.0.128"

#define fd_num 8
#define BUFFER_SIZE 512

int send_message (const int sock_fd,char *date_buf,int alen);
int open_file(const char *filepath, int *array);
int do_recv_date(char *buff);
int send_file_date(int send_sock,int *array);

int  wfp = -1;
int  rfp = -1;	
fd_set read_set;

//struct sockaddr_in addr_client;

struct udp_date_head{
	char type ;
	char file_num ;
	char filename[16];
};

struct udp_date_head date_head;
struct sockaddr_in addr_server;


int main(void)
{
	int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd == -1 ) {	
		perror("sock_fd error\n");
		return -1;
	}
	else {
		printf("sock_fd ok:%d\n",sock_fd);
	}
	
	memset (&addr_server,0,sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(server_port);
	addr_server.sin_addr.s_addr = inet_addr(server_IP);
	int addr_len = sizeof(addr_server);
	
//接收	
			
	fd_set write_set;
	FD_ZERO(&read_set);	//清空集合
	FD_ZERO(&write_set);	//清空集合
	
	FD_SET(sock_fd,&read_set);
	FD_SET(0,&read_set);
	
	int array[fd_num] ;	//描述符集合
	memset(array,-1,sizeof(array));
	
	array[0]= sock_fd;
	array[1]= 0;

	printf("服务器准备就绪\n");
	
	while(1)
	{
		int MAX_FD=-1;
		for (int i = 0; i < fd_num; ++i) //取最大值
			{
				if(array[i]>0){
					MAX_FD = MAX_FD > array[i] ? MAX_FD : array[i];
				}	
			}
		int result = select(MAX_FD+1,&read_set,&write_set,NULL,NULL);
		switch(result)
		{
			case -1 :
				perror("select error：");
				break;
			case 0 :
				printf("select timeout...\n");
				break;
			default:{
				for(int i=0;i<fd_num;++i)
				{//收
					////发
					if(FD_ISSET(rfp,&read_set)){
						//printf("0\n");
						send_file_date(sock_fd,array);
					}
					
					if(FD_ISSET(0,&read_set)){
						char buff[256];
						read(0,buff,sizeof(buff));
						if(buff[0] == '/'){
							open_file(buff,array);//开文件，加文件描述符
						}
						send_message(sock_fd,buff,sizeof(addr_server));	
						memset(buff,0,sizeof(buff));
					}	
					
					if (FD_ISSET(sock_fd,&write_set)){
						printf("1");
						char buff[512];
						int recv_n = recvfrom(sock_fd,buff,sizeof(buff),0,(struct sockaddr *)&addr_server,&addr_len);
						
						if(recv_n == -1){
							perror ("recv_n error\n");
							return -1;
						}
						
						else {
						printf("recv_n ok\n");
						}
						do_recv_date(buff);
						
						memset(buff,0,sizeof(buff));
					}	
					
					
					
				}
			}
		}

	}
	close(sock_fd);
	return 0;	
}

int send_message (const int sock_fd,char *date_buf,int slen)
{
	date_head.type = 1;
	
	int send;	
	//发	
		//printf("发送:");		
		char send_buf[sizeof(date_head)+BUFFER_SIZE] = {0};
		memset(send_buf,0,sizeof(send_buf));
		memcpy(send_buf,&date_head,sizeof(date_head));
		memcpy(send_buf+sizeof(date_head),date_buf,sizeof(date_buf));

		send = sendto(sock_fd,send_buf,sizeof(send_buf),0,(struct sockaddr*)&addr_server,slen);	
		if(send < 0)
		{
			perror("send error");
			return -1;		
		}
		else 
			printf("send ok\n");
}

//打开发送的文件
int open_file(const char *filepath, int *array)
{
	//int addr_len = sizeof(addr_client);
	printf("filepath:%s\n",filepath);//打印路径
	printf("filepath长度：%ld\n",strlen(filepath));
	
	int k=0;  
	for(int i = strlen(filepath) ; i >= 0; i--)  {  
		if(filepath[i]!='/')	  {  
			k++;  
		}  
		else   
		break;	  
	} 
	rfp = open("./1.txt",O_RDWR);
	if(rfp == -1){
		perror("file open error");
		close(rfp);
		return -2;
	}
	else{
		//read_flag =!read_flag;
		printf("open file ok:%d \n",rfp);
	}
	
	FD_SET(rfp,&read_set);
	for(int i = 0; i < fd_num;i++){  //加入文件描述符到集合	
		if (*(array + i) == -1) {
			*(array + i )= rfp ;
			break;
		}
	}	
	char filename[k+1];
	strcpy(filename,filepath+(strlen(filepath)-k)+1);	//取名字
	strcpy(date_head.filename,filename);
	
	return 0;
}


int send_file_date(const int send_sock,int *array)
{
	
	date_head.type = 0;//wenjian 
	strcpy(date_head.filename , "1.txt");
	
	char read_buff[BUFFER_SIZE] = {0};
	//memset(buff,0,sizeof(buff));

	//int rfp = open(path,O_RDONLY);
	lseek(rfp,date_head.file_num*BUFFER_SIZE,SEEK_SET);
	int read_num = read(rfp,read_buff,BUFFER_SIZE);
	if(read_num == -1){
		perror("read error:");
	}
	
	else {
		printf("read ok :%d byte\n",read_num);
		int addr_len = sizeof(addr_server);
		char send_buf[sizeof(date_head)+BUFFER_SIZE] = {0};
		memset(send_buf,0,sizeof(send_buf));
		memcpy(send_buf,&date_head,sizeof(date_head));
		memcpy(send_buf+sizeof(date_head),read_buff,sizeof(read_buff));
		
		int send_num = sendto(send_sock,send_buf,sizeof(send_buf),0,(struct sockaddr*)&addr_server,addr_len);
		if(send_num == -1){
			perror("send file  date error:");
		}
		
		else {
			printf("send file date %d byte\n:",send_num);
			date_head.file_num ++;
		}
		//if(read_num < (BUFFER_SIZE+sizeof(struct udp_date_head))){
		if(read_num < BUFFER_SIZE){
			printf("send file over\n");
			FD_CLR(rfp,&read_set);
			close(rfp);
			for(int i = 0;i < fd_num; i++){
				if(*(array + 1) == rfp ){
					*(array + 1) = -1;	
					bzero(&date_head,sizeof(date_head));
					break;
				}			
			}		
		}
	}	
}
//文件（0）or消息（1）/编号（8位）/名字(16)/   18字节 

int do_recv_date(char *buff)
{
	char *p = buff;
	//片段消息还是文件
	if(*p == 0){
		printf("收到消息：%s\n",p+18);
	}
	//文件
	else {
	char path[256] = "/home/zhengquan/share/git/udp/";
	char name[16] = {0};
	memcpy(name,buff+1,16);
	strcat(path,name);
	int wfp = open(path,O_WRONLY | O_CREAT ); //开文件
	lseek(rfp,(*p+1)*(BUFFER_SIZE),SEEK_SET);
	int writelength = write(wfp,p+18,strlen(buff)-18);
	printf("写入：%d byte\n",writelength);
	if(writelength == -1){
		perror("write error");
	}

	else {
		printf("writelength:%d\n",writelength);
		if (writelength < BUFFER_SIZE && writelength>=0) {
			printf("接收文件完成\n");
			}    
		}					
	}
	close(wfp);	//关闭文件    	
}
#include<unistd.h>
#include<linux/if_ether.h>

#define server_port 88
#define server_ip "192.168.0.129"


#define fd_num 64
#define BUFFER_SIZE 1024

int send_file(int file_sock,struct sockaddr_in server_addr,char flie_path[256]);
int   cheat (int client_sock,struct sockaddr_in server_addr,char recv_buf[256]);



int  main ()
{
//创建聊天套接字client_sock
	int client_sock;
	if( (client_sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror ("sock error");
		return -1;
	
	}
	else printf("sock ok\n");
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
								cheat (array[i],server_addr,buff);
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



int send_file(int file_sock,struct sockaddr_in server_addr,char flie_path[256])
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

int  cheat (int client_sock,struct sockaddr_in server_addr,char recv_buf[256])
{
	int send,recv;
	int addr_len = sizeof(server_addr);	
	char send_buf[256];
	//char recv_buf[256];
	//发	
	//printf("发送：");
	//scanf("%s",send_buf);
	
	send = sendto(client_sock,send_buf,strlen(send_buf),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
	if(send < 0)
	{
		perror("发送未完成");
		return -1;
		
	}
	else 
		printf("发送完成");
	//收
	recv = recvfrom(client_sock, recv_buf, strlen(recv_buf), 0, (struct sockaddr *)&server_addr, &addr_len);	   
	if(recv < 0)  
	{  
		perror("recvfrom error:");
		return -1;	
	}
	printf("接收到%d bytes；%s\n",recv,recv_buf);
	close (client_sock);

}



