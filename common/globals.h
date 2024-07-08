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
#ifndef globals_h
#define globals_h

/*-------------------------------------------------------
 * Include the correct header for the htons() function
 * and determine if this machine is little or big endian.
 *-----------------------------------------------------*/
#if defined _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define strcasecmp _stricmp
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <cstdio>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
#define LITTLE_ENDIAN (!BIG_ENDIAN)
#endif

#define DEFAULT_PORT 27015
#define DEFAULT_SERVER_IP "127.0.0.1"
#define MAX_INPUT_BYTES 100
#define CHUNK_BYTES 64

#ifndef  __cplusplus
#endif

#if defined (_WIN32)
#define socket_size int
#else
#define socket_size size_t
#endif

/**************************************************************
* A simple struct for managing a uint8_t* along with its size;
* The "size" member can be used to define the actual size of
* the byte array or the number of bytes used in a byte array,
* depending on the context.
*************************************************************/
  typedef struct byte_array_
  {
    size_t size;
    uint8_t *data;
  } byte_array;

#define LICENSE_NAME "LicenseCompanyName"
#define LICENSE_KEY "LicenseKey"

//---------------------------------------------------
// MKE and Fixed length add-ons are NOT in all SDK
// MTE versions. If the name of the SDK includes
// "-MKE" then it will contain the MKE add-on. If the
// name of the SDK includes "-FLEN" then it contains
// the Fixed length add-on.
//---------------------------------------------------
//
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

#ifdef __cplusplus
}
#endif

#endif