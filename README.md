# 🐚 SIMPLE SHELL

Um shell interativo para sistemas baseados em Unix, construído em C, com foco exclusivo na didática e na experiência de iniciantes no mundo Linux.

## 🎯 Propósito
O terminal Linux muitas vezes assusta novos usuários com mensagens de erro enigmáticas ou páginas de manuais (`man pages`) extremamente densas e longas. O objetivo deste projeto é desmistificar o uso do terminal. 

Ao invés de despejar 100 linhas de opções técnicas quando o usuário tem uma dúvida, este shell permite o uso do argumento especial `explica`. Ao digitar `comando explica` (ex: `ls explica`), o shell intercepta a chamada antes de enviá-la ao Sistema Operacional e exibe:
1. Uma explicação humana, simples e direta.
2. Um exemplo prático e aplicável de como usar o comando na vida real.

Este projeto foi pensado para criar um ambiente acolhedor, incentivando colegas e estudantes universitários a praticarem seus conhecimentos de sistemas operacionais e linha de comando sem medo de errar.

## ✨ Funcionalidades
- **Execução Padrão POSIX:** Gerenciamento real de processos usando chamadas de sistema nativas do C (`fork`, `execvp`, `waitpid`).
- **Interceptador Didático:** Bloqueia a execução de comandos seguidos da palavra `explica` e apresenta ajuda amigável.
- **Base de Dados Dinâmica:** As mensagens de ajuda não estão *hardcoded* (fixas no código). Elas são carregadas dinamicamente na memória a partir de um arquivo `command.json`.
- **Histórico e Prompt Interativo:** Implementado com a biblioteca `linenoise` para permitir a navegação no histórico com as setas do teclado, simulando a experiência real de terminais modernos.
- **Comandos Built-in:** Suporte nativo para navegação de diretórios (`cd`) e exibição do caminho atual (`pwd`).

## 🛠️ Como o arquivo JSON é estruturado
O shell carrega os ensinamentos a partir do arquivo `command.json` na raiz do projeto. Isso permite que qualquer pessoa ou professor contribua adicionando novos comandos de forma fácil, sem precisar tocar no código fonte em C:

```json
[
  {
    "comando": "ls",
    "descricao": "Lista os arquivos e pastas que estão no diretório atual.",
    "exemplo": "ls -l"
  },
  {
    "comando": "cat",
    "descricao": "Lê arquivos e os imprime na tela. Muito útil para ver textos rapidamente.",
    "exemplo": "cat arquivo.txt"
  }
]
```

## 🚀 Como Compilar e Rodar

### Pré-requisitos
- Compilador GCC.
- Sistema Operacional baseado em Unix (Linux, macOS, ou WSL).

### Compilação
Para compilar o shell junto com as dependências do parser de JSON e controle de linha, utilize:
```bash
gcc -o meyshell shell.c simpleShell.c lib/cJSON.c lib/linenoise.c -Wall
```
*(Nota: Ajuste os caminhos das bibliotecas de acordo com a sua estrutura de pastas)*

### Execução
Inicie o seu shell didático executando o binário gerado:
```bash
./meyshell
```

## 📚 Arquitetura do Código
- `shell.c`: Ponto de entrada (`main`), responsável por invocar o carregamento do JSON e iniciar o laço REPL (Read-Eval-Print Loop).
- `simpleShell.c` / `simpleShell.h`: Núcleo da ferramenta. Contém a lógica de execução, divisão das strings digitadas, execução de *built-ins* e o interceptador didático.
- **Bibliotecas externas utilizadas:**
  - [cJSON](https://github.com/DaveGamble/cJSON): Para realizar o *parsing* do banco de dados JSON de forma eficiente em C.
  - [linenoise](https://github.com/antirez/linenoise): Para uma substituição leve e moderna da tradicional *readline*, provendo o prompt e o histórico de comandos.
