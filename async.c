#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hiredis.h"
#include "async.h"
#include "adapters/ae.h"

#define LOG(fmt, args...) printf(""fmt"\n", ##args)
/* Put event loop in the global scope, so it can be explicitly stopped */
static aeEventLoop *loop;

void reply(redisReply *_r)
{   
    redisReply *r = (redisReply *)_r;
    switch(r->type){
    case REDIS_REPLY_STRING  ://1
        if(memcmp(r->str, "quit", 4) == 0){
            aeStop(loop);
        }
        LOG("[STRING] %s", r->str);
        break;
    case REDIS_REPLY_ARRAY   ://2
        {
            int i;
            for(i = 0; i < r->elements; ++i){
                reply(r->element[i]);
            }
        }
        break;
    case REDIS_REPLY_INTEGER ://3
        LOG("[INT] %lld", r->integer);
        break;
    case REDIS_REPLY_NIL     ://4
        LOG("[NIL] ");
        break;
    case REDIS_REPLY_STATUS  ://5
        LOG("[STATUS] %lld", r->integer);
        break;
    case REDIS_REPLY_ERROR   ://6
        LOG("[ERROR] %s", r->str);
        break;
    default:
        LOG("[UNKNOW] %d", r->type);
        break;
    }
}


void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    if (r == NULL) {
        if(c->err){
            aeStop(loop);
        }
        LOG("read IS NULL");
        return;
    }
    reply(r);
    /* Disconnect after receiving the reply to GET */
    //redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        aeStop(loop);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {

    if (status != REDIS_OK) {
        return;
    }
    printf("Disconnected...\n");
}


void channel()
{
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);

    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return ;
    }
    loop = aeCreateEventLoop(10);
    redisAeAttach(loop, c);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);
    redisAsyncCommand(c, getCallback, NULL, "SUBSCRIBE channel");

    loop->stop = 0;
    while(!loop->stop){
        aeProcessEvents(loop, AE_ALL_EVENTS);
    }
    aeDeleteEventLoop(loop);
    redisAsyncFree(c); //FIX ME
}

int main (int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    do{
        channel();
//        sleep(1);
    }while(1);

    return 0;
}

