/*
 * This file provides the implementation of the functions used to drive a
 * LCD panel.
 *
 * Characters written to the device will appear on the LCD panel as though
 * it is a very small terminal.  If the lines written to the terminal are
 * longer than the number of characters on the terminal then it will scroll
 * the lines of text automatically to display them all.
 *
 * If more lines are written than will fit on the terminal then it will scroll
 * when characters are written to the line "below" the last displayed one -
 * the cursor is allowed to sit below the visible area of the screen providing
 * that this line is entirely blank.
 *
 * The following control sequences may be used to move around and do useful
 * stuff:
 *    CR    Moves back to the start of the current line
 *    LF    Moves down a line and back to the start
 *    BS    Moves back a character without erasing
 *    ESC   Starts a VT100 style escape sequence
 *
 * The following escape sequences are recognised:
 *    ESC [ <row> ; <col> H   Move to row and column specified (positions are
 *                            counted from the top left which is 1;1)
 *    ESC [ K                 Clear from current position to end of line
 *    ESC [ 2 J               Clear screen and go to top left
 *
 */

/* ===================================================================== */

#include <string.h>
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "sys/alt_dev.h"
#include "sys/alt_alarm.h"

#include "altera_avalon_lcd_16207.h"
#include "altera_avalon_lcd_16207_regs.h"

/* --------------------------------------------------------------------- */

/* Commands which can be written to the COMMAND register */

enum /* Write to character RAM */
{
  LCD_CMD_WRITE_DATA    = 0x80
  /* Bits 6:0 hold character RAM address */
};

enum /* Write to character generator RAM */
{
  LCD_CMD_WRITE_CGR     = 0x40
  /* Bits 5:0 hold character generator RAM address */
};

enum /* Function Set command */
{
  LCD_CMD_FUNCTION_SET  = 0x20,
  LCD_CMD_8BIT          = 0x10,
  LCD_CMD_TWO_LINE      = 0x08,
  LCD_CMD_BIGFONT       = 0x04
};

enum /* Shift command */
{
  LCD_CMD_SHIFT         = 0x10,
  LCD_CMD_SHIFT_DISPLAY = 0x08,
  LCD_CMD_SHIFT_RIGHT   = 0x04
};

enum /* On/Off command */
{
  LCD_CMD_ONOFF         = 0x08,
  LCD_CMD_ENABLE_DISP   = 0x04,
  LCD_CMD_ENABLE_CURSOR = 0x02,
  LCD_CMD_ENABLE_BLINK  = 0x01
};

enum /* Entry Mode command */
{
  LCD_CMD_MODES         = 0x04,
  LCD_CMD_MODE_INC      = 0x02,
  LCD_CMD_MODE_SHIFT    = 0x01
};

enum /* Home command */
{
  LCD_CMD_HOME          = 0x02
};

enum /* Clear command */
{
  LCD_CMD_CLEAR         = 0x01
};

/* Where in LCD character space do the rows start */
static char colstart[4] = { 0x00, 0x40, 0x20, 0x60 };

/* --------------------------------------------------------------------- */

static void lcd_write_command(alt_LCD_16207_dev * dev, unsigned char command)
{
  unsigned int base = dev->base;

  /* We impose a timeout on the driver in case the LCD panel isn't connected.
   * The first time we call this function the timeout is approx 25ms
   * (assuming 5 cycles per loop and a 200MHz clock).  Obviously systems
   * with slower clocks, or debug builds, or slower memory will take longer.
   */
  int i = 1000000;

  /* Don't bother if the LCD panel didn't work before */
  if (dev->broken)
    return;

  /* Wait until LCD isn't busy. */
  while (IORD_ALTERA_AVALON_LCD_16207_STATUS(base) & ALTERA_AVALON_LCD_16207_STATUS_BUSY_MSK)
    if (--i == 0)
    {
      dev->broken = 1;
      return;
    }

  /* Despite what it says in the datasheet, the LCD isn't ready to accept
   * a write immediately after it returns BUSY=0.  Wait for 100us more.
   */
  usleep(100);

  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, command);
}

/* --------------------------------------------------------------------- */

static void lcd_write_data(alt_LCD_16207_dev * dev, unsigned char data)
{
  unsigned int base = dev->base;

  /* We impose a timeout on the driver in case the LCD panel isn't connected.
   * The first time we call this function the timeout is approx 25ms
   * (assuming 5 cycles per loop and a 200MHz clock).  Obviously systems
   * with slower clocks, or debug builds, or slower memory will take longer.
   */
  int i = 1000000;

  /* Don't bother if the LCD panel didn't work before */
  if (dev->broken)
    return;

  /* Wait until LCD isn't busy. */
  while (IORD_ALTERA_AVALON_LCD_16207_STATUS(base) & ALTERA_AVALON_LCD_16207_STATUS_BUSY_MSK)
    if (--i == 0)
    {
      dev->broken = 1;
      return;
    }

  /* Despite what it says in the datasheet, the LCD isn't ready to accept
   * a write immediately after it returns BUSY=0.  Wait for 100us more.
   */
  usleep(100);

  IOWR_ALTERA_AVALON_LCD_16207_DATA(base, data);

  dev->address++;
}

/* --------------------------------------------------------------------- */

static void lcd_clear_screen(alt_LCD_16207_dev * dev)
{
  int y;

  lcd_write_command(dev, LCD_CMD_CLEAR);

  dev->x = 0;
  dev->y = 0;
  dev->address = 0;

  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    memset(dev->line[y].data, ' ', sizeof(dev->line[0].data));
    memset(dev->line[y].visible, ' ', sizeof(dev->line[0].visible));
    dev->line[y].width = 0;
  }
}

/* --------------------------------------------------------------------- */

static void lcd_repaint_screen(alt_LCD_16207_dev * dev)
{
  int y, x;

  /* scrollpos controls how much the lines have scrolled round.  The speed
   * each line scrolls at is controlled by its speed variable - while
   * scrolline lines will wrap at the position set by width
   */

  int scrollpos = dev->scrollpos;

  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    int width  = dev->line[y].width;
    int offset = (scrollpos * dev->line[y].speed) >> 8;
    if (offset >= width)
      offset = 0;

    for (x = 0 ; x < ALT_LCD_WIDTH ; x++)
    {
      char c = dev->line[y].data[(x + offset) % width];

      /* Writing data takes 40us, so don't do it unless required */
      if (dev->line[y].visible[x] != c)
      {
        unsigned char address = x + colstart[y];

        if (address != dev->address)
        {
          lcd_write_command(dev, LCD_CMD_WRITE_DATA | address);
          dev->address = address;
        }

        lcd_write_data(dev, c);
        dev->line[y].visible[x] = c;
      }
    }
  }
}

/* --------------------------------------------------------------------- */

static void lcd_scroll_up(alt_LCD_16207_dev * dev)
{
  int y;

  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    if (y < ALT_LCD_HEIGHT-1)
      memcpy(dev->line[y].data, dev->line[y+1].data, ALT_LCD_VIRTUAL_WIDTH);
    else
      memset(dev->line[y].data, ' ', ALT_LCD_VIRTUAL_WIDTH);
  }

  dev->y--;
}

/* --------------------------------------------------------------------- */

static void lcd_handle_escape(alt_LCD_16207_dev * dev, char c)
{
  int parm1 = 0, parm2 = 0;

  if (dev->escape[0] == '[')
  {
    char * ptr = dev->escape+1;
    while (isdigit(*ptr))
      parm1 = (parm1 * 10) + (*ptr++ - '0');

    if (*ptr == ';')
    {
      ptr++;
      while (isdigit(*ptr))
        parm2 = (parm2 * 10) + (*ptr++ - '0');
    }
  }
  else
    parm1 = -1;

  switch (c)
  {
  case 'H': /* ESC '[' <y> ';' <x> 'H'  : Move cursor to location */
  case 'f': /* Same as above */
    if (parm2 > 0)
      dev->x = parm2 - 1;
    if (parm1 > 0)
    {
      dev->y = parm1 - 1;
      if (dev->y > ALT_LCD_HEIGHT * 2)
        dev->y = ALT_LCD_HEIGHT * 2;
      while (dev->y > ALT_LCD_HEIGHT)
        lcd_scroll_up(dev);
    }
    break;

  case 'J':
    /*   ESC J      is clear to beginning of line    [unimplemented]
     *   ESC [ 0 J  is clear to bottom of screen     [unimplemented]
     *   ESC [ 1 J  is clear to beginning of screen  [unimplemented]
     *   ESC [ 2 J  is clear screen
     */
    if (parm1 == 2)
      lcd_clear_screen(dev);
    break;

  case 'K':
    /*   ESC K      is clear to end of line
     *   ESC [ 0 K  is clear to end of line
     *   ESC [ 1 K  is clear to beginning of line    [unimplemented]
     *   ESC [ 2 K  is clear line                    [unimplemented]
     */
    if (parm1 < 1)
    {
      if (dev->x < ALT_LCD_VIRTUAL_WIDTH)
        memset(dev->line[dev->y].data + dev->x, ' ', ALT_LCD_VIRTUAL_WIDTH - dev->x);
    }
    break;
  }
}

/* --------------------------------------------------------------------- */

int alt_lcd_16207_write(alt_fd* fd, const char* ptr, int len)
{
  alt_LCD_16207_dev * dev = (alt_LCD_16207_dev*) fd->dev;
  const char * end = ptr + len;

  int y;
  int widthmax;

  /* When running in a multi threaded environment, obtain the "write_lock"
   * semaphore. This ensures that writing to the device is thread-safe.
   */

  ALT_SEM_PEND (dev->write_lock, 0);

  /* Tell the routine which is called off the timer interrupt that the
   * foreground routines are active so it must not repaint the display. */
  dev->active = 1;

  for ( ; ptr < end ; ptr++)
  {
    char c = *ptr;

    if (dev->esccount >= 0)
    {
      unsigned int esccount = dev->esccount;

      /* Single character escape sequences can end with any character
       * Multi character escape sequences start with '[' and contain
       * digits and semicolons before terminating
       */
      if ((esccount == 0 && c != '[') ||
          (esccount > 0 && !isdigit(c) && c != ';'))
      {
        dev->escape[esccount] = 0;

        lcd_handle_escape(dev, c);

        dev->esccount = -1;
      }
      else if (dev->esccount < sizeof(dev->escape)-1)
      {
        dev->escape[esccount] = c;
        dev->esccount++;
      }
    }
    else if (c == 27) /* ESC */
    {
      dev->esccount = 0;
    }
    else if (c == '\r')
    {
      dev->x = 0;
    }
    else if (c == '\n')
    {
      dev->x = 0;
      dev->y++;

      /* Let the cursor sit at X=0, Y=HEIGHT without scrolling so the user
       * can print two lines of data without losing one.
       */
      if (dev->y > ALT_LCD_HEIGHT)
        lcd_scroll_up(dev);
    }
    else if (c == '\b')
    {
      if (dev->x > 0)
        dev->x--;
    }
    else if (isprint(c))
    {
      /* If we didn't scroll on the last linefeed then we might need to do
       * it now. */
      if (dev->y >= ALT_LCD_HEIGHT)
        lcd_scroll_up(dev);

      if (dev->x < ALT_LCD_VIRTUAL_WIDTH)
        dev->line[dev->y].data[dev->x] = c;

      dev->x++;
    }
  }

  /* Recalculate the scrolling parameters */
  widthmax = ALT_LCD_WIDTH;
  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    int width;
    for (width = ALT_LCD_VIRTUAL_WIDTH ; width > 0 ; width--)
      if (dev->line[y].data[width-1] != ' ')
        break;

    /* The minimum width is the size of the LCD panel.  If the real width
     * is long enough to require scrolling then add an extra space so the
     * end of the message doesn't run into the beginning of it.
     */
    if (width <= ALT_LCD_WIDTH)
      width = ALT_LCD_WIDTH;
    else
      width++;

    dev->line[y].width = width;
    if (widthmax < width)
      widthmax = width;
    dev->line[y].speed = 0; /* By default lines don't scroll */
  }

  if (widthmax <= ALT_LCD_WIDTH)
    dev->scrollmax = 0;
  else
  {
    widthmax *= 2;
    dev->scrollmax = widthmax;

    /* Now calculate how fast each of the other lines should go */
    for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
      if (dev->line[y].width > ALT_LCD_WIDTH)
      {
        /* You have three options for how to make the display scroll, chosen
         * using the preprocessor directives below
         */
#if 1
        /* This option makes all the lines scroll round at different speeds
         * which are chosen so that all the scrolls finish at the same time.
         */
        dev->line[y].speed = 256 * dev->line[y].width / widthmax;
#elif 1
        /* This option pads the shorter lines with spaces so that they all
         * scroll together.
         */
        dev->line[y].width = widthmax / 2;
        dev->line[y].speed = 256/2;
#else
        /* This option makes the shorter lines stop after they have rotated
         * and waits for the longer lines to catch up
         */
        dev->line[y].speed = 256/2;
#endif
      }
  }

  /* Repaint once, then check whether there has been a missed repaint
   * (because active was set when the timer interrupt occurred).  If there
   * has been a missed repaint then paint again.  And again.  etc.
   */
  for ( ; ; )
  {
    int old_scrollpos = dev->scrollpos;

    lcd_repaint_screen(dev);

    /* Let the timer routines repaint the display again */
    dev->active = 0;

    /* Have the timer routines tried to scroll while we were painting?
     * If not then we can exit */
    if (dev->scrollpos == old_scrollpos)
      break;

    /* We need to repaint again since the display scrolled while we were
     * painting last time */
    dev->active = 1;
  }

  /* Now that access to the display is complete, release the write
   * semaphore so that other threads can access the buffer.
   */

  ALT_SEM_POST (dev->write_lock);

  return len;
}

/* --------------------------------------------------------------------- */

/* This should be in a top level header file really */
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

/*
 * Timeout routine is called every second
 */

alt_u32 alt_lcd_16207_timeout(void * context)
{
  alt_LCD_16207_dev * dev = (alt_LCD_16207_dev *) context;

  /* Update the scrolling position */
  if (dev->scrollpos + 1 >= dev->scrollmax)
    dev->scrollpos = 0;
  else
    dev->scrollpos = dev->scrollpos + 1;

  /* Repaint the panel unless the foreground will do it again soon */
  if (dev->scrollmax > 0 && !dev->active)
    lcd_repaint_screen(dev);

  return dev->period;
}

/* --------------------------------------------------------------------- */

/*
 * lcd_16207_init is called at boot time to initialise the LCD driver
 */
void alt_lcd_16207_init(alt_LCD_16207_dev * dev)
{
  unsigned int base = dev->base;

  /* Mark the device as functional */
  dev->broken = 0;

  ALT_SEM_CREATE (&dev->write_lock, 1);

  /* TODO: check that usleep can be called in an initialisation routine */

  /* The initialisation sequence below is copied from the datasheet for
   * the 16207 LCD display.  The first commands need to be timed because
   * the BUSY bit in the status register doesn't work until the display
   * has been reset three times.
   */

  /* Wait for 15 ms then reset */
  usleep(15000);
  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT);

  /* Wait for another 4.1ms and reset again */
  usleep(4100);
  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT);

  /* Wait a further 1 ms and reset a third time */
  usleep(1000);
  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT);

  /* Setup interface parameters: 8 bit bus, 2 rows, 5x7 font */
  lcd_write_command(dev, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT | LCD_CMD_TWO_LINE);

  /* Turn display off */
  lcd_write_command(dev, LCD_CMD_ONOFF);

  /* Clear display */
  lcd_clear_screen(dev);

  /* Set mode: increment after writing, don't shift display */
  lcd_write_command(dev, LCD_CMD_MODES | LCD_CMD_MODE_INC);

  /* Turn display on */
  lcd_write_command(dev, LCD_CMD_ONOFF | LCD_CMD_ENABLE_DISP);

  dev->esccount = -1;
  memset(dev->escape, 0, sizeof(dev->escape));

  dev->scrollpos = 0;
  dev->scrollmax = 0;
  dev->active = 0;

  dev->period = alt_ticks_per_second() / 10; /* Call every 100ms */

  alt_alarm_start(&dev->alarm, dev->period, &alt_lcd_16207_timeout, dev);

  /* make the device available to the system */
  alt_dev_reg(&dev->dev);
}

/* --------------------------------------------------------------------- */
