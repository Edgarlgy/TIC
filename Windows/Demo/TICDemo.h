#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"

class CTICDemoApp : public CWinApp
{
	DECLARE_MESSAGE_MAP()
public:
	CTICDemoApp();

	virtual BOOL InitInstance();

	void setUserId(const std::string& uid) { this->uid_ = uid; };
	void setUserSig(const std::string& sig) { this->usig_ = sig; };
	void setClassId(int cls) { this->clsid_ = cls; }

	std::string getUserId() { return uid_; };
	std::string getUserSig() { return usig_; }
	int getClassId() { return clsid_; }

private:
	std::string uid_;
	std::string usig_;
	int clsid_ = 0;
};

extern CTICDemoApp theApp;
