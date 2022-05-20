/* 
 * A thread-safe version of echo that counts the total number
 * of bytes received from clients.
 */
/* $begin trade */
#include "csapp.h"

static int byte_cnt;  /* Byte counter */
static sem_t mutex;   /* and the mutex that protects it */

static void init_trade(void)
{
    Sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void trade(int connfd) 
{
    int n; 
    char buf[MAXLINE]; 
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    int id, ea;
    char *token; 

    Pthread_once(&once, init_trade); //line:conc:pre:pthreadonce
    Rio_readinitb(&rio, connfd);        //line:conc:pre:rioinitb
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0 && strncmp(buf, "exit", 4) != 0) {
	    P(&mutex);
	    if(strncmp(buf, "show", 4) == 0) {
            write(1, buf, strlen(buf));
            strcpy(buf, "");
            
            for(int j = 0; j < STOCK_NUM; j++) 
                    sprintf(buf, "%s%d %d %d\n", buf, tree[j].ID, tree[j].left_stock, tree[j].price);
        }
        else if (strncmp(buf, "buy", 3) == 0) {
            write(1, buf, strlen(buf));
            token = strtok(buf, " ");
            token = strtok(NULL, " ");
            id = atoi(token);
            token = strtok(NULL, " ");
            ea = atoi(token);
            for(int j = 0; j < STOCK_NUM; j++)
                if(tree[j].ID == id) {
                    if(tree[j].left_stock >= ea) {
                        tree[j].left_stock -= ea;
                        strcpy(buf, "[buy] success\n");
                    }
                    else {
                        strcpy(buf, "Not enough left stock\n");
                    }
                }
        }
        else if (strncmp(buf, "sell", 4) == 0) {
            write(1, buf, strlen(buf));
            token = strtok(buf, " ");
            token = strtok(NULL, " ");
            id = atoi(token);
            token = strtok(NULL, " ");
            ea = atoi(token);
            for(int j = 0; j < STOCK_NUM; j++)
                if(tree[j].ID == id)
                    tree[j].left_stock += ea;
            strcpy(buf, "[sell] success\n");
        }
        else if (strncmp(buf, "exit", 4) == 0) {
            write(1, buf, strlen(buf));
            strcpy(buf, "exit\0");
            Close(connfd);
        }
                
        FILE* fp = fopen("stock.txt", "w");
        for(int i = 0; i < STOCK_NUM; i++)
            fprintf(fp, "%d %d %d\n", tree[i].ID, tree[i].left_stock, tree[i].price);
        fclose(fp);
	    V(&mutex);
	    Rio_writen(connfd, buf, MAXLINE);
    }
}
/* $end trade */