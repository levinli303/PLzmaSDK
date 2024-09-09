//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2024 Oleh Kulykov <olehkulykov@gmail.com>
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

#if defined(LIBPLZMA_POSIX)
#include <utime.h>
#endif

namespace plzma {

    using namespace pathUtils;
    
    void Path::set(const String & str) {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        set(str.wide());
#elif defined(LIBPLZMA_POSIX)
        set(str.utf8());
#endif
    }
    
    void Path::set(const wchar_t * LIBPLZMA_NULLABLE str) {
        copyFrom(str, plzma_erase_zero);
        const auto reduced = normalize<wchar_t>(_ws);
        if (reduced > 0) {
            _size -= reduced;
        }
    }
    
    void Path::set(const char * LIBPLZMA_NULLABLE str) {
        copyFrom(str, plzma_erase_zero);
        const auto reduced = normalize<char>(_cs);
        if (reduced > 0) {
            _size -= reduced;
            _cslen -= reduced;
        }
    }
    
    void Path::append(const wchar_t * LIBPLZMA_NULLABLE str) {
        syncWide();
        const size_t len = str ? wcslen(str) : 0;
        if (len > 0) {
            const wchar_t * stringsList[2];
            Pair<size_t, size_t> sizesList[2];
            size_t count = 0;
            if (requireSeparator<wchar_t>(_ws, _size, str)) {
                stringsList[count] = CLZMA_SEP_WSTR;
                sizesList[count].first = sizesList[count].second = 1;
                count++;
            }
            stringsList[count] = str;
            sizesList[count].first = sizesList[count].second = len;
            String::append(stringsList, sizesList, ++count, plzma_erase_zero);
            const auto reduced = normalize<wchar_t>(_ws);
            if (reduced > 0) {
                _size -= reduced;
            }
        }
    }
    
    void Path::append(const char * LIBPLZMA_NULLABLE str) {
        syncUtf8();
        const auto len = lengthMaxCount(str, static_cast<size_t>(plzma_max_size()));
        if (len.first > 0) {
            const char * stringsList[2];
            Pair<size_t, size_t> sizesList[2];
            size_t count = 0;
            if (requireSeparator<char>(_cs, _cslen, str)) {
                stringsList[count] = CLZMA_SEP_CSTR;
                sizesList[count].first = sizesList[count].second = 1;
                count++;
            }
            stringsList[count] = str;
            sizesList[count] = len;
            String::append(stringsList, sizesList, ++count, plzma_erase_zero);
            const auto reduced = normalize<char>(_cs);
            if (reduced > 0) {
                _size -= reduced;
                _cslen -= reduced;
            }
        }
    }
    
    void Path::append(const Path & path) {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        append(path.wide());
#elif defined(LIBPLZMA_POSIX)
        append(path.utf8());
#endif
    }
    
    Path Path::appending(const wchar_t * LIBPLZMA_NULLABLE str) const {
        Path result(*this);
        result.append(str);
        return result;
    }

    Path Path::appending(const char * LIBPLZMA_NULLABLE str) const {
        Path result(*this);
        result.append(str);
        return result;
    }

    Path Path::appending(const Path & path) const {
        Path result(*this);
        result.append(path);
        return result;
    }

    void Path::appendRandomComponent() {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        syncWide();
        _cs.clear(plzma_erase_zero, sizeof(char) * _cslen);
        _size += appendRandComp<wchar_t>(_ws, _size);
#elif defined(LIBPLZMA_POSIX)
        syncUtf8();
        _ws.clear(plzma_erase_zero, sizeof(wchar_t) * _size);
        const auto appended = appendRandComp<char>(_cs, _cslen);
        _size += appended;
        _cslen += appended;
#endif
    }
    
    Path Path::appendingRandomComponent() const {
        Path result(*this);
        result.appendRandomComponent();
        return result;
    }

    Path Path::lastComponent() const {
        Path res;
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        syncWide();
        if (_ws && _size > 0) {
            const auto comp = lastComp<wchar_t>(_ws, _size);
            if (comp.second > 0) {
                const wchar_t * stringsList[1] = { comp.first };
                Pair<size_t, size_t> sizesList[1];
                sizesList[0].first = sizesList[0].second = comp.second;
                res.String::append(stringsList, sizesList, 1, plzma_erase_zero);
            }
        }
#elif defined(LIBPLZMA_POSIX)
        syncUtf8();
        if (_cs && _cslen > 0) {
            const auto comp = lastComp<char>(_cs, _cslen);
            if (comp.second > 0) {
                const char * stringsList[1] = { comp.first };
                Pair<size_t, size_t> sizesList[1] = { lengthMaxLength(comp.first, comp.second) };
                res.String::append(stringsList, sizesList, 1, plzma_erase_zero);
            }
        }
#endif
        return res;
    }
    
    void Path::removeLastComponent() {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        syncWide();
        if (_ws && _size > 0 && removeLastComp<wchar_t>(_ws, _size)) {
            _size = static_cast<plzma_size_t>(wcslen(_ws));
            _cs.clear(plzma_erase_zero, sizeof(char) * _cslen);
            _cslen = 0;
        }
#elif defined(LIBPLZMA_POSIX)
        syncUtf8();
        if (_cs && _cslen > 0 && removeLastComp<char>(_cs, _cslen)) {
            const auto len = String::lengthMaxCount(_cs, static_cast<size_t>(plzma_max_size()));
            _ws.clear(plzma_erase_zero, sizeof(wchar_t) * _size);
            _cslen = static_cast<plzma_size_t>(len.first);
            _size = static_cast<plzma_size_t>(len.second);
        }
#endif
    }
    
    Path Path::removingLastComponent() const {
        Path result(*this);
        result.removeLastComponent();
        return result;
    }
    
    bool Path::exists(bool * LIBPLZMA_NULLABLE isDir /* = nullptr */) const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return (_size > 0) ? pathExists<wchar_t>(wide(), isDir) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathExists<char>(utf8(), isDir) : false;
#endif
    }
    
    bool Path::readable() const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return (_size > 0) ? pathReadable<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathReadable<char>(utf8()) : false;
#endif
    }
    
    bool Path::writable() const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return (_size > 0) ? pathWritable<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathWritable<char>(utf8()) : false;
#endif
    }
    
    bool Path::readableAndWritable() const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return (_size > 0) ? pathReadableAndWritable<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathReadableAndWritable<char>(utf8()) : false;
#endif
    }
    
    Stat Path::stat() const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return pathStat<wchar_t>(wide());
#elif defined(LIBPLZMA_POSIX)
        return pathStat<char>(utf8());
#endif
    }
    
    bool Path::remove(const bool skipErrors) const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return (_size > 0) ? removePath<wchar_t>(wide(), skipErrors) : true;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? removePath<char>(utf8(), skipErrors) : true;
#endif
    }
    
    bool Path::createDir(const bool withIntermediates) const {
        if (withIntermediates) {
            if (_size > 0) {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
                const wchar_t * w = wide(); // syncWide, changed '_size'
                return createIntermediateDirs<wchar_t>(w, _size);
#elif defined(LIBPLZMA_POSIX)
                const char * c = utf8(); // syncUtf8, changed '_cslen'
                return createIntermediateDirs<char>(c, _cslen);
#endif
            }
            return false;
        }
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        return (_size > 0) ? createSingleDir<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? createSingleDir<char>(utf8()) : false;
#endif
    }
    
    FILE * LIBPLZMA_NULLABLE Path::openFile(const char * LIBPLZMA_NONNULL mode) const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        if (_size > 0) {
            wchar_t wmode[32] = { 0 }; // more than enough for a max mode: "w+b, ccs=UNICODE"
            for (size_t i = 0, n = ::strlen(mode); ((i < n) && (i < 31)); i++) {
                wmode[i] = static_cast<wchar_t>(mode[i]);
            }
#  if defined(HAVE__WFOPEN_S)
            FILE * f = nullptr;
            const errno_t err = ::_wfopen_s(&f, wide(), wmode);
            return (err == 0) ? f : nullptr;
#  else
            return _wfopen(wide(), wmode);
#  endif
        }
        return nullptr;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? ::fopen(utf8(), mode) : nullptr;
#endif
    }

    bool Path::setFilePermissions(const uint16_t permissions) const
    {
#if defined(LIBPLZMA_MSC)
        return _wchmod(wide(), permissions) == 0;
#elif defined(LIBPLZMA_POSIX)
        return chmod(utf8(), permissions) == 0;
#endif
    }

    bool Path::setTime(const time_t creationTime, const time_t accessTime, const time_t modificationTime) const
    {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        HANDLE hFile;
        hFile = CreateFileW(wide(),                // file to open
                            GENERIC_WRITE,         // open for reading
                            FILE_SHARE_READ,       // share for reading
                            NULL,                  // default security
                            OPEN_EXISTING,         // existing file only
                            FILE_ATTRIBUTE_NORMAL, // normal file
                            nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return false;

        FILETIME ctime = UnixTimeToFILETIME(creationTime);
        FILETIME atime = UnixTimeToFILETIME(accessTime);
        FILETIME mtime = UnixTimeToFILETIME(modificationTime);
        BOOL success = SetFileTime(hFile, &ctime, &atime, &mtime);
        CloseHandle(hFile);
        return (bool)success;
#elif defined(LIBPLZMA_POSIX)
        struct utimbuf timeBuff{0, 0};
        timeBuff.actime = accessTime;
        timeBuff.modtime = modificationTime;
        return (::utime(utf8(), &timeBuff) == 0);
#endif
    }
    
    bool Path::operator == (const Path & path) const {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        const wchar_t * a = wide();       // syncWide
        const wchar_t * b = path.wide();  // syncWide
        return pathsAreEqual<wchar_t>(a, b, _size, path._size);
#elif defined(LIBPLZMA_POSIX)
        const char * a = utf8();        // syncUtf8
        const char * b = path.utf8();   // syncUtf8
        return pathsAreEqual<char>(a, b, _cslen, path._cslen);
#endif
    }
    
    Path & Path::operator = (Path && path) noexcept {
        moveFrom(static_cast<Path &&>(path), plzma_erase_zero);
        return *this;
    }
    
    Path::Path(Path && path) noexcept : String(static_cast<Path &&>(path)) {
        
    }
    
    Path & Path::operator = (const Path & path) {
        copyFrom(path, plzma_erase_zero);
        return *this;
    }
    
    Path::Path(const Path & path) : String(path) {
        
    }
    
    Path::Path(const wchar_t * LIBPLZMA_NULLABLE path) : String(path) {
        const auto reduced = normalize<wchar_t>(_ws);
        if (reduced > 0) {
            _size -= reduced;
        }
    }
    
    Path::Path(const char * LIBPLZMA_NULLABLE path) : String(path) {
        const auto reduced = normalize<char>(_cs);
        if (reduced > 0) {
            _size -= reduced;
            _cslen -= reduced;
        }
    }
    
    Path::~Path() noexcept {
        _ws.erase(plzma_erase_zero, sizeof(wchar_t) * _size);
        _cs.erase(plzma_erase_zero, sizeof(char) * _cslen);
    }
    
#if !defined(PATH_MAX)
#  define PATH_MAX 1024
#endif
    
    Path Path::tmpPath() {
        Path path;
#if defined(__APPLE__) && defined(_CS_DARWIN_USER_TEMP_DIR)
        char buff[PATH_MAX];
        const size_t res = ::confstr(_CS_DARWIN_USER_TEMP_DIR, buff, PATH_MAX);
        if ((res > 0) && (res < PATH_MAX) && initializeTmpPath<char>(buff, "libplzma", path)) {
            return path;
        }
#endif
        
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
        static const wchar_t * const wevs[4] = { L"TMPDIR", L"TEMPDIR", L"TEMP", L"TMP" };
        for (size_t i = 0; i < 4; i++) {
#  if defined(HAVE__WDUPENV_S)
            wchar_t * p = nullptr;
            size_t len = 0;
            const errno_t err = ::_wdupenv_s(&p, &len, wevs[i]);
            if ((err == 0) && p && initializeTmpPath<wchar_t>(p, L"libplzma", path)) {
                ::free(p);
                return path;
            }
            if (p) {
                ::free(p);
            }
#  else
            const wchar_t * p = ::_wgetenv(wevs[i]);
            if (p && initializeTmpPath<wchar_t>(p, L"libplzma", path)) {
                return path;
            }
#  endif
        }
#endif
        
        static const char * const cevs[4] = { "TMPDIR", "TEMPDIR", "TEMP", "TMP" };
        for (size_t i = 0; i < 4; i++) {
#if defined(LIBPLZMA_MSC) || defined(LIBPLZMA_MINGW)
#  if defined(HAVE__DUPENV_S)
            char * p = nullptr;
            size_t len = 0;
            const errno_t err = ::_dupenv_s(&p, &len, cevs[i]);
            if ((err == 0) && p && initializeTmpPath<char>(p, "libplzma", path)) {
                ::free(p);
                return path;
            }
            if (p) {
                ::free(p);
            }
#  else
            char * p = ::getenv(cevs[i]);
            if (p && initializeTmpPath<char>(p, "libplzma", path)) {
                return path;
            }
#  endif
#else
            char * p = ::getenv(cevs[i]);
            if (p && initializeTmpPath<char>(p, "libplzma", path)) {
                return path;
            }
#endif
        }
#if !defined(LIBPLZMA_OS_WINDOWS)
        if (initializeTmpPath<char>("/tmp", "libplzma", path)) {
            return path;
        }
#endif
        return Path();
    }
    
} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_path plzma_path_create_with_wide_string(const wchar_t * LIBPLZMA_NULLABLE path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_path)
    createdCObject.object = static_cast<void *>(new Path(path));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_path_create_with_utf8_string(const char * LIBPLZMA_NULLABLE path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_path)
    createdCObject.object = static_cast<void *>(new Path(path));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_path_create_with_tmp_dir(void) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_path)
    auto tmp = Path::tmpPath();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(tmp)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_size_t plzma_path_count(const plzma_path * LIBPLZMA_NONNULL path) {
    return path->exception ? 0 : static_cast<const Path *>(path->object)->count();
}

void plzma_path_set_wide_string(plzma_path * LIBPLZMA_NONNULL path, const wchar_t * LIBPLZMA_NULLABLE str) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->set(str);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

void plzma_path_set_utf8_string(plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NULLABLE str) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->set(str);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

void plzma_path_append_wide_component(plzma_path * LIBPLZMA_NONNULL path, const wchar_t * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->append(component);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_appending_wide_component(const plzma_path * LIBPLZMA_NONNULL path, const wchar_t * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->appending(component);
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_append_utf8_component(plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->append(component);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_appending_utf8_component(const plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->appending(component);
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_append_random_component(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->appendRandomComponent();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_appending_random_component(const plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->appendingRandomComponent();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

const wchar_t * LIBPLZMA_NULLABLE plzma_path_wide_string(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, nullptr)
    return static_cast<Path *>(path->object)->wide();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, nullptr)
}

const char * LIBPLZMA_NULLABLE plzma_path_utf8_string(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, nullptr)
    return static_cast<Path *>(path->object)->utf8();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, nullptr)
}

bool plzma_path_exists(plzma_path * LIBPLZMA_NONNULL path, bool * LIBPLZMA_NULLABLE isDir) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->exists(isDir);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

bool plzma_path_readable(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->readable();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

bool plzma_path_writable(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->writable();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

bool plzma_path_readable_and_writable(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->readableAndWritable();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

plzma_stat plzma_path_get_stat(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Stat result = static_cast<const Path *>(path->object)->stat();
    createdCObject.object = static_cast<void *>(new Stat(static_cast<Stat &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_clear(plzma_path * LIBPLZMA_NONNULL path, const plzma_erase erase_type) {
    plzma_object_exception_release(path);
    static_cast<Path *>(path->object)->clear(erase_type);
}

bool plzma_path_remove(plzma_path * LIBPLZMA_NONNULL path, const bool skip_errors) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->remove(skip_errors);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

plzma_path plzma_path_last_component(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    auto comp = static_cast<Path *>(path->object)->lastComponent();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(comp)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_remove_last_component(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->removeLastComponent();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_removing_last_component(const plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->removingLastComponent();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

bool plzma_path_create_dir(plzma_path * LIBPLZMA_NONNULL path, const bool with_intermediates) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->createDir(with_intermediates);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

FILE * LIBPLZMA_NULLABLE plzma_path_open_file(plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NONNULL mode) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, nullptr)
    return static_cast<Path *>(path->object)->openFile(mode);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, nullptr)
}

void plzma_path_release(plzma_path * LIBPLZMA_NULLABLE path) {
    plzma_object_exception_release(path);
    delete static_cast<Path *>(path->object);
    path->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
