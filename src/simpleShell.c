#include "simpleShell.h"
// variaveis globais
char CWD[PATH_MAX];
DidacticCmd didactic_cmds[MAX_DIDACTIC_CMDS];
int num_didactic_cmds = 0;

int stringRead(char *input, char **args)
{
  int i = 0;
  char *token = strtok(input, TOKEN_SEP);

  while (token != NULL && i < (MAX_ARGS - 1))
  {
    args[i++] = token;
    token = strtok(NULL, " \t");
  }
  args[i] = NULL;
  return i;
}

int stringExecute(char *cmd, char **cmdArg)
{

  fprintf(stdout, "Executando: %s \n", cmd);

  int status;
  pid_t pid;

  pid = fork();

  if (pid < 0)
  {
    fprintf(stderr, "Comando não executado! \n");
    return -1;
  }

  if (pid == 0)
  {
    execvp(cmd, cmdArg);
  }
  else
  {
    // o pai precisa esperar o filho executar o comadno
    if (waitpid(pid, &status, 0) != pid)
    {
      fprintf(stderr, "não conseguiu esperar o filho");
      return -1;
    }
  }
  return status;
}

void (*BUILTIN_TABLE[])(char **args, size_t n_args) = {
    [CD] = builtin_impl_cd,
    [PWD] = builtin_impl_pwd,
};

Builtin builtin_code(char *cmd)
{
  if (!strncmp(cmd, "cd", 2))
  {
    return CD;
  }
  else if (!strncmp(cmd, "pwd", 3))
  {
    return PWD;
  }
  else
  {
    return INVALID;
  }
}

int is_builtin(char *cmd)
{
  return builtin_code(cmd) != INVALID;
}

void s_execute_builtin(char *cmd, char **args, size_t n_args)
{
  BUILTIN_TABLE[builtin_code(cmd)](args, n_args);
}

void refresh_cwd(void)
{
  if (getcwd(CWD, sizeof(CWD)) == NULL)
  {
    fprintf(stderr, "Error: Could not read working dir");
    exit(1);
  }
}

void builtin_impl_cd(char **args, size_t n_args)
{
  char *new_dir = *args;
  if (chdir(new_dir) != 0)
  {
    fprintf(stderr, "Error: Could not change directory");
    exit(1);
  }
  refresh_cwd();
}

void builtin_impl_pwd(char **args, size_t n_args)
{
  fprintf(stdout, "%s\n", CWD);
}

void readJson(char *file_contents)
{
  cJSON *root = cJSON_Parse(file_contents);
  if (root == NULL)
  {
    fprintf(stderr, "Erro ao fazer o parse do JSON.\n");
    return;
  }

  cJSON *cmd;
  // Itera sobre o array JSON e popula o vetor de structs
  cJSON_ArrayForEach(cmd, root)
  {
    if (num_didactic_cmds >= MAX_DIDACTIC_CMDS)
      break;

    const char *name = cJSON_GetObjectItem(cmd, "comando")->valuestring;
    const char *desc = cJSON_GetObjectItem(cmd, "descricao")->valuestring;
    const char *ex = cJSON_GetObjectItem(cmd, "exemplo")->valuestring;

    strncpy(didactic_cmds[num_didactic_cmds].name, name, 49);
    strncpy(didactic_cmds[num_didactic_cmds].desc, desc, 255);
    strncpy(didactic_cmds[num_didactic_cmds].ex, ex, 255);

    num_didactic_cmds++;
  }

  cJSON_Delete(root); // Libera a memória alocada pelo cJSON
}

void loadJsonFile(const char *filename)
{
  // 1. Abre o arquivo JSON recebido por parâmetro
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo %s\n", filename);
    return;
  }

  // 2. Descobre o tamanho do arquivo para alocar a memória correta
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  // 3. Aloca a memória e lê o conteúdo do arquivo para uma string
  char *json_data = malloc(length + 1);
  if (json_data)
  {
    fread(json_data, 1, length, file);
    json_data[length] = '\0'; // Adiciona o terminador de string
  }
  fclose(file);

  // 4. Passa o conteúdo lido para a sua função analisar
  if (json_data)
  {
    readJson(json_data);
    free(json_data); // Libera a memória após o cJSON fazer o parse
  }
}

void history(void)
{
  if (!linenoiseHistorySetMaxLen(HISTORY_LENGTH))
  {
    fprintf(stderr, "Não foi possivel configurar o historico");
    exit(1);
  }
  char *line;
  char *args[MAX_ARGS];

  while ((line = linenoise(PROMPT)) != NULL)
  {

    // realiza a leitura
    int argsRead = stringRead(line, args);

    // printa o que foi lido
    fprintf(stdout, "Lido %d argumentos\n", argsRead);

    for (int i = 0; i < argsRead; i++)
    {
      fprintf(stdout, "args[%d] = %s\n", i, args[i]);
    }

    // Pula as linhas vazias
    if (argsRead == 0)
    {
      linenoiseFree(line);
      continue;
    }

    // TODO: eval + print

    char *cmd = args[0];

    if (argsRead >= 2 && strcmp(args[1], "explica") == 0)
    {
      int found_didactic = 0;

      for (int i = 0; i < num_didactic_cmds; i++)
      {
        if (strcmp(didactic_cmds[i].name, cmd) == 0)
        {
          fprintf(stdout, "\n--- [ COMANDO: %s ] ---\n", didactic_cmds[i].name);
          fprintf(stdout, "O que faz: %s\n", didactic_cmds[i].desc);
          fprintf(stdout, "Exemplo prático: %s\n", didactic_cmds[i].ex);
          fprintf(stdout, "--------------------------------\n\n");

          found_didactic = 1;
          break;
        }
      }

      if (found_didactic)
      {
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
        continue;
      }
    }

    char **cmdArg = args;
    if (is_builtin(cmd))
    {
      s_execute_builtin(cmd, (cmdArg + 1), argsRead - 1);
    }
    else
    {
      stringExecute(cmd, cmdArg);
    }

    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }
}
