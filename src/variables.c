#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "variables.h"
#include "io_helpers.h"

int checkEquals(char* line){
    int length = strlen(line);
    int i;
    for (i=0; i<length; i++){
        if (line[i]=='='){
            return 0;
        }
    }
    return 1;
}
int checkDollars(char* lines){
    if (lines[0]=='$'){
        return 0;
    }
    else{
        return 1;
    } 
}


void getVar(char** vars, int amount, char* line, char* output){

    char* input= "";
    input = malloc(MAX_STR_LEN*sizeof(char));
    char* variClone = "";
    char* variClone2 = "";
    variClone2 = malloc(MAX_STR_LEN*sizeof(char));
    char* variChop = "";
    variClone = malloc(MAX_STR_LEN*sizeof(char));
    strcpy(input,line);
    char* keyword = "";
    keyword = malloc(MAX_STR_LEN*sizeof(char));
    strcpy(keyword, &input[1]);
    int i;
    for (i=0; i<amount;i++){
        strcpy(variClone, vars[i]);
        variChop = strtok(variClone, "=");
        
        if (strcmp(keyword,variChop) == 0){
            strcpy(output,"");
            strcpy(variClone2, vars[i]);
            strcpy(output,&variClone2[strlen(keyword)+1]);
            
        }

    }
    free(input);
    free(variClone);
    free(variClone2);
    free(keyword);
}
