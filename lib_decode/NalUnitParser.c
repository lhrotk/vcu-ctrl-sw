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

/****************************************************************************
   -----------------------------------------------------------------------------
 **************************************************************************//*!
   \addtogroup lib_decode_hls
   @{
   \file
 *****************************************************************************/

#include <assert.h>

#include "lib_common/Utils.h"
#include "lib_common/HwScalingList.h"

#include "lib_common_dec/DecSliceParam.h"
#include "lib_common_dec/DecBuffers.h"
#include "lib_common_dec/RbspParser.h"

#include "lib_parsing/Avc_PictMngr.h"
#include "lib_parsing/Hevc_PictMngr.h"
#include "lib_parsing/SliceHdrParsing.h"

#include "FrameParam.h"
#include "I_DecoderCtx.h"
#include "DefaultDecoder.h"
#include "SliceDataParsing.h"

/*************************************************************************//*!
   \brief This function returns a pointer to the Nal data cleaned of the AE bytes
   \param[in]  pBufNoAE  Pointer to the buffer receiving the nal data without the antiemulated bytes
   \param[in]  pStream   Pointer to the circular stream buffer
   \param[in]  uLength   Maximum bytes's number to be parsed
*****************************************************************************/
static uint32_t AL_sCount_AntiEmulBytes(TCircBuffer* pStream, uint32_t uLength)
{
  uint8_t uSCDetect = 0;
  uint32_t uNumAE = 0;
  uint32_t uZeroBytesCount = 0;

  uint8_t* pBuf = pStream->tMD.pVirtualAddr;

  uint32_t uSize = pStream->tMD.uSize;
  uint32_t uOffset = pStream->iOffset;
  uint32_t uEnd = uOffset + uLength;

  // Replaces in m_pBuffer all sequences such as 0x00 0x00 0x03 0xZZ with 0x00 0x00 0xZZ (0x03 removal)
  // iff 0xZZ == 0x00 or 0x01 or 0x02 or 0x03.
  for(uint32_t uRead = uOffset; uRead < uEnd; ++uRead)
  {
    const uint8_t read = pBuf[uRead % uSize];

    if((uZeroBytesCount == 2) && (read == 0x03))
    {
      uZeroBytesCount = 0;
      ++uEnd;
      ++uNumAE;
      continue;
    }

    if((uZeroBytesCount >= 2) && (read == 0x01))
    {
      ++uSCDetect;

      if(uSCDetect == 2)
        return uNumAE;
    }

    if(read == 0x00)
      ++uZeroBytesCount;
    else
      uZeroBytesCount = 0;
  }

  return uNumAE;
}

/*****************************************************************************/
static uint32_t GetNonVclSize(uint32_t uOffset, TCircBuffer* pBufStream)
{
  int iNumZeros = 0;
  int iNumNALFound = 0;
  uint8_t* pParseBuf = pBufStream->tMD.pVirtualAddr;
  uint32_t uSize = pBufStream->tMD.uSize;
  uint32_t uLengthNAL = 0;

  for(uint32_t i = uOffset; i < uOffset + uSize; ++i)
  {
    uint8_t uRead = pParseBuf[i % uSize];

    if(iNumZeros >= 2 && uRead == 0x01)
    {
      if(++iNumNALFound == 2)
      {
        uLengthNAL -= iNumZeros;
        break;
      }
      else
        iNumZeros = 0;
    }

    if(uRead == 0x00)
      ++iNumZeros;
    else
      iNumZeros = 0;

    ++uLengthNAL;
  }

  return RoundUp(uLengthNAL, ANTI_EMUL_GRANULARITY);
}

/*****************************************************************************/
static void InitNonVclBuf(AL_TDecCtx* pCtx, uint32_t uOffset, TCircBuffer* pBufStream)
{
  uint32_t uLengthNAL = GetNonVclSize(uOffset, pBufStream);

  if(uLengthNAL > pCtx->BufNoAE.tMD.uSize) /* should occurs only on long SEI message */
  {
    Rtos_Free(pCtx->BufNoAE.tMD.pVirtualAddr);
    pCtx->BufNoAE.tMD.pVirtualAddr = (uint8_t*)Rtos_Malloc(uLengthNAL);
    pCtx->BufNoAE.tMD.uSize = uLengthNAL;
  }

  Rtos_Memset(pCtx->BufNoAE.tMD.pVirtualAddr, 0, pCtx->BufNoAE.tMD.uSize);
}

/*****************************************************************************/
static uint32_t GetSliceHdrSize(AL_TRbspParser* pRP, TCircBuffer* pBufStream)
{
  uint32_t uLengthNAL = (offset(pRP) + 7) >> 3;
  int iNumAE = AL_sCount_AntiEmulBytes(pBufStream, uLengthNAL);
  return uLengthNAL + iNumAE;
}

/*****************************************************************************/
void UpdateContextAtEndOfFrame(AL_TDecCtx* pCtx)
{
  pCtx->bIsFirstPicture = false;
  pCtx->bLastIsEOS = false;

  pCtx->tConceal.iFirstLCU = -1;
  pCtx->tConceal.bValidFrame = false;
  pCtx->PictMngr.uNumSlice = 0;

  Rtos_Memset(&pCtx->PoolPP[pCtx->uToggle], 0, sizeof(AL_TDecPicParam));
  Rtos_Memset(&pCtx->PoolPB[pCtx->uToggle], 0, sizeof(AL_TDecPicBuffers));
  AL_SET_DEC_OPT(&pCtx->PoolPP[pCtx->uToggle], IntraOnly, 1);
}

/*****************************************************************************/
void UpdateCircBuffer(AL_TRbspParser* pRP, TCircBuffer* pBufStream, int* pSliceHdrLength)
{
  uint32_t uLengthNAL = GetSliceHdrSize(pRP, pBufStream);

  // remap stream offset
  if(pRP->iTotalBitIndex % 8)
    *pSliceHdrLength = 16 + (pRP->iTotalBitIndex % 8);
  else
    *pSliceHdrLength = 24;
  uLengthNAL -= (*pSliceHdrLength + 7) >> 3;

  // Update Circular buffer information
  pBufStream->iAvailSize -= uLengthNAL;
  pBufStream->iOffset += uLengthNAL;
  pBufStream->iOffset %= pBufStream->tMD.uSize;
}

/*****************************************************************************/
bool SkipNal()
{
  return false;
}

/*****************************************************************************/
AL_TRbspParser getParserOnNonVclNal(AL_TDecCtx* pCtx)
{
  TCircBuffer* pBufStream = &pCtx->Stream;
  uint32_t uOffset = pBufStream->iOffset;
  InitNonVclBuf(pCtx, uOffset, pBufStream);
  AL_TRbspParser rp;
  InitRbspParser(pBufStream, pCtx->BufNoAE.tMD.pVirtualAddr, true, &rp);
  return rp;
}

