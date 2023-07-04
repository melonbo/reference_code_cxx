#include "server.h"

int sock;  
struct sockaddr_in addr;  
struct sockaddr_in in_addr;
int result;
char CMD[CMD_LEN];
char data[BUFFER_SIZE];
int cmdSock;
int dataSock = 0;
int len;
int logged = 0;
int passed = 0;
int endflag = 0;
int type = 2;

void sendCmd(int num, char* msg, int last)
{ 
        memset(CMD, 0, sizeof(CMD));
        sprintf(CMD, "%d %s\r\n", num ,msg);
        write(cmdSock, CMD, strlen(CMD));
  
}

void pasv()
{
    printf("enter pasv\n");
    if (dataSock == 1){
        close(dataSock);
        dataSock = -1;
    }
    struct sockaddr_in dataAddr;

    dataSock = socket(AF_INET, SOCK_STREAM, 0);

    if (dataSock < 0){
        perror("[err] Create datasocket failed!\n");
        sendCmd(425, "[err] Create datasocket failed!", 0);
        return;
    }
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_addr.s_addr = INADDR_ANY;
    dataAddr.sin_port = 0; 
    
    int length = sizeof(dataAddr);
    if (bind(dataSock, (struct sockaddr*)&dataAddr, sizeof(dataAddr)) < 0)
    {
        perror("[err] Bind failed! \n");
        sendCmd(425, "[err] Bind failed! \n", 0);
        return;
    }
    if (getsockname(dataSock, (struct sockaddr*)&dataAddr, (socklen_t*)&length) < 0)
    {
        perror("can't get the data port");
        sendCmd(425, "can't get the data port", 0);
        return;
    }

    listen(dataSock, 5);

    char buff[CMD_LEN];
    int ip1, ip2, ip3, ip4, port1, port2;
    port1 = ntohs(dataAddr.sin_port); 
    port2 = port1 % 256;
    port1 = port1 / 256;

    if (getsockname(cmdSock, (struct sockaddr*)&dataAddr, (socklen_t*)&length) < 0)
    {
        perror("can't get the cmd port");
        sendCmd(425, "can't get the cmd port", 0);
        return;
    }

    sprintf(buff, "%s", inet_ntoa(dataAddr.sin_addr)); 

    sscanf(buff, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
    sprintf(buff, "PASV OK (%d,%d,%d,%d,%d,%d)",
            ip1, ip2, ip3, ip4, port1, port2);
    sendCmd(227, buff, 0);
}

void typeChg(char* arg)
{
    if (strcasecmp(arg, "i") == 0)
    {
        sendCmd(200, "current type i", 0);
        type = 1;
    }
    else if (strcasecmp(arg, "a") == 0)
    {
        sendCmd(200, "now is type a", 0);
        type = 2;
    }
    else
    {
        sendCmd(504, "wrong type", 0);
    }
}

void pwd()
{
        char* path = (char *)malloc(100);
        getcwd(path, 100);
        if (strstr(path, ROOT) != NULL){
            path = path + strlen(ROOT);
        }
        else
        {
            sendCmd(550, "file dosen't exist in the root", 0);
            return;
        }
        if (strcmp(path, "") == 0)
        {
            strcat(path, "/");
        }
        strcat(path, " is the current directory.");
        sendCmd(257,path, 0);
}

void cd(char* path)
{
   	char* newpath = (char *)malloc(CMD_LEN);
        if (path[0] == '/'){
            sprintf(newpath, "%s%s", ROOT, path);
        }
        else{
            sprintf(newpath, "%s", path);
        }
        if (chdir(newpath) < 0){
            sendCmd(550, "the Currentpath can't be changed!", 0);
            return;
        }
        memset(newpath, 0, sizeof(newpath));
        getcwd(newpath, CMD_LEN);
        if (strstr(newpath, ROOT) != NULL){
            newpath = newpath + strlen(ROOT);
        }
        else{
            printf("dir dosen't exist in the root\n");
        }   
        strcat(newpath, "is the current directory.Change Curretpath Succeed!");
        sendCmd(250, newpath, 0);
}

void quit()
{
    memset(CMD, 0, sizeof(CMD));
    strcpy(CMD, "quit succeed£¡\r\n");
    write(cmdSock, CMD, strlen(CMD));
    close(cmdSock);
    endflag = 0;
}

void list(char* path)
{
    
    int new_dataSock = accept(dataSock, NULL, NULL);
    if (new_dataSock < 0){
        perror("[err] Create datasocket failed!\n");
        close(dataSock);
        dataSock = -1;
    }
    sendCmd(150, " Create datasocket", 0);

    if (dataSock == -1){
        sendCmd(425, "[err] Create datasocket failed!", 0);
        return;
    }

    char* newpath = (char *)malloc(CMD_LEN);

    if (path[0] == '/'){
        sprintf(newpath, "%s", ROOT, path);
    }
    else if (path == NULL){
        sprintf(newpath, "%s", "");
    }
    else{
        sprintf(newpath, "%s", path);
    }
    
    char* buff = (char *)malloc(BUFFER_SIZE);
    sprintf(buff, "ls -la %s", newpath);
    FILE* file = popen(buff, "r");
    int flag = 0;
    while (!feof(file)){
        memset(data, 0, sizeof(data));
        memset(buff, 0, sizeof(buff));
        fgets(buff, BUFFER_SIZE, file);
        if (flag != 0){
            sprintf(data, "%s", buff);
            write(new_dataSock, buff, strlen(buff));
        }
        flag = flag + 1;
    }
    fgets(buff, BUFFER_SIZE, file);

    sendCmd(226, "226 Transfer OK", 0);
    close(new_dataSock);
    close(dataSock);
    dataSock = -1;

}

void stor(char* filename)
{
    if (dataSock == -1){
        sendCmd(425, "[err] Create datasocket failed!", 0);
        return;
    }
    if (strlen(filename) == 0){
        sendCmd(501, "please enter the file name!", 0);
    }

    int new_dataSock = accept(dataSock, NULL, NULL);

    if (new_dataSock < 1){
        perror("[err] Create datasocket failed!\n");
        close(dataSock);
        dataSock = -1;
    }

    sendCmd(150, "Create datasocket", 0);

    FILE* file = fopen(filename, "wb");
    if (file == NULL){
        sendCmd(550, "open file failed", 0);
        return;
    }
    
    int length;
    char buff[BUFFER_SIZE];
    memset(buff, 0, sizeof(buff));
    FILE* from = fdopen(new_dataSock, "rb");
    while(!feof(from)){
        length = fread(buff, 1, BUFFER_SIZE, from);
        fwrite(buff, 1, length, file);
        memset(buff, 0, sizeof(buff));
    }

    fclose(from);
    fclose(file);

    sendCmd(226, "Send msg succeed in PASV", 0);
    close(new_dataSock);
    close(dataSock);
    dataSock = -1;
}

void retr(char* filename)
{

    if (dataSock == -1){
        sendCmd(425, "[err] Create datasocket failed!", 0);
        return;
    }
    if (strlen(filename) == 0){
        sendCmd(501, "please enter the file name!", 0);
    }

    int new_dataSock = accept(dataSock, NULL, NULL);
    if (new_dataSock < 1){
        perror("[err] Create datasocket failed!\n");
        close(dataSock);
        dataSock = -1;
    }

    sendCmd(150, "Create datasocket", 0);

    FILE* file = fopen(filename, "rb");
    if (file == NULL){
        sendCmd(550, "open file failed", 0);
        return;
    }

    FILE* to = fdopen(new_dataSock, "wb");

    char buff[BUFFER_SIZE];
    memset(buff, 0, sizeof(buff));
    int length;
    while (!feof(file)){
        length = fread(buff, 1, BUFFER_SIZE, file);
        fwrite(buff, 1, length, to);
        memset(buff, 0, sizeof(buff));
    }

    fclose(to);
    fclose(file); 

    sendCmd(226, "226 Transfer OK", 0);
    close(new_dataSock);
    close(dataSock);

    dataSock = -1;
}

void dealCMD(char* incomingCmd)
{
    char* head;
    char* end; 
    char space[50];
    char p[50];
    head = strtok(incomingCmd, " ");
    end = strtok(NULL, " ");
    if (end != NULL){
        strcpy(space, end);
    }
    if (strcasecmp(head, "user") == 0 && strcmp(space, USER) == 0){
        memset(CMD, 0, sizeof(CMD));
        strcpy(CMD, "331 Password required for user\r\n");
        write(cmdSock, CMD, strlen(CMD));
        logged = 1;
    }
    else if (strcasecmp(head, "pass") == 0)    {
        if ( logged == 1){
            if (strcmp(space, PASSWD) == 0){
                memset(CMD, 0, sizeof(CMD));
                strcpy(CMD, "230 LOG IN SUCCEED\r\n");
                write(cmdSock, CMD, strlen(CMD));
                passed = 1;
            }
            else {
                memset(CMD, 0, sizeof(CMD));
                strcpy(CMD, "[err] Wrong Password\r\n");
                write(cmdSock, CMD, strlen(CMD));
            }
        }
    }
    else if (strcasecmp(head, "pwd") == 0){
        pwd();
    }
    else if (strcasecmp(head, "cwd") == 0){
        cd(space);
    }
    else if (strcasecmp(head, "quit") == 0) {
        quit();
    }
    else if (strcasecmp(head, "pasv") == 0){
        pasv();
    }
    else if (strcasecmp(head, "type") == 0){
        typeChg(space);
    }
    else if (strcasecmp(head, "list") == 0){
        list(space);
    }
    else if (strcasecmp(head, "stor") == 0) {
        stor(space);
    }
    else if (strcasecmp(head, "retr") == 0){
        retr(space);
    }
}

void receiveCMD()
{
    while (1) {
        printf("\n[sys] Waiting for connection...\n");
        cmdSock = accept(sock, (struct sockaddr*)&in_addr, (socklen_t*)&len); 
        if (fork() == 0){
        	if (chdir(ROOT) < 0){
            		printf("enter root failed\n");
        	}
        	endflag = 1; 
        	if (cmdSock == -1){
            		perror("[err] Accept failed!\n");
            		break;
       	 	}
        
        	memset(CMD, 0, CMD_LEN);
        	strcpy(CMD, "please enter the USER\r\n");
        	write(cmdSock, CMD, strlen(CMD));
      
        	printf("Connect\n");
        	
        	do{
             		memset(CMD, 0, CMD_LEN);
             		result = read(cmdSock, CMD, CMD_LEN);
             		CMD[strlen(CMD) - 2] = '\0';
             		printf("%s\n", CMD);
             		
             		dealCMD(CMD);
             		if (endflag == 0){
                 		break;
             		}
        	}
		while (result > 0);
        	logged = 0;
        	passed = 0;
        	printf("Connecting end\n");
        }
    }
}

int main(int argc, char* argv[])
{ 
    if ((sock=socket(AF_INET,SOCK_STREAM,0))<0){
       	perror("[err] Create socket failed!\n");
    }

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(SERV_PORT);
    int on = 1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); 
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr) )<0)  {
        perror("[err] Bind failed! \n");
    }
    
    listen(sock, 10);
        
    receiveCMD();
    return 1;
}





