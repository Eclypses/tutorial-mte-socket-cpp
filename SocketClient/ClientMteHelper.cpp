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
#include "ClientMteHelper.h"

static MteEnc encoder_;
static MteDec decoder_;
static MteSetupInfo *clientEncoderInfo_ = new MteSetupInfo();
static MteSetupInfo *clientDecoderInfo_ = new MteSetupInfo();

uint64_t ClientMteHelper::getTimestamp()
{
  return static_cast<uint64_t>(time(nullptr));
}

void ClientMteHelper::createGuid(byte_array& guid)
{
  const size_t guidSize = 36;
  const size_t tempBytes = guidSize / 2;

  // Create temp byte array at half the size needed size (so hex value can become guid).
  byte_array temp = MteSetupInfo::createByteArray(tempBytes);

  // Randomly generate values for temp array.
  ecdh_p256_random(temp);

  uint8_t* tempData = new uint8_t[guidSize];
  // Convert temp to hex, then copy these to temp guid byte array.
  memcpy((void*)tempData, bytesToHex(const_cast<uint8_t*>(temp.data), tempBytes), guidSize);

  // Create array to hold hyphen '-' positions.
  const uint8_t hyphens[] = { 8, 13, 18, 23 };

  // Set hyphen '-' symbol at designated positions.
  for (uint8_t i = 0; i < sizeof(hyphens); i++)
  {
    tempData[hyphens[i]] = '-';
  }
  // Copy temp guid.
  guid = MteSetupInfo::createByteArray(tempData, guidSize);
}

char *ClientMteHelper::bytesToHex(uint8_t *in, size_t insz)
{
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

void ClientMteHelper::displayMessage(const byte_array* message)
{
  printf("%s\n", bytesToHex(const_cast<uint8_t*>(message->data), message->size));
}

bool ClientMteHelper::initMte()
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

void ClientMteHelper::finishMte()
{
  // Uninstantiate Encoder and Decoder.
  encoder_.uninstantiate();
  decoder_.uninstantiate();
}

bool ClientMteHelper::createEncoder()
{
  const byte_array publicKey = clientEncoderInfo_->getPublicKey();
  const byte_array peerKey = clientEncoderInfo_->getPeerPublicKey();
  const byte_array nonce = clientEncoderInfo_->getNonce();
  const byte_array personal = clientEncoderInfo_->getPersonalization();

  // Display all info related to the client Encoder.
  printf("Client Encoder public key:\n");
  displayMessage(&publicKey);
  printf("Client Encoder peer's key:\n");
  displayMessage(&peerKey);
  printf("Client Encoder nonce:\n");
  displayMessage(&nonce);
  printf("Client Encoder personalization\n");
  printf("%.*s\n",static_cast<int>(personal.size), reinterpret_cast<char*>(personal.data));

  // Create shared secret.
  const byte_array secret = clientEncoderInfo_->getSharedSecret();

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

  // Delete client Encoder info.
  delete clientEncoderInfo_;

  return true;
}

bool ClientMteHelper::createDecoder()
{
  const byte_array publicKey = clientDecoderInfo_->getPublicKey();
  const byte_array peerKey = clientDecoderInfo_->getPeerPublicKey();
  const byte_array nonce = clientDecoderInfo_->getNonce();
  const byte_array personal = clientDecoderInfo_->getPersonalization();

  // Display all info related to the client Decoder.
  printf("Client Decoder public key:\n");
  displayMessage(&publicKey);
  printf("Client Decoder peer's key:\n");
  displayMessage(&peerKey);
  printf("Client Decoder nonce:\n");
  displayMessage(&nonce);
  printf("Client Decoder personalization\n");
  printf("%.*s\n", static_cast<int>(personal.size), reinterpret_cast<char*>(personal.data));

  // Create shared secret.
  const byte_array secret = clientDecoderInfo_->getSharedSecret();

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

  // Delete client Decoder info.
  delete clientDecoderInfo_;

  return true;
}

bool ClientMteHelper::exchangeMteInfo()
{
  // The client Encoder and the server Decoder will be paired.
  // The client Decoder and the server Encoder will be paired.

  // Prepare to send client information.

  // Create personalization strings.
  byte_array clientEncoderPersonal;
  createGuid(clientEncoderPersonal);
  clientEncoderInfo_->setPersonalization(clientEncoderPersonal.data, clientEncoderPersonal.size);

  byte_array clientDecoderPersonal;
  createGuid(clientDecoderPersonal);
  clientDecoderInfo_->setPersonalization(clientDecoderPersonal.data, clientDecoderPersonal.size);

  // Send out information to the server.
  // 1 - client Encoder public key (to server Decoder)
  // 2 - client Encoder personalization string (to server Decoder)
  // 3 - client Decoder public key (to server Encoder)
  // 4 - client Decoder personalization string (to server Encoder)
  SocketManager::sendMessage('1', clientEncoderInfo_->getPublicKey());
  SocketManager::sendMessage('2', clientEncoderInfo_->getPersonalization());
  SocketManager::sendMessage('3', clientDecoderInfo_->getPublicKey());
  SocketManager::sendMessage('4', clientDecoderInfo_->getPersonalization());

  // Wait for ack from server.
  struct recv_msg recvData = SocketManager::receiveMessage();
  if (recvData.header != 'A')
  {
    return false;
  }

  delete recvData.message.data;

  // Processing incoming messages, all 4 will be needed.
  uint8_t recvCount = 0;

  // Loop until all 4 data are received from server, can be in any order.
  while (recvCount < 4)
  {
    // Receive the next message from the server.
    recvData = SocketManager::receiveMessage();

    // Evaluate the header.
    // 1 - client Decoder public key (from server Encoder)
    // 2 - client Decoder nonce (from server Encoder)
    // 3 - client Encoder public key (from server Decoder)
    // 4 - client Encoder nonce (from server Decoder)
    switch (recvData.header)
    {
    case '1':
      if (clientDecoderInfo_->getPeerPublicKey().size == 0)
      {
        recvCount++;
      }
      clientDecoderInfo_->setPeerPublicKey((uint8_t*)recvData.message.data, recvData.message.size);
      break;
    case '2':
      if (clientDecoderInfo_->getNonce().size == 0)
      {
        recvCount++;
      }
      clientDecoderInfo_->setNonce((uint8_t*)recvData.message.data, recvData.message.size);
      break;
    case '3':
      if (clientEncoderInfo_->getPeerPublicKey().size == 0)
      {
        recvCount++;
      }
      clientEncoderInfo_->setPeerPublicKey((uint8_t*)recvData.message.data, recvData.message.size);
      break;
    case '4':
      if (clientEncoderInfo_->getNonce().size == 0)
      {
        recvCount++;
      }
      clientEncoderInfo_->setNonce((uint8_t*)recvData.message.data, recvData.message.size);
      break;
    default:
      // Unknown message, abort here, send an ‘E’ for error.
      SocketManager::sendMessage('E', "ERR", 3);
      return false;
    }
  }

  // Now all values from server have been received, send an 'A' for acknowledge to server.
  SocketManager::sendMessage('A', "ACK", 3);

  return true;
}

bool ClientMteHelper::encodeMessage(byte_array message, byte_array& encoded)
{
  // Display original message.
  printf("\nMessage to be encoded: %.*s\n",static_cast<int>(message.size), reinterpret_cast<char*>(message.data));

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
  encoded.data = new uint8_t[encoded.size];
  memcpy((void*)encoded.data, encodedMessage, encoded.size);

  // Display encoded message.
  printf("Encoded message being sent:\n");
  displayMessage(&encoded);

  return true;
}

bool ClientMteHelper::decodeMessage(byte_array encoded, byte_array &decoded)
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
















