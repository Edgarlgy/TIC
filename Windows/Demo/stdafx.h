#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��


#include <assert.h>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <io.h>
#include <direct.h>
#include <afxinet.h>
#include <thread>

#include "..\SDK\TIC\TICManager.h"

#define WM_UPDATE_THUMB_IMAGE WM_USER+100

#define ThumpWidth 134
#define ThumpHeight 75

std::wstring a2w(const std::string &str, unsigned int codePage = CP_ACP);
std::string w2a(const std::wstring &wstr, unsigned int codePage = CP_ACP);

std::string  i2s(int num);

void showErrorMsg(TICModule module, int code, const char* desc);

void stretchImage(CImage *pImage, CImage *ResultImage, int outWidth, int outHeight);

std::string savePic(std::string strImgUrl);