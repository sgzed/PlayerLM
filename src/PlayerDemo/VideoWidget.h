#pragma once

#include <Windows.h>
#include <QWidget>
#include "IVideoCall.h"

class VideoWidget :public QWidget, public IVideoCall
{
	Q_OBJECT

public:
	VideoWidget(QWidget* parent = Q_NULLPTR);
	~VideoWidget();

	//bool InitWndClass();

	void Init(DecodeClass* decode) override;

	void SetWindowOnWorker(int width, int height);

	//不管成功与否都释放frame空间
	virtual void Repaint(std::shared_ptr<AVFrame> frame);

protected:
	//是否启用了硬解码
	bool bAccel = false;
};

