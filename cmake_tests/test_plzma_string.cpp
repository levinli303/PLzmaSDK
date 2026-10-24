//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2026 Oleh Kulykov <olehkulykov@gmail.com>
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


#include "plzma_public_tests.hpp"

using namespace plzma;

int test_plzma_string_test4(void) {
    auto len = String::lengthMaxCount(nullptr, static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 0 && len.second == 0)
    len = String::lengthMaxCount("", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 0 && len.second == 0)
    len = String::lengthMaxCount("a", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 1 && len.second == 1)
    len = String::lengthMaxCount("ä", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 2 && len.second == 1 && len.first == strlen("ä"))
    len = String::lengthMaxCount("ы", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 2 && len.second == 1 && len.first == strlen("ы"))
    return 0;
}

int test_plzma_string_test3(void) {
    String s;
    
    s.set(L"a");
    PLZMA_TESTS_ASSERT(s.utf8() != nullptr)
    PLZMA_TESTS_ASSERT(s.count() == 1)
    
    s.set("ä");
    PLZMA_TESTS_ASSERT(s.count() == 1)
    
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"ä") == 0)
    s.set("H");
    PLZMA_TESTS_ASSERT(s.count() == 1)
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"H") == 0)
    s.set("");
    PLZMA_TESTS_ASSERT(s.count() == 0)
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"") == 0)
    
    return 0;
}

int test_plzma_string_test1(void) {
    String s("");
    PLZMA_TESTS_ASSERT(s.count() == 0)
    PLZMA_TESTS_ASSERT(s.count() == 0)
    s.set("a");
    PLZMA_TESTS_ASSERT(s.count() > 0)
    PLZMA_TESTS_ASSERT(s.utf8() != nullptr)
    s.set("a");
    PLZMA_TESTS_ASSERT(s.count() > 0)
    PLZMA_TESTS_ASSERT(s.wide() != nullptr)
    s.append("b");
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), "ab") == 0)
    PLZMA_TESTS_ASSERT(s.count() == 2)
    s.append(L"c");
    PLZMA_TESTS_ASSERT(s.count() == 3)
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), "abc") == 0)
    s.append(L"ä");
    PLZMA_TESTS_ASSERT(s.count() == 4)
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), "abcä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"abcä") == 0)
    s.clear(plzma_erase_zero);
    return 0;
}

int test_plzma_string_non_bmp(void) {
    const char * utf8 = "A\xF0\x9F\x97\x82" "B\xF0\x9F\x98\x80";
    const wchar_t * wide = L"A\U0001F5C2B\U0001F600";
    String s(utf8);
    PLZMA_TESTS_ASSERT(s.count() == wcslen(wide))
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), wide) == 0)
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), utf8) == 0)

    String fromWide(wide);
    PLZMA_TESTS_ASSERT(fromWide.count() == s.count())
    PLZMA_TESTS_ASSERT(strcmp(fromWide.utf8(), utf8) == 0)

    String copy(s);
    copy.append(L"/");
    copy.append("\xF0\x9F\x98\x80");
    PLZMA_TESTS_ASSERT(wcscmp(copy.wide(), L"A\U0001F5C2B\U0001F600/\U0001F600") == 0)
    PLZMA_TESTS_ASSERT(copy.count() == wcslen(copy.wide()))
    PLZMA_TESTS_ASSERT(strcmp(copy.utf8(), "A\xF0\x9F\x97\x82" "B\xF0\x9F\x98\x80/\xF0\x9F\x98\x80") == 0)

    const size_t units = wcslen(L"\U0001F600");
    auto len = String::lengthMaxCount("\xF0\x9F\x98\x80" "x", units);
    PLZMA_TESTS_ASSERT(len.first == 4 && len.second == units)
    len = String::lengthMaxCount("\xF0\x9F\x98\x80", units - 1);
    PLZMA_TESTS_ASSERT(len.first == 0 && len.second == 0)
    len = String::lengthMaxLength("\xF0\x9F\x98\x80" "x", 4);
    PLZMA_TESTS_ASSERT(len.first == 4 && len.second == units)

    Path path("root");
    path.append("\xF0\x9F\x97\x82");
    path.append(L"\U0001F600");
    PLZMA_TESTS_ASSERT(wcscmp(path.lastComponent().wide(), L"\U0001F600") == 0)
    Path parent(path);
    parent.removeLastComponent();
    PLZMA_TESTS_ASSERT(wcscmp(parent.lastComponent().wide(), L"\U0001F5C2") == 0)
    PLZMA_TESTS_ASSERT(parent.count() == wcslen(parent.wide()))
    return 0;
}

int test_plzma_path_windows_extended(void) {
#if defined(LIBPLZMA_OS_WINDOWS)
    for (const char * value : { "\\\\?\\C:\\folder\\file", "\\\\?\\UNC\\server\\share\\file", "\\\\server\\share\\file" }) {
        Path path(value);
        PLZMA_TESTS_ASSERT(strcmp(path.utf8(), value) == 0)
        PLZMA_TESTS_ASSERT(path.count() == strlen(value))
        Path wide(path.wide());
        PLZMA_TESTS_ASSERT(strcmp(wide.utf8(), value) == 0)
        wide.append("child");
        wide.removeLastComponent();
        PLZMA_TESTS_ASSERT(strcmp(wide.utf8(), value) == 0)
        path.set(wide.wide());
        PLZMA_TESTS_ASSERT(strcmp(path.utf8(), value) == 0)
    }

    Path temporary = Path::tmpPath();
    std::string value = temporary.utf8();
    value = value.compare(0, 2, "\\\\") == 0 ? "\\\\?\\UNC\\" + value.substr(2) : "\\\\?\\" + value;
    Path root(value.c_str());
    root.appendRandomComponent();
    Path nested(root);
    while (nested.count() < 600) {
        nested.append("abcdefghijklmnopqrst");
    }
    PLZMA_TESTS_ASSERT(nested.createDir(true))
    bool isDir = false;
    PLZMA_TESTS_ASSERT(nested.exists(&isDir) && isDir)
    PLZMA_TESTS_ASSERT(nested.createDir(true))
    Path file = nested.appending("contents.txt");
    FILE * output = file.openFile("wb");
    PLZMA_TESTS_ASSERT(output != nullptr)
    PLZMA_TESTS_ASSERT(fwrite("long path", 1, 9, output) == 9)
    PLZMA_TESTS_ASSERT(fclose(output) == 0)
    FILE * input = file.openFile("rb");
    PLZMA_TESTS_ASSERT(input != nullptr)
    char content[10] = { 0 };
    PLZMA_TESTS_ASSERT(fread(content, 1, 9, input) == 9)
    PLZMA_TESTS_ASSERT(fclose(input) == 0)
    PLZMA_TESTS_ASSERT(strcmp(content, "long path") == 0)
    PLZMA_TESTS_ASSERT(file.remove())
    PLZMA_TESTS_ASSERT(!file.exists())
    PLZMA_TESTS_ASSERT(root.remove())
    PLZMA_TESTS_ASSERT(!root.exists())
#endif
    return 0;
}

int main(int argc, char* argv[]) {
    std::flush(std::cout) << plzma_version() << std::endl;
    int ret = 0;
    
    if ( (ret = test_plzma_string_test1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_string_test3()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_string_test4()) ) {
        return ret;
    }

    if ( (ret = test_plzma_string_non_bmp()) ) {
        return ret;
    }

    if ( (ret = test_plzma_path_windows_extended()) ) {
        return ret;
    }
    
//    while (1) {
//        usleep(50);
//    }
    
    return ret;
}
