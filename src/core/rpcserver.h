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

#include <list>
#include <queue>
#include <random>
#include <optional>

#include "utils/log.h"
#include "messages/message.h"
#include "rpccall.h"
#include "scheduler.h"

namespace elastos {
namespace carrier {

class Node;

class RPCServer {
public:
    enum class State {
        INITIAL,
        RUNNING,
        STOPPED
    };

    RPCServer(Node& _node, const Sp<DHT> _dht4, const Sp<DHT> _dht6);
    ~RPCServer();

    void start();
    void stop();

    void sendCall(Sp<RPCCall>& call);
    void dispatchCall(Sp<RPCCall>& call);
    void sendMessage(Sp<Message> msg);
    void handleMessage(Sp<Message> msg);

    bool isReachable() const {
        return _isReachable;
    }

    void updateReachability(uint64_t now);

    bool hasIPv4() const {
        std::lock_guard<std::mutex> lk(lock);
        return sock4 != -1;
    }

    bool hasIPv6() const {
        std::lock_guard<std::mutex> lk(lock);
        return sock6 != -1;
    }

    Scheduler& getScheduler() {
        return scheduler;
    }

    int getNumberOfActiveRPCCalls() {
        return calls.size();
    }

    SocketAddress& getAddress(sa_family_t af) {
        return (af == AF_INET) ? bound4: bound6;
    }

    void sendError(Sp<Message> msg, int code, const std::string& err);

private:
    void bindSockets(const SocketAddress& bind4, const SocketAddress& bind6);
    void openSockets();
    int sendData(Sp<Message>& msg);
    void handlePacket(const uint8_t *buf, size_t buflen, const SocketAddress& from);
    void periodic();

#if defined(MSG_PRINT_DETAIL)
    bool filterMessage(std::string name);
#endif

    Sp<Logger> log;
    Node& node;

    std::optional<std::reference_wrapper<DHT>> dht4;
    std::optional<std::reference_wrapper<DHT>> dht6;

    int sock4 {-1};
    int sock6 {-1};

    SocketAddress bound4;
    SocketAddress bound6;

    std::thread rcv_thread;
    std::atomic_bool running {false};

    std::list<Sp<RPCCall>> callQueue;
    std::map<int, Sp<RPCCall>> calls;

    State state {State::INITIAL};
    volatile int nextTxid {0};
    volatile bool _isReachable {false};
    uint64_t messagesAtLastReachableCheck {0};
    uint64_t lastReachableCheck {0};
    uint64_t startTime;
    std::atomic<uint64_t> receivedMessages {0};

    mutable std::mutex lock;

    std::queue<Sp<Message>> messageQueue {};
    Scheduler scheduler {};
};
}
}
