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

struct replicaHeader
{
    char *type;
    char  *filename;
    char *filetype;
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




#endif //DISTRIBUTEDALGORITHMS_REPLICA_H
