/*****************************************************************
 Sockets Daemon Program
 
 This code was modified from Nigel Horspools, "The Berkeley
 Unix Environment".
 
 A daemon process is started on some host.  A socket is acquired
 for use, and it's number is displayed on the screen.  For clients
 to connect to the server, they muse use this socket number.
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

/* Use port number 0 so that we can dynamically assign an unused
 * port number. */
#define PORTNUM         0

/* Set up the location for the file to display when the daemon (okay,
 * server for you religious types) is contacted by the client. */
#define BBSD_FILE       "./test.bbs.file"
/*"/nfs/net/share/ftp/pub/class/330/test.file" */

/* Display error message on stderr and then exit. */
#define OOPS(msg)       {perror(msg); exit(1);}

#define MAXLINE 512

void * process_handler ();

int main()
{
    struct sockaddr_in saddr;       /* socket information */
    struct hostent *hp;     /* host information */
    char hostname[256];     /* host computer */
    socklen_t slen;         /* length socket address */
    int s;                  /* socket return value */
    int sfd;                /* socket descriptor returned from accept() */
    char ch[MAXLINE];       /* character for i/o */
    FILE *sf;               /* various file descriptors */
    int bbf;
    pthread_t thread;
    
    /*
     * Build up our network address. Notice how it is made of machine name + port.
     */
    
    /* Clear the data structure (saddr) to 0's. */
    memset(&saddr,0,sizeof(saddr));
    
    /* Tell our socket to be of the internet family (AF_INET). */
    saddr.sin_family = AF_INET;
    
    /* Aquire the name of the current host system (the local machine). */
    gethostname(hostname,sizeof(hostname));
    
    printf("Hostname: %s\n", hostname);
    
    /* Return misc. host information.  Store the results in the
     * hp (hostent) data structure.  */
    hp = gethostbyname(hostname);
    
    /* Copy the host address to the socket data structure. */
    memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
    
    /* Convert the integer Port Number to the network-short standard
     * required for networking stuff. This accounts for byte order differences.*/
    saddr.sin_port = htons(PORTNUM);
    
    /*
     * Now that we have our data structure full of useful information,
     * open up the socket the way we want to use it.
     */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
        OOPS("Could not create socket.\n");
    
    printf("Socket created.\n");
    
    /* Register our address with the system. */
    if(bind(s,(struct sockaddr *)&saddr,sizeof(saddr)) != 0)
        OOPS("Bind failed.\n");
    
    printf("Bind successful.\n");
    
    /* Display the port that has been assigned to us. */
    slen = sizeof(saddr);
    
    getsockname(s,(struct sockaddr *)&saddr,&slen);
    printf("Socket assigned: %d\n",ntohs(saddr.sin_port));
    
    /* Tell socket to wait for input.  Queue length is 1. */
    if(listen(s,5) != 0)
        OOPS("Could not listen.\n");
    
    /* Loop indefinately and wait for events. */
    for(;;)
    {
        sfd = accept(s,NULL,NULL);
        if(sfd == -1)
            OOPS("accept");
        
        pthread_create(&thread, NULL, process_handler, &sfd);
        pthread_detach(thread);
        
    }
    
    return 0;
}

void * process_handler (int* sfd)
{
    int num_char=MAXLINE;
    char arg1[MAXLINE];
    char arg2[MAXLINE];
    //int sfd;
    char ch[MAXLINE];       /* character for i/o */
    
    if (!*sfd) pthread_exit(0);
    //getting filename
    memset(&arg1[0], 0, sizeof(arg1));
    memset(&arg2[0], 0, sizeof(arg2));
    
    if(read(*sfd,arg1,7)<1)
    {
        printf("error reading arg1\n");
        memset(&arg1[0], 0, sizeof(arg1));
        pthread_exit(0);
    }
    printf("arg1 read: %s\n", arg1);
    
    if (strcmp(arg1, "listdir")==0)
    {
        int status;
        pid_t fork_return = fork();
        if (fork_return == 0) /* child process */
        {
            int bbf = open("tempFile", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            if (bbf < 0)
            {
                printf("error getfile opening");
                pthread_exit(0);
            }
            dup2(bbf, 1);  // make stdout go to file
            close(bbf);
            execl("/bin/ls","ls",NULL);
            exit(0);
        }
        else /* parent process */
        {
            wait(&status);
            struct stat fileStat;
            if(stat("tempFile",&fileStat) < 0)
            {
                printf("Error - Filestat\n");
                pthread_exit(0);
            }
            char *data = (char *) malloc(fileStat.st_size);
            int filedesc = open("tempFile", O_RDONLY);
            if (filedesc == -1)
                pthread_exit(0);
            if(read(filedesc, data, fileStat.st_size) < 0)
                printf("error reading file\n");
            if(close(filedesc) < 0)
            {
                printf("file close error\n");
                pthread_exit(0);
            }
            int bytes_sent;
            if((bytes_sent=send(*sfd, data, fileStat.st_size, 0))!= fileStat.st_size)
            {
                printf("Error - filesend\n");
                pthread_exit(0);
            }else
            {
                printf("bytes sent: %d & file size: %lld\n", bytes_sent, fileStat.st_size);
            }
            printf("No error sending file?\n");
        }
        printf("ls\n");
    }
    else
    {
        if(strcmp(arg1, "getfile")==0)
        {
            if(read(*sfd,arg2,20)<1)
            {
                printf("error reading arg2\n");
                pthread_exit(0);
            }
            printf("arg2 read: %s\n", arg2);
            
            struct stat fileStat;
            if(stat(arg2,&fileStat) < 0)
            {
                printf("Error - Filestat\n");
                pthread_exit(0);
            }
            char *data = (char *) malloc(fileStat.st_size);
            int filedesc = open(arg2, O_RDONLY);
            if (filedesc == -1)
                pthread_exit(0);
            if(read(filedesc, data, fileStat.st_size) < 0)
                printf("error reading file\n");
            if(close(filedesc) < 0)
            {
                printf("file close error\n");
                pthread_exit(0);
            }
            int bytes_sent;
            if((bytes_sent=send(*sfd, data, fileStat.st_size, 0))!= fileStat.st_size)
            {
                printf("Error - filesend\n");
                pthread_exit(0);
            }else
            {
                printf("bytes sent: %d & file size: %lld\n", bytes_sent, fileStat.st_size);
            }
            printf("No error sending file?\n");
            
        }
        else if(strcmp(arg1, "putfile")==0)
        {
            if(read(*sfd,arg2,5)<1)
            {
                printf("error reading arg2\n");
                pthread_exit(0);
            }
            printf("arg2 read: %s\n", arg2);
            
            int bbf = open(arg2, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            //getting data of file
            while((num_char=recv(*sfd,ch,MAXLINE,0)) > 0)
            {
                if (write(bbf,ch,num_char) < num_char)
                    OOPS("writing");
            }
            close(bbf);
        }
        else if(strcmp(arg1, "rmvfile")==0)
        {
            if(read(*sfd,arg2,20)<1)
            {
                printf("error reading arg2\n");
                pthread_exit(0);
            }
            printf("arg2 read: %s\n", arg2);
            pid_t fork_return = fork();
            if (fork_return == 0) /* child process */
            {
                execl("/bin/rm","rm",arg2,NULL);
                exit(0);
            }
        }
    }
    close(*sfd);
    pthread_exit(0);
}
