#include "mraa.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFSIZE 30
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
static mraa_gpio_context CS1;
static mraa_gpio_context SCK;
static mraa_gpio_context MOSI;
static mraa_gpio_context MISO;

static uint8_t read_data[BUFFSIZE];
static sem_t semaphore;
uint8_t sendbyte(uint8_t byte, int delay);
void spiinit();
int spifetch();
void *update(void *arg);

int main() {
 const struct sched_param priority = { 1 };
 sched_setscheduler(0, SCHED_FIFO, &priority);
 spiinit();
 uint8_t read;
 int cycle=0;
 
 MYFLOAT myfloat;
 MYINT myint;
 MYLONG mylong;

 sem_init(&semaphore, 0, 1);
 pthread_t pid;
 pthread_create(&pid, NULL, update, NULL); 
 while(1)
 {
    int j;
    sem_wait(&semaphore);

    printf(">>>>>>>>>Main thread print!>>>>>>>>>>\n");
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
    printf(">>>>>>>>>Main thread finish printing!>>>>>>>>>>\n");
    sem_post(&semaphore);
    printf("cycle %d\n", cycle++);
    sleep(5);

 }
 pthread_join(pid, NULL);
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

void spiinit()
{
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
}

int spifetch()
{
    int delay = 1000;
    int i;
    char c = 'a';
    uint8_t checksum = 0;
    int checksumTrue;
    mraa_gpio_write(CS1, 0);
    sendbyte(c, delay);
    usleep(100);
    uint8_t byte = 0;
    sendbyte(byte, delay);
    for(i=0; i<BUFFSIZE; i++)
    {   
        usleep(100);
        byte = 10;
        read_data[i] = sendbyte(byte, delay);
        checksum += read_data[i];
    }
    usleep(100);
    if ((checksum & 0xff) == sendbyte(byte, delay))
    {
        printf("checksum matches!\n");
        checksumTrue = 1;
    }
    else
    {
        printf("checksum ERROR!\n");
        checksumTrue = 0;
    }
    mraa_gpio_write(CS1, 1);
 
    return checksumTrue;
}

void *update(void *arg)
{
    while(1)
    {
        sem_wait(&semaphore);
        printf("=========enter update========\n");
        while(!spifetch())
        {
            usleep(500000);
        }
        printf("========finish update, release========\n");
        sem_post(&semaphore);
        sleep(3);
    }
}