/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 16/02/2016                                                                 */
/*Filename: filemanager.c                                                          */
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

//Store all metadata for the clients
GPtrArray *metadata=NULL;

/***********************************************************************************/
/*                                      LOCKERS                                    */
/***********************************************************************************/
//Locker clientCounter
pthread_mutex_t lockercliCoun;
pthread_mutex_t lockerFileids;


//locker for the decode function
pthread_mutex_t locker;

/***********************************************************************************/
/*                                 FUNCTIONS                                       */
/***********************************************************************************/
int decode(char *buf , FILEHEADER *header )
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
        else if( strcmp(header->type , "REQCREATE" )== 0)
        {
            header->filename = strdup(strtok(NULL,","));
            header->owner = atol( strtok(NULL,","));
        }
        else if( strcmp(header->type , "REQID" )== 0)
        {
            header->filename = strdup(strtok(NULL, ","));
        }
        else if( strcmp(header->type , "REQFILEID" )== 0)
        {
            header->filename = strdup(strtok(NULL,","));
            header->owner = atol( strtok(NULL,","));
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
    FILEHEADER *msg = (FILEHEADER *)malloc(sizeof(FILEHEADER));

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
            //Show the message that received//
            printf("----------------------------------\n");
            printf("accept_thread received: %s\n", buf);
        }
        else if (strlen(buf) == 0)
        {
            //printf("Unable to received messsage!\n");
            close(acptsock);
            pthread_exit((void *) 0);
        }

        //Decode the message that receive from the client
        //in order to identify the type of the message//
        decode(buf,msg);

        if( strcmp(msg->type , "REQCLIENTID" )== 0)
        {
            bzero(buf,sizeof(buf));
            //encode the clientID
            sprintf(buf,"%ld" , registerClient(msg->username ));
            if (send(acptsock, buf, 16 , 0) < 0 )
            {
                perror("Send:Unable to send clientID");
            }

            //Deallocations
            free(msg->username);
        }
        else if( strcmp(msg->type , "REQCREATE" )== 0 )
        {
            printf("Received Create: %s , owner:%ld\n" , msg->filename ,msg->owner);

            //Store the new file in the metadata
            //Also , retrieve the fileID for the file
            unsigned long fileid = registerFile(msg->filename , msg->owner );

            printf("REQCREATE Function return: %ld \n" , fileid);

            bzero(buf,sizeof(buf));
            //encode the clientID
            sprintf(buf,"%ld" , fileid );
            if (send(acptsock, buf, 64 , 0) < 0 )
            {
               perror("Send:Unable to send clientID");
            }
            //Deallocations
            free(msg->filename);
        }
        else if( strcmp(msg->type , "REQFILEID" )== 0 )
        {
            printf("Received REQFILEID: %s , owner:%ld\n" , msg->filename ,msg->owner);

            //Store the new file in the metadata
            //Also , retrieve the fileID for the file
            unsigned long fileid = lookUpFileID(msg->filename , msg->owner );

            printf("REQFILEID Function return: %ld \n" , fileid);

            bzero(buf,sizeof(buf));
            //encode the clientID
            sprintf(buf,"%ld" , fileid );
            if (send(acptsock, buf, 64 , 0) < 0 )
            {
                perror("Send:Unable to send clientID");
            }

            //Deallocations
            free(msg->filename);
        }

        //print the message that directory sent
        printf("\nSend:%s\n", buf);

    }//While

}

long registerClient(char *username)
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

        //Initialize gstring
        entry->username = g_string_new(NULL);

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
    return  point2client->client_id;
}

long registerFile(char *filename , long owner)
{
    //Counter for the index
    int i=0;

    //Store the error of mutex
    int err;

    //Store the fileid
    long tmpfileid=0;

    //Allocate memory for the new entry
    METADATA *entry = (METADATA *)malloc(sizeof(METADATA));

    //Initialize gstring
    entry->filename = g_string_new(NULL);

    //Insert the filename the metadata table
    g_string_assign( entry->filename , g_strdup(filename));

    //Insert create of the file in the metadata
    entry->owner = owner;

    //Lock strtok due to is not deadlock free
    if(err=pthread_mutex_lock(&lockerFileids))
    {
        perror2("Failed to lock()",err);
    }
        //Increase counter for fileIds
        countFileIds +=1;

        //Copy fileid for consistence purpose
        tmpfileid = countFileIds;

        printf("Function registerFile: %ld\n" , countFileIds);

    //Store new client ID in the struct
    entry->fileid = tmpfileid;

    //Unloack Mutex
    if (err = pthread_mutex_unlock(&lockerFileids))
    {
        perror2("Failed to lock()", err);
    }

    //insert new entry in client data table
    g_ptr_array_add(metadata, (gpointer) entry );

    printf("entry fileid: %ld\n" , entry->fileid);

    //Return the new fileID for the file
    return  entry->fileid;
}

long lookUpFileID(char *filename , long owner)
{
    //Counter for the index
    int i=0;

    //Store the error of mutex
    int err;

    //Check if the username exist in the data
    int isFound=0;

    //Look up for the fileid for the parameter filename
    long tmpfileid=-1;

    //Pointer to the metadata
    METADATA *point2meta = NULL;

    //Check if the username exist in the array
    for(i=0; i < metadata->len; i++)
    {
        //Retrieve  the data from the specific index
        point2meta = ( METADATA *) g_ptr_array_index(clidata ,i );

        //Check if it found the filename that it looking for
        if(strcmp(filename , point2meta->filename->str)==0)
        {
            tmpfileid = point2meta->fileid;

            //Stop the loop
            break;
        }
    }

    return tmpfileid;
}

void initialization()
{
    //Initialize metadata array
    clidata = g_ptr_array_sized_new(10);

    //Store metadata for all the files
    metadata = g_ptr_array_sized_new(100);

    //initialize clientsIDS
    countClientIds=0;

    //Initialize fileIDS
    countFileIds=0;

    //Initialization of mutex for strtok in decode function//
    pthread_mutex_init(&locker,NULL);

    //Initialization of mutex for lock the metadata table
    pthread_mutex_init(&lockercliCoun,NULL);

    //Initialization of mutex for lock the metadata table
    pthread_mutex_init(&lockerFileids,NULL);

}

void signal_handler()
{
    printf("Signal Handled here\n");

    int i=0;

    printf("\nd****************************************\n");
    printf("CLIENTS CREDENTIALS\n");
    printf("****************************************\n");

    //Pointer to the metadata
    CLIENT *point2cli = NULL;

    //Deallocate all the memory
    for(i=0; i < clidata->len; i++)
    {
        //Retrieve  the data from the specific index
        point2cli = (CLIENT *) g_ptr_array_index(clidata , i );

        //deallocations
        printf("Username: %s , ClientID: %ld \n" , point2cli->username->str , point2cli->client_id );

        //deallocate filename
        g_string_free (point2cli->username, TRUE);

        //deallocate struct
        free(point2cli);
    }

    //Point to metadata array
    METADATA *point2meta = NULL;


    printf("\n****************************************\n");
    printf("FILES METADATA CREDENTIALS\n");
    printf("****************************************\n");

    //Deallocate all the memory
    for(int i=0; i < metadata->len; i++)
    {
        //Retrieve  the data from the specific index
        point2meta = (METADATA *) g_ptr_array_index(metadata , i );

        //deallocations
        printf("Filename: %s , Fileid: %lu , owner: %ld \n" , point2meta->filename->str , point2meta->fileid , point2meta->owner );

        //deallocate filename
        g_string_free (point2meta->filename , TRUE);

        //deallocate struct
        free(point2meta);
    }

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

    //Initialize
    initialization();

    //Retrieve input parameters
    port=atoi(argv[2]);

    printf("--------------------------------------\n");
    printf("\nStarting File manager on port:%d ....\n" , port);
    printf("--------------------------------------\n");

    // SIGINT is signal name create  when Ctrl+c will pressed
    signal(SIGINT,signal_handler);
    //Handle segmentation corrupt
    signal(SIGSEGV, signal_handler);

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