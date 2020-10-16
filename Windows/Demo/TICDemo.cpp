#include "stdafx.h"
#include "TICDemo.h"
#include "TICDemoDlg.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTICDemoApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CTICDemoApp::CTICDemoApp()
{

}

BOOL CTICDemoApp::InitInstance()
{
	// ��ȡ�����ļ�
	bool bRet =	Config::GetInstance().ReadConfig();
	if (!bRet)
	{
		::MessageBox(NULL, _T("��ȡ������Ϣ����"), _T("����"), MB_OK);
		return FALSE;
	}

	CWinApp::InitInstance();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	auto dlg = std::make_shared<CTICDemoDlg>();
	m_pMainWnd = dlg.get();
	INT_PTR nResponse = dlg->DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

CTICDemoApp theApp;


int main() { return _tWinMain(GetModuleHandle(NULL), NULL, _T(""), SW_SHOWDEFAULT); }
