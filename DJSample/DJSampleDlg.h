
// DJSampleDlg.h : header file
//

#pragma once
#include "soundrenderer.h"
class CWASAPIRenderer;

// CDJSampleDlg dialog
class CDJSampleDlg : public CDialogEx
{
// Construction
public:
	CDJSampleDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DJSAMPLE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CSharedBuffer m_Buffer;
	IMMDevice* m_pDevice;
	IMMDevice* m_pCaptureDevice;
	int m_TargetFrequency;
	int m_TargetLatency;
	int m_TargetDurationInSec;
	bool PickDevice(IMMDevice** DeviceToUse, bool* IsDefaultDevice, ERole* DefaultDeviceRole); 
	CWASAPIRenderer* m_pRenderer;
	CMFCEditBrowseCtrl m_LoadFileEdit;
	//void resampleIPP(
	//	int      inRate,    // input frequency
	//	int      outRate,   // output frequency
	//	short* pSrc,      // input pcm file
	//	DWORD dwSrcSize,
	//	vector<short>& vOut);

	//void resampleIPPFile(
	//	int      inRate,    // input frequency
	//	int      outRate,   // output frequency
	//	FILE* infd,      // input pcm file
	//	FILE* outfd);     // output pcm file
	//
	//void resampleIPPFile2(
	//	int      inRate,    // input frequency
	//	int      outRate,   // output frequency
	//	FILE* infd,      // input pcm file
	//	vector<short>& vOut);     // output pcm file
	CSliderCtrl m_Slider;
	void OnSlider();
	void resampleIPPFile(
		int      inRate,    // input frequency
		int      outRate,   // output frequency
		short* pSrc,      // input pcm file
		DWORD dwSrcSize,
		vector<short>& vOut);
	size_t ReadMem(void* pBuffer, long nNumWords, short* pSrc, DWORD dwSrcSizeInWords, long& nCurrentPos);
	//CAudioBuffer m_AudioBuffer;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public: 
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnNMCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton1();
};
