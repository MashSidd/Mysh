#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"

/* BUG NOTES
DINGA DINGADINGADINGA DINGA DINGADINGADINGA DINGA DINGADINGADINGA DINGA DINGADINGADINGA 
ls --rec --d 2 | cat do first and second leads to heap after free.

PLEASE ALSO MAKE SURE START-SERVER DOESN'T KILL SHIT OK THANKS BYE BYE I HATE THIS <3

*/

void handle(int sig_num){
    signal(SIGINT, handle);
    display_message("\nmysh$ ");
}
int main(int argc, char* argv[]) {
    signal(SIGINT, handle);
    char *prompt = "mysh$ "; // TODO Step 1, Uncomment this.
    FILE *fp;

    char input_buf[MAX_STR_LEN + 1];
    input_buf[MAX_STR_LEN] = '\0';
    char *token_arr[MAX_STR_LEN] = {NULL};
    char **variables;
    int varHolder = 0;
    int numVars = 0;
    variables = malloc(sizeof(char*)); //make sure to allocate for MAX STR LEN with each storage
    variables[0] = malloc(sizeof(char)*MAX_STR_LEN);
    int loopCount;
    char* input = "";
    input = malloc (MAX_STR_LEN*sizeof(char*));
    char* remember = "";
    remember = malloc(MAX_STR_LEN*sizeof(char*));
    
    char* in_buff2 = "";
    in_buff2 = malloc(MAX_STR_LEN*sizeof(char*));
    int highTok =0;
    int d;
    int pipeLooper;
    int inLen;
    int pipeCount;
    int writeFlag = 0;
    int orig_in;
    int orig_out;
    int kidFlag = 0;
    pid_t id1 = 1, id2 = 1;
    orig_in = dup(STDIN_FILENO);
    orig_out = dup(STDOUT_FILENO);
    int* processees = malloc(0*sizeof(int)); 
    int processCount = 0;
    int backgroundFlag = 0;
    int bc;
    pid_t bp;
    int stat;
    char* doneMessage;
    doneMessage = malloc(MAX_STR_LEN*sizeof(char));
    char* numConv;
    numConv = malloc(MAX_STR_LEN*sizeof(char));
    char** pNames;
    pNames = malloc(sizeof(char*));
    int backFlag;
    int ij;
    int stopFlag;
    char* makeSure;
    makeSure = malloc(sizeof(char)*MAX_STR_LEN);
    int chekerFlag;
    while (1) { //TODO: MULTIPIPE: DO LOOP...note: wc might be broken. check with tester.... exit?
        // Prompt and input tokenization
        if(id1>0 &&id2>0 ){
        // TODO Step 2:
        // Display the prompt via the display_message function. 
        dup2(orig_in,STDIN_FILENO);
        dup2(orig_out,STDOUT_FILENO);
        backgroundFlag = 0;
        for (bc = 0; bc < processCount; bc++){ //print if process has finished
            if(processees[bc]>=0){
                bp = waitpid(processees[bc],&stat, WNOHANG);
                if (bp == processees[bc]){
                    processees[bc] = -1;
                    strcpy(doneMessage,"[");
                    sprintf(numConv,"%d",bc+1);
                    strcat(doneMessage,numConv);
                    strcat(doneMessage, "]+  Done ");
                    strcat(doneMessage, pNames[bc]);
                    strcat(doneMessage,"\n");
                    display_message(doneMessage);
                }
            }
        }
        for(bc=0; bc <processCount; bc++){ //loop thru entire processes, if all -1 then clear the entire list and reset the counter
            if (processees[bc]>0){
                break;
            }
            else if(bc == processCount-1){
                for (ij=0; ij<processCount; ij++){
                    free(pNames[ij]);
                }
                pNames = realloc(pNames,sizeof(char*));
                processees = realloc(processees,0*sizeof(int));
                processCount = 0;
            }
        }
        display_message(prompt);
        int ret = get_input(input_buf, 0);
        inLen = strlen(input_buf);
        pipeCount = 0;
        size_t firstSpace = 0;
        int stopCount = 0;
        for (pipeLooper = 0; pipeLooper<inLen; pipeLooper++){
            if(input_buf[pipeLooper] == '|'){
                pipeCount++;
            }
            if (stopCount == 0 ){
                firstSpace = firstSpace + 1;
                if (input_buf[pipeLooper] == ' '){
                    stopCount = 1;
                    firstSpace = firstSpace - 1;
                }
            }
            if(inLen>1){
                if(pipeLooper == inLen-1 && input_buf[pipeLooper-1] == '&'){ //MAKE IT WORK FOR & SPACE
                    backgroundFlag = 1;
                    input_buf[pipeLooper-2] = '\0';
                }
            }
        }
        int sysCaller = 0;
        char* firstWord;
        firstWord = malloc(MAX_STR_LEN*sizeof(char));
        strncpy(firstWord, input_buf, firstSpace);
        firstWord[firstSpace] = '\0';
        bn_ptr dummy = check_builtin(firstWord);
        if (dummy == NULL){
            char* combineBin2;
            combineBin2 = malloc(MAX_STR_LEN);
            strcpy(combineBin2, "/bin/");
            strcat(combineBin2,firstWord);
            DIR* checkDir = opendir(combineBin2);
            if (errno != ENOENT){
                sysCaller = 1;
            }
            closedir(checkDir);
            free(combineBin2);
        }
        free(firstWord);
        ssize_t token_count;
        if (pipeCount == 0 || sysCaller == 1){ //NO PIP VERSION
            token_count = tokenize_input(input_buf, token_arr);

            // Clean exit
            if (ret != -1 && (token_count == 0 || (strncmp("exit", token_arr[0], 4) == 0))) {
                break;
            }

            // Command execution
        
            if (token_count >= 1) {
                if(token_count>highTok){
                    highTok = token_count;
                }
                bn_ptr builtin_fn = check_builtin(token_arr[0]);
                varHolder = checkEquals(token_arr[0]); 
                for (loopCount = 0; loopCount<token_count; loopCount++){ //loop to check for dollar signs, then replace tokens accoringly
                    if(checkDollars(token_arr[loopCount]) == 0){
                        strcpy(input, token_arr[loopCount]);
                        getVar(variables, numVars, input, token_arr[loopCount]);
                    }
                }
                char* combineBin;
                combineBin = malloc(MAX_STR_LEN);
                strcpy(combineBin, "/bin/");
                strcat(combineBin,token_arr[0]);
                DIR* checkDir = opendir(combineBin);
                free(combineBin);
                if (builtin_fn != NULL) { //builtin exists
                    if (backgroundFlag == 0){
                        ssize_t err = builtin_fn(token_arr, pipeCount, processees, processCount, pNames);
                        if (err == - 1) {
                            display_error("ERROR: Builtin failed: ", token_arr[0]);
                        }
                    }
                    else{
                        processCount++;
                        processees = realloc(processees, processCount*sizeof(int));
                        pNames = realloc(pNames,processCount*sizeof(char*) );
                        pNames[processCount-1] = malloc(MAX_STR_LEN*MAX_STR_LEN*sizeof(char));
                        strcpy(pNames[processCount-1], token_arr[0]);
                        for (ij=1; ij<token_count; ij++){
                            strcat(pNames[processCount-1]," ");
                            strcat(pNames[processCount-1],token_arr[ij]);
                        }
                        processees[processCount-1] = fork();
                        if(processees[processCount-1] == 0){
                            ssize_t err = builtin_fn(token_arr, pipeCount, processees, processCount, pNames);
                            if (err == - 1) {
                                display_error("ERROR: Builtin failed: ", token_arr[0]);
                            }
                            backFlag = 1;
                            break;
                        }
                        else{
                            strcpy(doneMessage,"[");
                            sprintf(numConv,"%d",processCount);
                            strcat(doneMessage,numConv);
                            strcat(doneMessage, "] ");
                            sprintf(numConv,"%d",processees[processCount-1]);
                            strcat(doneMessage,numConv);
                            strcat(doneMessage,"\n");
                            display_message(doneMessage);
                        }
                            
                    }
                }
                else if(varHolder == 0){ //variables storage
                    numVars++;
                    variables = realloc(variables, (numVars+1)*sizeof(char*));
                    variables[numVars] = malloc(MAX_STR_LEN*sizeof(char));
                    strcpy(variables[numVars-1], token_arr[0]);
                }
                else if(strcmp(token_arr[0],"start-server") == 0){
                    stopFlag = 0;
                    if(token_arr[1] == NULL){
                            display_error("ERROR: ","No port provided");
                            display_error("ERROR: Builtin failed: ", token_arr[0]);
                            stopFlag++;
                        }
                        if(stopFlag == 0){
                            int port;
                            port = atoi(token_arr[1]);
                            if(port == 0 && stopFlag == 0){
                                display_error("ERROR: ","Invalid port provided");
                                display_error("ERROR: Builtin failed: ", token_arr[0]);
                                stopFlag++;
                            }
                            if(stopFlag ==-1){
                                int net_socket;
                                net_socket = socket(AF_INET, SOCK_STREAM, 0);
                                if(net_socket == -1){
                                    display_error("ERROR: ","Invalid port provided");
                                    display_error("ERROR: Builtin failed: ", token_arr[0]);
                                    stopFlag++;
                                }
                                close(net_socket);
                                if(stopFlag == -1){
                                    int pid = 1;
                                    if(pid == 0){
                                        struct sockaddr_in server_address;
                                        server_address.sin_family = AF_INET;
                                        server_address.sin_port = htons(port);
                                        server_address.sin_addr.s_addr = INADDR_ANY;
                                        int is_connected = bind(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
                                        if(is_connected == -1){
                                            display_error("ERROR: ","Could not connect");
                                            display_error("ERROR: Builtin failed: ", token_arr[0]);
                                            stopFlag++;
                                        }
                                        while(1 && stopFlag == 0){
                                        //char server_resp[256] = "I am sending this";
                                        listen(net_socket, 5);

                                        accept(net_socket, NULL, NULL); //this is where hostname goes i think
                                        //send(client_socket, server_resp, sizeof(server_resp), 0);
                                        }
                                        close(net_socket);
                                        return 0;
                                    }
                                }
                            }
                        }
                }
                else if(strcmp(token_arr[0],"start-client") == 0){
                    if(token_arr[1] == NULL){
                        display_error("ERROR: ","No port provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                        stopFlag++;
                    }
                    else if(token_arr[2] == NULL){
                        display_error("ERROR: ","No hostname provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                        stopFlag++;
                    }
                    int port;
                    port = atoi(token_arr[1]);
                    if(port == 0){
                        display_error("ERROR: ","Invalid port provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                        stopFlag++;
                    }
                    int net_socket;
                    net_socket = socket(AF_INET, SOCK_STREAM, 0);
                    if(net_socket == -1){
                        display_error("ERROR: ","Invalid port provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                        stopFlag++;
                    }
                    struct sockaddr_in server_address;
                    server_address.sin_family = AF_INET;
                    server_address.sin_port = htons(port);
                    server_address.sin_addr.s_addr = INADDR_ANY;
                    int is_connected = connect(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
                    if(is_connected == -1){
                        display_error("ERROR: ","Could not connect");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                        stopFlag++;
                    }
                    //char server_resp[256];
                   // recv(net_socket, &server_resp, sizeof(server_resp),0);

                }
                else if(strcmp(token_arr[0],"send-message") == 0){
                    if(token_arr[1] == NULL){
                        display_error("ERROR: ","No port provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                    }
                    else if(token_arr[2] == NULL){
                        display_error("ERROR: ","No hostname provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                    }
                    else if(token_arr[3] == NULL){
                        display_error("ERROR: ","No message provided");
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                    }

                }
                else if (ENOENT != errno){ //system call
                    int t;
                    char* input;
                    input = malloc(MAX_STR_LEN*sizeof(char));
                    strcpy(input, token_arr[0]);
                    for (t=1; t<token_count; t++){
                        strcat(input, " ");
                        strcat(input,token_arr[t]);
                    }
                    if(backgroundFlag == 0){
                        fp = popen(input,"r");
                        char* output;
                        output = malloc(MAX_STR_LEN*MAX_STR_LEN*sizeof(char));
                        char* r = fgets(output,MAX_STR_LEN*MAX_STR_LEN,fp);
                        if (errno == 1){
                            display_error("ERROR: Unrecognized command: ", token_arr[0]);
                        }
                        else{
                            if (r!=NULL && output != NULL && strcmp(token_arr[0],"sleep") != 0){
                                display_message(output);
                            }
                        }
                        free(input);
                        free(output);
                        fclose(fp);
                    }
                    else{
                        processCount++;
                        processees = realloc(processees, processCount*sizeof(int));
                        pNames = realloc(pNames,processCount*sizeof(char*) );
                        pNames[processCount-1] = malloc(MAX_STR_LEN*MAX_STR_LEN*sizeof(char));
                        strcpy(pNames[processCount-1], token_arr[0]);
                        for (ij=1; ij<token_count; ij++){
                            strcat(pNames[processCount-1]," ");
                            strcat(pNames[processCount-1],token_arr[ij]);
                        }
                        processees[processCount-1] = fork();
                        if(processees[processCount-1] == 0){
                            fp = popen(input,"r");
                            char* output;
                            output = malloc(MAX_STR_LEN*MAX_STR_LEN*sizeof(char));
                            fgets(output,MAX_STR_LEN*MAX_STR_LEN,fp);
                            if (errno == 1){
                                display_error("ERROR: Unrecognized command: ", token_arr[0]);
                            }
                            else{
                                if (output != NULL && strcmp(token_arr[0],"sleep") != 0){
                                    display_message(output);
                                }
                            }
                            free(output);
                            free(input);
                            fclose(fp);
                            backFlag = 1;
                            break;
                        }
                        else{
                            strcpy(doneMessage,"[");
                            sprintf(numConv,"%d",processCount);
                            strcat(doneMessage,numConv);
                            strcat(doneMessage, "] ");
                            sprintf(numConv,"%d",processees[processCount-1]);
                            strcat(doneMessage,numConv);
                            strcat(doneMessage,"\n");
                            display_message(doneMessage);
                        }
                        free(input);
                    }
                }
                else {
                    combineBin = malloc(MAX_STR_LEN);
                    strcpy(combineBin, "/usr/bin/");
                    strcat(combineBin,token_arr[0]);
                    DIR* checkDir2 = opendir(combineBin);
                    free(combineBin);
                    if (ENOENT != errno){ //system call
                        errno = 0;
                        fp = popen(token_arr[0],"r");
                        char* output;
                        output = malloc(MAX_STR_LEN*MAX_STR_LEN*sizeof(char));
                        fgets(output,MAX_STR_LEN*MAX_STR_LEN,fp);
                        if (errno == 1){
                        display_error("ERROR: Unrecognized command: ", token_arr[0]);
                        }
                        //else if(errno != 0){
                           // display_error("ERROR: improper input for: ", token_arr[0]);
                        //}
                        else{
                            if (output != NULL){
                                display_message(output);
                            }
                        }
                        free(output);
                        fclose(fp);
                        closedir(checkDir2);
                    }
                    else{
                        display_error("ERROR: Unrecognized command: ", token_arr[0]);
                    }
                }
                closedir(checkDir);

                for (d=0; d<token_count;d++){
                    free(token_arr[d]);
                    token_arr[d] = NULL;
                }
                token_count = 0;
            }
        }
        else //PIPE VERSION
        {
            int outter;
            outter = dup(1);
            int inner;
            inner = dup(0);
            int fd[2];
            
            if (pipe(fd) < 0){
                display_error("PERROR","Pipe cannot be itialized");
            }
            if ((id1 = fork())<0){
                display_error("FERROR","Could not fork");
            }
            if (id1 == 0){ //child number 1
                char* dToken = "";
                close(fd[0]);
                dup2(fd[1],STDOUT_FILENO);
                close(fd[1]);
                strcpy(in_buff2, input_buf);
                dToken = strtok(in_buff2, "|");
                token_count = tokenize_input(dToken, token_arr);  
                free(in_buff2);
            }
            else{
                char* fToken;
                fToken = malloc(sizeof(char)*MAX_STR_LEN*MAX_STR_LEN);
                char* f2Token;
                //f2Token = malloc(sizeof(char)*MAX_STR_LEN);
                chekerFlag = 0;
                strcpy(fToken, input_buf);
                f2Token = strtok(fToken, " ");
                bn_ptr cheker = check_builtin(f2Token);
                if(cheker == NULL){
                    chekerFlag = 1;
                }
                free(fToken);
                //free(f2Token);
                if ((id2 = fork())<0){ 
                    display_error("FERROR","Could not fork");
                }
                if (id2 == 0){//child number 2
                    close(fd[1]);
                    dup2(fd[0],STDIN_FILENO);
                    close(fd[0]);
                    strcpy(in_buff2, input_buf);                    
                    char* dToken = "";
                    dToken = strtok(in_buff2, "|"); //these two lines select the second part (after |) will need to rework for n |'s
                    dToken = strtok(NULL, "|");
                    token_count = tokenize_input(dToken, token_arr);
                    free(in_buff2);
                    if (token_count==1){
                        if (chekerFlag == 0){
                            char* following = "";
                            following = malloc(MAX_STR_LEN*MAX_STR_LEN*sizeof(char));
                            get_input(following, 1);
                            token_arr[token_count] = malloc(sizeof(char)*MAX_STR_LEN*MAX_STR_LEN);
                            strcpy(token_arr[token_count],following);
                            token_count++;
                            free(following);
                        }   
                    }
                }
                else{ //parent waits then ends
                    wait(NULL);
                    wait(NULL);
                    dup2(outter,1);
                    dup2(inner,0);
                    writeFlag = 1;
                }
            }

            
            if (writeFlag == 0){
            // Clean exit
            if (ret != -1 && (token_count == 0 || (strncmp("exit", token_arr[0], 4) == 0))) {
                break;
            }

            // Command execution
        
            if (token_count >= 1) {
                if(token_count>highTok){
                    highTok = token_count;
                }
                bn_ptr builtin_fn = check_builtin(token_arr[0]);
                varHolder = checkEquals(token_arr[0]); 
                for (loopCount = 0; loopCount<token_count; loopCount++){ //loop to check for dollar signs, then replace tokens accoringly
                    if(checkDollars(token_arr[loopCount]) == 0){
                        strcpy(input, token_arr[loopCount]);
                        getVar(variables, numVars, input, token_arr[loopCount]);
                    }
                }
                if (builtin_fn != NULL) {
                    ssize_t err;
                    if (id2 == 0){
                        err = builtin_fn(token_arr, pipeCount, processees, processCount, pNames);
                    }
                    else{
                        err = builtin_fn(token_arr, -1, processees, processCount, pNames);
                    }
                    if (err == - 1) {
                        display_error("ERROR: Builtin failed: ", token_arr[0]);
                        display_message(" ");
                    }
                }
                else if(varHolder == 0){
                    numVars++;
                    variables = realloc(variables, (numVars+1)*sizeof(char*));
                    variables[numVars] = malloc(MAX_STR_LEN*sizeof(char));
                    strcpy(variables[numVars-1], token_arr[0]);
                }
                else {
                    display_error("ERROR: Unrecognized command: ", token_arr[0]);
                }
                
                for (d=0; d<token_count;d++){
                    free(token_arr[d]);
                   token_arr[d] = NULL;
                }
            }
            }
            close(fd[0]);
            close(fd[1]);
            writeFlag = 0;
        }
        }else{
            kidFlag = 1;
            break;
        }
    }
    int i;
    for (i=0;i<numVars+1;i++){
        free(variables[i]);
    }
    if(token_arr[0]!=NULL && kidFlag == 0){
        for(i=0;i<2;i++){
            free(token_arr[i]);
            token_arr[i] = NULL;
        }
    }
    free(makeSure);
    free(numConv);
    free(doneMessage);
    free(processees);
    free(variables);
    free(input);
    free(remember);
    if(id1!=0 && id2!=0){
        free(in_buff2);
    }
    if (kidFlag == 0){
        if(backFlag != -2){
            for (i=0; i<processCount; i++){
                free(pNames[i]);
            }
            free(pNames);
        }
        //free(in_buff2);
    }else{
        for (i=0; i<processCount; i++){
                free(pNames[i]);
            }
            free(pNames);
            
    }
    return 0;
}
