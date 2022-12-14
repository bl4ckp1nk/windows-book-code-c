// RedirectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Redirect.h"
#include "RedirectDlg.h"

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
// CRedirectDlg dialog

CRedirectDlg::CRedirectDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CRedirectDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CRedirectDlg)
    m_strResult = _T("");
    m_strCommand = _T("");
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRedirectDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CRedirectDlg)
    DDX_Text(pDX, IDC_EDIT_Result, m_strResult);
    DDX_Text(pDX, IDC_EDIT_Command, m_strCommand);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRedirectDlg, CDialog)
    //{{AFX_MSG_MAP(CRedirectDlg)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_Run, OnBUTTONRun)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRedirectDlg message handlers

BOOL CRedirectDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    // Add "About..." menu item to system menu.
    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);
    CMenu* pSysMenu = GetSystemMenu(FALSE);

    if (pSysMenu != NULL) {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);

        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon
    // TODO: Add extra initialization here
    m_strCommand = "ping 127.0.0.1";
    UpdateData(FALSE);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRedirectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRedirectDlg::OnPaint()
{
    if (IsIconic()) {
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
    } else {
        CDialog::OnPaint();
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRedirectDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CRedirectDlg::OnOK()
{
    // TODO: Add extra validation here
    //CDialog::OnOK();
}

void CRedirectDlg::OnCancel()
{
    // TODO: Add extra cleanup here
    if (MessageBox("?????????????", "????????", MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDOK) {
        CDialog::OnCancel();
    }
}

#define MAXREADBUFFLEN  (20*1000)

void CRedirectDlg::OnBUTTONRun()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
    CString strCommand, strFilename = "Redirect.txt";
    m_strCommand.TrimRight(" ");

    if (m_strCommand == "")
        return;

    strCommand.Format("cmd.exe /c \"%s\">%s", m_strCommand, strFilename);
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    PROCESS_INFORMATION pi;
    BOOL res = CreateProcess(NULL, strCommand.GetBuffer(0),
                             NULL, NULL, NULL, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (!res) {
        MessageBox("??????????????");
        return;
    }

    //????????????????
    WaitForSingleObject(pi.hProcess, INFINITE);
    char buff[MAXREADBUFFLEN] = {0};
    BOOL bSuccess = FALSE;

    try {
        CFile file;

        if (file.Open(strFilename, CFile::modeReadWrite, NULL)) {
            file.Read((char *)buff, MAXREADBUFFLEN);
            file.Close();
            bSuccess = TRUE;
        }
    } catch (CFileException e) {
        //e.m_cause;
        Sleep(1000);
    }

    if (bSuccess) {
        DeleteFile(strFilename);
        m_strResult.Format("%s", (char *)buff);
    } else {
        MessageBox("??????????????", "????????");
    }

    UpdateData(FALSE);
}
