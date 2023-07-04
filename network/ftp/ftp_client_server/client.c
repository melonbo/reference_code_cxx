#include "client.h"

int sock;
struct hostent *hp;
struct sockaddr_in addr;
int result;
char CMD[CMD_LEN];
char data[BUFFER_SIZE];
char safeguard = '\0';
FILE* file;

void userName(char* user)
{
    memset(CMD, 0, sizeof(CMD));  
    strcpy(CMD, "user ");
    strcat(CMD, user);
    strcat(CMD, "\r\n");

    if ((result = write(sock, CMD, strlen(CMD))) < 0){
       printf("[err] Sending USER failed!");
    }
     
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s",CMD);
}

void passwd(char* passwd)
{
    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "pass ");
    strcat(CMD, passwd);
    strcat(CMD, "\r\n");

    if ((result = write(sock, CMD, strlen(CMD))) < 0){
       printf("[err] Recv PASSWORD failed!\n");
    }
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s",CMD);
}

void put(char* filename)
{
    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "pasv\r\n");
    if ((result = write(sock, CMD, strlen(CMD))) < 0)
    {
       printf("[err] Sending data failed!\n");
    }
    memset(CMD, 0 , sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);
    
    int ip1, ip2, ip3, ip4, port1, port2;
    char tmp[CMD_LEN];
    char *buff = (char *)alloca(CMD_LEN);
    memset(buff, 0 , sizeof(buff));
    memset(tmp, 0 , sizeof(tmp));
    strcpy(tmp, CMD);
    buff = strchr(tmp, '(');
    sscanf(buff, "(%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    int port;
    port = port1 * 256 + port2;
    
    int tempSock;
    tempSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tempAddr;
    bzero(&tempAddr, sizeof(struct sockaddr_in));
    tempAddr.sin_family = AF_INET;
    tempAddr.sin_port = htons(port);
    bcopy(hp->h_addr, (char *)&tempAddr.sin_addr, hp->h_length);
    int conn;
    conn = connect(tempSock, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
    if (conn < 0){
        perror("[err] Connect failed!\n");
    }

    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "type i\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    }
    memset(CMD, 0 , sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD); 
   
    memset(CMD, 0 , sizeof(CMD));
    strcpy(CMD, "stor ");
    strcat(CMD, filename);
    strcat(CMD, "\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    } 	
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);

    result = 1;
    FILE* file = fopen(filename, "rb");
    memset(data, 0, sizeof(data));
    while (result = fread(data, sizeof(char), BUFFER_SIZE, file))
    {
        write(tempSock, data, result);
        memset(data, 0, sizeof(data));
    }
    fclose(file);
    close(tempSock);

    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);
}

void get(char* filename)
{   
    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "pasv\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    }
    memset(CMD, 0 , sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);
    
    int ip1, ip2, ip3, ip4, port1, port2;
    char tmp[CMD_LEN];
    char *buff = (char *)alloca(CMD_LEN);
    memset(buff, 0 , sizeof(buff));
    memset(tmp, 0 , sizeof(tmp));
    strcpy(tmp, CMD);
    buff = strchr(tmp, '(');
    sscanf(buff, "(%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    int port;
    port = port1 * 256 + port2;
    int tempSock;
    tempSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tempAddr;
    bzero(&tempAddr, sizeof(struct sockaddr_in));
    tempAddr.sin_family = AF_INET;
    tempAddr.sin_port = htons(port);
    bcopy(hp->h_addr, (char *)&tempAddr.sin_addr, hp->h_length);
    int conn;
    conn = connect(tempSock, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
    if (conn < 0){
        perror("[err] Connect failed!\n");
    }

    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "type i\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0)
    {
       printf("[err] Sending CMD failed!\n");
    }
    memset(CMD, 0 , sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD); 

    memset(CMD, 0 , sizeof(CMD));
    strcpy(CMD, "retr ");
    strcat(CMD, filename);
    strcat(CMD, "\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    }

    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> "); 
    printf("%s", CMD); 
 
    FILE* file = fopen(filename, "w");
    while (result > 0){    
        memset(data, 0 , sizeof(data));
        result = read(tempSock, data, BUFFER_SIZE);
        fwrite(data, sizeof(char), result, file);
    }
    fclose(file);
    close(tempSock);
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);

}

void pwd()
{
    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "pwd");
    strcat(CMD, "\r\n");
    if ((result= write(sock, CMD, strlen(CMD))) < 0){
       printf("[err] Sending CMD failed!");
    }
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s",CMD);
}

void cd(char* cwd)
{
    memset(CMD, 0, sizeof(CMD));
    char end[CMD_LEN];
    strcpy(end, cwd);
    strcpy(CMD, "cwd ");
    strcat(CMD, end);
    strcat(CMD, "\r\n");
    result = write(sock, CMD, strlen(CMD));  
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    }
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN); 
    printf("-> ");
    printf("%s",CMD);
}

void dir(char* path)
{
    
    memset(CMD, 0, sizeof(CMD));
    strcat(CMD, "pasv\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    }
    memset(CMD, 0 , sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);
    
    int ip1, ip2, ip3, ip4, port1, port2;
    char tmp[CMD_LEN];
    char *buff = (char *)alloca(CMD_LEN);
    memset(buff, 0 , sizeof(buff));
    memset(tmp, 0 , sizeof(tmp)); //init
    strcpy(tmp, CMD);
    buff = strchr(tmp, '(');
    sscanf(buff, "(%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    int port;
    port = port1 * 256 + port2;
    int tempSock;
    tempSock = socket(AF_INET, SOCK_STREAM, 0);  
    struct sockaddr_in tempAddr;
    bzero(&tempAddr, sizeof(struct sockaddr_in));
    tempAddr.sin_family = AF_INET;
    tempAddr.sin_port = htons(port);
    bcopy(hp->h_addr, (char *)&tempAddr.sin_addr, hp->h_length);
    int conn;
    conn = connect(tempSock, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
    if (conn < 0){
        perror("[err] Connect failed!\n");
    }
    result = 1;  

    
    memset(CMD, 0 , sizeof(CMD));
    strcpy(CMD, "list ");
    strcat(CMD, path);
    strcat(CMD, "\r\n");
    result = write(sock, CMD, strlen(CMD));
    if (result < 0){
       printf("[err] Sending CMD failed!\n");
    } 	

    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);

    while (result > 0){    
        memset(data, 0 , sizeof(data));
        result = read(tempSock, data, BUFFER_SIZE);
        printf("%s", data);    
    }
   
    close(tempSock);
    
    memset(CMD, 0, sizeof(CMD));
    result = read(sock, CMD, CMD_LEN);
    printf("-> ");
    printf("%s", CMD);
    
}

void help()
{
    printf("FTP Command List:\n"\
	 "\t\t-get:  Download a file\n"\
	 "\t\t-put:  Upload a file\n" \
	 "\t\t-pwd:  Show the current path\n" \
	 "\t\t-dir:  Show all files in the current path\n"\
	 "\t\t-cd:   Change the current path\n"\
	 "\t\t-?:    List all commands\n"\
	 "\t\t-quit: Quit the FTP\n");
}

void quit()
{
    result = write(sock, "quit\r\n", 6);
    if (result < 0){
        printf("[err] Sending CMD failed!\n");
    }
    memset(CMD, 0, sizeof(CMD));
    printf("-> ");
    result = read(sock, CMD, CMD_LEN);
    printf("%s",CMD);
    if (close(sock) == 0){
        printf("Close connect succeed£¡\n");
    }
    else{
        printf("Close connect failed£¡\n");
    }
    exit(0);
}

void myOrder()
{
    char tmp[CMD_LEN];
    char* head;
    char* end; 
    char space[50];
    char p[50];
    int flag = 0;
    memset(tmp, 0, sizeof(tmp));
    printf("-> ");
    memset(CMD, 0, sizeof(CMD_LEN));
    fgets(CMD, CMD_LEN, stdin); 
    strncpy(tmp, CMD, (strlen(CMD) - 1));
    head = strtok(tmp, " "); 
    end = strtok(NULL, " "); 
    if (end != NULL){
        strcpy(space, end);
        flag = 1;
    }
    if (strcasecmp(head, "pwd") == 0){
        pwd();
    } 
    else if (strcasecmp(head, "quit") == 0) {
        quit();
    }
    else if (strcasecmp(head, "?") == 0){
        help();
    }
    else if (strcasecmp(head, "dir") == 0)
    {
        if (flag == 1)
        {
            dir(space); 
        }
        else
        {
            dir("");
	}
    }
    else if (strcasecmp(head, "get") == 0)
    {
        if (flag == 1)
        {
            get(space);
        }
        else
        {
            printf("-> ");
            printf("please enter the file name\n");
        }
    }
    else if (strcasecmp(head, "put") == 0)
    {
        if (flag == 1)
        {
            put(space);
        }
        else
        {
            printf("-> ");
            printf("please enter the file name\n");
        }
    }
    else if (strcasecmp(head, "cd") == 0)
    {
      	cd(space);
    }
    else{
        printf("wrong command£¡\n");
    }
}

int main(int argc, char* argv[])
{
    printf("\n");

    char buffer[BUFFER_SIZE];
 
if (argc > 1) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[err] Create socket failed!\n");
    }

    hp = gethostbyname(argv[1]);

    if (hp == NULL){ 
        perror("The host name wrong!\n");
    }
    
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERV_PORT);
    bcopy(hp->h_addr, (char *)&addr.sin_addr, hp->h_length);

    printf("IP: %s.\n", inet_ntoa(addr.sin_addr));
    printf("PORT : %d.\n", SERV_PORT);

    int c ;
    if ((c= connect(sock, (struct sockaddr*)&addr, sizeof(addr)) )< 0){
        	perror("Connect failed.\n");
    } 

    result = read(sock, data,BUFFER_SIZE);
    file = fdopen(sock, "rw+");

    userName(argv[2]);
    passwd(argv[3]);
        
    while (1) {
        myOrder();
    }
}
    else{
        printf("Please enter the host name\n");
    	}
    printf("\n");
}


