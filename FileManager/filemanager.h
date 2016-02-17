//
// Created by elias on 2/17/16.
//

#ifndef DISTRIBUTEDALGORITHMS_FILEMANAGER_H
#define DISTRIBUTEDALGORITHMS_FILEMANAGER_H

#include <glib.h>

typedef struct
{
    unsigned long client_id;
    GString *username;

}CLIENT;


typedef struct
{
    char *type;
    char *filename;
    char *username;
    long owner;

}FILEHEADER;


void *accept_thread(void *accept_sock);
long registerClient(char *username);

#endif //DISTRIBUTEDALGORITHMS_FILEMANAGER_H
