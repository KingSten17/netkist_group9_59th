#include "header.h"
#include "logmsg.h"
#include "DataTransferProtocol.h"
#include "config.h"
#include "threadPool.h"




int handle_input();
command_t get_command_type();
void handle_register();
void handle_login();
void handle_ls();
void handle_cd(char *path);
void handle_pwd();
void handle_rm(char *filename) ;
void handle_mkdir(char *dirname);
void handle_rename(char *oldname, char *newname) ;
void handle_puts(char *filename);
void handle_gets(char *filename);
void handle_message(char *message);
void handle_quit(char *command);
void handle_unknown(char *command);