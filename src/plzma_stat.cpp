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

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_path_utils.hpp"

namespace plzma {  
    uint64_t Stat::size() const noexcept {
        return _size;
    }  

    time_t Stat::lastAccess() const noexcept {
        return _lastAccess;
    }

    time_t Stat::lastModification() const noexcept {
        return _lastModification;
    }

    time_t Stat::lastChange() const noexcept {
        return _lastChange;
    }

    time_t Stat::creation() const noexcept {
        return _creation;
    }

    bool Stat::hasPermissions() const noexcept {
        return _hasPermissions;
    }

    uint16_t Stat::permissions() const noexcept {
        return _permissions;
    }

    bool Stat::isSymbolicLink() const noexcept {
        return _isSymbolicLink;
    }

    const String & Stat::symbolicLink() const noexcept {
        return _symbolicLink;
    }

    void Stat::setSize(uint64_t size) noexcept {
        _size = size;
    }

    void Stat::setLastAccess(time_t lastAccess) noexcept {
        _lastAccess = lastAccess;
    }

    void Stat::setLastModification(time_t lastModification) noexcept {
        _lastModification = lastModification;
    }

    void Stat::setLastChange(time_t lastChange) noexcept {
        _lastChange = lastChange;
    }

    void Stat::setCreation(time_t creation) noexcept {
        _creation = creation;
    }

    void Stat::setPermissions(uint16_t permissions) noexcept {
        _hasPermissions = true;
        _permissions = permissions;
    }

    void Stat::setIsSymbolicLink(bool isSymbolicLink) noexcept {
        _isSymbolicLink = isSymbolicLink;
    }

    void Stat::setSymbolicLink(const String & symbolicLink) noexcept {
        _isSymbolicLink = true;
        _symbolicLink = symbolicLink;
    }

    Stat::Stat() noexcept {
    }

    Stat & Stat::operator = (Stat && stat) noexcept {
        _size = stat._size;
        _lastAccess = stat._lastAccess;
        _lastModification = stat._lastModification;
        _creation = stat._creation;
        _hasPermissions = stat._hasPermissions;
        _permissions = stat._permissions;
        _isSymbolicLink = stat._isSymbolicLink;
        _symbolicLink = static_cast<String &&>(stat._symbolicLink);
        return *this;
    }
    
    Stat::Stat(Stat && stat) noexcept : _symbolicLink(static_cast<String &&>(stat._symbolicLink)) {
        _size = stat._size;
        _lastAccess = stat._lastAccess;
        _lastModification = stat._lastModification;
        _creation = stat._creation;
        _hasPermissions = stat._hasPermissions;
        _permissions = stat._permissions;
        _isSymbolicLink = stat._isSymbolicLink;
    }
    
    Stat & Stat::operator = (const Stat & stat) {
        _size = stat._size;
        _lastAccess = stat._lastAccess;
        _lastModification = stat._lastModification;
        _creation = stat._creation;
        _hasPermissions = stat._hasPermissions;
        _permissions = stat._permissions;
        _isSymbolicLink = stat._isSymbolicLink;
        _symbolicLink = stat._symbolicLink;
        return *this;
    }
    
    Stat::Stat(const Stat & stat) : _symbolicLink(stat._symbolicLink) {
        _size = stat._size;
        _lastAccess = stat._lastAccess;
        _lastModification = stat._lastModification;
        _creation = stat._creation;
        _hasPermissions = stat._hasPermissions;
        _permissions = stat._permissions;
        _isSymbolicLink = stat._isSymbolicLink;
    }
} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_stat plzma_stat_create(void) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_stat)
    createdCObject.object = static_cast<void *>(new Stat);
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

uint64_t plzma_stat_size(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? 0 : static_cast<const Stat *>(stat->object)->size();
}

time_t plzma_stat_last_access(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? 0 : static_cast<const Stat *>(stat->object)->lastAccess();
}

time_t plzma_stat_last_modification(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? 0 : static_cast<const Stat *>(stat->object)->lastModification();
}

time_t plzma_stat_last_change(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? 0 : static_cast<const Stat *>(stat->object)->lastChange();
}

time_t plzma_stat_creation(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? 0 : static_cast<const Stat *>(stat->object)->creation();
}

bool plzma_stat_has_permissions(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? false : static_cast<const Stat *>(stat->object)->hasPermissions();
}

uint16_t plzma_stat_permissions(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? 0 : static_cast<const Stat *>(stat->object)->permissions();
}

bool plzma_stat_is_symbolic_link(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? false : static_cast<const Stat *>(stat->object)->isSymbolicLink();
}

const char * plzma_stat_symbolic_link_utf8_string(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? nullptr : static_cast<const Stat *>(stat->object)->symbolicLink().utf8();
}

const wchar_t * plzma_stat_symbolic_link_wide_string(const plzma_stat * LIBPLZMA_NONNULL stat) {
    return stat->exception ? nullptr : static_cast<const Stat *>(stat->object)->symbolicLink().wide();
}

void plzma_stat_set_size(plzma_stat * LIBPLZMA_NONNULL stat, const uint64_t size) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setSize(size); }
}

void plzma_stat_set_last_access(plzma_stat * LIBPLZMA_NONNULL stat, const time_t lastAccess) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setLastAccess(lastAccess); }
}

void plzma_stat_set_last_modification(plzma_stat * LIBPLZMA_NONNULL stat, const time_t lastModification) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setLastModification(lastModification); }
}

void plzma_stat_set_creation(plzma_stat * LIBPLZMA_NONNULL stat, const time_t creation) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setCreation(creation); }
}

void plzma_stat_set_last_change(plzma_stat * LIBPLZMA_NONNULL stat, const time_t last_change) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setLastChange(last_change); }
}

void plzma_stat_set_permissions(plzma_stat * LIBPLZMA_NONNULL stat, const uint16_t permissions) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setPermissions(permissions); }
}

void plzma_stat_set_is_symbolic_link(plzma_stat * LIBPLZMA_NONNULL stat, const bool isSymbolicLink) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setIsSymbolicLink(isSymbolicLink); }
}

void plzma_stat_set_symbolic_link_utf8_string(plzma_stat * LIBPLZMA_NONNULL stat, const char * LIBPLZMA_NULLABLE symbolicLink) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setSymbolicLink(symbolicLink); }
}

void plzma_stat_set_symbolic_link_wide_string(plzma_stat * LIBPLZMA_NONNULL stat, const wchar_t * LIBPLZMA_NULLABLE symbolicLink) {
    if (!stat->exception) { static_cast<Stat *>(stat->object)->setSymbolicLink(symbolicLink); }
}

void plzma_stat_release(plzma_stat * LIBPLZMA_NONNULL stat) {
    plzma_object_exception_release(stat);
    delete static_cast<Stat *>(stat->object);
    stat->object = nullptr;
}

#endif // # !LIBPLZMA_NO_C_BINDINGS
