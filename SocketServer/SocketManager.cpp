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

#include "SocketManager.h"

int32_t m_sock = -1;
int32_t s_sock = -1;
struct sockaddr_in m_addr;
struct sockaddr_in rm_addr;
struct hostent* hp;

int SocketManager::createSocket()
{
  memset(&m_addr, 0, sizeof(m_addr));
  memset(&rm_addr, 0, sizeof(rm_addr));

#if defined _WIN32
  long RESPONSE;
  struct WSAData WinSockData;
  WORD DLLVERSION = MAKEWORD(2, 1);
  RESPONSE = WSAStartup(DLLVERSION, &WinSockData);
  if (RESPONSE != 0)
  {
    return 0;
  }
#endif

  m_sock = (int32_t)socket(AF_INET, SOCK_STREAM, 6);

  if (!isSocketValid())
  {
    return 0;
  }

  // TIME_WAIT - argh
  int32_t on = 1;
  if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
  {
    return 0;
  }

  return 1;
}

int SocketManager::bindSocket(int port)
{
  if (!isSocketValid())
  {
    return 0;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons((uint16_t)port);

  int32_t bind_return = bind(m_sock, (struct sockaddr*)&m_addr, sizeof(m_addr));

  printf("Socket server listening on port: %i\n", port);

  if (bind_return == -1)
  {
    return 0;
  }
  return 1;
}

int SocketManager::listenSocket()
{
  if (!isSocketValid())
  {
    return 0;
  }

  int32_t listen_return = listen(m_sock, 1);

  if (listen_return == -1)
  {
    return 0;
  }

  return 1;
}

int SocketManager::acceptSocket(char* port)
{
  struct sockaddr_in client_addr;
  socklen_t slen = sizeof(client_addr);
  s_sock = (int32_t)accept(m_sock, (struct sockaddr*)&client_addr, &slen);
  if (s_sock == 0)
  {
    return 0;
  }
  printf("Socket Server is listening on %s : port %s\n", inet_ntoa(client_addr.sin_addr), port);
  return 1;
}

void SocketManager::closeSocket()
{
  if (isSocketValid())
  {
#if defined _WIN32
    closesocket(m_sock);
#else
    close(m_sock);
#endif
  }
}

size_t SocketManager::sendMessage(const char header, const char* message, size_t messageBytes)
{
  // Create a union to be able to set the length as a simple size.
  // Then the char array will automatically be set and
  // ready to be sent to the other side, possibly needing to reverse
  // depending on Endianess.
  union bytes_length
  {
    uint32_t length;
    char arr[5];
  };

  // Get the length of the packet to send.
  union bytes_length to_send_len_bytes;
  to_send_len_bytes.length = (uint32_t)(messageBytes);

  // Check if little Endian and reverse if no - all sent in Big Endian.
#ifdef LITTLE_ENDIAN  
  int size = sizeof(to_send_len_bytes.length);
  for (int i = 0; i < size / 2; i++)
  {
    char temp = to_send_len_bytes.arr[i];
    to_send_len_bytes.arr[i] = to_send_len_bytes.arr[size - 1 - i];
    to_send_len_bytes.arr[size - 1 - i] = temp;
  }
#endif

  // Put the header byte into 5th byte
  to_send_len_bytes.arr[4] = header;


  // Send the message size as big-endian and the header byte.
  size_t res = (size_t)(send(s_sock, to_send_len_bytes.arr, sizeof(to_send_len_bytes.arr), 0));
  if (res < sizeof(to_send_len_bytes.arr))
  {
    printf("Sending the message size failed.");
    closeSocket();
    return 0;
  }

  // Send the actual message.
  // Chunk the message into chunks.
  res = 0;
  size_t bytes_sent = 0;
  while (res < messageBytes)
  {
    size_t sending_size = messageBytes - res;
    if (CHUNK_BYTES < sending_size)
    {
      sending_size = CHUNK_BYTES;
    }

    bytes_sent = static_cast<size_t>(send(s_sock, message + res, static_cast<int>(sending_size), 0));
    if (bytes_sent == 0)
    {
      // No bytes were sent;
      return 0;
    }
    res += bytes_sent;
  }

  if (res < messageBytes)
  {
    printf("Sending the message failed.");
    closeSocket();
    return 0;
  }

  return res;
}

size_t SocketManager::sendMessage(const char header, const byte_array message)
{
  return sendMessage(header, (char*)message.data, message.size);
}

struct recv_msg SocketManager::receiveMessage()
{
  // Create recv_msg struct.
  struct recv_msg msg_struct;
  msg_struct.success = false;
  msg_struct.header = '\0';
  msg_struct.message.data = nullptr;
  msg_struct.message.size = 0;

  // Create a union to be able to get the char array from the Client.
  // It may need to be reversed depending on Endianess.
  // Then the length will have already been set.
  union bytes_length
  {
    uint32_t length;
    char arr[5];
  };

  // Create an array to hold the message size coming in.
  union bytes_length to_recv_len_bytes;
  if (!recvData((uint8_t*)(to_recv_len_bytes.arr), sizeof(to_recv_len_bytes.arr)))
    return msg_struct;

  // Check if little Endian and reverse if no - all received in Big Endian.
#ifdef LITTLE_ENDIAN
  int size = sizeof(to_recv_len_bytes.length);
  for (int i = 0; i < size / 2; i++)
  {
    char temp = to_recv_len_bytes.arr[i];
    to_recv_len_bytes.arr[i] = to_recv_len_bytes.arr[size - 1 - i];
    to_recv_len_bytes.arr[size - 1 - i] = temp;
  }
#endif

  // Get the header byte from the 5th byte
  msg_struct.header = to_recv_len_bytes.arr[4];

  // Receive the message from the Client.
  msg_struct.message.data = (uint8_t*)malloc(to_recv_len_bytes.length);
  if (!recvData((uint8_t*)msg_struct.message.data, to_recv_len_bytes.length))
    return  msg_struct;

  // The size will be the rest of the message.
  msg_struct.message.size = to_recv_len_bytes.length;

  // Set status to true;
  msg_struct.success = true;

  return msg_struct;
}

bool SocketManager::isSocketValid()
{
  return m_sock != -1;
}

bool SocketManager::recvData(uint8_t* data, size_t bytesNeeded)
{
  size_t res = 0;
  size_t bytes_read = 0;
  while (res < bytesNeeded)
  {
    size_t receiving_size = bytesNeeded - res;
    if (CHUNK_BYTES < receiving_size)
    {
      receiving_size = CHUNK_BYTES;
    }

    bytes_read = static_cast<size_t>(recv(s_sock, reinterpret_cast<char*>(data) + res, static_cast<int>(receiving_size), 0));
    if (bytes_read == 0)
    {
      // No bytes were received;
      return false;
    }
    res += bytes_read;
  }

  if (res < bytesNeeded)
  {
    printf("Receiving the message failed.");
    closeSocket();
    return false;
  }

  return true;
}