// Windows/FileIO.h

#ifndef __WINDOWS_FILE_IO_H
#define __WINDOWS_FILE_IO_H

#include "../Common/MyWindows.h"

#define _my_IO_REPARSE_TAG_MOUNT_POINT  (0xA0000003L)
#define _my_IO_REPARSE_TAG_SYMLINK      (0xA000000CL)
#define _my_IO_REPARSE_TAG_LX_SYMLINK   (0xA000001DL)

#define _my_SYMLINK_FLAG_RELATIVE 1

// what the meaning of that FLAG or field (2)?
#define _my_LX_SYMLINK_FLAG 2

#ifdef _WIN32

#if defined(_WIN32) && !defined(UNDER_CE)
#include <WinIoCtl.h>
#endif

#else

#include <sys/types.h>
#include <sys/stat.h>

#endif

#include "../Common/MyString.h"
#include "../Common/MyBuffer.h"

#include "Defs.h"

HRESULT GetLastError_noZero_HRESULT();

#define my_FSCTL_SET_REPARSE_POINT     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // REPARSE_DATA_BUFFER
#define my_FSCTL_GET_REPARSE_POINT     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)     // REPARSE_DATA_BUFFER
#define my_FSCTL_DELETE_REPARSE_POINT  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // REPARSE_DATA_BUFFER

namespace NWindows {
namespace NFile {

#if defined(_WIN32) && !defined(UNDER_CE)
bool FillLinkData(CByteBuffer &dest, const wchar_t *path, bool isSymLink, bool isWSL);
#endif

struct CReparseShortInfo final
{
  unsigned Offset;
  unsigned Size;

  bool Parse(const Byte *p, size_t size);
};

struct CReparseAttr final
{
  UInt32 Tag;
  UInt32 Flags;
  UString SubsName;
  UString PrintName;

  AString WslName;

  bool HeaderError;
  bool TagIsUnknown;
  bool MinorError;
  DWORD ErrorCode;

  CReparseAttr(): Tag(0), Flags(0) {}

  // Parse()
  // returns (true) and (ErrorCode = 0), if (it'a correct known link)
  // returns (false) and (ErrorCode = ERROR_REPARSE_TAG_INVALID), if unknown tag
  bool Parse(const Byte *p, size_t size);

  bool IsMountPoint()  const { return Tag == _my_IO_REPARSE_TAG_MOUNT_POINT; } // it's Junction
  bool IsSymLink_Win() const { return Tag == _my_IO_REPARSE_TAG_SYMLINK; }
  bool IsSymLink_WSL() const { return Tag == _my_IO_REPARSE_TAG_LX_SYMLINK; }

  bool IsRelative_Win() const { return Flags == _my_SYMLINK_FLAG_RELATIVE; }

  bool IsRelative_WSL() const
  {
    if (WslName.IsEmpty())
      return true;
    char c = WslName[0];
    return !IS_PATH_SEPAR(c);
  }

  // bool IsVolume() const;

  bool IsOkNamePair() const;
  UString GetPath() const;
};


#ifdef _WIN32

namespace NIO {

bool GetReparseData(CFSTR path, CByteBuffer &reparseData, BY_HANDLE_FILE_INFORMATION *fileInfo = NULL);
bool SetReparseData(CFSTR path, bool isDir, const void *data, DWORD size);
bool DeleteReparseData(CFSTR path);

class CFileBase  MY_UNCOPYABLE
{
protected:
  HANDLE _handle;
  
  bool Create(CFSTR path, DWORD desiredAccess,
      DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);

public:

  bool DeviceIoControl(DWORD controlCode, LPVOID inBuffer, DWORD inSize,
      LPVOID outBuffer, DWORD outSize, LPDWORD bytesReturned, LPOVERLAPPED overlapped = NULL) const
  {
    return BOOLToBool(::DeviceIoControl(_handle, controlCode, inBuffer, inSize,
        outBuffer, outSize, bytesReturned, overlapped));
  }

  bool DeviceIoControlOut(DWORD controlCode, LPVOID outBuffer, DWORD outSize, LPDWORD bytesReturned) const
  {
    return DeviceIoControl(controlCode, NULL, 0, outBuffer, outSize, bytesReturned);
  }

  bool DeviceIoControlOut(DWORD controlCode, LPVOID outBuffer, DWORD outSize) const
  {
    DWORD bytesReturned;
    return DeviceIoControlOut(controlCode, outBuffer, outSize, &bytesReturned);
  }

public:
  bool PreserveATime;
  #ifdef SUPPORT_DEVICE_FILE
  bool IsDeviceFile;
  bool SizeDefined;
  UInt64 Size; // it can be larger than real available size
  #endif

  CFileBase(): _handle(INVALID_HANDLE_VALUE), PreserveATime(false) {};
  ~CFileBase() { Close(); }

  bool Close() throw();

  bool GetPosition(UInt64 &position) const throw();
  bool GetLength(UInt64 &length) const throw();

  bool Seek(Int64 distanceToMove, DWORD moveMethod, UInt64 &newPosition) const throw();
  bool Seek(UInt64 position, UInt64 &newPosition) const throw();
  bool SeekToBegin() const throw();
  bool SeekToEnd(UInt64 &newPosition) const throw();
  
  bool GetFileInformation(BY_HANDLE_FILE_INFORMATION *info) const
    { return BOOLToBool(GetFileInformationByHandle(_handle, info)); }

  static bool GetFileInformation(CFSTR path, BY_HANDLE_FILE_INFORMATION *info)
  {
    // probably it can work for complex paths: unsupported by another things
    NIO::CFileBase file;
    if (!file.Create(path, 0, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS))
      return false;
    return file.GetFileInformation(info);
  }
};

#ifndef UNDER_CE
#define IOCTL_CDROM_BASE  FILE_DEVICE_CD_ROM
#define IOCTL_CDROM_GET_DRIVE_GEOMETRY  CTL_CODE(IOCTL_CDROM_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS)
// #define IOCTL_CDROM_MEDIA_REMOVAL  CTL_CODE(IOCTL_CDROM_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)

// IOCTL_DISK_GET_DRIVE_GEOMETRY_EX works since WinXP
#define my_IOCTL_DISK_GET_DRIVE_GEOMETRY_EX  CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)

struct my_DISK_GEOMETRY_EX final
{
  DISK_GEOMETRY Geometry;
  LARGE_INTEGER DiskSize;
  BYTE Data[1];
};
#endif

class CInFile final : public CFileBase
{
  #ifdef SUPPORT_DEVICE_FILE

  #ifndef UNDER_CE
  
  bool GetGeometry(DISK_GEOMETRY *res) const
    { return DeviceIoControlOut(IOCTL_DISK_GET_DRIVE_GEOMETRY, res, sizeof(*res)); }

  bool GetGeometryEx(my_DISK_GEOMETRY_EX *res) const
    { return DeviceIoControlOut(my_IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, res, sizeof(*res)); }

  bool GetCdRomGeometry(DISK_GEOMETRY *res) const
    { return DeviceIoControlOut(IOCTL_CDROM_GET_DRIVE_GEOMETRY, res, sizeof(*res)); }
  
  bool GetPartitionInfo(PARTITION_INFORMATION *res)
    { return DeviceIoControlOut(IOCTL_DISK_GET_PARTITION_INFO, LPVOID(res), sizeof(*res)); }
  
  #endif

  void CorrectDeviceSize();
  void CalcDeviceSize(CFSTR name);
  
  #endif

public:
  bool Open(CFSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool OpenShared(CFSTR fileName, bool shareForWrite);
  bool Open(CFSTR fileName);

  #ifndef UNDER_CE

  bool OpenReparse(CFSTR fileName)
  {
    // 17.02 fix: to support Windows XP compatibility junctions:
    //   we use Create() with (desiredAccess = 0) instead of Open() with GENERIC_READ
    return
        Create(fileName, 0,
        // Open(fileName,
        FILE_SHARE_READ, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS);
  }
  
  #endif

  bool Read1(void *data, UInt32 size, UInt32 &processedSize) throw();
  bool ReadPart(void *data, UInt32 size, UInt32 &processedSize) throw();
  bool Read(void *data, UInt32 size, UInt32 &processedSize) throw();
  bool ReadFull(void *data, size_t size, size_t &processedSize) throw();
};

class COutFile final : public CFileBase
{
public:
  bool Open(CFSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool Open(CFSTR fileName, DWORD creationDisposition);
  bool Create(CFSTR fileName, bool createAlways);
  bool CreateAlways(CFSTR fileName, DWORD flagsAndAttributes);

  bool SetTime(const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime) throw();
  bool SetMTime(const FILETIME *mTime) throw();
  bool WritePart(const void *data, UInt32 size, UInt32 &processedSize) throw();
  bool Write(const void *data, UInt32 size, UInt32 &processedSize) throw();
  bool WriteFull(const void *data, size_t size) throw();
  bool SetEndOfFile() throw();
  bool SetLength(UInt64 length) throw();
  bool SetLength_KeepPosition(UInt64 length) throw();
};

}


#else // _WIN32

namespace NIO {

bool GetReparseData(CFSTR path, CByteBuffer &reparseData);
// bool SetReparseData(CFSTR path, bool isDir, const void *data, DWORD size);

// parameters are in reverse order of symlink() function !!!
bool SetSymLink(CFSTR from, CFSTR to);
bool SetSymLink_UString(CFSTR from, const UString &to);


class CFileBase
{
protected:
  int _handle;

  bool OpenBinary(const char *name, int flags);
public:
  bool PreserveATime;

  CFileBase(): _handle(-1), PreserveATime(false) {};
  ~CFileBase() { Close(); }
  bool Close();
  bool GetLength(UInt64 &length) const;
  off_t seek(off_t distanceToMove, int moveMethod) const;
  off_t seekToBegin() const throw();
  off_t seekToCur() const throw();
  // bool SeekToBegin() throw();
  int my_fstat(struct stat *st) const  { return fstat(_handle, st); }
};

class CInFile final : public CFileBase
{
public:
  bool Open(const char *name);
  bool OpenShared(const char *name, bool shareForWrite);
  ssize_t read_part(void *data, size_t size) throw();
  // ssize_t read_full(void *data, size_t size, size_t &processed);
  bool ReadFull(void *data, size_t size, size_t &processedSize) throw();
};

class COutFile final : public CFileBase
{
  bool CTime_defined;
  bool ATime_defined;
  bool MTime_defined;

  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;

  AString Path;
  ssize_t write_part(const void *data, size_t size) throw();
public:
  COutFile():
      CTime_defined(false),
      ATime_defined(false),
      MTime_defined(false)
      {}

  bool Close();
  bool Create(const char *name, bool createAlways);
  bool Open(const char *name, DWORD creationDisposition);
  ssize_t write_full(const void *data, size_t size, size_t &processed) throw();

  bool WriteFull(const void *data, size_t size) throw()
  {
    size_t processed;
    ssize_t res = write_full(data, size, processed);
    if (res == -1)
      return false;
    return processed == size;
  }

  bool SetLength(UInt64 length) throw();
  bool SetLength_KeepPosition(UInt64 length) throw()
  {
    return SetLength(length);
  }
  bool SetTime(const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime) throw();
  bool SetMTime(const FILETIME *mTime) throw();
};

}

#endif  // _WIN32

}}


#endif
