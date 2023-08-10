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

#include <carrier.h>

#include "utils.h"
#include "value_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ValueTests);

void ValueTests::testImmutable() {
    auto data = Utils::getRandomData(32);
    auto value = Value::createValue(data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(!value.isMutable());
    CPPUNIT_ASSERT(!value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(!value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(value.getData() == data);
}

void ValueTests::testSigned1() {
    auto data = Utils::getRandomData(32);
    auto value = Value::createSignedValue(data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(value.isMutable());
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(value.hasPrivateKey());
    CPPUNIT_ASSERT(!value.getSignature().empty());
    CPPUNIT_ASSERT(value.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(value.getData() == data);
}

void ValueTests::testSigned2() {
    auto data = Utils::getRandomData(32);
    auto keypair = Signature::KeyPair::random();
    auto nonce = CryptoBox::Nonce::random();
    auto value = Value::createSignedValue(keypair, nonce, data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(value.isMutable());
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getPublicKey() == keypair.publicKey());
    CPPUNIT_ASSERT(value.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(value.getNonce() == nonce);
    CPPUNIT_ASSERT(!value.getSignature().empty());
    CPPUNIT_ASSERT(value.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(value.getData() == data);
}

void ValueTests::testSigned3() {
    auto data = Utils::getRandomData(32);
    auto keypair = Signature::KeyPair::random();
    auto nonce = CryptoBox::Nonce::random();
    int sequenceNumber = 55;
    auto value = Value::createSignedValue(keypair, nonce, sequenceNumber, data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(value.isMutable());
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getPublicKey() == keypair.publicKey());
    CPPUNIT_ASSERT(value.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(value.getNonce() == nonce);
    CPPUNIT_ASSERT(!value.getSignature().empty());
    CPPUNIT_ASSERT(value.getSequenceNumber() == sequenceNumber);
    CPPUNIT_ASSERT(value.getData() == data);
}

void ValueTests::testEncrypted1() {
    auto data = Utils::getRandomData(32);
    auto keypair = Signature::KeyPair::random();
    auto nodeId = keypair.publicKey();
    auto value = Value::createEncryptedValue(nodeId, data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(value.isMutable());
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(value.isEncrypted());
    CPPUNIT_ASSERT(value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getRecipient() == nodeId);
    CPPUNIT_ASSERT(!value.getSignature().empty());
    CPPUNIT_ASSERT(value.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(value.getData() != data); // encrypted data.
}

void ValueTests::testEncrypted2() {
    auto data = Utils::getRandomData(32);
    auto keypair = Signature::KeyPair::random();
    auto nodeId = keypair.publicKey();
    auto nonce = CryptoBox::Nonce::random();
    auto value = Value::createEncryptedValue(keypair, nodeId, nonce, data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(value.isMutable());
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(value.isEncrypted());
    CPPUNIT_ASSERT(value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getPublicKey() == keypair.publicKey());
    CPPUNIT_ASSERT(value.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(value.getNonce() == nonce);
    CPPUNIT_ASSERT(value.getRecipient() == nodeId);
    CPPUNIT_ASSERT(!value.getSignature().empty());
    CPPUNIT_ASSERT(value.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(value.getData() != data); // encrypted data.
}

void ValueTests::testEncrypted3() {
    auto data = Utils::getRandomData(32);
    auto keypair = Signature::KeyPair::random();
    auto nodeId = keypair.publicKey();
    auto nonce = CryptoBox::Nonce::random();
    int sequenceNumber = 55;
    auto value = Value::createEncryptedValue(keypair, nodeId, nonce, sequenceNumber, data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(value.isMutable());
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(value.isEncrypted());
    CPPUNIT_ASSERT(value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getPublicKey() == keypair.publicKey());
    CPPUNIT_ASSERT(value.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(value.getNonce() == nonce);
    CPPUNIT_ASSERT(value.getRecipient() == nodeId);
    CPPUNIT_ASSERT(!value.getSignature().empty());
    CPPUNIT_ASSERT(value.getSequenceNumber() == sequenceNumber);
    CPPUNIT_ASSERT(value.getData() != data); // encrypted data.
}

}  // namespace test
