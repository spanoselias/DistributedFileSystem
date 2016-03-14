/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 30/09/2015                                                                 */
/*Filename: client.c                                                               */
/*                                                                                 */
/***********************************************************************************/
/***********************************************************************************/
/*                                     LIBRARIES                                   */
/***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "client1.h"
#include <time.h>
#include <error.h>
#include <gtk-2.0>


/***********************************************************************************/
/*                                   MACROS                                        */
/***********************************************************************************/
#define SUCCESS 1
#define FAILURE -1

#define MAXBUF 99999
#define BUF_SIZE 256 //Size of the buffer


#define GINT_TO_POINTER(i) ((gpointer) (glong) (i))
#define GPOINTER_TO_INT(p) ((gint)  (glong) (p))

/***********************************************************************************/
/*                             GLOBAL VARIABLES                                    */
/***********************************************************************************/
FILE *LOG_FILE = NULL;                       //File to store the log information
char msg_info[256];                          //Buffer to store message for the log file
fd_set direcs_fds;                           //Set of socket descriptor for select for directories
fd_set replicas_fd;                          //Set of socket descriptor for select for replicas
fd_set crash_direc_fds;                      //To store all the crash directories that exist in the system
fd_set crash_repli_fds;                      //To store all the crash replicas that exist in the system
int max_direc_fd=-1;                         //To store the max directory socket descriptor for the SELECT
int max_replica_fd=-1;                       //To store the max replica socket descriptor for the SELECT

long message_id;                              //Store the message counter

int    *direcSocks;                          //Store all the sockets for directories
int    *replicaSocks;                        //Store all the sockets for replicas
int    *filemanagerSocks;                    //Store all the sockets for filemanagers
struct sockaddr_in *direc_sockaddr;          //Store all the ips and ports for directories
struct sockaddr_in *replica_sockaddr;        //Store all the ips and ports for Replicas
struct sockaddr_in *filemanager_sockaddr;    //Store all the ips and ports for filemanagers
struct serinfo *direcNodes;                  //Store IP and port for each directory server
struct serinfo *replicaNodes;                //Store IP and port for each replica server
struct serinfo *filemanagerNodes;            //Store IP and port for each filemanager
int    MAX_DIRECTORIES=0;                    //Store the number of directories that exist in the system
int    MAX_REPLICAS=0;                       //Store the number of replicas that exist in the system
int    MAX_FILEMANAGERS;                     //Store the number of filemanagers that exist in the system
int    *repliVals;

long   clientID=0;                           //Store clientID

double    failrate=1;                        //store the rate for the fail

GHashTable * hashFiletags;                   //Metadata table that holds all tag for the data

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
void shuffle(int *array, size_t n)
{
    srand(time(NULL));

    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int isMaxTag(struct TAG *tag1 , struct TAG tag2 )
{

    if ((tag1->num < tag2.num) || (tag1->num == tag2.num  && tag1->clientID <  tag2.clientID)  )
    {
        return 1;
    }

    return 0;
}

char *checksum_get(char *filename)
{

    int MAX_SIZE=900000;

    GChecksum   *cs;
    guchar       data[MAX_SIZE];
    gsize        size = 0;
    FILE        *input;

    const char *sum;

    cs = g_checksum_new( G_CHECKSUM_MD5 );
    input = fopen( filename, "rb" );
    do
    {
        size = fread( (void *)data, sizeof( guchar ), MAX_SIZE, input );
        g_checksum_update( cs, data, size );
    }
    while( size == MAX_SIZE );
    fclose( input );

    sum = g_checksum_get_string( cs );
    // g_checksum_free( cs );

    return (char*)sum;

}

/***********************************************************************************/
/*                             decode message                                      */
/***********************************************************************************/
GSList* decode(struct message *msg , char *buf)
{
    //Use comma delimiter to find the type of
    //send message and retrieve the apropriate
    //fields//
    msg->type=strdup(strtok(buf, ","));

    //Check if the type of the message is RREAD-OK//
    if( (strcmp(msg->type , "RREAD-OK" )== 0) )
    {
        int i=0 , len= 0 , repliValue;
        gpointer val;

        msg->tag.num = atoi(strtok(NULL, ","));
        msg->tag.clientID = atoi(strtok(NULL, ","));
        len = atoi(strtok(NULL, ","));
        //Read all replica that hold the data
        for(i=0; i<len; i++)
        {
           repliValue = atoi(strtok(NULL, ","));
           val=GINT_TO_POINTER(repliValue);
           msg->replicaSet=g_slist_prepend(msg->replicaSet , val );
        }
        msg->msg_id = atoi(strtok(NULL, ","));
        msg->fileID = atoi(strtok(NULL, ","));
        return msg->replicaSet;
    }

    else if((strcmp(msg->type , "WREAD-OK" )== 0))
    {

        int i=0 , len= 0 , repliValue;
        gpointer val;

        msg->tag.num = atoi(strtok(NULL, ","));
        msg->tag.clientID = atoi(strtok(NULL, ","));
        len = atoi(strtok(NULL, ","));
        //Read all replica that hold the data
        for(i=0; i<len; i++)
        {
            repliValue = atoi(strtok(NULL, ","));
            val=GINT_TO_POINTER(repliValue);
            msg->replicaSet=g_slist_prepend(msg->replicaSet , val );
        }
        msg->msg_id = atoi(strtok(NULL, ","));
        msg->fileID = atoi(strtok(NULL, ","));

        return msg->replicaSet;
    }

    //Check if the type of the message is RWRITE
    else if((strcmp(msg->type , "RWRITE-OK" )== 0))
    {
        //msg->tag.num =atoi(strtok(NULL, ","));
       // msg->tag.id = atoi(strtok(NULL, ","));
        msg->msg_id = atoi(strtok(NULL, ","));
        //msg->fileID = atoi(strtok(NULL, ","));
    }

    else if((strcmp(msg->type , "WWRITE-OK" )== 0))
    {
        msg->msg_id = atoi(strtok(NULL, ","));
    }
    else if((strcmp(msg->type , "READ-OK" )== 0))
    {
        msg->tag.num = atoi(strtok(NULL, ","));
        msg->tag.clientID = atoi(strtok(NULL, ","));
        msg->msg_id = atoi(strtok(NULL, ","));
        msg->fileSize = atoi(strtok(NULL, ","));
        msg->checksum = strdup(strtok(NULL, ","));
    }
    else if((strcmp(msg->type , "WRITE-OK" )== 0))
    {
        msg->msg_id = atoi(strtok(NULL, ","));
        msg->filename = strdup(strtok(NULL, ","));
    }
    else if((strcmp(msg->type , "REQCLIENTID" )== 0))
    {
        msg->clientID = atol(strtok(NULL, ","));
        msg->msg_id =  atol(strtok(NULL, ",")) ;
    }
    else if((strcmp(msg->type , "REQCREATE" )== 0))
    {
        msg->fileID = atol(strtok(NULL, ","));
        msg->msg_id =  atol(strtok(NULL, ","));
    }
    else if((strcmp(msg->type , "REQFILEID" )== 0))
    {
        msg->fileID = atol(strtok(NULL, ","));
        msg->msg_id =  atol(strtok(NULL, ","));
    }


    return NULL;

}//Function decode message

/***********************************************************************************/
/*                            encode message                                       */
/***********************************************************************************/
void encode(struct message *msg , char *buf , char *type)
{

    bzero(buf,sizeof(buf));

    //Check if the message is secure
    if((strcmp(type , "SECURE" )== 0) )
    {
        sprintf(buf,"%s,%ld,%ld,%ld,%s,%s" ,type , msg->tag.num , msg->tag.clientID, message_id , msg->filename , msg->filetype );
    }

    //Check if the type of the message is RREAD
    if((strcmp(type , "RREAD" )== 0) )
    {
        sprintf(buf,"%s,%ld,%s" ,type , message_id , msg->filename);
    }
    //Check if the type of the message is RWRITE
    else if( (strcmp(type , "RWRITE" )== 0 ) || (strcmp(type , "WWRITE" )== 0 ) )
        {
            int i=0 , len , repliValue ;
            len=g_slist_length(msg->replicaSet);
            //Read all replica that hold the data
            if( len != 0 )
            {
                //To point on replica set & go through all the list
                GSList* iterator=NULL;
                //To serialize replicaSet to string
                char strSet[256];
                char str[40];

                //Initialize str
                bzero(strSet,sizeof(strSet));

                for (iterator = msg->replicaSet; iterator; iterator = iterator->next)
                {
                    bzero(str,sizeof(str));
                    sprintf(str,",%d" , GPOINTER_TO_INT(iterator->data) );
                    strcat(strSet,str);
                }
                sprintf(buf, "%s,%d%s,%ld,%ld,%ld,%s", type ,len, strSet ,msg->tag.num, msg->tag.clientID, message_id , msg->filename);

            }
            else
            {
                sprintf(buf, "%s,0,%ld,%ld,%ld,%s", type , msg->tag.num, msg->tag.clientID, message_id , msg->filename);
            }

        }//If RWRITE statment

    else if((strcmp(type , "WWRITE" )== 0 ))
    {
        int i=0 , len , repliValue ;
        len=g_slist_length(msg->replicaSet);
        //Read all replica that hold the data
        if( len != 0 )
        {
            //To point on replica set & go through all the list
            GSList* iterator=NULL;
            //To serialize replicaSet to string
            char strSet[256];
            char str[40];

            //Initialize str
            bzero(strSet,sizeof(strSet));

            for (iterator = msg->replicaSet; iterator; iterator = iterator->next)
            {
                bzero(str,sizeof(str));
                sprintf(str,",%d" , GPOINTER_TO_INT(iterator->data));
                strcat(strSet,str);
            }
            sprintf(buf, "%s,%d%s,%ld,%ld,%ld,%s", type ,len, strSet ,msg->tag.num, msg->tag.clientID, message_id , msg->filename);

        }
        else
        {
            sprintf(buf, "%s,0,%ld,%ld,%ld,%s", type , msg->tag.num, msg->tag.clientID, message_id , msg->filename);
        }
    }

    else if( (strcmp(type , "WREAD" )== 0))
    {
        sprintf(buf,"%s,%ld,%ld,%s,L1" ,type , message_id , msg->fileID , msg->filename   ) ;
    }


}//Function decode message

/***********************************************************************************/
/*                             RECEIVE MAJORITY                                   */
/***********************************************************************************/
//The isCheck parameter is a set with domain {0,1,2}
//In case where you are interest to know the replica you have to
//specify the variable is check as 1.If your are interested only
//for tag you have to specify just only 2
GSList* receive_quorum(struct TAG *maxTag, GSList *replicaSet,int ischeck , int *ismajor)
{
    int bytes=0;            //To validate the size of the received msg.
    struct timeval tv;      //To set a time out for the select
    int i;                  //For loop statment
    char buffer[512];
    int readsocks=0;
    int majority=0;         //Count the acks from the directories.
    int IsComplete=0;
    (*ismajor) = -1;

    //To store the message that receive from each server//
    struct message *msg;

    //Allocation memory
    msg=(struct message *)malloc(sizeof(struct message));

    //initialization
    msg->replicaSet = NULL;

    /*Initialize tag*/
    //msg.tag.num=0;
    //msg.tag.id=0;

    //Sleep x microsecond
    //usleep(10000);

    //set the end time to the current time plus 5 seconds
    time_t endTime = time(NULL) + 5;

    usleep(5000);
    while(time(NULL) < endTime)
    {
        // if(result < 0){exit(-1);}
        for(i=0; i <MAX_DIRECTORIES; i++)
        {
            bzero(buffer, sizeof(buffer));
            if ((bytes = recv(direcSocks[i], buffer, sizeof(buffer) , MSG_DONTWAIT)) < 0)
            {
                //perror("Failed client received the message");
                continue;
            }
            if(strlen(buffer) != 0 )
            {
                //printf("MessageDirecMajor: %s\n", buffer);

                //Decode the message that you received from server//
                msg->replicaSet=decode(msg, buffer);

                //Check if you received the message that you wait..otherwise drop the msg//
                if (msg->msg_id == message_id)
                {
                    ++IsComplete;
                    /*In case you wait the tag&value with the response message
                     * otherwise it's not nessesary to wait something*/
                    if (ischeck ==1)
                    {
                       if( isMaxTag(maxTag , msg->tag) == 1)
                       {
                            (maxTag->num) = msg->tag.num;
                            (maxTag->clientID) = msg->tag.clientID;
                            GSList* iterator = NULL;

                            //Store all replica that hold the data
                            for (iterator = msg->replicaSet; iterator; iterator = iterator->next)
                            {
                                //Check if the replica exist in replicaSet if does not exist
                                //insert it
                                if(g_slist_find(replicaSet,iterator->data) == NULL)
                                {
                                    replicaSet=g_slist_append(replicaSet, iterator->data);
                                }
                            }
                        }
                    }
                    else if(ischeck==2)
                    {
                        if( isMaxTag(maxTag , msg->tag) == 1)
                        {
                            (maxTag->num) = msg->tag.num;
                            (maxTag->clientID) = msg->tag.clientID;
                        }
                    }
                    /*condition ? valueIfTrue : valueIfFalse*/
                }
                /*  #ifdef DEVMODE
                  else
                  {
                      printf("Old message received, msg_id:%d",msg.msg_id);
                  }
                  #endif

                  printf("Before going to is_major:%d\n" , is_major );
                 */
            }//For statment
        }//For statment

        if(IsComplete  >=  ( floor((MAX_DIRECTORIES/2)) +1))
        {
            break;
        }
        usleep(500000);
    }

    if(IsComplete >= (floor(MAX_DIRECTORIES/2)+1))
    {
      //  printf("Received Directory Quorum\n");
        (*ismajor) = 1;
        return replicaSet;
    }

    return NULL;
}//Function recv_majority

GSList* recvrepliquorum(struct message *msg , GSList *replicaSet)
{
    int bytes=0;            //To validate the size of the received msg.
    struct timeval tv;      //To set a time out for the select
    int i;                  //For loop statment
    char buffer[512];
    int readsocks=0;
    int majority=0;         //Count the acks from the directories.
    int IsComplete=0;
    int fd=0;


    // set the end time to the current time plus 5 seconds
    time_t endTime = time(NULL) + 5;

    /*To store the message that receive from each server*/
    //struct message *msg;
    //Allocation memory
    //msg=(struct message *)malloc(sizeof(struct message));

    usleep(5000);
    while(time(NULL) < endTime)
    {
        for(i=0; i <MAX_REPLICAS; i++)
        {
               bzero(buffer, sizeof(buffer));
               if ((bytes = recv(replicaSocks[i], buffer, sizeof(buffer), MSG_DONTWAIT)) < 0)
               {
                   // perror("Failed client received the message");
                    continue;
               }
               if(strlen(buffer) != 0 )
               {
                   // printf("ReplicaQuorumReceived:%s\n",buffer);

                    //Decode the message that you received from server*/
                    decode(msg, buffer);

                    //Check if you received the message that you wait..otherwise drop the msg*/
                    if (msg->msg_id == message_id)
                    {
                        //Count how many acknowledge received so far
                        ++IsComplete;

                        //Store all replica that hold the data
                        replicaSet=g_slist_append(replicaSet, GINT_TO_POINTER(i));
                    }
               }//For
        }//For
        //Check if you receive quorum of replicas
        if(IsComplete >=  ( ceil((MAX_REPLICAS *failrate) ) ))
        {
            break;
        }
        //sleep x second for avoid congestion in cpu utilization
        usleep(500000);
    }

    if( IsComplete >= (ceil( ( MAX_REPLICAS * failrate ) ) ) )
    {
      //printf("Received Replica Quorum\n");
        return replicaSet;
    }

    return NULL;
}

GHashTable *storefiletag(char *filename, struct TAG *filetag )
{
    //Create a new tag to be insert in list
    struct TAG* newfiletag = g_new(struct TAG,1);
    newfiletag->num = filetag->num ;
    newfiletag->clientID = filetag->clientID ;

    //Point to the tag for the specific filename
    struct TAG *pointTag=NULL;

    //Retrieve all the tags which are associated withe the specific filename
    pointTag =(struct TAG*) g_hash_table_lookup(hashFiletags , filename);

    if(pointTag != NULL)
    {
        //Delete the old one tag and replace it with the new one
        free(pointTag);
    }

    //Insert updated list in hashtable to the associated key(filename)
    g_hash_table_insert(hashFiletags , g_strdup( filename ) , newfiletag);

    return hashFiletags;
}

/***********************************************************************************/
/*                             READ_QUERY FUNCTION                                 */
/***********************************************************************************/
GSList  *read_Query(struct cmd *cmdmsgIn  ,  struct TAG *tag , struct message *msg )
{
    int IsSuccess=1;

    char buf[512];
    int i;

    int isReceiveMajor;

    //The replicas that hold the file
    GSList  *setOfReplica=NULL;

    //delete
    tag->num=0;
    tag->clientID=0;

    //Retrieve information about the file
    msg->fileID= cmdmsgIn->fileid;
    msg->filename=strdup(cmdmsgIn->filename);
    msg->filetype=strdup(cmdmsgIn->fileType);

    //Initialize buffer
    bzero(buf, sizeof(buf));
    encode(msg, buf , "RREAD");

    //Broadcast to all directories to request the data with the
    //specific ID
    for(i=0; i < MAX_DIRECTORIES; i++)
    {
        if (send(direcSocks[i], buf, sizeof(buf), 0) < 0)
        {
            perror("send() failed\n");
            //exit(EXIT_FAILURE);
        }
    }//For statment

    //Wait until to receive a quorum the last tag with a set of replica that
    //hold the data
    setOfReplica=receive_quorum(tag , setOfReplica , 1 , &isReceiveMajor );
    if( isReceiveMajor != 1 )
    {
        printf("--------------------------------------\n");
        printf("\nUNABLE TO RECEIVE DIRECTORY QUORUM\n");
        printf("--------------------------------------\n");
        return NULL;
    }

    #ifdef DEVMODE
        printf("TagNum:%d\n" , tag->num);
        printf("TagID:%d\n" , tag->clientID);
        printf("Directory READER SECOND PHASE\n");
    #endif

    //Return the uptodate replicas that stored the file
    return setOfReplica;
}

/***********************************************************************************/
/*                             READ_INFORM FUNCTION                                */
/***********************************************************************************/
int read_Inform( struct cmd *cmdmsgIn  ,  struct TAG *tag , struct message *msg , GSList  *setOfReplica  )
{

    //Buffer to store the message that it will sent
    char buf[512];

    //index for the for
    int i;

    //Variable that indicate if it receive majority
    int isReceiveMajor;

    //Variable that indicate if it receive correctly the file
    int IsSuccess=1;

    //Second phase of the Directory
    // Announce the maxtag & value that you found in order
    // guaranties strong consistent //
    msg->tag.num=tag->num;
    msg->tag.clientID=tag->clientID;
    msg->replicaSet = setOfReplica;
    msg->msg_id= (++message_id);

    bzero(buf, sizeof(buf));
    encode(msg , buf , "RWRITE" );
    for(i=0; i < MAX_DIRECTORIES; i++)
    {
        if (send(direcSocks[i], buf, sizeof(buf), 0) < 0)
        {
            perror("send() failed\n");
        }
    }//For statment

    receive_quorum(tag,setOfReplica,0 , &isReceiveMajor);
    if( (isReceiveMajor) != 1)
    {
        printf("-------------------------------------------------------\n");
        printf("UNABLE TO READ QUORUM IN THE SECOND PHASE OF READER\n");
        printf("-------------------------------------------------------\n");
    }

    //Store file tag in the hashtable
    storefiletag(msg->filename , tag);

    printf("----------------------------------------------------------------------------------\n");
    printf("Read operation completed: Value: , tagNum:%ld , tagID:%ld\n",tag->num,tag->clientID );
    printf("----------------------------------------------------------------------------------\n");


    //Go through the list that holds the data.
    //In case when one replica is fault you can
    //request the data from the next available replica
    GSList   *iter=NULL;


    printf("\n-------------------------------------------------\n");
    printf("Replica Set\n");
    printf("{ ");
    for (iter = setOfReplica; iter; iter = iter->next)
    {
        if(iter->next !=NULL)
        {
            printf("%d," , GPOINTER_TO_INT(iter->data));
        }
        else
        {
            printf("%d }" , GPOINTER_TO_INT(iter->data));
        }

    }
    printf("\n-------------------------------------------------\n");


    //Go through available replicas
    for (iter = setOfReplica; iter; iter = iter->next)
    {
        IsSuccess=get_file(replicaSocks[(GPOINTER_TO_INT(iter->data))] , msg );
        //Check if the file have transfered  successful
        if(IsSuccess == SUCCESS )
        {
            break;
        }
    }

    //deallocate
    free(msg);
    free(tag);

    return IsSuccess;

}

/***********************************************************************************/
/*                             READER FUNCTION                                     */
/***********************************************************************************/
int reader_oper(int msg_id , struct cmd *cmdmsgIn )
{
   //Store if the action are
   int isSuccess=FAILURE;

   //Allocation memory
   struct message *msg=(struct message *)malloc(sizeof(struct message));
   struct TAG *tag=(struct TAG *)malloc(sizeof(struct TAG));

   //Store the file that hold the UpToDate replicas
   GSList  *setOfReplica=NULL;

   //First phase of ABD algorithm
   setOfReplica = read_Query(cmdmsgIn,tag,msg);

   //Check if it receive a replica set
   if(setOfReplica == NULL)
   {
       return FAILURE;
   }

   //Retrive the file from the replica
   isSuccess = read_Inform(cmdmsgIn, tag , msg , setOfReplica);

   return isSuccess;

}//Function reader

/***********************************************************************************/
/*                           WRITER FUNCTION                                        */
/***********************************************************************************/
int writer_oper(int msg_id , struct cmd *cmdmsgIn  )
{
    int IsSuccess=1;
    int isReceiveMajor;

    char buf[512];
    int i;
    //The replicas that hold the file
    struct message *msg;
    struct TAG *tag;

    //The replicas that hold the file
    GSList  *setOfReplica=NULL;

    //Allocation memory
    msg=(struct message *)malloc(sizeof(struct message));
    tag=(struct TAG *)malloc(sizeof(struct TAG));

    //delete
    tag->num=0;
    tag->clientID=0;

    //Retrieve information about the file
    msg->fileID = cmdmsgIn->fileid;
    msg->filename=strdup(cmdmsgIn->filename);
    msg->filetype=strdup(cmdmsgIn->fileType);

    //Initialize buffer
    bzero(buf, sizeof(buf));
    encode(msg, buf , "WREAD");

    //Broadcast to all directories to request the tag for a
    //specific file ID
    for(i=0; i < MAX_DIRECTORIES; i++)
    {
        if (send(direcSocks[i], buf, sizeof(buf), 0) < 0)
        {
            perror("send() failed\n");
            //exit(EXIT_FAILURE);
        }
    }//For statment

    //Wait until to receive a quorum with the last tag
    setOfReplica = receive_quorum(tag,setOfReplica,1 , &isReceiveMajor);


    //Check if you received the latest tag
    if( isReceiveMajor != 1)
    {
        printf("----------------------------------\n");
        printf("UNABLE TO RECEIVE QUORUM OF DIRECTORY IN WRITER\n");
        printf("----------------------------------\n");
        return FAILURE;
    }

    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("READ TAG FROM DIRECTORIES OPERATION COMPLETED , TAG_NUM:%ld , TAG_CLIENTID:%ld\n",tag->num , tag->clientID );
    printf("--------------------------------------------------------------------------------------------------------\n");

    //Retrieve the tag for the file
    struct TAG *filetag = NULL;
    filetag = g_hash_table_lookup(hashFiletags , msg->filename );
    if(filetag == NULL )
    {
        filetag =(struct TAG*) malloc(sizeof(tag));
        filetag->num = 2 ;
        filetag->clientID = clientID;
    }

    if(filetag->num < tag->num || (filetag->num == tag->num && filetag->clientID < tag->clientID))
    {
        printf("---------------------\n");
        printf("FOUND CONFLICT ON TAG\n");
        printf("---------------------\n");

        char tmpfilename[256];
        bzero(tmpfilename,sizeof(tmpfilename));

        //Replace the old one version of the file with the new one
        unsigned int length=sprintf(tmpfilename,"%s.%s", msg->filename , msg->filetype);
        //tmpfilename[length-1]='\0';

        //Remove the old file
        //remove(tmpfilename);

        //Read the new version of the file
        read_Inform(cmdmsgIn , tag , msg , setOfReplica);

       //reader_oper(msg_id , cmdmsgIn);

        return FAILURE;
    }

    //Increase the msgID before to broadcast the message to all replicas
    ++message_id;
    tag->num +=1;
    tag->clientID = clientID;

    //Clear the old replica set
    g_slist_free(setOfReplica);
    setOfReplica= NULL;

    shuffle(repliVals,MAX_REPLICAS);

    //Broadcast to all replicas the object and wait from f(fail number) replicas to response
    //with an acknowledgment
    for(i=0; i < ceil(( MAX_REPLICAS * failrate )); i++)
    {
        int repl= repliVals[i];

        if( send2ftp(cmdmsgIn,replicaSocks[repl] , tag ,(message_id) ) == FAILURE)
        {
            printf("Error:Send2ftp\n");
            return  FAILURE;
        }
    }

    setOfReplica = recvrepliquorum(msg,setOfReplica);
    if(setOfReplica  == NULL)
    {
        printf("--------------------------------------\n");
        printf("UNABLE TO RECEIVE RATEFAIL OF REPLICAS\n");
        printf("--------------------------------------\n");
        return FAILURE;
    }

    printf("------------------------------------------------------------------------------------------------\n");
    printf("WRITE TO REPLICAS OPERATION COMPLETED , TAG_NUM:%ld , TAG_CLIENTID:%ld\n",tag->num , tag->clientID );
    printf("------------------------------------------------------------------------------------------------\n");

    //Debug Purpose
    GSList *iter=NULL;

    printf("-------------------------------------------\n");
    printf("Replica Set\n");
    printf("{ ");
    for (iter = setOfReplica; iter; iter = iter->next)
    {
        if(iter->next !=NULL)
        {
            printf("%d," , GPOINTER_TO_INT(iter->data));
        }
        else
        {
            printf("%d }" , GPOINTER_TO_INT(iter->data));
        }

    }
    printf("\n-------------------------------------------\n");


    //Announce to all directories that f replicas
    //had store the object
    (++message_id);
    msg->replicaSet = setOfReplica;
    msg->msg_id= message_id ;
    msg->tag.num =tag->num;
    msg->tag.clientID = tag->clientID;

    bzero(buf, sizeof(buf));
    encode(msg , buf , "WWRITE" );

    //Inform all directories which replicas hold the data
    for(i=0; i < MAX_DIRECTORIES; i++)
    {
        if (send(direcSocks[i], buf, sizeof(buf), 0) < 0)
        {
            perror("send() failed\n");
        }
    }//For statment

    //Phase wdw.Wait a quorum Directories
    receive_quorum(tag , setOfReplica , 0 , &isReceiveMajor);
    if( isReceiveMajor != 1)
    {
        printf("wdw..Unable to receive a majority\n");
    }

    //Update the hashtable with the new tag for the file
    storefiletag(msg->filename , tag );

    printf("-----------------------------------------------------------------------------------------------------\n");
    printf("WRITE TO DIRECTORIES OPERATION COMPLETED , LAST_TAG_NUM:%ld , LAST_TAG_IS:%ld\n",tag->num , tag->clientID);
    printf("-----------------------------------------------------------------------------------------------------\n");

    //delete
    //sleep(2);

    //Initialize buffer
    bzero(buf, sizeof(buf));

    //Encode the message that it will send
    encode(msg , buf , "SECURE" );

    //GSList *iter=NULL;

    for (iter = setOfReplica; iter; iter = iter->next)
    {
        if (send(replicaSocks[(GPOINTER_TO_INT(iter->data))], buf , sizeof(buf) , 0) < 0)
        {
            perror("send() secure failed\n");
        }
    }


    printf("----------------------------------------------------------------------------------------------\n");
    printf("SEND SECURE MESSAGE TO REPLICAS , LAST_TAG_NUM:%ld , LAST_TAG_IS:%ld\n",tag->num , tag->clientID );
    printf("----------------------------------------------------------------------------------------------\n");


    //deallocate
    free(msg->filename);
    free(msg->filetype);
    g_slist_free(msg->replicaSet);
    g_slist_free(setOfReplica);
    free(msg);
    free(tag);

}//Function writer

/***********************************************************************************/
/*                              WRITE LOG                                          */
/***********************************************************************************/
int writeLog(FILE *log_file , char *info)
{
    log_file=fopen("client.log" , "a+");
    fprintf(log_file , info , sizeof(info));
    fclose(log_file);
}//Function writeLog

/***********************************************************************************/
/*                           READ COMMAND                                          */
/***********************************************************************************/
int read_cmd(char *cmd_str , struct cmd *cmdmsg )
{
    char *cmd;
    char *temp;
    /* get the first token */
    cmd = strtok(cmd_str, " ");
    strcpy(cmdmsg->oper,cmd);

    //Check the command if it exist
    if(strcmp(cmd,"exit") ==0)
    {
        return 1;
    }
    else if(strcmp(cmd , "loggin" ) == 0)
    {
        //Store the username of the client
        char *username;

        username = strdup(strtok(NULL," "));

        //Request clientID
        reqClientID(username);

        //Deallocate
        free(username);

        return 1;
    }
    else if(strcmp(cmd , "create" ) == 0)
    {
        //Store the filename
        char *filename;

        filename = strdup(strdup(strtok(NULL," ")));

        long fileid = reqCreate(filename);

        printf("Create executed! FileID: %ld" , fileid);

        //Deallocate
        free(filename);

        return 1;
    }

    //Retieve information of the file
    temp=strtok(NULL," ");
    cmdmsg->filename=strdup(strtok(temp,"."));
    cmdmsg->fileType=strdup(strtok(NULL," "));

    if(strcmp(cmd , "read" ) == 0 )
    {
        cmdmsg->fileid = reqFileID(cmdmsg , clientID);

        //In case where it receive correct the fileid
        if(cmdmsg->fileid != FAILURE)
        {
            //get_file(replicaSocks[1] , msg);
            if( reader_oper(message_id , cmdmsg ) == FAILURE )
            {
                printf("Unable to read the data object\n");
            }
        }

        else
        {
            printf("Unable to receive the fileid correct\n");
        }

    }

    //In case when client interested to write a file to replicasc
    else if(strcmp(cmd , "write" ) == 0)
    {
        cmdmsg->fileid = reqFileID(cmdmsg , clientID);

        //In case where it receive correct the fileid
        if(cmdmsg->fileid != FAILURE)
        {

            writer_oper(message_id, cmdmsg);
        }
        else
        {
            printf("Unable to receive the fileID correct\n");
        }
    }

    //deallocate
    free(cmdmsg->filename);
    free(cmdmsg->fileType);

    return 1;
}

/***********************************************************************************/
/*                              Read config                                        */
/***********************************************************************************/
void readConfig(char *filename)
{

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int i=0,j=0,d=0;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Unable to open config fiLe\n");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {

        if(strcmp(line,"#DIRECTORIES\n") == 0)
        {
            while ((read = getline(&line, &len, fp)) != -1)
            {
                if(strcmp(line,"#REPLICAS\n") == 0)
                {
                    break;
                }
                if(MAX_DIRECTORIES==0)
                {
                    MAX_DIRECTORIES=atoi(line);
                    direcNodes=(struct serinfo *)malloc(sizeof(struct serinfo ) * MAX_DIRECTORIES );
                }
                else
                {
                    strcpy(direcNodes[i].ip_addr, strtok(line, " ") );
                    direcNodes[i].port = atoi(strtok(NULL, " "));
                    i +=1;
                }
            }//While
        }

        if(strcmp(line,"#REPLICAS\n") == 0)
        {
            while ((read = getline(&line, &len, fp)) != -1)
            {
                if(strcmp(line,"#FILEMANAGERS\n") == 0)
                {
                    break;
                }
                if (MAX_REPLICAS == 0)
                {
                    MAX_REPLICAS = atoi(line);
                    replicaNodes=(struct serinfo *)malloc(sizeof(struct serinfo ) * MAX_REPLICAS );
                }
                else
                {
                    strcpy(replicaNodes[j].ip_addr, strtok(line, " "));
                    replicaNodes[j].port = atoi(strtok(NULL, ","));
                    j += 1;
                }
            }//While
        }
        if(strcmp(line,"#FILEMANAGERS\n") == 0)
        {
            while ((read = getline(&line, &len, fp)) != -1)
            {
                if (MAX_FILEMANAGERS == 0)
                {
                    MAX_FILEMANAGERS = atoi(line);
                    filemanagerNodes=(struct serinfo *)malloc(sizeof(struct serinfo ) * MAX_FILEMANAGERS );
                }
                else
                {
                    strcpy(filemanagerNodes[d].ip_addr , strtok(line, " ") );
                    filemanagerNodes[d].port = atoi(strtok(NULL, " "));
                    d += 1;
                }
            }//While
        }

    }

    fclose(fp);
    if (line)
        free(line);
}

int reqClientID(char *username)
{
    //Buffer message
    char buf[256];

    //Initialize the buffer
    bzero(buf,sizeof(buf));

    //Store the bytes that sent
    int bytes;

    //Message ID to validate that it receive the correct message
    long messageID = (++message_id);

    //Allocation memory
    struct message *msg=(struct message *)malloc(sizeof(struct message));

    //Request from the filemanager to set up a client ID
    sprintf(buf,"REQCLIENTID,%s,%ld",username , messageID );

    if (bytes = send(filemanagerSocks[0] , buf, sizeof(buf) , 0) < 0)
    {
        perror("Send:Unable to request clientID");
    }

    //Initialize the buffer
    bzero(buf,sizeof(buf));

    //Wait to receive the message
    if (recv(filemanagerSocks[0], buf, sizeof(buf), 0) < 0)
    {
        perror("Received() Unable to receive clientID");
    }

    //Decode the message that it receive
    decode(msg,buf);

    //Deallocate type
    free(msg->type);

    //Check if it received the correct message
    if(msg->msg_id == messageID)
    {
        //Deallocate struct
        free(msg);

        //Assign client ID that it received
        clientID = msg->clientID;

        g_print("CLIENTID:%d",clientID);
    }
    else
    {
        //Deallocate struct
        free(msg);

        return FAILURE;
    }
}

long reqCreate(char *filename)
{
    //Buffer message
    char buf[256];

    //Message ID to validate that it receive the correct message
    long messageID = (++message_id);

    //Store the number of bytes that sent via socket
    int bytes;

    //Store the fileid for the file that it create
    long fileid;

    //Request from the filemanager to set up a client ID
    sprintf(buf,"REQCREATE,%s,%ld,%ld" , filename , clientID, messageID );

    if (bytes = send(filemanagerSocks[0] , buf, sizeof(buf) , 0) < 0)
    {
        perror("Send:Unable to request clientID");
    }

    bzero(buf,sizeof(buf));
    if (recv(filemanagerSocks[0], buf, sizeof(buf) , 0) < 0)
    {
        perror("Received() Unable to receive clientID");
    }

    printf("ReqCreate Received: %s\n" , buf );

    //Allocation memory
    struct message *msg=(struct message *)malloc(sizeof(struct message));

    //Decode the message that it receive
    decode(msg,buf);

    //Deallocate type
    free(msg->type);

    //Check if it received the correct message
    if(msg->msg_id == messageID)
    {
        //Assign fileid
        int fileid = msg->fileID;

        //Deallocate struct
        free(msg);

        return fileid;
    }

    else
    {
        //Deallocate struct
        free(msg);

        return FAILURE;
    }
}

long reqFileID(struct cmd *cmdmsg , long clientID)
{
    //Buffer message
    char buf[256];

    //Store the number of bytes that sent via socket
    int bytes;

    //Message ID to validate that it receive the correct message
    long messageID = (++message_id);

    bzero(buf,sizeof(buf));
    //Request from the filemanager to set up a client ID
    sprintf(buf,"REQFILEID,%s.%s,%ld,%ld" , cmdmsg->filename , cmdmsg->fileType , clientID , messageID );

    if (bytes = send(filemanagerSocks[0] , buf, sizeof(buf) , 0) < 0)
    {
        perror("Send:Unable to request clientID");
    }

    bzero(buf,sizeof(buf));
    if (recv(filemanagerSocks[0], buf, sizeof(buf), 0) < 0)
    {
        perror("Received() Unable to receive clientID");
    }

    printf("REQFILEID Received: %s\n" , buf );

    //Allocation memory
    struct message *msg=(struct message *)malloc(sizeof(struct message));

    //Decode the message that it receive
    decode(msg,buf);

    //Deallocate type
    free(msg->type);

    //Check if it received the correct message
    if(msg->msg_id == messageID)
    {
        //Assign fileid
        int fileid = msg->fileID;

        //Deallocate struct
        free(msg);

        return fileid;
    }

    else
    {
        //Deallocate struct
        free(msg);

        return FAILURE;
    }


}

/***********************************************************************************/
/*                            Establish connections                                */
/***********************************************************************************/
int connect2Replicas()
{
    int i=0;

    for(i=0; i < MAX_REPLICAS; i++)
    {
        /*Establish a connection withe the IP address from the struct server*/
        if (connect(replicaSocks[i] , (struct sockaddr *) &replica_sockaddr[i], sizeof(replica_sockaddr[i])) < 0)
        {
            /*Use htohs to convert network port to host port.*/
            printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(replica_sockaddr[i].sin_addr), ntohs(replica_sockaddr[i].sin_port) );
        }
        else
        {
            printf("Connection established to IP: %s ,  PORT: %d\n", inet_ntoa(replica_sockaddr[i].sin_addr),ntohs(replica_sockaddr[i].sin_port));
        }
    }
}

void conn2direc()
{
    int i=0;
    char buf[50];
    bzero(buf,sizeof(buf));
    int bytes;

    for(i=0; i < MAX_DIRECTORIES; i++)
    {
        /*Establish a connection withe the IP address from the struct server*/
        if (connect(direcSocks[i] , (struct sockaddr *) &direc_sockaddr[i], sizeof(direc_sockaddr[i])) < 0)
        {
            /*Use htohs to convert network port to host port.*/
            printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(direc_sockaddr[i].sin_addr), ntohs(direc_sockaddr[i].sin_port) );
        }
        else
        {

            printf("Connection established to IP: %s ,  PORT: %d\n", inet_ntoa(direc_sockaddr[i].sin_addr),ntohs(direc_sockaddr[i].sin_port) );

        }
        //Add socket to set
        FD_SET(direcSocks[i] , &direcs_fds);
        if (direcSocks[i] > max_direc_fd)
        {
            max_direc_fd = direcSocks[i];
        }
    }//For statment

}//Function establish connections

int conn2filemanager()
{
    int i=0;

    //Establish Connection with all the filemanager servers
    for(i=0; i < MAX_FILEMANAGERS; i++)
    {
        /*Establish a connection withe the IP address from the struct server*/
        if (connect(filemanagerSocks[i] , (struct sockaddr *) &filemanager_sockaddr[i], sizeof(filemanager_sockaddr[i])) < 0)
        {
            /*Use htohs to convert network port to host port.*/
            printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(filemanager_sockaddr[i].sin_addr), ntohs(filemanager_sockaddr[i].sin_port) );
        }
        else
        {
            printf("Connection established to IP: %s ,  PORT: %d\n", inet_ntoa(filemanager_sockaddr[i].sin_addr),ntohs(filemanager_sockaddr[i].sin_port));
        }
    }

}

int get_file(int sock, struct message *msg )
{
    FILE     *received_file;
    int      remain_data;
    int      bytes;
    int      bufsize=2048;
    char     buf[bufsize];
    char     filename[256];
    unsigned    int length;

    //
    struct message *recvmsg;

    //Allocation memory
    recvmsg=(struct message *)malloc(sizeof(struct message));

    sprintf(buf ,"READ,%ld,%ld,%ld,%s,%s" , msg->tag.num , msg->tag.clientID , (++msg->msg_id) , msg->filename , msg->filetype );

    //printf("Filename in get_file: %s , length: %d\n",buf, file_size);
    if ((bytes=send(sock, buf, 512 , 0)) < 0)
    {
        perror("send() getfile failed\n");
        return FAILURE;
    }

    printf("Bytes sends: %d\n" ,bytes );

    //Wait until to receive an acknowldge with the
    //same messageID
    bzero(buf,sizeof(buf));
    if(recv(sock, buf, 512 , 0) < 0)
    {
        perror("Received");
        exit(sock);
        return FAILURE;
    }

    //Unable to receive details for the file
    if(strlen(buf) ==0)
    {
        return FAILURE;
    }

    printf("Received in getFile: %s\n" , buf);

    decode(recvmsg,buf);

    length=sprintf(filename , "%s.%s" , msg->filename , msg->filetype);
    //filename[length-1]='\0';

    received_file = fopen(filename, "w");
    if (received_file == NULL)
    {
        printf("Failed to open file foo --> %s" , filename);
        return FAILURE;
    }

    //Retrieve the size of the file
    remain_data = recvmsg->fileSize;

    bzero(buf, bufsize);
    while (((bytes = recv(sock, buf, bufsize, 0)) > 0) && (remain_data > 0))
    {
        fwrite(buf, sizeof(char), bytes, received_file);
        remain_data -= bytes;
     // fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", bytes, remain_data);

      //  printf("Received: %d , remain : %d\n",bytes , remain_data);
        if(remain_data == 0 )
        {
            break;
        }

    }//While


    //close file descriptor
    fclose(received_file);


    //Retrieve the checksum from the file that the
    //client receive in order to check if receive
    //the whole file
    char *filechecksum = checksum_get(filename);


    //check if it received the whole file
    //otherwise return an error
    if(remain_data > 0)
    {
        return FAILURE;
    }
    else if(strcmp(filechecksum , recvmsg->checksum) != 0)
    {
        printf("Invalid checksum\n");
        //Remove the file
        remove(filename);

        return FAILURE;
    }


    return SUCCESS;
}

int send2ftp(struct cmd *msgCmd, int newsock , struct TAG *tagIn , long msgIDIn )
{
    int         fd;
    off_t       offset = 0;
    int         remain_data;
    int         sent_bytes;
    struct      stat file_stat; /*to retrieve information for the file*/
    int         len;
    int         file_size;
    char        buf[512];
    char        filename[256];

    bzero(filename,sizeof(filename));

    unsigned int length=sprintf(filename,"%s.%s", msgCmd->filename,msgCmd->fileType);
    //filename[length-1]='\0';

    fd = open(filename,  O_RDONLY);
    if (fd < 0 )
    {
        printf("Error opening file --> %s" , filename);
        return FAILURE;
    }

    // Get file stats //
    if (fstat(fd, &file_stat) < 0)
    {
        printf("Error fstat");
        close(fd);
        return FAILURE;
    }

    // Sending file size //
    file_size=file_stat.st_size;

    //Retrieve checksum for the file
    char *filechecksum = checksum_get(filename);

    bzero(buf,sizeof(buf));
    sprintf(buf,"WRITE,%s,%s,%ld,%ld,%ld,%d,%s" , msgCmd->filename,msgCmd->fileType,tagIn->num,tagIn->clientID , msgIDIn , file_size, filechecksum );

    // If connection is established then start communicating //
    len = send(newsock, buf, 512 , 0);
    if (len < 0)
    {
        perror("send");
        close(fd);
        return FAILURE;
    }
    //Wait to received acknowledge that received the metadata for the file

    //  printf("Sending... file:%s.%s\n" ,msgCmd->filename, msgCmd->fileType );
    //printf("Server sent %d bytes for msg size\n" , len);

    remain_data = file_stat.st_size;
    // Sending file data //
    while (((sent_bytes = sendfile(newsock, fd, &offset, MAXBUF)) > 0) && (remain_data > 0))
    {
        remain_data -= sent_bytes;
    //  printf("Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);

    }

    //Close the file descriptor
    close(fd);

    //check if it received the whole file
    //otherwise return an error
    if(remain_data > 0)
    {
        return FAILURE;
    }
    else
    {
        printf("File: %s send...\n" , msgCmd->filename);

    }

    return SUCCESS;
}

void inisialization() 
{

    int i;

    //Allocate the array to store all the replica nodes
    repliVals = (int *)malloc(sizeof(int) * MAX_REPLICAS);

    //Allocate the array to store all the sockaddr_in for directories
    direc_sockaddr=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in ) * MAX_DIRECTORIES );

    //Allocate the array to store all the sockaddr_in for Replicas
    replica_sockaddr=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in ) * MAX_REPLICAS );

    //Allocate the array to store all the sockaddr_in for file managers
    filemanager_sockaddr=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in ) * MAX_FILEMANAGERS );

    //Allocate the array to store all the sockets descriptor for each directory
    direcSocks=( int *)malloc(sizeof(int ) * MAX_DIRECTORIES );

    //Allocate the array to store all the sockets descriptor for each Replicas
    replicaSocks=(int *)malloc(sizeof(int ) * MAX_REPLICAS );

    //Allocate the array to store all the sockets descriptor for each Replicas
    filemanagerSocks=(int *)malloc(sizeof(int ) * MAX_FILEMANAGERS );

    //Fill the direcSocks for directories server which is type sockaddr_in
    for (i = 0; i < MAX_DIRECTORIES; i++)
    {
        direc_sockaddr[i].sin_family = AF_INET; /*Internet domain*/
        direc_sockaddr[i].sin_addr.s_addr = inet_addr(direcNodes[i].ip_addr);
        direc_sockaddr[i].sin_port = htons(direcNodes[i].port);
    }//For statement

    //Fill the replicaSocks for directories server which is type sockaddr_in
    for (i = 0; i < MAX_REPLICAS; i++)
    {
        replica_sockaddr[i].sin_family = AF_INET; /*Internet domain*/
        replica_sockaddr[i].sin_addr.s_addr = inet_addr(replicaNodes[i].ip_addr);
        replica_sockaddr[i].sin_port = htons(replicaNodes[i].port);
    }//For statement


    //Fill the replicaSocks for directories server which is type sockaddr_in
    for (i = 0; i < MAX_FILEMANAGERS; i++)
    {
        filemanager_sockaddr[i].sin_family = AF_INET; /*Internet domain*/
        filemanager_sockaddr[i].sin_addr.s_addr = inet_addr(filemanagerNodes[i].ip_addr);
        filemanager_sockaddr[i].sin_port = htons(filemanagerNodes[i].port);
       // filemanager_sockaddr[i].sin_addr.s_addr = inet_addr("127.0.0.1");
        //filemanager_sockaddr[i].sin_port = htons(40001);
    }//For statement

    //initialize hash table that stores the tags for the files
    hashFiletags = g_hash_table_new( g_str_hash, g_str_equal);

    //Inisialization  of the set of sockets descriptors
    FD_ZERO(&direcs_fds);
    FD_ZERO(&replicas_fd);
    FD_ZERO(&crash_direc_fds);

    //Go through all the available directories to establish a connection.*/
    for (i = 0; i < MAX_DIRECTORIES; i++)
    {
        //Create a socket and check if is created correct
        if ((direcSocks[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket() failed");
            exit(1);
        }
    }//For statement

    //Fill the replicaSocks for directories server which is type sockaddr_in
    for (i = 0; i < MAX_REPLICAS; i++)
    {
        //Fill with all the replica
        repliVals[i]=i;

        //Create a socket and check if is created correct
        if ((replicaSocks[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket() failed");
            exit(1);
        }
        //Add socket to set
        FD_SET(replicaSocks[i] , &replicas_fd);
        if (replicaSocks[i] > max_replica_fd)
        {
            max_replica_fd = replicaSocks[i];
        }
    }//For statement

    //Fill the replicaSocks for filemanager servers
    for (i = 0; i < MAX_FILEMANAGERS; i++)
    {
        //Create a socket and check if is created correct
        if ((filemanagerSocks[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket() failed");
            exit(1);
        }
    }//For statement


    /*Is an option to set a timeout value for input operations.
      It accepts a struct timeval parameter with the number of seconds
      and microseconds used to limit waits for input operations to complete.*/

    //Configure sockets
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    //Set the socket to block only for some second
    for (i = 0; i < MAX_REPLICAS; i++)
    {
          setsockopt(replicaSocks[i], SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout , sizeof(timeout)) ;

    }

    //Set the socket to block only for some second
    for (i = 0; i < MAX_FILEMANAGERS; i++)
    {
        setsockopt(filemanagerSocks[i], SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout , sizeof(timeout));
    }

}//Function inisialization

void unitTest(char *filename , char *filetype , char *username)
{
    //Check the system if is working correct
    struct cmd *cmdmsg;

    cmdmsg=(struct cmd*)malloc(sizeof(struct cmd));
    cmdmsg->filename=strdup(filename);
    cmdmsg->fileType=strdup(filetype);

    //Unitest for the function reqClientID
    long clientID = reqClientID(username);

    //Counter to calculate the times that the unitest run
    int i=0;

        //newline
        printf("\n");
        if (clientID != FAILURE)
        {
            printf("-----------------------------------\n");
            printf("TEST1 reqClientID function PASSED!!\n");
            printf("Received clientID: %ld\n", clientID);
            printf("-----------------------------------\n");

        }
        else
        {
            printf("-----------------------------------\n");
            printf("TEST1 reqClientID function FAILED!!\n");
            printf("Received clientID: %ld\n", clientID);
            printf("-----------------------------------\n");
        }
        printf("\n--------------------------------------------------------------------\n");

    for(i=0; i<1; i++)
    {
        //Unit test for the function reqCreate
        //Store the filename with the corresponding type
        char newfilename[256];
        bzero(newfilename, sizeof(newfilename));

        sprintf(newfilename, "%s.%s", filename, filetype);

        //Call reqCreate function
        long fileid = reqCreate(newfilename);

        //newline
        if (fileid != FAILURE)
        {
            //Store the file id that received
            cmdmsg->fileid = fileid;

            printf("-----------------------------------\n");
            printf("TEST2 reqCreate function PASSED!!\n");
            printf("Received fileID: %ld\n", fileid);
            printf("-----------------------------------\n");

        }
        else
        {
            printf("-----------------------------------\n");
            printf("TEST2 reqCreate function FAILED!!\n");
            printf("Received fileID: %ld\n", fileid);
            printf("-----------------------------------\n");
        }
        printf("\n--------------------------------------------------------------------\n");


        //message id
        long messageid=0;

        //Call reqCreate function
        int isSuccess =  writer_oper(messageid,cmdmsg);

        //newline
        if (isSuccess != FAILURE)
        {
            printf("\n**********************************\n");
            printf("TEST3 WRITER_OPER FUNCTION PASSED!!\n");
            printf("**********************************\n");
        }
        else
        {
            printf("**********************************\n");
            printf("TEST3 WRITER_OPER FUNCTION FAILED!!\n");
            printf("**********************************\n");
        }
        printf("\n--------------------------------------------------------------------\n");


    }//For statment



    //deallocate
    free(cmdmsg->filename);
    free(cmdmsg->fileType);

}

/* The file selection widget and the string to store the chosen filename */
GtkWidget *file_selector;
gchar *selected_filename;

/* The Text Buffer as a global variabel*/
GtkTextBuffer *buffer;
/* File descriptors for pipe. */
int fds[2];

GtkWidget     *view;

/* This is a callback function to watch data in standard output and write to a view text widget. */
void input_callback( gpointer          data,
                     gint              source,
                     GdkInputCondition condition )
{

    //Clear buffer
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (buffer, "", -1);

    gchar buf[1024];
    gint chars_read;
    GtkTextIter iter;
    chars_read = 1024;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    while (chars_read == 1024)
    {
        chars_read = read(fds[0], buf, 1024);
        // fprintf(stderr, "%i chars: %s\n", chars_read, buf);
        gtk_text_buffer_insert (buffer, &iter, buf, chars_read);
    }
}

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below. */
static void hello( GtkWidget *widget,
                   gpointer   data )
{
    gtk_widget_show (file_selector);
}

static void delete_event( GtkWidget *widget,
                          GdkEvent  *event,
                          gpointer   data )
{
    exit(1);
}

void store_filename(GtkFileSelection *selector, gpointer user_data)
{
    char cwd[1024];
    char command[2048];

    bzero(cwd,sizeof(cwd));
    bzero(command,sizeof(command));

    getcwd(cwd, sizeof(cwd));

    // g_print("Current working dir: %s\n", cwd);
    selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(file_selector));

     g_print ("%s\n" ,selected_filename);

    //g_print ("%s\n" , g_path_get_basename(selected_filename));

    // g_print("%s\n", cwd);

    /*sprintf(command,"cp %s %s", selected_filename, cwd);

    g_print("%s\n", command);

    system(command);*/

    char* filename =g_strdup(g_path_get_basename(selected_filename) );

    struct cmd *cmdmsg = (struct cmd *) malloc((sizeof(struct cmd)));

    cmdmsg->filename=strdup(strtok(g_path_get_basename(selected_filename),"."));
    cmdmsg->fileType=strdup(strtok(NULL," "));

    cmdmsg->fileid = 1;

    g_print("%s , %s\n" , cmdmsg->filename , cmdmsg->fileType);

    writer_oper(message_id, cmdmsg);

    g_free(filename);
    free(cmdmsg->filename);
    free(cmdmsg->fileType);

    gtk_widget_hide(file_selector);

}

void hide_file_selector(GtkFileSelection *selector, gpointer user_data)
{
    gtk_widget_hide(file_selector);
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    g_print ("destroy event occurred\n");
    gtk_main_quit ();
}

/* "print text" button "clicked" callback function. */
void
on_button1_clicked(GtkButton* button, gpointer data)
{
    /* cast the data back to a GtkEntry*  */
    GtkEntry* entry = (GtkEntry*)data;

    //Request ClientID
    reqClientID(gtk_entry_get_text(entry));
}



/***********************************************************************************/
/*                                  MAIN                                           */
/***********************************************************************************/
int main(int argc , char *argv)
{
    //Tha name of the file that holds all the information for the servers
    char *filename="config.txt";

    //Retrieve information for the servers
    readConfig(filename);

    inisialization();

    //Establish  connection with all available diretories & replicas  servers.
    conn2direc();
    connect2Replicas();

    //Establish connection with the filemanager
    conn2filemanager();
    //Gt// GtkWidget is the storage type for widgets//
    GtkWidget     *window;
    GtkWidget     *vbox;
    GtkWidget     *vbox2;

    GtkWidget     *button;
    GtkWidget     *store;
    GtkWidget     *submit;

    /* this will store a text entry. */
    GtkWidget* entry;

    GtkWidget *toolbar;
    GtkToolItem *saveTb;
    GtkToolItem *exitTb;

    // Create a pipe. File descriptors for the two ends of the pipe are placed in fds.
    pipe (fds);
    // Redirect fds[1] to be writed with the standard output.
    dup2 (fds[1], STDOUT_FILENO);

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (&argc, &argv);

    // Create a new window //
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //Set the position the main window in the Center
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //Set the size the main window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);

    //Set the name of the window application
    gtk_window_set_title(GTK_WINDOW(window), "Distributed File System");

    /* Create the selector */
    file_selector = gtk_file_selection_new("Please select a file for editing.");


    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);

    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
                        "clicked", GTK_SIGNAL_FUNC (hide_file_selector), NULL);

    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (delete_event), NULL);

    /* Here we connect the "destroy" event to a signal handler.
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (destroy), NULL);

    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);


    /* create a text entry. */
    entry = gtk_entry_new_with_max_length(20);

    /* insert the following text into the text entry, as its initial value. */
    gtk_entry_set_text(GTK_ENTRY(entry), "spanos");


    submit = gtk_button_new_with_label("Submit");


    gtk_signal_connect(GTK_OBJECT(submit), "clicked",
                       GTK_SIGNAL_FUNC(on_button1_clicked),
                       (gpointer)entry);


    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

    saveTb = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), saveTb, -1);

    exitTb = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exitTb, -1);


    g_signal_connect(G_OBJECT(exitTb), "clicked",
                     G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);


    /* Creates a new button with the label "Hello World". */
    button = gtk_button_new_with_label ("Upload");


    store = gtk_button_new_with_label ("Store");

    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (hello), NULL);

    g_signal_connect (G_OBJECT (store), "clicked",
                      G_CALLBACK (hello), NULL);

    /* This will cause the window to be destroyed by calling
     * gtk_widget_destroy(window) when "clicked".  Again, the destroy
     * signal could come from here, or the window manager. */
/*
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (window));*/

    //Creates a new text view with a text buffer "Hello, this is some text". //
    view = gtk_text_view_new ();

    //Creates a new vertical box.//
    vbox = gtk_vbox_new (FALSE, 3);

    //Creates a new vertical box.//
    vbox2 = gtk_vbox_new (FALSE, 3);

    //This packs the vbox into the window (a gtk container).
    gtk_container_add (GTK_CONTAINER (window), vbox);

    //This packs the vbox into the window (a gtk container).
    gtk_container_add (GTK_CONTAINER (vbox), vbox2);

    /* This packs the text view into the vbox. */
    gtk_box_pack_end (GTK_BOX (vbox), view, TRUE, TRUE, 20);

    /* This packs the button into the vbox. */
    gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 5);

    /* This packs the button into the vbox. */
    gtk_box_pack_end (GTK_BOX (vbox2), store, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox2), toolbar, FALSE, FALSE, 5);

    /* This packs the button into the vbox. */
    gtk_box_pack_start (GTK_BOX (vbox2), entry, FALSE, FALSE, 5);

    /* This packs the button into the vbox. */
    gtk_box_pack_start (GTK_BOX (vbox2), submit, FALSE, FALSE, 5);

    gtk_text_view_set_border_window_size(view,GTK_TEXT_WINDOW_TOP,400);

    gtk_button_set_alignment(button,0.5 , 0.0);
    gtk_button_set_alignment(store,0.5 ,  0.0);

    /* This is the singnal connection to call input_callback when we have data in standard output read end pipe. */
    gdk_input_add(fds[0], GDK_INPUT_READ, input_callback, NULL);

    //The final step is to display this newly created widget.
    gtk_widget_show (button);
    gtk_widget_show (store);
    gtk_widget_show (view);
    gtk_widget_show (vbox);
    /* make the text entry visible. */
    gtk_widget_show(entry);
    gtk_widget_show(submit);


    /* and the window */
    gtk_widget_show_all(window);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();

    return 0;
}
