#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

const int DOUBLE_SIZE = sizeof(double);

int uart0Filestream = -1;

void UARTInit()
{
  uart0Filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
  if(uart0Filestream == -1)
  {
    printf("Cant open UART\n");
  }

  struct termios options;
  tcgetattr(uart0Filestream, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart0Filestream, TCIFLUSH);
  tcsetattr(uart0Filestream, TCSANOW, &options);
}

int sendStartByte() {
  #ifdef SERF
    printf("I'm not sending the start byte. I'm the SERF!");
  #endif
  char const data[] = {0x12};
  if(uart0Filestream)
  {
    int count = write(uart0Filestream, &data, 1);
    if(count < 0)
    {
      printf("UART TX error\n");
      return -1;
    }
  }
}

int setAngleData(double & angle) {
  #ifndef SERF
    printf("I'm not sending angle data. I'm the MASTER!\n");
    return -1;
  #endif
  char * const data = reinterpret_cast<char * const>(&angle);
  if(uart0Filestream)
  {
    int count = write(uart0Filestream, &data, DOUBLE_SIZE);
    if(count < 0)
    {
      printf("UART TX error\n");
      return -1;
    }
  }
}
