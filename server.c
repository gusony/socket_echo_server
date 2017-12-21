#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>    // for wait()
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>     //write
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>

#define MAXLINE              512
#define MESSAGE_LEN          10000
#define SERV_TCP_PORT        7575

int  readline(int fd, char *ptr, int maxlen);
void start_server(int argc,char *argv[],int *serverfd);


int  readline(int fd, char *ptr, int maxlen){
    // 這個function 是從client 一個個byte的讀，直到讀到\n 或\r 或\0 就當作讀完一行
    int n,rc;
    char c;
    for(n=1; n<maxlen;n++){
        if((rc=read(fd, &c, 1))==1){
            /*
            read() https://linux.die.net/man/3/read
            fd :file descriptor
            &c :要把讀到的byte存到哪(讀資料最小單位是byte)
            1  :要讀幾個bytes,可以搭配sizeof()使用

            return: 成功的話read會回傳讀了幾個bytes
            失敗會回傳-1
            沒東西了回傳0
            */
            if(c=='\n' || c=='\0' || c=='\r')
                break;
            *ptr++ = c;
        }
        else if (rc==0){
            if(n==1) return(0);
            else break;
        }
        else
            return(-1);
    }
    *ptr = 0;
    return(n);
}
void start_server(int argc,char *argv[],int *serverfd){
    struct sockaddr_in  serv_addr;
    printf("Server starting...\n");

    /* 1.Socket */
    if((*serverfd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("server : can't open stream socket\n");
        return;
    }
    else printf("create socketfd successful\n");

    /* 2.bind */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //htonl :convert "ASCII IP" to "int IP"
    if(argc<2)
        serv_addr.sin_port = htons(SERV_TCP_PORT);
    else
        serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(*serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0){ //bind serverfd and serv_addr
        printf("server : can't bind local address\n");
        exit(0);
    }
    else printf("bind successful\n");


    /* 3.listen */
    listen(*serverfd, 5);
    printf("Start Server finish, wait client\n");
}
int main(int argc,char *argv[]){
    int serverfd=0, clientfd=0,clilen=0;
    char temp[MESSAGE_LEN];         bzero(temp, sizeof(temp));//init temp
    char inputBuffer[MESSAGE_LEN];  bzero(inputBuffer, sizeof(inputBuffer));//init inputbuffer
    struct sockaddr_in cli_addr;

    char welcome_message[]="****************************************\n**       Welcome to echo server       **\n****************************************\n% ";

    start_server(argc,argv,&serverfd);

    while(1){
        //wait client connet to this server , and then accept client
        clilen = sizeof(cli_addr);
        clientfd = accept(serverfd, (struct sockaddr*)&cli_addr,(socklen_t *) &clilen);
        printf("accept client fd=%d\n",clientfd);

        // child process serve accepted client
        write(clientfd,welcome_message,strlen(welcome_message));

        while(1){
            //read one line
            if(readline(clientfd, inputBuffer, sizeof(inputBuffer))>1){
                //if read exit, server will close the connection
                if(strcmp(inputBuffer, "exit") == 0){
                    close(clientfd);
                    printf("\n===exit===\n");
                    break;
                }
                sprintf(temp, "server=>:%s\n%% ",inputBuffer);
                write(clientfd,temp,strlen(temp));
            }
        }
    }
    return(0);
}
