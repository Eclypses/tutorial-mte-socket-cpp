

<img src="Eclypses.png" style="width:50%;margin-right:0;"/>

<div align="center" style="font-size:40pt; font-weight:900; font-family:arial; margin-top:300px; " >
C++ Socket Tutorial</div>
<br>
<div align="center" style="font-size:28pt; font-family:arial; " >
MTE Implementation Tutorial (MTE Core, MKE, MTE Fixed Length)</div>
<br>
<div align="center" style="font-size:15pt; font-family:arial; " >
Using MTE version 4.x.x</div>





[Introduction](#introduction)

[Socket Tutorial Server and Client](#socket-tutorial-server-and-client)


<div style="page-break-after: always; break-after: page;"></div>

# Introduction

This tutorial is sending messages via a socket connection. This is only a sample, the MTE does NOT require the usage of sockets, most communication protocols can be used..

This tutorial demonstrates how to use MTE Core, MKE, and Fixed Length. For this application, only one type can be used at a time. However, any new or existing application can be adapted to handle multiple types at the same time.

This tutorial contains two main programs, a client and a server, and also for Windows and Linux. Note that any of the available language socket tutorials can be used for any available platform as long as communication is possible between the platforms. It is recommended that a server program is started and wait for a client program to start and establish a connection.

The MTE Encoder and Decoder need several pieces of information to be the same in order to function properly. This includes entropy, nonce, and personalization. If this information must be shared, the entropy MUST be passed securely. One way to do this is with a Diffie-Hellman key exchange. Each side will then be able to create two shared secrets to use as entropy for each pair of Encoder/Decoder. The two personalization values will be created by the client and shared to the other side. The two nonce values will be created by the server and shared with the client.

***MTE Core:*** This is the default functionality of the MTE to use. Unless payloads are large or sequencing is needed this is the recommended style of the MTE and the most secure.

***MTE MKE:*** This add-on of the MTE is recommended when payloads are very large, the MTE Core would, depending on the token byte size, be multiple times larger than the original payload. Because this uses the MTE technology on encryption keys and encrypts the payload, the payload is only enlarged minimally. If the SDK contains "-MKE" in the name then this add-on is included.

***MTE Fixed Length:*** This add-on of the MTE is very secure and is used when the resulting payload is desired to be the same size for every transmission. The Fixed Length add-on is mainly used when using the sequencing verifier with MTE. In order handle dropped, skipped, or asynchronous packets the sequencing verifier requires that all packets be a predictable size. The Fixed Length add-on is a great choice to make that easier. This is ONLY an Encoder change - the Decoder that is used is the MTE Core Decoder. All data smaller than the fixed length size will be padded, and all data larger will be dropped to the fixed length size. When strings are used, any extra padding will be removed. Raw bytes will contain zeros for any padding; any trimming needed should be handled by the application. If the SDK contains "-FLEN" in the name then this add-on is included.

***MTE Random:*** This add-on will help with the generation of cryptographically secure random numbers. This application uses it for the generation of information that will be used for initializing the Encoder/Decoder.  Note that for Windows, the bcrypt.dll library may need to be linked in during compilation.

***MTE ECDH:*** This add-on utilizes the Elliptic Curve Diffie-Hellman (ECDH) key agreement protocol. This application uses it to generate public keys that get exchanged and to generate shared secrets to be used as entropy for initiliazing Encoders and Decoders. If the SDK contains "-ECDH" in the name then this add-on is included. Note that this application will need this add-on to function as-is; however, the application could be modified to use another key exchange protocol if needed.

In this tutorial, there is an MTE Encoder on the client that is paired with an MTE Decoder on the server. Likewise, there is an MTE Encoder on the server that is paired with an MTE Decoder on the client. Secured messages wil be sent to and from both sides. If a system only needs to secure messages one way, only one pair could be used.

**IMPORTANT**
>Please note the solution provided in this tutorial does NOT include the MTE library or supporting MTE library files. Please contact Eclypses Inc. if the MTE SDK (which contatins the library and supporting files) has NOT been provided. The solution will only work AFTER the MTE library and other files have been incorporated.

# Socket Tutorial Server and Client

## MTE Directory and File Setup
<ol>
<li>
Navigate to the "tutorial-mte-socket-cpp" directory.
</li>
<li>
Create a directory named "MTE". This will contain all needed MTE files.
</li>
<li>
Copy the "lib" directory and contents from the MTE SDK into the "MTE" directory.
</li>
<li>
Copy the "include" directory and contents from the MTE SDK into the "MTE" directory.
</li>
<li>
Copy the "src/cpp" directory and contents from the MTE SDK into the "MTE" directory.
</li>
<li>
Copy the "src/c" directory and contents from the MTE SDK into the "MTE" directory.
</li>
<li>
Copy this "MTE" directory into both the "SocketClient" and "SockerServer" directories.
</li>
</ol>


The common source code between the client and server will be found in the "common" directory. The client and server specific source code will be found in their respective directories.

## Project Settings
<ol>
<li>
Ensure that the include directory path contains the path to the "MTE/include" and "common" directories. 
</li>
<li>
Ensure that the library directory path contains the path to the "MTE/lib" directory.
</li>
<li>
The projects by defualt use the dynamic MTE library (mte.dll for Windows and libmte.so for Linux). For Windows the bcrypt dll is also needed for the MTE Random functionality.
</li>
</ol>

## Source Code Key Points

### MTE Setup

<ol>
<li>
Utilize preprocessor directives to more easily handle the function calls for the MTE Core or the add-on configurations. In the file "globals.h", uncomment 'USE_MTE_CORE' to utilize the main MTE Core functions; uncomment 'USE_MKE_ADDON' to use the MTE MKE add-on functions; or uncomment 'USE_FLEN_ADDON' to use the Fixed length add-on functions. In this application, only one can be used at a time. This file is shared between the two projects, so both projects will have the changes reflected accordingly.

```cpp
//-----------------------------------
// To use the core MTE, uncomment the
// following preprocessor definition.
//-----------------------------------
#define USE_MTE_CORE
//---------------------------------------
// To use the MTE MKE add-on, uncomment
// the following preprocessor definition.
//---------------------------------------
//#define USE_MKE_ADDON
//-------------------------------------------------
// To use the MTE Fixed length add-on,
// uncomment the following preprocessor definition.
//-------------------------------------------------
//#define USE_FLEN_ADDON
```

</li>

<li>
In this application, the MTE ECDH add-on is used by each side (client and server) to create public and private keys. The public keys are then shared between the client and server, and then shared secrets are generated with the other's public key to use as matching entropy for the creation of the Encoders and Decoders. The personalization strings and nonces are created using the MTE Random add-on.

```cpp
// Create the private and public keys.
const int res = ecdhManager.createKeyPair(publicKey.data, publicKey.size);
if (res)
{
  throw res;
}
```
The C++ MTE ECDH class will keep the private key to itself and not provide access to the calling application.
</li>
<li>
The public keys created by the client will be sent to the server, and vice versa, and will be received as <i>peer public keys</i>. Then the shared secret can be created on each side. These should match as long as the information has been created and shared correctly.

```cpp
// Create temp byte array.
byte_array temp = createByteArray(MteEcdh::SzSecretData);

// Create shared secret.
const int res = ecdhManager.createSecret(peerKey.data, peerKey.size, temp.data, temp.size);
if (res < 0)
{
  throw MteEcdh::MemoryFail;
}
```
These secrets will then be used to fufill the entropy needed for the Encoders and Decoders.
</li>
<li>
The client will create the personalization strings, in this case a guid-like structure using MTE Random.

```cpp
void ClientMteHelper::createGuid(byte_array &guid)
{
  const size_t guidSize = 36;
  const size_t tempBytes = guidSize / 2;

  // Create temp byte array at half the size needed size (so hex value can become guid).
  byte_array temp = MteSetupInfo::createByteArray(tempBytes);

  // Randomly generate values for temp array.
  MteRandom::getBytes(temp.data, temp.size);

  uint8_t *tempData = new uint8_t[guidSize];

  // Convert temp to hex, then copy these to temp guid byte array.
  memcpy((void *)tempData, bytesToHex(const_cast<uint8_t *>(temp.data), tempBytes), guidSize);

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
```
</li>
<li>
The two public keys and the two personalization strings will then be sent to the server. The client will wait for an acknowledgment.

```cpp
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
if (recvData.header != 'a')
{
  return false;
}

  delete recvData.message.data;
```
</li>
<li>
The server will wait for the two public keys and the two personalization strings from the client. Once all four pieces of information have been received, it will send an acknowledgment.

```cpp
// Processing incoming messages, all 4 will be needed.
uint8_t recvCount = 0;
recv_msg recvData;

byte_array message;

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

    serverDecoderInfo_->setPeerPublicKey((uint8_t *)recvData.message.data, recvData.message.size);
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
    serverEncoderInfo_->setPeerPublicKey((uint8_t *)recvData.message.data, recvData.message.size);
    break;
  case '4':
    if (serverEncoderInfo_->getPersonalization().size == 0)
    {
      recvCount++;
    }
    serverEncoderInfo_->setPersonalization(recvData.message.data, recvData.message.size);
    break;
  default:
    // Unknown message, abort here.
    return false;
  }
}

// Now all values from client have been received, send an 'A' for acknowledge to client.
message.size = 3;
message.data = (uint8_t *)("ack");
SocketManager::sendMessage('a', message);
```
</li>
<li>
The server will create the private and public keypairs, one for the server Encoder and client Decoder, and one for the server Decoder and client Encoder. The server uses the same file "MTESeuptInfo.cpp" 

```cpp
// Create the private and public keys.
const int res = ecdhManager.createKeyPair(publicKey.data, publicKey.size);
if (res)
{
  throw res;
}
```

</li>
<li>
The server will create the nonces, using the platform supplied secure RNG.

```cpp
// Create nonces.
size_t minNonceBytes = MteBase::getDrbgsNonceMinBytes(MTE_DRBG_ENUM);
if (minNonceBytes == 0)
{
  minNonceBytes = 1;
}

byte_array serverEncoderNonce = MteSetupInfo::createByteArray(minNonceBytes);
int res = MteRandom::getBytes(serverEncoderNonce.data, serverEncoderNonce.size);
if (res < 0)
{
  return false;
}
serverEncoderInfo_->setNonce(serverEncoderNonce.data, serverEncoderNonce.size);

byte_array serverDecoderNonce = MteSetupInfo::createByteArray(minNonceBytes);
res = MteRandom::getBytes(serverDecoderNonce.data, serverDecoderNonce.size);
if (res < 0)
{
  return false;
}
serverDecoderInfo_->setNonce(serverDecoderNonce.data, serverDecoderNonce.size);
```
</li>
<li>
The two public keys and the two nonces will then be sent to the client. The server will wait for an acknowledgment. 
```cpp
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

return (recvData.header == 'a');
```
</li>

<li>
The client will now wait for information from the server. This includes the two server public keys, and the two nonces. Once all pieces of information have been obtained, the client will send an acknowledgment back to the server.

```cpp
// Processing incoming messages, all 4 will be needed.
uint8_t recvCount = 0;

byte_array message;

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
    clientDecoderInfo_->setPeerPublicKey((uint8_t *)recvData.message.data, recvData.message.size);
    break;
  case '2':
    if (clientDecoderInfo_->getNonce().size == 0)
    {
      recvCount++;
    }
    clientDecoderInfo_->setNonce((uint8_t *)recvData.message.data, recvData.message.size);
    break;
  case '3':
    if (clientEncoderInfo_->getPeerPublicKey().size == 0)
    {
      recvCount++;
    }
    clientEncoderInfo_->setPeerPublicKey((uint8_t *)recvData.message.data, recvData.message.size);
    break;
  case '4':
    if (clientEncoderInfo_->getNonce().size == 0)
    {
      recvCount++;
    }
    clientEncoderInfo_->setNonce((uint8_t *)recvData.message.data, recvData.message.size);
    break;
  default:
    // Unknown message, abort here.
    return false;
  }
}

// Now all values from server have been received, send an 'A' for acknowledge to server.
message.size = 3;
message.data = (uint8_t *)("ack");
SocketManager::sendMessage('a', message);
```

</li>
<li>
After the client and server have exchanged their information, the client and server can each create their respective Encoder and Decoder. This is where the personalization string and nonce will be added. Additionally, the entropy will be set by getting the shared secret from ECDH. This sample code showcases the client Encoder. There will be four of each of these that will be very similar. Ensure carefully that each function uses the appropriate client/server, and Encoder/Decoder variables and functions.

```cpp
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
printf("%.*s\n", static_cast<int>(personal.size), reinterpret_cast<char *>(personal.data));

// Create shared secret.
const byte_array secret = clientEncoderInfo_->getSharedSecret();

// Set Encoder entropy using this shared secret.
encoder_->setEntropy((void *)secret.data, secret.size);

// Set Encoder nonce.
encoder_->setNonce(nonce.data, nonce.size);

// Instantiate Encoder.
const mte_status status = encoder_->instantiate(personal.data, personal.size);
if (status != mte_status_success)
{
  fprintf(stderr, "Encoder instantiate error (%s): %s\n",
    MteBase::getStatusName(status),
    MteBase::getStatusDescription(status));
  return false;
}

  // Delete client Encoder info.
  delete clientEncoderInfo_;
```

</li>
</ol>

### Diagnostic Test
<ol>
<li>
The application will run a diagnostic test, where the client will encode the word "ping", then send the encoded message to the server. The server will decode the received message to confirm that the original message is "ping". Then the server will encode the word "ack" and send the encoded message to the client. The client then decodes the received message, and confirms that it decodes it to the word "ack". 
</li>
</ol>

### User Interaction
<ol>
<li>
The application will continously prompt the user for an input (until the user types "quit"). That input will be encoded with the client Encoder and sent to the server.

```cpp
bool ClientMteHelper::encodeMessage(byte_array message, byte_array& encoded)
{
  // Display original message.
  printf("\nMessage to be encoded: %.*s\n",message.size, message.data);

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
```
</li>
<li>
The server will use its Decoder to decode the message.

```c
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
  printf("Decoded message: %.*s\n", decoded.size, decoded.data);

  return true;
}
```

</li>
<li>
Then that message will be re-encoded with the server Encoder and sent to the client.The client Decoder will then decode that message, which then will be compared with the original user input.
</li>
</ol>

### MTE Finialize

<ol>
<li>
Once the user has stopped the user input, the program should securely clear out MTE Encoder and Decoder information.

```c

// Uninstantiate Encoder and Decoder.
encoder_->uninstantiate();
decoder_->uninstantiate();

delete encoder_;
delete decoder_;
```
</li>
</ol>

<div style="page-break-after: always; break-after: page;"></div>

# Contact Eclypses

<img src="Eclypses.png" style="width:8in;"/>

<p align="center" style="font-weight: bold; font-size: 20pt;">Email: <a href="mailto:info@eclypses.com">info@eclypses.com</a></p>
<p align="center" style="font-weight: bold; font-size: 20pt;">Web: <a href="https://www.eclypses.com">www.eclypses.com</a></p>
<p align="center" style="font-weight: bold; font-size: 20pt;">Chat with us: <a href="https://developers.eclypses.com/dashboard">Developer Portal</a></p>

<p style="font-size: 8pt; margin-bottom: 0; margin: 300px 24px 30px 24px; " >
<b>All trademarks of Eclypses Inc.</b> may not be used without Eclypses Inc.'s prior written consent. No license for any use thereof has been granted without express written consent. Any unauthorized use thereof may violate copyright laws, trademark laws, privacy and publicity laws and communications regulations and statutes. The names, images and likeness of the Eclypses logo, along with all representations thereof, are valuable intellectual property assets of Eclypses, Inc. Accordingly, no party or parties, without the prior written consent of Eclypses, Inc., (which may be withheld in Eclypses' sole discretion), use or permit the use of any of the Eclypses trademarked names or logos of Eclypses, Inc. for any purpose other than as part of the address for the Premises, or use or permit the use of, for any purpose whatsoever, any image or rendering of, or any design based on, the exterior appearance or profile of the Eclypses trademarks and or logo(s).
</p>