//
// Created by elias on 2/17/16.
//

#ifndef DISTRIBUTEDALGORITHMS_FILEMANAGER_H
#define DISTRIBUTEDALGORITHMS_FILEMANAGER_H

#include <glib.h>

typedef struct
{
    long client_id;
    GString *username;

}CLIENT;

typedef struct
{
    char *type;
    long MSGID;
    char *filename;
    char *filetype;
    char *username;
    long owner;

}FILEHEADER;

typedef struct
{
    GString *filename;
    long fileid;
    long owner;
    GString *username;
}METADATA;


/***********************************************************************************/
/*                                 VARIABLES                                       */
/***********************************************************************************/
//Counter for client IDs
long countClientIds;

//Counter for File IDs
long countFileIds;


/***********************************************************************************/
/*                            PROTOTYPES                                           */
/***********************************************************************************/

int decode(char *buf , FILEHEADER *header );

GString *getfilelist(GString* strset);

void *bind_thread(void *port);

void *accept_thread(void *accept_sock);

long registerClient(char *username);

long registerFile(char *filename , long owner);

long lookUpFileID(char *filename , long owner);

void initialization();

void signal_handler();


#endif //DISTRIBUTEDALGORITHMS_FILEMANAGER_H
