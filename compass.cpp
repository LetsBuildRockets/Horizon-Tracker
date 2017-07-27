#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>
#include <linux/i2c-dev.h>

#define I2C_ADDR 0x1C

int file_i2c;
int length;
unsigned char buffer[60] = {0};

void compassInit();
void readCompass(int&, int&, int&);

int main()
{
    int x, y, z;
    compassInit();
    readCompass(x, y, z);
    std::cout << "x: " << x << " y: " << y << " z: " << z << std::endl ;
}

void compassInit()
{
  char *filename = (char*)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return;
	}
  if (ioctl(file_i2c, I2C_SLAVE, I2C_ADDR) < 0)
  {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    //ERROR HANDLING; you can check errno to see what went wrong
    return;
  }
  buffer[0] = 0x00;
  buffer[1] = 0xB8;
  length = 2;			//<<< Number of bytes to write
  if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
  {
  		/* ERROR HANDLING: i2c transaction failed */
  		printf("1) Failed to write to the i2c bus.\n");
  }
  buffer[0] = 0x02;
  buffer[1] = 0x00;
  length = 2;			//<<< Number of bytes to write
  if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
  {
  		/* ERROR HANDLING: i2c transaction failed */
  		printf("2) Failed to write to the i2c bus.\n");
  }
}

void readCompass(int& x, int& y, int& z)
{
  unsigned char value[6];
  buffer[0] = 0x03;
  length = 1;
  if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
  {
  		/* ERROR HANDLING: i2c transaction failed */
  		printf("3) Failed to write to the i2c bus.\n");
  }
  for(int i = 0; i < 6; i++)
  {
    length = 1;			//<<< Number of bytes to read
    if (read(file_i2c, buffer, length) != length)		//read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
    {
      //ERROR HANDLING: i2c transaction failed
      printf("Failed to read from the i2c bus.\n");
    }
    else
    {
      printf("Data read: %s\n", buffer);
      value[i] = buffer[0];
    }
  }
  x = (value[0] << 8) | value[1];
  z = (value[2] << 8) | value[3];
  y = (value[4] << 8) | value[5];
}
