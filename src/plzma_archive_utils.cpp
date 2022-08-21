//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2022 Oleh Kulykov <olehkulykov@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


#include <cstddef>

#include "plzma_archive_utils.hpp"
#include "plzma_path_utils.hpp"
#include "plzma_common.hpp"

#include "CPP/Windows/PropVariant.h"

namespace plzma {
    Stat GetArchiveItemStat(const CMyComPtr<IInArchive> & archive, const plzma_size_t index) {
        Stat stat;
        NWindows::NCOM::CPropVariant prop;
        if (archive->GetProperty(index, kpidSize, &prop) == S_OK) {
            stat.setSize(PROPVARIANTGetUInt64(prop));
        }
            
        prop.Clear();
        if (archive->GetProperty(index, kpidCTime, &prop) == S_OK && prop.vt == VT_FILETIME) {
            stat.setCreation(FILETIMEToUnixTime(prop.filetime));
        }
            
        prop.Clear();
        if (archive->GetProperty(index, kpidATime, &prop) == S_OK && prop.vt == VT_FILETIME) {
            stat.setLastAccess(FILETIMEToUnixTime(prop.filetime));
        }
            
        prop.Clear();
        if (archive->GetProperty(index, kpidMTime, &prop) == S_OK && prop.vt == VT_FILETIME) {
            stat.setLastModification(FILETIMEToUnixTime(prop.filetime));
        }

        prop.Clear();
        if (archive->GetProperty(index, kpidPosixAttrib, &prop) == S_OK && prop.vt != VT_EMPTY) {
            auto mode = PROPVARIANTGetUInt64(prop);
            stat.setPermissions(static_cast<uint16_t>(mode & 0777));
            if (S_ISLNK(mode)) {
                stat.setIsSymbolicLink(true);
                prop.Clear();
                if (archive->GetProperty(index, kpidSymLink, &prop) == S_OK && prop.vt == VT_BSTR) {
                    stat.setSymbolicLink(String(prop.bstrVal));
                }
            }
        }
        return stat;
    }
}
