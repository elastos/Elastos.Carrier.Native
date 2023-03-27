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

#include <sys/file.h>
#include <unistd.h>

#include <string>
#include <stdexcept>
#include <iostream>

class ApplicationLock {
public:
    ApplicationLock() : fd(-1) {};
    ~ApplicationLock() { release(); };

    int acquire(const std::string& filename) {
        this->filename = filename;
        fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
        //test by chenyu
        std::cout << "fd: " << fd << std::endl;
        if (fd < 0) {
            return -1;
        }

        int rc = flock(fd, LOCK_EX | LOCK_NB);
        if (rc < 0) {
            close(fd);
            fd = -1;
            return -1;
        }

        return 0;
    }

    void release() {
        if (fd >= 0) {
            flock(fd, LOCK_UN);
            close(fd);
            std::remove(filename.c_str());
        }
    }

private:
    ApplicationLock(const ApplicationLock&) = delete;
    ApplicationLock& operator=(const ApplicationLock&) = delete;

    std::string filename;
    int fd;
};