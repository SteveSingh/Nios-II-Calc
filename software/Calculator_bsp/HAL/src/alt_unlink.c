/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/

/******************************************************************************
*                                                                             *
* THIS IS A LIBRARY READ-ONLY SOURCE FILE. DO NOT EDIT IT DIRECTLY.           *
*                                                                             *
* Overriding HAL Functions                                                    *
*                                                                             *
* To provide your own implementation of a HAL function, include the file in   *
* your Nios II IDE application project. When building the executable, the     *
* Nios II IDE finds your function first, and uses it in place of the HAL      *
* version.                                                                    *
*                                                                             *
******************************************************************************/

#include "sys/alt_errno.h"
#include "sys/alt_warning.h"
#include "os/alt_syscall.h"

/*
 * unlink() is used by newlib to delete an existing link to a file. This is 
 * unsupported in the HAL environment. However a "do-nothing" implementation 
 * is still provied for newlib compatability.
 *
 * ALT_UNLINK is mapped onto the unlink() system call in alt_syscall.h
 */
 
int ALT_UNLINK (char *name)
{
  /* Generate a link time warning, should this function ever be called. */
  
  ALT_STUB_WARNING(unlink);
  
  /* Indicate an error */
  
  ALT_ERRNO = ENOSYS;
  return -1;
}
