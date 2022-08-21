// IsoRegister.cpp

#include "StdAfx.h"

#include "../../Common/RegisterArc.h"

#include "IsoHandler.h"

namespace NArchive {
namespace NIso {

static const Byte k_Signature[] = { 'C', 'D', '0', '0', '1' };

REGISTER_ARC_I(
  "Iso", "iso img", 0, 0xE7,
  k_Signature,
  NArchive::NIso::kStartPos + 1,
  0,
  NULL)

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_37(void) {
    return NArchive::NIso::g_ArcInfo.Flags;
}
#endif