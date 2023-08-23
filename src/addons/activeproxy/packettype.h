/*
 * Copyright (c) 2022-2023 trinity-tech.io
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

#include "crypto/random.h"

namespace elastos {
namespace carrier {
namespace activeproxy {

#define AUTH_MIN        0x00
#define AUTH_MAX        0x07
#define ATTACH_MIN      0x08
#define ATTACH_MAX      0x0F
#define PING_MIN        0x10
#define PING_MAX        0x1F
#define CONNECT_MIN     0x20
#define CONNECT_MAX     0x2F
#define DISCONNECT_MIN  0x30
#define DISCONNECT_MAX  0x3F
#define DATA_MIN        0x40
#define DATA_MAX        0x6F
#define ERROR_MIN       0x70
#define ERROR_MAX       0x7F

#define ACK_MASK        0x80
#define TYPE_MASK       0x7F

class PacketType {
public:
    enum Enum : uint8_t {
        AUTH = AUTH_MIN,
        AUTH_ACK = ACK_MASK | AUTH_MIN,
        ATTACH = ATTACH_MIN,
        ATTACH_ACK = ACK_MASK | ATTACH_MIN,
        PING = PING_MIN,
        PING_ACK = ACK_MASK | PING_MIN,
        CONNECT = CONNECT_MIN,
        CONNECT_ACK = ACK_MASK | CONNECT_MIN,
        DISCONNECT = DISCONNECT_MIN,
        DISCONNECT_ACK = ACK_MASK | DISCONNECT_MIN,
        DATA = DATA_MIN,
        ERROR = ERROR_MIN
    };

    constexpr PacketType() = delete;
    constexpr PacketType(Enum e) : e(e) {};

    // Allows comparisons with Enum constants.
    constexpr operator Enum() const noexcept {
        return e;
    }

    // Needed to prevent if(e)
    explicit operator bool() const = delete;

    static uint8_t random(uint8_t min, uint8_t max) {
        // [min, max]
        return Random::uint8(max - min + 1) + min;
    }

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

    // uint8_t value() {
    //     // [min, max]
    //     return Random::uint8(max - min + 1) + min;
    // }

    bool isAck() {
		return e & ACK_MASK;
	}

    static inline PacketType valueOf(uint8_t value) {
        bool ack = (value & ACK_MASK) != 0;
        uint8_t type = value & TYPE_MASK;

        switch (type >> 4) {
            case 0:
                if (type <= AUTH_MAX)
                    return ack ? AUTH_ACK : AUTH;
            else
                return ack ? ATTACH_ACK : ATTACH;

            case 1:
                return ack ? PING_ACK : PING;

            case 2:
                return ack ? CONNECT_ACK : CONNECT;

            case 3:
                return ack ? DISCONNECT_ACK : DISCONNECT;

            case 4:
            case 5:
            case 6:
                if (ack)
                    throw std::invalid_argument("Should never happen: invalid flag");
                else
                    return DATA;

            case 7:
                if (ack)
                    throw std::invalid_argument("Should never happen: invalid flag");
                else
                    return ERROR;

            default:
                throw std::invalid_argument("Should never happen: invalid flag");
        }
    }

    std::string toString() const noexcept {
        switch (e) {
            case AUTH: return "AUTH";
            case AUTH_ACK: return "AUTH ACK";
            case ATTACH: return "ATTACH";
            case ATTACH_ACK: return "ATTACH ACK";
            case PING: return "PING";
            case PING_ACK: return "PING ACK";
            case CONNECT: return "CONNECT";
            case CONNECT_ACK: return "CONNECT ACK";
            case DISCONNECT: return "DISCONNECT";
            case DISCONNECT_ACK: return "DISCONNECT ACK";
            case DATA: return "DATA";
            case ERROR: return "ERROR";
        }
    }

private:

    Enum e {};

};

} // namespace activeproxy
} // namespace carrier
} // namespace elastos
