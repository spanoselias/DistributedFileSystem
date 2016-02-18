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
    char *username;
    long owner;

}FILEHEADER;

typedef struct
{
    GString *filename;
    long fileid;
    long owner;
}METADATA;


void *accept_thread(void *accept_sock);
long registerClient(char *username);
long registerFile(char *filename , long owner);

#endif //DISTRIBUTEDALGORITHMS_FILEMANAGER_H
