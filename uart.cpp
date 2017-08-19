#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "horizonTracker.h"

#define START_BYTE 0x12

const int DOUBLE_SIZE = sizeof(double);

int uart0filestream = -1;

void UARTInit();
int writeStartByte();
int writeAngleData(double&);
int readAngleData(double&);
int readStartByte();
void reverse_array(unsigned char*, int);
void shiftLeft(unsigned char*, int, int);



unsigned char rx_buffer[256] = { 0 };
int offset = 0;

void UARTInit()
{
  uart0filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
  if(uart0filestream == -1)
  {
    printf("Cant open UART\n");
  }
  struct termios options;
  tcgetattr(uart0filestream, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart0filestream, TCIFLUSH);
  tcsetattr(uart0filestream, TCSANOW, &options);
}

int writeStartByte()
{
  if(mode == SERF)
  {
    printf("I'm not writing the start byte. I'm the SERF!");
  }

  char const data[] = { START_BYTE };
  if(uart0filestream)
  {
    int count = write(uart0filestream, &data, 1);
    if(count < 0)
    {
      printf("UART TX error\n");
      return -1;
    }
  }
}

int writeAngleData(double & angle) {
  if(mode == MASTER)
  {
    printf("I'm not writing angle data. I'm the MASTER!\n");
    return -1;
  }

  unsigned char data[DOUBLE_SIZE+1] = { 0 };
  memcpy((&data+1), &angle, DOUBLE_SIZE);
  //unsigned char * const doubleData = reinterpret_cast<unsigned char * const>(&angle);
  if(uart0filestream)
  {
    int count = write(uart0filestream, &data, DOUBLE_SIZE+1);
    if(count < 0)
    {
      printf("UART TX error\n");
      return -1;
    }
  }
}

int readAngleData(double & angle)
{
  if(mode == SERF)
  {
    printf("I'm not reading angle data. I'm the SERF!\n");
    return -1;
  }
  if(uart0filestream != -1)
  {
    int rx_length = read(uart0filestream, (void*)(rx_buffer+offset), 255-offset);
    offset = rx_length;
    if(rx_length <= 0)
    {
      // no data
      return 0;
    }
    else if(rx_length >= DOUBLE_SIZE)
    {

      //consume bytes;
      while(rx_buffer[0] != 0) {
        shiftLeft(rx_buffer, 256, 1);
      }


      //reverse_array(rx_buffer, DOUBLE_SIZE);
      memcpy(&angle, (&rx_buffer+1), DOUBLE_SIZE);

      printf("%f: %x %x %x %x %x %x %x %x\n", angle, rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6], rx_buffer[7], rx_buffer[8])
      // pop off 9 bytes
      shiftLeft(rx_buffer, 256, 9);
      return 0;
    }
    else
    {
      printf("Some uart error occurred. We received data, but not enough to make a double!!!\n");
      return 0;
    }
  }
}

int readStartByte()
{
  if(mode == MASTER)
  {
    printf("I'm not reading start byte data. I'm the MASTER!\n");
    return -1;
  }
  if(uart0filestream != -1)
  {
    unsigned char rx_buffer[256];
    int rx_length = read(uart0filestream, (void*)rx_buffer, 255);
    if(rx_length <= 0)
    {
      // no data
      return 0;
    }
    else
    {
      if(rx_buffer[0] == START_BYTE)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
}

void shiftLeft(unsigned char * bytes, int length, int shift)
{
  for(int j = 0; j < length-shift; j++) {
    bytes[j]=bytes[j+shift];
  }
  for(int j = length-shift; j < length; j++) {
    bytes[j]=bytes[0];
  }
}


void reverse_array( unsigned char array[], int arraylength )
{
    for (int i = 0; i < (arraylength / 2); i++)
    {
        unsigned char temporary = array[i];
        array[i] = array[(arraylength - 1) - i];
        array[(arraylength - 1) - i] = temporary;
    }
}
