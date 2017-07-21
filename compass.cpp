#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <ncurses.h>

#define I2C_ADDR 0x1E

int handle;
unsigned char command[2];

void compassInit();
void readCompass(int&, int&, int&);

void compassInit()
{
  handle = i2cOpen(1, I2C_ADDR, 0);
  command[0] = 0x00;
  command[1] = 0xB8;
  i2cWriteDevice(handle, command&, 2);
}

void readCompass(int x&, int y&, int z&)
{
  unsigned char value[6];
  const int firstByte = 0x03;
  for(i = 0; i < 6; i++)
  {
    value[i] = i2cReadByteData(handle, firstByte+i);
  }
  x = (value[0] << 8) | value[1];
  z = (value[2] << 8) | value[3];
  y = (value[4] << 8) | value[5];
}
