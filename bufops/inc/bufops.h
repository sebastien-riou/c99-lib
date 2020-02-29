#ifndef __BUFOPS_H__
#define __BUFOPS_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * shift a buffer left with byte granularity (little endian)
 *
 * @param   dst         destination buffer, can be equal to src
 * @param   src         source buffer
 * @param   byte_size   length in bytes of src/dst
 * @param   byte_shift  shift amount in bytes
 * @param   fill8       fill value for the LSBs
 *
 * byte_shift is allowed to be larger than byte_size, it behaves like they are equal
*/
static void bufops_shl(void*const dst,const void*const src,size_t byte_size, size_t byte_shift, uint8_t fill8){
    if(0==byte_size) return;
    if(byte_shift>byte_size) byte_shift=byte_size;
    uint8_t*const dst8=(uint8_t*)dst;
    const uint8_t*const src8=(const uint8_t*const)src;
    for(size_t i=byte_size-1;i!=byte_shift-1;i--){dst8[i] = src8[i-byte_shift];}
    for(size_t i=0;i<byte_shift;i++){dst8[i] = fill8;}
}
/**
 * shift a buffer left with bit granularity (little endian)
 *
 * @param   dst         destination buffer, can be equal to src
 * @param   src         source buffer
 * @param   size        length in bits of src/dst
 * @param   shift       shift amount in bits
 * @param   fill        fill value for the LSBs
 *
 * shift is allowed to be larger than size, it behaves like they are equal
*/
static void bufops_shlbits(void*const dst,const void*const src,size_t size, size_t shift, bool fill){
    if(0==size) return;
    const uint8_t fill8 = fill ? 0xFF : 0x00;
    if(shift>size) shift=size;
    uint8_t*const dst8=(uint8_t*)dst;
    const uint8_t*const src8=(const uint8_t*const)src;
    const size_t byte_size = (size+7)/8;
    const unsigned int byte_shift=shift%8;
    const unsigned int cshift = (8-byte_shift)%8;
    const uint8_t last = src8[byte_size-1];
    const size_t lsb = shift/8;
    if(0==(shift%8)){
        bufops_shl(dst,src,byte_size,lsb,fill8);
    } else {
        uint8_t carry=src8[byte_size-1-lsb];
        for(size_t i=byte_size-1;i!=lsb-1;i--){
            const size_t sidx = i-1-lsb;
            const uint8_t s = sidx>byte_size ?  fill8 : src8[sidx];
            const uint8_t d = (carry<<byte_shift)|(s >> cshift);
            carry = src8[sidx];
            dst8[i] = d;
        }
    }
    for(size_t i=0;i<lsb;i++){dst8[i]=fill8;}
    if(size%8){
        const uint8_t last_mask = 0xFF<<(size % 8);
        dst8[byte_size-1] &= ~last_mask;
        dst8[byte_size-1] |= last & last_mask;
    }
}
/**
 * shift a buffer right with byte granularity (little endian)
 *
 * @param   dst         destination buffer, can be equal to src
 * @param   src         source buffer
 * @param   byte_size   length in bytes of src/dst
 * @param   byte_shift  shift amount in bytes
 * @param   fill8       fill value for the MSBs
 *
 * byte_shift is allowed to be larger than byte_size, it behaves like they are equal
*/
static void bufops_shr(void*const dst,const void*const src,size_t byte_size, size_t byte_shift, uint8_t fill8){
    if(0==byte_size) return;
    if(byte_shift>byte_size) byte_shift=byte_size;
    uint8_t*const dst8=(uint8_t*)dst;
    const uint8_t*const src8=(const uint8_t*const)src;
    const size_t last=byte_size-byte_shift;
    for(size_t i=0;i!=last;i++){dst8[i] = src8[i+byte_shift];}
    for(size_t i=last;i<byte_shift;i++){dst8[i] = fill8;}
}
/**
 * shift a buffer right with bit granularity (little endian)
 *
 * @param   dst         destination buffer, can be equal to src
 * @param   src         source buffer
 * @param   size        length in bits of src/dst
 * @param   shift       shift amount in bits
 * @param   fill        fill value for the MSBs
 *
 * shift is allowed to be larger than size, it behaves like they are equal
*/
static void bufops_shrbits(void*const dst,const void*const src,size_t size, size_t shift, bool fill){
    if(0==size) return;
    const uint8_t fill8 = fill ? 0xFF : 0x00;
    if(shift>size) shift=size;
    uint8_t*const dst8=(uint8_t*)dst;
    const uint8_t*const src8=(const uint8_t*const)src;
    const size_t byte_size = (size+7)/8;
    const unsigned int bshift=shift%8;
    const unsigned int cshift = bshift ? (8-bshift)%8 : 8;
    const uint8_t last = src8[byte_size-1];
    const size_t lsb = shift/8;
    if((0==(shift%8)) && (0==(size%8))) {
        bufops_shr(dst,src,byte_size,lsb,fill8);
    } else {
        const uint8_t last_mask = size%8 ? 0xFF<<(size % 8) : 0;
        uint8_t carry = lsb+1 >=byte_size ? fill8 : src8[lsb+1];
        if(lsb+1 == byte_size-1) {
            carry &= ~last_mask;
            carry |= last_mask & fill8;
        }
        if(byte_size>lsb){
            for(size_t i=0;i<byte_size-lsb-1;i++){
                const size_t sidx = i+lsb;
                uint8_t s;
                if(sidx>=byte_size-1){
                    s=(src8[sidx] & ~last_mask) | (last_mask & fill8);
                }else{
                    s=src8[sidx];
                }
                const uint8_t d = (carry<<cshift)|(s >> bshift);
                carry = sidx+2 >=byte_size? fill8 : src8[sidx+2];
                if(sidx+2 == byte_size-1) {
                    carry &= ~last_mask;
                    carry |= last_mask & fill8;
                }
                dst8[i] = d;
            }
        }
        const size_t sidx = byte_size-lsb-1+lsb;
        carry &= ~last_mask;
        carry |= last_mask & fill8;
        uint8_t s;
        if(sidx>=byte_size-1){
            s=(src8[sidx] & ~last_mask) | (last_mask & fill8);
        }else{
            s=src8[sidx];
        }
        const uint8_t d = (carry<<cshift)|(s >> bshift);
        dst8[byte_size-lsb-1] = d;
    }
    for(size_t i=byte_size-lsb;i<byte_size;i++){dst8[i]=fill8;}
    if(size%8){
        const uint8_t last_mask = 0xFF<<(size % 8);
        dst8[byte_size-1] &= ~last_mask;
        dst8[byte_size-1] |= last & last_mask;
    }
}
#ifdef BUFOPS_INCLUDE_TESTS
#include <assert.h>
void bufops_bufshl_test(void){
    //WARNING: disable strict aliasing optimization (gcc: -fno-strict-aliasing)
    const uint64_t tv[] = {
        0x0000000000000000,
        0x0000000000000001,
        0x8000000000000000,
        0x8000000000000001,
        0x5555555555555555,
        0xAAAAAAAAAAAAAAAA,
        0xFFFFFFFFFFFFFFFF,
        0xFFFFFFFFFFFFFFFE,
        0x7FFFFFFFFFFFFFFF,
        0x7FFFFFFFFFFFFFFE,
        0x0123456789ABCDEF,
        0x257F4883E5289CD4,
        0x473E039541FD0E85,
        0x3D69139463520062,
        0xE3B4B05FB914863D,
    };
    for(unsigned int i=0;i<sizeof(tv)/sizeof(uint64_t);i++){
        const uint64_t src = tv[i];
        for(unsigned int size=0;size<64;size++){
            const uint64_t mask = 0xFFFFFFFFFFFFFFFF >> (64-size);
            const uint64_t imask = ~mask;
            for(unsigned int shift=0;shift<=(size+1);shift++){
                for(uint64_t fill=0;fill<2;fill++){
                    uint64_t result = src;
                    bufops_shlbits(&result,&result,size,shift,fill);
                    const unsigned int tb_shift = shift>size ? size : shift;
                    const uint64_t pad = fill ? (fill<<tb_shift)-1 : 0;
                    const uint64_t expected = ((src<<tb_shift) & mask) | (src & imask) | pad;
                    //printf("%2d %2d -> %16lx %16lx %16lx %16lx %16lx\n",size,shift,mask,imask,src,expected,result);
                    assert(expected==result);
                    if(size){
                        result=src & imask;
                        bufops_shlbits(&result,&src,size,shift,fill);
                        //printf("      -> %16lx %16lx %16lx %16lx %16lx\n",mask,imask,src,expected,result);
                        assert(expected==result);
                        result=(src & imask) | (0xFFFFFFFFFFFFFFFF & mask);
                        bufops_shlbits(&result,&src,size,shift,fill);
                        //printf("      -> %16lx %16lx %16lx %16lx %16lx\n",mask,imask,src,expected,result);
                        assert(expected==result);
                        result=(src & imask) | (~expected & mask);
                        bufops_shlbits(&result,&src,size,shift,fill);
                        //printf("      -> %16lx %16lx %16lx %16lx %16lx\n",mask,imask,src,expected,result);
                        assert(expected==result);
                    }
                }
            }
        }
    }
}
void bufops_bufshr_test(void){
    //WARNING: disable strict aliasing optimization (gcc: -fno-strict-aliasing)
    const uint64_t tv[] = {
        0x0000000000000000,
        0x0000000000000001,
        0x8000000000000000,
        0x8000000000000001,
        0x5555555555555555,
        0xAAAAAAAAAAAAAAAA,
        0xFFFFFFFFFFFFFFFF,
        0xFFFFFFFFFFFFFFFE,
        0x7FFFFFFFFFFFFFFF,
        0x7FFFFFFFFFFFFFFE,
        0x0123456789ABCDEF,
        0x257F4883E5289CD4,
        0x473E039541FD0E85,
        0x3D69139463520062,
        0xE3B4B05FB914863D,
    };
    for(unsigned int i=0;i<sizeof(tv)/sizeof(uint64_t);i++){
        const uint64_t src = tv[i];
        for(unsigned int size=0;size<64;size++){
            const uint64_t mask = 0xFFFFFFFFFFFFFFFF >> (64-size);
            const uint64_t imask = ~mask;
            for(unsigned int shift=0;shift<=(size+1);shift++){
                for(uint64_t fill=0;fill<2;fill++){
                    uint64_t result = src;
                    bufops_shrbits(&result,&result,size,shift,fill);
                    const unsigned int tb_shift = shift>size ? size : shift;
                    const uint64_t pad = (fill ? (1ull<<tb_shift)-1 : 0)<<(size-tb_shift);
                    const uint64_t expected = ((src& mask)>>tb_shift)  | (src & imask) | pad;
                    //printf("%2d %2d -> %16lx %16lx %16lx %16lx %16lx\n",size,shift,mask,imask,src,expected,result);
                    assert(expected==result);
                    if(size){
                        result=src & imask;
                        bufops_shrbits(&result,&src,size,shift,fill);
                        //printf("      -> %16lx %16lx %16lx %16lx %16lx\n",mask,imask,src,expected,result);
                        assert(expected==result);
                        result=(src & imask) | (0xFFFFFFFFFFFFFFFF & mask);
                        bufops_shrbits(&result,&src,size,shift,fill);
                        //printf("      -> %16lx %16lx %16lx %16lx %16lx\n",mask,imask,src,expected,result);
                        assert(expected==result);
                        result=(src & imask) | (~expected & mask);
                        bufops_shrbits(&result,&src,size,shift,fill);
                        //printf("      -> %16lx %16lx %16lx %16lx %16lx\n",mask,imask,src,expected,result);
                        assert(expected==result);
                    }
                }
            }
        }
    }
}
#endif

#endif
