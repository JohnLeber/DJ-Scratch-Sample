
// DJSampleDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "DJSample.h"
#include "DJSampleDlg.h"
#include "afxdialogex.h"

#include "mp3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDJSampleDlg dialog



CDJSampleDlg::CDJSampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DJSAMPLE_DIALOG, pParent)
{

	//CSharedBuffer m_Buffer;
	m_pDevice = 0;
	m_pCaptureDevice = 0;
	m_TargetFrequency = 0;
	m_TargetLatency = 0;
	m_TargetDurationInSec = 0;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDJSampleDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_SLIDER1, m_Slider);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDJSampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON() 
	ON_EN_CHANGE(IDC_EDIT1, &CDJSampleDlg::OnEnChangeEdit1)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CDJSampleDlg::OnNMCustomdrawSlider1)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_BUTTON1, &CDJSampleDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDJSampleDlg message handlers

BOOL CDJSampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_LoadFileEdit.SubclassDlgItem(IDC_EDIT1, this);
	m_LoadFileEdit.EnableFileBrowseButton(NULL, _T("MP3 Files (*.mp3)|*.mp3|All Files (*.*)|*.*||"));

    m_Slider.SetRange(-100, 100);
    m_Slider.SetPos(50);
   

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{


	}

	bool isDefaultDevice;
	ERole role;
	PickDevice(&m_pDevice, &isDefaultDevice, &role);

	m_pRenderer = new CWASAPIRenderer(m_pDevice);
	if (m_pRenderer == NULL)
	{
		printf("Unable to allocate renderer\n");
		return -1;
	}
    //m_pRenderer->m_pAudioBuffer = &m_AudioBuffer;
	if (m_pRenderer->Initialize(m_TargetFrequency))
	{
        m_pRenderer->m_nMix = 50 / 100.0f;
        m_pRenderer->m_nAmount = 50 / 100.0f;
        if (m_pRenderer->Start())
        {

        }
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}
//--------------------------------------------------------------------//
bool CDJSampleDlg::PickDevice(IMMDevice** DeviceToUse, bool* IsDefaultDevice, ERole* DefaultDeviceRole)
{
    HRESULT hr;
    bool retValue = true;
    IMMDeviceEnumerator* deviceEnumerator = NULL;
    IMMDeviceCollection* deviceCollection = NULL;

    *IsDefaultDevice = false;   // Assume we're not using the default device.

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
        retValue = false;
        goto Exit;
    }

    IMMDevice* device = NULL;

    ////
    ////  First off, if none of the console switches was specified, use the console device.
    ////
    //if (!UseConsoleDevice && !UseCommunicationsDevice && !UseMultimediaDevice && OutputEndpoint == NULL)
    //{
    //    //
    //    //  The user didn't specify an output device, prompt the user for a device and use that.
    //    //
    //    hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
    //    if (FAILED(hr))
    //    {
    //        printf("Unable to retrieve device collection: %x\n", hr);
    //        retValue = false;
    //        goto Exit;
    //    }

    //    printf("Select an output device:\n");
    //    printf("    0:  Default Console Device\n");
    //    printf("    1:  Default Communications Device\n");
    //    printf("    2:  Default Multimedia Device\n");
    //    UINT deviceCount;
    //    hr = deviceCollection->GetCount(&deviceCount);
    //    if (FAILED(hr))
    //    {
    //        printf("Unable to get device collection length: %x\n", hr);
    //        retValue = false;
    //        goto Exit;
    //    }
    //    for (UINT i = 0 ; i < deviceCount ; i += 1)
    //    {
    //        LPWSTR deviceName;

    //        deviceName = GetDeviceName(deviceCollection, i);
    //        if (deviceName == NULL)
    //        {
    //            retValue = false;
    //            goto Exit;
    //        }
    //        printf("    %d:  %S\n", i + 3, deviceName);
    //        free(deviceName);
    //    }
    //    wchar_t choice[10];
    //    _getws_s(choice);   // Note: Using the safe CRT version of _getws.

    //    long deviceIndex;
    //    wchar_t *endPointer;

    //    deviceIndex = wcstoul(choice, &endPointer, 0);
    //    if (deviceIndex == 0 && endPointer == choice)
    //    {
    //        printf("unrecognized device index: %S\n", choice);
    //        retValue = false;
    //        goto Exit;
    //    }
    //    switch (deviceIndex)
    //    {
    //    case 0:
    //        UseConsoleDevice = 1;
    //        break;
    //    case 1:
    //        UseCommunicationsDevice = 1;
    //        break;
    //    case 2:
    //        UseMultimediaDevice = 1;
    //        break;
    //    default:
    //        hr = deviceCollection->Item(deviceIndex - 3, &device);
    //        if (FAILED(hr))
    //        {
    //            printf("Unable to retrieve device %d: %x\n", deviceIndex - 3, hr);
    //            retValue = false;
    //            goto Exit;
    //        }
    //        break;
    //    }
    //} 
    //else if (OutputEndpoint != NULL)
    //{
    //    hr = deviceEnumerator->GetDevice(OutputEndpoint, &device);
    //    if (FAILED(hr))
    //    {
    //        printf("Unable to get endpoint for endpoint %S: %x\n", OutputEndpoint, hr);
    //        retValue = false;
    //        goto Exit;
    //    }
    //}

    if (device == NULL)
    {
        ERole deviceRole = eConsole;    // Assume we're using the console role.
       /* if (UseConsoleDevice)
        {
            deviceRole = eConsole;
        }
        else if (UseCommunicationsDevice)
        {
            deviceRole = eCommunications;
        }
        else if (UseMultimediaDevice)
        {
            deviceRole = eMultimedia;
        }*/
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, deviceRole, &device);
        if (FAILED(hr))
        {
            printf("Unable to get default device for role %d: %x\n", deviceRole, hr);
            retValue = false;
            goto Exit;
        }
        *IsDefaultDevice = true;
        *DefaultDeviceRole = deviceRole;
    }

    *DeviceToUse = device;
    retValue = true;
Exit:
    SafeRelease(&deviceCollection);
    SafeRelease(&deviceEnumerator);

    return retValue;
}

void CDJSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDJSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDJSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
#include <ippdc.h>
void CDJSampleDlg::OnEnChangeEdit1()
{
	CString strPath;
	GetDlgItemText(IDC_EDIT1, strPath);
	if (strPath.Right(4) != L".mp3") return;
	

	CMP3* pMP3 = new CMP3();
    if (SUCCEEDED(pMP3->OpenFromFile(strPath.GetBuffer(0))))
    {
        BYTE* pPCMData = 0;
        DWORD dwPCMBufferSize = 0;
        pMP3->SetBufferPtr(&pPCMData, dwPCMBufferSize);
        DWORD dwChannelSizeWords = dwPCMBufferSize / 2 / 2;//size in words for each channel
        WORD* pLeft = new WORD[dwChannelSizeWords];
        WORD* pRight = new WORD[dwChannelSizeWords];
        WORD* pSrc = (WORD*)pPCMData;
        //Intel ipp has deprecated ippsInterleaved (and the entire ac domain) that could have vectorized the following operation        
        for (UINT h = 0; h < dwChannelSizeWords; h++)
        {
            pLeft[h] = pSrc[2 * h];
            pRight[h] = pSrc[2 * h + 1];
        }
        vector<short> vLeft, vRight;
        int nDstBitrate = m_pRenderer->SamplesPerSecond();
        resampleIPPFile(
            pMP3->GetBitrate(),    // input frequency
            nDstBitrate,   // output frequency
            (short*)pLeft,      // input pcm file
            dwChannelSizeWords,
            vLeft);

        resampleIPPFile(
            pMP3->GetBitrate(),    // input frequency
            nDstBitrate,   // output frequency
            (short*)pRight,      // input pcm file
            dwChannelSizeWords,
            vRight);
        
        long nNewSize1 = vLeft.size();
        long nNewSize2 = vRight.size();
        ATLASSERT(nNewSize1 == nNewSize2);
        m_pRenderer->m_pPCMDataL = new BYTE[2 * nNewSize1];
        m_pRenderer->m_pPCMDataR = new BYTE[2 * nNewSize1];
        vector<BYTE> vTest;
        for (UINT h = 0; h < nNewSize1; h++)            
        {
            m_pRenderer->m_pPCMDataL[2 * h]     = LOBYTE(vLeft[h]);
            m_pRenderer->m_pPCMDataL[2 * h + 1] = HIBYTE(vLeft[h]);
            m_pRenderer->m_pPCMDataR[2 * h]     = LOBYTE(vRight[h]);
            m_pRenderer->m_pPCMDataR[2 * h + 1] = HIBYTE(vRight[h]);
        } 
        m_pRenderer->m_dwPCMBufferSize = vLeft.size() * 2;
        //pMP3->SetBufferPtr(&m_pRenderer->m_pPCMData, m_pRenderer->m_dwPCMBufferSize);
        m_pRenderer->m_nPosition = 0;
    }
    delete pMP3;

	//pMP3->Play();

}
size_t CDJSampleDlg::ReadMem(void* pBuffer, long nNumWords, short* pSrc, DWORD dwSrcSizeInWords, long& nCurrentPos)
{
    size_t n;
    n = min(nNumWords, dwSrcSizeInWords - nCurrentPos);
    memcpy(pBuffer, ((BYTE*)pSrc) + nCurrentPos * 2, n * 2);
    nCurrentPos = nCurrentPos + n;
    return n;
}
void CDJSampleDlg::resampleIPPFile(
    int      inRate,    // input frequency
    int      outRate,   // output frequency
    short* pSrc,      // input pcm file
    DWORD dwSrcSize,
    vector<short>& vOut)     // output pcm file
{
    //based on https://software.intel.com/content/www/us/en/develop/documentation/ipp-dev-reference/top/volume-1-signal-and-data-processing/filtering-functions/filtering-functions-1/polyphase-resampling-functions/resamplepolyphasegetfixedfilter.html
    short* inBuf, * outBuf;
    int bufsize = 4096;
    int history = 128;
    double time = history;
    int lastread = history;
    int inCount = 0, outCount = 0, inLen, outLen;
    int size, len, height;
    IppsResamplingPolyphaseFixed_16s* state;
    ippsResamplePolyphaseFixedGetSize_16s(inRate, outRate, 2 * (history - 1), &size, &len, &height, ippAlgHintAccurate);
    state = (IppsResamplingPolyphaseFixed_16s*)ippsMalloc_8u(size);
    ippsResamplePolyphaseFixedInit_16s(inRate, outRate, 2 * (history - 1), 0.95f, 9.0f, state, ippAlgHintAccurate);
    inBuf = ippsMalloc_16s(bufsize + history + 2);
    outBuf = ippsMalloc_16s((int)((bufsize - history) * outRate / (float)inRate + 2));
    ippsZero_16s(inBuf, history);
 
    //while ((inLen = fread(inBuf + lastread, sizeof(short), bufsize - lastread, infd)) > 0) {
    long nCurrentPos = 0;
    while ((inLen = ReadMem(inBuf + lastread, bufsize - lastread, pSrc, dwSrcSize, nCurrentPos)) > 0) {
        inCount += inLen;
        lastread += inLen;
        ippsResamplePolyphaseFixed_16s(inBuf, lastread - history - (int)time,
            outBuf, 0.98f, &time, &outLen, state);
        //fwrite(outBuf, outLen, sizeof(short), outfd);

        int nCurrSize = vOut.size();
        vOut.resize(nCurrSize + outLen);
        memcpy(&vOut[nCurrSize], (BYTE*)outBuf, outLen * 2);

        outCount += outLen;
        ippsMove_16s(inBuf + (int)time - history, inBuf, lastread + history - (int)time);
        lastread -= (int)time - history;
        time -= (int)time - history;
    }
    ippsZero_16s(inBuf + lastread, history);
    ippsResamplePolyphaseFixed_16s(inBuf, lastread - (int)time,
        outBuf, 0.98f, &time, &outLen, state);
    //fwrite(outBuf, outLen, sizeof(short), outfd);

    int nCurrSize = vOut.size();
    vOut.resize(nCurrSize + outLen);
    memcpy(&vOut[nCurrSize], (BYTE*)outBuf, outLen * 2);

    outCount += outLen;
    printf("%d inputs resampled to %d outputs\n", inCount, outCount);
    ippsFree(outBuf);
    ippsFree(inBuf);
    ippsFree(state);
}
//
//
//SIZE_T MemoryRead(short* pDest, short* pSrc, SIZE_T size)
//{
//    CopyMemory((BYTE*)pDest, (BYTE*)pSrc, size * 2);
//    pSrc += size * 2;
//
//    return size;
//}
//void CDJSampleDlg::resampleIPP(
//    int      inRate,    // input frequency
//    int      outRate,   // output frequency
//    short* pSrc,      // input pcm file
//    DWORD dwSrcSize,
//    vector<short>& vOut)
//{
//
//    resampleIPPFile3(inRate, outRate, pSrc, dwSrcSize, vOut);
//    return;
//    FILE* pFile;
//    pFile = fopen("C:\\Users\\user\\source\\repos\\DJSample\\_infile.bin", "wb");
//    if (pFile != NULL)
//    {
//        fwrite(pSrc, 2, dwSrcSize, pFile);
//        fclose(pFile);
//    }
//
//    FILE* pOutFile;
//    pOutFile = fopen("C:\\Users\\user\\source\\repos\\DJSample\\_outfile.bin", "wb");
//    if (pOutFile != NULL)
//    {
//        FILE* pFile2 = fopen("C:\\Users\\user\\source\\repos\\DJSample\\_infile.bin", "rb");
//        resampleIPPFile2(inRate, outRate, pFile2, vOut);
//        fclose(pOutFile);
//        fclose(pFile2);
//        return;
//    }
//
//
//    short* inBuf, * outBuf;
//    int bufsize = 4096;
//    int history = 128;
//    double time = history;
//    int lastread = history;
//    int inCount = 0, outCount = 0, inLen = 0, outLen;
//    int size, len, height;
//    IppsResamplingPolyphaseFixed_16s* state;
//    ippsResamplePolyphaseFixedGetSize_16s(inRate, outRate, 2 * (history - 1), &size, &len, &height, ippAlgHintAccurate);
//    state = (IppsResamplingPolyphaseFixed_16s*)ippsMalloc_8u(size);
//    ippsResamplePolyphaseFixedInit_16s(inRate, outRate, 2 * (history - 1), 0.95f, 9.0f, state, ippAlgHintAccurate);
//    inBuf = ippsMalloc_16s(bufsize + history + 2);
//    int nSize = (int)((bufsize - history) * outRate / (float)inRate + 2);
//    outBuf = ippsMalloc_16s(nSize);// (int)((bufsize - history) * outRate / (float)inRate + 2));
//
//    ippsZero_16s(inBuf, history);
//    //while ((inLen = fread(inBuf + lastread, sizeof(short), bufsize - lastread, infd)) > 0) {
//    //for(int h = 0; h < dwSrcSize / bufsize; h++)
//    {
//        inLen = bufsize - lastread;
//        inCount += inLen;
//        lastread += inLen;
//        ippsResamplePolyphaseFixed_16s(inBuf, lastread - history - (int)time, outBuf, 0.98f, &time, &outLen, state);
//        //fwrite(outBuf, outLen, sizeof(short), outfd);
//        int nCurrSize = vOut.size();
//        vOut.resize(nCurrSize + outLen);
//        memcpy(&vOut[nCurrSize], (BYTE*)outBuf, outLen * 2);
//
//        outCount += outLen;
//        ippsMove_16s(inBuf + (int)time - history, inBuf, lastread + history - (int)time);
//        lastread -= (int)time - history;
//        time -= (int)time - history;
//    }
//    ippsZero_16s(inBuf + lastread, history);
//    ippsResamplePolyphaseFixed_16s(inBuf, lastread - (int)time, outBuf, 0.98f, &time, &outLen, state);
//
//    /*  int nCurrSize = vOut.size();
//        vOut.resize(nCurrSize + outLen);
//        memcpy(&vOut[nCurrSize], outBuf, outLen);*/
//
//        //fwrite(outBuf, outLen, sizeof(short), outfd);
//    outCount += outLen;
//    //printf("%d inputs resampled to %d outputs\n", inCount, outCount);
//    ippsFree(outBuf);
//    ippsFree(inBuf);
//    ippsFree(state);
//}
//
//void CDJSampleDlg::resampleIPPFile(
//    int      inRate,    // input frequency
//    int      outRate,   // output frequency
//    FILE* infd,      // input pcm file
//    FILE* outfd)     // output pcm file
//{
//    short* inBuf, * outBuf;
//    int bufsize = 4096;
//    int history = 128;
//    double time = history;
//    int lastread = history;
//    int inCount = 0, outCount = 0, inLen, outLen;
//    int size, len, height;
//    IppsResamplingPolyphaseFixed_16s* state;
//    ippsResamplePolyphaseFixedGetSize_16s(inRate, outRate, 2 * (history - 1), &size, &len, &height, ippAlgHintAccurate);
//    state = (IppsResamplingPolyphaseFixed_16s*)ippsMalloc_8u(size);
//    ippsResamplePolyphaseFixedInit_16s(inRate, outRate, 2 * (history - 1), 0.95f, 9.0f, state, ippAlgHintAccurate);
//    inBuf = ippsMalloc_16s(bufsize + history + 2);
//    outBuf = ippsMalloc_16s((int)((bufsize - history) * outRate / (float)inRate + 2));
//    ippsZero_16s(inBuf, history);
//    fseek(infd, 0, SEEK_END);
//    long fsize = ftell(infd);
//    fseek(infd, 0, SEEK_SET);  /* same as rewind(f); */
//    while ((inLen = fread(inBuf + lastread, sizeof(short), bufsize - lastread, infd)) > 0) {
//        inCount += inLen;
//        lastread += inLen;
//        ippsResamplePolyphaseFixed_16s(inBuf, lastread - history - (int)time,
//            outBuf, 0.98f, &time, &outLen, state);
//        fwrite(outBuf, outLen, sizeof(short), outfd);
//        outCount += outLen;
//        ippsMove_16s(inBuf + (int)time - history, inBuf, lastread + history - (int)time);
//        lastread -= (int)time - history;
//        time -= (int)time - history;
//    }
//    ippsZero_16s(inBuf + lastread, history);
//    ippsResamplePolyphaseFixed_16s(inBuf, lastread - (int)time,
//        outBuf, 0.98f, &time, &outLen, state);
//    fwrite(outBuf, outLen, sizeof(short), outfd);
//    outCount += outLen;
//    printf("%d inputs resampled to %d outputs\n", inCount, outCount);
//    ippsFree(outBuf);
//    ippsFree(inBuf);
//    ippsFree(state);
//}
//
//
//void CDJSampleDlg::resampleIPPFile2(
//    int      inRate,    // input frequency
//    int      outRate,   // output frequency
//    FILE* infd,      // input pcm file
//    vector<short>& vOut)     // output pcm file
//{
//    short* inBuf, * outBuf;
//    int bufsize = 4096;
//    int history = 128;
//    double time = history;
//    int lastread = history;
//    int inCount = 0, outCount = 0, inLen, outLen;
//    int size, len, height;
//    IppsResamplingPolyphaseFixed_16s* state;
//    ippsResamplePolyphaseFixedGetSize_16s(inRate, outRate, 2 * (history - 1), &size, &len, &height, ippAlgHintAccurate);
//    state = (IppsResamplingPolyphaseFixed_16s*)ippsMalloc_8u(size);
//    ippsResamplePolyphaseFixedInit_16s(inRate, outRate, 2 * (history - 1), 0.95f, 9.0f, state, ippAlgHintAccurate);
//    inBuf = ippsMalloc_16s(bufsize + history + 2);
//    outBuf = ippsMalloc_16s((int)((bufsize - history) * outRate / (float)inRate + 2));
//    ippsZero_16s(inBuf, history);
//    fseek(infd, 0, SEEK_END);
//    long fsize = ftell(infd);
//    fseek(infd, 0, SEEK_SET);  /* same as rewind(f); */
//    while ((inLen = fread(inBuf + lastread, sizeof(short), bufsize - lastread, infd)) > 0) {
//        inCount += inLen;
//        lastread += inLen;
//        ippsResamplePolyphaseFixed_16s(inBuf, lastread - history - (int)time,
//            outBuf, 0.98f, &time, &outLen, state);
//        //fwrite(outBuf, outLen, sizeof(short), outfd);
//
//        int nCurrSize = vOut.size();
//        vOut.resize(nCurrSize + outLen);
//        memcpy(&vOut[nCurrSize], (BYTE*)outBuf, outLen * 2);
//
//        outCount += outLen;
//        ippsMove_16s(inBuf + (int)time - history, inBuf, lastread + history - (int)time);
//        lastread -= (int)time - history;
//        time -= (int)time - history;
//    }
//    ippsZero_16s(inBuf + lastread, history);
//    ippsResamplePolyphaseFixed_16s(inBuf, lastread - (int)time,
//        outBuf, 0.98f, &time, &outLen, state);
//    //fwrite(outBuf, outLen, sizeof(short), outfd);
//
//    int nCurrSize = vOut.size();
//    vOut.resize(nCurrSize + outLen);
//    memcpy(&vOut[nCurrSize], (BYTE*)outBuf, outLen * 2);
//
//    outCount += outLen;
//    printf("%d inputs resampled to %d outputs\n", inCount, outCount);
//    ippsFree(outBuf);
//    ippsFree(inBuf);
//    ippsFree(state);
//}

void CDJSampleDlg::OnNMCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;
}

//---------------------------------------------------------------//
void CDJSampleDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
    if (pSlider == &m_Slider)
    {
        OnSlider();
    }
    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
//---------------------------------------------------------------//
void CDJSampleDlg::OnSlider()
{
    long nPos = m_Slider.GetPos();
    //-100 fast reverse, 0 stopped, 50 noprmal speed fwd, 100 fast fwd
    long nSpeed = nPos;// *2 - 100;
    if (m_pRenderer)
    {
        ATLTRACE2(L"Speed = %d\n", nSpeed);
        m_pRenderer->SetSpeed(nSpeed);
    }
}
//---------------------------------------------------------------//
void CDJSampleDlg::OnBnClickedButton1()
{
    m_Slider.SetPos(50);
    m_pRenderer->SetSpeed(50);
}
