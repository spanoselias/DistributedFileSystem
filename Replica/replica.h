/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 09/10/2015                                                                 */
/*Filename:Replica.h                                                               */
/*                                                                                 */
/***********************************************************************************/
#ifndef DISTRIBUTEDALGORITHMS_REPLICA_H
#define DISTRIBUTEDALGORITHMS_REPLICA_H

#include <glib.h>

/***********************************************************************************/
/*                               FUNCTION PROTOTYPES                               */
/***********************************************************************************/
void *bind_thread(void *port);
void *accept_thread(void *accept_sock);
void *gossip(void *header );

typedef struct
{
    unsigned  int num;
    unsigned int clientID;
    unsigned int isSecure;

}TAG;

typedef struct
{
    char *filename;
    int fileid;
    TAG *tag;

}METADATA;


struct replicaHeader
{
    char *type;
    char  *filename;
    char *filetype;
    long fileid;
    TAG *tag;
    int msgID;
    int fileSize;
    char *checksum;
};

struct serinfo
{
    char ip_addr[16];
    int port;
};

/***********************************************************************************/
/*                            PROTOTYPES                                           */
/***********************************************************************************/

void readConfig(char *filename , int port);

char *checksum_get(char *filename);

void conn2direc();

int connect2Replicas();

struct replicaHeader* decode(char *buf );

int send2ftp(int newsock , struct replicaHeader *msg );

int ftp_recv(int sock, struct replicaHeader *msg );

GHashTable *  insertMetadata(GHashTable * funmetatable ,  TAG* filetag , char *filename );

TAG* findMaxTag(GHashTable * funmetatable , TAG* filetag , char *filename  , int *hasError);

GHashTable *   deleteUnsecureTags(GHashTable * funmetatable ,TAG* secureTag , struct replicaHeader *header);

GHashTable *   addSecure(GHashTable * funmetatable ,TAG* secureTag , struct replicaHeader *header );

TAG* findSecureTag(GHashTable * funmetatable , char *filename );

int isTagExist(GHashTable * funmetatable , char *filename , TAG* secureTag );

void *bind_thread(void *port);

void *accept_thread(void *accept_sock);

void initialization();

void signal_handler();


#endif //DISTRIBUTEDALGORITHMS_REPLICA_H
