#include "D3DVideoWidget.h"
#include "DecodeClass.h"
#include "workerutil.h"

#define CLASSNAME L"D3dWndClass"

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

	//WorkerWinManger::Instance()->AddToDesktop(m_hWnd);
	//WorkerWinManger::Instance()->MoveWindowOnWorkW(m_hWnd, QRect(0, 0, 1920 * 2, 1080 * 1));

	return true;
}

void D3DVideoWidget::Init(DecodeClass* decode)
{

	if (!m_hWnd)	return;

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);


	//LONG_PTR Style = ::GetWindowLongPtr(m_hWnd, GWL_STYLE);
	//Style = Style & ~WS_CAPTION & ~WS_SYSMENU & ~WS_SIZEBOX;
	//::SetWindowLongPtr(m_hWnd, GWL_STYLE, Style);
	//auto hMenu = GetMenu(m_hWnd);
	//SetMenu(m_hWnd, NULL);

	mDecode = decode;
	bAccel = false;
	bAccel = decode->HwAcceleration(m_hWnd);
	return;
}

void D3DVideoWidget::Repaint(std::shared_ptr<AVFrame> frame)
{
	mDecode->RetrieveFrame(frame);
}

D3DVideoWidget::~D3DVideoWidget()
{
	WorkerWinManger::Instance()->RemoveFormDesktop(m_hWnd);
}
