#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "horizonTracker.h"

#define START_BYTE 0x12

const int DOUBLE_SIZE = sizeof(double);

int uart0filestream = -1;

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
  unsigned char * const data = reinterpret_cast<unsigned char * const>(&angle);
  if(uart0filestream)
  {
    int count = write(uart0filestream, &data, DOUBLE_SIZE);
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
    unsigned char rx_buffer[256];
    int rx_length = read(uart0filestream, (void*)rx_buffer, 255);
    if(rx_length < 0)
    {
      printf("Some uart error occurred. Why are there LESS than 0 bytes in the buffer!!!\n");
      return -1;
    }
    else if(rx_length == 0)
    {
      // no data
      return 0;
    }
    else if(rx_length >= DOUBLE_SIZE)
    {
      rx_buffer[rx_length] = '\0';
      memcpy(&angle, rx_buffer, DOUBLE_SIZE);
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
    if(rx_length < 0)
    {
      printf("Some uart error occurred. Why are there LESS than 0 bytes in the buffer!!!\n");
      return -1;
    }
    else if(rx_length == 0)
    {
      // no data
      return 0;
    }
    else
    {
      rx_buffer[rx_length] = '\0';
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
