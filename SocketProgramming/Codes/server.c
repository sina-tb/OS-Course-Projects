#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define TA 1
#define STUDENT 2
#define BUFFER_SIZE 1024
#define MAX_USERS 20
#define MAX_QUESTION_SIZE 20
#define MAX_ROOM_SIZE 10
#define MAX_QUESTION_MSG_SIZE 1024
#define NOT_ANSWERED 0
#define ANSWERING 1
#define ANSWERED 2


typedef struct
{
    char question_msg[MAX_QUESTION_MSG_SIZE];
    int fd_id;
    int status;
} question;

typedef struct
{
    question list_of_questions[MAX_QUESTION_SIZE];
    int count;

}questions;

typedef struct 
{
    int fd;
    int role;
}user;

typedef struct 
{
    user list_of_users[MAX_USERS];
    int count;
}users;

typedef struct 
{
    int student_fd;
    int TA_fd;
    int port;
    char question_msg[MAX_QUESTION_MSG_SIZE];
    int status;
    int room_fd;
}room;

typedef struct 
{
    room list_of_rooms[MAX_USERS];
    int count;
}rooms;

fd_set master_set;
int max_sd;

int setupServer(int port) {

    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, MAX_USERS);

    return server_fd;
}

int acceptClient(int server_fd) {

    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);
    printf("Client connected!\n");

    return client_fd;
}

void getNewClient(users *usr,int new_socket,char command[],int role)
{
    char buff[BUFFER_SIZE];
    user new_user;
    new_user.fd = new_socket;
    new_user.role = role;
    usr->list_of_users[usr->count] = new_user;
    usr->count = usr->count + 1;
    memset(buff, 0, BUFFER_SIZE);
    sprintf(buff,"Please enter 'help' to receive full guide of commandlines");
    send(new_socket, buff, BUFFER_SIZE, 0);

}
void getQuestion(int fd,questions *qu,char buffer[])
{
    char msg[BUFFER_SIZE];
    question new_question;
    new_question.fd_id = fd;
    strcpy(new_question.question_msg,buffer);
    new_question.status = NOT_ANSWERED;
    qu->list_of_questions[qu->count] = new_question;
    qu->count = qu->count + 1;
}
void getAnswer(char buffer[],questions *qu,int fd)
{
    char ques[BUFFER_SIZE];
    char buff1[BUFFER_SIZE];
    char buff2[BUFFER_SIZE];
    for ( int i = 0; i< qu->count; i++)
    {
        if( qu->list_of_questions[i].fd_id == fd && qu->list_of_questions[i].status == ANSWERING)
        {
            qu->list_of_questions[i].status = ANSWERED;
            strcpy(ques,qu->list_of_questions[i].question_msg);
        }
    }
    char* fileName = malloc(strlen("log.txt"));
    sprintf(fileName,"log.txt");
    int f = open(fileName, O_RDWR | O_APPEND | O_CREAT, 0644);
    free(fileName);
    strcat(buffer,"\n");
    strcat(ques,"\n");
    sprintf(buff1,"Q: ");
    sprintf(buff2,"A: ");
    strcat(buff1,ques);
    strcat(buff2,buffer);
    write(f,buff1 , strlen(buff1));
    write(f,buff2, strlen(buff2));
    close(f);
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer,"Your answer saved!\n");
    send(fd, buffer, BUFFER_SIZE, 0);
}
void getRoomList(rooms *ro,int fd)
{
    char buffer[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    char buffer3[BUFFER_SIZE];
    int count = 0;

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer,"List of Rooms:\n");
    for (int i = 0; i< ro->count; i++)
    {
        if(ro->list_of_rooms[i].status == ANSWERING)
        {
            count += 1;
            sprintf(buffer2,"%d- room with question: ",count);
            sprintf(buffer3,ro->list_of_rooms[i].question_msg,"\n");
            strcat(buffer3,"\n");
            strcat(buffer2,buffer3);
            strcat(buffer,buffer2);
            buffer2[0] = '\0';
            buffer3[0] = '\0';
        }
    }
    if (count == 0)
    {
        buffer[0] = '\0';
        sprintf(buffer,"There is no room here!\n");
        send(fd, buffer, BUFFER_SIZE, 0);
    }
    else
    {
        send(fd, buffer, BUFFER_SIZE, 0);
    }
}
void createBroadcastPort(rooms *ro,int ta_fd,int student_fd,char msg[])
{
    int port = ro->count + 1 + 8080;
    int sock, broadcast = 1, opt = 1;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.88.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    FD_SET(sock, &master_set);
    if (sock > max_sd)
        max_sd = sock;
    
    room new_room;
    new_room.port = port;
    new_room.student_fd = student_fd;
    new_room.TA_fd = ta_fd;
    new_room.status = ANSWERING;
    strcpy(new_room.question_msg,msg);
    ro->list_of_rooms[ro->count] = new_room;
    ro->list_of_rooms[ro->count].room_fd = sock;
    ro->count = ro->count + 1;


    printf("new broadcast with fd = %d added\n",sock);
    sprintf(buffer,"port %d",port);
    send(ta_fd, buffer, BUFFER_SIZE, 0);
    send(student_fd, buffer, BUFFER_SIZE, 0);

}

void chooseRoom(rooms *ro,int fd,int room_number)
{
    char buffer[BUFFER_SIZE];
    int count = 0;
    int port = -1;
    for (int i=0;i<ro->count;i++)
    {
        if(ro->list_of_rooms[i].status == ANSWERING)
        {
            count += 1;
            if (count == room_number)
            {
                port = ro->list_of_rooms[i].port;
            }
        }
    }
    if (port == -1)
    {
        sprintf(buffer,"Your room number is invalid\n");
        send(fd, buffer, BUFFER_SIZE, 0);
        return;
    }

    sprintf(buffer,"join %d",port);
    send(fd, buffer, BUFFER_SIZE, 0);
    return;
}
void chooseQuestion(questions *qu,rooms *ro,int fd,int question_number)
{
    char buffer[BUFFER_SIZE];
    int count = 0;
    int student_fd = -1;
    for (int i=0;i<qu->count;i++)
    {
        if(qu->list_of_questions[i].status == NOT_ANSWERED)
        {
            count += 1;
            if( count == question_number)
            {
                qu->list_of_questions[i].status = ANSWERING;
                student_fd = qu->list_of_questions[i].fd_id;
                strcpy(buffer,qu->list_of_questions[i].question_msg);
            }
        }
    }
    if (student_fd == -1)
    {
        sprintf(buffer,"Your question number is invalid\n");
        send(fd, buffer, BUFFER_SIZE, 0);
        return;
    }
    createBroadcastPort(ro,fd,student_fd,buffer);
}
void getQuestionList(questions *qu,int fd)
{
    char buffer[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    char buffer3[BUFFER_SIZE];
    int count = 0;

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer,"List of Questions:\n");
    for (int i = 0; i< qu->count; i++)
    {
        if(qu->list_of_questions[i].status == NOT_ANSWERED)
        {
            count += 1;
            sprintf(buffer2,"%d-",count);
            sprintf(buffer3,qu->list_of_questions[i].question_msg,"\n");
            strcat(buffer3,"\n");
            strcat(buffer2,buffer3);
            strcat(buffer,buffer2);
            buffer2[0] = '\0';
            buffer3[0] = '\0';
        }
    }
    if (count == 0)
    {
        buffer[0] = '\0';
        sprintf(buffer,"There is no question here!\n");
        send(fd, buffer, BUFFER_SIZE, 0);
    }
    else
    {
        send(fd, buffer, BUFFER_SIZE, 0);
    }
}
void getHelpTA(int fd){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer,"commands:\n" \
                    "\thelp -- show this page\n" \
                    "\tlist -- show the list of quesitons\n" \
                    "\tquestion n -- choose nth question of the list\n");
    send(fd, buffer, BUFFER_SIZE, 0);
}   


void getHelpStudent(int fd){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer,"commands:\n" \
                    "\thelp -- show this page\n" \
                    "\trooms -- show list of dicussion rooms\n" \
                    "\troom n -- choose nth room of the list\n" \
                    "\task <question> -- you can ask your question\n" \
                    "\tanswer <answer> -- you can insert the answer of quesiton here\n");
    send(fd, buffer, BUFFER_SIZE, 0);
}
void notAnsweredQuestion(rooms *ro,questions *qu,int fd)
{
    char msg[BUFFER_SIZE];
    for (int i = 0; i< ro->count; i++)
    {
        if( ro->list_of_rooms[i].TA_fd == fd && ro->list_of_rooms[i].status == ANSWERING)
        {
            ro->list_of_rooms[i].status = NOT_ANSWERED;
            printf("room with fd = %d stopped\n",ro->list_of_rooms[i].room_fd);
            strcpy(msg,ro->list_of_rooms[i].question_msg);
            close(ro->list_of_rooms[i].room_fd);
            FD_CLR(ro->list_of_rooms[i].room_fd, &master_set);
        }
    }
    for ( int i =0; i< qu->count; i++)
    {
        if( strcmp(msg,qu->list_of_questions[i].question_msg) == 0)
        {
            printf("that is good\n");
            qu->list_of_questions[i].status = NOT_ANSWERED;
        }
    }
}
void deleteRoom(int fd,rooms *ro)
{
    for (int i = 0; i< ro->count; i++)
    {
        if( ro->list_of_rooms[i].TA_fd == fd && ro->list_of_rooms[i].status == ANSWERING)
        {
            ro->list_of_rooms[i].status = ANSWERED;
            printf("room with fd = %d ended\n",ro->list_of_rooms[i].room_fd);
            close(ro->list_of_rooms[i].room_fd);
            FD_CLR(ro->list_of_rooms[i].room_fd, &master_set);
        }
    }
}
void accessDenied(int fd)
{
    char buffer[BUFFER_SIZE];
    sprintf(buffer,"you have no access to this part!\n");
    send(fd, buffer, BUFFER_SIZE, 0);
}
void wrong_command(int fd)
{
    char buffer[BUFFER_SIZE];
    sprintf(buffer,"the command is wrong!\n");
    send(fd, buffer, BUFFER_SIZE, 0);
}
void commandHandler(int fd,char buffer[],int role,questions *qu,rooms *ro,users *usr)
{   
    char *msg;
    char *command;
    char *string1 = strtok(buffer, " ");
    char *string2 = strtok(NULL," ");
    if (string2 == NULL)
    {
        command = strtok(string1,"\n");
    }
    else
    {
        command = string1;
        msg = strtok(string2,"\n");
    }
    
    if ( strcmp(command,"student") == 0)
    {
        getNewClient(usr,fd,command,STUDENT);
    }
    else if ( strcmp(command,"ta") == 0)
    {
        getNewClient(usr,fd,command,TA);
    }
    else if ( strcmp(command,"help") == 0)
    {    
        if( role == 1)
        {
            getHelpTA(fd);
        }
        if ( role == 2)
        {
            getHelpStudent(fd);
        }
    }
    else if ( strcmp(command,"list") == 0)
    {
        if( role == 1)
        {
            getQuestionList(qu,fd);
        }
        if ( role == 2)
        {
            accessDenied(fd);
        }
    }
    else if ( strcmp(command,"question") == 0)
    {
        if( role == 1)
        {
            chooseQuestion(qu,ro,fd,atoi(msg));
        }
        if ( role == 2)
        {
            accessDenied(fd);
        }
    }
    else if ( strcmp(command,"rooms") == 0)
    {
        if( role == 1)
        {
            accessDenied(fd);
        }
        if ( role == 2)
        {
            getRoomList(ro,fd);
        }
    }
    else if ( strcmp(command,"room") == 0)
    {
        if( role == 1)
        {
            accessDenied(fd);
        }
        if ( role == 2)
        {
            chooseRoom(ro,fd,atoi(msg));
        }
    }
    else if (strcmp(command,"ask") == 0)
    {
        if( role == 1)
        {
            accessDenied(fd);
        }
        if ( role == 2)
        {
            getQuestion(fd,qu,msg);
        }
    }
    else if ( strcmp(command,"answer") == 0)
    {
        if( role == 1)
        {
            accessDenied(fd);
        }
        if ( role == 2)
        {
            getAnswer(msg,qu,fd);
        }
    }
    else if ( strcmp(command,"finish") == 0 && role == TA)
    {
        deleteRoom(fd,ro);
    }
    else if ( strcmp(command,"stop") == 0 && role == TA)
    {
        notAnsweredQuestion(ro,qu,fd);
    }
    else
    {
        wrong_command(fd);
    }
}

int getRole(users *usr,int fd)
{
    for(int i = 0; i< usr->count;i++)
    {
        if ( usr->list_of_users[i].fd == fd)
        {
            return usr->list_of_users[i].role;
        }
    }
}

void getClientMessage(users *usr,int fd,questions *qu,char buffer[],rooms *ro){

    int role = getRole(usr,fd);
    commandHandler(fd,buffer,role,qu,ro,usr);
    memset(buffer, 0, BUFFER_SIZE);
}

void firstMessage(int new_socket)
{
    char buff[BUFFER_SIZE];
    memset(buff, 0, BUFFER_SIZE);
    sprintf(buff,"Please choose your role:\n1-ta\n2-student");
    send(new_socket, buff, BUFFER_SIZE, 0);
}

void serverProcess(int port)
{
    int server_fd, client_fd;
    char buffer[BUFFER_SIZE];
    fd_set working_set;
    users usr;
    questions qu;
    rooms ro;
    qu.count = 0;
    usr.count = 0;
    ro.count = 0;
    server_fd = setupServer(port);
    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server online\n", 15);

    while (1) 
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) 
        {
            if (FD_ISSET(i, &working_set)) 
            {
                
                if (i == server_fd)
                {
                    int new_socket = acceptClient(server_fd);
                        FD_SET(new_socket, &master_set);
                        if (new_socket > max_sd)
                            max_sd = new_socket;
                        firstMessage(new_socket);
                }

                else {
                    int bytes_received;
                    memset(buffer, 0, BUFFER_SIZE);
                    bytes_received = recv(i, buffer, BUFFER_SIZE, 0);   
                    if (bytes_received == 0) {
                    printf("client fd = %d closed\n", i);
                    close(i);
                    FD_CLR(i, &master_set);
                    }
                    printf("client %d: %s\n", i, buffer);
                    getClientMessage(&usr,i,&qu,buffer,&ro);
                    buffer[0] = '\0';
                }

            }
        }

    }

    return ;
}

int main(int argc, char const *argv[]) 
{
    serverProcess(atoi(argv[1]));
    return 0;
}