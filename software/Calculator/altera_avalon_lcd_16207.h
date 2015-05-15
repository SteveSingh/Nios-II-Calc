#ifndef __ALTERA_AVALON_LCD_16207_H__
#define __ALTERA_AVALON_LCD_16207_H__

#include <stddef.h>

#include "sys/alt_dev.h"
#include "sys/alt_alarm.h"
#include "os/alt_sem.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 * The alt_LCD_16207_dev structure is used to hold device specific data. This
 * includes the transmit and receive buffers.
 *
 * An instance of this structure is created in the auto-generated
 * alt_sys_init.c file for each UART listed in the systems PTF file. This is
 * done using the ALTERA_AVALON_LCD_16207_INSTANCE macro given below.
 */

#define ALT_LCD_HEIGHT         2
#define ALT_LCD_WIDTH         16
#define ALT_LCD_VIRTUAL_WIDTH 80

typedef struct alt_LCD_16207_dev alt_LCD_16207_dev;
struct alt_LCD_16207_dev
{
  alt_dev        dev;
  int            base;

  alt_alarm      alarm;
  int            period;

  char           broken;

  unsigned char  x;
  unsigned char  y;
  char           address;
  char           esccount;

  char           scrollpos;
  char           scrollmax;
  char           active;    /* If non-zero then the foreground routines are
                             * active so the timer call must not update the
                             * display. */

  char           escape[8];

  struct
  {
    char         visible[ALT_LCD_WIDTH];
    char         data[ALT_LCD_VIRTUAL_WIDTH+1];
    char         width;
    unsigned char speed;

  } line[ALT_LCD_HEIGHT];

  ALT_SEM       (write_lock)/* Semaphore used to control access to the
                             * write buffer in multi-threaded mode */
};

/*
 * alt_lcd_16207_init is called by alt_sys_init.c to initialise the driver.
 */
void alt_lcd_16207_init(alt_LCD_16207_dev * dev);

/*
 * alt_lcd_16207_write() is called by the write() system call for all
 * valid attempts to write to an instance of this device.
 */
int alt_lcd_16207_write(alt_fd * fd, const char* ptr, int len);

/* The LCD panel driver is not trivial, so leave it out in the small
 * drivers case.  Also leave it out in simulation because there is no
 * simulated hardware for the LCD panel.  These two can be overridden
 * by defining ALT_USE_LCE_16207 if you really want it.
 */

#if (!defined(ALT_USE_SMALL_DRIVERS) && !defined(ALT_SIM_OPTIMIZE)) || defined ALT_USE_LCD_16207

/*
 * The macro ALTERA_AVALON_LCD_16207_INSTANCE is used by the
 * auto-generated file
 * alt_sys_init.c to create an instance of this device driver.
 */
#define ALTERA_AVALON_LCD_16207_INSTANCE(name, device)   \
  static alt_LCD_16207_dev device =                      \
    {                                                    \
      {                                                  \
        ALT_LLIST_ENTRY,                                 \
        name##_NAME,                                     \
        NULL, /* open */                                 \
        NULL, /* close */                                \
        NULL, /* read */                                 \
        alt_lcd_16207_write,                             \
        NULL, /* lseek */                                \
        NULL, /* fstat */                                \
        NULL, /* ioctl */                                \
      },                                                 \
      name##_BASE                                        \
    }

/*
 * The macro ALTERA_AVALON_LCD_16207_INIT is used by the auto-generated file
 * alt_sys_init.c to initialise an instance of the device driver.
 */
#define ALTERA_AVALON_LCD_16207_INIT(name, device) alt_lcd_16207_init(&device)

#else /* exclude driver */

#define ALTERA_AVALON_LCD_16207_INSTANCE(name, dev) extern int alt_no_storage
#define ALTERA_AVALON_LCD_16207_INIT(name, dev) while (0)

#endif

/*
 *
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
