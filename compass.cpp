#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_ADDR (0x3C >> 1)

int file_i2c;
int length;
unsigned int microseconds;
unsigned char buffer[60] = {0};

void openi2c();
void compassInit();
void readCompass(short&, short&, short&);
int I2CRead(unsigned char, unsigned char&);
int I2CWrite(unsigned char, unsigned char);

int main()
{
    short x, y, z;
    openi2c();
    compassInit();
    while(1)
    {
      readCompass(x, y, z);
      std::cout << "x: " << x/2048.0*360 << " y: " << y/2048.0*360 << " z: " << z/2048.0*360 << std::endl ;
      usleep(1000000);
    }
    close(file_i2c);
    return 0;
}
void openi2c() {
  char *filename = (char*)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return;
	}
}
void compassInit()
{
  I2CWrite(0x00, 0x90);
  I2CWrite(0x02, 0x00);
  usleep(100000);
}

void readCompass(short& x, short& y, short& z)
{
  unsigned char value[6];
  I2CRead(0x03, value[0]);
  I2CRead(0x04, value[1]);
  I2CRead(0x05, value[2]);
  I2CRead(0x06, value[3]);
  I2CRead(0x07, value[4]);
  I2CRead(0x08, value[5]);
  /*buffer[0] = 0x03;
  length = 1;
  if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
  {
  		// ERROR HANDLING: i2c transaction failed
  		printf("3) Failed to write to the i2c bus.\n");
  }
  for(int i = 0; i < 6; i++)
  {
    usleep(1000);
    length = 1;			//<<< Number of bytes to read
    if (read(file_i2c, buffer, length) != length)		//read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
    {
      //ERROR HANDLING: i2c transaction failed
      printf("Failed to read from the i2c bus.\n");
    }
    else
    {
      printf("Data read: %x\n", buffer[0]);
      value[i] = buffer[0];
    }
  }*/
  x = (value[0] << 8) | value[1];
  z = (value[2] << 8) | value[3];
  y = (value[4] << 8) | value[5];
}


int I2CRead(unsigned char registerAddress, unsigned char &data) {
  unsigned char *inbuff, outbuff;
  int retVal = -1;
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];

  outbuff = registerAddress;
  messages[0].addr = I2C_ADDR;
  messages[0].flags= 0;
  messages[0].len = sizeof(outbuff);
  messages[0].buf = &outbuff;

  inbuff = &data;
  messages[1].addr = I2C_ADDR;
  messages[1].flags = I2C_M_RD;
  messages[1].len = sizeof(*inbuff);
  messages[1].buf = inbuff;

  packets.msgs = messages;
  packets.nmsgs = 2;

  retVal = ioctl(file_i2c, I2C_RDWR, &packets);
  if(retVal < 0)
      perror("Read from I2C Device failed");

  return retVal;
}

int I2CWrite(unsigned char registerAddress, unsigned char data) {
  unsigned char buff[2];
   int retVal = -1;
   struct i2c_rdwr_ioctl_data packets;
   struct i2c_msg messages[1];

   buff[0] = registerAddress;
   buff[1] = data;

   messages[0].addr = I2C_ADDR;
   messages[0].flags = 0;
   messages[0].len = sizeof(buff);
   messages[0].buf = buff;

   packets.msgs = messages;
   packets.nmsgs = 1;

   retVal = ioctl(file_i2c, I2C_RDWR, &packets);
   if(retVal < 0)
       perror("Write to I2C Device failed");

   return retVal;
}
