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
    //sem_t mutex;
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

//void echo(int connfd);

item tree[STOCK_NUM];

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    static pool pool;
    
    FILE* fp;
    fp = fopen("stock.txt", "r");
    for(i = 0; i < STOCK_NUM; i++)
        fscanf(fp, "%d %d %d", &tree[i].ID, &tree[i].left_stock, &tree[i].price);
    fclose(fp);

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);
    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }

        check_client(&pool);

        /*
	    clientlen = sizeof(struct sockaddr_storage); 
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
	    echo(connfd);
	    Close(connfd);
        */
    }

    exit(0);
}
/* $end echoserverimain */

void init_pool (int listenfd, pool *p) {
    int i;
    p->maxi = -1;
    for(i=0; i < FD_SETSIZE; i++) p->clientfd[i] = -1;

    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client (int connfd, pool *p) {
    int i;
    p->nready--;
    for(i = 0; i < FD_SETSIZE; i++) {
        if(p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);
            FD_SET(connfd, &p->read_set);
            if(connfd > p->maxfd) p->maxfd = connfd;
            if(i > p->maxi) p->maxi = i;
            break;
        }
    }
    if (i == FD_SETSIZE) app_error("add_client error: Too many clients");
}

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
                /*
                else if (strncmp(buf, "exit", 4) == 0) {
                    write(1, buf, strlen(buf));
                    strcpy(buf, "exit\0");
                }
                */
                
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