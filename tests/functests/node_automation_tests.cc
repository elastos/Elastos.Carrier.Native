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

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

// std
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

// carrier
#include <carrier.h>
#include "utils.h"
#include "node_automation_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(NodeAutomationTester);

void NodeAutomationTester::setUp() {
    dataDir = Utils::getPwdStorage("node_automation_tests_data");
    Utils::removeStorage(dataDir);

    auto builder = DefaultConfiguration::Builder {};
    std::string conf_path = "." + Utils::PATH_SEP + "automation.conf";
    builder.load(conf_path);
    builder.setIPv4Address(Utils::getLocalIpAddresses());
    builder.setListeningPort(42232);

    std::string path = dataDir + Utils::PATH_SEP + "temp1";
    builder.setStoragePath(dataDir);
    auto configuration = builder.build();

    node = std::make_shared<Node>(configuration);
    node->start();

    bootstrapNodes = configuration->getBootstrapNodes();
}

void NodeAutomationTester::testAutomaticNode() {
    auto bn = bootstrapNodes[Utils::getRandomInteger(bootstrapNodes.size())];

    std::cout << "----------" << std::endl;
    std::cout << "Trying to find node: " << bn->getId() << std::endl;
    auto future = node->findNode(bn->getId());
    auto n = future.get();
    CPPUNIT_ASSERT(!n.empty());

    std::cout << "----------" << std::endl;
    std::vector<uint8_t> data({0,1,2,3,4});
    auto value = Value::createValue(data);
    std::cout << "Trying to Sotre Value: " << value.getId() << std::endl;
    auto future1 = node->storeValue(value);
    future1.get();
    std::cout << "Store value succeeeed." << std::endl;

    std::cout << "----------" << std::endl;
    std::cout << "Trying to find Value with Id: " << value.getId() << std::endl;
    auto future2 = node->findValue(value.getId());
    auto val = future2.get();
    CPPUNIT_ASSERT(value == *val);
    std::cout << "Find value: " << val->toString() << std::endl;

    std::cout << "----------" << std::endl;
    std::cout << "Trying to announce peer " << std::endl;

    auto peerId = Id::random();
    auto nodeId = Id::random();
    auto origin = Id::random();
    std::vector<uint8_t> sig(64);
    Random::buffer(sig.data(), sig.size());

#if 0
    static PeerInfo create(const Id& nodeId, Id origin, int port, const std::string& alternativeURL) {

    auto future3 = node->announcePeer(peer);
    future3.get();
    std::cout << "Announce peer succeeeed." << std::endl;

    std::cout << "----------" << std::endl;
    std::cout << "Trying to find peer with Id: " << peerId << std::endl;

    //annotate momentarily，because super node don't support the new format of peer
    /*auto future4 = node->findPeer(peerId, 1, LookupOption::CONSERVATIVE);
    auto peers = future4.get();
    CPPUNIT_ASSERT(!peers.empty());
    for (auto& peer: peers) {
        std::cout << "Peer: " << peer << std::endl;
        peer->isValid();
    }*/
#endif
}


void NodeAutomationTester::tearDown() {
    if (node != nullptr)
        node->stop();

    Utils::removeStorage(dataDir);
}
}  // namespace test
