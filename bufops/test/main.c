

#include <stdio.h>
#include <assert.h>

#define BUFOPS_INCLUDE_TESTS
#include "bufops.h"

#include <string.h>
#include <stdlib.h>

static void bigint2str(void *buf,const void*const src,int len){
    char*buf_string = (char*)buf;
    const unsigned char*const src8 = (const unsigned char*const)src;
    int buf_string_pos = 0;
    buf_string_pos += sprintf(buf_string, "0x");
    for (int i = len-1; i >=0; i--) {
        uint32_t b=src8[i];
        buf_string_pos += sprintf(buf_string + buf_string_pos, "%02x", b);
        //printf("*%s*%d\n",buf_string,buf_string_pos);
    }
}

int main(int argc, char *argv[]){
    printf("testing bufshl and bufshr functions\n");
    bufops_bufshl_test();printf("bufshl_test PASS\n");
    bufops_bufshr_test();printf("bufshr_test PASS\n");
    uint8_t test[] = {0x12,0x34,0x56,0x78};
    char buf[128];
    bigint2str(buf,test,1);printf("*%s*\n",buf);assert(0==strcmp(buf,"0x12"));
    bigint2str(buf,test,2);printf("*%s*\n",buf);assert(0==strcmp(buf,"0x3412"));
    bigint2str(buf,test,3);printf("*%s*\n",buf);assert(0==strcmp(buf,"0x563412"));
    bigint2str(buf,test,4);printf("*%s*\n",buf);assert(0==strcmp(buf,"0x78563412"));
    uint8_t test2[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00};
    bigint2str(buf,test2,(137+7)/8);printf("*%s*\n",buf);assert(0==strcmp(buf,"0x008000000000000000000000000000000000"));

    printf("TEST PASS\n");
}
