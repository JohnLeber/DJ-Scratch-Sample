#pragma once
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

#define NORMAL_SPEED 50 
//-------------------------------------------------------------------------------------//
class CWASAPIRenderer : public IUnknown
{
    LONG m_nExitThread;

    BYTE* m_pPCMDataL;
    BYTE* m_pPCMDataR;
    DWORD m_dwPCMBufferSize;
    long m_nPosition;
    float m_nLastPosition;
public:
   // CWASAPIRenderer(void);
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
    
    void SetBuffers(BYTE* pPCMDataL, BYTE* pPCMDataR, DWORD dwSize);
   
    
private:
    CCriticalSection m_Lock;//lock buffers
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
