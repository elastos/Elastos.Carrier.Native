/*
 * Copyright (c) 2022 Elastos Foundation
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

#include <cstdint>
#include <cassert>

#include "carrier/crypto_core.h"

namespace elastos {
namespace carrier {
namespace activeproxy {

class PacketFlag {
public:
    const static uint8_t AUTH       = 0x00;
    const static uint8_t ATTACH     = 0x08;
    const static uint8_t PING       = 0x10;
    const static uint8_t CONNECT    = 0x20;
    const static uint8_t DISCONNECT = 0x30;
    const static uint8_t DATA       = 0x40;
    const static uint8_t ERROR      = 0x70;

    static uint8_t auth() {
        return random(AUTH_MIN, AUTH_MAX);
    }

    static uint8_t authAck() {
        return random(AUTH_MIN, AUTH_MAX) | ACK_MASK;
    }

    static uint8_t attach() {
        return random(ATTACH_MIN, ATTACH_MAX);
    }

    static uint8_t attachAck() {
        return random(ATTACH_MIN, ATTACH_MAX) | ACK_MASK;
    }

    static uint8_t ping() {
        return random(PING_MIN, PING_MAX);
    }

    static uint8_t pingAck() {
        return random(PING_MIN, PING_MAX) | ACK_MASK;
    }

    static uint8_t connect() {
        return random(CONNECT_MIN, CONNECT_MAX);
    }

    static uint8_t connectAck() {
        return random(CONNECT_MIN, CONNECT_MAX) | ACK_MASK;
    }

    static uint8_t disconnect() {
        return random(DISCONNECT_MIN, DISCONNECT_MAX);
    }

    static uint8_t data() {
        return random(DATA_MIN, DATA_MAX);
    }

    static uint8_t error() {
        return random(ERROR_MIN, ERROR_MAX) | ACK_MASK;
    }

    static bool isAck(uint8_t flag) {
        return (flag & ACK_MASK) != 0;
    }

    static uint8_t getType(uint8_t flag) {
        uint8_t type = flag & TYPE_MASK;
        switch (type) {
        case AUTH_MIN ... AUTH_MAX:
            return AUTH;

        case ATTACH_MIN ... ATTACH_MAX:
            return ATTACH;

        case PING_MIN ... PING_MAX:
            return PING;

        case CONNECT_MIN ... CONNECT_MAX:
            return CONNECT;

        case DISCONNECT_MIN ... DISCONNECT_MAX:
            return DISCONNECT;

        case DATA_MIN ... DATA_MAX:
            return DATA;

        case ERROR_MIN ... ERROR_MAX:
            return ERROR;

        default:
            assert("Should never happen");
            return ERROR;
        }
    }

private:
    // [MIN, MAX]
    const static uint8_t AUTH_MIN       = AUTH;
    const static uint8_t AUTH_MAX       = 0x07;

    const static uint8_t ATTACH_MIN     = ATTACH;
    const static uint8_t ATTACH_MAX     = 0x0F;

    const static uint8_t PING_MIN       = PING;
    const static uint8_t PING_MAX       = 0x1F;

    const static uint8_t CONNECT_MIN    = CONNECT;
    const static uint8_t CONNECT_MAX    = 0x2F;

    const static uint8_t DISCONNECT_MIN = DISCONNECT;
    const static uint8_t DISCONNECT_MAX = 0x3F;

    const static uint8_t DATA_MIN       = DATA;
    const static uint8_t DATA_MAX       = 0x6F;

    const static uint8_t ERROR_MIN      = ERROR;
    const static uint8_t ERROR_MAX      = 0x7F;

    const static uint8_t ACK_MASK       = 0x80;
    const static uint8_t TYPE_MASK      = 0x7F;

    static uint8_t random(uint8_t min, uint8_t max) {
        // [min, max]
        return Random::uint8(max - min + 1) + min;
    }
};

}
}
}
