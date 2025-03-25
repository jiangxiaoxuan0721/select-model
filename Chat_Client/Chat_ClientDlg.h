
// Chat_ClientDlg.h: 头文件
//

#pragma once
#define WM_SOCKET WM_USER + 1
#pragma warning (disable:4996)

// CChatClientDlg 对话框
class CChatClientDlg : public CDialogEx
{
// 构造
public:
	CChatClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	SOCKET sockCli{};
	sockaddr_in addrSer{};
	
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton c_connect;
	CButton c_send;
	CEdit c_ip;
	CString m_ip;
	CEdit c_port;
	CString m_port;
	CButton c_quit;
	CListBox c_recvbuf;
	CEdit c_sendbuf;
	CString m_sendbuf;
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedSend();
	afx_msg LRESULT OnReceiveData(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
