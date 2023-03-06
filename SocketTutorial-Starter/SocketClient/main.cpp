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
#define strcasecmp _stricmp
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <thread>
#include <vector>
#include <string>

#define IsLittleEndian char (0x0001)
#define _CRT_SECURE_NO_WARNINGS

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "27015"
#define DEFAULT_SERVER_IP "localhost"

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

/// <summary>
/// Connects to the socket.
/// </summary>
/// <param name="host"></param>
/// <param name="port"></param>
/// <returns></returns>
static int connectSocket(const char* host, uint16_t port);

int32_t m_sock = -1;
struct sockaddr_in m_addr;
struct sockaddr_in rm_addr;
struct hostent* hp;

int main(int argc, char** argv)
{
	std::cout << "Starting C++ Socket Client." << std::endl;

	// Request server ip and port
	std::cout << "Please enter ip address of Server, press Enter to use default: " << DEFAULT_SERVER_IP << std::endl;
	std::string ipAddress;
	std::getline(std::cin, ipAddress);
	ipAddress.erase(std::remove(ipAddress.begin(), ipAddress.end(), '\r'), ipAddress.end());
	if (ipAddress.empty())
	{
		ipAddress = DEFAULT_SERVER_IP;
	}

	std::cout << "Server is at " << ipAddress << std::endl;

	std::cout << "Please enter port to use, press Enter to use default: " << DEFAULT_PORT << std::endl;
	std::string port;
	std::getline(std::cin, port);
	port.erase(std::remove(port.begin(), port.end(), '\r'), port.end());
	if (port.empty())
	{
		port = DEFAULT_PORT;
	}

	std::string textToSend;

	memset(&m_addr, 0, sizeof(m_addr));
	memset(&rm_addr, 0, sizeof(rm_addr));

#ifdef _WIN32
	WSADATA wsaData;
#endif

	char recvbuf[DEFAULT_BUFLEN];
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif

	int socket_creation = createSocket();
	if (socket_creation == 0)
	{
		std::cerr << "Unable to create socket." << std::endl;
		return socket_creation;
	}

	int socket_connection = connectSocket(ipAddress.c_str(), atoi(port.c_str()));
	if (socket_connection == 0)
	{
		std::cerr << "Unable to connect to socket." << std::endl;
		return socket_connection;
	}

	std::cout << "Client connected to server." << std::endl;

	while (true)
	{
		//
		// Prompt user for input to send to other side
		std::cout << "Please enter text to send : (To end please type 'quit')" << std::endl;
		std::getline(std::cin, textToSend);

		std::string lowerCaseText = textToSend;
		std::for_each(lowerCaseText.begin(), lowerCaseText.end(), [](char& c) {
			c = ::tolower(c);
			});

		//
		// Check to see if we are quitting - if so set sendMessages to false so this is the last time it runs
		if (std::strcmp("quit", lowerCaseText.c_str()) == 0)
		{
			break;
		}

		std::cout << "Packet being sent: " << textToSend << std::endl;

		//
		// This puts the bytes of the send length.
		std::string toSendLenBytes;
		size_t textToSendLength = textToSend.length();

		toSendLenBytes += (unsigned char)((textToSendLength) & 0xFF);
		toSendLenBytes += (unsigned char)((textToSendLength >> 8) & 0xFF);
		toSendLenBytes += (unsigned char)((textToSendLength >> 16) & 0xFF);
		toSendLenBytes += (unsigned char)((textToSendLength >> 24) & 0xFF);

		//
		// Check if little Endian and reverse if so - all sent in Big Endian
#if defined IsLittleEndian
		for (int low = 0, high = 4 - 1; low < high; low++, high--)
		{
			std::swap(toSendLenBytes[low], toSendLenBytes[high]);
		}
#endif
		bool connected = false;
		while (!connected)
		{
			try
			{
				//
				// Send the length-prefix				
				int res = sendData(toSendLenBytes.c_str(), 4);
				if (res <= 0)
				{
					std::cerr << "Send failed." << std::endl;
					closeSocket();
					return 1;
				}

				//
				// Send the actual message
				res = sendData(reinterpret_cast<const char*>(textToSend.c_str()), textToSendLength);
				if (res <= 0)
				{
					std::cerr << "Send failed." << std::endl;
					closeSocket();
					return 1;
				}

				// Receive the response from the remote device.
				//
				// First get the length-prefix
				char rcvLenBytes[4];
				res = recvData(rcvLenBytes, 4);
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

				//
				// Get message
				res = recvData(recvbuf, rcvLen);

				std::string rcvBytesString(recvbuf, rcvLen);

				std::cout << "Received packet: " << rcvBytesString << std::endl;

				connected = true;
			}
			catch (const std::exception& ex)
			{
				int waitTime = 1000;
				std::cout << ex.what() << "Server application not available yet. Attempting to connect again in " << waitTime / 1000 << " seconds." << std::endl;
#ifdef _WIN32
				Sleep(1000);
#else
				sleep(1);
#endif
			}
		}
	}

	// shutdown the connection since no more data will be sent
	closeSocket();

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

static int sendData(const char* data, uint32_t data_size)
{
	int32_t status = send(m_sock, data, data_size, 0);
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
	int32_t charCount = 0;

	charCount = recv(m_sock, data, data_size, 0);

	return charCount;
}

static int connectSocket(const char* host, uint16_t port)
{
	if (!isSocketValid())
	{
		return 0;
	}

	m_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);

	int32_t status = inet_pton(AF_INET, host, &m_addr.sin_addr);

	status = connect(m_sock, (struct sockaddr*)&m_addr, sizeof(m_addr));
	if (status == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}