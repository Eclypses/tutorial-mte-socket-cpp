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

#ifndef SocketManager_h
#define SocketManager_h

#include "mtesupport_ecdh.h"
#include "globals.h"

struct recv_msg
{
  bool success;
  char header;
  byte_array message;
};

class SocketManager
{

public:
  /// <summary>
  /// Creates the socket.
  /// </summary>
  /// <returns>1 if socket was created, 0 otherwise.</returns>
  static int createSocket();

  /// <summary>
  /// Binds the socket to the port.
  /// </summary>
  /// <param name="port">The port number.</param>
  /// <returns>1 if socket was bound, 0 otherwise.</returns>
  static int bindSocket(int port);

  /// <summary>
  /// Listens for a socket connection.
  /// </summary>
  /// <returns>1 is socket is listening, 0 otherwise.</returns>
  static int listenSocket();

  /// <summary>
  /// Accepts the socket connection.
  /// </summary>
  /// <param name="port">The port number.</param>
  /// <returns>1 is socket connection was accepted, 0 otherwise.</returns>
  static int acceptSocket(char* port);

  /// <summary>
  /// Closes the socket.
  /// </summary>
  static void closeSocket();

  /// <summary>
  /// Sends the message through the socket.
  /// </summary>
  /// <param name="header">The header to go with the message.</param>
  /// <param name="message">The message to be sent.</param>
  /// <param name="messageBytes">The size of the message in bytes.</param>
  /// <returns>The number of bytes sent.</returns>
  static size_t sendMessage(const char header, const char* message, size_t messageBytes);

  /// <summary>
  /// Sends the byte array message through the socket.
  /// </summary>
  /// <param name="header">The header to go with the message.</param>
  /// <param name="message">The message byte array.</param>
  /// <returns>The number of bytes sent.</returns>
  static size_t sendMessage(const char header, const byte_array message);

  /// <summary>
  /// Receives the message through the socket.
  /// </summary>
  /// <returns>Struct that contains the message, message size in bytes, header, and success result.</returns>
  static struct recv_msg receiveMessage();

private:
  /// <summary>
  /// Determines if the socket is valid.
  /// </summary>
  /// <returns></returns>
  static bool isSocketValid();

  /// <summary>
  /// Receives data for the amount of bytes needed.
  /// </summary>
  /// <param name="data">The data to be received.</param>
  /// <param name="bytesNeeded">The </param>
  /// <returns></returns>
  static bool recvData(uint8_t* data, size_t bytesNeeded);
};

#endif