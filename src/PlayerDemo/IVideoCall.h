#pragma once

struct AVFrame;

class IVideoCall
{
public:

	//virtual void Init(int width, int height) = 0;

	//不管成功与否都释放frame空间
	virtual void Repaint(std::shared_ptr<AVFrame> frame) = 0;
};

