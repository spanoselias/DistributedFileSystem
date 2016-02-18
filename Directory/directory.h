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
    int num;
    int id;
};

struct message
{
    char *type;
    int  msg_id;
    GSList* replicaSet;
    struct tagID  tag;
    int fileID;
    char *filename;
    char *permission;
};

struct metadata
{
    int  file_id;
    GString* permission;
    struct tagID  tag;
    GSList* replicaSet;
    GString *filename;
};

struct sockaddr_in serv_addr;

/***********************************************************************************/
/*                            PROTOTYPES                                           */
/***********************************************************************************/



#endif //DISTRIBUTEDALGORITHM_DIRECTORY_H
