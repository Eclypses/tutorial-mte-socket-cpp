/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) Eclypses, Inc.
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/
#include <cstring>

#include "EcdhP256.h"



EcdhP256::EcdhP256(): myEntropyCb(nullptr) {
  hasCreatedKeys = false;
  myEntropyInput.data = nullptr;
  myEntropyInput.size = 0;
  // Allocate the buffers.
  myPrivateKey.data = new uint8_t[SzPrivateKey];
  myPrivateKey.size = SzPrivateKey;
  myPublicKey.data = new uint8_t[SzPublicKey];
  myPublicKey.size  = SzPublicKey;
}



EcdhP256::~EcdhP256() {
  // Zeroize the keys.
  ecdh_p256_zeroize(myPrivateKey.data, myPrivateKey.size);
  ecdh_p256_zeroize(myPublicKey.data, myPublicKey.size);
  // Free the buffers.
  delete [] myPrivateKey.data;
  myPrivateKey.data = 0;
  delete [] myPublicKey.data;
  myPublicKey.data = 0;
}



int EcdhP256::createKeyPair(byte_array &publicKey) {
  //---------------------------------------------
  // Check if the keys have already been created. 
  //---------------------------------------------
  if (!hasCreatedKeys) {
    // Check if publicKey is big enough
    // to receive a raw key. We know that
    // myPublicKey is big enough for that.
    if (publicKey.size < SzPublicKey)
      return MemoryFail;
    // Create the private and public keys.
    int res;
    if ((myEntropyCb == nullptr) && (myEntropyInput.data == nullptr))
      res = ecdh_p256_create_keypair(&myPrivateKey, &myPublicKey, nullptr, nullptr);
    else
      res = ecdh_p256_create_keypair(&myPrivateKey, &myPublicKey,
                                     EcdhP256EntropyCallback, this);
    if (res != Success)
      return res;
  }
  //---------------------------------------------
  // Copy the data from myPublicKey to publicKey.
  //---------------------------------------------
  memcpy(publicKey.data, myPublicKey.data, myPublicKey.size);
  publicKey.size = myPublicKey.size;
  hasCreatedKeys = true;
  return Success;
}



int EcdhP256::getSharedSecret(const byte_array peerPublicKey,
                              byte_array &secret) {
  //------------------------------------------------------
  // If the private key has not been set, return an error.
  //------------------------------------------------------
  if (!hasCreatedKeys)
    return InvalidPrivKey;
  //------------------------------------------
  // Check if peerPublicKey is the right size.
  //------------------------------------------
  if (peerPublicKey.size != SzPublicKey)
    return InvalidPubKey;
  //-----------------------------------------------------
  // Check if the result buffer would hold a P256 secret.
  //-----------------------------------------------------
  if (secret.size < SzSecretData)
    return MemoryFail;
  //----------------------
  // Create shared secret.
  //----------------------
  int res = ecdh_p256_create_secret(myPrivateKey, peerPublicKey, &secret);
  // Zeroize the private key and reset "hasCreatedKeys",
  // whether this just worked or not.
  ecdh_p256_zeroize(myPrivateKey.data, myPrivateKey.size);
  hasCreatedKeys = false;
  return res;
}



int EcdhP256::setEntropy(byte_array entropyInput) {
  if (entropyInput.size != SzPrivateKey)
    return MemoryFail;
  myEntropyInput = entropyInput;
  return Success;
}



void EcdhP256::setEntropyCallback(EntropyCallback *cb) {
  myEntropyCb = cb;
}



int EcdhP256::entropyCallback(byte_array entropyInput) {
  // Call the callback if set.
  if (myEntropyCb != nullptr) {
    return myEntropyCb->entropyCallback(entropyInput);
  }
  // Check the length.
  if ((myEntropyInput.data == nullptr) ||
      (myEntropyInput.size != SzPrivateKey) ||
      (entropyInput.size != SzPrivateKey))
    return InvalidPrivKey;
  // Copy entropy data and destroy original.
  memcpy(entropyInput.data, myEntropyInput.data, entropyInput.size);
  ecdh_p256_zeroize(myEntropyInput.data, myEntropyInput.size);
  myEntropyInput.data = nullptr;
  myEntropyInput.size = 0;
  // Success.
  return Success;
}



EcdhP256::EntropyCallback::~EntropyCallback() {
  // Nothing to do here.
}



int EcdhP256EntropyCallback(void *context, byte_array entropyInput) {
  EcdhP256 *ecdh = reinterpret_cast<EcdhP256 *>(context);
  return ecdh->entropyCallback(entropyInput);
}



int EcdhP256::getRandom(byte_array random) {
  return ecdh_p256_random(random);
}



void EcdhP256::zeroize(void *dest, size_t size) {
  ecdh_p256_zeroize(dest, size);
}
