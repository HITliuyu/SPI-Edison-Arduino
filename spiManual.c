#include "mraa.h"
#include <stdio.h>
#include <unistd.h>

#define BUFFSIZE 30
//|0.humidity|1.temperature|2.3.4.5.ratio0|6.7.8.9.ratio1|10.11.12.13.ratio2|14.15.CO2PPM|16.17.18.19.RangInCM|20.21.22.23.concentration|
//|24.25.light.visible|26.27.light.IR|28.29.light.UV*100|
typedef union cvtfloat {
    float val;
    unsigned char bytes[4];
} MYFLOAT;

typedef union cvtint {
    uint16_t val;
    unsigned char bytes[2];
} MYINT;

typedef union cvtlong {
    int val;
    unsigned char bytes[4];
} MYLONG;

uint8_t sendbyte(uint8_t byte, int delay);
mraa_gpio_context CS1;
mraa_gpio_context SCK;
mraa_gpio_context MOSI;
mraa_gpio_context MISO;

int main() {
 const struct sched_param priority = { 1 };
 sched_setscheduler(0, SCHED_FIFO, &priority);
 CS1 = mraa_gpio_init(10);
 mraa_gpio_use_mmaped(CS1, 1);
 mraa_gpio_dir(CS1, MRAA_GPIO_OUT);
 mraa_gpio_write(CS1, 1);
 SCK = mraa_gpio_init(13);
 mraa_gpio_use_mmaped(SCK, 1);
 mraa_gpio_dir(SCK, MRAA_GPIO_OUT);
 mraa_gpio_write(SCK, 0);
 MOSI = mraa_gpio_init(11);
 mraa_gpio_use_mmaped(MOSI, 1);
 mraa_gpio_dir(MOSI, MRAA_GPIO_OUT);
 mraa_gpio_write(MOSI, 0);
 MISO = mraa_gpio_init(12);
 mraa_gpio_use_mmaped(MISO, 1);
 mraa_gpio_dir(MISO, MRAA_GPIO_IN);
 mraa_gpio_read(MISO);
 int delay = 100;
 uint8_t read;
 int cycle=0;
 uint8_t read_data[BUFFSIZE];
 MYFLOAT myfloat;
 MYINT myint;
 MYLONG mylong;
 while(1)
 {
    int i, j;
    char c = 'a';
    mraa_gpio_write(CS1, 0);
    sendbyte(c, delay);
    usleep(100);
    uint8_t byte = 0;
    sendbyte(byte, delay);
    for(i=0; i<BUFFSIZE; i++)
    {   
        usleep(10);
        byte = 10;
        read_data[i] = sendbyte(byte, delay);
        
    }
    mraa_gpio_write(CS1, 1);
    printf("recv %d  %d\n", read_data[0], read_data[1]);
    for(j=0; j<4; j++)
    {
        myfloat.bytes[j] = read_data[j+2];
    }
    printf("ratio 0 is %f \n", myfloat.val);
    for(j=0; j<4; j++)
    {
        myfloat.bytes[j] = read_data[j+6];
    }
    printf("ratio 1 is %f \n", myfloat.val);
    for(j=0; j<4; j++)
    {
        myfloat.bytes[j] = read_data[j+10];
    }
    printf("ratio 2 is %f \n", myfloat.val);
    for(j=0; j<2; j++)
    {
        myint.bytes[j] = read_data[j+14];
    }
    printf("CO2PPM is %d \n", myint.val);
    for(j=0; j<4; j++)
    {
        mylong.bytes[j] = read_data[j+16];
    }

    printf("RangInCM is %d \n", mylong.val);
    for(j=0; j<4; j++)
    {
        myfloat.bytes[j] = read_data[j+20];
    }
    printf("PM concentration is %f \n", myfloat.val);
    for(j=0; j<2; j++)
    {
        myint.bytes[j] = read_data[j+24];
    }
    printf("visiable light is %d \n", myint.val);
    for(j=0; j<2; j++)
    {
        myint.bytes[j] = read_data[j+26];
    }
    printf("IR light is %d \n", myint.val);
    for(j=0; j<2; j++)
    {
        myint.bytes[j] = read_data[j+28];
    }
    printf("UV light is %f \n", (float)myint.val/100);

    printf("cycle %d\n", cycle++);
    sleep(3);

 }
 return MRAA_SUCCESS;
}

uint8_t sendbyte(uint8_t byte, int delay) {
 int i, j;
 int read = 0;
//  mraa_gpio_write(CS1, 0);
 for (j = 1; j < delay / 8 + 100; j++) {
 };
 for (i = 0; i < 8; i++){
  mraa_gpio_write(MOSI, byte & 0x80);
  byte = byte << 1;
  for (j = 1; j < delay; j++) {
  };
  mraa_gpio_write(SCK, 1);
  read = read << 1;
  read = read | (mraa_gpio_read(MISO));
  for (j = 1; j < delay; j++) {
  };
  mraa_gpio_write(SCK, 0);
 }
//  mraa_gpio_write(CS1, 1);
 for (j = 1; j < delay / 8 + 20; j++) { };
 return (uint8_t) read;
}
