#include "pch.h"
#include "SoundRenderer.h"
#include <Synchapi.h>
#include <assert.h>
#include <avrt.h>
#include <mmsystem.h>
#include <math.h>
#include <limits.h>  
#include "lowpassIIR.h"
CWASAPIRenderer::CWASAPIRenderer(IMMDevice* Endpoint) :
    _RefCount(1),
    _Endpoint(Endpoint),
    _AudioClient(NULL),
    _RenderClient(NULL),
    _RenderThread(NULL),
    _ShutdownEvent(NULL),
    _MixFormat(NULL)
{
    m_pPCMDataL = 0;
    m_pPCMDataR = 0;
    m_pPCMFilteredDataL = 0;
    m_pPCMFilteredDataR = 0;
    m_nPCMBufferSize = 0;
    m_nPosition = 0;
    m_nLastPosition = 0;
    m_nNearestSample = 0;
    _Endpoint->AddRef();    // Since we're holding a copy of the endpoint, take a reference to it.  It'll be released in Shutdown();
     
    m_nLowPassFilter = 1; 

    m_nSpeed = NORMAL_SPEED; 
} 

CWASAPIRenderer::~CWASAPIRenderer(void)
{
    /*if (m_pIIRWorkingBuffer)
    {
        ippsFree(m_pIIRWorkingBuffer);
        m_pIIRWorkingBuffer = 0;
    }*/
    if (m_pPCMDataL)
    {
        delete[] m_pPCMDataL;
        m_pPCMDataL = 0;
    }
    if (m_pPCMDataR)
    {
        delete[] m_pPCMDataR;
        m_pPCMDataR = 0;
    }
    if (m_pPCMFilteredDataL)
    {
        delete[] m_pPCMFilteredDataL;
        m_pPCMFilteredDataL = 0;
    }
    if (m_pPCMFilteredDataR)
    {
        delete[] m_pPCMFilteredDataR;
        m_pPCMFilteredDataR = 0;
    } 
}

#define PERIODS_PER_BUFFER 4 

bool CWASAPIRenderer::InitializeAudioEngine()
{
    REFERENCE_TIME bufferDuration = _EngineLatencyInMS * 10000 * PERIODS_PER_BUFFER;
    REFERENCE_TIME periodicity = _EngineLatencyInMS * 10000;

    //
    //  We initialize the engine with a periodicity of _EngineLatencyInMS and a buffer size of PERIODS_PER_BUFFER times the latency - this ensures 
    //  that we will always have space available for rendering audio.  We only need to do this for exclusive mode timer driven rendering.
    //
    HRESULT hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,//AUDCLNT_SHAREMODE_EXCLUSIVE, 
        AUDCLNT_STREAMFLAGS_NOPERSIST,
        bufferDuration,
        periodicity,
        _MixFormat,
        NULL);
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to initialize audio client: %x.\n", hr);
        return false;
    }

    //
    //  Retrieve the buffer size for the audio client.
    //
    hr = _AudioClient->GetBufferSize(&_BufferSize);
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to get audio client buffer: %x. \n", hr);
        return false;
    }

    m_BufferLength = BufferSizePerPeriod() * _FrameSize;

    hr = _AudioClient->GetService(IID_PPV_ARGS(&_RenderClient));
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to get new render client: %x.\n", hr);
        return false;
    }

 
    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////
//
//  That buffer duration is calculated as being PERIODS_PER_BUFFER x the
//  periodicity, so each period we're going to see 1/PERIODS_PER_BUFFERth 
//  the size of the buffer.
//
UINT32 CWASAPIRenderer::BufferSizePerPeriod()
{
    return _BufferSize / PERIODS_PER_BUFFER;
}
////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Retrieve the format we'll use to rendersamples.
//
//  Start with the mix format and see if the endpoint can render that.  If not, try
//  the mix format converted to an integer form (most audio solutions don't support floating 
//  point rendering and the mix format is usually a floating point format).
//
bool CWASAPIRenderer::LoadFormat()
{
    HRESULT hr = _AudioClient->GetMixFormat(&_MixFormat);
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to get mix format on audio client: %x.\n", hr);
        return false;
    }
    assert(_MixFormat != NULL);

    hr = _AudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, _MixFormat, NULL);
    if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT)
    {
        ATLTRACE2(L"Device does not natively support the mix format, converting to PCM.\n");

        //
        //  If the mix format is a float format, just try to convert the format to PCM.
        //
        if (_MixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            _MixFormat->wFormatTag = WAVE_FORMAT_PCM;
            _MixFormat->wBitsPerSample = 16;
            _MixFormat->nBlockAlign = (_MixFormat->wBitsPerSample / 8) * _MixFormat->nChannels;
            _MixFormat->nAvgBytesPerSec = _MixFormat->nSamplesPerSec * _MixFormat->nBlockAlign;
        }
        else if (_MixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_MixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
        {
            WAVEFORMATEXTENSIBLE* waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_MixFormat);
            waveFormatExtensible->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            waveFormatExtensible->Format.wBitsPerSample = 16;
            waveFormatExtensible->Format.nBlockAlign = (_MixFormat->wBitsPerSample / 8) * _MixFormat->nChannels;
            waveFormatExtensible->Format.nAvgBytesPerSec = waveFormatExtensible->Format.nSamplesPerSec * waveFormatExtensible->Format.nBlockAlign;
            waveFormatExtensible->Samples.wValidBitsPerSample = 16;
        }
        else
        {
            ATLTRACE2(L"Mix format is not a floating point format.\n");
            return false;
        }

        hr = _AudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, _MixFormat, NULL);
        if (FAILED(hr))
        {
            ATLTRACE2(L"Format is not supported \n");
            return false;
        }
    }

    _FrameSize = _MixFormat->nBlockAlign;
    if (!CalculateMixFormatType())
    {
        return false;
    }
    return true;
}

//
//  Crack open the mix format and determine what kind of samples are being rendered.
//
bool CWASAPIRenderer::CalculateMixFormatType()
{
    if (_MixFormat->wFormatTag == WAVE_FORMAT_PCM ||
        _MixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
        reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_MixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
    {
        if (_MixFormat->wBitsPerSample == 16)
        {
            _RenderSampleType = SampleType16BitPCM;
        }
        else
        {
            ATLTRACE2(L"Unknown PCM integer sample type\n");
            return false;
        }
    }
    else if (_MixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
        (_MixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_MixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
    {
        _RenderSampleType = SampleTypeFloat;
    }
    else
    {
        ATLTRACE2(L"unrecognized device format.\n");
        return false;
    }
    return true;
}
//--------------------------------------------------------------//
//
//  Initialize the renderer.
//
bool CWASAPIRenderer::Initialize(UINT32 EngineLatency)
{
    //
    //  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
    //
    _ShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_ShutdownEvent == NULL)
    {
        ATLTRACE2(L"Unable to create shutdown event: %d.\n", GetLastError());
        return false;
    }


    //
    //  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
    //
    HRESULT hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void**>(&_AudioClient));
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to activate audio client: %x.\n", hr);
        return false;
    }

    //
    // Load the MixFormat.  This may differ depending on the shared mode used
    //
    if (!LoadFormat())
    {
        ATLTRACE2(L"Failed to load the mix format \n");
        return false;
    }

    //
    //  Remember our configured latency in case we'll need it for a stream switch later.
    //
    _EngineLatencyInMS = EngineLatency;

    if (!InitializeAudioEngine())
    {
        return false;
    }

    return true;
}
//--------------------------------------------------------------//
//
//  Shut down the render code and free all the resources.
//
void CWASAPIRenderer::Shutdown()
{
    if (_RenderThread)
    {
        SetEvent(_ShutdownEvent);
        WaitForSingleObject(_RenderThread, INFINITE);
        CloseHandle(_RenderThread);
        _RenderThread = NULL;
    }

    if (_ShutdownEvent)
    {
        CloseHandle(_ShutdownEvent);
        _ShutdownEvent = NULL;
    }

    SafeRelease(&_Endpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_RenderClient);

    if (_MixFormat)
    {
        CoTaskMemFree(_MixFormat);
        _MixFormat = NULL;
    }
}
//--------------------------------------------------------------//
bool CWASAPIRenderer::Start()
{
    HRESULT hr = S_OK;
     
    //
    //  Now create the thread which is going to drive the renderer.
    //
    _RenderThread = CreateThread(NULL, 0, WASAPIRenderThread, this, 0, NULL);
    if (_RenderThread == NULL)
    {
        ATLTRACE2(L"Unable to create transport thread: %x.", GetLastError());
        return false;
    }

    //
    //  We're ready to go, start rendering!
    //
    hr = _AudioClient->Start();
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to start render client: %x.\n", hr);
        return false;
    }

    return true;
}
//--------------------------------------------------------------//
//
//  Stop the renderer.
//
void CWASAPIRenderer::Stop()
{
    HRESULT hr;

    //
    //  Tell the render thread to shut down, wait for the thread to complete then clean up all the stuff we 
    //  allocated in Start().
    //
    if (_ShutdownEvent)
    {
        SetEvent(_ShutdownEvent);
    }

    hr = _AudioClient->Stop();
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to stop audio client: %x\n", hr);
    }

    if (_RenderThread)
    {
        WaitForSingleObject(_RenderThread, INFINITE);

        CloseHandle(_RenderThread);
        _RenderThread = NULL;
    }

    //
    //  Drain the buffers in the render buffer queue.
    //



}
//--------------------------------------------------------------//
//
//  Render thread - processes samples from the audio engine
//
DWORD CWASAPIRenderer::WASAPIRenderThread(LPVOID Context)
{
    CWASAPIRenderer* renderer = static_cast<CWASAPIRenderer*>(Context);
    return renderer->DoRenderThread();
}
//--------------------------------------------------------------//
void CWASAPIRenderer::SetNearestSample(BOOL bNearestSample)
{
    InterlockedExchange(&m_nNearestSample, bNearestSample ? 1 : 0);
}
//--------------------------------------------------------------//
void CWASAPIRenderer::EnableLowPassFilter(BOOL bLowPassFilter)
{
    InterlockedExchange(&m_nLowPassFilter, bLowPassFilter ? 1 : 0);
} 
//--------------------------------------------------------------//
void CWASAPIRenderer::SetBuffers(WORD* pPCMDataL, WORD* pPCMDataR, WORD* pPCMFilteredDataL, WORD* pPCMFilteredDataR, int nSize)
{
    m_Lock.Lock();
    if (m_pPCMDataL)
    {
        delete[] m_pPCMDataL;
    }
    if (m_pPCMDataR)
    {
        delete[] m_pPCMDataR;
    }
    if (m_pPCMFilteredDataL)
    {
        delete[] m_pPCMFilteredDataL;
    }
    if (m_pPCMFilteredDataR)
    {
        delete[] m_pPCMFilteredDataR;
    }
    m_pPCMFilteredDataL = pPCMFilteredDataL;
    m_pPCMFilteredDataR = pPCMFilteredDataR;
    m_pPCMDataL = pPCMDataL;
    m_pPCMDataR = pPCMDataR;
    m_nPCMBufferSize = nSize;
    m_nPosition = 0;
    m_nLastPosition = 0;
    m_Lock.Unlock();
}
//--------------------------------------------------------------//
DWORD CWASAPIRenderer::DoRenderThread()
{
    bool stillPlaying = true;
    HANDLE waitArray[1] = { _ShutdownEvent };
    HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;
    double InitialTheta = 0;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        ATLTRACE2(L"Unable to initialize COM in render thread: %x\n", hr);
        return hr;
    }

    //
    //  We want to make sure that our timer resolution is a multiple of the latency, otherwise the system timer cadence will
    //  cause us to starve the renderer.
    //
    //  Set the system timer to 1ms as a worst case value.
    //
    timeBeginPeriod(1);

    if (false)//!DisableMMCSS)
    {
        mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
        if (mmcssHandle == NULL)
        {
            ATLTRACE2(L"Unable to enable MMCSS on render thread: %d\n", GetLastError());
        }
    }
 
    while (stillPlaying)
    {
        HRESULT hr;
        //
        //  When running in timer mode, wait for half the configured latency.
        //
        DWORD waitResult = WaitForMultipleObjects(1, waitArray, FALSE, _EngineLatencyInMS / 2);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_TIMEOUT:          // Timeout
            //
            //  We need to provide the next buffer of samples to the audio renderer.  If we're done with our samples, we're done.
            //
           // if (_RenderBufferQueue == NULL)
            {
                //      stillPlaying = false;
            }
            //  else
        {
            BYTE* pData;
            UINT32 padding;
            UINT32 framesAvailable;

            //
            //  We want to find out how much of the buffer *isn't* available (is padding).
            //
            hr = _AudioClient->GetCurrentPadding(&padding);
            if (SUCCEEDED(hr))
            {
                //
                //  Calculate the number of frames available.  We'll render
                //  that many frames or the number of frames left in the buffer, whichever is smaller.
                //
                framesAvailable = _BufferSize - padding;

                //
                //  If the buffer at the head of the render buffer queue fits in the frames available, render it.  If we don't
                //  have enough room to fit the buffer, skip this pass - we will have enough room on the next pass.
                //
                double theta = InitialTheta;
                
                m_Lock.Lock();
                while (m_pPCMDataL && m_pPCMDataR && m_BufferLength <= (framesAvailable * _FrameSize))
                {
                    UINT32 framesToWrite = m_BufferLength / _FrameSize;
                    
                    hr = _RenderClient->GetBuffer(framesToWrite, &pData);
                    if (SUCCEEDED(hr))
                    {
                        long nNumChannels = 2; 
                        for (int h = 0; h < framesToWrite * _FrameSize / 2; h += nNumChannels)
                        { 
                            short* pS = (short*)((short*)pData + h);
                            WORD wLeft = 0;
                            WORD wRight = 0;
                            short w1 = 0;
                            short w2 = 0;
                            if (m_nSpeed == NORMAL_SPEED)//normal speed
                            {
                                wLeft  = m_pPCMDataL[m_nPosition];
                                wRight = m_pPCMDataR[m_nPosition]; 
                                m_nLastPosition = (float)m_nPosition;
                            }
                            else if ( m_nSpeed != NORMAL_SPEED)//speed up or slowed down
                            { 
                                WORD* pLeftChannel  = m_pPCMDataL;
                                WORD* pRightChannel = m_pPCMDataR;
                                if (m_nLowPassFilter > 0 && (m_nSpeed > NORMAL_SPEED || m_nSpeed < NORMAL_SPEED))
                                {//if playing back at a faster speed, play the wave that has beeen filtered through the low pass filter to avoid potenmtial aliasing artifacts
                                    pLeftChannel = m_pPCMFilteredDataL;
                                    pRightChannel = m_pPCMFilteredDataL;
                                }
                                wLeft = 0;
                                wRight = 0;
                                float nSpeed = m_nSpeed / (float)NORMAL_SPEED;
                                //if nSpeed is -ve then we are playing the wave form in reverse
                                float nPosition = m_nLastPosition + nSpeed;
                                m_nLastPosition = nPosition;
                                int nPos1 = (int)floor(nPosition);
                                int nPos2 = nPos1 + 1;// ceil(nPosition);
                                if (nSpeed < 0) nPos2 = nPos1 + 1;                                
                                if (nPos1 < m_nPCMBufferSize / 2 && nPos1 > 0 && nPos2 > 0)
                                {
                                    float weight = nPosition - nPos1;
                                    //first the left channel
                                    w1 = (short)pLeftChannel[nPos1];
                                    w2 = (short)pLeftChannel[nPos2];
                                     
                                    wLeft = w1 + (w2 - w1) * weight;
                                    if (m_nNearestSample) {
                                        wLeft = w1;//in this case, do not interpolate, just take the nearest sample
                                    }
                                    //and again for the right channel
                                    w1 = pRightChannel[nPos1];
                                    w2 = pRightChannel[nPos2];
                                    wRight = w1 + (w2 - w1) * weight;
                                    if (m_nNearestSample) {
                                        wRight = w1;
                                    }
                                    m_nPosition = nPos2;
                                }
                                if (m_nPosition > m_nPCMBufferSize / 2)
                                {
                                    m_nPosition = m_nPCMBufferSize / 2;
                                }
                                else if (m_nPosition < 0 || m_nLastPosition < 0)
                                {
                                    m_nPosition = 0;
                                    m_nLastPosition = 0;
                                }  
                            } 

                            

                            if (m_nPosition + 1 < m_nPCMBufferSize / 2)
                            {
                                *pS = wLeft;
                                *(pS + 1) = wRight;
                                m_nPosition = m_nPosition + 1;
                            }
                            else
                            {
                                *pS = 0;
                                *(pS + 1) = 0;
                            }
                            
                        }
                        //if (InitialTheta != NULL)
                        
                        hr = _RenderClient->ReleaseBuffer(framesToWrite, 0);
                        if (!SUCCEEDED(hr))
                        {
                            ATLTRACE2(L"Unable to release buffer: %x\n", hr);
                            stillPlaying = false;
                        }
                    }
                    else
                    {
                        ATLTRACE2(L"Unable to release buffer: %x\n", hr);
                        stillPlaying = false;
                    }
                    //
                    //  We're done with this set of samples, free it.
                    //
                    //delete renderBuffer;

                    //
                    //  Now recalculate the padding and frames available because we've consumed
                    //  some of the buffer.
                    //
                    hr = _AudioClient->GetCurrentPadding(&padding);
                    if (SUCCEEDED(hr))
                    {
                        //
                        //  Calculate the number of frames available.  We'll render
                        //  that many frames or the number of frames left in the buffer, 
                        //  whichever is smaller.
                        //
                        framesAvailable = _BufferSize - padding;
                    }
                    else
                    {
                        ATLTRACE2(L"Unable to get current padding: %x\n", hr);
                        stillPlaying = false;
                    }
                }
                m_Lock.Unlock();
                InitialTheta = theta;
            }
        }
        break;
        }
    }

    //
    //  Unhook from MMCSS.
    //
   // if (!DisableMMCSS)
    //{
   //     AvRevertMmThreadCharacteristics(mmcssHandle);
    //}

    //
    //  Revert the system timer to the previous value.
    //
    timeEndPeriod(1);

    CoUninitialize();
    return 0;
}

void CWASAPIRenderer::SetSpeed(LONG nSpeed)
{
    InterlockedExchange(&m_nSpeed, nSpeed);
}
//
//  IUnknown
//
HRESULT CWASAPIRenderer::QueryInterface(REFIID Iid, void** Object)
{
    if (Object == NULL)
    {
        return E_POINTER;
    }
    *Object = NULL;

    if (Iid == IID_IUnknown)
    {
        *Object = static_cast<IUnknown*>(this);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }
    return S_OK;
}
ULONG CWASAPIRenderer::AddRef()
{
    return InterlockedIncrement(&_RefCount);
}
ULONG CWASAPIRenderer::Release()
{
    ULONG returnValue = InterlockedDecrement(&_RefCount);
    if (returnValue == 0)
    {
        delete this;
    }
    return returnValue;
}
