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

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "utils/time.h"
#include "utils/random_generator.h"
#include "carrier/node.h"
#include "messages/message.h"
#include "messages/error_message.h"
#include "error_code.h"
#include "rpcserver.h"
#include "dht.h"

namespace elastos {
namespace carrier {

#ifdef MSG_PRINT_DETAIL
static std::map<int, std::string> txidNames {};

static std::vector<std::string> filter {
//    "User-level node lookup",
//    "User-level value lookup",
//    "NodeLookup: preTask to value announce",
//    "Nested value announce",
//    "User-level peer lookup",
//    "NodeLookup: PreTask to peer announce",
//    "Nested peer announce",
};

bool RPCServer::filterMessage(std::string name) {
    if (filter.empty())
        return true;
    else
        return std::find(filter.begin(), filter.end(), name) != filter.end();
}
#endif

RPCServer::RPCServer(Node& _node, const Sp<DHT> _dht4, const Sp<DHT> _dht6): node(_node),
    dht4(_dht4 ? std::optional<std::reference_wrapper<DHT>>(*_dht4) : std::nullopt),
    dht6(_dht6 ? std::optional<std::reference_wrapper<DHT>>(*_dht6) : std::nullopt) {

    nextTxid = RandomGenerator<int>(1,32768)();

    log = Logger::get("RpcServer");

    SocketAddress bind4, bind6;
    if (_dht4 != nullptr)
        bind4 = _dht4->getOrigin();
    if (_dht6 != nullptr)
        bind6 = _dht6->getOrigin();

    bindSockets(bind4, bind6);
}

RPCServer::~RPCServer() {
    stop();
    if (rcv_thread.joinable())
        rcv_thread.join();
}

static bool setNonblocking(int fd, bool nonblocking = true)
{
#ifdef _WIN32
    unsigned long mode = !!nonblocking;
    int rc = ioctlsocket(fd, FIONBIO, &mode);
    return rc == 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;

    if (nonblocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(fd, F_SETFL, flags) >= 0;
#endif
}

static int bindSocket(const SocketAddress& addr, SocketAddress& bound)
{
    int sock = socket(addr.family(), SOCK_DGRAM, 0);
    if (sock < 0)
        throw std::runtime_error("Failed to create socket: " + std::string(std::strerror(errno)));

    int set = 1;
#ifdef SO_NOSIGPIPE
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
#endif
    if (addr.family() == AF_INET6)
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &set, sizeof(set));

    setNonblocking(sock);
    int rc = bind(sock, addr.addr(), addr.length());
    if (rc < 0) {
        close(sock);
        throw std::runtime_error("Can't bind socket on " + addr.toString() + " " + std::string(std::strerror(errno)));
    }

    sockaddr_storage bound_addr;
    socklen_t bound_addr_len = sizeof(bound_addr);
    getsockname(sock, reinterpret_cast<sockaddr*>(&bound_addr), &bound_addr_len);
    bound = {bound_addr};
    return sock;
}

#ifdef _WIN32
static void udpPipe(int fds[2])
{
    int lst = socket(AF_INET, SOCK_DGRAM, 0);
    if (lst < 0)
        throw std::runtime_error(std::string("Can't open socket: ") + strerror(WSAGetLastError()));
    sockaddr_in inaddr;
    sockaddr addr;
    memset(&inaddr, 0, sizeof(inaddr));
    memset(&addr, 0, sizeof(addr));
    inaddr.sin_family = AF_INET;
    inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    inaddr.sin_port = 0;
    int yes = 1;
    setsockopt(lst, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
    int rc = bind(lst, (sockaddr*)&inaddr, sizeof(inaddr));
    if (rc < 0) {
        close(lst);
        throw std::runtime_error("Can't bind socket on " + print_addr((sockaddr*)&inaddr, sizeof(inaddr)) + " " + strerror(rc));
    }
    socklen_t len = sizeof(addr);
    getsockname(lst, &addr, &len);
    fds[0] = lst;
    fds[1] = socket(AF_INET, SOCK_DGRAM, 0);
    connect(fds[1], &addr, len);
}
#endif

int RPCServer::sendData(Sp<Message>& msg) {
    const auto& remoteAddr = msg->getRemoteAddress();

    if (!remoteAddr) {
        auto failureReason = "Remote address is invalid, need to be specified!";
        log->error("send data failed: {}", failureReason);
        throw std::runtime_error(failureReason);
    }

    int sockfd = -1;
    switch (remoteAddr.family()) {
        case AF_INET:
            sockfd = sock4;
            break;
        case AF_INET6:
            sockfd = sock6;
            break;
        default:
            throw std::runtime_error("Unsupported address family!");
    }

    if (sockfd < 0)
        throw std::runtime_error("Socket fd is error!!!");

    int flags = 0;
    #ifdef MSG_NOSIGNAL
        flags |= MSG_NOSIGNAL;
    #endif
    #ifdef MSG_CONFIRM
        //if (replied)
            //flags |= MSG_CONFIRM;
    #endif

    auto buffer = msg->serialize();
    auto encrypted = node.encrypt(msg->getRemoteId(), {buffer});
    buffer.resize(ID_BYTES + encrypted.size());
    std::memcpy(buffer.data(), msg->getId().data(), ID_BYTES);
    std::memcpy(buffer.data() + ID_BYTES, encrypted.data(), encrypted.size());

    int ret = sendto(sockfd, buffer.data(), buffer.size(), flags, remoteAddr.addr(), remoteAddr.length());
    if (ret == 0 || (ret == -1 && errno == EAGAIN)) {
        messageQueue.push(msg);
        return EAGAIN;
    } else if (ret == -1) {
        log->debug("Failed to send message to {}: {}", remoteAddr.toString(), std::strerror(errno));
        return errno;
    } else {
#ifdef MSG_PRINT_DETAIL
        msg->setName(txidNames[msg->getTxid()]);
        if (filterMessage(msg->name)) {
            auto af = msg->getRemoteAddress().family();
            log->info("\n\n-- Sent: {} bytes --\nLocal: {}\nTo: {}\n{}\n-- ** --\n",
                    ret, getAddress(af).toString(), msg->getRemoteAddress().toString(), static_cast<std::string>(*msg));
        }
#else
        log->info("Sent {}/{} to {}: [{}] {}", msg->getMethodString(), msg->getTypeString(),
                msg->getRemoteAddress().toString(), buffer.size(), static_cast<std::string>(*msg));
#endif
        return 0;
    }
}

void
RPCServer::bindSockets(const SocketAddress& bind4, const SocketAddress& bind6)
{
    sock4 = -1;
    sock6 = -1;

    bound4 = {};
    if (bind4) {
        try {
            sock4 = bindSocket(bind4, bound4);
        } catch (const DhtException& e) {
            if (log)
                log->error("Can't bind inet socket: {}", e.what());
        }
    }

    bound6 = {};
    if (dht6 && bind6) {
        if (bind6.port() == 0) {
            // Attempt to use the same port as IPv4 with IPv6
            if (auto p4 = bound4.port()) {
                auto b6 = SocketAddress({bind6.inaddr(), bind6.inaddrLength()}, p4);
                try {
                    sock6 = bindSocket(b6, bound6);
                } catch (const DhtException& e) {
                    if (log)
                        log->error("Can't bind inet6 socket: {}", e.what());
                }
            }
        }
        if (sock6 == -1) {
            try {
                sock6 = bindSocket(bind6, bound6);
            } catch (const DhtException& e) {
                if (log)
                    log->error("Can't bind inet6 socket: {}", e.what());
            }
        }
    }

    if (sock4 == -1 && sock6 == -1) {
        throw DhtException("Can't bind socket");
    }
}

void
RPCServer::openSockets()
{
    int stopfds[2];
#ifndef _WIN32
    auto status = pipe(stopfds);
    if (status == -1) {
        throw DhtException(std::string("Can't open pipe: ") + strerror(errno));
    }
#else
    udpPipe(stopfds);
#endif
    int stop_readfd = stopfds[0];

    stopfd = stopfds[1];

    running = true;
    rcv_thread = std::thread([this, stop_readfd, ls4=sock4, ls6=sock6]() mutable {
        int selectFd = std::max({ls4, ls6, stop_readfd}) + 1;
        struct timeval timeout;

// TODO:: will be remove
        // //--------------------For Debug-----------------------
        // char name[16];
        // snprintf(name, 16, "Tread%d", num++);
        // pthread_setname_np(name);

        try {
            while (running) {
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(stop_readfd, &readfds);

                if (ls4 >= 0) {
                    FD_SET(ls4, &readfds);
                }
                if (ls6 >= 0) {
                    FD_SET(ls6, &readfds);
                }

                timeout.tv_sec = 0;
                timeout.tv_usec = 100000;
                int rc = select(selectFd, &readfds, NULL, NULL, &timeout);
                if (rc < 0) {
                    if (errno != EINTR) {
                        if (log)
                            log->error("Select error: {}", strerror(errno));
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }

                if (not running)
                    break;

                if (rc > 0) {
                    std::array<uint8_t, 1024 * 64> buf;
                    sockaddr_storage from;
                    socklen_t from_len = sizeof(from);

                    if (FD_ISSET(stop_readfd, &readfds)) {
                        if (recv(stop_readfd, (char*)buf.data(), buf.size(), 0) < 0) {
                            if (log)
                                log->error("Got stop packet error: {}", strerror(errno));
                            break;
                        }
                    }
                    else if (ls4 >= 0 && FD_ISSET(ls4, &readfds))
                        rc = recvfrom(ls4, (char*)buf.data(), (size_t)buf.size(), 0, (sockaddr*)&from, &from_len);
                    else if (ls6 >= 0 && FD_ISSET(ls6, &readfds))
                        rc = recvfrom(ls6, (char*)buf.data(), (size_t)buf.size(), 0, (sockaddr*)&from, &from_len);
                    else
                        continue;

                    if (rc > 0) {
                        SocketAddress addr = {from};
                        handlePacket((uint8_t*)buf.data(), rc, addr);
                    } else if (rc == -1) {
                        if (log)
                            log->error("Error receiving packet: {}", strerror(errno));
                        int err = errno;
                        if (err == EPIPE || err == ENOTCONN || err == ECONNRESET) {
                            if (not running) break;
                            std::unique_lock<std::mutex> lk(lock, std::try_to_lock);
                            if (lk.owns_lock()) {
                                if (not running) break;
                                if (ls4 >= 0) {
                                    close(ls4);
                                    try {
                                        ls4 = bindSocket(bound4, bound4);
                                    } catch (const DhtException& e) {
                                        if (log)
                                            log->error("Can't bind inet socket: {}", e.what());
                                    }
                                }
                                if (ls6 >= 0) {
                                    close(ls6);
                                    try {
                                        ls6 = bindSocket(bound6, bound6);
                                    } catch (const DhtException& e) {
                                        if (log)
                                            log->error("Can't bind inet6 socket: {}", e.what());
                                    }
                                }
                                if (ls4 < 0 && ls6 < 0)
                                    break;
                                sock4 = ls4;
                                sock6 = ls6;
                                selectFd = std::max({ls4, ls6, stop_readfd}) + 1;
                            } else {
                                break;
                            }
                        }
                    }
                }

                periodic();
            }
        } catch (const std::exception& e) {
            if (log)
                log->error("Error in RPCServer rx thread: {}", e.what());
        }
        if (ls4 >= 0)
            close(ls4);
        if (ls6 >= 0)
            close(ls6);
        if (stop_readfd != -1)
            close(stop_readfd);
        if (stopfd != -1)
            close(stopfd);
        std::unique_lock<std::mutex> lk(lock, std::try_to_lock);
        if (lk.owns_lock()) {
            sock4 = -1;
            sock6 = -1;
            bound4 = {};
            bound6 = {};
            stopfd = -1;
        }
    });
}

//--------------------------------------------------------------

void RPCServer::start() {
    if (state != State::INITIAL)
        return;

    openSockets();

    state = State::RUNNING;
    startTime = currentTimeMillis();

    if (!bound6)
        log->info("Started RPC server ipv4: {}", bound4.toString());
    else
        log->info("Started RPC server ipv4: {}, ipv6: {}", bound4.toString(), bound6.toString());

}

void RPCServer::stop() {
    if(state == State::STOPPED)
        return;

    state = State::STOPPED;
    if (running.exchange(false)) {
        auto sfd = stopfd;
        if (sfd != -1 && write(sfd, "\0", 1) == -1)
            log->error("Can't write to stop fd");
    }

    if (rcv_thread.joinable())
        rcv_thread.join();

    if (bound4)
        log->info("Stopped RPC Server ipv4: {}", bound4.toString());
    if (bound6)
        log->info("Stopped RPC Server ipv6: {}", bound6.toString());
}

void RPCServer::updateReachability(uint64_t now) {
    // don't do pings too often if we're not receiving anything
    // (connection might be dead)
    if (receivedMessages != messagesAtLastReachableCheck) {
        _isReachable = true;
        lastReachableCheck = now;
        messagesAtLastReachableCheck = receivedMessages;
        return;
    }

    if (now - lastReachableCheck > Constants::RPC_SERVER_REACHABILITY_TIMEOUT)
        _isReachable = false;
}

void RPCServer::sendCall(Sp<RPCCall>& call) {
    int txid = nextTxid++;
    if (txid == 0) // 0 is invalid txid, skip
        txid = nextTxid++;

    if (calls.find(txid) != calls.end())
        throw std::runtime_error("Transaction ID already exists");

    call->getRequest()->setTxid(txid);
    calls.insert({txid, call});
    dispatchCall(call);
}

void RPCServer::dispatchCall(Sp<RPCCall>& call) {
    auto request = call->getRequest();
    assert(request != nullptr);

    auto responseHandler = [](RPCCall*, Sp<Message>&) {};
    auto timeoutHandler = [=](RPCCall* _call) {
        auto it = calls.find(_call->getRequest()->getTxid());
        if (it != calls.end()) {
            it->second->getDHT().onTimeout(_call);
            calls.erase(it);
        }
    };


    call->addTimeoutHandler(timeoutHandler);
    call->addResponseHandler(responseHandler);

    request->setAssociatedCall(call.get());
    sendMessage(request);
}

void RPCServer::sendMessage(Sp<Message> msg) {
    msg->setId(node.getId());
    msg->setVersion(Constants::VERSION);

    auto call = msg->getAssociatedCall();
    if (call != nullptr) {
        call->getDHT().onSend(call->getTargetId());
        call->sent(this);

#ifdef MSG_PRINT_DETAIL
        if (!call->getName().empty())
            txidNames[msg->getTxid()] = call->getName();
#endif
    }

    sendData(msg);
}

void RPCServer::sendError(Sp<Message> msg, int code, const std::string& err) {
    auto em = std::make_shared<ErrorMessage>(msg->getMethod(), msg->getTxid(), code, err);
    em->setRemote(msg->getId(), msg->getOrigin());
    sendMessage(em);
}

void RPCServer::handlePacket(const uint8_t *buf, size_t buflen, const SocketAddress& from) {
    Sp<Message> msg = nullptr;
    std::vector<uint8_t> buffer;

    Id sender({buf, ID_BYTES});

    try {
        buffer = node.decrypt(sender, {buf + ID_BYTES, buflen - ID_BYTES});
    } catch(std::exception &e) {
        log->warn("Decrypt packet error from {}, ignored: len {}, {}", from.toString(), buflen, e.what());
        return;
    }

    try {
        msg = Message::parse(buffer.data(), buffer.size());
    } catch(std::exception& e) {
        log->warn("Got a wrong packet from {}, ignored.", from.toString());
        return;
    }

    receivedMessages++;
    msg->setId(sender);
    msg->setOrigin(from);

#ifdef MSG_PRINT_DETAIL
    msg->setName(txidNames[msg->getTxid()]);
    if (filterMessage(msg->name)) {
        log->info("\n\n-- Received: {} bytes -- \nLocal: {}\nFrom: {}\n{}\n-- ** --\n",
                  buflen,  getAddress(from.family()).toString(), from.toString(), static_cast<std::string>(*msg));
    }
#else
    log->info("Received {}/{} from {}: [{}] {}", msg->getMethodString(), msg->getTypeString(),
            from.toString(), buflen, static_cast<std::string>(*msg));
#endif

    // transaction id should be a non-zero integer
    if (msg->getType() != Message::Type::ERROR && msg->getTxid() == 0) {
        log->warn("Received a message with invalid transaction id.");

        sendError(msg, ErrorCode::ProtocolError,
                "Received a message with an invalid transaction id, expected a non-zero transaction id");
    }

    // just respond to incoming requests, no need to match them to pending requests
    if(msg->getType() == Message::Type::REQUEST) {
        handleMessage(msg);
        return;
    }

    // check if this is a response to an outstanding request
    auto it = calls.find(msg->getTxid());
    if (it != calls.end()) {
        Sp<RPCCall> call = it->second;
        // message matches transaction ID and origin == destination
        // we only check the IP address here. the routing table applies more strict checks to also verify a stable port
        if (call->getRequest()->getRemoteAddress() == msg->getOrigin()) {
            // remove call first in case of exception
            calls.erase(it);
            msg->setAssociatedCall(call.get());
            call->responsed(msg);

            // processCallQueue();
            // apply after checking for a proper response
            handleMessage(msg);

            return;
        }

        // 1. the message is not a request
        // 2. transaction ID matched
        // 3. request destination did not match response source!!
        // this happening by chance is exceedingly unlikely
        // indicates either port-mangling NAT, a multhomed host listening on any-local address or some kind of attack
        // ignore response
        log->warn("Transaction id matched, socket address did not, ignoring message, request: {} -> response: {}, version: {}",
                call->getRequest()->getRemoteAddress().toString(), msg->getOrigin().toString(), msg->getReadableVersion());

        if(msg->getType() == Message::Type::RESPONSE && dht6) {
            // this is more likely due to incorrect binding implementation in ipv6. notify peers about that
            // don't bother with ipv4, there are too many complications
            auto err = std::make_shared<ErrorMessage>(msg->getMethod(), msg->getTxid(), ErrorCode::ProtocolError,
                    "A request was sent to " + call->getRequest()->getRemoteAddress().toString() +
                    " and a response with matching transaction id was received from " + msg->getOrigin().toString() +
                    " . Multihomed nodes should ensure that sockets are properly bound and responses are sent with the correct source socket address. See BEPs 32 and 45.");
            err->setRemote(msg->getId(), call->getRequest()->getRemoteAddress());
            sendMessage(err);
        }

        // but expect an upcoming timeout if it's really just a misbehaving node
        call->responseSocketMismatch();
        call->stall();
        return;
    }

    // a) it's not a request
    // b) didn't find a call
    // c) up-time is high enough that it's not a stray from a restart
    // did not expect this response
    if (msg->getType() == Message::Type::RESPONSE && (currentTimeMillis() - startTime) > 2 * 60 * 1000) {
        log->warn("Cannot find RPC call for {}", msg->getType() == Message::Type::RESPONSE
                ? "response" : "error", msg->getTxid());

        sendError(msg, ErrorCode::ProtocolError,
                "Received a response message whose transaction ID did not match a pending request or transaction expired");
        return;
    }

    if (msg->getType() == Message::Type::ERROR) {
        handleMessage(msg);
        return;
    }

    log->debug("Ignored message: {}", static_cast<std::string>(*msg));
}

void RPCServer::handleMessage(Sp<Message> msg) {
    if (msg->getOrigin().family() == AF_INET)
        dht4->get().onMessage(msg);
    else
        dht6->get().onMessage(msg);
}

void RPCServer::periodic() {
    while(!messageQueue.empty()) {
        auto msg = messageQueue.front();
        messageQueue.pop();
        sendData(msg);
    }

    scheduler.syncTime();
    scheduler.run();
}

} // namespace carrier
} // namespace elastos

