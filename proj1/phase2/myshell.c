/* $begin shellmain */
#include "myshell.h"
#include<errno.h>

#define MAXARGS   128
#define READ 0
#define WRITE 1

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int *idx);
int builtin_command(char **argv);
void pipe_command(char *argv[][MAXARGS], int cnt, int fin);

char PATH[MAXARGS] = "/bin/"; /* Default command path : /bin/ */

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
	/* Read */
	printf("CSE4100:P1-myshell> ");
	fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    }
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    int idx[MAXARGS] = {0, }; /* Pipe index */
    int cnt = 0; /* Number of pipe */
    char *cmd[MAXARGS][MAXARGS];

    int status;
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv, idx); 

    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */

    int i, j = 0, k;

    /* if there is pipe, make a pipe */
    while(idx[cnt++] > 0);
    if(--cnt) {
        for(i = 0; i <= cnt; i++) {
            k = 0;
            while(argv[j] != NULL)
                cmd[i][k++] = argv[j++];
            j++;
        }

        if((pid = fork()) == 0)
            pipe_command(cmd, 0, STDIN_FILENO);
        if(waitpid(pid, &status, 0) < 0)
            unix_error("waitfg: waitpid error");
    }
    else {
        if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
            if((pid = Fork()) == 0) { //child process
                if (execve(strcat(PATH, argv[0]), argv, environ) < 0) {	//ex) /bin/ls ls -al &
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }
            }

	        /* Parent waits for foreground job to terminate */
	        if (!bg){ 
                if (waitpid(pid, &status, 0) < 0)
                    unix_error("waitfg: waitpid error");
	            }
	        else//when there is background process!
	            printf("%d %s", pid, cmdline);
        }
    }
    return;
}

void pipe_command(char *argv[][MAXARGS], int cnt, int fin) {
    int fd[2];
    pid_t pid;

    if(argv[cnt + 1][0]) {
        if(pipe(fd) < 0) {
            printf("pipe error!\n");
            exit(1);
        }
        if((pid = fork()) == 0) { //child process
            close(fd[0]);
            if(fin != STDIN_FILENO) {
                dup2(fin, STDIN_FILENO);
                close(fin);
            }
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);

            if(!builtin_command(argv[cnt])) {
                if (execve(strcat(PATH, argv[cnt][0]), argv[cnt], environ) < 0) {
                    printf("%s: Command not found.\n", argv[cnt][0]);
                    exit(0);
                }
            }
            exit(0);
        }
        close(fd[1]);
        pipe_command(argv, cnt + 1, fd[0]);
        int status;
        if(waitpid(pid, &status, 0) < 0)
            unix_error("waitfg: waitpid error\n");
    }
    else {
        dup2(fin, STDIN_FILENO);
        close(fin);
        if(!builtin_command(argv[cnt])) {
            if (execve(strcat(PATH, argv[cnt][0]), argv[cnt], environ) < 0) {
                printf("%s: Command not found.\n", argv[cnt][0]);
                exit(0);
            }
        }
        exit(0);
    }
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit")) /* quit command */
	    exit(0);
    if(!strcmp(argv[0], "cd")) {
        if(argv[1] == NULL && (argv[1] = (char*)getenv("HOME"))==NULL)
            argv[1] = ".";
        if(chdir(argv[1]) < 0)
            fprintf(stderr, "%s: No such file or directory\n", argv[1]);
        return 1;
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	    return 1;
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv, int *idx) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    int cnt = 0; /* Number of pipe */
    int i;
    char *quote;

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    /* Parsing '|' character */
    int blank_cnt = 0;
    for(i = 1; i < strlen(buf); i++) {
        if(buf[i] == '|') {
            idx[cnt++] = blank_cnt + 1;
            buf[i] = ' ';
        }
        else if(buf[i - 1] == ' ' && buf[i] != ' ') ++blank_cnt;
    }

    for(i = 0; i < cnt; i++) idx[i] += i;
    /* Build the argv list */
    argc = 0; i = 0;
    while (delim = strchr(buf, ' ')) {
        if(*buf == '\"' || *buf == '\'') {
            quote = buf++;
            quote = strchr(buf, *quote);
            delim = quote;
        }
        argv[argc++] = buf;
	    *delim = '\0';
	    buf = delim + 1;
	    while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
        if(argc == idx[i]) { argv[argc++] = NULL; i++; }
    }
    argv[argc] = NULL;

    if (argc == 0)  /* Ignore blank line */
	    return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;

    return bg;
}
/* $end parseline */