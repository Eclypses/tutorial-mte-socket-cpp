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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

#define IsLittleEndian char (0x0001)
#define _CRT_SECURE_NO_WARNINGS

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "27015"

/// <summary>
/// Creates the socket.
/// </summary>
/// <returns></returns>
static int createSocket();

/// <summary>
/// Closes the socket.
/// </summary>
static void closeSocket();

/// <summary>
/// Determines if the socket is valid.
/// </summary>
/// <returns></returns>
static int isSocketValid();

/// <summary>
/// Binds to the socket.
/// </summary>
/// <param name="port"></param>
/// <returns></returns>
static int bindSocket(int port);

/// <summary>
/// Listens via the socket.
/// </summary>
/// <returns></returns>
static int listenToSocket();

/// <summary>
/// Accepts the socket
/// </summary>
/// <param name="port">The port number.</param>
/// <returns></returns>
static int acceptSocket(const char* port);

/// <summary>
/// Sends the data through the socket.
/// </summary>
/// <param name="data">The data to be sent.</param>
/// <param name="data_size">The size of the data.</param>
/// <returns>The number of bytes sent.</returns>
static int sendData(const char* data, uint32_t data_size);

/// <summary>
/// Receives the data through the socket.
/// </summary>
/// <param name="data">The data to be received.</param>
/// <param name="data_size">The size of the data.</param>
/// <returns></returns>
static int recvData(char* data, uint32_t data_size);

int32_t m_sock = -1;
int32_t s_sock = -1;
struct sockaddr_in m_addr;
struct sockaddr_in rm_addr;
struct hostent* hp;

/// <summary>
/// Mains the specified argc.
/// </summary>
/// <param name="argc">The argc.</param>
/// <param name="argv">The argv.</param>
/// <returns>int.</returns>
int main(int argc, char** argv)
{
	std::cout << "Starting C++ Socket Server." << std::endl;

	std::cout << "Please enter port to use, press Enter to use default: " << DEFAULT_PORT << std::endl;
	std::string port;
	std::getline(std::cin, port);
	port.erase(std::remove(port.begin(), port.end(), '\r'), port.end());
	if (port.empty())
	{
		port = DEFAULT_PORT;
	}

	memset(&m_addr, 0, sizeof(m_addr));
	memset(&rm_addr, 0, sizeof(rm_addr));

#ifdef _WIN32
	WSADATA wsaData;
#endif

	char recvbuf[DEFAULT_BUFLEN];

	int socket_creation = createSocket();
	if (socket_creation == 0)
	{
		std::cerr << "Unable to create socket." << std::endl;
		return socket_creation;
	}

	int socket_binding = bindSocket(atoi(port.c_str()));
	if (socket_binding == 0)
	{
		std::cerr << "Unable to bind to socket." << std::endl;
		return socket_binding;
	}

	std::cout << "Listening for new Client connection..." << std::endl;
	int socket_listening = listenToSocket();
	if (socket_listening == 0)
	{
		std::cerr << "Unable to listen to socket." << std::endl;
		return socket_listening;
	}

	int socket_accepting = acceptSocket(port.c_str());
	if (socket_accepting == 0)
	{
		std::cerr << "Unable to accept the socket." << std::endl;
		return socket_accepting;
	}

	//Set/Prompt for all parameters needed
	std::string ipAddress = "localhost";

	// Receive until the peer shuts down the connection
	while (true)
	{

		std::cout << "Listening for messages from Client . . ." << std::endl;

		//
		// Get the length of bytes coming in
		char rcvLenBytes[4];
		int res = recvData(rcvLenBytes, 4);
		if (res == 0)
		{
			break;
		}

		unsigned char recLenBytes[4];
		recLenBytes[0] = rcvLenBytes[0];
		recLenBytes[1] = rcvLenBytes[1];
		recLenBytes[2] = rcvLenBytes[2];
		recLenBytes[3] = rcvLenBytes[3];

#if defined IsLittleEndian
		for (int low = 0, high = 4 - 1; low < high; low++, high--)
		{
			std::swap(recLenBytes[low], recLenBytes[high]);
		}
#endif

		uint32_t rcvLen = *(reinterpret_cast<int32_t*>(recLenBytes));

		res = recvData(recvbuf, rcvLen);
		std::string rcvBytesString(recvbuf, rcvLen);

		char* rcvBytes = new char[rcvLen];
		memset(rcvBytes, '\0', rcvLen);
		for (uint32_t i = 0; i < rcvLen; i++)
		{
			rcvBytes[i] = rcvBytesString[i];
		}

		//
		// If there is a message to receive, we want to grab it
		if (rcvLen > 0)
		{
			//
			// For demonstration purposes only to show packets
			std::cout << "Received packet: " << rcvBytesString << std::endl;

			size_t receivedLength = rcvLen;

			//
			// This puts the bytes of the send length
			std::string toSendLenBytes;
			toSendLenBytes += (unsigned char)receivedLength & 0xFF;
			toSendLenBytes += (unsigned char)((receivedLength >> 8) & 0xFF);
			toSendLenBytes += (unsigned char)((receivedLength >> 16) & 0xFF);
			toSendLenBytes += (unsigned char)((receivedLength >> 24) & 0xFF);

			//
			// Check if little Endian and reverse if so - all sent in Big Endian
#if defined IsLittleEndian
			for (int low = 0, high = 4 - 1; low < high; low++, high--)
			{
				std::swap(toSendLenBytes[low], toSendLenBytes[high]);
			}
#endif

			std::cout << "Packet Sent: " << rcvBytesString << std::endl;

			//
			// Send the length of the message
			res = sendData(toSendLenBytes.c_str(), 4);

			//
			// Send the actual message
			res = sendData(rcvBytesString.c_str(), (uint32_t)strlen(rcvBytesString.c_str()));
		}
		closeSocket();
	}

	//
	// Close server socket and prompt to end
	std::cout << "Program stopped." << std::endl;

	return 0;
}

static int createSocket()
{
#ifdef _WIN32
	long RESPONSE;
	struct WSAData WinSockData;
	WORD DLLVERSION = MAKEWORD(2, 1);
	RESPONSE = WSAStartup(DLLVERSION, &WinSockData);
#endif

	m_sock = socket(AF_INET, SOCK_STREAM, 0);

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

static void closeSocket()
{
	if (isSocketValid())
	{
#ifdef _WIN32
		closesocket(m_sock);
#else
		close(m_sock);
#endif

	}
}

static int isSocketValid()
{
	return m_sock != -1;
}

static int bindSocket(int port)
{
	if (!isSocketValid())
	{
		return 0;
	}

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = INADDR_ANY;
	m_addr.sin_port = htons(port);

	int32_t bind_return = bind(m_sock, (struct sockaddr*)&m_addr, sizeof(m_addr));

	if (bind_return == -1)
	{
		return 0;
	}
	return 1;
}

static int listenToSocket()
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

static int acceptSocket(const char* port)
{
	struct sockaddr_in client_addr;
	socklen_t slen = sizeof(client_addr);
	s_sock = accept(m_sock, (struct sockaddr*)&client_addr, &slen);
	if (s_sock == 0)
	{
		return 0;
	}
	std::cout << "Socket Server is listening on " << inet_ntoa(client_addr.sin_addr) << " : port " << port << std::endl;
	return 1;
}

static int sendData(const char* data, uint32_t data_size)
{
	int32_t status = send(s_sock, data, data_size, 0);
	if (status == -1)
	{
		return 0;
	}
	else
	{
		return status;
	}
}

static int recvData(char data[], uint32_t data_size)
{
	// Get Message length
	int32_t charCount = 0;

	charCount = recv(s_sock, data, data_size, 0);

	return charCount;
}
