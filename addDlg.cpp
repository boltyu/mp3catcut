
// addDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "add.h"
#include "addDlg.h"
#include "afxdialogex.h"
#include "minimp3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_FRAME		55555
#define MAX_TOTAL_BUFF	300000000
#define MAX_SINGLE_BUFF	50000000
#define MAX_WRITE_TRYS	15

CListBox mlist;
CSliderCtrl mslide;
int sw_focus = 0;

CaddDlg::CaddDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ADD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CaddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MLIST, mlist);
	DDX_Control(pDX, IDC_TIMEZONE, mslide);
}

BEGIN_MESSAGE_MAP(CaddDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDOK, &CaddDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CaddDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_DELETE, &CaddDlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_CUT, &CaddDlg::OnBnClickedCut)
	ON_LBN_SELCHANGE(IDC_MLIST, &CaddDlg::OnLbnSelchangeMlist)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TIMEZONE, &CaddDlg::OnNMCustomdrawTimezone)
	ON_STN_CLICKED(IDC_TIMEBEGIN, &CaddDlg::OnStnClickedTimebegin)
	ON_LBN_DBLCLK(IDC_MLIST, &CaddDlg::OnLbnDblclkMlist)
END_MESSAGE_MAP()

BOOL CaddDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	ShowWindow(SW_SHOW);

	mslide.SetRange(0, 10, 1);

	SetDlgItemTextW(IDC_TIMEBEGIN,_T("0.000"));
	SetDlgItemTextW(IDC_TIMEEND,_T("0.000"));

	
	mlist.SetHorizontalExtent(1111);
	
	DragAcceptFiles();


	return TRUE;  
}

void CaddDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CaddDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CaddDlg::OnBnClickedOk()
{
	int nitem = mlist.GetListBoxInfo(), itemcount = 0, framecount = 0, loopcount = 0, iResult = 0, samples = 0;
	HANDLE hFile;
	CString sFile;
	DWORD lenofread = 0, targetlen = 0;
	OVERLAPPED ol = { 0 };
	unsigned char *newbuf = new unsigned char[MAX_TOTAL_BUFF];
	unsigned char *tmpbuf = new unsigned char[MAX_SINGLE_BUFF];
	unsigned char *endbuf = new unsigned char[500000];
	int alldata = 0, lastendframe = 0, endframelenth = 0, datacount = 0;
	mp3dec_frame_info_t info;
	static mp3dec_t mp3d;
	mp3dec_init(&mp3d);
	// if num of mp3 < 1
	if (nitem < 1)
		return;
	// add 1st mp3's full data to buffer
	mlist.GetText(0, sFile);
	hFile = CreateFile(sFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	if (ReadFile(hFile, newbuf, MAX_TOTAL_BUFF, &lenofread, &ol) == FALSE)
	{
		iResult = GetLastError();
		CloseHandle(hFile);
		return;
	}
	alldata += lenofread;
	for (framecount = 0; framecount < MAX_FRAME; framecount++)
	{
		samples = mp3dec_decode_frame(&mp3d, &newbuf[lastendframe], alldata - lastendframe, 0, &info);
		if (samples == 0)
		{	
			break;
		}
		lastendframe += info.frame_bytes;
	}
	CloseHandle(hFile);
	endframelenth = alldata - lastendframe;
	for (int loopcount = 0; loopcount < endframelenth; loopcount++)
	{
		endbuf[loopcount] = newbuf[lastendframe + loopcount];
	}
	// add all left mp3's frame data to buffer
	for (itemcount = 1; itemcount < nitem; itemcount++)
	{
		mlist.GetText(itemcount, sFile);
		hFile = CreateFile(sFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
		if (ReadFile(hFile, tmpbuf, MAX_SINGLE_BUFF, &lenofread, &ol) == FALSE)
		{
			CloseHandle(hFile);
			return;
		}

		for (framecount = 0; framecount < MAX_FRAME; framecount++)
		{
			samples = mp3dec_decode_frame(&mp3d, &tmpbuf[datacount], lenofread - datacount, 0, &info);
			datacount += info.frame_bytes;
			if(info.frame_bytes < 2000)
			{
				if(samples == 0)
					framecount = MAX_FRAME;
				break;
			}
			
		}
		for (framecount = 0; framecount < MAX_FRAME; framecount++)
		{
  			samples = mp3dec_decode_frame(&mp3d, &tmpbuf[datacount], lenofread - datacount, 0, &info);
			if (samples == 0)
				break;
			for (int loopcount = 0; loopcount < info.frame_bytes; loopcount++)
			{
				newbuf[lastendframe++] = tmpbuf[datacount++];
			}
		}
		datacount = 0;
		CloseHandle(hFile);
	}
	for (loopcount = 0; loopcount < endframelenth; loopcount++)
	{
		newbuf[lastendframe++] = endbuf[loopcount];
	}
	sFile = _T(".\\yuan.mp3");
	for (loopcount = 0; loopcount < MAX_WRITE_TRYS; loopcount++)
	{
		hFile = CreateFile(sFile , GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			sFile.Format(_T(".\\yuan%d.mp3"), loopcount);
		else
			break;
	}
	if (loopcount > 9)
		return;
		
	if (WriteFile(hFile, newbuf, lastendframe, &lenofread, &ol) == FALSE)
	{
		iResult = GetLastError();
		CloseHandle(hFile);
		return;
	}

	CloseHandle(hFile);
	delete(tmpbuf);
	delete(newbuf);
	return;
}

void CaddDlg::OnDropFiles(HDROP dropInfo)
{
	CString sFile;
	DWORD   nBuffer = 0;
	// Get the number of files dropped 
	int nFilesDropped = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, 0);

	for (int i = 0; i < nFilesDropped; i++)
	{
		// Get the buffer size of the file. 
		nBuffer = DragQueryFile(dropInfo, i, NULL, 0);
		// Get path and name of the file 
		DragQueryFile(dropInfo, i, sFile.GetBuffer(nBuffer + 1), nBuffer + 1);
		mlist.InsertString(mlist.GetListBoxInfo(), sFile);
		sFile.ReleaseBuffer();
	}
	// Free the memory block containing the dropped-file information 
	DragFinish(dropInfo);
}


void CaddDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


void CaddDlg::OnBnClickedDelete()
{
	mlist.DeleteString(mlist.GetCurSel());
}


void CaddDlg::OnBnClickedCut()
{
	int headerlen = 0, framecount = 0, datacount = 0, loopcount = 0, samples = 0;
	float starttime = GetDlgTime(IDC_EDITBEGIN),
		endtime = GetDlgTime(IDC_EDITEND),
		fulltime = GetDlgTime(IDC_TIMEEND);
	unsigned char *tmpbuf = new unsigned char[MAX_SINGLE_BUFF];
	unsigned char *newbuf = new unsigned char[MAX_SINGLE_BUFF];
	int *frame_pos = new int[MAX_FRAME];	
	int *frame_len = new int[MAX_FRAME];
	HANDLE hFile;
	CString sFile;
	DWORD lenofread = 0, targetlen = 0;
	OVERLAPPED ol = { 0 };
	mp3dec_frame_info_t info;
	static mp3dec_t mp3d;
	mp3dec_init(&mp3d);
	mlist.GetText(mlist.GetCurSel(), sFile);
	hFile = CreateFile(sFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	
	if (ReadFile(hFile, tmpbuf, MAX_SINGLE_BUFF, &lenofread, &ol) == FALSE)
	{
		CloseHandle(hFile);
		return;
	}
	CloseHandle(hFile);
	for (framecount = 0; framecount < MAX_FRAME; framecount++)
	{
		samples = mp3dec_decode_frame(&mp3d, &tmpbuf[datacount], lenofread - datacount, 0, &info);
		frame_pos[framecount] = datacount;
		frame_len[framecount] = info.frame_bytes;
		if (samples == 0)
		{
			break;
		}
		datacount += info.frame_bytes;
	}
	
	if (starttime > fulltime)
		starttime = fulltime;
	if (endtime > fulltime)
		endtime = fulltime;
	int startframe = starttime / fulltime * framecount;
	int endframe = endtime / fulltime * framecount;
	
	
	if (startframe > endframe)
		return;
	memset(newbuf, 0, MAX_SINGLE_BUFF);
	for (loopcount = 0; loopcount < frame_pos[1]; loopcount++)
	{
		newbuf[loopcount] = tmpbuf[loopcount];
	}
	datacount = loopcount;
	for (loopcount = frame_pos[startframe]; loopcount <= frame_pos[endframe]; loopcount++, datacount++)
	{
		newbuf[datacount] = tmpbuf[loopcount];
	}
	for (loopcount = frame_pos[framecount]; loopcount < frame_len[framecount]; loopcount++, datacount++)
	{
		newbuf[datacount] = tmpbuf[loopcount];
	}
	CString sNewFile;
	sFile.Delete(sFile.GetLength() - 4, 4);
	sNewFile = sFile + _T("_cut.mp3");
	for (loopcount = 0; loopcount < MAX_WRITE_TRYS; loopcount++)
	{
		hFile = CreateFile(sNewFile, GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE && GetLastError() == 80)
			sNewFile.Format(sFile + _T("_cut%d.mp3"), loopcount);
		else
			break;
	}
	if (WriteFile(hFile, newbuf, datacount, &lenofread, &ol) == FALSE)
	{
		return;
	}
	CloseHandle(hFile);

	delete(tmpbuf);
	delete(newbuf);
	delete(frame_pos);
	delete(frame_len);
}


void CaddDlg::OnLbnSelchangeMlist()
{
	CString sFile;
	HANDLE hFile;
	DWORD lenofread;
	OVERLAPPED ol = { 0 };
	int itemindex = mlist.GetCurSel();
	if (itemindex == -1)
		return;
	else
		mlist.GetText(itemindex, sFile);
	hFile = CreateFile(sFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	unsigned char *tmpbuf = new unsigned char[MAX_SINGLE_BUFF];
	if (ReadFile(hFile, tmpbuf, MAX_SINGLE_BUFF, &lenofread, &ol) == FALSE)
	{
		delete(tmpbuf);
		return;
	}
	int countframes = 0, samples = 0, frame_pos = 0;
	float songslength = 0;
	mp3dec_frame_info_t info;
	static mp3dec_t mp3d;
	mp3dec_init(&mp3d);
	samples = mp3dec_decode_frame(&mp3d, tmpbuf, lenofread, 0, &info);
	songslength = (float)samples / info.hz;
	for (countframes = 1; countframes < MAX_FRAME; countframes++)
	{
		frame_pos += info.frame_bytes;
		samples = mp3dec_decode_frame(&mp3d, &tmpbuf[frame_pos], lenofread - frame_pos, 0, &info);
		if (samples == 0)
		{
			break;
		}
	}
	CloseHandle(hFile);
	delete(tmpbuf);
	SetDlgTime(IDC_TIMEEND, songslength * countframes);
	mslide.SetRange(0, songslength * countframes * 1000, 1);
	mslide.SetTicFreq(songslength * countframes * 1000);
	mslide.SetPageSize(1000);
	mslide.SetLineSize(5);
}




void CaddDlg::OnNMCustomdrawTimezone(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString sTemp;
	float newtime = (float)mslide.GetPos() / mslide.GetRangeMax() * GetDlgTime(IDC_TIMEEND);
	int end = newtime / 60;
	newtime = newtime - end * 60;
	sTemp.Format(_T("%d:%.3f"), end, newtime);
	SetDlgItemTextW(IDC_TIMEBEGIN, sTemp);

}

float CaddDlg::GetDlgTime(int nID)
{
	CString sTemp;
	GetDlgItemTextW(nID, sTemp);
	char ss[20], mm[20];
	memset(ss, 0, 20);
	memset(ss, 0, 20);
	int loopcount = 0;
	int end = sTemp.Find(_T(":"), 0);
	for (; loopcount < end; loopcount++)
	{
		mm[loopcount] = sTemp[loopcount];
	}
	end = sTemp.GetLength();
	for (int loopss = 0; loopcount < end; )
	{
		loopcount++;
		ss[loopss] = sTemp[loopcount];
		loopss++;
	}
	return (atof(ss) + 60 * atoi(mm));
}

void CaddDlg::SetDlgTime(int nID, float newtime)
{
	int minutes = newtime / 60;
	newtime = newtime - minutes * 60;
	CString sSonglength;
	sSonglength.Format(_T("%d:%.3f"), minutes, newtime);
	SetDlgItemTextW(nID, sSonglength);
}

void CaddDlg::OnStnClickedTimebegin()
{
	if (sw_focus)
	{
		SetDlgTime(IDC_EDITEND, GetDlgTime(IDC_TIMEBEGIN));
		sw_focus = 0;
	}
	else
	{
		SetDlgTime(IDC_EDITBEGIN, GetDlgTime(IDC_TIMEBEGIN));
		sw_focus = 1;
	}

}


void CaddDlg::OnLbnDblclkMlist()
{
	CString execmd;
	mlist.GetText(mlist.GetCurSel(), execmd);
	ShellExecute(nullptr, _T("open"), execmd, _T(""), _T(""), SW_SHOW);
}
