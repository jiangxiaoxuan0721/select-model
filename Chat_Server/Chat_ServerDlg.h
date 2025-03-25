
// Chat_ServerDlg.h: 头文件
//

#pragma once
#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")
#pragma warning (disable:4996)
// CChatServerDlg 对话框
class CChatServerDlg : public CDialogEx
{
// 构造
public:
	CChatServerDlg(CWnd* pParent = nullptr);	// 标准构造函数
	SOCKET sockSer{};							// 监听套接字
	sockaddr_in addrSer{};						// 本地地址端口
	fd_set fdsock{};		//保存所有服务器套接字的集合 sockCon集合
	fd_set fdread{};		//select 函数检测的可读套接字集合
	static UINT selectThread(LPVOID any);		// 声明线程函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_SERVER_DIALOG };
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
	DECLARE_MESSAGE_MAP()
public:
	CString m_ip;
	CEdit c_ip;
	CString m_port;
	CEdit c_port;
	CListBox c_recvbuf;
	CButton c_create;
	CButton c_quit;
	afx_msg void OnBnClickedCreate();
	CString m_sendbuf;
	CEdit c_sendbuf;
	afx_msg void OnBnClickedSend();
};
