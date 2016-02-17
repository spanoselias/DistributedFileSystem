/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 09/10/2015                                                                 */
/*Filename:Replica.c                                                               */
/*                                                                                 */
/***********************************************************************************/
/***********************************************************************************/
/*                                     LIBRARIES                                   */
/***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //To retrieve the ip_add to asci
#include <sys/stat.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <pthread.h>

#include "replica.h"

#define BUFSIZE 4098
#define MAXCLIENT 100

#define SUCCESS  1
#define FAILURE -1

//Format for error for the threads
#define perror2(s,e) fprintf(stderr, "%s:%s\n" , s, strerror(e))

struct sockaddr_in *direc_sockaddr;   //Store all the ips and ports for directories
struct sockaddr_in *replica_sockaddr; //Store all the ips and ports for Replicas
struct serinfo *direcNodes;           //Store IP and port for each directory server
struct serinfo *replicaNodes;         //Store IP and port for each replica server
int    *direcSocks;                   //Store all the sockets for directories
int    *replicaSocks;                 //Store all the sockets for replicas
int    MAX_DIRECTORIES=0;             //Store the number of directories that exist in the system
int    MAX_REPLICAS=0;                //Store the number of replicas that exist in the system
int    NODEID;                        //Store which ID is the replica based on port

//Metadata table that holds all tag for the data
GHashTable * metadatatable;


//Lock metadata table
pthread_mutex_t lockermetadata;

/*Lock the strtok*/
pthread_mutex_t locker;

/***********************************************************************************/
/*                              Read config                                        */
/***********************************************************************************/
void readConfig(char *filename , int port)
{

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int i=0,j=0;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Unable to open config fie\n");
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
                    strcpy(direcNodes[i].ip_addr,strdup(strtok(line, " ")));
                    direcNodes[i].port = atoi(strtok(NULL, " "));
                    i +=1;
                }
            }//While
        }

        if(strcmp(line,"#REPLICAS\n") == 0)
        {
            while ((read = getline(&line, &len, fp)) != -1)
            {
                if (MAX_REPLICAS == 0)
                {
                    MAX_REPLICAS = atoi(line);
                    replicaNodes=(struct serinfo *)malloc(sizeof(struct serinfo ) * MAX_REPLICAS );
                }
                else
                {
                    strcpy(replicaNodes[j].ip_addr, strdup(strtok(line, " ")));
                    replicaNodes[j].port = atoi(strtok(NULL, ","));

                    j += 1;
                }
            }//While
        }
    }

    fclose(fp);
    if (line)
        free(line);


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
/*                            Establish connections                                */
/***********************************************************************************/
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

            printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(direc_sockaddr[i].sin_addr), ntohs(direc_sockaddr[i].sin_port) );
        }
        else
        {
            printf("Connection established to IP: %s ,  PORT: %d\n", inet_ntoa(direc_sockaddr[i].sin_addr),ntohs(direc_sockaddr[i].sin_port) );
        }

    }//For statment

}//Function establish connections

int connect2Replicas()
{
    int i=0;

    for(i=0; i < MAX_REPLICAS; i++)
    {

        if( NODEID != replicaNodes[i].port  )
        {
            /*Establish a connection withe the IP address from the struct server*/
            if (connect(replicaSocks[i] , (struct sockaddr *) &replica_sockaddr[i], sizeof(replica_sockaddr[i])) < 0)
            {
                printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(replica_sockaddr[i].sin_addr), ntohs(replica_sockaddr[i].sin_port) );
            }
            else
            {
                printf("Connection established to IP: %s ,  PORT: %d\n", inet_ntoa(replica_sockaddr[i].sin_addr),ntohs(replica_sockaddr[i].sin_port));
            }
        }
    }
}

struct replicaHeader* decode(char *buf )
{
    //Store the thread error if exist
    int err;

    //Lock strtok due to is not deadlock free//
    if(err=pthread_mutex_lock(&locker))
    {
        perror2("Failed to lock()",err);
    }

        struct replicaHeader *msg;
        msg  = (struct replicaHeader *)malloc(sizeof(struct replicaHeader));
        msg->tag = (TAG *)malloc(sizeof(TAG));

        /*Use comma delimiter to find the type of
             * send message and retrieve the apropriate
             * fields*/
        msg->type=strdup(strtok(buf, ","));
        if( (strcmp(msg->type , "WRITE" )== 0))
        {
            msg->filename = strdup(strtok(NULL,","));
            msg->filetype = strdup(strtok(NULL,","));
            msg->tag->num = atoi(strtok(NULL,","));
            msg->tag->clientID = atoi(strtok(NULL,","));
            msg->tag->isSecure = 0 ;
            msg->msgID = atoi(strtok(NULL,","));
            msg->fileSize = atoi(strtok(NULL,","));
            msg->checksum = strdup(strtok(NULL,","));
        }
        else if( (strcmp(msg->type , "READ" )== 0))
        {
            msg->tag->num = atoi(strtok(NULL,","));
            msg->tag->clientID = atoi(strtok(NULL,","));
            msg->msgID = atoi(strtok(NULL,","));
            msg->filename = strdup(strtok(NULL,","));
            msg->filetype = strdup(strtok(NULL,","));
        }
        else if((strcmp(msg->type , "SECURE" )== 0))
        {
            msg->tag->num = atoi(strtok(NULL,","));
            msg->tag->clientID = atoi(strtok(NULL,","));
            msg->tag->isSecure=1;
            msg->msgID = atoi(strtok(NULL,","));
            msg->filename = strdup(strtok(NULL,","));
            msg->filetype = strdup(strtok(NULL,","));
        }

    /*Unloack Mutex*/
    if(err=pthread_mutex_unlock(&locker))
    {
        perror2("Failed to lock()",err);
    }


    return msg;
}

int send2ftp(int newsock , struct replicaHeader *msg )
{
    int         fd;
    off_t       offset = 0;
    int         remain_data;
    int         sent_bytes;
    /*to retrieve information for the file*/
    struct      stat file_stat;
    int         len;
    int         file_size;
    char        sendheader[256];
    char        filename[255];
    unsigned    int length;

    bzero(sendheader,sizeof(sendheader));

    //
    length=sprintf(filename,"%s_%d_%d_%d.%s",msg->filename,msg->tag->num , msg->tag->clientID,msg->tag->isSecure,msg->filetype);
    filename[length-1]='\0';

    printf("FileNAmeIn:%s" , filename);
    fd = open(filename,  O_RDONLY);
    if (fd < 0 )
    {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));
        close(fd);
        return FAILURE;
    }

    //Get file stats //
    if (fstat(fd, &file_stat) < 0)
    {
        printf("Error fstat");
        close(fd);
    }

    /* Sending file size */
    file_size=file_stat.st_size;
    printf("\nFile Size: \n %d bytes\n",file_size);


    //Retrieve checksum for the file
    char *filechecksum = checksum_get(filename);

    //
    sprintf(sendheader,"READ-OK,%d,%d,%d,%d,%s", msg->tag->num , msg->tag->clientID ,msg->msgID , file_size , filechecksum );

    /* If connection is established then start communicating */
    len = send(newsock, sendheader, 256, 0);
    if (len < 0)
    {
        perror("send");
        return FAILURE;
    }

    printf("Server sent %d bytes for the size\n" , len);

    remain_data = file_stat.st_size;
    /* Sending file data */
    while (((sent_bytes = sendfile(newsock, fd, &offset, BUFSIZE)) > 0) && (remain_data > 0))
    {
        remain_data -= sent_bytes;
        fprintf(stdout, "Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    }

    printf("Finish sending\n");
    //close(newsock);
    close(fd);

    //deallocate
    free(filechecksum);

    //check if it received the whole file
    //otherwise return an error
    if(remain_data > 0)
    {
        return FAILURE;
    }

    return SUCCESS;

}

int ftp_recv(int sock, struct replicaHeader *msg )
{
    FILE     *received_file;
    int      remain_data;
    int      bytes;
    int      file_size;

    //Store the actual filename
    char     filename[256];

    char     buffer[BUFSIZE];

    //Store the size of the filename
    unsigned int length;

    printf("Received: %d\n" , msg->fileSize);

    bzero(filename,sizeof(filename));

    length= sprintf(filename , "%s_%d_%d_0.%s" , msg->filename , msg->tag->num , msg->tag->clientID , msg->filetype);
    filename[length-1]='\0';

    received_file = fopen(filename, "w");
    if (received_file == NULL)
    {
        fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
        return FAILURE;
    }
    remain_data = msg->fileSize;

    bzero(buffer, sizeof(buffer));
    while (((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) && (remain_data > 0))
    {
        fwrite(buffer, sizeof(char), bytes, received_file);
        remain_data -= bytes;
       // fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", bytes, remain_data);

       // printf("Received: %d , remain : %d\n",bytes , remain_data);
        if(remain_data <=0 )
        {
            break;
        }

    }//While

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
  /*  else if(strcmp(filechecksum , msg->checksum) != 0)
    {
        printf("Invalid checksum\n");

        //Remove the file
        remove(filename);
        return FAILURE;
    }*/

    printf("Received file: %s successful\n" , filename);

    return SUCCESS;
}

GHashTable *  insertMetadata(GHashTable * funmetatable ,  TAG* filetag , char *filename )
{

    //printf("Enter Function insertMetadata\n");

    //Create a new tag to be insert in list
    TAG* newfiletag = g_new(TAG,1);
    newfiletag->num = filetag->num ;
    newfiletag->clientID = filetag->clientID ;
    newfiletag->isSecure = filetag->isSecure ;

    int err=0;

    //Check if the filename exist in metadata table
    //if(g_hash_table_contains(funmetatable , filename) == 1 )
    //{

    //Lock strtok due to is not deadlock free
    if(err=pthread_mutex_lock(&lockermetadata))
    {
        perror2("Failed to lock()",err);
    }

            //Pointer to a list to retrieve all the tags for the
            //specific filename
            GSList *pointlist=NULL;

            //Retrieve all the tags which are associated withe the specific filename
            pointlist =(GSList*) g_hash_table_lookup(funmetatable,  filename );

            if(pointlist != NULL)
            {

                //Insert first the tag in list and after in hashtable
                pointlist=g_slist_prepend(pointlist , newfiletag );

                //Insert updated list in hashtable to the associated key(filename)
                g_hash_table_insert(funmetatable , filename, pointlist);

            }
                //In case where the filename does not exist in hashtable you have to insert
                //the key and also create a list that will holds the tags
            else
            {
                //Create of the list that will hold the tags for a specific filename
                GSList *tagset=NULL;

                //Insert new tag in the list
                tagset=g_slist_prepend(tagset , newfiletag );

                //Insert new list in hashtable to the associated key(filename)
                g_hash_table_insert(funmetatable , g_strdup( filename ), tagset);

            }


    //Unloack Mutex//
    if(err=pthread_mutex_unlock(&lockermetadata))
    {
        perror2("Failed to lock()",err);
    }

   // printf("Exit  Function insertMetadata\n");

    return funmetatable;

}

TAG* findMaxTag(GHashTable * funmetatable , TAG* filetag , char *filename  , int *hasError)
{

     printf("Enter findMAxTag  function \n");


    //Initialize variable hasError
    //If error exist the variable will become 1
    *hasError=0;

    //Pointers for tags
    TAG* secureTag = NULL;
    TAG* existTag  = NULL;


    //Check if the filename exist in metadata table
    //otherwise you must fill the hasError variable
    //if(g_hash_table_contains(funmetatable , filename) == 1 )
    //{

    //Pointer to a list to retrieve all the tags for the
    //specific filename
    GSList *pointlist=NULL;


    //Retrieve all the tags which are associated with the specific filename
    pointlist =(GSList*) g_hash_table_lookup(funmetatable,filename);


    if(pointlist != NULL)
    {
            //To point at the beginning of the list and to go through all the list
            //to detect if the tag that looking for exist. Otherwise, must return
            //the max secure tag
            GSList* iter=NULL;

            //A temp tag to go through the list tags
            TAG* curTag  = NULL;

            for(iter=pointlist; iter; iter = iter->next)
            {
                //Point to each tag in list
                curTag = (TAG *) iter->data;

                //Check if the tag that looking for exist in the list
                if(curTag->num == filetag->num && curTag->clientID == filetag->clientID )
                {
                    //Tag found
                    existTag=curTag;
                    break;
                }
                    //Looking for secure tag
                else if(curTag->isSecure == 1)
                {
                    //Secure tag found
                    secureTag = curTag;
                }
            }
    }

    printf("Exit findMAxTag  function \n");
    //Check if you found the tag , otherwise you must return the max secure tag
    if(existTag != NULL)
    {
        return existTag;
    }
    else if(secureTag != NULL)
    {
        return secureTag;
    }
    else
    {
        return NULL;
    }

}

GHashTable *   deleteUnsecureTags(GHashTable * funmetatable ,TAG* secureTag , struct replicaHeader *header )
{

   // printf("Enter delete  function insertMetadata\n");

    //Check if the filename exist in metadata table
    //otherwise you must fill the hasError variable
    //if(g_hash_table_contains(funmetatable , header->filename) == 1 )
    //{
        //Pointer to a list to retrieve all the tags for the
        //specific filename
        GSList *pointlist = NULL;

        int err=0;

        //Store all the tags file that you must
        //delete
        GSList *delSet = NULL;


        //Retrieve all the tags which are associated with the specific filename
        pointlist = (GSList *) g_hash_table_lookup(funmetatable, header->filename);


        if(pointlist != NULL)
        {

                //To point at the beginning of the list and to go through all the list
                //to detect if the tag that looking for exist. Otherwise, must return
                //the max secure tag
                GSList *iter = NULL;

                //A temp tag to go through the list tags
                TAG *curTag = NULL;

                //Go through all the list to find which tag are smaller from
                //the secure tags
                //So, you have to find all the smaller tags and delete it
                for (iter = pointlist; iter; )
                {
                    //Point to each tag in list
                    curTag = (TAG *) iter->data;

                    //Find all the tags that are smaller than the secure tag
                    //In this case, you must delete all the tags that are smaller
                    if (curTag->num < secureTag->num || curTag->num == secureTag->num && curTag->clientID < secureTag->clientID)
                    {
                        TAG* deletfiletag = ( TAG *)malloc(sizeof( TAG));
                        deletfiletag->num = curTag->num ;
                        deletfiletag->clientID = curTag->clientID ;
                        deletfiletag->isSecure = curTag->isSecure ;

                        //Insert the tag in the delete set
                        //You have to delete this tag
                        delSet=g_slist_prepend( delSet , deletfiletag );

                        //Delete the tag that you found from the list of metadata
                         iter=iter->next;

                         pointlist = g_slist_remove(pointlist , curTag);
                       //  free(curTag);
                    }
                    else
                    {
                          iter=iter->next;
                    }
                }



    /*    for (iter = delSet; iter; iter=iter->next)
        {
            //Point to each tag in list
            curTag = (TAG *) iter->data;

            pointlist = g_slist_remove(pointlist , curTag);
           // free(curTag);
        }*/


        //Reuse temporary tag
        curTag = NULL;

        unsigned    int length;
        //Go through the list to delete all the tags
        for (iter = delSet; iter; iter=iter->next)
        {
            //Store the status of remove function
            int status;

            //Point to each tag in list
            curTag = (TAG *) iter->data;

            char delfile[256];
            bzero(delfile,sizeof(delfile));
            length=sprintf(delfile,"%s_%d_%d_%d.%s",header->filename,curTag->num ,curTag->clientID,curTag->isSecure,header->filetype );
            delfile[length-1]='\0';

            //remove the file with the specific tag
            status=remove(delfile);

            //Check if remove correct the file
            if(status !=0 )
            {
                printf("Unable to delete file:%s\n",delfile);
                perror("Remove error");
            }
        }


        //Go through the list to delete all the tags
        for (iter = delSet; iter; iter=iter->next)
        {
            //Point to each tag in list
           // curTag = (TAG *) iter->data;
            free(iter->data);
        }

        g_slist_free(delSet);

        //Insert updated list in hashtable to the associated key(filename)
        g_hash_table_insert(funmetatable , header->filename , pointlist);

    }




    //printf("Exit delete  function insertMetadata\n");


    return funmetatable;

}

GHashTable *   addSecure(GHashTable * funmetatable ,TAG* secureTag , struct replicaHeader *header )
{

    printf("Enter Add1 secure function insertMetadata\n");


    //Check if the filename exist in metadata table
    //otherwise you must fill the hasError variable
    //if(g_hash_table_contains(funmetatable , header->filename) == 1 )
    //{
        //Pointer to a list to retrieve all the tags for the
        //specific filename
        GSList *pointlist = NULL;

        int err=0;

        //Store all the tags file that you must
        //delete
        GSList *delSet = NULL;



         //Retrieve all the tags which are associated with the specific filename
         pointlist = (GSList *) g_hash_table_lookup(funmetatable, header->filename);

         if(pointlist != NULL)
         {

                //To point at the beginning of the list and to go through all the list
                //to detect if the tag that looking for exist. Otherwise, must return
                //the max secure tag
                GSList *iter = NULL;

                //A temp tag to go through the list tags
                TAG *curTag = NULL;

                int ret;
                char oldname[255];
                char newname[255];
                unsigned    int length;

                //Clears buffers
                bzero(oldname,sizeof(oldname));
                bzero(newname,sizeof(newname));

                length=sprintf(oldname,"%s_%d_%d_0.%s",header->filename,header->tag->num,header->tag->clientID,header->filetype);
                oldname[length-1]='\0';

                length = sprintf(newname,"%s_%d_%d_1.%s" ,header->filename,header->tag->num,header->tag->clientID,header->filetype);
                newname[length-1]='\0';

                //Go through all the list to find which tag are smaller from
                //the secure tags
                //So, you have to find all the smaller tags and delete it*/
                for (iter = pointlist; iter; iter = iter->next )
                {
                    //Point to each tag in list
                     curTag = (TAG *) iter->data;

                    //Find all the tags that are smaller than the secure tag
                    //In this case, you must delete all the tags that are smaller
                    if (curTag->num == secureTag->num &&
                        curTag->clientID == secureTag->clientID && curTag->isSecure != 1 )
                    {
                       //Define the file as secure
                         curTag->isSecure=1;

                         //Rename the file with the secure tag
                          ret = rename(oldname, newname);

                        if(ret !=0)
                        {
                            printf("Error: unable to rename the file :%s",header->filename);
                        }

                        //If the found the file
                        break;
                    }
                }

         }




    printf("Exit Add secure function insertMetadata\n");

    return funmetatable;

}

TAG* findSecureTag(GHashTable * funmetatable , char *filename )
{
    //Pointers for tags
    TAG *secureTag = malloc(sizeof(TAG));

    //Initialization of secureTag
    secureTag->num = 0;
    secureTag ->clientID = 0;
    secureTag ->isSecure = 0;

    int err=0;

    //Check if the filename exist in metadata table
    //otherwise you must fill the hasError variable
    //  if(g_hash_table_contains(funmetatable , filename) == 1 )
    //{

    //Pointer to a list to retrieve all the tags for the
    //specific filename
    GSList *pointlist=NULL;

    //Lock strtok due to is not deadlock free
    if(err=pthread_mutex_lock(&lockermetadata))
    {
        perror2("Failed to lock()",err);
    }


        //Retrieve all the tags which are associated with the specific filename
        pointlist =(GSList*) g_hash_table_lookup(funmetatable,filename);

        if(pointlist != NULL)
        {

                //To point at the beginning of the list and to go through all the list
                //to detect if the tag that looking for exist. Otherwise, must return
                //the max secure tag
                GSList* iter=NULL;

                //A temp tag to go through the list tags
                TAG* curTag  = NULL;

                for(iter=pointlist; iter; iter = iter->next)
                {
                    //Point to each tag in list
                    curTag = (TAG *) iter->data;

                    //Check if the tag that looking for exist in the list
                    if( (curTag->num > secureTag->num && curTag ->isSecure == 1 ) || (curTag->num == secureTag->num && curTag->clientID > secureTag->clientID && curTag->isSecure == 1) )
                    {
                        //Tag found
                        secureTag ->isSecure = curTag->isSecure;
                        secureTag->clientID = curTag ->clientID;
                        secureTag->num = curTag ->num;
                    }
                }
            }

    //Unloack Mutex//
    if(err=pthread_mutex_unlock(&lockermetadata))
    {
        perror2("Failed to lock()",err);
    }

        if(secureTag->num == 0)
        {
            return NULL;
        }
        else
        {
            return secureTag;
        }

}

int isTagExist(GHashTable * funmetatable , char *filename , TAG* secureTag )
{

    //Initialize with the value -1. Means that tag does not exist
    int isFound = -1;

    //Pointer to a list to retrieve all the tags for the
    //specific filename
    GSList *pointlist=NULL;

    //Retrieve all the tags which are associated with the specific filename
    pointlist =(GSList*) g_hash_table_lookup(funmetatable,filename);

    if(pointlist != NULL)
    {
        //To point at the beginning of the list and to go through all the list
        //to detect if the tag that looking for exist. Otherwise, must return
        //the max secure tag
        GSList* iter=NULL;

        //A temp tag to go through the list tags
        TAG* curTag  = NULL;

        for(iter=pointlist; iter; iter = iter->next)
        {
            //Point to each tag in list
            curTag = (TAG *) iter->data;

            //Check if the tag that looking for exist in the list
            if( ( curTag->num == secureTag->num && curTag->clientID == secureTag->clientID ))
            {
                secureTag->isSecure =1;
                isFound = 1;
            }
        }

        //Find the max secure tag
        TAG *maxSecureTag = findSecureTag(funmetatable , filename);
        if(maxSecureTag != NULL )
        {
            if(maxSecureTag ->num < secureTag->num && isFound == -1 || maxSecureTag ->num == secureTag ->num && maxSecureTag ->clientID < secureTag ->clientID && isFound ==-1 )
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return isFound;

        }
    }
    else
    {
        //Return -1 in case where the tag does not exist
        return -1;
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

    int         bytes;
    int         len;
    int         file_size;
    char        *filename=NULL;

    struct  replicaHeader *msg;
    char    buf[BUFSIZE];

    //While the client is connect to the system you have to keep the connection
    while(1)
    {
        //If connection is established then start communicating //
        //Initialize buffer with zeros //
        bzero(buf,sizeof(buf));
        //Waiting...to receive data from the client//
        if (recv(acptsock, buf, 256, 0) < 0)
        {
            perror("send() failed");
            close(acptsock);
            pthread_exit((void *) 0);
        }

        //Check if direc received a message
        if(strlen(buf) != 0)
        {
            //Show the message that received
            printf("----------------------------------\n");
            printf("accept_thread received: %s\n", buf);
        }
        else if(strlen(buf) == 0)
        {
             //printf("Received an empty message!\n");
            //close(acptsock);
             pthread_exit((void *) 0);
        }

        //Retrieve the message from the client
        msg = decode(buf);

        bzero(buf,sizeof(buf));
        //bzero(buf,sizeof(buf));
        //Received the actual data for the file

        if( strcmp(msg->type,"WRITE") == 0 )
        {
              //To store if received the data object successful or not
              int isSuccess=-1;
              int sendSize=0;

              //Receive the file
              isSuccess=ftp_recv(acptsock,msg);

              if(isSuccess == SUCCESS)
              {
                  //Store the metadata of new the file in the hashtable
                  metadatatable =  insertMetadata(metadatatable,msg->tag,msg->filename);

                  //Send an acknowledge to the client that received the object
                  bzero(buf,sizeof(buf));

                  //The format of the acknowledgment
                  sprintf(buf,"WRITE-OK,%d,%s", msg->msgID, msg->filename);
                  printf("%s\n",buf);
                  sendSize=strlen(msg->filename) + sizeof(int) + 13;
                  //sent the acknowledgment that received the file
                  if (send(acptsock, buf, sendSize, 0) < 0)
                  {
                      perror("WRITE-send() failed\n");
                  }
              }
        }
        else if(strcmp(msg->type,"READ") == 0 )
        {
            int error=0;
            int err=0;

            //Retrieve curTag
            TAG *curTag=NULL;

            //Check if the tag exist in the system. If exist return the tag or
            //retrun the latest tag that is secure
            //Lock strtok due to is not deadlock free
            if(err=pthread_mutex_lock(&lockermetadata))
            {
                perror2("Failed to lock()",err);
            }

               curTag = findMaxTag( metadatatable , msg->tag , msg->filename , &error);

            //Unloack Mutex//
            if(err=pthread_mutex_unlock(&lockermetadata))
            {
                perror2("Failed to unlock()",err);
            }

            if(curTag !=NULL)
            {

                msg->tag->num = curTag->num;
                msg->tag->clientID = curTag->clientID;
                msg->tag->isSecure = curTag->isSecure;

                printf("ReadOperation, FoundMax, Tag:%d,clientID:%d,isSecure:%d\n", msg->tag->num, msg->tag->clientID,
                       msg->tag->isSecure);

                //Send the file to client
                send2ftp(acptsock, msg);
            }
        }

        else if(strcmp(msg->type,"SECURE") == 0 )
        {
            //Store error of pthread
            int err;


            //Lock strtok due to is not deadlock free
            if(err=pthread_mutex_lock(&lockermetadata))
            {
                perror2("Failed to lock()",err);
            }
                  metadatatable = addSecure(metadatatable,msg->tag,msg);

                  metadatatable =  deleteUnsecureTags(metadatatable , msg->tag , msg);

            //Unloack Mutex//
            if(err=pthread_mutex_unlock(&lockermetadata))
            {
                perror2("Failed to lock()",err);
            }


        }

        //clear buffer
        bzero(buf,sizeof(buf));

        //deallocate
        free(msg->filename);
        free(msg->filetype);
        free(msg->tag);
        free(msg);

    }//While 1

}

void initialization()
{
    //Initialize metadata table and also determine hash function
    metadatatable = g_hash_table_new( g_str_hash, g_str_equal);


    /*Initialization of mutex for strtok in decode function*/
    pthread_mutex_init(&locker,NULL);

    //Initialization of mutex for lock the metadata table
    pthread_mutex_init(&lockermetadata,NULL);

    int i=0;

    //delete
   /* int i=0;
    TAG *mytag = g_new(TAG, 1);
    for(i=0; i<5; i++)
    {
        mytag->num = i;
        mytag->clientID = 0;
        mytag->isSecure = 0;

        metadatatable = insertMetadata(metadatatable, mytag, "m");
    }

    mytag->num = 3;
    mytag->clientID = 0;
    mytag->isSecure = 1;

    metadatatable = insertMetadata(metadatatable, mytag, "110");*/


    //Allocate the array to store all the sockaddr_in for directories
    direc_sockaddr=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in ) * MAX_DIRECTORIES );

    //Allocate the array to store all the sockaddr_in for Replicas
    replica_sockaddr=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in ) * MAX_REPLICAS );

    //Allocate the array to store all the sockets descriptor for each directory
    direcSocks=( int *)malloc(sizeof(int ) * MAX_DIRECTORIES );

    //Allocate the array to store all the sockets descriptor for each Replicas
    replicaSocks=(int *)malloc(sizeof(int ) * MAX_REPLICAS );


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

        if( NODEID != replicaNodes[i].port  )
        {

            replica_sockaddr[i].sin_family = AF_INET; /*Internet domain*/
            replica_sockaddr[i].sin_addr.s_addr = inet_addr(replicaNodes[i].ip_addr);
            replica_sockaddr[i].sin_port = htons(replicaNodes[i].port);
        }
    }//For statement

    /*Go through all the available directories to establish a connection.*/
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
        if( NODEID != replicaNodes[i].port  )
        {
            //Create a socket and check if is created correct
            if ((replicaSocks[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Socket() failed");
                exit(1);
            }
        }

    }//For statement

}

void signal_handler()
{
    printf("Signal Handled here\n");

    //Retrieve all the key values from the hashtable
    GList *hashPoint = g_hash_table_get_keys (metadatatable);


    GSList* iter=NULL , *iter2=NULL , *pointlist=NULL;

    TAG *tempTag=NULL;

    for(iter=hashPoint; iter; iter = iter->next)
    {
       // printf("key:%s\n" , (char *)iter->data);

        //Retrieve all the tags which are associated withe the specific filename
        pointlist =(GSList*) g_hash_table_lookup(metadatatable, iter->data );

        if(pointlist != NULL)
        {
            for(iter2=pointlist; iter2; iter2 = iter2->next)
            {
                tempTag = ( TAG*) iter2->data;

                printf("key:%s , Num:%d, ClientID:%d \n" , (char *)iter->data ,tempTag->num , tempTag->clientID );
                free(tempTag);
            }
            g_slist_free(pointlist);
        }

       //  tempTag = ( TAG*) iter->data;
      //  printf("Num:%d, ClientID:%d \n" , tempTag->num , tempTag->clientID );

        /*for(tagList=iter; tagList; tagList = tagList->next)
        {
            tempTag = ( TAG*) tagList->data;

            printf("Num:%d, ClientID:%d \n" , tempTag->num , tempTag->clientID );

        }*/
    }

    //Destroy HashTable
    g_hash_table_destroy(metadatatable);

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

    //Initialization functions
    initialization();

    //Retrieve input parameters
    port=atoi(argv[2]);
    NODEID= port;

    printf("--------------------------------------\n");
    printf("\nStarting Replica on port:%d ....\n" , port);
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