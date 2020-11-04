#pragma once

#include <Windows.h>
#include <QObject>
#include "IVideoCall.h"

class D3DVideoWidget :public QObject, public IVideoCall
{
	Q_OBJECT

public:
	D3DVideoWidget();
	~D3DVideoWidget();

	bool InitWndClass();

	void Init(DecodeClass* decode) override;

	//不管成功与否都释放frame空间
	virtual void Repaint(std::shared_ptr<AVFrame> frame);

protected:
	//是否启用了硬解码
	bool bAccel = false;

private:
	HWND m_hWnd;
};

