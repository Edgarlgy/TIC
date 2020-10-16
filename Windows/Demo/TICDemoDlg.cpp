#include "stdafx.h"
#include "TICDemo.h"
#include "TICDemoDlg.h"
#include "afxdialogex.h"
#include "Config.h"

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
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CTICDemoDlg::OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CTICDemoDlg::OnBtnLogout)
	ON_BN_CLICKED(IDC_BTN_CREATE_ROOM, &CTICDemoDlg::OnBtnCreateRoom)
	ON_BN_CLICKED(IDC_BTN_DESTROY_ROOM, &CTICDemoDlg::OnBtnDestroyRoom)
	ON_BN_CLICKED(IDC_BTN_JOIN_ROOM, &CTICDemoDlg::OnBtnJoinRoom)
	ON_BN_CLICKED(IDC_BTN_QUIT_ROOM, &CTICDemoDlg::OnBtnQuitRoom)
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

	DDX_Control(pDX, IDC_COMBOX_USER, cbUser_);

	DDX_Control(pDX, IDC_EDIT_CLASSID, editClassId_);

	DDX_Control(pDX, IDC_BTN_LOGIN, btnLogin_);
	DDX_Control(pDX, IDC_BTN_LOGOUT, btnLogout_);
	DDX_Control(pDX, IDC_BTN_CREATE_ROOM, btnCreateRoom_);
	DDX_Control(pDX, IDC_BTN_DESTROY_ROOM, btnDestroyRoom_);
	DDX_Control(pDX, IDC_BTN_JOIN_ROOM, btnJoinRoom_);
	DDX_Control(pDX, IDC_BTN_QUIT_ROOM, btnQuitRoom_);
}

BOOL CTICDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(hIcon_, TRUE);  // ���ô�ͼ��
	SetIcon(hIcon_, FALSE); // ����Сͼ��

	//����TABҳ
	tabCtrl_.InsertItem(0, _T("��Ƶ"));
	tabCtrl_.InsertItem(1, _T("�װ�"));

	videoDlg_->Create(IDD_VIDEO_DIALOG, &tabCtrl_);
	boardDlg_->Create(IDD_BOARD_DIALOG, &tabCtrl_);

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
	videoDlg_->ShowWindow(SW_SHOW);
	boardDlg_->MoveWindow(&clientRect);
	boardDlg_->ShowWindow(SW_HIDE);

	//�û��б�
	const std::vector<UserInfo>& userInfoList = Config::GetInstance().UserInfoList();
	for (size_t i=0; i<userInfoList.size(); ++i)
	{
		cbUser_.InsertString(i, a2w(userInfoList[i].userid).c_str());
	}
	cbUser_.SetCurSel(0);

	//TIC�ص�
	TICManager::GetInstance().AddEventListener(videoDlg_.get());
	TICManager::GetInstance().AddMessageListener(videoDlg_.get());
	TICManager::GetInstance().AddStatusListener(this);
	std::weak_ptr<CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().Init(Config::GetInstance().SdkAppId(), [this, weakSelf](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0) {
			UpdateUI(UserState::Init);
		}
		else {
			UpdateUI(UserState::NotInit);
			showErrorMsg(module, code, desc);
		}
	});

	// ��дĬ�ϲ���
	editClassId_.SetWindowText(_T("13582"));

	//���ð汾��Ϣ;
	CString version;
	version.Format(_T("IMSDK: %s\n\nTRTC: %s\n\nTEduBoard: %s"),
		a2w(TIMGetSDKVersion()).c_str(),
		a2w(TICManager::GetInstance().GetTRTCCloud()->getSDKVersion()).c_str(),
		a2w(GetTEduBoardVersion()).c_str());

	GetDlgItem(IDC_VERSION)->SetWindowText(version);

	return TRUE;
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
		videoDlg_->ShowWindow(SW_SHOW);
		boardDlg_->ShowWindow(SW_HIDE);
		break;
	case 1:
		videoDlg_->ShowWindow(SW_HIDE);
		boardDlg_->ShowWindow(SW_SHOW);
		break;
	case 2:
		videoDlg_->ShowWindow(SW_HIDE);
		boardDlg_->ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}
}

void CTICDemoDlg::OnBtnLogin()
{
	btnLogin_.EnableWindow(FALSE);

	int nSelectIndex = cbUser_.GetCurSel();
	const std::vector<UserInfo>& userInfoList = Config::GetInstance().UserInfoList();

	theApp.setUserId(userInfoList[nSelectIndex].userid);
	theApp.setUserSig(userInfoList[nSelectIndex].usersig);

	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().Login(userInfoList[nSelectIndex].userid, userInfoList[nSelectIndex].usersig, [this, weakSelf](TICModule module, int code, const char *desc){
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}
		
		if (code == 0) {
			UpdateUI(UserState::Login);
		}
		else {
			UpdateUI(UserState::Init);
			showErrorMsg(module, code, desc);
		}
	});
}

void CTICDemoDlg::OnBtnLogout()
{
	btnLogout_.EnableWindow(FALSE);

	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().Logout([this, weakSelf](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0) {
			UpdateUI(UserState::Init);
		}
		else {
			UpdateUI(UserState::Login);
			showErrorMsg(module, code, desc);
		}
	});
}

void CTICDemoDlg::OnBtnCreateRoom()
{
	CString str;
	editClassId_.GetWindowText(str);
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("������д�����"), MB_OK);
		return;
	}
	int classId = atoi( w2a( str.GetBuffer() ).c_str() );
	if (classId == 0)
	{
		AfxMessageBox(_T("���������"), MB_OK);
		return;
	}

	btnCreateRoom_.EnableWindow(FALSE);

	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().CreateClassroom(classId, TIC_CLASS_SCENE_VIDEO_CALL,[this, weakSelf](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0) {
			UpdateUI(UserState::Login);
			AfxMessageBox(_T("�������óɹ�����\"�������\""), MB_OK);
		}
		else {
			UpdateUI(UserState::Login);
			
			if (code == 10021) {
				AfxMessageBox(_T("�ÿ����ѱ����˴�������\"�������\""), MB_OK);
			}
			else if (code == 10025) {
				AfxMessageBox(_T("�ÿ����Ѵ�������\"�������\""), MB_OK);
			}
			else
			{
				showErrorMsg(module, code, desc);
			}
		}
	});
}

void CTICDemoDlg::OnBtnDestroyRoom()
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

	btnDestroyRoom_.EnableWindow(FALSE);

	std::weak_ptr< CTICDemoDlg> weakSelf = this->shared_from_this();
	TICManager::GetInstance().DestroyClassroom(classId, [this, weakSelf](TICModule module, int code, const char *desc) {
		std::shared_ptr<CTICDemoDlg> self = weakSelf.lock();
		if (!self)
		{
			return;
		}

		if (code == 0) {
			UpdateUI(UserState::Login);
			AfxMessageBox(_T("���ٷ���ɹ�"), MB_OK);
		}
		else {
			UpdateUI(UserState::Login);
			showErrorMsg(module, code, desc);
		}
	});
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

	btnJoinRoom_.EnableWindow(FALSE);

	TICClassroomOption option;
	option.classId = classId;
	option.boardCallback = boardDlg_.get();

	theApp.setClassId(classId);

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
			UpdateUI(UserState::Login);

			if (code == 10015) {
				AfxMessageBox(_T("���ò����ڣ���\"��������\""), MB_OK);
			}
			else {
				showErrorMsg(module, code, desc);
			}
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
			UpdateUI(UserState::InRoom);
			showErrorMsg(module, code, desc);
		}
	});
}

void CTICDemoDlg::onTICForceOffline()
{
	UpdateUI(UserState::Init);

	AfxMessageBox(_T("�˺��������ط���½"), MB_OK);
}

void CTICDemoDlg::onTICUserSigExpired()
{
	AfxMessageBox(_T("Sig����"), MB_OK);
}

void CTICDemoDlg::UpdateUI(UserState state)
{
	if (state != UserState::Unknown) {
		state_ = state;
	}
	switch (state_)
	{
	case UserState::Init:
	{
		editClassId_.EnableWindow(TRUE);
		btnLogin_.EnableWindow(TRUE);
		btnLogout_.EnableWindow(FALSE);
		btnCreateRoom_.EnableWindow(FALSE);
		btnDestroyRoom_.EnableWindow(FALSE);
		btnJoinRoom_.EnableWindow(FALSE);
		btnQuitRoom_.EnableWindow(FALSE);
		cbUser_.EnableWindow(TRUE);
		break;
	}
	case UserState::Login:
	{
		editClassId_.EnableWindow(TRUE);
		btnLogin_.EnableWindow(FALSE);
		btnLogout_.EnableWindow(TRUE);
		btnCreateRoom_.EnableWindow(TRUE);
		btnDestroyRoom_.EnableWindow(TRUE);
		btnJoinRoom_.EnableWindow(TRUE);
		btnQuitRoom_.EnableWindow(FALSE);
		cbUser_.EnableWindow(FALSE);
		break;
	}
	case UserState::InRoom:
	{
		editClassId_.EnableWindow(FALSE);
		btnLogin_.EnableWindow(FALSE);
		btnLogout_.EnableWindow(FALSE);
		btnCreateRoom_.EnableWindow(FALSE);
		btnDestroyRoom_.EnableWindow(FALSE);
		btnJoinRoom_.EnableWindow(FALSE);
		btnQuitRoom_.EnableWindow(TRUE);
		cbUser_.EnableWindow(FALSE);
		break;
	}
	default: break;
	}

	videoDlg_->UpdateUI(state_);
	boardDlg_->UpdateUI();
}
