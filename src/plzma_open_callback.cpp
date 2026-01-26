//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2025 Oleh Kulykov <olehkulykov@gmail.com>
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

#include "plzma_open_callback.hpp"
#include "plzma_common.hpp"
#include "plzma_archive_utils.hpp"

#include <sys/stat.h>

#include "CPP/7zip/Archive/DllExports2.h"
#include "CPP/7zip/UI/Common/LoadCodecs.h"
#include "CPP/7zip/UI/Common/OpenArchive.h"

namespace plzma {
    
    // Global CCodecs instance for format detection
    // Initialized once and reused for all archive operations
    static CMyComPtr<CCodecs> g_Codecs;
    
    // Initialize CCodecs with all registered archive formats
    // This loads format metadata including signatures for detection
    static HRESULT InitializeCodecs()
    {
        if (g_Codecs)
            return S_OK;  // Already initialized
        
        g_Codecs = new CCodecs();
        if (!g_Codecs)
            return E_OUTOFMEMORY;
        
        // Load() registers all REGISTER_ARC formats from 7zip SDK
        // including 7z, ZIP, TAR, and their signature information
        return g_Codecs->Load();
    }
    
    STDMETHODIMP OpenCallback::SetTotal(const UInt64 * files, const UInt64 * bytes) throw() {
        return S_OK; // unused
    }
    
    STDMETHODIMP OpenCallback::SetCompleted(const UInt64 * files, const UInt64 * bytes) throw() {
        return S_OK; // unused
    }

    STDMETHODIMP OpenCallback::CryptoGetTextPassword(BSTR * password) throw() {
        _passwordRequested = true;
        return getTextPassword(nullptr, password);
    }
    
    STDMETHODIMP OpenCallback::CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password) throw() {
        _passwordRequested = true;
        return getTextPassword(passwordIsDefined, password);
    }
    
    bool OpenCallback::open() {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        if (_result != S_OK) {
            return false;
        }
        
        LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
        auto results = open(_initialStream);
        OpenResult result = std::get<0>(results);
        LIBPLZMA_UNIQUE_LOCK_LOCK(lock)

        switch (result) {
            case OpenResult::Ok:
                _itemsCount =  std::get<1>(results);
                return true;
            case OpenResult::Cancelled:
                _itemsCount = 0;
                return false;
            case OpenResult::IncorrectCodec:
            default:
                Exception internalException(plzma_error_code_internal, "Can't open in archive.", __FILE__, __LINE__);
#if defined(LIBPLZMA_NO_CRYPTO)
                internalException.setReason("no crypto", nullptr);
#endif
                throw internalException;
        }
    }

    std::tuple<OpenResult, UInt32> OpenCallback::open(CMyComPtr<IInStream> stream) {
        // Initialize CCodecs if not already done
        // This loads all archive format metadata including signatures
        HRESULT hr = InitializeCodecs();
        if (hr != S_OK) {
            return std::make_tuple(OpenResult::IncorrectCodec, 0);
        }
        
        // Setup COpenOptions - matching official 7-Zip Windows app pattern
        COpenOptions options;
        #ifndef Z7_SFX
        options.props = NULL;  // No custom properties
        #endif
        options.codecs = g_Codecs;
        
        // Empty types vector = try all types (auto-detect)
        CObjectVector<COpenType> types;
        options.types = &types;
        
        CIntVector excludedFormats;  // Empty = no formats excluded
        options.excludedFormats = &excludedFormats;
        
        options.stdInMode = false;
        options.stream = stream;
        options.filePath = UString();  // Empty for stream-based opening
        options.callback = this;  // Use this OpenCallback for progress/password
        options.openType.FormatIndex = -1;  // -1 = auto-detect format
        
        // Use CArchiveLink instead of CArc
        // CArchiveLink automatically handles nested archives (DMG/HFS, TAR.GZ, etc.)
        CArchiveLink archiveLink;
        HRESULT result = archiveLink.Open(options);
        
        if (result == S_OK && !archiveLink.Arcs.IsEmpty())
        {
            // Success! CArchiveLink opened the archive(s)
            // For nested archives, Arcs contains the chain (e.g., DMG -> HFS)
            // The last one is the innermost archive we want to show
            const CArc &arc = archiveLink.Arcs.Back();
            
            if (arc.Archive) {
                UInt32 numItems = 0;
                arc.Archive->GetNumberOfItems(&numItems);
                
                // Save all archives in the chain
                for (unsigned i = 0; i < archiveLink.Arcs.Size(); i++) {
                    _openedArchives.Add(archiveLink.Arcs[i].Archive);
                    if (archiveLink.Arcs[i].InStream) {
                        _openedStreams.Add(archiveLink.Arcs[i].InStream);
                    }
                }
                
                // If no custom stream wrappers, add the original stream
                if (_openedStreams.Size() == 0) {
                    _openedStreams.Add(stream);
                }
                
                return std::make_tuple(OpenResult::Ok, numItems);
            }
        }
        else if (result == E_ABORT || _result == E_ABORT)
        {
            return std::make_tuple(OpenResult::Cancelled, 0);
        }
        
        // Unable to open with any format
        return std::make_tuple(OpenResult::IncorrectCodec, 0);
    }

    void OpenCallback::abort() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _result = E_ABORT;
        _itemsCount = 0;
    }
    
    CMyComPtr<IInArchive> OpenCallback::archive() const noexcept {
        return _openedArchives.Back();
    }
    
    plzma_size_t OpenCallback::itemsCount() noexcept {
        return _itemsCount;
    }
    
    SharedPtr<Item> OpenCallback::initialItemAt(const plzma_size_t index) {
        auto archive = _openedArchives.Back();
        if (index < _itemsCount) {
            NWindows::NCOM::CPropVariant path;
            SharedPtr<Item> item;
            if (archive->GetProperty(index, kpidPath, &path) == S_OK && (path.vt == VT_EMPTY || path.vt == VT_BSTR)) {
                item = makeShared<Item>(static_cast<Path &&>(Path(path.bstrVal)), index);
            }
            return item;
        }
        return SharedPtr<Item>();
    }
    
    SharedPtr<Item> OpenCallback::itemAt(const plzma_size_t index) {
        auto archive = _openedArchives.Back();
        auto item = initialItemAt(index);
        if (item) {
            Stat stat = GetArchiveItemStat(archive, index);

            item->setSize(stat.size());
            item->setCreationTime(stat.creation());
            item->setChangeTime(stat.lastChange());
            item->setAccessTime(stat.lastAccess());
            item->setModificationTime(stat.lastModification());
            if (stat.hasPermissions()) {
                item->setPermissions(stat.permissions());
            }
            if (stat.isSymbolicLink()) {
                item->setIsSymbolicLink(true);
                item->setSymbolicLink(stat.symbolicLink());
            }

            NWindows::NCOM::CPropVariant prop;
    
            prop.Clear();
            if (archive->GetProperty(index, kpidPackSize, &prop) == S_OK) {
                item->setPackSize(PROPVARIANTGetUInt64(prop));
            }

            prop.Clear();
            if (archive->GetProperty(index, kpidEncrypted, &prop) == S_OK) {
                item->setEncrypted(PROPVARIANTGetBool(prop));
            }
            
            prop.Clear();
            if (archive->GetProperty(index, kpidCRC, &prop) == S_OK) {
                item->setCrc32(static_cast<uint32_t>(PROPVARIANTGetUInt64(prop)));
            }
            
            prop.Clear();
            if (archive->GetProperty(index, kpidIsDir, &prop) == S_OK) {
                item->setIsDir(PROPVARIANTGetBool(prop));
            }
        }
        return item;
    }
    
    SharedPtr<ItemArray> OpenCallback::allItems() {
        auto items = makeShared<ItemArray>(_itemsCount);
        for (plzma_size_t i = 0; i < _itemsCount; i++) {
            items->push(static_cast<SharedPtr<Item> &&>(itemAt(i)));
        }
        return items;
    }
    
    OpenCallback::OpenCallback(const CMyComPtr<InStreamBase> & stream,
#if !defined(LIBPLZMA_NO_CRYPTO)
                               const String & passwd
#endif
    ) : CMyUnknownImp(),
        _initialStream(stream) {
#if !defined(LIBPLZMA_NO_CRYPTO)
            _password = passwd;
#endif
    }
    
} // namespace plzma
