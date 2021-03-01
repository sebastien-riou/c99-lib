#include <stdio.h>

#define PRINT_PREFIX uart0_
static void uart0_print_impl(const char*msg){
  while(*msg){
    printf(" %c",*msg);
    msg++;
  }
}
#include "print.h"

#define PRINT_PREFIX uart1_
static void uart1_print_impl(const char*msg){
  while(*msg){
    printf(".%c",*msg);
    msg++;
  }
}
#include "print.h"


int main(void){
  uint32_t dat[] = {0,1,2,3};
  uart0_println("hello from uart0");
  uart1_println("hello from uart1");
  uart0_println_bytes_0x("bytes: ",dat,sizeof(dat));
  uart0_print_array32x_0xln(dat,sizeof(dat)/4);
  uart1_println_bytes_0x("bytes: ",dat,sizeof(dat));
  uart1_print_array32x_0xln(dat,sizeof(dat)/4);
}
