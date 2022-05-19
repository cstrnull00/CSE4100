/* $begin echoserverimain */
#include "csapp.h"
#include "sbuf.h"

#define NTHREADS 4
#define SBUFSIZE 16
#define STOCK_NUM 10

typedef struct ITEM {
    int ID;
    int left_stock;
    int price;
    sem_t mutex;
} item;

item tree[STOCK_NUM];

void echo_cnt(int connfd);
void *thread(void *vargp);

sbuf_t sbuf; /* Shared buffer of connected descriptors */
 
int main(int argc, char **argv) 
{
    int i, listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; 

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }
    listenfd = Open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUFSIZE); //line:conc:pre:initsbuf
    for (i = 0; i < NTHREADS; i++)  /* Create worker threads */ //line:conc:pre:begincreate
	    Pthread_create(&tid, NULL, thread, NULL);               //line:conc:pre:endcreate

    while (1) { 
        clientlen = sizeof(struct sockaddr_storage);
	    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
	    sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
    }
}

void *thread(void *vargp) 
{  
    Pthread_detach(pthread_self()); 
    while (1) { 
	    int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
	    echo_cnt(connfd);                /* Service client */
	    Close(connfd);
    }
}
/* $end echoservertpremain */

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