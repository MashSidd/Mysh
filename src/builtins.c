#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "builtins.h"
#include "io_helpers.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// ====== Command execution =====

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd) {
    ssize_t cmd_num = 0;
    while (cmd_num < BUILTINS_COUNT &&
           strncmp(BUILTINS[cmd_num], cmd, MAX_STR_LEN) != 0) {
        cmd_num += 1;
    }
    return BUILTINS_FN[cmd_num];
}


// ===== Builtins =====

/* Prereq: tokens is a NULL terminated sequence of strings.
 * Return 0 on success and -1 on error ... but there are no errors on echo. 
 */
ssize_t bn_echo(char **tokens, int pipeFlag, int* proc, int pn, char** procNames) {
    ssize_t index = 1;
    int i=0;
    if (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        display_message(tokens[index] );
        index++;
    }
    while (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        display_message(" ");
        display_message(tokens[index] );
        index += 1;
    }
    if(tokens[index-1] != NULL){
        while(tokens[index-1][i] != '\0'){
            i++;
        }
        //if (i>0 && tokens[index -1][i-1]!='\n'){
            display_message("\n");
        //}
    }
    else{
        display_message("\n");
    }

    return 0;
}
ssize_t bn_ls(char **tokens, int pipeFlag, int* proc, int pn, char** procNames){
    DIR *fold;
    int spot=1;
    int spotHolder = 0;
    char* substr = "";
    substr=(char*)malloc(MAX_STR_LEN*sizeof(char));
    int substrer = 0;
    int recTrack = 0;
    int dCheck=0;
    int level = -1;
    char* levelStr = "";
    int hasPath = 0;
    levelStr=(char*)malloc(MAX_STR_LEN*sizeof(char));
    if (tokens[spot] != NULL){ //check if argument provided
        while (tokens[spot][0]=='-' && tokens[spot][1]=='-'){ //determine tags used
            if (tokens[spot][2]=='r'&&tokens[spot][3]=='e'&&tokens[spot][4]=='c'){
                recTrack = 1;
            }
            else if(tokens[spot][2]=='d'){
                dCheck=1;
                spot++;
                if(tokens[spot]!=NULL){
                    strcpy(levelStr,tokens[spot]);
                }
            }
            else if (tokens[spot][2]=='f'){
                spot++;
                strcpy(substr,tokens[spot]);
                substrer = 1;
            }
            else{
                display_error("ERROR: Invalid path",tokens[spot]);
                display_message("");
                free(levelStr);
                free(substr);
                return -1;
            }
            spot++;
            if (tokens[spot]== NULL){
                break;
            }
        }
    }
    if (tokens[spot] != NULL){
        hasPath = 1;
        spotHolder = spot;
        fold = opendir(tokens[spot]);
        if (fold == NULL){
            display_error("ERROR: Invalid path ",tokens[spot]);
            display_message("");
            free(levelStr);
            free(substr);
            return -1;
        }
    }
    spot++;
    if (tokens[spot] != NULL){ //check if argument provided
        while (tokens[spot][0]=='-' && tokens[spot][1]=='-'){ //determine tags used
            if (tokens[spot][2]=='r'&&tokens[spot][3]=='e'&&tokens[spot][4]=='c'){
                recTrack = 1;
            }
            else if(tokens[spot][2]=='d'){
                dCheck=1;
                spot++;
                strcpy(levelStr,tokens[spot]);
            }
            else if (tokens[spot][2]=='f'){
                spot++;
                strcpy(substr,tokens[spot]);
                substrer = 1;
            }
            else{
                display_error("ERROR: Invalid path",tokens[spot]);
                display_message("");
                free(levelStr);
                free(substr);
                return -1;
            }
            spot++;
            if (tokens[spot]== NULL){
                break;
            }
        }
    }
    if (dCheck != recTrack){ //check if only one of either a d or rec was provided
        display_error("need both d and rec","");
        display_message("\0");
        free(levelStr);
        free(substr);
        return -1;
    }
    if(dCheck != 0){
        level = atoi(levelStr);
    }
    if (tokens[spot] != NULL && hasPath != 1){
        hasPath = 1;
        fold = opendir(tokens[spot]);
        if (fold == NULL){
            display_error("ERROR: Invalid path ",tokens[spot]);
            display_message("");
            free(levelStr);
            free(substr);
            return -1;
        }
    }
    else if (hasPath != 1){ //default use of ls, print this folder's contents
        fold = opendir(".");
    }
    
    struct  dirent *entry = NULL;
    int parse = 0;

    while ( (entry = readdir(fold)) && recTrack == 0 ){ //loop for non-recursive ls print
        if (substrer == 0){
            display_message(entry->d_name);
            display_message("\n");
        }
        else{
            if (strstr(entry->d_name, substr)!=NULL){
                display_message(entry->d_name);
                display_message("\n");
            }
            
        }
        parse = parse + 1;
    }
    rewinddir(fold);//reset fold
    if (recTrack != 0){ //check if recursive was requested
        if(hasPath != 0){ //if a path was provided
            recursivePrint(tokens[spotHolder], level, substrer, substr); //call resursive print function with path provided
        }
        else{
            recursivePrint(".",level, substrer, substr);//call resursive print function with current path
        }
    }
    free(levelStr);
    closedir(fold);
    free(substr);
    return 1;
}
int recursivePrint(char* curDir, int level, int gate, char* sub){ //recursive ls print method
    if (level ==0){
        return 0;
    }
    char *next =(char*) malloc(MAX_STR_LEN*sizeof(char));
    DIR *fold;
    fold = opendir(curDir);
    struct  dirent *entry = NULL;
    while ((entry = readdir(fold))){
        if (gate == 0){
            display_message(entry->d_name);
            display_message("\n");
        }
        else{
            if(strstr(entry->d_name, sub)!=NULL){
                display_message(entry->d_name);
                display_message("\n");
            }
        }
        
    }
    rewinddir(fold);
    while ((entry = readdir(fold))){
        if (entry->d_type == DT_DIR && strcmp(entry->d_name,"..") && strcmp(entry->d_name,".")){
            strcpy(next,curDir);
            strcat(next,"/");
            strcat(next,entry->d_name);
            recursivePrint(next, level -1, gate, sub);
        }
    }
    closedir(fold);
    free(next);
    return 0;
}
ssize_t bn_cd(char **tokens, int pipeFlag, int* proc, int pn, char** procNames){
    int i = 3;
    if (tokens[1] == NULL){ //check if no arguments passed, move to root directory
        chdir("/");
    }
    else if (tokens[2] != NULL){ //check if too many arguments
        display_error("ERROR: Too many arguments", "");
        display_message("");
        return -1;
    }
    else if(tokens[1][0] == '.' && tokens[1][1] == '.'&& tokens[1][2] == '.'){ //check if substring of "..." appears
        chdir(".."); 
        chdir("..");//move two directories up
        while (tokens[1][i] == '.'){ //if another '.' is found
            chdir(".."); //move another directory up
            i++; //increment counter
        }
    }
    else if (chdir(tokens[1])!= 0){ //change directory to path, check if successful
        display_error("ERROR: Invalid path ", tokens[1]);
        display_message("");
        return -1;
    }
    if(pipeFlag!=0){
        display_message(" ");
    }
    return 0;
}

ssize_t bn_cat(char** tokens, int pipeFlag, int* proc, int pn, char** procNames){
    FILE* fp;
    char* c;
    c = malloc(sizeof(char)*MAX_STR_LEN);
    if (tokens[1]!=NULL){
        fp = fopen(tokens[1], "r"); //create read only file  based on user input
    }
    if (tokens[1] == NULL){ //check if no arguments passed, move to root directory
        display_error("ERROR: No input source provided","");
        display_message("");
        free(c);
        return -1;
    }
    else if (fp == NULL && pipeFlag <= 0){ //check if file is not valid
        display_error("ERROR: Cannot open file ",tokens[1]);
        display_message("");
        free(c);
        return -1;
    }
    else if (fp == NULL){
        bn_echo(tokens,pipeFlag, NULL, 0, NULL);
    }
    else{
        while(fgets(c, MAX_STR_LEN, fp)){ //move to next line, loop while there is more text in input file
            display_message(c); //display one line 
        }
        fclose(fp); //close file pointer
        display_message("\n");
    }
    free(c);
    return 0;
}
ssize_t bn_wc(char **tokens, int pipeFlag, int* proc, int pn, char** procNames){
    FILE* fp;
    char c;
    char*converter;
    converter = malloc(sizeof(char)*MAX_STR_LEN);

    int charCount = 0, wordCount=0, newLineCount = 0, flip=2;
    if (tokens[1]!=NULL){
        fp = fopen(tokens[1], "r"); //create read only file  based on user input
    }
    if (tokens[1] == NULL){ //check if no arguments passed, move to root directory
        display_error("ERROR: No input source provided","");
        free(converter);
        return -1;
    }
    else if (fp == NULL){ //check if file is not provided
        if (pipeFlag <=0){
             display_error("ERROR: Cannot open file","");
             display_message("");
            free(converter);
            return -1;
        }
        int i = 1;
        int j, len;
        int emptyFlag = 0;
        while(tokens[i]!=NULL){
            len = strlen(tokens[i]);
            for (j=0; j<len; j++){
                c = tokens[i][j];
                if(c==' '||c=='\t'||c=='\r'||c=='\n'||c=='\v'||c=='\f'){
                    if(flip != 1){
                        flip = 1;
                        wordCount++;
                    }
                    if (c == '\n'){
                        newLineCount++;
                    }
                }
                else if (c!='\0'){
                    flip = 0;
                }
                if (c!='\0'){
                    emptyFlag = 1;
                    charCount++;
                }
            }
            i++;
        }
        i--;
        wordCount = wordCount + i;
        wordCount--;
        charCount = charCount + i;
        charCount = charCount - 1;
        if (emptyFlag == 0){
            wordCount = 0;
        }

    }
    else{ //normal call
        c = fgetc(fp);
        while(c!=EOF){
            if(c==' '||c=='\t'||c=='\r'||c=='\n'||c=='\v'||c=='\f'){
                if(flip != 1){
                    flip = 1;
                    wordCount++;
                }
                if (c == '\n'){
                    newLineCount++;
                }
            }
            else if (c!='\0'){
                flip = 0;
            }
            if (c!='\0'){
                charCount++;
            }
            c = fgetc(fp);
        }
        if (flip == 0 ){
            wordCount++;
        }
    }
    sprintf(converter, "word count %d \ncharacter count %d \nnewline count %d \n",wordCount,charCount,newLineCount);
    display_message(converter);
    free(converter);
    return 0;
}
ssize_t bn_kill(char **tokens, int pipeFlag, int* proc, int pn, char** procNames){
    int signal;
    int err;
    pid_t id;
    if(tokens[1] == NULL){
        display_error("ERROR: No PID provided","");
        display_message("");
        return -1;
    }
    else{
        id = atoi(tokens[1]); 
    }
    if(tokens[2]==NULL){
        signal = SIGTERM;
    }
    else{
        signal = atoi(tokens[2]);
    }
    err = kill(id,signal);
    if (err<0){
        if(errno == 3){
            display_error("ERROR: The process does not exist ", tokens[1]);
            display_message("");
            return -1;
        }
        else if(errno == 22){
            display_error("ERROR: Invalid signal specified ", tokens[2]);
            display_message("");
            return -1;
        }
        else{
            display_error("Error: could not kill signal","");
            display_message("");
            return -1;
        }
    }
    return 0;
}
ssize_t bn_ps(char **tokens, int pipeFlag, int* proc, int pn, char** procNames){
    int i;
    char* tok;
    char* convNum;
    convNum = malloc(MAX_STR_LEN*sizeof(char));
    char* doneMessage;
    doneMessage = malloc(MAX_STR_LEN*sizeof(char));
    char* cloneMessage;
    cloneMessage = malloc(MAX_STR_LEN*sizeof(char));
    for (i = 0; i < pn; i++){
        if(proc[i]>=0){
            strcpy(cloneMessage,procNames[i]);
            tok = strtok(cloneMessage, " ");
            strcpy(doneMessage, tok);
            strcat(doneMessage, " ");
            sprintf(convNum,"%d",proc[i]);
            strcat(doneMessage,convNum);
            strcat(doneMessage,"\n");
            display_message(doneMessage);
        }
    }
    free(doneMessage);
    free(convNum);
    free(cloneMessage);
    if(pn == 0 && pipeFlag == -1){
        display_message(" ");
    }
    return 0;
}/*
ssize_t bn_startServer(char **tokens, int pipeFlag, int* proc, int pn, char** procNames) {
    if(tokens[1] == NULL){
        display_error("ERROR: ","No port provided");
        return -1;
    }
    int port;
    port = atoi(tokens[1]);
    if(port == 0){
        display_error("ERROR: ","Invalid port provided");
        return -1;
    }
    int net_socket;
    net_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(net_socket == -1){
        display_error("ERROR: ","Invalid port provided");
        return -1;
    }
    int pid = fork();
    if(pid == 0){
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int is_connected = bind(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(is_connected == -1){
        display_error("ERROR: ","Could not connect");
        return -1;
    }
    char server_resp[256] = "I am sending this";
    listen(net_socket, 5);

    int client_socket;
    client_socket = accept(net_socket, NULL, NULL); //this is where hostname goes i think
    send(client_socket, server_resp, sizeof(server_resp), 0);
    close(net_socket);
    }
    return 0;
}
ssize_t bn_startClient(char **tokens, int pipeFlag, int* proc, int pn, char** procNames) {
    if(tokens[1] == NULL){
        display_error("ERROR: ","No port provided");
        return -1;
    }
    if(tokens[2] == NULL){
        display_error("ERROR: ","No hostname provided");
        return -1;
    }
    int port;
    port = atoi(tokens[1]);
    if(port == 0){
        display_error("ERROR: ","Invalid port provided");
        return -1;
    }
    int net_socket;
    net_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(net_socket == -1){
        display_error("ERROR: ","Invalid port provided");
        return -1;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int is_connected = connect(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(is_connected == -1){
        display_error("ERROR: ","Could not connect");
        return -1;
    }
    char server_resp[256];
    recv(net_socket, &server_resp, sizeof(server_resp),0);
    return 0;
}*/