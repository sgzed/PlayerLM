#include "VideoWidget.h"

#include "DecodeClass.h"
#include "workerutil.h"


VideoWidget::VideoWidget(QWidget* parent)
	:QWidget(parent){
	
}


void VideoWidget::Init(DecodeClass* decode)
{

	show();
	//showMinimized();

	HWND m_hWnd = (HWND)winId();
	          
	WorkerWinManger::Instance()->AddToDesktop(m_hWnd);
	SetWindowOnWorker(1920*2,1080);

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

void VideoWidget::SetWindowOnWorker(int width, int height)
{
	HWND m_hWnd = (HWND)winId();
	WorkerWinManger::Instance()->MoveWindowOnWorkW(m_hWnd, QRect(0, 0, width, height));
}

void VideoWidget::Repaint(std::shared_ptr<AVFrame> frame)
{
	mDecode->RetrieveFrame(frame);
}

VideoWidget::~VideoWidget()
{
	HWND m_hWnd = (HWND)winId();
	WorkerWinManger::Instance()->RemoveFormDesktop(m_hWnd);
}
