/* 
 * A thread-safe version of echo that counts the total number
 * of bytes received from clients.
 */
/* $begin trade */
#include "csapp.h"

void trade(int connfd) 
{
    int i, n;
    char buf[MAXLINE]; 
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    int id, ea;
    char *token; 

    //Pthread_once(&once, init_trade); //line:conc:pre:pthreadonce
    Rio_readinitb(&rio, connfd);        //line:conc:pre:rioinitb
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0 && strncmp(buf, "exit", 4) != 0) {
	    if(strncmp(buf, "show", 4) == 0) {
            write(1, buf, strlen(buf));
            strcpy(buf, "");
            
            for(i = 1; i <= STOCK_NUM; i++) {
                P(&tree[i].mutex);
                tree[i].readcnt++;
                if(tree[i].readcnt == 1)
                    P(&tree[i].w);
                V(&tree[i].mutex);
                
                sprintf(buf, "%s%d %d %d\n", buf, tree[i].ID, tree[i].left_stock, tree[i].price);
                
                P(&tree[i].mutex);
                tree[i].readcnt--;
                if(tree[i].readcnt == 0)
                    V(&tree[i].w);
                V(&tree[i].mutex);

            }
        }
        else if (strncmp(buf, "buy", 3) == 0) {
            write(1, buf, strlen(buf));
            token = strtok(buf, " ");
            token = strtok(NULL, " ");
            id = atoi(token);
            token = strtok(NULL, " ");
            ea = atoi(token);
            for(i = 1; i <= STOCK_NUM; i++)
                if(tree[i].ID == id) {
                    P(&tree[i].w);
                    if(tree[i].left_stock >= ea) {
                        tree[i].left_stock -= ea;
                        strcpy(buf, "[buy] success\n");
                    }
                    else {
                        strcpy(buf, "Not enough left stock\n");
                    }
                    V(&tree[i].w);
                }
        }
        else if (strncmp(buf, "sell", 4) == 0) {
            write(1, buf, strlen(buf));
            token = strtok(buf, " ");
            token = strtok(NULL, " ");
            id = atoi(token);
            token = strtok(NULL, " ");
            ea = atoi(token);
            for(i = 1; i <= STOCK_NUM; i++)
                if(tree[i].ID == id) {
                    P(&tree[i].w);
                    tree[i].left_stock += ea;
                    V(&tree[i].w);
                }
            strcpy(buf, "[sell] success\n");
        }
        else if (strncmp(buf, "exit", 4) == 0) {
            write(1, buf, strlen(buf));
            strcpy(buf, "exit\0");
            Close(connfd);
        }
        
        FILE* fp = fopen("stock.txt", "w");
        for(i = 1; i <= STOCK_NUM; i++) {
            P(&tree[i].mutex);
            tree[i].readcnt++;
            if(tree[i].readcnt == 1)
                P(&tree[i].w);
            V(&tree[i].mutex);
            
            fprintf(fp, "%d %d %d\n", tree[i].ID, tree[i].left_stock, tree[i].price);

            P(&tree[i].mutex);
            tree[i].readcnt--;
            if(tree[i].readcnt == 0)
                V(&tree[i].w);
            V(&tree[i].mutex);
        }
        fclose(fp);

	    Rio_writen(connfd, buf, MAXLINE);
    }
}
/* $end trade */