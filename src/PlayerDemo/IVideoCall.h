#pragma once

#include <memory>

struct AVFrame;
class DecodeClass;

class IVideoCall
{
public:

	//virtual void Init(int width, int height) = 0;
	virtual void Init(DecodeClass* decode) = 0;

	virtual void SetWindowOnWorker(int width,int height) = 0;

	//不管成功与否都释放frame空间
	virtual void Repaint(std::shared_ptr<AVFrame> frame) = 0;

protected:
	DecodeClass* mDecode = 0;
};

