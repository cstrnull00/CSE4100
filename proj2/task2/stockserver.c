/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

#define FD_SETSIZE 100
#define STOCK_NUM 10

typedef struct ITEM {
    int ID;
    int left_stock;
    int price;
    sem_t mutex;
} item;

typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
} pool;

void echo(int connfd);

void *thread(void *vargp) {
    int connfd = *((int*)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    echo(connfd);
    Close(connfd);
    return NULL;
}

item tree[STOCK_NUM];

int main(int argc, char **argv) 
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    pthread_t tid;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    listenfd = Open_listenfd(argv[1]);
    
    while (1) {
	    clientlen = sizeof(struct sockaddr_storage); 
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    exit(0);
}
/* $end echoserverimain */

/*
void check_client (pool *p) {
    int i, connfd, n;
    char buf[MAXLINE];
    rio_t rio;

    int id, ea;
    char *token; 

    for(i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        if((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0 && strncmp(buf, "exit", 4) != 0) {
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
                }
                
                FILE* fp = fopen("stock.txt", "w");
                for(int i = 0; i < STOCK_NUM; i++) {
                    fprintf(fp, "%d %d %d\n", tree[i].ID, tree[i].left_stock, tree[i].price);
                }
                fclose(fp);

	            Rio_writen(connfd, buf, MAXLINE);
            }
            else {
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }

    }
}
*/