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
#include <unistd.h>

#define server_port  88
#define server_IP "172.16.163.157"

#define fd_num 8
#define BUFFER_SIZE 1024

int send_message (const int sock_fd, char *date_buf, int alen);
int  open_file(const char *filepath, int *array, char flag);
int do_recv_date(char *buff);
int send_file_date(int send_sock, int *array);
int recv_from_sock(int sock_fd, int *array);
int creat_udp_socket();

int wfp = -1;
int rfp = -1;
int sock_fd = -1;
fd_set read_set;

//文件（0）or消息（1）/编号（8位）/date_len/名字(16)/   18字节 
struct udp_date_head{
	int  type ; 
	int  file_num ;
	int  date_len;
	char filename[16];
	char file_date[BUFFER_SIZE];
};

struct udp_date_head date_head;
struct sockaddr_in addr_server;

int main(void)
{
    if (creat_udp_socket() == -1) {
        exit -1;
    }
    //接收				
    fd_set write_set;
    FD_ZERO(&read_set);	//清空集合
    FD_ZERO(&write_set);	//清空集合

    int array[fd_num] ;	//描述符集合
    memset(array, -1,sizeof(array));

    array[0]= sock_fd;
    array[1]= 0;
    printf("客户端准备就绪\n");
    while (1) {
        int MAX_FD=-1;
        for (int i = 0; i < fd_num; ++i) //取最大值
        {
            if (array[i] >= 0) {
                FD_SET(array[i], &read_set);
                FD_SET(array[i], &write_set);
                MAX_FD = MAX_FD > array[i] ? MAX_FD : array[i];
            }	
        }

        int result = select(MAX_FD+1, &read_set,&write_set,NULL,0);
        switch(result)
        {
            case -1 :
                perror("select error：");
                break;
            case 0 :
                printf("select timeout...\n");
                break;
            default:
                if (FD_ISSET(rfp, &read_set)) {
                    send_file_date(sock_fd, array);
                }

                if (FD_ISSET(0, &read_set)) {
                    char buff[BUFFER_SIZE];
                    memset (buff, 0,sizeof(buff));
                    read(0, buff,sizeof(buff));
                    if (buff[0] == '/') {
                        open_file(buff, array,0);//开文件，加文件描述符
                    }
                    send_message(sock_fd, buff,sizeof(addr_server));	
                    memset(buff, 0,sizeof(buff));
                }	

                if (FD_ISSET(sock_fd, &read_set)) {
                    recv_from_sock(sock_fd, array);
                }
        }
    }
    close(sock_fd);
    return 0;
}



int creat_udp_socket()
{
    sock_fd = socket(AF_INET, SOCK_DGRAM,0);
    if (sock_fd == -1 ) {	
        perror("sock_fd error\n");
        return -1;
    }

    (void)memset (&addr_server, 0,sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(server_port);
    addr_server.sin_addr.s_addr = inet_addr(server_IP);
    return sock_fd;
}



int recv_from_sock(int sock_fd, int *array)
{
    int addr_len_1  = sizeof(addr_server);
    char buff[sizeof(date_head)] = {'\0'};
    int recv_n = recvfrom(sock_fd, buff, sizeof(buff), 0, (struct sockaddr*) &addr_server, &addr_len_1);
    if (recv_n == -1) {
        perror ("recv_n error\n");
        return -1;
    } else {
        printf("recv_n ok:%d byte\n", recv_n);
        if (buff[0] == 1) {
            char message[BUFFER_SIZE];
            memset(message, 0, BUFFER_SIZE);
            memcpy(message, 
            buff+sizeof(date_head.type)+sizeof(date_head.file_num)+sizeof(date_head.date_len)+sizeof(date_head.filename), sizeof(message));
            if (message[0] == '/') {
                printf("file\n");
                open_file(buff, array, 1)	;
            }
            printf("收到消息：%s\n", message);
        } else {
            do_recv_date(buff);
        }

        memset(buff, 0,sizeof(buff));
    }
}



int send_message (const int sock_fd, char *date_buf,int slen)
{
    date_head.type = 1;
    memset(date_head.file_date, 0,sizeof(date_head.file_date));
    stpcpy(date_head.file_date, date_buf);	
    char send_buf[sizeof(date_head)] = {'\0'};
    memset(send_buf, 0, sizeof(send_buf));
    memcpy(send_buf, &date_head, sizeof(date_head));
    int send;	
    send = sendto(sock_fd, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&addr_server, slen);	
    if (send < 0) {
        perror("send error");
        return -1;		
    } else {
        printf("send ok\n");
    }

    return 0;
}

//打开发送的文件
int open_file(const char *filepath, int *array,char flag)
{
    switch (flag) 
    {
        case 0: {
            //printf("filepath:%s\n", filepath);//打印路径
            //printf("filepath长度：%ld\n", strlen(filepath));
            int k=0;  
            for (int i = strlen(filepath) ; i >= 0; i--)  {  
                if (filepath[i]!='/')	  {  
                    k++;  
                } else {  
                    break;	  
                } 
            }
            rfp = open("./1.txt", O_RDWR);
            if (rfp == -1) {
                perror("file open error");
                close(rfp);
                return -2;
            } else{
                printf("open file ok:%d \n", rfp);
            }

            FD_SET(rfp, &read_set);
            for (int i = 0; i < fd_num;i++) {  //加入文件描述符到集合	
                if (*(array + i) == -1) {
                    *(array + i )= rfp ;
                    break;
                }
            }	
            char filename[k+1];
            strcpy(filename, filepath+(strlen(filepath)-k)+1);	//取名字
            strcpy(date_head.filename, filename);
            break;

        }
               //shou 
        case 1: {
            char path[] = "/home/zhengquan/share/git/udp/1.txt";
            int wfp = open(path, O_WRONLY | O_CREAT,S_IWUSR ); //开文件
            if (wfp == -1) {
                perror("wfp error");
                close(wfp);
            }

            printf("wfp:%d\n", wfp);	
            break;	
        }
    }
    return 0;
}


int send_file_date(const int send_sock, int *array)
{
    date_head.type = 0;
    strcpy(date_head.filename, "1.txt");
    char read_buff[BUFFER_SIZE] = {'\0'};
    lseek(rfp, date_head.file_num*BUFFER_SIZE,SEEK_SET);
    int read_num = read(rfp, read_buff,BUFFER_SIZE);
    if (read_num == -1) {
        perror("read error:");
    } else {
        date_head.date_len = read_num;
        memset(date_head.file_date, 0,sizeof(date_head.file_date));
        stpcpy(date_head.file_date, read_buff);//装载数据
        int addr_len = sizeof(addr_server);
        char send_buf[sizeof(date_head)] = {0};
        memset(send_buf, 0,sizeof(send_buf));
        memcpy(send_buf, &date_head,sizeof(date_head));

        int send_num = sendto(send_sock, send_buf,sizeof(send_buf),0,(struct sockaddr*)&addr_server,addr_len);
        if (send_num == -1) {
            perror("send file  date error:");
        } else {
            date_head.file_num ++;
        }

        if (read_num < BUFFER_SIZE) {
            printf(".....................send file over.......................\n");
            FD_CLR(rfp, &read_set);
            close(rfp);
            for (int i = 0;i < fd_num; i++) {
                if (*(array + i) == rfp ) {
                    *(array + i) = -1;	
                    bzero(&date_head, sizeof(date_head));
                    break;
                }			
            }		
        }
    }	
}




int do_recv_date(char *buff)
{
    char date[BUFFER_SIZE]= {'\0'};
    memcpy(&date, buff+sizeof(date_head.type)+sizeof(date_head.file_num)+sizeof(date_head.date_len)+sizeof(date_head.filename), BUFFER_SIZE);
    int n = *(int*)(buff+sizeof(date_head.type));
    int date_len = *(int*)(buff+sizeof(date_head.type) + sizeof(date_head.file_num));
    lseek(wfp, n*BUFFER_SIZE,SEEK_SET);
    int writelength = write(wfp, date,date_len);
    if (writelength == -1) {
        perror("write error");
    } else {
        printf("writelength:%d\n", writelength);
        if (writelength  < BUFFER_SIZE) {
            printf("接收文件完成\n");
        }    
    }						
}
