#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
Answers can be either in Portuguese or in English.
Respostas podem sem tanto em português como em inglês.
*/

/*
1. Term of commitment

The group members declare that all code developed for this project is their own.
The group members declare that they have not copied material from the Internet
  nor obtained code from third parties.

2. Group members and allocation of effort

Fill in the lines below with the name and email of the group members.
Replace XX with the contribution of each group member in the development of the work.

Carla Beatriz Ferreira <carlabferreira@ufmg.br> 55%
Manuela Monteiro Fernandes de Oliveira <manuelamfo@ufmg.br> 45%


3. Solutions
Briefly describe the solutions implemented for this project and justify their choices.

- Correção da Função fork1:
Implementamos a função `fork1` utilizando a syscall `fork()` com verificação de erro.
A syscall 'fork()' já retorna em sua chamada o endereço que deve ser retornado na variavel pid ('Process ID')
Caso falhe, imprime uma mensagem e encerra o programa.

- Executando comandos simples:
Implementamos a função 'handle_simple_cmd' para ler e executar comandos simples.

Uma comparação é feita para verificar se existe algum comando a ser executado, caso ele seja inexixtente, o programa é encerrado.
Caso contrário, a função 'execvp' recebe o nome do comando e um vetor de argumentos e passa o controle do processo 
atual para o comando lido (que é essencialmente o que o comando exec faz)

- Redirecionamento de Entrada e Saída
Implementamos a função 'handle_rdirection' para lidar com o redirecionamento de entrada e saída.

O arquivo é aberto com a função 'open' e as permissõs para leitura e escrita por proprierário de arquivo, leitura para o grupo
e leitura para outros usuários é definida. Então, um descritor de arquivo duplicado é criado com a função dup2, para possibilitar o redirecionamento da entrada e saída.
Caso o redirecionamento seja de entrada, o descritor de arquivo é redirecionado para a entrada padrão (0), e se for de escrita, para a saída padrão(1).

- Sequenciamento de Comandos
TODO

- Correção da mensagem de erro
Solução apresentada no bloco correspondente (na main).
Em resumo: cd = "change directory" e não busca por processos.


4. Bibliographic references

- SILBERSCHATZ, Abraham; GALVIN, Peter Baer; GAGNE, Greg. Fundamentos de Sistemas Operacionais. 
    8. ed. LTC.

- TEODORO, George Luiz Medeiros. Slides virtuais da disciplina de Sistemas Operacionais. 
    Disponibilizado via Moodle. Departamento de Ciência da Computação, Universidade Federal de Minas Gerais, 
    Belo Horizonte.

- GNU. Bash Reference Manual. Disponível em: https://www.gnu.org/software/bash/manual/bash.html. 
    Acesso em: 15 abr. 2024.

- THOMPSON, Ken; RITCHIE, Dennis M. The UNIX Time-Sharing System. Disponível em: https://www.scs.stanford.edu/17wi-cs140/sched/readings/unix.pdf. 
    Acesso em: 15 abr. 2024.

- LINUX MAN PAGES. close(2) - Linux manual page. Disponível em: https://linux.die.net/man/2/close. 
    Acesso em: 15 abr. 2024.

- LINUX MAN PAGES. fork(2) - Linux manual page. Disponível em: https://linux.die.net/man/2/fork. 
    Acesso em: 15 abr. 2024.

- LINUX MAN PAGES. pipe(2) - Linux manual page. Disponível em: https://linux.die.net/man/2/pipe. 
    Acesso em: 15 abr. 2024.

- How to use the execvp() function in C/C++ | DigitalOcean. Disponível em: <https://www.digitalocean.com/community/tutorials/execvp-function-c-plus-plus>. Acesso em: 26 abr. 2025.

- GEEKSFORGEEKS. How to Redirect Output to a File and stdout. Disponível em: <https://www.geeksforgeeks.org/redirect-output-to-a-file-and-stdout/>. Acesso em: 26 abr. 2025.

- open(3): open file - Linux man page. Disponível em: <https://linux.die.net/man/3/open>. Acesso em: 26 abr. 2025.

‌- dup() and dup2() Linux system call. Disponível em: <https://www.geeksforgeeks.org/dup-dup2-linux-system-call/>. Acesso em: 26 abr. 2025.
? - vídeos do youtube
TODO
*/

/****************************************************************
 * Simplified xv6 Shell
 *
 * This code was adapted from the UNIX xv6 code and material from
 * the MIT Operating Systems course (6.828).
 ***************************************************************/

#define MAXARGS 10

/* Every command has a type. After identifying the command's type,
    the code converts a *cmd into the specific command type. */
struct cmd {
    int type; /* ' ' (exec)
                 '|' (pipe)
                 '<' or '>' (redirection) */
};

struct execcmd {
    int type;             // ' ' (exec)
    char *argv[MAXARGS];  // Arguments for the command to be executed
};

struct redircmd {
    int type;         // < or > (redirection)
    struct cmd *cmd;  // The command to execute (e.g., an execcmd)
    char *file;       // The input or output file
    int mode;         // The mode in which the file should be opened
    int fd;           // The file descriptor number to be used
};

struct pipecmd {
    int type;           // | (pipe)
    struct cmd *left;   // Left side of the pipe
    struct cmd *right;  // Right side of the pipe
};

int fork1(void);                                        // Fork but exit if an error occurs
struct cmd *parsecmd(char *);                           // Process the command line
void handle_simple_cmd(struct execcmd *ecmd);           // Handle simple commands
void handle_redirection(struct redircmd *rcmd);         // Handle redirection
void handle_pipe(struct pipecmd *pcmd, int *p, int r);  // Handle pipes

/* Execute the command cmd. It never returns. */
void runcmd(struct cmd *cmd) {
    int p[2], r;
    struct execcmd *ecmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0)
        exit(0);

    switch (cmd->type) {
        default:
            fprintf(stderr, "Unknown command type\n");
            exit(-1);

        case ' ': //! citado pelo enunciado
            ecmd = (struct execcmd *)cmd;
            if (ecmd->argv[0] == 0)
                exit(0);
            handle_simple_cmd(ecmd);
            break;

        case '>':
        case '<':
            rcmd = (struct redircmd *)cmd;
            handle_redirection(rcmd);
            runcmd(rcmd->cmd);
            break;

        case '|':
            pcmd = (struct pipecmd *)cmd;
            handle_pipe(pcmd, p, r);
            break;
    }
    exit(0);
}

int fork1(void) {
    /* Task 1: Implement the fork1 function.
    The function is supposed to create a new process using the `fork()` system call.
    It should print a message if the fork fails, otherwise return the process ID of the child process (or -1 if the fork fails).
    */
    pid_t pid;

    if ((pid = fork()) < 0) {
        fprintf(stderr, "Fork function fails\n");
        exit(-1);
    } 
    return (pid);

    /* END OF TASK 1 */
}

void handle_simple_cmd(struct execcmd *ecmd) {
     /* Task 2: Implement the code below to execute simple commands. */
    if (ecmd->argv[0] == 0) 
        exit(0);

    execvp(ecmd->argv[0], ecmd->argv);
    /* END OF TASK 2 */
}

void handle_redirection(struct redircmd *rcmd) {
     /* Task 3: Implement the code below to handle input/output redirection. */
    if (rcmd->cmd == 0)
        exit(0);
    
    int fd = open(rcmd->file, rcmd->mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // permissões de leitura e escrita para o dono do arquivo, leitura para o grupo e outros
    if(rcmd->type == '<'){
        if(fd < 0) // caso haja erro na abertura do arquivo
            exit(1);
         else
        {
            int d = dup2(fd, 0);
            close(fd);
            if (d < 0) // erro na execução de dup2
                exit(-1); 
        }
    }
    else if(rcmd->type == '>')
    {
        if(fd < 0) // caso haja erro na abertura do arquivo
            exit(1);
         else
        {
            int d = dup2(fd, 1);
            close(fd);
            if (d < 0) // erro na execução de dup2
                exit(-1); 
        }
    }
    else
        exit(1);

    /* END OF TASK 3 */
}

void handle_pipe(struct pipecmd *pcmd, int *p, int r) {
     /* Task 4: Implement the code below to handle pipes. */
    //todo conferir!!! tenho um rascunho
    /* */
    fprintf(stderr, "pipe not implemented\n");
    /* END OF TASK 4 */
}

int getcmd(char *buf, int nbuf) {
    if (isatty(fileno(stdin)))
        fprintf(stdout, "$ ");
    memset(buf, 0, nbuf);
    fgets(buf, nbuf, stdin);
    if (buf[0] == 0)  // EOF
        return -1;
    return 0;
}

int main(void) {
    static char buf[100];
    int r;

    // Read and execute commands.
    while (getcmd(buf, sizeof(buf)) >= 0) {
        /* Task 5: Explain the purpose of the if statement below and correct the error message.
        Why is the current error message incorrect? Justify the new message. */
        /* Answer:
            A mensagem "process does not exist" está incorreta porque a função `cd` não cria processos 
            mas sim muda o diretorio (cd = "change directory").
            O erro nesse contexto vem do fato do diretório não existir ou não poder ser acessado.
         */
        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
            buf[strlen(buf) - 1] = 0;
            if (chdir(buf + 3) < 0)
                fprintf(stderr, "Directory not found: %s\n", buf + 3);
            continue;
        }
        /* END OF TASK 5 */

        if (fork1() == 0)
            runcmd(parsecmd(buf));
        wait(&r);
    }
    exit(0);
}

/****************************************************************
 * Helper functions for creating command structures
 ***************************************************************/

struct cmd *
execcmd(void) {
    struct execcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = ' ';
    return (struct cmd *)cmd;
}

struct cmd *
redircmd(struct cmd *subcmd, char *file, int type) {
    struct redircmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = type;
    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->mode = (type == '<') ? O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC;
    cmd->fd = (type == '<') ? 0 : 1;
    return (struct cmd *)cmd;
}

struct cmd *
pipecmd(struct cmd *left, struct cmd *right) {
    struct pipecmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = '|';
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

/****************************************************************
 * Command Line Processing
 ***************************************************************/

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int gettoken(char **ps, char *es, char **q, char **eq) {
    char *s;
    int ret;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    if (q)
        *q = s;
    ret = *s;
    switch (*s) {
        case 0:
            break;
        case '|':
        case '<':
            s++;
            break;
        case '>':
            s++;
            break;
        default:
            ret = 'a';
            while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
                s++;
            break;
    }
    if (eq)
        *eq = s;

    while (s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return ret;
}

int peek(char **ps, char *es, char *toks) {
    char *s = *ps;
    while (s < es && strchr(whitespace, *s)) s++;
    *ps = s;
    return *s && strchr(toks, *s);
}

struct cmd *parseline(char **, char *);
struct cmd *parsepipe(char **, char *);
struct cmd *parseexec(char **, char *);

/* Copy characters from the input buffer, starting from s to es.
 * Place a null terminator at the end to create a valid string. */
char *mkcopy(char *s, char *es) {
    int n = es - s;
    char *c = malloc(n + 1);
    assert(c);
    strncpy(c, s, n);
    c[n] = 0;
    return c;
}

struct cmd *
parsecmd(char *s) {
    char *es;
    struct cmd *cmd;

    es = s + strlen(s);
    cmd = parseline(&s, es);
    peek(&s, es, "");
    if (s != es) {
        fprintf(stderr, "leftovers: %s\n", s);
        exit(-1);
    }
    return cmd;
}

struct cmd *
parseline(char **ps, char *es) {
    struct cmd *cmd;
    cmd = parsepipe(ps, es);
    return cmd;
}

struct cmd *
parsepipe(char **ps, char *es) {
    struct cmd *cmd;

    cmd = parseexec(ps, es);
    if (peek(ps, es, "|")) {
        gettoken(ps, es, 0, 0);
        cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
}

struct cmd *
parseredirs(struct cmd *cmd, char **ps, char *es) {
    int tok;
    char *q, *eq;

    while (peek(ps, es, "<>")) {
        tok = gettoken(ps, es, 0, 0);
        if (gettoken(ps, es, &q, &eq) != 'a') {
            fprintf(stderr, "missing file for redirection\n");
            exit(-1);
        }
        switch (tok) {
            case '<':
                cmd = redircmd(cmd, mkcopy(q, eq), '<');
                break;
            case '>':
                cmd = redircmd(cmd, mkcopy(q, eq), '>');
                break;
        }
    }
    return cmd;
}

struct cmd *
parseexec(char **ps, char *es) {
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    ret = execcmd();
    cmd = (struct execcmd *)ret;

    argc = 0;
    ret = parseredirs(ret, ps, es);
    while (!peek(ps, es, "|")) {
        if ((tok = gettoken(ps, es, &q, &eq)) == 0)
            break;
        if (tok != 'a') {
            fprintf(stderr, "syntax error\n");
            exit(-1);
        }
        cmd->argv[argc] = mkcopy(q, eq);
        argc++;
        if (argc >= MAXARGS) {
            fprintf(stderr, "too many args\n");
            exit(-1);
        }
        ret = parseredirs(ret, ps, es);
    }
    cmd->argv[argc] = 0;
    return ret;
}
