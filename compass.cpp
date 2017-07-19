#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <ncurses.h>

#define I2C_ADDR 1234

int handle;

void compassInit();
void readCompass(unsigned char[4]&);

void compassInit()
{
  handle = i2cOpen(1, I2C_ADDR, 0);
}
