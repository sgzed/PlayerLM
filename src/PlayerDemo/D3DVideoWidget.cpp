#include "D3DVideoWidget.h"
#include "DecodeClass.h"
#include "workerutil.h"

#define CLASSNAME L"D3dWndClass"

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,
	HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	static BOOL first = TRUE;   //标志
	std::vector<RECT>* pRect = (std::vector<RECT>*)dwData;
	//保存显示器信息
	MONITORINFO monitorinfo;
	monitorinfo.cbSize = sizeof(MONITORINFO);

	//获得显示器信息，将信息保存到monitorinfo中
	GetMonitorInfo(hMonitor, &monitorinfo);
	//若检测到主屏
	if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY)
	{
		if (first)  //第一次检测到主屏
		{
			first = FALSE;
			pRect->push_back(monitorinfo.rcMonitor);
		}
		else //第二次检测到主屏,说明所有的监视器都已经检测了一遍，故可以停止检测了
		{
			first = TRUE;    //标志复位
			return FALSE;    //结束检测
		}
	}
	else
	{
		pRect->push_back(monitorinfo.rcMonitor);
	}
	first = TRUE;     // 恢复主屏标记为初始状态
	return TRUE;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProc(hWnd, message, wParam, lParam);
}


D3DVideoWidget::D3DVideoWidget() {
	InitWndClass();
}

bool D3DVideoWidget::InitWndClass()
{
	HINSTANCE hinstance = (HINSTANCE)GetModuleHandle(NULL);

	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hinstance;
	wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = CLASSNAME;
	wcx.hIconSm = NULL;

	RegisterClassEx(&wcx);

	m_hWnd = CreateWindow(
		CLASSNAME,        // name of window class 
		L"Sample",            // title-bar string 
		WS_OVERLAPPEDWINDOW, // top-level window 
		CW_USEDEFAULT,       // default horizontal position 
		CW_USEDEFAULT,       // default vertical position 
		500,       // default width 
		500,       // default height 
		(HWND)NULL,         // no owner window 
		(HMENU)NULL,        // use class menu 
		hinstance,           // handle to application instance 
		(LPVOID)NULL);      // no window-creation data 

	EnableWindow(m_hWnd, false);
	WorkerWinManger::Instance()->AddToDesktop(m_hWnd);

	RECT winRect;
	GetWindowRect(m_hWnd,&winRect);

	std::vector<RECT> vRect;
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&vRect);  // 枚举所有显示器的Rect

	RECT rectCmb = vRect[0];
	for (auto& rect : vRect) {
		if (rectCmb.left > rect.left)  rectCmb.left = rect.left;
		if (rectCmb.right < rect.right) rectCmb.right = rect.right;
		if (rectCmb.top > rect.top) rectCmb.top = rect.top;
		if (rectCmb.bottom < rect.bottom) rectCmb.bottom = rect.bottom;
	}

	SetWindowOnWorker(rectCmb.right - rectCmb.left, rectCmb.bottom - rectCmb.top);

	return true;
}

void D3DVideoWidget::Init(DecodeClass* decode)
{

	if (!m_hWnd)	return;

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);


	LONG_PTR Style = ::GetWindowLongPtr(m_hWnd, GWL_STYLE);
	Style = Style & ~WS_CAPTION & ~WS_SYSMENU & ~WS_SIZEBOX;
	::SetWindowLongPtr(m_hWnd, GWL_STYLE, Style);
	auto hMenu = GetMenu(m_hWnd);
	SetMenu(m_hWnd, NULL);

	mDecode = decode;
	bAccel = false;
	bAccel = decode->HwAcceleration(m_hWnd);
	return;
}

void D3DVideoWidget::SetWindowOnWorker(int width, int height)
{
	WorkerWinManger::Instance()->MoveWindowOnWorkW(m_hWnd, QRect(0, 0, width, height));
}

void D3DVideoWidget::Repaint(std::shared_ptr<AVFrame> frame)
{
	mDecode->RetrieveFrame(frame);
}

D3DVideoWidget::~D3DVideoWidget()
{
	WorkerWinManger::Instance()->RemoveFormDesktop(m_hWnd);
}
