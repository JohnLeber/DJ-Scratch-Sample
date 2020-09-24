
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

//--------------------------------------------------------------------//
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
//--------------------------------------------------------------------//
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}
//--------------------------------------------------------------------//
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}
//--------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//--------------------------------------------------------------------//
// CDJSampleDlg dialog
CDJSampleDlg::CDJSampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DJSAMPLE_DIALOG, pParent)
{ 
	m_pDevice = 0; 
	m_TargetFrequency = 0; 
    m_pRenderer = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
//--------------------------------------------------------------------//
CDJSampleDlg::~CDJSampleDlg()
{
    if (m_pRenderer)
    {
        m_pRenderer->Stop();
        m_pRenderer->Shutdown();
        SafeRelease(&m_pRenderer);
    }
}
//--------------------------------------------------------------------//
void CDJSampleDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_SLIDER1, m_Slider);
	CDialogEx::DoDataExchange(pDX);
}
//--------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(CDJSampleDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON() 
    ON_EN_CHANGE(IDC_EDIT1, &CDJSampleDlg::OnEnChangeEdit1) 
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_NORMAL_SPEED, &CDJSampleDlg::OnResetSpeed)
    ON_BN_CLICKED(IDC_REWIND2, &CDJSampleDlg::OnBnClickedRewind2)
    ON_BN_CLICKED(IDC_REWIND, &CDJSampleDlg::OnBnClickedRewind)
    ON_BN_CLICKED(IDC_STOP, &CDJSampleDlg::OnBnClickedStop)
    ON_BN_CLICKED(IDC_FAST_FWD, &CDJSampleDlg::OnBnClickedFastFwd)
    ON_BN_CLICKED(IDC_NEAREST_SAMPLE, &CDJSampleDlg::OnBnClickedNearestSample)
END_MESSAGE_MAP()
//--------------------------------------------------------------------//
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
        AfxMessageBox(L"CoInitializeEx failed");
    }

    bool isDefaultDevice;
    ERole role;
    PickDevice(&m_pDevice, &isDefaultDevice, &role);

    m_pRenderer = new CWASAPIRenderer(m_pDevice);

    if (m_pRenderer->Initialize(m_TargetFrequency))
    {
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
        ATLTRACE2(L"Unable to instantiate device enumerator: %x\n", hr);
        retValue = false;
        goto Exit;
    }

    IMMDevice* device = NULL; 
    if (device == NULL)
    {
        ERole deviceRole = eConsole;    // Assume we're using the console role.
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, deviceRole, &device);
        if (FAILED(hr))
        {
            CString strError;
            strError.Format(L"Unable to get default device for role %d: %x", deviceRole, hr);
            AfxMessageBox(strError);
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
//--------------------------------------------------------------------//
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
//--------------------------------------------------------------------//
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
//--------------------------------------------------------------------//
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDJSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//--------------------------------------------------------------------//
void CDJSampleDlg::OnEnChangeEdit1()
{
	CString strPath;
	GetDlgItemText(IDC_EDIT1, strPath);
    if (strPath.Right(4) != L".mp3") {
        AfxMessageBox(L"unrecognized file type");
        return;
    } 

    CWaitCursor wait;
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

        //Intel ipp appears to have deprecated ippsInterleaved... that could have vectorized the following operation        
        //seperate wave into left and right channels
        for (UINT h = 0; h < dwChannelSizeWords; h++)
        {
            pLeft[h] = pSrc[2 * h];
            pRight[h] = pSrc[2 * h + 1];
        }
        vector<short> vLeft, vRight;
        int nDstBitrate = m_pRenderer->SamplesPerSecond();
        //TODO: check if dest and src bitrates are the same, then skip next step

        //resample left and rights channel from 44.1kHz to 44.8 kHz
        ResampleIPP(
            pMP3->GetBitrate(),    // input frequency(most likely 44100)
            nDstBitrate,   // output frequency (most likely 48000)
            (short*)pLeft,      // input pcm file
            dwChannelSizeWords,
            vLeft);

        //resample right channel
        ResampleIPP( pMP3->GetBitrate(), nDstBitrate, (short*)pRight, dwChannelSizeWords, vRight);
        
        long nNewSizeL = vLeft.size();//size is in words
        long nNewSizeR = vRight.size();
        ATLASSERT(nNewSizeL == nNewSizeR);
        BYTE* pPCMDataL = new BYTE[2 * nNewSizeL];//convert size to bytes by multiplying by 2
        BYTE* pPCMDataR = new BYTE[2 * nNewSizeR];
        
        //now recombine left and right channels in interleaved format
        vector<BYTE> vTest;
        for (UINT h = 0; h < nNewSizeL; h++)            
        {
           pPCMDataL[2 * h]     = LOBYTE(vLeft[h]);
           pPCMDataL[2 * h + 1] = HIBYTE(vLeft[h]);
           pPCMDataR[2 * h]     = LOBYTE(vRight[h]);
           pPCMDataR[2 * h + 1] = HIBYTE(vRight[h]);
        } 
        
        //Pass new wave to audio renderer       
        m_pRenderer->SetBuffers(pPCMDataL, pPCMDataR, vLeft.size() * 2);
        OnResetSpeed();//reset play speed to normal speed

        delete[] pLeft;
        delete[] pRight;
    }
    delete pMP3;
}
//--------------------------------------------------------------------//
size_t CDJSampleDlg::ReadBytes(void* pBuffer, long nNumWords, short* pSrc, DWORD dwSrcSizeInWords, long& nCurrentPos)
{
    size_t n;
    n = min(nNumWords, dwSrcSizeInWords - nCurrentPos);
    memcpy(pBuffer, ((BYTE*)pSrc) + nCurrentPos * 2, n * 2);
    nCurrentPos = nCurrentPos + n;
    return n;
}
//--------------------------------------------------------------------//
void CDJSampleDlg::ResampleIPP(
    int      inRate,    // input frequency
    int      outRate,   // output frequency
    short* pSrc,      // input pcm file
    DWORD dwSrcSize,
    vector<short>& vOut)     // output pcm file
{
    if (inRate <= 0 || outRate <= 0)
    {
        ATLASSERT(!L"inRate <= 0 || outRate <= 0");
        return;
    }
    long nEstimatedOutSize = dwSrcSize * ((float)(outRate + 1) / (float)inRate);
    vOut.reserve(nEstimatedOutSize);//reduce dynamic memory allocations
    //This code is based on the sample from https://software.intel.com/content/www/us/en/develop/documentation/ipp-dev-reference/top/volume-1-signal-and-data-processing/filtering-functions/filtering-functions-1/polyphase-resampling-functions/resamplepolyphasegetfixedfilter.html
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
  
    long nCurrentPos = 0;
    while ((inLen = ReadBytes(inBuf + lastread, bufsize - lastread, pSrc, dwSrcSize, nCurrentPos)) > 0) 
    {
        inCount += inLen;
        lastread += inLen;
        ippsResamplePolyphaseFixed_16s(inBuf, lastread - history - (int)time,
            outBuf, 0.98f, &time, &outLen, state); 

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

    int nCurrSize = vOut.size();
    vOut.resize(nCurrSize + outLen);
    memcpy(&vOut[nCurrSize], (BYTE*)outBuf, outLen * 2);

    outCount += outLen;
    ATLTRACE2(L"%d inputs resampled to %d outputs\n", inCount, outCount);
    ippsFree(outBuf);
    ippsFree(inBuf);
    ippsFree(state);
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
void CDJSampleDlg::OnResetSpeed()
{
    m_Slider.SetPos(NORMAL_SPEED);
    m_pRenderer->SetSpeed(NORMAL_SPEED);
}
//--------------------------------------------------------------------//
void CDJSampleDlg::OnBnClickedRewind2()
{
    m_Slider.SetPos(-2 * NORMAL_SPEED);
    m_pRenderer->SetSpeed(-2 * NORMAL_SPEED);
}
//--------------------------------------------------------------------//
void CDJSampleDlg::OnBnClickedRewind()
{
    m_Slider.SetPos(-NORMAL_SPEED);
    m_pRenderer->SetSpeed(-NORMAL_SPEED);
}
//--------------------------------------------------------------------//
void CDJSampleDlg::OnBnClickedStop()
{
    m_Slider.SetPos(0);
    m_pRenderer->SetSpeed(0);
}
//--------------------------------------------------------------------//
void CDJSampleDlg::OnBnClickedFastFwd()
{
    m_Slider.SetPos(2 * NORMAL_SPEED);
    m_pRenderer->SetSpeed(2 * NORMAL_SPEED);
}
//--------------------------------------------------------------------//
void CDJSampleDlg::OnBnClickedNearestSample()
{
    m_pRenderer->SetNearestSample(IsDlgButtonChecked(IDC_NEAREST_SAMPLE) == BST_CHECKED);
}
