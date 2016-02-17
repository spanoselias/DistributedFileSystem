#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

//Format for error for the threads
#define perror2(s,e) fprintf(stderr, "%s:%s\n" , s, strerror(e))

void *bind_thread(void *port)
{
    int PORT=(int)port;

    //Socket descriptor for the Replica
    int         sock;
    //Store Client socket.
    int         newsock;
    //The size of the details for the accepted client
    int         clientlen;

    struct      sockaddr *clientPtr;

    struct      sockaddr_in serv_addr;
    //Declare&Initialize sockaddr_in pointer for accept function
    struct      sockaddr_in client_addr;
    struct      sockaddr *servPtr;

    //for setsockopt to be possible to reuse the port
    int allow=1;

    //Store pthread ID
    pthread_t tid;
    //Store threads errors
    int err;

    /*Initialize socket structure */
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(port);

    //Point to struct serv_addr.
    servPtr=(struct sockaddr *) &serv_addr;

    /*Create a socket and check if is created correct.*/
    if((sock=socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("Socket() failed"); exit(1);
    }

    //Allow to reuse the port
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &allow, sizeof(int)) == -1)
    {
        perror("\nsetsockopt\n");
        close(sock);
        pthread_exit((void *) 0);
    }


    //Bind socket to address //
    if(bind(sock,servPtr, sizeof(serv_addr)) < 0)
    {
        perror("Bind() failed");
        exit(1);
    }

    /*Listen for connections */
    /* MAXCLIENT. requests in queue*/
    if(listen(sock , MAXCLIENT) < 0)
    {
        perror("Listen() failed"); exit(1);
    }

    printf("\n**************Start to listen to port:%d*******************\n" , PORT);

    //Accept connection from clients forever.
    while(1)
    {

        clientPtr=(struct sockaddr *) &client_addr;
        clientlen= sizeof(client_addr);

        if((newsock=accept(sock , clientPtr  , &clientlen )) < 0)
        {
            perror("accept() failed"); exit(1);
        }


        if(err=pthread_create(&tid , NULL , &accept_thread ,  (void *) newsock))
        {
            perror2("pthread_create for accept_thread" , err);
            exit(1);
        }


    }//While(1)

}

void *accept_thread(void *accept_sock)
{
    //socket descriptor for each client
    int acptsock;
    //Retrieve the socket that passed from pthread_create
    acptsock= ((int)(accept_sock));


    //While the client is connect to the system you have to keep the connection
    while(1)
    {




    }//While 1

}

void signal_handler()
{
    printf("Signal Handled here\n");


    //exit the server
    exit(0);
}



int main(int argc , char  *argv[])
{

/***********************************************************************************/
/*                                   LOCAL VARIABLES                               */
/***********************************************************************************/

    //Store pthread ID
    pthread_t tid;
    //Store threads errors
    int         err;
    int         port;

    //Check of input arguments
    if(argc !=3)
    {
        printf("\nUsage: argv[0] -p [port]\n");
        exit(-1);
    }

    //Retrieve input parameters
    port=atoi(argv[2]);

    printf("--------------------------------------\n");
    printf("\nStarting File manager on port:%d ....\n" , port);
    printf("--------------------------------------\n");

    // SIGINT is signal name create  when Ctrl+c will pressed
    signal(SIGINT,signal_handler);

    //Create a thread for the bind.
    if(err=pthread_create(&tid , NULL ,(void *) &bind_thread , (void *)port))
    {
        perror2("pthread_create", err);
        exit(1);
    }

    //Wait until all threads to finish. Normally Bind thread
    //is always running
    pthread_join(tid , (void *) 0);

    return 0;
}//Main Function