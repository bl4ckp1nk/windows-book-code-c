// FtpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxinet.h"
#include "FtpFile.h"
#include "FtpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFtpDlg dialog

CFtpDlg::CFtpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFtpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFtpDlg)
	m_strFtpSite = _T("");
	m_strName = _T("");
	m_strPwd = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFtpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFtpDlg)
	DDX_Control(pDX, IDOK, m_BtnQuery);
	DDX_Control(pDX, IDC_UPLOAD, m_BtnUpLoad);
	DDX_Control(pDX, IDC_STATIC_PWD, m_StaPwd);
	DDX_Control(pDX, IDC_STATIC_NAME, m_StaName);
	DDX_Control(pDX, IDC_STATIC_FTP, m_StaFtp);
	DDX_Control(pDX, IDC_EDIT_PWD, m_EditPwd);
	DDX_Control(pDX, IDC_EDIT_NAME, m_EditName);
	DDX_Control(pDX, IDC_EDIT_FTP, m_EditFtp);
	DDX_Control(pDX, IDC_DOWNLOAD, m_BtnDownLoad);
	DDX_Control(pDX, IDC_LIST_FILE, m_ListFile);
	DDX_Text(pDX, IDC_EDIT_FTP, m_strFtpSite);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_PWD, m_strPwd);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFtpDlg, CDialog)
	//{{AFX_MSG_MAP(CFtpDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnQuery)
	ON_LBN_SELCHANGE(IDC_LIST_FILE, OnSelchangeListFile)
	ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_UPLOAD, OnUpload)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFtpDlg message handlers

BOOL CFtpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	
	// TODO: Add extra initialization here
	//??????????
	m_strFtpSite=_T("");
	//????????????
	m_strName=_T("");
	//??????????
	m_strPwd=_T("");
	//????????
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFtpDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFtpDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFtpDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFtpDlg::OnQuery() 
{
	//????????????
	UpdateData(TRUE);

	//????????????????
	while(m_ListFile.GetCount()!=0)
		m_ListFile.DeleteString(0);

	//????????????????
	ListContent();
}

void CFtpDlg::ListContent()
{
	CInternetSession* pSession;
	CFtpConnection* pConnection;
	CFtpFileFind* pFileFind;
	CString strFileName;
	BOOL bContinue;

	pConnection=NULL;
	pFileFind=NULL;

	//????Internet????
	pSession=new CInternetSession(	AfxGetAppName(),
										1,
										PRE_CONFIG_INTERNET_ACCESS);

	try
	{
		//????????FTP????
		pConnection=pSession->GetFtpConnection(m_strFtpSite,
			m_strName,m_strPwd);
	}
	catch (CInternetException* e)
	{
		//????????
		e->Delete();
		pConnection=NULL;
	}

	if (pConnection!=NULL)
	{
		//????CFtpFileFind????
		//????CFtpConnection??????????
		pFileFind=new CFtpFileFind(pConnection);
	
		//????????????
		bContinue=pFileFind->FindFile("*");

		if (!bContinue)
		{
			//????????
			pFileFind->Close();
			pFileFind=NULL;
		}

		while (bContinue)
		{
			//??????????????
			bContinue=pFileFind->FindNextFile();
			//??????????????????????
			strFileName=pFileFind->GetFileName();

			if (pFileFind->IsDirectory())
			{
				//??????????????
				//??????????
				strFileName="["+strFileName;
				strFileName+="]";
			}
			//??????????????
			m_ListFile.AddString(strFileName);
		}

		if (pFileFind!=NULL)
		{
			//????????
			pFileFind->Close();
			pFileFind=NULL;
		}
	}

	//????????
	delete pFileFind;
	if (pConnection!=NULL)
	{
		pConnection->Close();
		delete pConnection;
	}
	delete pSession;
}

void CFtpDlg::OnSelchangeListFile() 
{
	//??????????????????????????????
	m_EditFtp.EnableWindow(FALSE);
	m_EditName.EnableWindow(FALSE);
	m_EditPwd.EnableWindow(FALSE);
	m_StaFtp.EnableWindow(FALSE);
	m_StaName.EnableWindow(FALSE);
	m_StaPwd.EnableWindow(FALSE);

	//??????????????????
	m_BtnUpLoad.EnableWindow(FALSE);
	m_BtnQuery.EnableWindow(FALSE);

	//????????????
	m_BtnDownLoad.EnableWindow(TRUE);
}

void CFtpDlg::OnDownload() 
{
	//????????????
	UpdateData(TRUE);
	int nSel=m_ListFile.GetCurSel();
	CString strSourceName;

	//????????????????????????
	nSel=m_ListFile.GetCurSel();
	m_ListFile.GetText(nSel,strSourceName);

	if (strSourceName.GetAt(0)!='[')
	{
		//????????????
		CString strDestName;
		CFileDialog dlg(FALSE,"","*.*");
		if (dlg.DoModal()==IDOK)
		{
			//??????????????????????????????????????
			strDestName=dlg.GetPathName();

			//????????????????
			if (GetFile(strSourceName,strDestName))
				AfxMessageBox("??????????",MB_OK|MB_ICONINFORMATION);
			else
				AfxMessageBox("??????????",MB_OK|MB_ICONSTOP);
		}
		else
		{
			AfxMessageBox("??????????????",MB_OK|MB_ICONSTOP);
		}
	}
	else
	{
		//????????????
		AfxMessageBox("????????????!\n??????!",MB_OK|MB_ICONSTOP);
	}

	//????????????
	m_BtnDownLoad.EnableWindow(FALSE);

	//??????????????????
	m_BtnUpLoad.EnableWindow(TRUE);
	m_BtnQuery.EnableWindow(TRUE);

	//??????????????????????????????
	m_EditFtp.EnableWindow(TRUE);
	m_EditName.EnableWindow(TRUE);
	m_EditPwd.EnableWindow(TRUE);
	m_StaFtp.EnableWindow(TRUE);
	m_StaName.EnableWindow(TRUE);
	m_StaPwd.EnableWindow(TRUE);
}

BOOL CFtpDlg::GetFile(CString strSourceName, CString strDestName)
{
	CInternetSession* pSession;
	CFtpConnection* pConnection;

	pConnection=NULL;

	//????Internet????
	pSession=new CInternetSession(	AfxGetAppName(),
									1,
									PRE_CONFIG_INTERNET_ACCESS);

	try
	{
		//????FTP????
		pConnection=pSession->GetFtpConnection(	m_strFtpSite,
												m_strName,
												m_strPwd);
	}
	catch (CInternetException* e)
	{
		//????????
		e->Delete();
		pConnection=NULL;
		return FALSE;
	}

	if (pConnection!=NULL)
	{
		//????????
		if (!pConnection->GetFile(strSourceName,strDestName))
		{
			//????????????
			pConnection->Close();
			delete pConnection;
			delete pSession;
			return FALSE;
		}
	}

	//????????
	if (pConnection!=NULL)
	{
		pConnection->Close();
		delete pConnection;
	}
	delete pSession;

	return TRUE;
}


void CFtpDlg::OnUpload() 
{
	//????????????
	UpdateData(TRUE);

	//????????????????????????????
	m_EditFtp.EnableWindow(FALSE);
	m_EditName.EnableWindow(FALSE);
	m_EditPwd.EnableWindow(FALSE);
	m_StaFtp.EnableWindow(FALSE);
	m_StaName.EnableWindow(FALSE);
	m_StaPwd.EnableWindow(FALSE);

	//????????????
	m_BtnQuery.EnableWindow(FALSE);

	CString strSourceName;
	CString strDestName;
	CFileDialog dlg(TRUE,"","*.*");
	if (dlg.DoModal()==IDOK)
	{
		//??????????????????????????????????
		strSourceName=dlg.GetPathName();
		strDestName=dlg.GetFileName();

		//????????????????
		if (PutFile(strSourceName,strDestName))
			AfxMessageBox("??????????",MB_OK|MB_ICONINFORMATION);
		else
			AfxMessageBox("??????????",MB_OK|MB_ICONSTOP);
	}
	else
	{
		//??????????????
		AfxMessageBox("????????????",MB_OK|MB_ICONSTOP);
	}

	//????????????
	m_BtnQuery.EnableWindow(TRUE);

	//????????????????????????????
	m_EditFtp.EnableWindow(TRUE);
	m_EditName.EnableWindow(TRUE);
	m_EditPwd.EnableWindow(TRUE);
	m_StaFtp.EnableWindow(TRUE);
	m_StaName.EnableWindow(TRUE);
	m_StaPwd.EnableWindow(TRUE);
}

BOOL CFtpDlg::PutFile(CString strSourceName, CString strDestName)
{
	CInternetSession* pSession;
	CFtpConnection* pConnection;

	pConnection=NULL;

	//????Internet????
	pSession=new CInternetSession(	AfxGetAppName(),
									1,
									PRE_CONFIG_INTERNET_ACCESS);

	try
	{
		//????FTP????
		pConnection=pSession->GetFtpConnection(	m_strFtpSite,
												m_strName,
												m_strPwd);
	}
	catch (CInternetException* e)
	{
		//????????
		e->Delete();
		pConnection=NULL;
		return FALSE;
	}

	if (pConnection!=NULL)
	{
		//????????
		if (!pConnection->PutFile(strSourceName,strDestName))
		{
			//????????????
			pConnection->Close();
			delete pConnection;
			delete pSession;
			return FALSE;
		}
	}

	//????????
	if (pConnection!=NULL)
	{
		pConnection->Close();
		delete pConnection;
	}

	delete pSession;

	return TRUE;
}

void CFtpDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//????????????
	m_BtnDownLoad.EnableWindow(FALSE);

	//??????????????????
	m_BtnUpLoad.EnableWindow(TRUE);
	m_BtnQuery.EnableWindow(TRUE);

	//??????????????????????????????
	m_EditFtp.EnableWindow(TRUE);
	m_EditName.EnableWindow(TRUE);
	m_EditPwd.EnableWindow(TRUE);
	m_StaFtp.EnableWindow(TRUE);
	m_StaName.EnableWindow(TRUE);
	m_StaPwd.EnableWindow(TRUE);
	
	CDialog::OnRButtonDown(nFlags, point);
}
