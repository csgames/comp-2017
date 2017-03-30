#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

// Taille du buffer pour la lecture du socket
int BUFFER_SIZE = 2048;

int validation(char* username, char* password)
{
    int i = strlen(username);
    int j = strlen(password);
    int good = 1;

    if(i == j && i >= 5 && !(strcmp(username,"Admin")))
    {
        for(int k=0;k<i;k++)
        {
            if (!((unsigned int)username[k] == (unsigned int)password[k]+1))
            {
                good = 0;
            }
        }

    }
    else
    {
        good = 0;
    }

    if(good)
        printf("[+] Key is valid\n");
    else
        printf("[-] Key isnt valid\n");
    fflush(stdout);
    return good;
}

int login(int sock, char* username)
{
    int auth = 0;
    char password[1024];
    char message[1024];

    bzero(password,1024);
    bzero(message,1024);

    printf("[+] Login of user %s\n",username);
    fflush(stdout);

    read(sock,password,1024);
    printf("[+] Password validation ( %s ) for the user: %s\n",password,username);
    fflush(stdout);

    auth = validation(username,password);

    if(auth)
        strcpy(message,"The key is correct [login_flag]");
    else
        strcpy(message,"The key is incorrect");
    write(sock,message,sizeof(message));

    return auth;
}

void listFiles(int sock)
{
    printf("[+] User is listing files\n");
    DIR *d;
    struct dirent *dir;
    char fileName[BUFFER_SIZE];
    char* encodedName;

    d = opendir("./srv");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            bzero(fileName,BUFFER_SIZE);
            if((strcmp(dir->d_name,".")) && (strcmp(dir->d_name,"..")))
            {
                strcpy(fileName,dir->d_name);
                printf("[+] Sending %s\n",fileName);
                Base64Encode(&fileName,strlen(fileName),&encodedName);
                write(sock,encodedName,strlen(encodedName));
                write(sock,"\x03",1);
            }
        }
        closedir(d);
        write(sock,"\x04",1);
    }
}

void downloadFile(int sock, char* fileName)
{
    printf("[+] The user is downloading a file\n");

    char path[BUFFER_SIZE+6];
    strcpy(path,"./srv/");
    strcat(path,fileName);

    char *source = NULL;
    FILE *fp = fopen(path, "r");
    long bufsize = 0;

    char* encodedContent;
    if (fp != NULL)
    {
        // Go to the end of the file.
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            // Get the size of the file.
            bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            // Allocate our buffer to that size.
            source = malloc(sizeof(char) * (bufsize + 1));
            bzero(source,sizeof(char) * (bufsize + 1));

            // Go back to the start of the file.
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            // Read the entire file into memory.
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 )
            {
                perror("[-] Error reading file");
            }
            else
            {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }
        Base64Encode(source,sizeof(char) * (bufsize),&encodedContent);
        write(sock,encodedContent,strlen(encodedContent));
        write(sock,"\x04",1);
        fclose(fp);

    }
    free(source);
}

void uploadFile(int sock, char* fileName)
{
    FILE *fp;
    char buffer[BUFFER_SIZE];
    char path[BUFFER_SIZE+6];

    char* decodedContent;
    size_t decoded_length;

    bzero(buffer,BUFFER_SIZE);
    strcpy(path,"./srv/");
    strcat(path,fileName);

    printf("[+] The user is uploading a file\n");
    fp=fopen(path, "w");
    if(fp == NULL)
            exit(-1);

    read(sock,buffer,BUFFER_SIZE);

    Base64Decode(buffer, &decodedContent, &decoded_length);
    fwrite(decodedContent, decoded_length, 1, fp);
    fclose(fp);
    write(sock,"Upload done [upload_flag]",11);
}

void processClient (int sock)
{
    int n;
    char buffer[BUFFER_SIZE];
    char message[1024];

    unsigned int idCmd;
    int authenticated = 0;

    bzero(buffer,BUFFER_SIZE);
    bzero(message,1024);

    char banner[] = "### SRC GUARD 2017 ###";
    write(sock,banner,sizeof(banner));

    while(1)
    {
        bzero(buffer,BUFFER_SIZE);
        bzero(message,1024);
        n = read(sock,buffer,BUFFER_SIZE);
        if (n < 0)
        {
            perror("[-] Session ended");
            exit(1);
        }

        //printf("Here is the message: %s\n",buffer);
        idCmd = (unsigned int) buffer[0];
        printf("[+] Command Id: %i\n",idCmd);

        switch(idCmd)
        {
            // Exit
            case 0:
                printf("[+] Close session\n");
                fflush(stdout);
                strcpy(message,"Session closing...");
                n = write(sock,message,sizeof(message));
                return;
            // Login
            case 1:
                if(authenticated)
                {
                    printf("[+] The user is already authenticated\n");
                    fflush(stdout);
                    strcpy(message,"You are already authenticated");
                    n = write(sock,message,sizeof(message));
                }
                else
                {
                    strcpy(message,"Please provide a valid key");
                    n = write(sock,message,sizeof(message));
                    authenticated = login(sock,&buffer[1]);
                }
                break;
            // List files
            case 2:
                if(authenticated)
                    listFiles(sock);
                break;

            // Read file
            case 3:
                if(authenticated)
                    downloadFile(sock,&buffer[1]);
                break;
            // Write file
            case 4:
                if(authenticated)
                    uploadFile(sock,&buffer[1]);
                break;
        }

        if (n < 0)
        {
            perror("[-] ERROR writing to socket");
            exit(1);
        }
    }
}

int main( int argc, char *argv[] )
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("[-] ERROR opening socket");
        exit(1);
    }
    int enable = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("[-] ERROR setting SO_REUSEADDR");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 4919;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
         perror("[-] ERROR on binding");
         exit(1);
    }

    /* Now start listening for the clients, here
     * process will go in sleep mode and will wait
     * for the incoming connection
     */
    listen(sockfd,5);
    printf("[+] Server is now listening\n");
    clilen = sizeof(cli_addr);
    while (1)
    {
        newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        {
            perror("[-] ERROR on accept");
            exit(1);
        }

        /* Create child process */
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("[-] ERROR on fork");
            exit(1);
        }
        if (pid == 0)
        {
            /* This is the client process */
            close(sockfd);
            printf("[+] Got a new connection\n");
            processClient(newsockfd);
            exit(0);
        }
        else
        {
            close(newsockfd);
        }
    } /* end of while */
}
