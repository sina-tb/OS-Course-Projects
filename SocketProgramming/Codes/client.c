#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

#define STUDENT 2
#define TA 1
#define BUFFER_SIZE 1024
#define FINISH 1
#define CONTINUE 0
#define QUIT 2
#define STOP 3

int roleHandler(char buff[],int role)
{
    char* command = strtok(buff, "\n");
    if(strcmp("ta",command) == 0)
    {
        return TA;
    }
    if(strcmp("student",command) == 0)
    {
        return STUDENT;
    }
    return role;
}
int quitBroadcast(char buff[],int role)
{
    char* command = strtok(buff, "\n");
    if(strcmp("finish",command) == 0 && role == TA)
    {
        return FINISH;
    }
    else if (strcmp("quit",command) == 0 && role == STUDENT)
    {
        return QUIT;
    }
    else
    {
        return CONTINUE;
    }
}

int joinBroadcast(int port)
{
    int sock, broadcast = 1, opt = 1;

    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.88.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
    
    int status = 0;
    char buffer[BUFFER_SIZE];

    printf("welcome to room %d\n",sock);

    while (1) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
        status=quitBroadcast(buffer,STUDENT);
        if (status == QUIT)
        {
            printf("you have leaved the room!\n");
            close(sock);
            return QUIT;
        }
    }
}
int studentBroadcast(int port)
{
    int sock, broadcast = 1, opt = 1;

    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.88.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
    int status = 0;
    char buffer[BUFFER_SIZE];

    printf("welcome to room %d\n",sock);

    while (1) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
        status=quitBroadcast(buffer,STUDENT);
        if (status == QUIT)
        {
            printf("you have leaved the room!\n");
            close(sock);
            return QUIT;
        }
        read(0, buffer, BUFFER_SIZE);
        status=quitBroadcast(buffer,TA);
        if (status == QUIT)
        {
            printf("you have leaved the room!\n");
            close(sock);
            return QUIT;
        }
        int a = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
    }
}
void alarm_handler()
{
    printf("room time limit passed!\n");
    return;
}
int taBroadcast(int port,int server_fd)
{
    int sock, broadcast = 1, opt = 1;

    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.88.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
    int status = 0;
    char buffer[BUFFER_SIZE];
    printf("welcome to room %d\n",sock);
    
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);

    while (1) 
    {
        alarm(60);
        int interupt =read(0, buffer, BUFFER_SIZE);
        alarm(0);
        if (interupt == -1)
        {
           break;
        }
        status=quitBroadcast(buffer,TA);
        if (status == FINISH)
        {
            printf("you have closed the room!\n");
            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer,"finish");
            send(server_fd, buffer, strlen(buffer), 0);
            close(sock);
            return FINISH;
        }
        int a = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }
    

    char msg[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    printf("you have kicked from room!\n");
    sprintf(msg,"stop");
    send(server_fd, msg, strlen(msg), 0);
    return STOP;
}
void broadcastPortHandler(char buff[BUFFER_SIZE],int role,int server_fd)
{
    char* command= strtok(buff," ");
    if (strcmp("port",command) != 0 && strcmp("join",command) != 0 )
    {
        return;
    }

    char msg[BUFFER_SIZE];
    char* string2=strtok(NULL," ");
    int port = atoi(string2);

    if (strcmp("port",command) == 0)
    {
        if(role == STUDENT)
        {
            studentBroadcast(port);
        }
        if(role == TA)
        {
            taBroadcast(port,server_fd);
        }
    }
    if (strcmp("join",command) == 0)
    {
        joinBroadcast(port);
    }

}
int connectServer(int port) {

    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Error in connecting to server\n");
    }

    return fd;
}

int main(int argc, char const *argv[]) {
    int fd, role;
    char buff[BUFFER_SIZE];
    char buff2[BUFFER_SIZE];
    int port = atoi(argv[1]);
    fd = connectServer(port);
    memset(buff, 0, BUFFER_SIZE);

    while(1)
    {
        memset(buff, 0, BUFFER_SIZE);
        memset(buff2, 0, BUFFER_SIZE);
        recv(fd, buff, BUFFER_SIZE, 0);
        strcpy(buff2,buff);
        printf("server:\n%s\n",buff);
        broadcastPortHandler(buff2,role,fd);
        memset(buff, 0, BUFFER_SIZE);
        memset(buff2, 0, BUFFER_SIZE);
        read(0, buff, BUFFER_SIZE);
        strcpy(buff2,buff);
        role = roleHandler(buff,role);
        send(fd, buff, strlen(buff), 0); 
    }

    return 0;
}