/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 30/09/2015                                                                 */
/*Filename: directory.c                                                            */
/*                                                                                 */
/***********************************************************************************/
/***********************************************************************************/
/*                                     LIBRARIES                                   */
/***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h> //For bzero
#include <arpa/inet.h> //To retrieve the ip_add to asci
#include <string.h>
#include "directory.h"
#include <time.h>
#include <err.h>
#include <pthread.h>

#define GINT_TO_POINTER(i) ((gpointer) (glong) (i))
#define GPOINTER_TO_INT(p) ((gint)  (glong) (p))

#define MAX_PENDING 200 // Pending clients in


#define BUFSIZE 256

//Define the array size of metadata table
#define METATABLESIZE 10000

//Format for error for the threads
#define perror2(s,e) fprintf(stderr, "%s:%s\n" , s, strerror(e))

GSList* decode(struct message *msg , char *buf);

/***********************************************************************************/
/*                             GLOBAL VARIABLES                                    */
/***********************************************************************************/
struct sockaddr_in serv_addr;

FILE *log_fd = NULL; //File to store the log information of the directory server.
char *logfilename;

/*Lock the strtok*/
pthread_mutex_t locker;

//Lock metadata table
pthread_mutex_t lockermetadata;

//Lock clientCounter
pthread_mutex_t lockercliCoun;


/***********************************************************************************/
/*                             METADATA TABLE                                      */
/***********************************************************************************/
//Store all the metadata information by fileID
GPtrArray *metatable=NULL;

//Find the index of metadata for specific fileID
int findByFileID(char *filename , int *freePos)
{

   int index=-1;
   int i=0;
   //Pointer to the metadata
   struct metadata *point2metadata = NULL;

   for(i=0; i < metatable->len; i++)
   {
       //Retrieve  the data from the specific index
       point2metadata = (struct metadata *) g_ptr_array_index(metatable ,i );

       //Check if the filename of the position is what is looking for
       if(strcmp(filename , point2metadata->filename->str)==0)
       {
           index=i;
           break;
       }
   }

   return index;
}

GSList * insertList(GSList *metadata , GSList *curList )
{
    //A temp pointer to go through the curList
    GSList   *iter=NULL;

    //Go through available replicas
    for (iter = curList; iter; iter = iter->next)
    {
        if (g_slist_find(metadata, iter->data ) == NULL)
        {
            g_slist_prepend(metadata, iter->data );
        }
    }

    return metadata;
}

int IsMaxTag(struct message *tag2)
{
    //Store the error that pthread_mutex_lock failed
    int err;
    int funfreepos;
    int isFound=0;

    //Pointer to the metadata
    struct metadata *point2metadata = NULL;

    //Lock strtok due to is not deadlock free//
    if(err=pthread_mutex_lock(&lockermetadata))
    {
        perror2("Failed to lock()",err);
    }

            //Find the position for the specific filename
            int index=findByFileID( tag2->filename , &funfreepos);

            //Retrieve  the data from the specific index
            point2metadata = (struct metadata *) g_ptr_array_index(metatable , index );

            if ((point2metadata->tag.num < tag2->tag.num) \
                                        || (point2metadata->tag.num == tag2->tag.num && point2metadata->tag.id < tag2->tag.id ))
            {
                point2metadata->tag.num=tag2->tag.num;
                point2metadata->tag.id=tag2->tag.id;

                //Remove old replica set because now is unnecessary due to
                //the tag is old one
                g_slist_free ( point2metadata->replicaSet );
                point2metadata->replicaSet=NULL;

                //Point to new replica set with the latest tag
                point2metadata->replicaSet=tag2->replicaSet;

               isFound=1;
            }

            else if(point2metadata->tag.num == tag2->tag.num && point2metadata->tag.num == tag2->tag.id )
            {
                point2metadata->replicaSet = insertList( point2metadata->replicaSet , tag2->replicaSet);
            }

    //Unloack Mutex/
    if(err=pthread_mutex_unlock(&lockermetadata))
    {
        perror2("Failed to lock()",err);
    }

    return isFound;
}

/***********************************************************************************/
/*                        Initialization of variables & Sockets                    */
/***********************************************************************************/
void inisializations(int portIn)
{
    //Pointer for g_slist to store the replica that hold
    //the data & repli is which replica hold the data
    gpointer v;
    int      repli;

    //Initialize metadata array
    metatable = g_ptr_array_sized_new(1000);

    //Initialization of mutex for strtok in decode function//
    pthread_mutex_init(&locker,NULL);

    //Initialization of mutex for lock the metadata table
    pthread_mutex_init(&lockermetadata,NULL);

}

/***********************************************************************************/
/*              FUNCTION TO CONFIGURE A CONNECTION FOR THE SERVER                  */
/***********************************************************************************/
void *bind_thread(void *port)
{

    int PORT=(intptr_t)port;

    //Socket descriptor for the directory
    int sock;
    //Store server socket.
    int newsock;
    //The size of the details for the accepted client
    int clientlen;
    //Variable to store the thread id
    pthread_t tid;
    //Store the error of pthread_create if exist
    int err;
    //for setsockopt to be possible to reuse the port
    int allow=1;

    //Declare&Initialize sockaddr_in pointer for accept function
    struct sockaddr *clientPtr;
    struct sockaddr_in client_addr;

    /* Initialize socket structure */
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(PORT);

    //Pointer to Struct for the structure of the TCP/IP socket.
    struct sockaddr *servPtr;
    //Point to struct serv_addr.
    servPtr=(struct sockaddr *) &serv_addr;

    //Create a socket and check if is created correct.
    if((sock=socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
            perror("Socket() failed");
            pthread_exit((void *) 0);
    }

    //Allow to reuse the port
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &allow, sizeof(int)) == -1)
    {
        perror("\nsetsockopt\n");
        close(sock);
        pthread_exit((void *) 0);
    }

    //Accosiate address with the sock socket//
    if(bind(sock, servPtr, sizeof(serv_addr)) < 0)
    {
        perror("Bind() failed"); exit(1);
    }

    //Listen for connections//
    //MAX_PENDING:the number of client that can wait in the queue.//
    if(listen(sock ,MAX_PENDING) < 0)
    { /* 15 max. requests in queue*/
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

        if(err=pthread_create(&tid , NULL , &accept_thread ,  (void *)(intptr_t) newsock))
        {
            perror2("pthread_create for accept_thread" , err);
            exit(1);
        }

    }//While(1)

}//Function connect

/***********************************************************************************/
/*                            accept_thread                                        */
/***********************************************************************************/
void *accept_thread(void *accept_sock)
{
    //socket descriptor for each client
    int acpt_sock;
    //Buffer to send&receive data//
    char buf[256];
    //Received bytes
    int bytes;

    /*Declare the structure of the sending message in order
     * the client to communicate with server and vice versa*/
    struct message *msg;

    //Allocation memory
    msg = (struct message *) malloc(sizeof(struct message));

    //Retrieve the socket that passed from pthread_create
    acpt_sock = (intptr_t) accept_sock ;

 while(1)
 {
     /*If connection is established then start communicating */
     /*Initialize buffer with zeros */
     bzero(buf, sizeof(buf));
     /*Waiting...to receive data from the client*/
     if (recv(acpt_sock, buf, sizeof(buf), 0) < 0)
     {
         perror("Received() failed!");
         close(acpt_sock);
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
         close(acpt_sock);
         pthread_exit((void *) 0);
     }

     //Decode the message that receive from the client
     // in order to identify the type of the message//
     msg->replicaSet =  decode(msg, buf);

     //Directory ALGORITHM//

     //Check if the message if of type RREAD
     if ((strcmp("RREAD", msg->type) == 0))
     {
         int freepos = -1 , err=0;

         bzero(buf, sizeof(buf));

         //Lock strtok due to is not deadlock free
         if(err=pthread_mutex_lock(&lockermetadata))
         {
             perror2("Failed to lock()",err);
         }
            int index = findByFileID(msg->filename, &freepos);

         if (index < 0)
         {
             printf("Unable to find the file in metadata table\n");

         }
         //<RREAD-OK,tag.num , tag.id ,value,  MSG_ID >//
         //In case when the file exist
         else
         {
             //Pointer to the metadata
             struct metadata *point2metadata = NULL;

             //Retrieve  the data from the specific index
             point2metadata = (struct metadata *) g_ptr_array_index(metatable , index );

             int i = 0, len, repliValue;
             len = g_slist_length( point2metadata->replicaSet);
             //Read all replica that hold the data
             if (len != 0)
             {
                 //To point on replica set & go through all the list
                 GSList *iterator = NULL;
                 //To serialize replicaSet to string
                 char strSet[256];
                 char str[40];

                 //Initialize str
                 bzero(strSet, sizeof(strSet));

                 for (iterator = point2metadata->replicaSet; iterator; iterator = iterator->next)
                 {
                     bzero(str, sizeof(str));
                     sprintf(str, ",%d", GPOINTER_TO_INT(iterator->data));
                     strcat(strSet, str);
                 }
                 sprintf(buf, "RREAD-OK,%ld,%ld,%d%s,%ld,%ld", point2metadata->tag.num, point2metadata->tag.id, len,
                         strSet,
                         msg->msg_id, msg->fileID);
             }
             else
             {

                 printf("Unable to find the file:%s", msg->filename);
                 //sprintf(buf, "RREAD-OK,%d,%d,0,%d,%d", metatable[index].tag.num, metatable[index].tag.id, msg->msg_id,
                 //      msg->fileID);
             }
             //Unloack Mutex
             if (err = pthread_mutex_unlock(&lockermetadata))
             {
                 perror2("Failed to lock()", err);
             }
             // sprintf(buf,"RREAD-OK,%d,%d,%d,%d,%d",metatable[index].tag.num,metatable[index].tag.id,metatable[index].replicaSet,msg.msg_id , msg.fileID );

             //Send response to the client
             if (bytes = send(acpt_sock, buf, strlen(buf), 0) < 0)
             {
                 perror("Send() failed");
                 pthread_exit((void *) 0);
             }
         }
     }
     //Check if the message if of type RREAD
     else if ((strcmp("RWRITE", msg->type) == 0))
     {
         //Check if the tag that received if is greater
         if (IsMaxTag(msg) == 1)
         {
             printf("Found max Tag\n");
         }

         bzero(buf, sizeof(buf));
         sprintf(buf, "RWRITE-OK,%ld\n", msg->msg_id);
         if (bytes = send(acpt_sock, buf, strlen(buf), 0) < 0)
         {
             perror("Send() failed");
             pthread_exit((void *) 0);
         }

     }//RREAD RESPONSE

     else if (strcmp("WREAD", msg->type) == 0)
     {
                 int freepos = -1,err=0;

                 /*Lock strtok due to is not deadlock free*/
                 if(err=pthread_mutex_lock(&lockermetadata))
                 {
                     perror2("Failed to lock()",err);
                 }

                 //Find the index of the filename
                 int index = findByFileID(msg->filename, &freepos);

                 if(index != -1)
                 {
                     //Pointer to the metadata
                     struct metadata *point2metadata = NULL;

                     //Retrieve  the data from the specific index
                     point2metadata = (struct metadata *) g_ptr_array_index(metatable, index);

                     int i = 0, len, repliValue;
                     len = g_slist_length(point2metadata->replicaSet);
                     //Read all replica that hold the data
                     if (len != 0)
                     {
                         //To point on replica set & go through all the list
                         GSList *iterator = NULL;
                         //To serialize replicaSet to string
                         char strSet[256];
                         char str[40];

                         //Initialize str
                         bzero(strSet, sizeof(strSet));

                         for (iterator = point2metadata->replicaSet; iterator; iterator = iterator->next) {
                             bzero(str, sizeof(str));
                             sprintf(str, ",%d", GPOINTER_TO_INT(iterator->data));
                             strcat(strSet, str);
                         }
                         sprintf(buf, "WREAD-OK,%ld,%ld,%d%s,%ld,%ld", point2metadata->tag.num, point2metadata->tag.id, len,
                                 strSet,
                                 msg->msg_id, msg->fileID);
                     }
                     //In case when the replica set is NULL
                     else
                     {
                         sprintf(buf, "WREAD-OK,%ld,%ld,0,%ld,%ld", point2metadata->tag.num, point2metadata->tag.id,
                                  msg->msg_id, msg->fileID);

                     }
                 }

                 //If the filename does not exist in metadata table store it
                 else
                 {
                     //Allocate a new entry for the metadata table
                     struct metadata *entry = (struct metadata*) malloc(sizeof(struct metadata));

                     //Initialize new entry
                     entry->file_id = msg->fileID;
                     entry->tag.num = 1;
                     entry->tag.id  = 0;
                     entry->replicaSet=NULL;
                     entry->filename=g_string_new(NULL);
                     entry->permission=g_string_new(NULL);

                     //Insert the string the metadata table
                     g_string_assign( entry->filename ,  g_strdup( msg->filename ) );

                     //insert new entry in metadata table
                     g_ptr_array_add(metatable, (gpointer) entry );

                     //Find the index of the new entry
                     index = findByFileID(msg->filename, &freepos);

                     //Store permission for the file
                     g_string_assign( entry->permission ,  g_strdup( msg->permission ) );

                     //Pointer to the metadata
                     struct metadata *point2metadata = NULL;

                     //Retrieve  the data from the specific index
                     point2metadata = (struct metadata *) g_ptr_array_index(metatable , index );

                     bzero(buf, sizeof(buf));
                     sprintf(buf, "WREAD-OK,%ld,%ld,0,%ld,%ld", point2metadata->tag.num, point2metadata->tag.id, msg->msg_id,
                             point2metadata->file_id);

                 }

         //Unloack Mutex//
         if(err=pthread_mutex_unlock(&lockermetadata))
         {
             perror2("Failed to lock()",err);
         }


         if (bytes = send(acpt_sock, buf, strlen(buf), 0) < 0)
         {
             perror("Send() failed");
             pthread_exit((void *) 0);
         }

     }
     else if (strcmp("WWRITE", msg->type) == 0)
     {

         if (IsMaxTag(msg) == 1)
         {
             printf("\nFound new max tag");
         }

         bzero(buf, sizeof(buf));
         sprintf(buf, "WWRITE-OK,%ld\n", msg->msg_id);
         if (bytes = send(acpt_sock, buf, strlen(buf), 0) < 0)
         {
             perror("Send() failed");
             pthread_exit((void *) 0);
         }
     }

     //print the message that directory sent
      printf("\nSend:%s\n", buf);

 }//while(1)
     close(acpt_sock);
     pthread_exit((void *) 0);

 }

/***********************************************************************************/
/*                             DECODE MSG                                          */
/***********************************************************************************/
GSList* decode(struct message *msg , char *buf)
{
    //Store the thread error if exist
    int err;

    //Lock strtok due to is not deadlock free//
    if(err=pthread_mutex_lock(&locker))
    {
        perror2("Failed to lock()",err);
    }

        //To store a point to the set of replicas
        GSList *replicaSet=NULL;

        //Use comma delimiter to find the type of
         // send message and retrieve the apropriate
         // fields//
        msg->type=strdup(strtok(buf, ","));

        //Check if the type of the message is RWRITE//
        if( (strcmp(msg->type , "RWRITE" )== 0) || (strcmp(msg->type , "WWRITE" )== 0) )
            {
                int i=0 , len = 0 , repliValue;
                gpointer val;
                len = atoi(strtok(NULL, ","));
                //Read all replica that hold the data
                for(i=0; i<len; i++)
                {
                    repliValue = atoi(strtok(NULL, ","));
                    val=GINT_TO_POINTER(repliValue);
                    msg->replicaSet=g_slist_prepend(msg->replicaSet , val );
                }
                msg->tag.num=atoi(strtok(NULL,","));
                msg->tag.id=atoi(strtok(NULL,","));
                msg->msg_id=atoi(strtok(NULL,","));
                msg->filename=strdup(strtok(NULL,","));
                replicaSet = msg->replicaSet;
            }
        //Check if the type of the message is RREAD//
        else if(strcmp(msg->type , "RREAD" )== 0)
        {
            msg->msg_id=atoi(strtok(NULL,","));
            msg->filename=strdup(strtok(NULL,","));
        }
        else if(strcmp(msg->type , "WREAD" )== 0)
        {
            msg->msg_id=atoi(strtok(NULL,","));
            msg->fileID = atol(strtok(NULL,","));
            msg->filename=strdup(strtok(NULL,","));
            msg->permission = strdup(strtok(NULL,","));
        }

    //Unloack Mutex//
    if(err=pthread_mutex_unlock(&locker))
    {
        perror2("Failed to lock()",err);
    }

    return replicaSet;

}//Function decode message

/***********************************************************************************/
/*                              WRITE LOG                                          */
/***********************************************************************************/
int writeLog(char *info)
{
    log_fd=fopen(logfilename , "a+");
    fprintf(log_fd , info , sizeof(info));
    fclose(log_fd);
}//Function writeLog

/***********************************************************************************/
/*                            GET CURRENT DATE                                     */
/***********************************************************************************/
/*
char* set_date()
{
    char namelog[100];
    time_t t = time(NULL);
    struct tm tm=*localtime(&t);
    sprintf(namelog,"directory_%d_%d_%d.log",(tm.tm_mday),(tm.tm_mon+1),(tm.tm_year+1900));
    return namelog;
}//Fucntion setDate
*/

void signal_handler()
{
    printf("Signal Handled here\n");

    int i=0;

    //Pointer to the metadata
    struct metadata *point2metadata = NULL;

    for(i=0; i < metatable->len; i++)
    {
        //Retrieve  the data from the specific index
        point2metadata = (struct metadata *) g_ptr_array_index(metatable , i );

        //deallocations
        printf("[ Filename , TagNo , ClientID , FILEID , Permission ] \n");
        printf("[ %s       ,   %ld  ,    %ld    ,   %ld   ,     %s    ] \n" , point2metadata->filename->str , point2metadata->tag.num ,point2metadata->tag.id , point2metadata->file_id, point2metadata->permission->str);

        //deallocate list
        g_slist_free (point2metadata->replicaSet);

        //deallocate filename
        g_string_free (point2metadata->filename, TRUE);

        //deallocate permission string
        g_string_free (point2metadata->permission, TRUE);

        //deallocate struct
        free(point2metadata);
    }

    //exit the server
    exit(0);
}

/***********************************************************************************/
/*                                  MAIN                                           */
/***********************************************************************************/
int main(int argc , char *argv[])
{

/*********************************************/
/*           LOCAL DECLARATION               */
/********************************************/

   int port=-1; //Store the port that server listen

/*********************************************/
/*           MEMORY ALLOCATIONS              */
/********************************************/

    //Check if the input parameters are correct. //
    if(argc!=3)
    {
        printf("\nUsage: ./executable -p [port]\n");
        exit(-1);
    }

    //Store pthread ID
    pthread_t tid;
    //Store threads errors
    int err;

    /*store the port from the inputs*/
    port = atoi(argv[2]);

    //Inisialization
    inisializations(port);


    // SIGINT is signal name create  when Ctrl+c will pressed
    signal(SIGINT,signal_handler);
    //Handle segmentation corrupt
    signal(SIGSEGV, signal_handler);

    printf("----------------------------------------------------------");
    printf("\nDirectory: starting on port: %d...\n" ,  port);
    printf("----------------------------------------------------------\n");


    //Create a thread for the bind.
    if(err=pthread_create(&tid , NULL ,(void *) &bind_thread , (void *) (intptr_t)port))
    {
        perror2("pthread_create" , err);
        exit(1);
    }

    pthread_exit(NULL);

    return 0;
}//Main Function