/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 16/02/2016                                                                 */
/*Filename: filemanager.c                                                               */
/*                                                                                 */
/***********************************************************************************/
/***********************************************************************************/
/*                                     LIBRARIES                                   */
/***********************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include "filemanager.h"

/***********************************************************************************/
/*                                   MACROS                                        */
/***********************************************************************************/
#define MAXCLIENT 10000


//Format for error for the threads
#define perror2(s,e) fprintf(stderr, "%s:%s\n" , s, strerror(e))


/***********************************************************************************/
/*                             DATA STRUCTURES                                     */
/***********************************************************************************/
//Store all usernames for the clients
GPtrArray *clidata=NULL;


/***********************************************************************************/
/*                                      LOCKERS                                    */
/***********************************************************************************/
//Locker clientCounter
pthread_mutex_t lockercliCoun;


//locker for the decode function
pthread_mutex_t locker;


/***********************************************************************************/
/*                                 VARIABLES                                       */
/***********************************************************************************/
//Counter for client IDs
long countClientIds;

//Counter for File IDs
unsigned long countFileIds;


/***********************************************************************************/
/*                                 FUNCTIONS                                       */
/***********************************************************************************/
int decode(char *buf , HEADER *header )
{
    //Store the thread error if exist
    int err;

    //Lock strtok due to is not deadlock free//
    if(err=pthread_mutex_lock(&locker))
    {
        perror2("Failed to lock()",err);
    }

         //Use comma delimiter to find the type of
         //send message and retrieve the apropriate
         // fields*/
         header->type=strdup(strtok(buf, ","));

        if( strcmp(header->type , "REQCLIENTID" )== 0)
        {
            header->username=strdup(strtok(NULL,","));
        }

    //Unloack Mutex//
    if(err=pthread_mutex_unlock(&locker))
    {
        perror2("Failed to lock()",err);
    }

}

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

    //Buffer to send&receive data//
    char buf[256];

    //Received bytes
    int bytes;

    //Declare the structure of the sending message in order
    //the filemanager to communicate with the client and vice versa
    HEADER *msg = (HEADER *)malloc(sizeof(HEADER));


    //While the client is connect to the system you have to keep the connection
    while(1)
    {

        //If connection is established then start communicating //
        //Initialize buffer with zeros //
        bzero(buf, sizeof(buf));
        //Waiting...to receive data from the client//
        if (recv(acptsock, buf, sizeof(buf), 0) < 0)
        {
            perror("Received() failed!");
            close(acptsock);
            pthread_exit((void *) 0);
        }

        //Check if direc received a message
        if(strlen(buf) != 0)
        {
            /*Show the message that received.*/
            printf("----------------------------------\n");
            printf("accept_thread received: %s\n", buf);
        }
        else if (strlen(buf) == 0)
        {
            //printf("Unable to received messsage!\n");
            close(acptsock);
            pthread_exit((void *) 0);
        }


        if( strcmp(msg->type , "REQCLIENTID" )== 0)
        {
            bzero(buf,sizeof(buf));
            //encode the clientID
            sprintf(buf,"%ld" , registerClient(msg->username );
            if (send(acptsock, buf, 16 , 0) < 0 )
            {
                perror("Send:Unable to send clientID");
            }
        }

    }//While 1

}

unsigned long registerClient(char *username)
{
    //Counter for the index
    int i=0;

    //Store the error of mutex
    int err;

    //Check if the username exist in the data
    int isFound=0;

    //Pointer to the metadata
     CLIENT *point2client = NULL;

    //Check if the username exist in the array
    for(i=0; i < clidata->len; i++)
    {
        //Retrieve  the data from the specific index
        point2client = ( CLIENT *) g_ptr_array_index(clidata ,i );

        if(strcmp(username , point2client->username->str)==0)
        {
            //Indicate that username exist in the data
            isFound=1;

            //Stop the loop
            break;
        }

    }

    if(isFound==0)
    {

        //Allocate memory for the new entry
        CLIENT *entry = (CLIENT *)malloc(sizeof(CLIENT));

        //Insert the username the metadata table
        g_string_assign( entry->username , g_strdup(username));


        //Lock strtok due to is not deadlock free
        if(err=pthread_mutex_lock(&lockercliCoun))
        {
            perror2("Failed to lock()",err);
        }
            //Increase client ID
            countClientIds +=1;

            //Store new client ID in the struct
            entry->client_id = countClientIds;

        //Unloack Mutex
        if (err = pthread_mutex_unlock(&lockercliCoun))
        {
            perror2("Failed to lock()", err);
        }

        //insert new entry in client data table
        g_ptr_array_add(clidata, (gpointer) entry );

        return entry->client_id;
    }

    //If it found the client Id return it
    point2client->client_id;

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