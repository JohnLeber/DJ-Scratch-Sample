#include "pch.h"
#include "LowPassIIR.h"
//----------------------------------------------------------------------------//
CLowPassIIR::CLowPassIIR()
{
     m_pIIRWorkingBuffer = 0;
     m_pIIRState = 0;  
     long nS = 2 * (LOWPASS_FILTER_ORDER + 1);
     for (int h = 0; h < nS; h++)
     {
         m_pIIRTaps[h] = 0;
     }
}
//----------------------------------------------------------------------------//
CLowPassIIR::~CLowPassIIR()
{
    if (m_pIIRWorkingBuffer)
    {
        ippsFree(m_pIIRWorkingBuffer);
        m_pIIRWorkingBuffer = 0;
    }
    if (m_pIIRState)
    {
        //ippsIIRFree_64f(m_pIIRState); //used to call this but doens't seem to be supported aby IPP anymore?
        m_pIIRState = 0;
    }
}
//----------------------------------------------------------------------------//
bool CLowPassIIR::Init(Ipp64f nCutoff)
{ 
    int nBufferSize = 0; 
    IppStatus status = ippsIIRGenGetBufferSize(LOWPASS_FILTER_ORDER, &nBufferSize);
    if (status == ippStsNoErr)
    {
        m_pIIRWorkingBuffer = ippsMalloc_8u(nBufferSize);
        ZeroMemory(m_pIIRWorkingBuffer, nBufferSize);
        IppStatus status = ippsIIRGenLowpass_64f(nCutoff, 0.5, LOWPASS_FILTER_ORDER, m_pIIRTaps, ippButterworth/*ippChebyshev1*/, m_pIIRWorkingBuffer);
        if (status == ippStsNoErr)
        {
            status = ippsIIRInit_64f(&m_pIIRState, m_pIIRTaps, LOWPASS_FILTER_ORDER, 0, m_pIIRWorkingBuffer);
            if (status == ippStsNoErr)
            {
                return true;
            }
        }
    }
    return false;
}
//----------------------------------------------------------------------------// 
Ipp64f CLowPassIIR::Filter(Ipp64f x)
{  
    Ipp64f y = 0;
    int status = ippsIIR_64f(&x, &y, 1, m_pIIRState);
    if (y != y)//testing for nan, which means the filter has gone unstable
    {
        ATLASSERT(0);
    }
    if (status == ippStsNoErr)
    { 
        return y;
    }
    return x;
}
//----------------------------------------------------------------------------// 
void CLowPassIIR::Filter(WORD* pDst, WORD* pSrc, int nSize)
{ 
    const int nChuckSize = 4096; 
    Ipp64f y = 0;
    Ipp64f x = 0;
    //this could be vectorized but we'll use a loop to ckeep it simple...
    for (int h = 0; h < nSize; h++)
    {
        x = (short)pSrc[h];
        int status = ippsIIR_64f(&x, &y, 1, m_pIIRState);
        pDst[h] = (WORD)(short)y;
    }
}
//----------------------------------------------------------------------------// 