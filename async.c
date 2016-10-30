#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hiredis.h"
#include "async.h"
#include "adapters/ae.h"

int reconnect(const redisAsyncContext *ac, const char *err);

#define LOG(fmt, args...) printf(""fmt"\n", ##args)
/* Put event loop in the global scope, so it can be explicitly stopped */
static aeEventLoop *loop;

int reply(redisReply *_r)
{   
    redisReply *r = (redisReply *)_r;
    switch(r->type){
    case REDIS_REPLY_STRING  ://1
        LOG("[STRING] %s", r->str);
        if(memcmp(r->str, "quit", 4) == 0){
            return -1;
        }
        break;
    case REDIS_REPLY_ARRAY   ://2
        {
            int i;
            int ret = 0;
            for(i = 0; i < r->elements; ++i){
                if(reply(r->element[i]) == -1){
                    ret = - 1;
                }

            }
            return ret;
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
    return 0;
}


void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    if (r == NULL) {
        if(c->err){
            reconnect(c, c->errstr);
        }
        return;
    }
    if(reply(r) == -1){
        redisAsyncDisconnect(c);
        aeStop(loop);
    }
    /* Disconnect after receiving the reply to GET */
    //redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        reconnect(c, c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {

    if (status != REDIS_OK) {
        LOG("Disconnected fail");
        return;
    }
    printf("Disconnected...\n");
}


int reconnect(const redisAsyncContext *_ac, const char *err)
{ 
    redisAsyncContext * ac;

    if(_ac){
        //aeDeleteFileEvent(loop, _ac->c.fd,  AE_ALL_EVENTS);
        sleep(1);
    }
    ac = redisAsyncConnect("127.0.0.1", 6379);
    if(ac->err){
        LOG("reconnect fail Error:%s", ac->errstr);
        redisAsyncFree(ac);
        return -1;
    }
    LOG("RECONNECT... fd=%d,error=%s", ac->c.fd, err?err:"");
    redisAeAttach(loop, ac);
    redisAsyncSetConnectCallback(ac,connectCallback);
    redisAsyncSetDisconnectCallback(ac,disconnectCallback);
    redisAsyncCommand(ac, getCallback, NULL, "SUBSCRIBE channel");
    return 0;
}

int main (int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    loop = aeCreateEventLoop(10);
    reconnect(NULL, NULL);
    loop->stop = 0;
    while(!loop->stop){
        aeProcessEvents(loop, AE_ALL_EVENTS);
    }
    aeDeleteEventLoop(loop);
    LOG("********QUIT*********");
    return 0;
}

