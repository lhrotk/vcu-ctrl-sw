/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <assert.h>

#include "CodecUtils.h"
#include "al_resource.h"
#include "lib_app/utils.h"

/******************************************************************************/
void DisplayFrameStatus(int iFrameNum)
{
#if VERBOSE_MODE
  Message("\n\n> % 3d", iFrameNum);
#else
  Message("\r  Displayed picture #%-6d - ", iFrameNum);
#endif
}


/*****************************************************************************/
void DisplayVersionInfo()
{
  Message("%s - %s v%d.%d.%d - %s\n", AL_DECODER_COMPANY,
          AL_DECODER_PRODUCT_NAME,
          AL_DECODER_VERSION,
          AL_DECODER_COPYRIGHT);

  SetConsoleColor(CC_YELLOW);

  Message("%s\n", AL_DECODER_COMMENTS);

  SetConsoleColor(CC_DEFAULT);

  Message("\n");
}

