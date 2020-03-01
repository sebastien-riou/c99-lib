

#include <stdio.h>
#include <assert.h>

#define HAS_ASSERT
#define HAS_FPRINTF
#include "paged_op.h"



#include <string.h>
#include <stdlib.h>

#include "printx.h"

#ifndef MEM_WORD_SIZE
#define MEM_WORD_SIZE       8
#endif
#ifndef MEM_WORD_PER_PAGE
#define MEM_WORD_PER_PAGE   4
#endif

#define MEM_PAGE_SIZE       (MEM_WORD_PER_PAGE*MEM_WORD_SIZE)
#define MEM_PAGES           11
#define MEM_SIZE            (MEM_PAGES*MEM_PAGE_SIZE)

#if MEM_WORD_SIZE == 1
uint8_t mem[MEM_PAGES][MEM_WORD_PER_PAGE];
#endif
#if MEM_WORD_SIZE == 2
uint16_t mem[MEM_PAGES][MEM_WORD_PER_PAGE];
#endif
#if MEM_WORD_SIZE == 4
uint32_t mem[MEM_PAGES][MEM_WORD_PER_PAGE];
#endif
#if MEM_WORD_SIZE == 8
uint64_t mem[MEM_PAGES][MEM_WORD_PER_PAGE];
#endif

void access (uint8_t*buf, uint32_t offset, uint32_t size){
    //printf("\taccess offset=%u, size=%u\n",offset,size);
    if(0==size) return;
    assert(0==(offset % MEM_WORD_SIZE));//granularity check
    assert(0==(size   % MEM_WORD_SIZE));//granularity check
    const uint32_t page = offset / MEM_PAGE_SIZE;
    //printf("\t\tpage=%u, (offset+size-1)/MEM_PAGE_SIZE=%u\n",page,(offset+size-1)/MEM_PAGE_SIZE);
    assert(page==((offset+size-1)/MEM_PAGE_SIZE));//page_size check
    const uint32_t page_offset = (offset % MEM_PAGE_SIZE) / MEM_WORD_SIZE;
    const uint32_t words = size/MEM_WORD_SIZE;
    //printf("\t");
    for(uint32_t i=0;i<words;i++){
        uint64_t word = mem[page][page_offset+i];
        //printf("%016lx ",word);
        memcpy(buf+i*MEM_WORD_SIZE,&word,MEM_WORD_SIZE);
    }
    //printf("\n");
}

uint64_t checksum;
void process(uint8_t*buf, uint32_t offset, uint32_t size){
    //printf("\tprocess offset=%u, size=%u\n",offset,size);
    //printf("\t");
    for(uint32_t i=0;i<size;i++){
        //printf("%02x ",buf[offset+i]);
        checksum+=buf[offset+i];
    }
    //printf("\n");
}

//we model the processing by a call to:     process(uint8_t*buf, uint32_t offset, uint32_t size)
//we model the access to data by a call to: access (uint8_t*buf, uint32_t offset, uint32_t size)
//
static void paged_op32_test_case(uint32_t offset,uint32_t size){
    //printf("test_case %u, %u\n",offset,size);
    const uint32_t page_size = MEM_PAGE_SIZE;
    const uint32_t granularity = MEM_WORD_SIZE;
    paged_op32_t op;
    paged_op32_compute(&op, offset, size, page_size, granularity);
    //paged_op32_dump(stdout,"\t",&op);
    uint32_t acc_offset = op.first_offset;
    uint32_t acc_size   = op.first_size;
    uint32_t buf_offset = op.first_buf_offset;
    uint32_t buf_size   = op.first_buf_size;
    uint8_t buf[page_size];
    for(uint32_t page=op.first_page;page<op.last_page;page++){//do first and all full page accesses
        access(buf,page*page_size+acc_offset,acc_size);
        process(buf,buf_offset,buf_size);//note process is optional and can be done before access for write operation
        acc_offset = 0;
        acc_size   = page_size;
        buf_offset = 0;
        buf_size   = page_size;
    }
    if(op.last_size){//do last page access if not a full page
        access(buf,op.last_page*page_size,op.last_size);
        process(buf,0,op.last_buf_size);//note process is optional and can be done before access for write operation
    }
}

#define NUM_ELEMS(a) (sizeof(a)/sizeof 0[a])
int main(int argc, char *argv[]){
    printf("MEM_WORD_SIZE=%3u and MEM_WORD_PER_PAGE=%3u: ",MEM_WORD_SIZE,MEM_WORD_PER_PAGE);
    fflush(stdout);
    uint8_t mem_ref[MEM_SIZE];
    srand(0);
    for(int i=0;i<MEM_SIZE;i++) mem_ref[i] = rand();
    memcpy(mem,mem_ref,MEM_SIZE);
    for(uint32_t offset = 0; offset < MEM_SIZE; offset++){
        for(uint32_t size = 0; size < MEM_SIZE-offset +1; size++){
            //printf("--- offset=%u, size=%u\n",offset,size);
            checksum=0;
            process(mem_ref,offset,size);
            const uint64_t checksum_ref=checksum;
            checksum=0;
            paged_op32_test_case(offset,size);
            if(checksum_ref!=checksum){
                printf("\n--- offset=%u, size=%u\n",offset,size);
                printx32_dump_buf64("mem\n",mem,sizeof(mem),0);
                assert(0);
            }
        }
    }
    //printx_dump_buf64("mem\n",mem,sizeof(mem));
    printf("TEST PASS\n");
}
