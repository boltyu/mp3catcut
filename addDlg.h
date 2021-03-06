
// addDlg.h: 头文件
//

#pragma once


// CaddDlg 对话框
class CaddDlg : public CDialogEx
{
// 构造
public:
	CaddDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDropFiles(HDROP dropInfo);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedCut();
	afx_msg void OnLbnSelchangeMlist();
	afx_msg void OnNMCustomdrawTimezone(NMHDR *pNMHDR, LRESULT *pResult);
	float GetDlgTime(int nID);
	void SetDlgTime(int nID, float newtime);
	afx_msg void OnStnClickedTimebegin();
	afx_msg void OnLbnDblclkMlist();
};
