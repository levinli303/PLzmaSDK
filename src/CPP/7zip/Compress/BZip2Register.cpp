// BZip2Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "BZip2Decoder.h"
#if !defined(Z7_EXTRACT_ONLY) && !defined(Z7_BZIP2_EXTRACT_ONLY)
#include "BZip2Encoder.h"
#endif

namespace NCompress {
namespace NBZip2 {

REGISTER_CODEC_CREATE(CreateDec, CDecoder)

#if !defined(Z7_EXTRACT_ONLY) && !defined(Z7_BZIP2_EXTRACT_ONLY)
REGISTER_CODEC_CREATE(CreateEnc, CEncoder)
#else
#define CreateEnc NULL
#endif

REGISTER_CODEC_2(BZip2, CreateDec, CreateEnc, 0x40202, "BZip2")

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_18(void) {
    return static_cast<uint64_t>(NCompress::NBZip2::g_CodecInfo_BZip2.Id);
}
#endif
