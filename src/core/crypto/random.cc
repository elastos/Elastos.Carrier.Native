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

#include <vector>
#include <sodium.h>
#include "random.h"

namespace elastos {
namespace carrier {

uint8_t Random::uint8()
{
    return (uint8_t)randombytes_uniform(UINT8_MAX + 1);
}

uint8_t Random::uint8(uint8_t upbound)
{
    return (uint8_t)randombytes_uniform(upbound);
}

uint16_t Random::uint16()
{
    return (uint16_t)randombytes_uniform(UINT16_MAX + 1);
}

uint16_t Random::uint16(uint16_t upbound)
{
    return (uint16_t)randombytes_uniform(upbound);
}

uint32_t Random::uint32()
{
    return randombytes_random();
}

uint32_t Random::uint32(uint32_t upbound)
{
    return randombytes_uniform(upbound);
}

uint64_t Random::uint64()
{
    return ((uint64_t)randombytes_random() << 32) | (uint64_t)randombytes_random();
}

uint64_t Random::uint64(uint64_t upbound)
{
    return (((uint64_t)randombytes_random() << 32) | (uint64_t)randombytes_random()) % upbound;
}

void Random::buffer(void* buf, size_t length)
{
    randombytes_buf(buf, length);
}

void Random::buffer(Blob& blob)
{
    randombytes_buf(blob.begin(), blob.size());
}

void Random::buffer(std::vector<uint8_t>& bytes)
{
    randombytes_buf(bytes.data(), bytes.size());
}

} // namespace carrier
} // namespace elastos
