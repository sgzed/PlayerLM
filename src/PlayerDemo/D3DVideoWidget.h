#pragma once

#include <qwidget.h>
#include "IVideoCall.h"

struct AVCodec;
struct AVCodecContext;

class D3DVideoWidget : public QWidget, public IVideoCall
{
	Q_OBJECT

public:
	
	bool Init(AVCodec* codec,AVCodecContext* codecCtx);

	//不管成功与否都释放frame空间
	virtual void Repaint(std::shared_ptr<AVFrame> frame);

	D3DVideoWidget(QWidget* parent = nullptr);

	~D3DVideoWidget();

private:
	AVCodecContext* mCodecCtx;
};

