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
#ifndef EcdhP256_h
#define EcdhP256_h

#include <cstdlib>
#include <memory>
#include <mtesupport_ecdh.h>



class EcdhP256 {

public:
  const static int Success = 0;
  const static int RandomFail = -1;
  const static int InvalidPubKey = -2;
  const static int InvalidPrivKey = -3;
  const static int MemoryFail = -4;

  const static int SzPrivateKey = SZ_ECDH_P256_PRIVATE_KEY;
  const static int SzPublicKey = SZ_ECDH_P256_PUBLIC_KEY;
  const static int SzSecretData = SZ_ECDH_P256_SECRET_DATA;

  int createKeyPair(byte_array &publicKey);
  int getSharedSecret(const byte_array peerPublicKey, byte_array &secret);
  int setEntropy(byte_array entropyInput);

  static int getRandom(byte_array random);
  static void zeroize(void *dest, size_t size);

  class EntropyCallback {

  public:
    virtual ~EntropyCallback();
    virtual int entropyCallback(byte_array entropyInput) = 0;
  };


public:
  EcdhP256();
  virtual ~EcdhP256();
  void setEntropyCallback(EntropyCallback *cb);


public:
  // This method is public only so the C callback can access it.
  // It is not meant to be used directly.

  // The entropy callback.
  int entropyCallback(byte_array entropyInput);


protected:
  byte_array myPrivateKey;
  byte_array myPublicKey;


private:  
  bool hasCreatedKeys = false;
  EntropyCallback *myEntropyCb;
  byte_array myEntropyInput;

};

extern "C" int EcdhP256EntropyCallback(void *context, byte_array entropyInput);

#endif
