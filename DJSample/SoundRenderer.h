#pragma once
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

#define NORMAL_SPEED 50
struct CSharedBuffer
{
    CSharedBuffer()
    {
        Clear();
    }
    BYTE* m_pData;
    LONG m_nIndex;
    LONG m_nMax;
    void Clear()
    {
        m_pData = 0;
        m_nIndex = 0;
        m_nMax = 0;
    }
};
 
//-------------------------------------------------------------------------------------//
class CWASAPIRenderer : public IUnknown
{
    LONG m_nExitThread;
public:
    CWASAPIRenderer(void);
    ~CWASAPIRenderer(void);
    void SetSpeed(LONG nSpeed);
    void DoWork();

    BYTE* m_pPCMDataL;
    BYTE* m_pPCMDataR;
    DWORD m_dwPCMBufferSize;
    long m_nPosition;
    float m_nLastPosition;
 
     
    //CAudioDelay m_Delay;
  //  CAudioBuffer* m_pAudioBuffer;
    enum RenderSampleType
    {
        SampleTypeFloat,
        SampleType16BitPCM,
    };
 
    CWASAPIRenderer(IMMDevice* Endpoint);
    bool Initialize(UINT32 EngineLatency);
    void Shutdown();
    bool Start();
    void Stop();
    WORD ChannelCount() { return _MixFormat->nChannels; }
    UINT32 SamplesPerSecond() { return _MixFormat->nSamplesPerSec; }
    UINT32 BytesPerSample() { return _MixFormat->wBitsPerSample / 8; }
    RenderSampleType SampleType() { return _RenderSampleType; }
    UINT32 FrameSize() { return _FrameSize; }
    UINT32 BufferSize() { return _BufferSize; }
    UINT32 BufferSizePerPeriod();
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    double m_nFrequency;
    double m_nFrequency2;
    double m_nFrequency3;
    double m_nMix;
    double m_nAmount;
    double m_nVolumne;

    CSharedBuffer* m_pBuffer;
    
    bool m_bCarrierInput;
    bool m_bAM;
    bool m_bInputSquare;
    bool m_bLCOSquare;
    double m_lastcarrier;
    bool m_bUseMic;
    long m_nMixFrequency;
private:
    long m_nNumChannels;
    LONG m_nSpeed;
    bool m_bNearestSample;
    LONG    _RefCount;
    IMMDevice* _Endpoint;
    IAudioClient* _AudioClient;
    IAudioRenderClient* _RenderClient;
    UINT32 m_BufferLength;

    //UINT32 FrameSize() { return _FrameSize; }

    HANDLE      _RenderThread;
    HANDLE      _ShutdownEvent;
    WAVEFORMATEX* _MixFormat;
    UINT32      _FrameSize;
    RenderSampleType _RenderSampleType;
    UINT32      _BufferSize;
    LONG        _EngineLatencyInMS;

    static DWORD __stdcall WASAPIRenderThread(LPVOID Context);
    DWORD CWASAPIRenderer::DoRenderThread();
    double InitialTheta;
    double InitialTheta2;
    double InitialTheta3;
    
 


    //
   //  IUnknown
   //
    STDMETHOD(QueryInterface)(REFIID iid, void** pvObject);

    //
    //  Utility functions.
    //
    bool CalculateMixFormatType();
    bool InitializeAudioEngine();
    bool LoadFormat();
};
