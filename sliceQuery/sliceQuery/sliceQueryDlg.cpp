
// sliceQueryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sliceQuery.h"
#include "sliceQueryDlg.h"
#include "afxdialogex.h"

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


// CsliceQueryDlg dialog



CsliceQueryDlg::CsliceQueryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SLICEQUERY_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CsliceQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CsliceQueryDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CsliceQueryDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CsliceQueryDlg message handlers

BOOL CsliceQueryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// TODO: Add extra initialization here
	SetDlgItemText(IDC_EDIT3, L"20");
	SetDlgItemText(IDC_EDIT4, L"postgres");
	SetDlgItemText(IDC_EDIT5, L"password");
	SetDlgItemText(IDC_EDIT6, L"slice");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CsliceQueryDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CsliceQueryDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CsliceQueryDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

using namespace std;

string wchar2string(WCHAR* wch)
{
	wstring wlist(wch);
	string list(wlist.begin(), wlist.end());
	return list;
}

string strFromVector(vector<unsigned>& list)
{
	string str;
	for (int i = 0; i < list.size(); i++)
	{
		str.append(to_string(list[i]));
		if (i != list.size() - 1)
			str.push_back('\n');
	}
	return str;
}

//calculate factorial of a positive number
inline size_t factorial(unsigned n)
{
	size_t res = 1;
	int m = n;
	while (m)
	{
		res *= m;
		m--;
	}
	return res;
}

//combination of n choose m
inline size_t combination(int n, int m)
{
	size_t res = 1;
	for (int i = n; i > n - m; i--)
	{
		res *= i;
	}
	res /= factorial(m);
	return res;
}

wstring widen(const string& str)
{
	wostringstream wstm;
	const ctype<wchar_t>& ctfacet =
		use_facet< ctype<wchar_t> >(wstm.getloc());
	for (size_t i = 0; i < str.size(); ++i)
		wstm << ctfacet.widen(str[i]);
	return wstm.str();
}

wstring DBOperation(vector<unsigned>& list, const unsigned nquery, string& const db, string& const usr, string& const psd)
{
	const int qCount = list.size();
	string conninfo = "host=localhost port=5432 dbname=" + db + " user=" + usr+" password=" + psd;
	PGconn     *conn;
	PGresult   *res;
	conn = PQconnectdb(conninfo.c_str());
	if (PQstatus(conn) != CONNECTION_OK)
		return widen(PQerrorMessage(conn));
	PQexec(conn, "BEGIN;\
		 TRUNCATE TABLE query; \
		  COPY query (radius) FROM stdin;");
	if (PQerrorMessage(conn) != NULL && strlen(PQerrorMessage(conn)))
		return widen(PQerrorMessage(conn));
	string concatNums = strFromVector(list);
	int copyStatus = PQputCopyData(conn, concatNums.c_str(), concatNums.size());
	if (copyStatus == -1)
		return 	widen(PQerrorMessage(conn));
	const char* errmsg = NULL;
	int endStatus = PQputCopyEnd(conn, errmsg);
	if (endStatus == -1)
		return 	widen(PQerrorMessage(conn));
	PQexec(conn, "COMMIT;");
	if (PQerrorMessage(conn) != NULL && strlen(PQerrorMessage(conn)))
		return widen(PQerrorMessage(conn));
	
	string strCmd = "select * from calcProbability()";
	if (nquery)
		strCmd += " limit " + to_string(nquery);
	strCmd += ";";
	res = PQexec(conn, strCmd.c_str());
	string result;
	int rows = PQntuples(res);
	for (int i = 0; i < rows; i++)
	{
		result.append(PQgetvalue(res, i, 0));
		result.push_back('\t');
		result.append(PQgetvalue(res, i, 1));
		result.append(" %");
		if (i != rows - 1)
			result.append("\r\n");
	}
	return widen(result.c_str());
}

void CsliceQueryDlg::OnBnClickedOk()
{
	//calculates the corresponding samples
	WCHAR* rawList = new  WCHAR[2048];
	GetDlgItemText(IDC_EDIT1, rawList, 2048);
	string list = wchar2string(rawList);

	WCHAR* numquery = new WCHAR[5];
	GetDlgItemText(IDC_EDIT3, numquery, 5);
	unsigned nquery = wcstol(numquery, NULL, 10);

	WCHAR* userName = new WCHAR[128];
	GetDlgItemText(IDC_EDIT4, userName, 128);
	string usr = wchar2string(userName);

	WCHAR* password = new WCHAR[128];
	GetDlgItemText(IDC_EDIT5, password, 128);
	string psd = wchar2string(password);

	WCHAR* databaseName = new WCHAR[128];
	GetDlgItemText(IDC_EDIT6, databaseName, 128);
	string db = wchar2string(databaseName);

	vector<unsigned> slices;
	int i = 0;
	while (i < list.size())
	{
		string buffer;
		while (i < list.size() && isdigit(list[i]))
		{
			buffer.push_back(list[i]);
			i++;
		}
		if (buffer.size())
		{
			double slice = 0;
			try
			{
				slice = stod(buffer);
				if (slice <= 0)
					throw std::invalid_argument("Invalid numbers");
			}
			catch (exception e)
			{
				SetDlgItemText(IDC_EDIT1, L"Invalid numbers.");
				return;
			}
			slices.push_back((unsigned)(slice + 0.5));
		}
		i++;
	}
	if (slices.size() == 0)
		return;
	wstring returnVal = DBOperation(slices, nquery, db, usr, psd);
	if (returnVal.size())
		SetDlgItemText(IDC_EDIT2, returnVal.c_str());
	else
		SetDlgItemText(IDC_EDIT2, L"No suitable samples found");

	delete[] rawList;
	delete[] numquery;
	delete[] userName;
	delete[] password;
}

