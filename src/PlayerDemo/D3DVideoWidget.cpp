#include "D3DVideoWidget.h"
#include "ffmpeg_dxva2.h"

AVPixelFormat GetHwFormat(AVCodecContext* s, const AVPixelFormat* pix_fmts)
{
	InputStream* ist = (InputStream*)s->opaque;
	ist->active_hwaccel_id = HWACCEL_DXVA2;
	ist->hwaccel_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
	return ist->hwaccel_pix_fmt;
}

D3DVideoWidget::D3DVideoWidget(QWidget* parent) :QWidget(parent) {

}

bool D3DVideoWidget::Init(AVCodec* codec,AVCodecContext* codecCtx)
{
	mCodecCtx = codecCtx;
	if (mCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO) {

		switch (codec->id)
		{
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
			ist->hwaccel_device = "dxva2";
			ist->dec = codec;
			ist->dec_ctx = mCodecCtx;

			mCodecCtx->opaque = ist;
			HWND hwnd = (HWND)this->winId();
			if (dxva2_init(mCodecCtx, hwnd) == 0)
			{
				mCodecCtx->get_buffer2 = ist->hwaccel_get_buffer;
				mCodecCtx->get_format = GetHwFormat;
				mCodecCtx->thread_safe_callbacks = 1;
				return true;
			}

			return false;
		}
		default:
			return false;
		}
	}
	return false;
}

void D3DVideoWidget::Repaint(std::shared_ptr<AVFrame> frame)
{
	dxva2_retrieve_data_call(mCodecCtx, frame.get());
}

D3DVideoWidget::~D3DVideoWidget()
{
}
