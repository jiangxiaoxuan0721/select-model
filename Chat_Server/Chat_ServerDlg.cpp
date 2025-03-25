
// Chat_ServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Chat_Server.h"
#include "Chat_ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatServerDlg 对话框
CChatServerDlg::CChatServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_SERVER_DIALOG, pParent)
	, m_ip(_T(""))
	, m_port(_T(""))
	, m_sendbuf(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

UINT CChatServerDlg::selectThread(LPVOID any) {
	CChatServerDlg* p = (CChatServerDlg*)any;
	SOCKET sockCon{};
	sockaddr_in addrCli{};
	int len = sizeof(sockaddr);

	while (TRUE) {
		FD_ZERO(&p->fdread);
		p->fdread = p->fdsock; // 可读套接字集合

		// 可以设置一个超时时间，避免长时间阻塞
		timeval timeout{};
		timeout.tv_sec = 1;  // 设置为1秒
		timeout.tv_usec = 0;

		// 使用 select 阻塞，此处使用了超时设置
		int selectResult = select(0, &p->fdread, NULL, NULL, &timeout);

		if (selectResult > 0) {
			for (int i = 0; i < p->fdsock.fd_count; ++i) {
				if (FD_ISSET(p->fdsock.fd_array[i], &p->fdread)) {
					if (p->fdsock.fd_array[i] == p->sockSer) { // 如果是监听套接字
						sockCon = accept(p->sockSer, (sockaddr*)&addrCli, &len);
						if (sockCon == INVALID_SOCKET) {
							AfxMessageBox(L"accept 调用失败");
						}
						else {
							char clibuf[255]{};
							char cliinfo[256]="欢迎来到聊天室！";
							sprintf(clibuf, "客户端<%s:%d>连接成功！", inet_ntoa(addrCli.sin_addr), ntohs(addrCli.sin_port));
							
							p->c_recvbuf.AddString(CStringW(clibuf));
							//启用发送按钮
							p->c_sendbuf.EnableWindow(TRUE);
							CT2A cA((CString)cliinfo, CP_UTF8);
							const char* c_buff = cA;
							if (send(sockCon, c_buff, strlen(c_buff), 0) == SOCKET_ERROR) {
								AfxMessageBox(L"转发数据失败！");
								closesocket(sockCon);
							}
							else {
								FD_SET(sockCon, &p->fdsock);
							}
						}
					}
					else { // 处理通信套接字
						char recvbuf[256]{};
						char cliinfo[256]{};
						CString clibuf = L"客户端";
						int bytesReceived = recv(p->fdsock.fd_array[i], recvbuf, sizeof(recvbuf) - 1, 0); // 留出一个字节存放'\0'
						if (bytesReceived > 0) {
							recvbuf[bytesReceived] = '\0'; // 确保字符串以空字符终止
							getpeername(p->fdsock.fd_array[i], (sockaddr*)&addrCli, &len);
							sprintf(cliinfo, "<%s:%d>:", inet_ntoa(addrCli.sin_addr), ntohs(addrCli.sin_port));
							strcat(cliinfo, recvbuf);
							clibuf += CA2W(cliinfo, CP_UTF8); // 指定使用 UTF-8 编码进行转换

							p->c_recvbuf.AddString(clibuf); // 更新显示
							
							for (int j = 0; j < p->fdsock.fd_count; ++j) {
								if (p->fdsock.fd_array[j] != p->sockSer && j != i) {
									CT2A cA(clibuf, CP_UTF8);
									const char* c_buff = cA;
									// 发送数据
									int sendResult = send(p->fdsock.fd_array[j], c_buff, strlen(c_buff), 0); // 发送数据
									if (sendResult == SOCKET_ERROR) {
										AfxMessageBox(L"转发数据失败！");
										return -1;
									}
								}
							}
						}
						else {
							int error = WSAGetLastError();
							// 判断客户端是否正常关闭
							if (bytesReceived == 0) {
								// 客户端正常关闭连接
								char cliinfo[256]{};
								sprintf(cliinfo, "客户端%s:%d已断开连接", inet_ntoa(addrCli.sin_addr), ntohs(addrCli.sin_port));
								CString clibuf = L"客户端";
								clibuf += CA2W(cliinfo, CP_UTF8); // 指定使用 UTF-8 编码进行转换
								p->c_recvbuf.AddString(clibuf + L"已断开连接");
								closesocket(p->fdsock.fd_array[i]);
								FD_CLR(p->fdsock.fd_array[i], &p->fdsock);
							}
							else if (error == WSAECONNRESET || error == WSAENOTCONN) {
								// 客户端重置连接或未连接，处理已断开的连接
								char cliinfo[256]{};
								sprintf(cliinfo, "<%s:%d>", inet_ntoa(addrCli.sin_addr), ntohs(addrCli.sin_port));
								CString clibuf = L"客户端";
								clibuf += CA2W(cliinfo, CP_UTF8); // 指定使用 UTF-8 编码进行转换
								p->c_recvbuf.AddString(clibuf + L"断开连接");
								closesocket(p->fdsock.fd_array[i]);
								FD_CLR(p->fdsock.fd_array[i], &p->fdsock);
							}
							else if (error != WSAEWOULDBLOCK) {
								// 其他错误处理
								char errorMsg[256]{};
								sprintf(errorMsg, "recv 调用失败！错误代码: %d", error);
								AfxMessageBox(CA2W(errorMsg, CP_UTF8), MB_OK | MB_ICONSTOP);
								return -1;
							}
						}
					}
				}
			}
		}
		else if (selectResult < 0) {
			p->c_recvbuf.AddString(L"select 函数调用失败！");
			p->c_recvbuf.AddString(L"错误码：" + GetLastError());
		}

		// 可以在这里添加一些逻辑用于退出线程，当前此处保持无限循环
	}
	return 0;
}

void CChatServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, EDT_IP, m_ip);
	DDX_Control(pDX, EDT_IP, c_ip);
	DDX_Text(pDX, EDT_PORT, m_port);
	DDX_Control(pDX, EDT_PORT, c_port);
	DDX_Control(pDX, LST_RECVBUF, c_recvbuf);
	DDX_Control(pDX, BTN_CREATE, c_create);
	DDX_Control(pDX, IDOK, c_quit);
	DDX_Control(pDX, EDT_SENDBUF, c_sendbuf);
	DDX_Text(pDX, EDT_SENDBUF, m_sendbuf);
}

BEGIN_MESSAGE_MAP(CChatServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BTN_CREATE, &CChatServerDlg::OnBnClickedCreate)
	ON_BN_CLICKED(BTN_SEND, &CChatServerDlg::OnBnClickedSend)
END_MESSAGE_MAP()


// CChatServerDlg 消息处理程序

BOOL CChatServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~WS_SIZEBOX); // 禁用可调整大小
	
	// TODO: 在此添加额外的初始化代码
	// TODO: 在此添加额外的初始化代码
	WSADATA wsaData{};
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		AfxMessageBox(L"初始化 Winsock 失败", MB_OK | MB_ICONSTOP);
	}
	sockSer = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSer == INVALID_SOCKET) {
		int errorCode = WSAGetLastError();
		CString errorMsg;
		errorMsg.Format(L"创建套接字失败！错误代码: %d", errorCode);
		AfxMessageBox(errorMsg, MB_OK | MB_ICONSTOP);
		WSACleanup();
	}

	m_ip = CStringW("127.0.0.1");
	m_port = CStringW("5566");
	UpdateData(FALSE);

	//禁用发送按钮
	c_sendbuf.EnableWindow(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，这将由框架自动完成。
void CChatServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

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

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CChatServerDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatServerDlg::OnBnClickedCreate(){
	UpdateData(FALSE);
	addrSer.sin_family = AF_INET;
	addrSer.sin_port = htons(_ttoi(m_port));
	addrSer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int len = sizeof(sockaddr);
	int ret = bind(sockSer, (sockaddr*)&addrSer, len);
	if (ret == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		CString errorMsg;
		switch (errorCode) {
		case WSAEADDRINUSE:
			errorMsg.Format(L"指定的地址已被使用。错误代码: %d", errorCode);
			break;
		case WSAEADDRNOTAVAIL:
			errorMsg.Format(L"指定的地址无法在本地机器上使用。错误代码: %d", errorCode);
			break;
		case WSAEFAULT:
			errorMsg.Format(L"name 指针或 namelen 参数无效。错误代码: %d", errorCode);
			break;
		default:
			errorMsg.Format(L"绑定失败，未知错误。错误代码: %d", errorCode);
			break;
		}
		AfxMessageBox(errorMsg, MB_OK | MB_ICONSTOP);
		closesocket(sockSer);
		WSACleanup();
		return;
	}
	else {
		if (listen(sockSer, 5) == 0) {
			c_recvbuf.AddString(L"等待客户端连接……\n");
		}
		// 按钮限制
		c_create.EnableWindow(FALSE);
		FD_ZERO(&fdsock);
		FD_SET(sockSer, &fdsock);
		AfxBeginThread(&selectThread, (LPVOID)this);
	}
}


void CChatServerDlg::OnBnClickedSend()
{
	UpdateData(TRUE);
	//禁止发送空消息
	if (m_sendbuf.IsEmpty()) {
		return;
	}
	CString sendMsg = L"系统消息：" + m_sendbuf;
	// 发送消息到所有客户端
	for (int i = 0; i < fdsock.fd_count; ++i) {
		if (fdsock.fd_array[i] != sockSer) {
			CT2A cA(sendMsg, CP_UTF8);
			const char* c_buff = cA;
			// 发送数据
			int sendResult = send(fdsock.fd_array[i], c_buff, strlen(c_buff), 0); // 发送数据
			if (sendResult == SOCKET_ERROR) {
				AfxMessageBox(L"发送数据失败！");
				return;
			}
		}
	}
	c_sendbuf.SetWindowTextW(L"");
	//显示发送消息
	c_recvbuf.AddString(sendMsg);
}
