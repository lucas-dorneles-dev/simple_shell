#include "simpleShell.h"
// variaveis globais
char CWD[PATH_MAX];
DidacticCmd didacticCMD[MAX_DIDACTIC_CMDS];
int numDidacticCMD = 0;

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
void stringToExecuteBuiltin(char *cmd, char **args, size_t nArgs)
{
  // 1-> acha o ponteiro certo          2-> chama esse ponteiro,
  //   (busca na tabela)                entregando os argumentos
  BUILTIN_TABLE[builtinCode(cmd)](args, nArgs);
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
    perror("Erro: Não foi possível ler o diretório de trabalho");
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
void builtinImplementPWD(char **args, size_t nArgs)
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
    if (numDidacticCMD >= MAX_DIDACTIC_CMDS)
      break;

    cJSON *nameItem = cJSON_GetObjectItem(cmd, "comando");
    cJSON *descItem = cJSON_GetObjectItem(cmd, "descricao");
    cJSON *exItem = cJSON_GetObjectItem(cmd, "exemplo");

    if (!cJSON_IsString(nameItem) || !cJSON_IsString(descItem) || !cJSON_IsString(exItem))
    {
      fprintf(stderr, "Entrada inválida no JSON (posição %d), pulando...\n", numDidacticCMD);
      continue;
    }

    strncpy(didacticCMD[numDidacticCMD].name, nameItem->valuestring, 49);
    didacticCMD[numDidacticCMD].name[49] = '\0';

    strncpy(didacticCMD[numDidacticCMD].desc, descItem->valuestring, 255);
    didacticCMD[numDidacticCMD].desc[255] = '\0';

    strncpy(didacticCMD[numDidacticCMD].ex, exItem->valuestring, 255);
    didacticCMD[numDidacticCMD].ex[255] = '\0';

    numDidacticCMD++;
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
  // se ele não consegue nem carregar o historico, ele aborta
  if (!linenoiseHistorySetMaxLen(HISTORY_LENGTH))
  {
    fprintf(stderr, "Não foi possivel configurar o historico\n");
    exit(1);
  }
  char *line;
  char *args[MAX_ARGS];

  // o prompt é o nome do usuário (L do REPL)
  while ((line = linenoise(PROMPT)) != NULL)
  {
    // check de linha vazia
    if (line[0] == '\0')
    { // usuário só apertou Enter
      linenoiseFree(line);
      continue;
    }
    linenoiseHistoryAdd(line);

    // realiza a leitura
    int argsRead = stringRead(line, args);

// printa o que foi lido
#ifdef SHELL_DEBUG
    fprintf(stdout, "Lido %d argumentos\n", argsRead);

    for (int i = 0; i < argsRead; i++)
    {
      fprintf(stdout, "args[%d] = %s\n", i, args[i]);
    }
#endif
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
      int foundDidactic = 0;

      for (int i = 0; i < numDidacticCMD; i++)
      {
        if (strcmp(didacticCMD[i].name, cmd) == 0)
        {
          fprintf(stdout, "\n--- [ COMANDO: %s ] ---\n", didacticCMD[i].name);
          fprintf(stdout, "O que faz: %s\n", didacticCMD[i].desc);
          fprintf(stdout, "Exemplo prático: %s\n", didacticCMD[i].ex);
          fprintf(stdout, "--------------------------------\n\n");

          foundDidactic = 1;
          break;
        }
      }

      if (foundDidactic)
      {
        linenoiseFree(line);
        continue;
      }
      // se ainda n tiver explicação
      fprintf(stdout, "Ainda não tenho explicação para '%s'.\n", cmd);
      linenoiseFree(line);
      continue;
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

    linenoiseFree(line);
  }
}