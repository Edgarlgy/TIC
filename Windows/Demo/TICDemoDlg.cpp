#include "stdafx.h"
#include "TICDemo.h"
#include "TICDemoDlg.h"
#include "afxdialogex.h"
#include "TIC/jsoncpp/json.h"
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern CTICDemoApp theApp;

BEGIN_MESSAGE_MAP(CTICDemoDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CTICDemoDlg::OnTabSelChange)
	ON_BN_CLICKED(IDC_BTN_JOIN_ROOM, &CTICDemoDlg::OnBtnJoinRoom)
	ON_BN_CLICKED(IDC_BTN_QUIT_ROOM, &CTICDemoDlg::OnBtnQuitRoom)
	ON_WM_TIMER()
END_MESSAGE_MAP()


CTICDemoDlg::CTICDemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TICDEMO_DIALOG, pParent)
{
	hIcon_ = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	videoDlg_ = std::make_shared<CVideoDlg>();
	boardDlg_ = std::make_shared<CBoardDlg>();
}

CTICDemoDlg::~CTICDemoDlg()
{

}

void CTICDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TAB, tabCtrl_);

	DDX_Control(pDX, IDC_EDIT_USER, editUser_);
	DDX_Control(pDX, IDC_EDIT_CLASSID, editClassId_);

	DDX_Control(pDX, IDC_BTN_JOIN_ROOM, btnJoinRoom_);
	DDX_Control(pDX, IDC_BTN_QUIT_ROOM, btnQuitRoom_);
}

BOOL CTICDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(hIcon_, TRUE);  // ���ô�ͼ��
	SetIcon(hIcon_, FALSE); // ����Сͼ��

	//����TABҳ
	tabCtrl_.InsertItem(0, _T("�װ�"));
	tabCtrl_.InsertItem(1, _T("��Ƶ"));
	

	boardDlg_->Create(IDD_BOARD_DIALOG, &tabCtrl_);
	videoDlg_->Create(IDD_VIDEO_DIALOG, &tabCtrl_);

	//��ȡ��ǩ�߶�
	CRect itemRect;
	tabCtrl_.GetItemRect(0, &itemRect);

	//��ȡ�������ǩҳ��������С
	CRect clientRect;
	tabCtrl_.GetClientRect(clientRect);
	clientRect.top += itemRect.Height() + 2;
	clientRect.bottom -= 2;
	clientRect.left += 2;
	clientRect.right -= 2;

	boardDlg_->MoveWindow(&clientRect);
	boardDlg_->ShowWindow(SW_SHOW);
	videoDlg_->MoveWindow(&clientRect);
	videoDlg_->ShowWindow(SW_HIDE);
	
	

	// TIC�ص�
	TICManager::GetInstance().AddEventListener(videoDlg_.get());
	TICManager::GetInstance().AddMessageListener(videoDlg_.get());
	TICManager::GetInstance().AddStatusListener(this);

	// ��дĬ�ϲ���
	std::default_random_engine randomEngine(time(NULL));
	std::uniform_int_distribution<int> uid(10000, 99999);
	std::wstring userId = L"tiw_win_" + std::to_wstring(uid(randomEngine));
	theApp.setUserId(w2a(userId));
	if (__argc == 1) {
		editClassId_.SetWindowText(L"13582");
	}
	else {
		int classId = 0;
		std::sscanf(__argv[1], "%d", &classId);
		theApp.setClassId(classId);
		std::wstring classIdStr = std::to_wstring(classId);
		editClassId_.SetWindowText(classIdStr.c_str());
	}

	// ������ʱ�����ڶ�ʱ���ڼ��ǩ����ȡ״̬������UI��ʾ��Ϣ��������¼����
	UpdateUI(UserState::NotInit);
	editUser_.SetWindowText(L"��ȡ�û���Ϣ...");
	this->SetTimer(0, 500, nullptr);

	// ��ȡǩ����Ϣ
	std::wstring url = L"https://classroom-6b29e9.service.tcloudbase.com/tiw_usersig?userId=" + userId + L"&timestamp=" + std::to_wstring(time(NULL));
	HttpHeaders headers;
	headers.SetHeader(L"Referer", L"demo.qcloudtiw.com");
	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	httpClient_.asynGet(url, [this, weakSelf](int code, const HttpHeaders& rspHeaders, const std::string& rspBody) {
		if (code != 0) {
			showErrorMsg(TICMODULE_TIC, -1, "Authentication failed");
			return;
		}
		Json::Reader reader;
		Json::Value root;
		bool bRet = reader.parse(rspBody, root);
		if (!bRet || root["sdkAppId"].isNull() || root["userSig"].isNull()) {
			showErrorMsg(TICMODULE_TIC, -2, "JSON parse failed!");
			return;
		}

		theApp.setSdkAppId(root["sdkAppId"].asInt());
		theApp.setUserSig(root["userSig"].asString());
		UpdateUI(UserState::Init);
		}, &headers);

	//���ð汾��Ϣ;
	CString version;
	version.Format(_T("�汾��Ϣ��IMSDK: %s | TRTC: %s | TEduBoard: %s"),
		a2w(TIMGetSDKVersion()).c_str(),
		a2w(getLiteAvSDKVersion()).c_str(),
		a2w(GetTEduBoardVersion()).c_str());

	GetDlgItem(IDC_VERSION)->SetWindowText(version);

	return TRUE;
}


void CTICDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	static int dot0 = 0;
	static int dot1 = 0;
	static bool login = false;
	if (nIDEvent == 0) {
		dot0++;
		if (dot0 > 3) dot0 = 1;
		std::wstring dotStr(dot0, L'.');
		if (this->state_ == UserState::NotInit) {
			std::wstring notify = L"��ȡ�û���Ϣ" + dotStr;  // ��ʾ���ڻ�ȡ�û���Ϣ
			editUser_.SetWindowText(notify.c_str());
		}
		else if (this->state_ == UserState::Init) {
			std::wstring notify = L"��¼" + dotStr;  // ��ʾ���ڵ�¼
			editUser_.SetWindowText(notify.c_str());
			if (!login) {
				login = true;
				std::weak_ptr<CTICDemoDlg> weakSelf = this->shared_from_this();
				TICManager::GetInstance().Init(theApp.getSdkAppId(), [this, weakSelf](TICModule module, int code, const char *desc) {
					std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
					if (!self)
					{
						return;
					}

					if (code == 0) {
						TICManager::GetInstance().Login(theApp.getUserId(), theApp.getUserSig(), [this, weakSelf](TICModule module, int code, const char *desc) {
							std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
							if (!self)
							{
								return;
							}

							if (code == 0) {
								UpdateUI(UserState::Login);
								if (theApp.getClassId() != 0) {  // ͨ�������в��������˷���ţ��Զ�����
									btnJoinRoom_.SendMessage(BM_CLICK, 0, 0);
								}
							}
							else {
								showErrorMsg(module, code, desc);
							}
							});
					}
					else {
						showErrorMsg(module, code, desc);
					}
					});
			}
		}
		if (this->state_ == UserState::Login) {
			std::wstring notify = a2w(theApp.getUserId());  // ��ʾ�û���
			editUser_.SetWindowText(notify.c_str());
			this->KillTimer(0);  // ɾ����ʱ��
		}
	}
	else if (nIDEvent == 1) {
		dot1++;
		if (dot1 > 3) dot1 = 1;
		std::wstring dotStr(dot1, L'.');
		if (this->state_ == UserState::Login) {
			std::wstring notify = L"���뷿��" + dotStr;  // ��ʾ���ڼ��뷿��
			editClassId_.SetWindowText(notify.c_str());
		}
		else if (this->state_ == UserState::InRoom) {
			std::wstring notify = std::to_wstring(theApp.getClassId());  // ��ʾ�����
			editClassId_.SetWindowText(notify.c_str());
			this->KillTimer(1);  // ɾ����ʱ��
		}
	}
	__super::OnTimer(nIDEvent);
}

HBRUSH CTICDemoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	int nCtrlId = pWnd->GetDlgCtrlID();
	if (nCtrlId == IDC_NOTIFY)
	{
		pDC->SetTextColor(RGB(200, 0, 0));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTICDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (tabCtrl_.m_hWnd) {
		//��ȡ��ǩ�߶�
		CRect itemRect;
		tabCtrl_.GetItemRect(0, &itemRect);

		//��ȡ�������ǩҳ��������С
		CRect clientRect;
		tabCtrl_.GetClientRect(clientRect);
		clientRect.top += itemRect.Height() + 2;
		clientRect.bottom -= 2;
		clientRect.left += 2;
		clientRect.right -= 2;

		videoDlg_->MoveWindow(&clientRect);
		boardDlg_->MoveWindow(&clientRect);
	}
}

void CTICDemoDlg::OnDestroy()
{
	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().Uninit([this, weakSelf](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0) {
			UpdateUI(UserState::NotInit);
		}
		else {
			UpdateUI(UserState::NotInit);
			showErrorMsg(module, code, desc);
		}
	});

	TICManager::GetInstance().RemoveEventListener(videoDlg_.get());
	TICManager::GetInstance().RemoveMessageListener(videoDlg_.get());
	TICManager::GetInstance().RemoveStatusListener(this);
}

void CTICDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, hIcon_);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
HCURSOR CTICDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(hIcon_);
}

void CTICDemoDlg::OnTabSelChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	int nCurSel = tabCtrl_.GetCurSel();
	switch (nCurSel)
	{
	case 0:
		boardDlg_->ShowWindow(SW_SHOW);
		videoDlg_->ShowWindow(SW_HIDE);
		break;
	case 1:
		boardDlg_->ShowWindow(SW_HIDE);
		videoDlg_->ShowWindow(SW_SHOW);
		break;
	case 2:
		boardDlg_->ShowWindow(SW_HIDE);
		videoDlg_->ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}
}

void CTICDemoDlg::OnBtnJoinRoom()
{
	CString str;
	editClassId_.GetWindowText(str);
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("������д�����"), MB_OK);
		return;
	}
	int classId = atoi(w2a(str.GetBuffer()).c_str());
	if (classId == 0)
	{
		AfxMessageBox(_T("���������"), MB_OK);
		return;
	}
	theApp.setClassId(classId);

	btnJoinRoom_.EnableWindow(FALSE);

	// ������ʱ�����ڶ�ʱ���ڼ�����״̬������UI��ʾ��Ϣ
	editClassId_.SetWindowText(L"���뷿��...");
	this->SetTimer(1, 500, nullptr);

	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().CreateClassroom(classId, TIC_CLASS_SCENE_VIDEO_CALL, [this, weakSelf, classId](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0 || code == 10021 || code == 10025) {
			TICClassroomOption option;
			option.classId = classId;
			option.boardCallback = boardDlg_.get();

			std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
			TICManager::GetInstance().JoinClassroom(option, [this, weakSelf](TICModule module, int code, const char *desc) {
				std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
				if (!self)
				{
					return;
				}

				if (code == 0) {
					UpdateUI(UserState::InRoom);
					boardDlg_->Init();
				}
				else {
					showErrorMsg(module, code, desc);
				}
				});
		}
		else {
			showErrorMsg(module, code, desc);
		}
		});
}

void CTICDemoDlg::OnBtnQuitRoom()
{
	btnQuitRoom_.EnableWindow(FALSE);

	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().QuitClassroom(false, [this, weakSelf](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0) {
			UpdateUI(UserState::Login);
			boardDlg_->Uninit();
		}
		else {
			showErrorMsg(module, code, desc);
		}
	});
}

void CTICDemoDlg::onTICForceOffline()
{
	UpdateUI(UserState::NotInit);

	AfxMessageBox(_T("�˺��������ط���½"), MB_OK);
}

void CTICDemoDlg::onTICUserSigExpired()
{
	AfxMessageBox(_T("Sig����"), MB_OK);
}

void CTICDemoDlg::UpdateUI(UserState state)
{
	switch (state)
	{
	case UserState::NotInit:
	{
		editClassId_.EnableWindow(FALSE);
		btnJoinRoom_.EnableWindow(FALSE);
		btnQuitRoom_.EnableWindow(FALSE);
		break;
	}
	case UserState::Init:
	{
		editClassId_.EnableWindow(TRUE);
		btnJoinRoom_.EnableWindow(FALSE);
		btnQuitRoom_.EnableWindow(FALSE);
		break;
	}
	case UserState::Login:
	{
		editClassId_.EnableWindow(TRUE);
		btnJoinRoom_.EnableWindow(TRUE);
		btnQuitRoom_.EnableWindow(FALSE);
		break;
	}
	case UserState::InRoom:
	{
		editClassId_.EnableWindow(FALSE);
		btnJoinRoom_.EnableWindow(FALSE);
		btnQuitRoom_.EnableWindow(TRUE);
		break;
	}
	default: break;
	}

	videoDlg_->UpdateUI(state);
	boardDlg_->UpdateUI();

	if (state != UserState::Unknown) {
		state_ = state;
	}
}
