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
    long  fileID;
    char *filename;
    char *filetype;
    long clientID;
    unsigned int fileSize;
    char *checksum;
    char *permission;
};

struct cmd
{
    char oper[9];
    char *filename;
    char *fileType;
    long fileid;
};

struct serinfo
{
    char ip_addr[16];
    int port;
};

/***********************************************************************************/
/*                               FUNCTION PROTOTYPES                               */
/***********************************************************************************/

long reqCreate(char *filename);
long reqFileID(struct cmd *cmdmsg , long clientID);

#endif //DISTRIBUTEDALGORITHM_CLIENT_H
