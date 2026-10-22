// Archive/IsoItem.h

#ifndef ZIP7_INC_ARCHIVE_ISO_ITEM_H
#define ZIP7_INC_ARCHIVE_ISO_ITEM_H

#include "../../../../C/CpuArch.h"

#include "../../../Common/MyString.h"
#include "../../../Common/MyBuffer.h"

#include "../../../Windows/TimeUtils.h"

#include "IsoHeader.h"

namespace NArchive {
namespace NIso {

struct CRecordingDateTime
{
  Byte Year;
  Byte Month;
  Byte Day;
  Byte Hour;
  Byte Minute;
  Byte Second;
  signed char GmtOffset; // min intervals from -48 (West) to +52 (East) recorded.
  
  bool GetFileTime(NWindows::NCOM::CPropVariant &prop) const
  {
    UInt64 v;
    const bool res = NWindows::NTime::GetSecondsSince1601(Year + 1900, Month, Day, Hour, Minute, Second, v);
    if (res)
    {
      v = (UInt64)((Int64)v - (Int64)((Int32)GmtOffset * 15 * 60));
      v *= 10000000;
      prop.SetAsTimeFrom_Ft64_Prec(v, k_PropVar_TimePrec_Base);
    }
    return res;
  }
};

enum EPx
{
  k_Px_Mode,
  k_Px_Links,
  k_Px_User,
  k_Px_Group,
  k_Px_SerialNumber

  // k_Px_Num
};

/*
enum ETf
{
  k_Tf_CTime,
  k_Tf_MTime,
  k_Tf_ATime,
  k_Tf_Attrib,
  k_Tf_Backup,
  k_Tf_Expiration,
  k_Tf_Effective

  // k_Tf_Num
};
*/

struct CDirRecord
{
  UInt32 ExtentLocation;
  UInt32 Size;
  CRecordingDateTime DateTime;
  Byte FileFlags;
  Byte FileUnitSize;
  Byte InterleaveGapSize;
  Byte ExtendedAttributeRecordLen;
  UInt16 VolSequenceNumber;
  CByteBuffer FileId;
  CByteBuffer SystemUse;
  // The joined Rock Ridge name (NM records).
  CByteBuffer SuspName;

  bool AreMultiPartEqualWith(const CDirRecord &a) const
  {
    return FileId == a.FileId
        && (FileFlags & (~NFileFlags::kNonFinalExtent)) ==
        (a.FileFlags & (~NFileFlags::kNonFinalExtent));
  }

  bool IsDir() const { return (FileFlags & NFileFlags::kDirectory) != 0; }
  bool IsNonFinalExtent() const { return (FileFlags & NFileFlags::kNonFinalExtent) != 0; }

  bool IsSystemItem() const
  {
    if (FileId.Size() != 1)
      return false;
    Byte b = *(const Byte *)FileId;
    return (b == 0 || b == 1);
  }

  
  // Finds the next record with the id at or after offset, and moves offset past it.
  const Byte* FindNextSuspRecord(unsigned &offset, Byte id0, Byte id1, unsigned &lenRes) const
  {
    lenRes = 0;
    if (SystemUse.Size() < offset)
      return NULL;
    const Byte *p = (const Byte *)SystemUse + offset;
    unsigned rem = (unsigned)(SystemUse.Size() - offset);
    // Records without data (like RE) are only 4 bytes long.
    while (rem >= 4)
    {
      unsigned len = p[2];
      if (len < 3 || len > rem)
        return NULL;
      if (p[0] == id0 && p[1] == id1 && p[3] == 1)
      {
        if (len < 4)
          return NULL; // Check it
        lenRes = len - 4;
        offset = (unsigned)(p + len - (const Byte *)SystemUse);
        return p + 4;
      }
      p += len;
      rem -= len;
    }
    return NULL;
  }

  const Byte* FindSuspRecord(unsigned skipSize, Byte id0, Byte id1, unsigned &lenRes) const
  {
    unsigned offset = skipSize;
    return FindNextSuspRecord(offset, id0, id1, lenRes);
  }

  // Joins the Rock Ridge name, which can be split into several NM records.
  void ReadSuspName(unsigned skipSize)
  {
    SuspName.Free();
    CByteBuffer name;
    bool continues = false;
    unsigned offset = skipSize;
    unsigned len;
    for (;;)
    {
      const Byte *p = FindNextSuspRecord(offset, 'N', 'M', len);
      if (!p)
        break;
      if (!continues)
        name.Free();
      continues = false;
      // "." and ".." (flags 2 and 4) aren't names of items.
      if (len < 2 || (p[0] != 0 && p[0] != 1))
        continue;
      continues = (p[0] == 1);
      CByteBuffer joined;
      joined.Alloc(name.Size() + len - 1);
      if (name.Size() != 0)
        memcpy(joined, name, name.Size());
      memcpy((Byte *)joined + name.Size(), p + 1, len - 1);
      name = joined;
    }
    SuspName = name;
  }

  const Byte* GetNameCur(bool checkSusp, unsigned skipSize, unsigned &nameLenRes) const
  {
    const Byte *res;
    unsigned len;
    (void)skipSize;
    if (checkSusp && SuspName.Size() != 0)
    {
      res = (const Byte *)SuspName;
      len = (unsigned)SuspName.Size();
    }
    else
    {
      res = (const Byte *)FileId;
      len = (unsigned)FileId.Size();
    }
    unsigned i;
    for (i = 0; i < len; i++)
      if (res[i] == 0)
        break;
    nameLenRes = i;
    return res;
  }


  // Components are like in libarchive. A component that isn't continued (flag 1)
  // ends with a separator, also at the end of an SL record, like in Linux.
  bool GetSymLink(unsigned skipSize, AString &link) const
  {
    link.Empty();
    bool continues = false;
    const char *separator = "";
    unsigned offset = skipSize;
    unsigned len;
    for (;;)
    {
      const Byte *p = FindNextSuspRecord(offset, 'S', 'L', len);
      if (!p)
        break;
      if (!continues)
      {
        link.Empty();
        separator = "";
      }
      continues = false;
      if (len < 1 || (p[0] != 0 && p[0] != 1))
        continue;
      continues = (p[0] == 1);
      p++;
      len--;
      while (len >= 2)
      {
        const unsigned flags = p[0];
        const unsigned cl = p[1];
        p += 2;
        len -= 2;
        if (cl > len)
          break;
        link += separator;
        separator = "/";
        bool known = true;
        switch (flags)
        {
          case 0:
          case 1:
            for (unsigned i = 0; i < cl; i++)
              link += (char)p[i];
            if (flags == 1)
              separator = "";
            break;
          case 2: link += '.'; break;
          case 4: link += ".."; break;
          case 8: link.Add_Slash(); separator = ""; break;
          case 0x10: link = "ROOT"; break;
          case 0x20: link += "hostname"; break;
          default: known = false; break;
        }
        if (!known)
          break;
        p += cl;
        len -= cl;
      }
    }
    return !link.IsEmpty();
  }

  static bool GetLe32Be32(const Byte *p, UInt32 &dest)
  {
    UInt32 v1 = GetUi32(p);
    UInt32 v2 = GetBe32(p + 4);
    if (v1 == v2)
    {
      dest = v1;
      return true;
    }
    return false;
  }


  bool GetPx(unsigned skipSize, unsigned pxType, UInt32 &val) const
  {
    val = 0;
    const Byte *p = NULL;
    unsigned len = 0;
    p = FindSuspRecord(skipSize, 'P', 'X', len);
    if (!p)
      return false;
    // px.Clear();
    if (len < (pxType + 1) * 8)
      return false;

    return GetLe32Be32(p + pxType * 8, val);
  }

  /*
  bool GetTf(int skipSize, unsigned pxType, CRecordingDateTime &t) const
  {
    const Byte *p = NULL;
    unsigned len = 0;
    p = FindSuspRecord(skipSize, 'T', 'F', len);
    if (!p)
      return false;
    if (len < 1)
      return false;
    Byte flags = *p++;
    len--;

    unsigned step = 7;
    if (flags & 0x80)
    {
      step = 17;
      return false;
    }

    if ((flags & (1 << pxType)) == 0)
      return false;

    for (unsigned i = 0; i < pxType; i++)
    {
      if (len < step)
        return false;
      if (flags & (1 << i))
      {
        p += step;
        len -= step;
      }
    }

    if (len < step)
      return false;
    
    t.Year = p[0];
    t.Month = p[1];
    t.Day = p[2];
    t.Hour = p[3];
    t.Minute = p[4];
    t.Second = p[5];
    t.GmtOffset = (signed char)p[6];

    return true;
  }
  */

  bool CheckSusp(const Byte *p, unsigned &startPos) const
  {
    if (p[0] == 'S' &&
        p[1] == 'P' &&
        p[2] == 0x7 &&
        p[3] == 0x1 &&
        p[4] == 0xBE &&
        p[5] == 0xEF)
    {
      startPos = p[6];
      return true;
    }
    return false;
  }

  bool CheckSusp(unsigned &startPos) const
  {
    const Byte *p = (const Byte *)SystemUse;
    const size_t len = SystemUse.Size();
    const unsigned kMinLen = 7;
    if (len < kMinLen)
      return false;
    if (CheckSusp(p, startPos))
      return true;
    const unsigned kOffset2 = 14;
    if (len < kOffset2 + kMinLen)
      return false;
    return CheckSusp(p + kOffset2, startPos);
  }
};

}}

#endif
