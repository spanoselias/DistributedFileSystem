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
    long num;
    long clientID;
};

struct message
{
    char *type;
    long  msg_id;
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

void shuffle(int *array, size_t n);

int isMaxTag(struct TAG *tag1 , struct TAG tag2 );

char *checksum_get(char *filename);

GSList* decode(struct message *msg , char *buf);

void encode(struct message *msg , char *buf , char *type);

GSList* receive_quorum(struct TAG *maxTag, GSList *replicaSet,int ischeck , int *ismajor);

GSList* recvrepliquorum(struct message *msg , GSList *replicaSet);

GHashTable *storefiletag(char *filename, struct TAG *filetag );

GSList  *read_Query(struct cmd *cmdmsgIn  ,  struct TAG *tag , struct message *msg );

int read_Inform( struct cmd *cmdmsgIn  ,  struct TAG *tag , struct message *msg , GSList  *setOfReplica  );

int reader_oper(int msg_id , struct cmd *cmdfile );

int writer_oper(int msg_id , struct cmd *cmdfile  );

int writeLog(FILE *log_file , char *info);

int read_cmd(char *cmd_str , struct cmd *cmdmsg );

void readConfig(char *filename);

int reqClientID(char *username);

long reqCreate(char *filename);

long reqFileID(struct cmd *cmdmsg , long clientID);

int connect2Replicas();

void conn2direc();

int conn2filemanager();

int get_file(int sock, struct message *msg );

int send2ftp(struct cmd *msgCmd, int newsock , struct TAG *tagIn , long msgIDIn );

int get_filelist();

void inisialization();

void unitTest(char *filename , char *filetype , char *username);


#endif //DISTRIBUTEDALGORITHM_CLIENT_H
