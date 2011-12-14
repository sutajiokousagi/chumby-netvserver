#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define DEV_I2C_0       "/dev/i2c-0"

/*
 * Ioctl definitions -- should sync to kernel source instead of copying here
 */

#define FPGA_IOC_MAGIC  'c'

#define FPGA_IOCWTEST    _IOW(FPGA_IOC_MAGIC,  1, int)
#define FPGA_IOCRTEST    _IOR(FPGA_IOC_MAGIC,  2, int)
#define FPGA_IOCRESET    _IOW(FPGA_IOC_MAGIC,  3, int)
#define FPGA_IOCLED0     _IOW(FPGA_IOC_MAGIC,  4, int)
#define FPGA_IOCLED1     _IOW(FPGA_IOC_MAGIC,  5, int)
#define FPGA_IOCDONE     _IOR(FPGA_IOC_MAGIC,  6, int)
#define FPGA_IOCINIT     _IOR(FPGA_IOC_MAGIC,  7, int)


/*
 * Function prototypes
 */

void ioctl_reset(int file_desc);
void ioctl_led0(int file_desc, int state);
void ioctl_led1(int file_desc, int state);
void ioctl_done(int file_desc, int *state);
void ioctl_init(int file_desc, int *state);
int write_eeprom(const char *i2c_device, int addr, int start_reg,unsigned char *buffer, int bytes);
int read_eeprom(const char *i2c_device, int addr, int start_reg,unsigned char *buffer, int bytes);
void print_fpga_version();
int dump_registers(int stats);
int dump_register(unsigned char address);
void print_help(char code);
int main_motor(int argc, char **argv);
