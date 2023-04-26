/*
 * Copyright (c) 2022 - 2023 trinity-tech.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <cstdlib>
#include <cstdio>
#include <io.h>
#include <windef.h>
#else
#include <string>
#include <stdexcept>
#include <filesystem>
#endif

namespace elastos {
namespace carrier {

class ApplicationLock {
public:
    ApplicationLock() {};
    ~ApplicationLock() { release(); };

    int acquire(const std::string& filename) {
        // Make sure the existence of the parent directory
#ifdef _WIN32
        #define WC_LINK L""

        if (!_access(filename.c_str(), 0)) {
            size_t pathLen = mbstowcs(0, filename.c_str(), 0);
            size_t wcLen = 2 * pathLen + 1;
            wchar_t* wcharPathBuf = new wchar_t[wcLen];
            mbstowcs(wcharPathBuf, filename.c_str(), wcLen);

            aHandle = CreateFileA(reinterpret_cast<LPCSTR>(wcharPathBuf), GENERIC_READ | GENERIC_WRITE, NULL, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            delete wcharPathBuf;
            if(aHandle == INVALID_HANDLE_VALUE) {
                DWORD errCode = GetLastError();
                CloseHandle(aHandle);
                return -1;
            }
        }

        this->filename = filename;

        DWORD dwHigh = 0;
        dwLow = GetFileSize(aHandle, (LPDWORD)&dwHigh);
        if(dwLow == INVALID_FILE_SIZE)
            return -1;

        bool retLockFile = LockFile(aHandle, NULL, NULL, dwLow, NULL);
        if (!retLockFile) {
            CloseHandle(aHandle);
            aHandle = INVALID_HANDLE_VALUE;
            return -1;
        }
#else
        std::filesystem::path lockPath = filename;
        auto parent = lockPath.parent_path();
        if (!std::filesystem::exists(parent))
            std::filesystem::create_directories(parent);

        this->filename = filename;
        fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd < 0)
            return -1;

        int rc = flock(fd, LOCK_EX | LOCK_NB);
        if (rc < 0) {
            close(fd);
            fd = -1;
            return -1;
        }
#endif
        return 0;
    }

    void release() {
#ifdef _WIN32
        if (aHandle != INVALID_HANDLE_VALUE) {
            UnlockFile(aHandle, NULL, NULL, dwLow, NULL);
            CloseHandle(aHandle);
            std::remove(filename.c_str());
        }
#else
        if (fd >= 0) {
            flock(fd, LOCK_UN);
            close(fd);
            std::remove(filename.c_str());
        }
#endif
    }

private:
    ApplicationLock(const ApplicationLock&) = delete;
    ApplicationLock& operator=(const ApplicationLock&) = delete;

    std::string filename;

#ifdef _WIN32
    HANDLE aHandle = INVALID_HANDLE_VALUE;
    DWORD dwLow = 0;
#else
    int fd = -1;
#endif
};

} // namespace carrier
} // namespace elastos
