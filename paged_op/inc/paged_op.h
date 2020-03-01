#ifndef __PAGED_OP_H__
#define __PAGED_OP_H__
//paged_op_t
//struct and function to compute parameters for "paged operation"
//we have to process a range from offset to offset+size
//
//we can access data with the following constraints:
//  - minimum size/alignement is granularity bytes
//  - maximum size/alignement is page_size bytes
//
//see paged_op32_test_case function in the test code for an example of usage
//
// WARNING: this is tested only with granularity being a power of 2

#include <stdint.h>

static void paged_op32_match_granularity_lo(uint32_t*size,uint32_t granularity){
    const uint32_t s_mod_gra = *size % granularity;
    const uint32_t out_size = s_mod_gra ? *size - s_mod_gra : *size;
    *size=out_size;
}

static void paged_op32_match_granularity_up(uint32_t*size,uint32_t granularity){
    const uint32_t s_mod_gra = *size % granularity;
    const uint32_t out_size = s_mod_gra ? *size + granularity - s_mod_gra : *size;
    *size=out_size;
}

static void paged_op32_match_granularity_range(uint32_t *poffset,uint32_t*size,uint32_t granularity){
    //printf("match_granularity_int 0x%x 0x%x 0x%x --> ",*poffset,*size,granularity);
    const uint32_t offset = *poffset;
    paged_op32_match_granularity_lo(poffset,granularity);
    *size =  *size + offset - *poffset;
    paged_op32_match_granularity_up(size,granularity);
    //printf("0x%x 0x%x\n",*poffset,*size);
}

typedef struct paged_op32_struct {
    uint32_t first_page;          //first page to access
    uint32_t first_offset;        //offset for access in first page
    uint32_t first_size;          //size for access in first page
    uint32_t first_buf_offset;    //offset of first valid data within the buffer returned by the access
    uint32_t first_buf_size;      //size of valid data within the buffer returned by the first access
    uint32_t last_page;           //last page to access if last_size not 0, otherwise it is the page after the last page to access
    uint32_t last_size;           //size of last page access if not equal to page_size
    uint32_t last_buf_size;       //size of last data in buffer if last_size!=0
} paged_op32_t;

#ifdef HAS_FPRINTF
static void paged_op32_dump(FILE*stream,const char *prefix,const paged_op32_t *op){
    fprintf(stream,"%s.first_page      =%u\n",prefix,op->first_page);
    fprintf(stream,"%s.first_offset    =%u\n",prefix,op->first_offset);
    fprintf(stream,"%s.first_size      =%u\n",prefix,op->first_size);
    fprintf(stream,"%s.first_buf_offset=%u\n",prefix,op->first_buf_offset);
    fprintf(stream,"%s.first_buf_size  =%u\n",prefix,op->first_buf_size);
    fprintf(stream,"%s.last_page       =%u\n",prefix,op->last_page);
    fprintf(stream,"%s.last_size       =%u\n",prefix,op->last_size);
    fprintf(stream,"%s.last_buf_size   =%u\n",prefix,op->last_buf_size);
}
#endif

static void paged_op32_compute(paged_op32_t *op, uint32_t offset, uint32_t size, uint32_t page_size, uint32_t granularity){
    #ifdef HAS_ASSERT
    assert(page_size>0);
    assert(granularity>0);
    assert(page_size>=granularity);
    #endif
    if(0==size){
        op->first_page=0;
        op->first_size=0;
        op->first_buf_size=0;
        op->last_page=0;
        op->last_size=0;
        op->last_buf_size=0;
        return;
    }
    uint32_t base_offset = offset;
    paged_op32_match_granularity_lo(&base_offset,page_size);
    op->first_page = base_offset / page_size;
    const uint32_t first_offset = offset-base_offset;
    op->first_offset = first_offset;
    if(size+op->first_offset > page_size) op->first_size = page_size-first_offset;
    else op->first_size = size;
    paged_op32_match_granularity_range(&(op->first_offset),&(op->first_size),granularity);
    op->first_buf_offset = first_offset - op->first_offset;
    if(size+op->first_offset+op->first_buf_offset > page_size) op->first_buf_size = page_size - first_offset;
    else op->first_buf_size = size;
    #ifdef HAS_ASSERT
    assert(op->first_buf_offset<op->first_size);
    assert(op->first_size>=op->first_buf_size);
    if(size==1) assert(op->first_size<=granularity);
    if(size<=granularity) assert(op->first_size<=2*granularity);
    #endif
    if(op->first_buf_size > size) op->first_buf_size = size;
    const uint32_t remaining = size - op->first_buf_size;
    const uint32_t n_loops = remaining / page_size;
    op->last_page = op->first_page + 1 + n_loops;
    op->last_size = remaining % page_size;
    paged_op32_match_granularity_up(&(op->last_size),granularity);
    op->last_buf_size = offset+size-(op->last_page * page_size);
}
#endif //__PAGED_OP_H__
