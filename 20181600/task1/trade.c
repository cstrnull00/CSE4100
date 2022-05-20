/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

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

void trade(pool *p) 
{
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
                    for(int j = 1; j <= STOCK_NUM; j++) 
                        sprintf(buf, "%s%d %d %d\n", buf, tree[j].ID, tree[j].left_stock, tree[j].price);
                }
                else if (strncmp(buf, "buy", 3) == 0) {
                    write(1, buf, strlen(buf));
                    token = strtok(buf, " ");
                    token = strtok(NULL, " ");
                    id = atoi(token);
                    token = strtok(NULL, " ");
                    ea = atoi(token);
                    for(int j = 1; j <= STOCK_NUM; j++)
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
                    for(int j = 1; j <= STOCK_NUM; j++)
                        if(tree[j].ID == id)
                            tree[j].left_stock += ea;
                    strcpy(buf, "[sell] success\n");
                }
                
                FILE* fp = fopen("stock.txt", "w");
                for(int j = 1; j <= STOCK_NUM; j++) {
                    fprintf(fp, "%d %d %d\n", tree[j].ID, tree[j].left_stock, tree[j].price);
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
/* $end echo */

