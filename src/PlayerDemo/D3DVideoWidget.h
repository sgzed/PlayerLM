#pragma once

#include <qwidget.h>
#include "IVideoCall.h"

class D3DVideoWidget : public QWidget, public IVideoCall
{
	Q_OBJECT

public:
	
	void Init(DecodeClass* decode) override;

	//不管成功与否都释放frame空间
	virtual void Repaint(std::shared_ptr<AVFrame> frame);

	D3DVideoWidget(QWidget* parent = nullptr);

	~D3DVideoWidget();

	//是否启用了硬解码
	bool bAccel = false;
};

