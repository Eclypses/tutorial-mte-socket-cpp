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
#include "mte_base64.h"

//---------------------------------------------------
// MKE and Fixed length add-ons are NOT in all SDK
// MTE versions. If the name of the SDK includes
// "-MKE" then it will contain the MKE add-on. If the
// name of the SDK includes "-FLEN" then it contains
// the Fixed length add-on.
//---------------------------------------------------

/* Step 7 */
//---------------------------------------------------
// Uncomment the following two #includes to use the
// MTE core.
//---------------------------------------------------
#include "MteEnc.h"
#include "MteDec.h"
//---------------------------------------------------
// Uncomment the following two #includes to use the
// MKE add-on.
//---------------------------------------------------
//#include "MteMkeEnc.h"
//#include "MteMkeDec.h"
//---------------------------------------------------
// Uncomment the following two #includes to use the
// Fixed length add-on.
//---------------------------------------------------
//#include "MteFlenEnc.h"
//#include "MteDec.h"

#define IsLittleEndian char (0x0001)
#define _CRT_SECURE_NO_WARNINGS

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "27015"
#define DEFAULT_SERVER_IP "localhost"

/* Step 8 */

//--------------------------------------------
// The fixed length, only needed for MTE FLEN
//--------------------------------------------
size_t fixedBytes = 8;

//----------------------------------------------------------
// Create the MTE decoder, uncomment to use MTE core OR FLEN
// Create the Mte Fixed length decoder (SAME as MTE Core)
//----------------------------------------------------------
MteDec g_decoder;
//---------------------------------------------------
// Create the Mte MKE decoder, uncomment to use MKE
//---------------------------------------------------
//MteMkeDec g_decoder;
mte_status g_decoderStatus;

//---------------------------------------------------
// Create the Mte encoder, uncomment to use MTE core
//---------------------------------------------------
MteEnc g_encoder;
std::string mteType = "Core";
//---------------------------------------------------
// Create the Mte MKE encoder, uncomment to use MKE
//---------------------------------------------------
//MteMkeEnc g_encoder;
//std::string mteType = "MKE";
//---------------------------------------------------------------
// Create the Mte Fixed length encoder, uncomment to use MTE FLEN
//---------------------------------------------------------------
//MteFlenEnc g_encoder(fixedBytes);
//std::string mteType = "FLEN";
mte_status g_encoderStatus;

/* Step 9 - Part 1 */
// Set default entropy, nonce and identifier
// Providing Entropy in this fashion is insecure. This is for demonstration purposes only and should never be done in practice. 
// If this is a trial version of the MTE, entropy must be blank - set this is global variables
uint8_t* g_entropy;

// OPTIONAL!!! adding 1 to decoder nonce so return value changes -- same nonce can be used for encoder and decoder
// on client side values will be switched so they match up encoder to decoder and vice versa
int g_encoderNonce = 1;
int g_decoderNonce = 0;
std::string g_personal = "demo";

void createEncoder();
void createDecoder();

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
	//
	// This tutorial uses Sockets for communication.
	// It should be noted that the MTE can be used with any type of communication. (SOCKETS are not required!)
	//

	std::cout << "Starting C++ Socket Client." << std::endl;

	// Display what version of MTE we are using
	const char* mteVersion = MteBase::getVersion();
	std::cout << "Using MTE Version: " << mteVersion << "-" + mteType << std::endl;

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

	//
	// Step 10
	// Check mte license.
	// Initialize MTE license. If a license code is not required (e.g., trial
	// mode), this can be skipped. This demo attempts to load the license info
	// from the environment if required.
	if (!MteBase::initLicense("LicenseCompanyName", "LicenseKey"))
	{
		std::cerr << "License init error ("
			<< MteBase::getStatusName(mte_status_license_error)
			<< "): "
			<< MteBase::getStatusDescription(mte_status_license_error)
			<< std::endl;
		return mte_status_license_error;
	}

	/* Step 11 */
	createEncoder();

	createDecoder();

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

		//
		// Step 12
		// encode text to send
		size_t encodedBytes;
		const void* encoded = g_encoder.encode(textToSend, encodedBytes, g_encoderStatus);
		if (g_encoderStatus != mte_status_success)
		{
			std::cout << "Error encoding: Status: "
				<< MteBase::getStatusName(g_encoderStatus)
				<< "/"
				<< MteBase::getStatusDescription(g_encoderStatus)
				<< std::endl;
			std::cout << "Socket client is closed due to encoding error, press ENTER to end this..." << std::endl;
			std::cin;
			return g_encoderStatus;
		}

		//
		// For demonstration purposes only to show packets
		size_t base64EncodedBytes = mte_base64_encode_bytes(encodedBytes);
		char* base64CharArray = new char[base64EncodedBytes];
		mte_base64_encode(encoded, encodedBytes, base64CharArray);
		std::cout << "Base64 encoded representation of the packet being sent: " << base64CharArray << std::endl;

		//
		// This puts the bytes of the send length.
		std::string toSendLenBytes;

		toSendLenBytes += (unsigned char)((encodedBytes) & 0xFF);
		toSendLenBytes += (unsigned char)((encodedBytes >> 8) & 0xFF);
		toSendLenBytes += (unsigned char)((encodedBytes >> 16) & 0xFF);
		toSendLenBytes += (unsigned char)((encodedBytes >> 24) & 0xFF);

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
				res = sendData(reinterpret_cast<const char*>(encoded), encodedBytes);
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

				//
				// Step 12 Continue
				// Decode incoming message and check for non-error response.
				// When checking the status on decode use "statusIsError".
				// Only checking if status is success can be misleading, there may be a
				// warning returned that the user can ignore.
				// See MTE Documentation for more details.
				size_t decodedBytes;
				const char* returnedText = static_cast<const char*>(g_decoder.decode(recvbuf, rcvLen, decodedBytes, g_decoderStatus));

				if (g_decoder.statusIsError(g_decoderStatus))
				{
					std::cout << "Error decoding: Status: "
						<< MteBase::getStatusName(g_decoderStatus)
						<< "/"
						<< MteBase::getStatusDescription(g_decoderStatus)
						<< std::endl;
					std::cout << "Socket client is closed due to decoding error, press ENTER to end this..." << std::endl;
					std::cin;
					return g_decoderStatus;
				}

				//
				// Convert byte array to string to view in console (this step is for display purposes)				
				base64EncodedBytes = mte_base64_encode_bytes(rcvLen);
				base64CharArray = new char[base64EncodedBytes];
				mte_base64_encode(recvbuf, rcvLen, base64CharArray);
				std::cout << "Base64 encoded representation of the received packet: " << base64CharArray << std::endl;

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

/* Step 11*/
/// <summary>
/// Creates the encoder.
/// </summary>
void createEncoder()
{
	// Check how long entropy we need, set default
	const size_t entropyMinBytes = MteBase::getDrbgsEntropyMinBytes(g_encoder.getDrbg());
	g_entropy = new uint8_t[entropyMinBytes];
	memset(g_entropy, '0', entropyMinBytes);
	g_encoder.setEntropy(g_entropy, entropyMinBytes);

	g_encoder.setNonce(g_encoderNonce);

	g_encoderStatus = g_encoder.instantiate(g_personal);
	if (g_encoderStatus != mte_status_success)
	{
		std::cout << "Encoder instantiate error ("
			<< MteBase::getStatusName(g_encoderStatus)
			<< "): "
			<< MteBase::getStatusDescription(g_encoderStatus)
			<< std::endl;
	}
}

/// <summary>
/// Creates the decoder.
/// </summary>
void createDecoder()
{
	// Check how long entropy we need, set default
	const size_t entropyMinBytes = g_decoder.getDrbgsEntropyMinBytes(g_decoder.getDrbg());

	g_entropy = new uint8_t[entropyMinBytes];
	// When entropy is set in the encoder, it is 'zeroed' out, therefore, we need to 'refill' it before setting in decoder
	memset(g_entropy, '0', entropyMinBytes);
	g_decoder.setEntropy(g_entropy, entropyMinBytes);

	g_decoder.setNonce(g_decoderNonce);

	g_decoderStatus = g_decoder.instantiate(g_personal);
	if (g_decoderStatus != mte_status_success)
	{
		std::cout << "Decoder instantiate error ("
			<< MteBase::getStatusName(g_decoderStatus)
			<< "): "
			<< MteBase::getStatusDescription(g_decoderStatus)
			<< std::endl;
	}
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