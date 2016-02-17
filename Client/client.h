//
// Created by elias on 9/29/15.
//

#ifndef DISTRIBUTEDALGORITHM_CLIENT_H
#define DISTRIBUTEDALGORITHM_CLIENT_H

#include <glib.h>

/************************************/
/*           STRUCTS                */
/************************************/

struct TAG
{
    unsigned int num;
    unsigned  int clientID;
};

struct message
{
    char *type;
    int  msg_id;
    struct TAG tag;
    GSList* replicaSet;
    int  fileID;
    char *filename;
    char *filetype;
    unsigned int fileSize;
    char *checksum;
};

struct cmd
{
    char oper[9];
    char *filename;
    char *fileType;
};

struct serinfo
{
    char ip_addr[16];
    int port;
};

/***********************************************************************************/
/*                               FUNCTION PROTOTYPES                               */
/***********************************************************************************/



#endif //DISTRIBUTEDALGORITHM_CLIENT_H
