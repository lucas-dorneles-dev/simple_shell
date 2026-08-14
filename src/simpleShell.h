#ifndef SIMPLESHELL_H
#define SIMPLESHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "./lib/cJSON.h"
#include "./lib/linenoise.h"

#define PROMPT "Estudante $ "
#define HISTORY_LENGTH 1024
#define MAX_ARGS 1024
#define TOKEN_SEP " \t"
#define PATH_MAX 4096
#define MAX_DIDACTIC_CMDS 100

typedef struct {
    char name[50];
    char desc[256];
    char ex[256];
} DidacticCmd;

typedef enum Builtin{
  CD,
  PWD,
  INVALID
} Builtin;

void history(void);
//
int stringRead(char* input_user, char** argumentos);
//
int stringExecute(char* cmd, char** cmdArg);
//
void builtin_impl_cd(char** args, size_t n_args);
//
void builtin_impl_pwd(char** args, size_t n_args);
//
void readJson(char* file_contents);
//
void loadJsonFile(const char* filename);

#endif
