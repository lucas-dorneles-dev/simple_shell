#include "simpleShell.h"

char CWD[PATH_MAX];

int stringRead(char* input, char** args){
  int i = 0;
  char* token = strtok(input, TOKEN_SEP);
  
  while(token != NULL && i < (MAX_ARGS -1)){
    args[i++] = token;
    token = strtok(NULL, " \t");
  }
  args[i] = NULL;
  return i;
}

int stringExecute(char* cmd, char** cmdArg){
  
  fprintf(stdout, "Executando: %s \n", cmd);
  
  int status;
  pid_t pid;

  pid = fork();
  
  if (pid < 0) {
    fprintf(stderr, "Comando não executado! \n");
    return -1;
  }

  if (pid == 0) {
    execvp(cmd, cmdArg);
  }
  else {
    // o pai precisa esperar o filho executar o comadno
    if (waitpid(pid, &status, 0) != pid) {
      fprintf(stderr, "não conseguiu esperar o filho");
      return -1;
    }
  }
  return status;
}

void (*BUILTIN_TABLE[]) (char **args, size_t n_args) = {
  [CD] = builtin_impl_cd,
  [PWD] = builtin_impl_pwd,
};

Builtin builtin_code(char *cmd) {
  if (!strncmp(cmd, "cd", 2)) {
    return CD;
  } else if (!strncmp(cmd, "pwd", 3)) {
    return PWD;
  } else {
    return INVALID;
  }
}

int is_builtin(char *cmd) {
  return builtin_code(cmd) != INVALID;
}

void s_execute_builtin(char *cmd, char **args, size_t n_args) {
  BUILTIN_TABLE[builtin_code(cmd)](args, n_args);
}

void refresh_cwd(void) {
  if (getcwd(CWD, sizeof(CWD)) == NULL) {
    fprintf(stderr, "Error: Could not read working dir");
    exit(1);
  }
}

void builtin_impl_cd(char **args, size_t n_args) {
  char *new_dir = *args;
  if (chdir(new_dir) != 0) {
    fprintf(stderr, "Error: Could not change directory");
    exit(1);
  }
  refresh_cwd();
}

void builtin_impl_pwd(char **args, size_t n_args) {
  fprintf(stdout, "%s\n", CWD);
}

void history(void){
  if(!linenoiseHistorySetMaxLen(HISTORY_LENGTH)){
    fprintf(stderr, "Não foi possivel configurar o historico");
    exit(1);
  }
  char *line;
  char *args[MAX_ARGS];
  
  while((line = linenoise(PROMPT)) != NULL){
 
    // realiza a leitura 
    int argsRead = stringRead(line, args);
 
    // printa o que foi lido
    fprintf(stdout, "Lido %d argumentos\n", argsRead);

    for (int i = 0; i < argsRead; i++) {
      fprintf(stdout,"args[%d] = %s\n", i, args[i]);
    }

    //Pula as linhas vazias
    if(argsRead == 0){
      linenoiseFree(line);
      continue;
    }

    // TODO: eval + print
    
    char* cmd = args[0];
    char** cmdArg = args;
    if (is_builtin(cmd)) {
      s_execute_builtin(cmd, (cmdArg+1), argsRead-1);
    }
    else {
      stringExecute(cmd, cmdArg);
    }

    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }
}
