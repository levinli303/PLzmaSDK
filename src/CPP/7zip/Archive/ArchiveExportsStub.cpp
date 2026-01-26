// Minimal ArchiveExports implementation for CreateArchiver
// Needed by DllExports2.cpp::CreateObject
// This is a subset of the full ArchiveExports.cpp that only provides
// what's needed for static linking with CCodecs/CArc approach

#include "StdAfx.h"

#include "../../../C/7zVersion.h"
#include "../../Common/ComTry.h"
#include "../../Windows/PropVariant.h"
#include "../Common/RegisterArc.h"

// These are the same as in LoadCodecs.cpp
// Both define them, but they share the same data via RegisterArc() calls
static const unsigned kNumArcsMax = 72;
static unsigned g_NumArcs = 0;
static const CArcInfo *g_Arcs[kNumArcsMax];

void RegisterArc(const CArcInfo *arcInfo) throw();

// Forward declare to avoid duplicate definition
// The real RegisterArc is in LoadCodecs.cpp
// This just needs to satisfy the linker for formats that reference it
extern void RegisterArc(const CArcInfo *arcInfo) throw();

Z7_DEFINE_GUID(CLSID_CArchiveHandler,
    k_7zip_GUID_Data1,
    k_7zip_GUID_Data2,
    k_7zip_GUID_Data3_Common,
    0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00);

#define CLS_ARC_ID_ITEM(cls) ((cls).Data4[5])

static int FindFormatCalssId(const GUID *clsid)
{
  GUID cls = *clsid;
  CLS_ARC_ID_ITEM(cls) = 0;
  if (cls != CLSID_CArchiveHandler)
    return -1;
  const Byte id = CLS_ARC_ID_ITEM(*clsid);
  for (unsigned i = 0; i < g_NumArcs; i++)
    if (g_Arcs[i]->Id == id)
      return (int)i;
  return -1;
}

STDAPI CreateArchiver(const GUID *clsid, const GUID *iid, void **outObject);
STDAPI CreateArchiver(const GUID *clsid, const GUID *iid, void **outObject)
{
  COM_TRY_BEGIN
  {
    const int needIn = (*iid == IID_IInArchive);
    const int needOut = (*iid == IID_IOutArchive);
    if (!needIn && !needOut)
      return E_NOINTERFACE;
    const int formatIndex = FindFormatCalssId(clsid);
    if (formatIndex < 0)
      return CLASS_E_CLASSNOTAVAILABLE;
    
    const CArcInfo &arc = *g_Arcs[formatIndex];
    if (needIn)
    {
      *outObject = arc.CreateInArchive();
      ((IInArchive *)*outObject)->AddRef();
    }
    #ifndef Z7_SFX
    else
    {
      *outObject = arc.CreateOutArchive();
      if (*outObject)
        ((IOutArchive *)*outObject)->AddRef();
      else
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    #else
    else
      return CLASS_E_CLASSNOTAVAILABLE;
    #endif
  }
  return S_OK;
  COM_TRY_END
}
