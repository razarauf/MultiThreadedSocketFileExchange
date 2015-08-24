/*****************************************************************
 Sockets Client Program
 
 This code is a modified version taken from Nigel Horspool's "The Berkeley
 Unix Environment".
 
 This client connects to a specified server (host) and receives
 information from it.
 *****************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

/* Display error message on stderr and then exit. */
#define OOPS(msg)       {perror(msg); exit(1);}

#define MAXLINE 512

int main(int argc, char *argv[])
{
    struct sockaddr_in bba; /* socket information */
    struct hostent *hp;     /* host information */
    int slen;               /* host computer */
    int s;                  /* length socket address */
    int rfd;
    char ch[MAXLINE];       /* character for i/o */
    int num_char=MAXLINE;   /* number of characters */
    int port;               /* port to connect to */
    
    char portnum[20];
    char hostname[256];
    char *filename = (char *) malloc(sizeof(char)*128);
    printf("Hostname: ");
    scanf("%s", hostname);
    
    printf("Port number: ");
    scanf("%s", portnum);
    fseek(stdin,0,SEEK_END); //buffer filled with newline characters
    
    /* Clear the data structure (saddr) to 0's. */
    memset(&bba,0,sizeof(bba));
    
    /* Tell our socket to be of the internet family (AF_INET). */
    bba.sin_family = AF_INET;
    
    /* Acquire the ip address of the server */
    hp=gethostbyname(hostname);
    
    /* Acquire the port #. */
    port=atoi(portnum);
    
    /* Copy the server's address to the socket address data structure. */
    memcpy(&bba.sin_addr, hp->h_addr, hp->h_length);
    
    /* Convert the integer Port Number to the network-short standard
     * required for networking stuff. This accounts for byte order differences.*/
    bba.sin_port=htons(port);
    
    for (;;)
    {
        
        /* Now that we have our data structure full of useful information,
         * open up the socket the way we want to use it.
         */
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s == -1)
            OOPS("socket");
        if(connect(s,(struct sockaddr *)&bba,sizeof(bba)) != 0)
            OOPS("connect");
        
        printf("\n>");
        
        char *tempPtr = fgets(filename, 200, stdin);
        
        if(strcmp(filename, "exit\n")==0)
        {
            close(s);
            free (filename);
            filename = NULL;
            return 0;
        }
        
        printf("\ncommand entered: %s", filename);
        
        if (tempPtr != NULL)
        {
            int last_char = strlen (filename) - 1;
            
            if (tempPtr[last_char] == '\n')
                tempPtr[last_char] = '\0';
        }
        
        char * tmpPtr;
        char * arg1;
        char * arg2;
        
        tmpPtr = strtok (filename," ");
        arg1 = tmpPtr;
        printf("arg1 entered: %s\n", arg1);
        
        if (strcmp(arg1, "listdir")==0)
        {
            write(s, "listdir", strlen("listdir"));
            
            int bbf = open("tempFile", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            
            if (bbf < 0)
            {
                printf("error listdir opening");
                return 1;
            }
            //getting data of file
            while((num_char=recv(s,ch,MAXLINE,0)) > 0)
            {
                if (write(bbf,ch,num_char) < num_char)
                    OOPS("writing");
            }
            close(bbf);
        }
        else
        {
            tmpPtr = strtok (NULL," ");
            arg2 = tmpPtr;
            printf("arg2 entered: %s\n", arg2);
            
            if(strcmp(arg1, "getfile")==0)
            {
                write(s, arg1, strlen(arg1));
                write(s, arg2, strlen(arg2));
                
                printf("arg2 is: %s\n", arg2);
                
                printf("arg2 read: %s\n", arg2);
                
                int bbf = open(arg2, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
                
                if (bbf < 0)
                {
                    printf("error getfile opening");
                    return 1;
                }
                //getting data of file
                while((num_char=recv(s,ch,MAXLINE,0)) > 0)
                {
                    if (write(bbf,ch,num_char) < num_char)
                        OOPS("writing");
                }
                close(bbf);
            }
            else if(strcmp(arg1, "putfile")==0)
            {
                
                struct stat fileStat;
                if(stat(arg2,&fileStat) < 0)
                {
                    printf("Error - Filestat\n");
                    return 1;
                }
                
                write(s, arg1, strlen(arg1));
                write(s, arg2, strlen(arg2));
                
                char *data = (char *) malloc(fileStat.st_size);
                int filedesc = open(arg2, O_RDONLY);
                if (filedesc == -1){return 1;}
                if(read(filedesc, data, fileStat.st_size) < 0)
                    printf("error reading file\n");
                if(close(filedesc) < 0)
                {
                    printf("file close error\n");
                    return 1;
                }
                int bytes_sent;
                if((bytes_sent=send(s, data, fileStat.st_size, 0))!= fileStat.st_size)
                {
                    printf("Error - filesend\n");
                    return 1;
                }else
                {
                    printf("bytes sent: %d & file size: %lld\n", bytes_sent, fileStat.st_size);
                }
                printf("No error sending file?\n");
                //Freeing dynamically allocated memory
                free(data);
                data = NULL;
            }
            else if(strcmp(arg1, "rmvfile")==0)
            {
                write(s, arg1, strlen(arg1));
                write(s, arg2, strlen(arg2));
                printf("%s ", arg2);
            }
            else if(strcmp(arg1, "exit")==0)
            {
                printf("exiting\n");
            }
            else
                printf("none\n");
            memset(arg2, 0, sizeof(*arg2));
        }
        memset(filename, 0, sizeof(*filename));
        memset(tempPtr, 0, sizeof(*tempPtr));
        memset(tmpPtr, 0, sizeof(*tmpPtr));
        memset(arg1, 0, sizeof(*arg1));
        
        close(s);
    }
    
    return 0;
}
