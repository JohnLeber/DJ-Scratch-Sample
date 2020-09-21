#pragma once
#include "pch.h"

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <mmsystem.h>
//#include <amstream.h>
#include <mmreg.h>
#include <msacm.h>
#include <wmsdk.h>
 

class CMP3 {
private:
    HWAVEOUT hWaveOut;
    DWORD bufferLength;
    double durationInSecond;
    BYTE* soundBuffer;
    DWORD m_dwBitrate;
public:

    HRESULT OpenFromFile(TCHAR* inputFileName);

    int GetBitrate();
    HRESULT OpenFromMemory(BYTE* mp3InputBuffer, DWORD mp3InputBufferSize);

    void SetBufferPtr(BYTE** ppSoundBuffer, DWORD& dwBufferLength);
    void __inline Close();


    double __inline GetDuration();
    double GetPosition();
    void Play();
};