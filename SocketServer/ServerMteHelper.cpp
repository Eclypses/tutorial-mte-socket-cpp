/*
THIS SOFTWARE MAY NOT BE USED FOR PRODUCTION. Otherwise,
The MIT License (MIT)

Copyright (c) Eclypses, Inc.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ServerMteHelper.h"

static MteEnc encoder_;
static MteDec decoder_;
static MteSetupInfo *serverEncoderInfo_ = new MteSetupInfo();
static MteSetupInfo *serverDecoderInfo_ = new MteSetupInfo();

bool ServerMteHelper::initMte()
{
  // Initialize MTE license. If a license code is not required (e.g., trial
  // mode), this can be skipped.
  if (!MteBase::initLicense("LicenseCompanyName", "LicenseKey"))
  {
    printf("There was an error attempting to initialize the MTE License.\n");
    return false;
  }

  // Exchange entropy, nonce, and personalization string between the client and server.
  if (!exchangeMteInfo())
  {
    printf("There was an error attempting to exchange information between this and the client.\n");
    return false;
  }

  return true;
}

bool ServerMteHelper::createEncoder()
{
  const byte_array publicKey = serverEncoderInfo_->getPublicKey();
  const byte_array peerKey = serverEncoderInfo_->getPeerPublicKey();
  const byte_array nonce = serverEncoderInfo_->getNonce();
  const byte_array personal = serverEncoderInfo_->getPersonalization();

  // Display all info related to the server Encoder.
  printf("Server Encoder public key:\n");
  displayMessage(&publicKey);
  printf("Server Encoder peer's key:\n");
  displayMessage(&peerKey);
  printf("Server Encoder nonce:\n");
  displayMessage(&nonce);
  printf("Server Encoder personalization\n");
  printf("%.*s\n", static_cast<int>(personal.size), reinterpret_cast<char*>(personal.data));

  // Create shared secret.
  const byte_array secret = serverEncoderInfo_->getSharedSecret();

  // Set Encoder entropy using this shared secret.
  encoder_.setEntropy((void*)secret.data, secret.size);

  // Set Encoder nonce.
  encoder_.setNonce(nonce.data, nonce.size);

  // Instantiate Encoder.
  const mte_status status = encoder_.instantiate(personal.data, personal.size);
  if (status != mte_status_success)
  {
    fprintf(stderr, "Encoder instantiate error (%s): %s\n",
      MteBase::getStatusName(status),
      MteBase::getStatusDescription(status));
    return false;
  }

  // Delete server Encoder info.
  delete serverEncoderInfo_;

  return true;
}

bool ServerMteHelper::createDecoder()
{
  const byte_array publicKey = serverDecoderInfo_->getPublicKey();
  const byte_array peerKey = serverDecoderInfo_->getPeerPublicKey();
  const byte_array nonce = serverDecoderInfo_->getNonce();
  const byte_array personal = serverDecoderInfo_->getPersonalization();

  // Display all info related to the server Decoder.
  printf("Server Decoder public key:\n");
  displayMessage(&publicKey);
  printf("Server Decoder peer's key:\n");
  displayMessage(&peerKey);
  printf("Server Decoder nonce:\n");
  displayMessage(&nonce);
  printf("Server Decoder personalization\n");
  printf("%.*s\n", static_cast<int>(personal.size), reinterpret_cast<char*>(personal.data));

  // Create shared secret.
  const byte_array secret = serverDecoderInfo_->getSharedSecret();

  // Set Decoder entropy using this shared secret.
  decoder_.setEntropy((void*)secret.data, secret.size);

  // Set Decoder nonce.
  decoder_.setNonce(nonce.data, nonce.size);

  // Instantiate Decoder.
  const mte_status status = decoder_.instantiate(personal.data, personal.size);
  if (status != mte_status_success)
  {
    fprintf(stderr, "Decoder instantiate error (%s): %s\n",
      MteBase::getStatusName(status),
      MteBase::getStatusDescription(status));
    return false;
  }

  // Delete server Decoder info.
  delete serverDecoderInfo_;

  return true;
}

bool ServerMteHelper::encodeMessage(byte_array message, byte_array& encoded)
{
  // Display original message.
  printf("\nMessage to be encoded: %.*s\n", static_cast<int>(message.size), reinterpret_cast<char*>(message.data));

  // Encode the message.
  mte_status status;
  const void* encodedMessage = encoder_.encode(message.data, message.size, encoded.size, status);
  // Ensure that it encoded successfully.
  if (status != mte_status_success)
  {
    fprintf(stderr, "Error encoding (%s): %s\n",
      MteBase::getStatusName(status),
      MteBase::getStatusDescription(status));
    return false;
  }
  encoded = MteSetupInfo::createByteArray((uint8_t*)encodedMessage, encoded.size);

  // Display encoded message.
  printf("Encoded message being sent:\n");
  displayMessage(&encoded);

  return true;
}

bool ServerMteHelper::decodeMessage(byte_array encoded, byte_array& decoded)
{
  // Display encoded message.
  printf("\nEncoded message received:\n");
  displayMessage(&encoded);

  // Decode the encoded message.
  mte_status status;
  const void* decodedMessage = decoder_.decode(encoded.data, encoded.size, decoded.size, status);

  // Ensure that there were no decoding errors.
  if (MteBase::statusIsError(status))
  {
    fprintf(stderr, "Error decoding: Status: %s/%s\n",
      MteBase::getStatusName(status),
      MteBase::getStatusDescription(status));
    return false;
  }

  // Set decoded message.
  decoded.data = new uint8_t[decoded.size];
  memcpy((void*)decoded.data, decodedMessage, decoded.size);

  // Display decoded message.
  printf("Decoded message: %.*s\n", static_cast<int>(decoded.size), reinterpret_cast<char*>(decoded.data));

  return true;
}

void ServerMteHelper::finishMte()
{
  // Uninstantiate Encoder and Decoder.
  encoder_.uninstantiate();
  decoder_.uninstantiate();
}

bool ServerMteHelper::exchangeMteInfo()
{
  // The client Encoder and the server Decoder will be paired.
  // The client Decoder and the server Encoder will be paired.

  // Processing incoming messages, all 4 will be needed.
  uint8_t recvCount = 0;
  struct recv_msg recvData;

  // Loop until all 4 data are received from client, can be in any order.
  while (recvCount < 4)
  {
    // Receive the next message from the client.
    recvData = SocketManager::receiveMessage();

    // Evaluate the header.
    // 1 - server Decoder public key (from client Encoder)
    // 2 - server Decoder personalization string (from client Encoder)
    // 3 - server Encoder public key (from client Decoder)
    // 4 - server Encoder personalization string (from client Decoder)
    switch (recvData.header)
    {
    case '1':
      if (serverDecoderInfo_->getPeerPublicKey().size == 0)
      {
        recvCount++;
      }

      serverDecoderInfo_->setPeerPublicKey((uint8_t*)recvData.message.data, recvData.message.size);
      break;
    case '2':
      if (serverDecoderInfo_->getPersonalization().size == 0)
      {
        recvCount++;
      }
      serverDecoderInfo_->setPersonalization(recvData.message.data, recvData.message.size);
      break;
    case '3':
      if (serverEncoderInfo_->getPeerPublicKey().size == 0)
      {
        recvCount++;
      }
      serverEncoderInfo_->setPeerPublicKey((uint8_t*)recvData.message.data, recvData.message.size);
      break;
    case '4':
      if (serverEncoderInfo_->getPersonalization().size == 0)
      {
        recvCount++;
      }
      serverEncoderInfo_->setPersonalization(recvData.message.data, recvData.message.size);
      break;
    default:
      // Unknown message, abort here, send an ‘E’ for error.
      SocketManager::sendMessage('E', "ERR", 3);
      return false;
    }
  }

  // Now all values from client have been received, send an 'A' for acknowledge to client.
  SocketManager::sendMessage('A', "ACK", 3);

  // Prepare to send server information now.

  // Create nonces.
  size_t minNonceBytes = MteBase::getDrbgsNonceMinBytes(MTE_DRBG_ENUM);
  if (minNonceBytes == 0)
  {
    minNonceBytes = 1;
  }

  byte_array serverEncoderNonce;
  serverEncoderNonce.data = new uint8_t[minNonceBytes];
  serverEncoderNonce.size = minNonceBytes;
  int res = ecdh_p256_random(serverEncoderNonce);
  if (res < 0)
  {
    return false;
  }
  serverEncoderInfo_->setNonce(serverEncoderNonce.data, serverEncoderNonce.size);

  byte_array serverDecoderNonce;
  serverDecoderNonce.data = new uint8_t[minNonceBytes];
  serverDecoderNonce.size = minNonceBytes;
  res = ecdh_p256_random(serverDecoderNonce);
  if (res < 0)
  {
    return false;
  }
  serverDecoderInfo_->setNonce(serverDecoderNonce.data, serverDecoderNonce.size);

  // Send out information to the client.
  // 1 - server Encoder public key (to client Decoder)
  // 2 - server Encoder nonce (to client Decoder)
  // 3 - server Decoder public key (to client Encoder)
  // 4 - server Decoder nonce (to client Encoder)
  SocketManager::sendMessage('1', serverEncoderInfo_->getPublicKey());
  SocketManager::sendMessage('2', serverEncoderInfo_->getNonce());
  SocketManager::sendMessage('3', serverDecoderInfo_->getPublicKey());
  SocketManager::sendMessage('4', serverDecoderInfo_->getNonce());

  // Wait for ack from client.
  recvData = SocketManager::receiveMessage();
 
  return (recvData.header == 'A');
}

uint64_t ServerMteHelper::getTimestamp(void* context) {
  // We don't use timestamp verifiers in this tutorial.
  // But if your MTE build does have timestamp verifiers enabled,
  // we must provide a valid callback function. This is such
  // a dummy function. It returns zero. In a real environment,
  // we could return "time(NULL)".
  return 0;
}

char* ServerMteHelper::bytesToHex(uint8_t* in, size_t insz) {
  if (LOGBUF_SZ < (2 * insz + 1)) {
    insz = (LOGBUF_SZ - 1) / 2;
  }
  uint8_t* pin = in;
  const char* hex = "0123456789ABCDEF";
  char* pout = logbuf;
  while (pin < in + insz) {
    *pout++ = hex[(*pin >> 4) & 0xF];
    *pout++ = hex[*pin++ & 0xF];
  }
  *pout = 0;
  return logbuf;
}

void ServerMteHelper::displayMessage(const byte_array* message)
{
  printf("%s\n", bytesToHex(const_cast<uint8_t*>(message->data), message->size));
}