#include "D3DVideoWidget.h"
#include "DecodeClass.h"


D3DVideoWidget::D3DVideoWidget(QWidget* parent) :QWidget(parent) {

}

void D3DVideoWidget::Init(DecodeClass* decode)
{
	mDecode = decode;
	bAccel = false;
	HWND hwnd = (HWND)this->winId();
	bAccel = decode->HwAcceleration(hwnd);
	return;
}

void D3DVideoWidget::Repaint(std::shared_ptr<AVFrame> frame)
{
	mDecode->RetrieveFrame(frame);
	
}

D3DVideoWidget::~D3DVideoWidget()
{
}
