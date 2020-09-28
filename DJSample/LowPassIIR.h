#pragma once
class CLowPassIIR
{
    Ipp8u* m_pIIRWorkingBuffer;
    Ipp64f m_pIIRTaps[2 * (LOWPASS_FILTER_ORDER + 1)];
    IppsIIRState_64f* m_pIIRState;   
public:
    CLowPassIIR();
    ~CLowPassIIR();
    bool Init(Ipp64f nCuroff);
    Ipp64f Filter(Ipp64f x);
    void Filter(Ipp16u* pDst, Ipp16u* pSrc, int nSize);
};

