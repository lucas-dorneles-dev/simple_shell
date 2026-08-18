#include "simpleShell.h"
// variaveis globais
char CWD[PATH_MAX];
DidacticCmd didactic_cmds[MAX_DIDACTIC_CMDS];
int num_didactic_cmds = 0;

// Le a linha de comando e tokeniza eles
int stringRead(char *input, char **args)
{
  int tokenReads = 0;
  // aponta para o primeiro char do token (cmd)
  char *token = strtok(input, TOKEN_SEP);
  // para quando não encontra mais token e reserva o ultimo para nulificar
  while (token != NULL && tokenReads < (MAX_ARGS - 1))
  {
    // o bloco guarda o token no arry
    args[tokenReads++] = token;
    token = strtok(NULL, TOKEN_SEP);
  }
  // nulifica a ultima possição
  args[tokenReads] = NULL;
  // retorno a quantidade de argumentos lidos
  return tokenReads;
}

int stringExecute(char *cmd, char **cmdArg)
{
  // imprime o nome do comando a ser executado
  fprintf(stdout, "Executando: %s \n", cmd);

  int status;
  pid_t pid; // id do processo

  pid = fork(); // duplica o processo em pai -> filho

  if (pid < 0)
  {
    // se o pid retornar um valor < 0 é porque deu problema no fork()
    fprintf(stderr, "Comando não executado! \n");
    return -1;
  }

  if (pid == 0)
  {
    // o filho procura o executavel do comando e executa ele
    execvp(cmd, cmdArg);
    // só chega aqui se execvp falhou
    if (errno == ENOENT)
    {
      fprintf(stderr, "%s: comando não encontrado\n", cmd);
    }
    else
    {
      perror(cmd);
    }
    exit(EXIT_FAILURE);
  }
  else
  {
    // o pai precisa esperar o filho executar o comando
    // se o pai não conseguiu esperar retorna o -1 e printa o erro
    if (waitpid(pid, &status, 0) != pid)
    {
      fprintf(stderr, "não conseguiu esperar o filho");
      return -1;
    }
  }
  // retorna o status do programa executado com as informações empacotadas pelo próprio kernel
  return status;
}

// variável global
// array onde cada posição guarda um ponteiro pra função
void (*BUILTIN_TABLE[])(char **args, size_t numArgs) = {
    // posição [CD] aponta para a função builtinImplementCd
    [CD] = builtinImplementCd,
    // posição [PWD] aponta para a função builtinImplementPWD
    [PWD] = builtinImplementPWD,
};

// Identifica se cmd corresponde a um builtin (cd, pwd, etc)
// e retorna o valor do enum correspondente (INVALID se não for nenhum)
Builtin builtinCode(char *cmd)
{
  if (!strcmp(cmd, "cd"))
  {
    return CD;
  }
  else if (!strcmp(cmd, "pwd"))
  {
    return PWD;
  }
  else
  {
    return INVALID;
  }
}

// valida se o builtin é valido
int isBuiltin(char *cmd)
{
  return builtinCode(cmd) != INVALID;
}

// executa o builtin
void stringToExecuteBuiltin(char *cmd, char **args, size_t n_args)
{
  // 1-> acha o ponteiro certo          2-> chama esse ponteiro,
  //   (busca na tabela)                entregando os argumentos
  BUILTIN_TABLE[builtinCode(cmd)](args, n_args);
}

// atualizar o diretório de trabalho atual
// CWD = Current Working Directory
void refreshCWD(void)
{
  // pega o caminho absoluto do dir atual
  // se da certo retorna o buffer(dir atual)
  // se falhar retorna null e o erro
  if (getcwd(CWD, sizeof(CWD)) == NULL)
  {
    perror("Erro: Não foi possível ler o diretório de trabalho\n");
    exit(1);
  }
}

// implementação do comando cd
void builtinImplementCd(char **args, size_t nArgs)
{
  if (nArgs == 0)
  {
    fprintf(stderr, "Erro: faltando argumento no cd\nExemplo: cd Documentos/simple_shell\n");
    return;
  }
  // lembrando que o args[0] aqui ta valendo o args [1]
  // ele é o primeiro depois do cmd
  char *newDir = args[0];
  if (chdir(newDir) != 0)
  {
    perror("cd");
    return;
  }
  refreshCWD();
}

// imprime o caminho absoluto do diretório atual
void builtinImplementPWD(char **args, size_t n_args)
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
    if (isBuiltin(cmd))
    {
      stringToExecuteBuiltin(cmd, (cmdArg + 1), argsRead - 1);
    }
    else
    {
      stringExecute(cmd, cmdArg);
    }

    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }
}