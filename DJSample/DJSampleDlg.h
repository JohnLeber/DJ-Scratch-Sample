
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
	~CDJSampleDlg();
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DJSAMPLE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon; 
	IMMDevice* m_pDevice; 
	int m_TargetFrequency; 
	bool PickDevice(IMMDevice** DeviceToUse, bool* IsDefaultDevice, ERole* DefaultDeviceRole); 
	CWASAPIRenderer* m_pRenderer;
	CMFCEditBrowseCtrl m_LoadFileEdit;
	 
	CSliderCtrl m_Slider;
	void OnSlider();
	void ResampleIPP(
		int      inRate,    // input frequency
		int      outRate,   // output frequency
		short* pSrc,      // input pcm file
		int dwSrcSize,
		vector<short>& vOut);
	int ReadBytes(void* pBuffer, int nNumWords, short* pSrc, int dwSrcSizeInWords, int& nCurrentPos);
 
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public: 
	afx_msg void OnEnChangeMP3Path();
	afx_msg void OnNMCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnResetSpeed();
	afx_msg void OnBnClickedRewind2();
	afx_msg void OnBnClickedRewind();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedFastFwd();
	afx_msg void OnBnClickedNearestSample();
	afx_msg void OnBnClickedLowPass();
};
