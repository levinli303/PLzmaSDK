// Deflate64Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "DeflateDecoder.h"
#if !defined(Z7_EXTRACT_ONLY) && !defined(Z7_DEFLATE_EXTRACT_ONLY)
#include "DeflateEncoder.h"
#endif

namespace NCompress {
namespace NDeflate {

REGISTER_CODEC_CREATE(CreateDec, NDecoder::CCOMCoder64())

#if !defined(Z7_EXTRACT_ONLY) && !defined(Z7_DEFLATE_EXTRACT_ONLY)
REGISTER_CODEC_CREATE(CreateEnc, NEncoder::CCOMCoder64())
#else
#define CreateEnc NULL
#endif

REGISTER_CODEC_2(Deflate64, CreateDec, CreateEnc, 0x40109, "Deflate64")

}}


#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_69(void) {
    return static_cast<uint64_t>(NCompress::NDeflate::g_CodecInfo_Deflate64.Id);
}
#endif