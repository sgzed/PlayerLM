#include "DemuxClass.h"

extern "C" {
	#include "libavformat/avformat.h"
}

DemuxClass::DemuxClass()
{
}

DemuxClass::~DemuxClass()
{
}

bool DemuxClass::Open(const char* url)
{
	return false;
}
