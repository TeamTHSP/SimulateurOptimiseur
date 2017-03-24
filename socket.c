#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <pthread.h>

#define BUFF_SIZE  1000000

int socket_c,client_c,c,read_size;
struct sockaddr_in server,client;
pthread_t thread;


char buffer[BUFF_SIZE];

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void execCmd(const char* cmd, int* count)
{
    printf("dans execCmd :: '%s'\n",cmd);
    FILE* pipe = popen(cmd, "r");
    if (pipe==NULL) error("Exec error");
    

    bzero(buffer, BUFF_SIZE);
    
    int r=read(fileno(pipe), buffer, BUFF_SIZE);
    
    pclose(pipe);
}

int sockExecHandler(const int socket)
{
    int status;
    
    int stop = 1;
    bzero(buffer, BUFF_SIZE);
    while (stop)
    {
        status = read(socket, buffer, BUFF_SIZE);
        if (status < 0)
            error("Read error");
        
        
        
        buffer[status-2] = '\0';
        
        printf("buffer sans dernier chara ::: '%s'\n",buffer);
        
        if (strcmp(buffer, "Fin") == 0)
            return 0;
        int size_count = 0;
        printf("avant execCmd\n");
        execCmd(buffer, &size_count);

        status = write(socket, buffer, BUFF_SIZE);
        if (status < 0) error("Write error");
    }
    printf("Fin\n");
    return -1;
}



/**
 parti de Thread
 */
static void * fn_thread(void * clientS)
{
    int* test = (int *) clientS;
    
    int clientSocket = *test; 
    
    sockExecHandler(clientSocket);
    return NULL;
}

pthread_t * createThread(int clientS)
{
    pthread_create(&thread,NULL,fn_thread,(void *) &clientS);
    return &thread;
}

/**
 parti Socket
 */
void createSocket()
{
    socket_c=socket(AF_INET , SOCK_STREAM , 0);
    if(socket_c)
    {
        printf("ok\n");
    }
    else
    {
        printf("nope\n");
    }
}

int bindSocket(int port)
{
    server.sin_family         = AF_INET;
    server.sin_addr.s_addr    = INADDR_ANY;
    server.sin_port           = htons(port);
    
    if( bind(socket_c,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("bind failed.\n");
        return 1;
    }
    printf("Bind ok\n");
    return 0;
}

void comSocket(int clientS){
    createThread(clientS);
}

int main(int argc, char* argv[])
{
    c = sizeof(struct sockaddr_in);
    
    createSocket();
    bindSocket(atoi(argv[1]));
    
    while(1){
        listen(socket_c,5);
        client_c = accept(socket_c, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_c < 0){
            return 1;
        }
        comSocket(client_c);
    }
    close(socket_c);
    return 0;
}
