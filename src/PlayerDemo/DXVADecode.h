#pragma once

#include "DecodeClass.h"
#include <Windows.h>

class CD3DVidRender;

class DXVADecode : public DecodeClass
{
public:
	DXVADecode();
	~DXVADecode();

	virtual bool HwAcceleration(HWND hwnd) override;

	virtual void RetrieveFrame(std::shared_ptr<AVFrame> frame) override;

private:

	AVFrame* pFrameBGR = nullptr;
	uint8_t* out_buffer = nullptr;
	struct SwsContext* img_convert_ctx = nullptr;

	CD3DVidRender* mD3DVidRender = nullptr;

};

