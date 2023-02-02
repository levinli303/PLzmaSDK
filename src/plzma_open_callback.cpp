//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2023 Oleh Kulykov <olehkulykov@gmail.com>
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

namespace plzma {
    
    STDMETHODIMP OpenCallback::SetTotal(const UInt64 * files, const UInt64 * bytes) {
        return S_OK; // unused
    }
    
    STDMETHODIMP OpenCallback::SetCompleted(const UInt64 * files, const UInt64 * bytes) {
        return S_OK; // unused
    }
    
    STDMETHODIMP OpenCallback::CryptoGetTextPassword(BSTR * password) {
        _passwordRequested = true;
        return getTextPassword(nullptr, password);
    }
    
    STDMETHODIMP OpenCallback::CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password) {
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
        auto supportedCodecs = getSortedSupportedCodecUUIDs();
        for (unsigned i = 0; i < supportedCodecs.Size(); i++) {
            auto result = open(stream, supportedCodecs[i]);
            switch (std::get<0>(result)) {
                case OpenResult::Ok:
                    return std::make_tuple(OpenResult::Ok, std::get<1>(result));
                case OpenResult::Cancelled:
                    return std::make_tuple(OpenResult::Cancelled, 0);
                case OpenResult::IncorrectCodec:
                default:
                    continue;
            }
        }
        return std::make_tuple(OpenResult::IncorrectCodec, 0);
    }

    std::tuple<OpenResult, UInt32> OpenCallback::open(CMyComPtr<IInStream> stream, const GUID & codecGuid) {
        IInArchive * ptr = nullptr;
        if (CreateObject(&codecGuid, &IID_IInArchive, reinterpret_cast<void**>(&ptr)) != S_OK || !ptr) {
            return std::make_tuple(OpenResult::IncorrectCodec, 0);
        }

        CMyComPtr<IInArchive> archive;
        archive.Attach(ptr);

        HRESULT thisResult = archive->Open(stream, nullptr, this);
        if (thisResult == S_OK && _result == S_OK) {
            // OK, proceed
        } else if (thisResult == E_ABORT || _result == E_ABORT) {
            _itemsCount = 0;
            return std::make_tuple(OpenResult::Cancelled, 0); // aborted -> false without exception
        } else if (_passwordRequested) {
            throw Exception(plzma_error_code_password_needed, "Password is needed for thie archive.", __FILE__, __LINE__);
        } else if (_exception) {
            Exception localException(static_cast<Exception &&>(*_exception));
            delete _exception;
            _exception = nullptr;
            throw localException;
        } else {
            // seek to zero to allow another round of detection
            if (stream->Seek(0, STREAM_SEEK_SET, nullptr) != S_OK)
            {
                Exception internalException(plzma_error_code_internal, "Failed to seek to the start of the file", __FILE__, __LINE__);
                throw internalException;
            }
            return std::make_tuple(OpenResult::IncorrectCodec, 0);
        }

        UInt32 numItems = 0;
        archive->GetNumberOfItems(&numItems);

        // save
        _openedArchives.Add(archive);
        _openedStreams.Add(stream);

        // Get sub stream if needed
        NWindows::NCOM::CPropVariant prop;
        if (archive->GetArchiveProperty(kpidMainSubfile, &prop) != S_OK || prop.vt == VT_EMPTY) {
            // No sub stream
            return std::make_tuple(OpenResult::Ok, numItems);
        }

        UInt32 mainSubfile = static_cast<uint32_t>(PROPVARIANTGetUInt64(prop));
        CMyComPtr<IInArchiveGetStream> getStream;
        if (archive->QueryInterface(IID_IInArchiveGetStream, (void **)&getStream) != S_OK || !getStream) {
            // cannot fetch get stream method
            return std::make_tuple(OpenResult::Ok, numItems);
        }

        CMyComPtr<ISequentialInStream> subSeqStream;
        if (getStream->GetStream(mainSubfile, &subSeqStream) != S_OK || !subSeqStream) {
            // failed to get stream
            return std::make_tuple(OpenResult::Ok, numItems);
        }        

        CMyComPtr<IInStream> subStream;
        if (subSeqStream.QueryInterface(IID_IInStream, &subStream) != S_OK || !subStream) {
            // is not an IINStream
            return std::make_tuple(OpenResult::Ok, numItems);
        }

        auto newResults = open(subStream);
        auto newResult = std::get<0>(newResults);
        switch (newResult) {
            case OpenResult::Ok:
                return std::make_tuple(OpenResult::Ok, std::get<1>(newResults));
            case OpenResult::Cancelled:
                return std::make_tuple(OpenResult::Cancelled, 0);
            case OpenResult::IncorrectCodec:
            default:
                return std::make_tuple(OpenResult::Ok, numItems);
        }
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

