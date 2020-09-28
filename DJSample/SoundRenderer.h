#pragma once
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
//const long LOWPASS_FILTER_ORDER = 9;
#define NORMAL_SPEED 50 
class CLowPassIIR;
//-------------------------------------------------------------------------------------//
class CWASAPIRenderer : public IUnknown
{
    LONG m_nExitThread;

    WORD* m_pPCMDataL;
    WORD* m_pPCMDataR;
    WORD* m_pPCMFilteredDataL;
    WORD* m_pPCMFilteredDataR;
    int m_nPCMBufferSize;
    long m_nPosition;
    double m_nLastPosition; 
public:
    ~CWASAPIRenderer(void);
    void SetSpeed(LONG nSpeed);
      
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
    
    void SetBuffers(WORD* pPCMDataL, WORD* pPCMDataR, WORD* pPCMFilteredDataL, WORD* pPCMFilteredDataR, int dwSize);
    void SetNearestSample(BOOL bNearestSample);
    void EnableLowPassFilter(BOOL bLowPassFilter);
    
private:
    CCriticalSection m_Lock;//lock buffers
    LONG m_nLowPassFilter;
    LONG m_nNearestSample;
    long m_nNumChannels;
    LONG m_nSpeed;
    bool m_bNearestSample;
    LONG    _RefCount;
    IMMDevice* _Endpoint;
    IAudioClient* _AudioClient;
    IAudioRenderClient* _RenderClient;
    UINT32 m_BufferLength; 
    HANDLE      _RenderThread;
    HANDLE      _ShutdownEvent;
    WAVEFORMATEX* _MixFormat;
    UINT32      _FrameSize;
    RenderSampleType _RenderSampleType;
    UINT32      _BufferSize;
    LONG        _EngineLatencyInMS;

    static DWORD __stdcall WASAPIRenderThread(LPVOID Context);
    DWORD CWASAPIRenderer::DoRenderThread();

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
