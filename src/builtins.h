#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include <unistd.h>


/* Type for builtin handling functions
 * Input: Array of tokens
 * Return: >=0 on success and -1 on error
 */
typedef ssize_t (*bn_ptr)(char **, int, int*, int, char **);
ssize_t bn_echo(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
ssize_t bn_ls(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
ssize_t bn_cd(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
ssize_t bn_cat(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
ssize_t bn_wc(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
ssize_t bn_kill(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
ssize_t bn_ps(char **tokens, int pipeFlag, int* proc, int pn, char** procNames);
//ssize_t bn_startServer(char **tokens, int pipeFlag, int* proc, int pn, char** procNames); 
//ssize_t bn_startClient(char **tokens, int pipeFlag, int* proc, int pn, char** procNames); 
int recursivePrint(char* curDir, int level, int gate, char* sub);
/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd);

/* BUILTINS and BUILTINS_FN are parallel arrays of length BUILTINS_COUNT
 */
static const char * const BUILTINS[] = {"echo","ls","cd","cat","wc","kill","ps"};
static const bn_ptr BUILTINS_FN[] = {bn_echo,bn_ls,bn_cd,bn_cat,bn_wc,bn_kill,bn_ps, NULL};    // Extra null element for 'non-builtin'
static const size_t BUILTINS_COUNT = sizeof(BUILTINS) / sizeof(char *);

#endif
