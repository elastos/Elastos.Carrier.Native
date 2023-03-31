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

#include <random>
#include <random>
#include <type_traits>

namespace elastos {
namespace carrier {

#include <random>
#include <type_traits>

template<typename T>
class RandomGenerator {
    static_assert(std::is_integral<T>::value, "Only supports integral types");

public:
    RandomGenerator() : RandomGenerator(std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) {}
    RandomGenerator(T min, T max) : engine(std::random_device()()), distribution(min, max) {}

    T operator()() {
        return distribution(engine);
    }

    constexpr T max() const {
        return distribution.max();
    }

    constexpr T min() const {
        return distribution.min();
    }

private:
    std::mt19937 engine;
    std::uniform_int_distribution<T> distribution;
};


} // carrier
} // elastos
