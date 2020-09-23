# DJ Scratch Sample

DJ hardware controllers allow DJs to emulate traditional vinyl turntables by sending signals from the controller turntable to the PC DJ software in order to alter the speed of the PC audio playback thus emulating "scratching". This technique has given DJs the option of replacing suitcases of vinyl records with a disk of MP3 files. For this to be possible the PC software must be able to dynamically resample the audio signal so that the speed and direction of playback can be controlled. This needs to be done with as little latency as possible. 

This sample application shows how this may be achieved in Microsoft Windows using [WASAPI](https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi) and some help from [Intel's IPP library](https://software.intel.com/content/www/us/en/develop/tools/integrated-performance-primitives.html). The application does the following:
1) allows a user to select an MP3 file. The MP3 file must have exactly two channels (i.e. stereo) and be sampled at 44100Hz.
2) extracts the audio signal from the MP3 in PCM wave format.
3) uses Intel IPP to re-sample the resulting wave file from 44100Hz to the sampling frequency used by WASAPI (normally 48000Hz).
4) passes the 48000 Hz wave to the WASAPI audio renderer that has been opened in shared mode.
5) allows a user to emulate a turntable by using a slider to dynamically control the speed and direction of playback by re-sampling/interpolating the wave just before it is passed to the WASAPI buffer.

**Future work**

1) When the signal is played faster than the original speed then it should really be passed through a low pass filter just before the last step..
2) The slider is moved backwards and forwards rapidly to simulate scratching. When I do this however, it dones't sound authentic to me. Rapidly rotating a real turntable with real vinyl may be producing other "scratchy" or analog sounds other than the actual music. Perhaps these could be recorrded, loaded into the application and then mixed with the existing audio output.

**Requirements**

1) The project was written using Visual studio 2019 (make sure the desktop c++ and MFC options are installed).
2) [Intel's IPP library](https://software.seek.intel.com/performance-libraries) must be installed. The version used here is 2020.2.254

**Credits**

1) The mp3 loading code was primarily written by Alexandre Mutel and is available [here](http://code4k.blogspot.com/2010/05/playing-mp3-in-c-using-plain-windows.html) or [here](https://xoofx.com/blog/2010/05/21/playing-mp3-in-c-using-plain-windows/).
2) The WASAPI code was modified from [here](https://github.com/microsoft/Windows-classic-samples/blob/master/Samples/Win7Samples/multimedia/audio/RenderSharedTimerDriven/WASAPIRenderer.cpp).
