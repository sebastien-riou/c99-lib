
#ifndef __PRINTX_H__
#define __PRINTX_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef printx_printf
#define printx_printf printf
#endif

//replace 0 by .
static void printx_diff_byte(uint8_t d, const char *sep){
    unsigned int n=d>>4;
    if(0==n) printx_printf("."); else printx_printf("%X",n);
    n = d & 0xF;
    if(0==n) printx_printf("."); else printx_printf("%X",n);
    printx_printf("%s",sep);
}
static void printx_diff_bytes_sep(const char *msg,const void *vbuf, unsigned int size, const char *m2, const char *sep){
    const uint8_t*const buf = (const uint8_t*const)vbuf;
    printx_printf("%s",msg);
    if(size){
        unsigned int i;
        for(i=0;i<size-1;i++) printx_diff_byte(buf[i],sep);
        printx_diff_byte(buf[i],"");
    }
    printx_printf("%s", m2);
}

static void printx_bytes_sep(const char *msg,const void *vbuf, unsigned int size, const char *m2, const char *sep){
    const uint8_t*const buf = (const uint8_t*const)vbuf;
    printx_printf("%s",msg);
    if(size){
        unsigned int i;
        for(i=0;i<size-1;i++) printx_printf("%02X%s",buf[i],sep);
        printx_printf("%02X",buf[i]);
    }
    printx_printf("%s", m2);
}
static void printx_bytes(const char *m,const void *buf, unsigned int size, const char *m2){printx_bytes_sep(m,buf,size,m2," ");}
static void printxln_bytes(const char *m,const void *buf, unsigned int size){printx_bytes(m,buf,size,"\n");}
static void printx_128(const char *m, const uint8_t a[16], const char *m2){
	printx_bytes_sep( m,a   ,4,"_","");
	printx_bytes_sep("",a+4 ,4,"_","");
	printx_bytes_sep("",a+8 ,4,"_","");
	printx_bytes_sep("",a+12,4,m2 ,"");
}
static void printxln_128(const char m[], const uint8_t a[16]){printx_128(m,a,"\n");}

static int printx_hexdigit_value(char c){
	int nibble = -1;
	if(('0'<=c) && (c<='9')) nibble = c-'0';
	if(('a'<=c) && (c<='f')) nibble = c-'a' + 10;
	if(('A'<=c) && (c<='F')) nibble = c-'A' + 10;
	return nibble;
}

static int printx_is_hexdigit(char c){
	return -1!=printx_hexdigit_value(c);
}

static size_t printx_hexstr_to_bytes(uint8_t *dst, size_t dst_size, const char *const hexstr){
	unsigned int len = strlen(hexstr);
	if(dst_size>(len/2))
		dst_size = (len/2);
	memset(dst,0,dst_size);
	for(unsigned int i=0;i<dst_size*2;i++){
		unsigned int shift = 4 - 4*(i & 1);
		unsigned int charIndex = i;//len-1-i;
		char c = hexstr[charIndex];
		uint8_t nibble = printx_hexdigit_value(c);
		dst[i/2] |= nibble << shift;
	}
	return dst_size;
}

static void printx_bytes_to_hexstr(char *dst,uint8_t *bytes, unsigned int nBytes){
	unsigned int i;
	for(i=0;i<nBytes;i++){
		sprintf(dst+2*i,"%02X",bytes[i]);
	}
}
static size_t printx_cleanup_hexstr(char *hexstr, size_t hexstr_size, char *str, size_t str_size){
	size_t cnt=0;
	int lastIs0=0;
	for(unsigned int j = 0;j<str_size;j++){
		char c = str[j];
		if(printx_is_hexdigit(c)){
			if(cnt==hexstr_size-1){//need final char for null.
				printf("Too many hex digits. hexstr=%s\n",hexstr);
				hexstr[cnt]=0;
				return -1;
			}
			hexstr[cnt++]=c;
		} else if(lastIs0) {
			if('x'==c) cnt--;
			if('X'==c) cnt--;
		}
		lastIs0 = '0'==c;
	}
	hexstr[cnt]=0;
	return cnt;
}
static size_t printx_user_hexstr_to_bytes(uint8_t*out, size_t out_size, char *str, size_t str_size){
	size_t hexstr_size = printx_cleanup_hexstr(str,str_size,str,str_size);
	size_t conv_size = (hexstr_size/2) < out_size ? hexstr_size/2 : out_size;
	return printx_hexstr_to_bytes(out,conv_size,str);
}

void printx_dump_buf(const char *msg,const void*const buf,size_t size,int bytes_per_word,int words_per_line){
    const int bytes_per_line=bytes_per_word*words_per_line;
    const uint8_t*pos=(const uint8_t*)buf;
    printx_printf("%s",msg);
    for(int i=0;i<size/bytes_per_line;i++){
        printx_printf("%08x: ",(uint32_t)(uintptr_t)(pos-(const uint8_t*const)buf));
        for(int j=0;j<words_per_line;j++) {
            printx_bytes_sep("",pos,bytes_per_word," ","");
            pos+=bytes_per_word;
        }
        printx_printf("\n");
    }
    const int remaining = size%bytes_per_line;
    if(remaining){
        printx_printf("%08x: ",(uint32_t)(uintptr_t)(pos-(const uint8_t*const)buf));
        for(int j=0;j<remaining/bytes_per_word;j++) {
            printx_bytes_sep("",pos,bytes_per_word," ","");
            pos+=bytes_per_word;
        }
        if(remaining%bytes_per_word){
            printx_bytes_sep("",pos,remaining%bytes_per_word," ","");
        }
        printx_printf("\n");
    }
}
void printx_dump_buf32(const char *msg,const void*const buf,size_t size){
    const int bytes_per_word=4;
    const int words_per_line=64/bytes_per_word;
    printx_dump_buf(msg,buf,size,bytes_per_word,words_per_line);
}
void printx_dump_buf64(const char *msg,const void*const buf,size_t size){
    const int bytes_per_word=8;
    const int words_per_line=64/bytes_per_word;
    printx_dump_buf(msg,buf,size,bytes_per_word,words_per_line);
}

static void printx_remove_unused_warnings(void){
    (void)printxln_bytes;
    (void)printxln_128;
    (void)printx_bytes_to_hexstr;
    (void)printx_user_hexstr_to_bytes;
    (void)printx_diff_bytes_sep;
}

#endif
