// NsisRegister.cpp

#include "StdAfx.h"

#include "../../Common/RegisterArc.h"

#include "NsisHandler.h"

namespace NArchive {
namespace NNsis {

REGISTER_ARC_I(
  "Nsis", "nsis", NULL, 0x9,
  kSignature,
  4,
  NArcInfoFlags::kFindSignature |
  NArcInfoFlags::kUseGlobalOffset,
  NULL)

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_67(void) {
    return NArchive::NNsis::g_ArcInfo.Flags;
}
#endif