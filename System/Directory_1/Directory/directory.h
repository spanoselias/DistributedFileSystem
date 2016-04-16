/***********************************************************************************/
/*                                                                                 */
/*Name: Elias Spanos                                                 			   */
/*Date: 09/10/2015                                                                 */
/*Filename:directory.h                                                             */
/*                                                                                 */
/***********************************************************************************/
#ifndef DISTRIBUTEDALGORITHM_DIRECTORY_H
#define DISTRIBUTEDALGORITHM_DIRECTORY_H

#include <glib.h>

void *bind_thread(void *port);
void *accept_thread(void *accept_sock);

struct tagID
{
    long num;
    long id;
};

struct message
{
    char *type;
    long  msg_id;
    GSList* replicaSet;
    struct tagID  tag;
    long fileID;
    char *filename;
    char *permission;
};

struct metadata
{
    long  file_id;
    GString* permission;
    struct tagID  tag;
    GSList* replicaSet;
    GString *filename;
};

struct sockaddr_in serv_addr;

/***********************************************************************************/
/*                            PROTOTYPES                                           */
/***********************************************************************************/
int findByFileID( struct message *msg , int *freePos);

GSList * insertList(GSList *metadata , GSList *curList );

int IsMaxTag(struct message *tag2);

void inisializations(int portIn);

void *bind_thread(void *port);

void *accept_thread(void *accept_sock);

GSList* decode(struct message *msg , char *buf);

int writeLog(char *info);

void signal_handler();




#endif //DISTRIBUTEDALGORITHM_DIRECTORY_H
