/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

#define FD_SETSIZE 100

void init_pool (int listenfd, pool *p);
void add_client (int connfd, pool *p);
void trade(pool *p);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    static pool pool;
    
    FILE* fp;
    fp = fopen("stock.txt", "r");
    for(int i = 1; i <= STOCK_NUM; i++)
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
        trade(&pool);
    }

    exit(0);
}
/* $end echoserverimain */
