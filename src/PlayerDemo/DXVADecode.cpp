#include "DXVADecode.h"
#include "FFmpegDxva2.h"
#include "D3DVidRender.h"

extern "C" {
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#pragma comment(lib,"swscale.lib")

AVPixelFormat GetHwFormat(AVCodecContext* s, const AVPixelFormat* pix_fmts)
{
	InputStream* ist = (InputStream*)s->opaque;
	ist->active_hwaccel_id = HWACCEL_DXVA2;
	ist->hwaccel_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
	return ist->hwaccel_pix_fmt;
}

DXVADecode::DXVADecode()
{
}

DXVADecode::~DXVADecode()
{
}

bool DXVADecode::HwAcceleration(HWND hwnd)
{
	bAccel = false;
	bool reset = false;

	std::lock_guard<std::mutex> lck(mMtx);

	if (mCodecCtx && mCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO) {

		switch (codec->id) {

		case AV_CODEC_ID_MPEG2VIDEO:
		case AV_CODEC_ID_H264:
		case AV_CODEC_ID_VC1:
		case AV_CODEC_ID_WMV3:
		case AV_CODEC_ID_HEVC:
		case AV_CODEC_ID_VP9: 
		{
			mCodecCtx->thread_count = 1;  // Multithreading is apparently not compatible with hardware decoding
			InputStream* ist = new InputStream();
			ist->hwaccel_id = HWACCEL_AUTO;
			ist->active_hwaccel_id = HWACCEL_AUTO;
			ist->hwaccel_device = (char*)"dxva2";
			ist->dec = codec;
			ist->dec_ctx = mCodecCtx;

			mCodecCtx->opaque = ist;

			if (dxva2_init(mCodecCtx, hwnd) == 0) {
				mCodecCtx->get_buffer2 = ist->hwaccel_get_buffer;
				mCodecCtx->get_format = GetHwFormat;
				mCodecCtx->thread_safe_callbacks = 1;
				bAccel = true;
				break;
			}
			bAccel = false;
			reset = true;
		}
		default:
			bAccel = false;
			break;
		}
	}

	if (!bAccel) {

		if (reset) {
			avcodec_close(mCodecCtx);
			mCodecCtx = avcodec_alloc_context3(codec);
			avcodec_parameters_to_context(mCodecCtx, mPara.get());
			avcodec_open2(mCodecCtx, 0, 0);
		}

		mD3DVidRender = new CD3DVidRender();

		mD3DVidRender->InitD3D_YUV(hwnd, mCodecCtx->width, mCodecCtx->height);

		pFrameBGR = av_frame_alloc();
		out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
			mCodecCtx->width, mCodecCtx->height, 1));
		av_image_fill_arrays(pFrameBGR->data, ((AVPicture*)pFrameBGR)->linesize, out_buffer, AV_PIX_FMT_YUV420P,
			mCodecCtx->width, mCodecCtx->height, 1);
		img_convert_ctx = sws_getContext(mCodecCtx->width, mCodecCtx->height, mCodecCtx->pix_fmt,
			mCodecCtx->width, mCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	}

	return bAccel;
}

void DXVADecode::RetrieveFrame(std::shared_ptr<AVFrame> frame)
{

	if (bAccel) {
		dxva2_retrieve_data_call(mCodecCtx, frame.get());
	}
	else {
		if (img_convert_ctx && pFrameBGR && out_buffer)
		{
			//转换数据并渲染
			sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, mCodecCtx->height,
				pFrameBGR->data, pFrameBGR->linesize);

			mD3DVidRender->Render_YUV(out_buffer, frame->width, frame->height);
		}
	}
}
