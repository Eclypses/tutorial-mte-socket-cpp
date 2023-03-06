

<img src="Eclypses.png" style="width:50%;margin-right:0;"/>

<div align="center" style="font-size:40pt; font-weight:900; font-family:arial; margin-top:300px; " >
C++ Socket Tutorial</div>
<br>
<div align="center" style="font-size:28pt; font-family:arial; " >
MTE Implementation Tutorial (MTE Core, MKE, MTE Fixed Length)</div>
<br>
<div align="center" style="font-size:15pt; font-family:arial; " >
Using MTE version 3.0.x</div>





[Introduction](#introduction)

[Socket Tutorial Server and Client](#socket-tutorial-server-and-client)


<div style="page-break-after: always; break-after: page;"></div>

# Introduction

This tutorial is sending messages via a socket connection. This is only a sample, the MTE does NOT require the usage of sockets, you can use whatever communication protocol that is needed.

This tutorial demonstrates how to use Mte Core, Mte MKE and Mte Fixed Length. Depending on what your needs are, these three different implementations can be used in the same application OR you can use any one of them. They are not dependent on each other and can run simultaneously in the same application if needed.

The SDK that you received from Eclypses may not include the MKE or MTE FLEN add-ons. If your SDK contains either the MKE or the Fixed Length add-ons, the name of the SDK will contain "-MKE" or "-FLEN". If these add-ons are not there and you need them please work with your sales associate. If there is no need, please just ignore the MKE and FLEN options.

Here is a short explanation of when to use each, but it is encouraged to either speak to a sales associate or read the dev guide if you have additional concerns or questions.

***MTE Core:*** This is the recommended version of the MTE to use. Unless payloads are large or sequencing is needed this is the recommended version of the MTE and the most secure.

***MTE MKE:*** This version of the MTE is recommended when payloads are very large, the MTE Core would, depending on the token byte size, be multiple times larger than the original payload. Because this uses the MTE technology on encryption keys and encrypts the payload, the payload is only enlarged minimally.

***MTE Fixed Length:*** This version of the MTE is very secure and is used when the resulting payload is desired to be the same size for every transmission. The Fixed Length add-on is mainly used when using the sequencing verifier with MTE. In order to skip dropped packets or handle asynchronous packets the sequencing verifier requires that all packets be a predictable size. If you do not wish to handle this with your application then the Fixed Length add-on is a great choice. This is ONLY an encoder change - the decoder that is used is the MTE Core decoder.

In this tutorial we are creating an MTE Encoder and an MTE Decoder in the server as well as the client because we are sending secured messages in both directions. This is only needed when there are secured messages being sent from both sides, the server as well as the client. If only one side of your application is sending secured messages, then the side that sends the secured messages should have an Encoder and the side receiving the messages needs only a Decoder.

These steps should be followed on the server side as well as on the client side of the program.

**IMPORTANT**
>Please note the solution provided in this tutorial does NOT include the MTE library or supporting MTE library files. If you have NOT been provided an MTE library and supporting files, please contact Eclypses Inc. The solution will only work AFTER the MTE library and MTE library files have been incorporated.
  

# Socket Tutorial Server and Client

<ol>
<li>Copy the include directory from the mte-Windows or mte-Linux package (as appropriate) to both SocketClient and SocketServer directories.</li>
<br>
<li>If using the MTE Core, copy the MteBase.h, MteBase.cpp, MteEnc.h, MteEnc.cpp, MteDec.h, MteDec.cpp files from the “src/cpp” directory from the package to each project into the include directory. (SocketClient AND SocketServer directories). If using the Mte MKE, copy the MteBase.h, MteBase.cpp, MteMkeEnc.h, MteMkeEnc.cpp, MteMkeDec.h, MteMkeDec.cpp files. If using the Mte Fixed length, copy the MteBase.h, MteBase.cpp, MteFlenEnc.h, MteFlenEnc.cpp, MteDec.h, MteDec.cpp files.</li>
<br>
<li>Copy the lib directory from the mte-Windows or mte-Linux package (as appropriate) to both SocketClient and SocketServer directories.</li>

<br>
<li>Update the SocketClient and SocketServer project settigs with the following:
<ul>
<li>The addional include directories will need to add <b><i>include</i></b> as an entry.</li>
<li>The addional library directories will need to add <b><i>lib</i></b> as an entry.</li>
<li>The additonal dependiencies will need to add <b><i>mte.lib;</i></b> (Windows) / library dependiencies will need to add <b><i>mte;</i></b> (Linux) as an entry.</li>
</ul>
</li>
<br>
<li>Add all source code files as inside the include directory of each project as existing files. (SocketClient and SocketServer).</li>
<br>
<li>Ensure that the dynamic libraries will be in the expected directory when the executable will run.</li> 
<ol type="a">
<li>For Windows, one way this can be done is to set in a post-build event command in each project with:</li>

```batchfile
xcopy /y /d  "$(ProjectDir)lib\mte.dll" "$(OutDir)"
```
This will copy the dynamic library to the same directory as the executable after it is built.
<li>For Linux, add the libmte.so file to each project. Additionally, it may need to be copied to a directory set in the environment path <code>LD_LIBRARY_PATH</code>.</li>
</ol> 
<br>
<li>Add include statements for both the MTE Encoder and MTE Decoder near the beginning of the main.cpp files (SocketClient And SocketServer).</li>

```c++
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
```
<li>Create the MTE Decoder and MTE Encoder as well as the accompanying MTE status for each as global variables. Also include fixed length parameter if using FLEN.</li>

***IMPORTANT NOTE***
>If using the fixed length MTE (FLEN), all messages that are sent that are longer than the set fixed length will be trimmed by the MTE. The other side of the MTE will NOT contain the trimmed portion. Also messages that are shorter than the fixed length will be padded by the MTE so each message that is sent will ALWAYS be the same length. When shorter message are "decoded" on the other side the MTE takes off the extra padding when using strings and hands back the original shorter message, BUT if you use the raw interface the padding will be present as all zeros. Please see official MTE Documentation for more information.

```c++
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

```


<li>We need to be able to set the entropy, nonce, and personalization/identification values.</li>
These values should be treated like encryption keys and never exposed. For demonstration purposes in the tutorial we are setting these values in the code. In a production environment these values should be protected and not available to outside sources. For the entropy, we have to determine the size of the allowed entropy value based on the drbg we have selected. A code sample below is included to demonstrate how to get these values.

To set the entropy in the tutorial we are simply getting the minimum bytes required and creating a byte array of that length that contains all zeros. We want to set the default first to be blank.

```c++
const size_t entropyMinBytes = MteBase::getDrbgsEntropyMinBytes(g_encoder.getDrbg());
const uint64_t entropyMaxBytes = MteBase::getDrbgsEntropyMaxBytes(g_encoder.getDrbg());
```

To set the entropy in the tutorial we are simply getting the minimum bytes required and creating a byte array of that length that contains all zeros. 

```c++
// If this is a trial version of the MTE, entropy must be blank - set this as a global variable
uint8_t* g_entropy;

// Check how long entropy we need and change if we need more, set default entropy to 0's
size_t entropyBytes = MteBase::getDrbgsEntropyMinBytes(g_encoder.getDrbg());
g_entropy = new uint8_t[entropyBytes];
memset(g_entropy, '0', entropyBytes);
```

To set the nonce and the personalization/identifier string we are simply adding our default values as global variables to the top of the class.

```c++

// OPTIONAL!!! adding 1 to decoder nonce so return value changes -- same nonce can be used for encoder and decoder
// on client side values will be switched so they match up encoder to decoder and vice versa
int g_encoderNonce = 0;
int g_decoderNonce = 1;
std::string g_personal = "demo";
```

<li>To ensure the MTE library is licensed correctly, run the license check. To ensure the DRBG is set up correctly, run the DRBGS self test. The LicenseCompanyName, and LicenseKey below should be replaced with your company’s MTE license information provided by Eclypses. If a trial version of the MTE is being used any value can be passed into those fields and it will work.</li>

```c++
// Initialize MTE license. If a license code is not required (e.g., trial
// mode), this can be skipped. This demo attempts to load the license info
// from the environment if required.
if (!MteBase::initLicense("Eclypses Inc", "Eclypses123"))
{
	std::cerr << "License init error ("
		<< MteBase::getStatusName(mte_status_license_error)
		<< "): "
		<< MteBase::getStatusDescription(mte_status_license_error)
		<< std::endl;
	return mte_status_license_error;
}

```

<li>Create MTE Decoder Instances and MTE Encoder Instances in a small number of functions.</li>

Here is a sample function that creates the MTE Decoder.

```c++
mte_status createDecoder()
{
	// Check how long entropy we need, set default
	const size_t entropyMinBytes = g_decoder.getDrbgsEntropyMinBytes(g_decoder.getDrbg());
	
	g_decoderEntropy = new uint8_t[entropyMinBytes];
	// When entropy is set in the encoder, it is 'zeroed' out, therefore, we need to 'refill' it before setting in decoder
	memset(g_entropy, '0', entropyMinBytes);
	g_decoder.setEntropy(g_decoderEntropy, entropyMinBytes);

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

	return g_decoderStatus;
}
```
*(For further information on Decoder constructor review the DevelopersGuide)*

Here is a sample function that creates the MTE Encoder.

```c++
mte_status createEncoder()
{
	// Check how long entropy we need, set default
	const size_t entropyMinBytes = MteBase::getDrbgsEntropyMinBytes(g_encoder.getDrbg());
	g_encoderEntropy = new uint8_t[entropyMinBytes];
	memset(g_encoderEntropy, '0', entropyMinBytes);
	g_encoder.setEntropy(g_encoderEntropy, entropyMinBytes);

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

	return g_encoderStatus;
}
```
*(For further information on Encoder constructor review the DevelopersGuide)*

Instantiate the MTE Decoder and MTE Encoder by calling that function at the start of your main function:

```c++
mte_status encoderStatus = createEncoder;
mte_status decoderStatus = createDecoder();
```

<li>Finally, we need to add the MTE calls to encode and decode the messages that we are sending and receiving from the other side. (Ensure on the client side the Encoder is used to encode the outgoing text, then the Decoder is used to decode the incoming response.)</li>

<br>
Here is a sample of how to do this on the Client Side.

```c++
//
// encode text to send
size_t encodedBytes;
const void* encoded = g_encoder.encode(textToSend, encodedBytes, encoderStatus);

if (encoderStatus != mte_status_success)
{
	std::cout << "Error encoding: Status: "
		<< MteBase::getStatusName(encoderStatus)
		<< "/"
		<< MteBase::getStatusDescription(encoderStatus)
		<< std::endl;
	std::cout << "Socket client is closed due to encoding error, press ENTER to end this..." << std::endl;
	std::cin;
	return encoderStatus;
}

//
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

```
<br>
Here is a sample of how to do this on the Server Side.

```c++
// Decode incoming message and check for non-error response.
// When checking the status on decode use "statusIsError".
// Only checking if status is success can be misleading, there may be a
// warning returned that the user can ignore.
// See MTE Documentation for more details.
size_t decodedBytes;
const char* decodedText = static_cast<const char*>(g_decoder.decode(recvbuf, rcvLen, decodedBytes, g_decoderStatus));

if (g_decoder.statusIsError(g_decoderStatus))
{
	std::cout << "Error decoding: Status: "
		<< MteBase::getStatusName(g_decoderStatus)
		<< "/"
		<< MteBase::getStatusDescription(g_decoderStatus)
		<< std::endl;

	std::cout << "Socket server is closed due to decoding error, press ENTER to end this..." << std::endl;
	std::cin;
	return g_decoderStatus;
}

//
// Encode returning text and ensure successful
size_t encodedBytes;
const char* encodedReturn = static_cast<const char*>(g_encoder.encode(decodedText, encodedBytes, encoderStatus));
if (encoderStatus != mte_status_success)
{
	std::cout << "Error encoding: Status: "
		<< MteBase::getStatusName(encoderStatus)
		<< "/"
		<< MteBase::getStatusDescription(encoderStatus)
		<< std::endl;
	std::cout << "Socket server is closed due to encoding error, press ENTER to end this..." << std::endl;
	std::cin;
	return encoderStatus;
}

```
</ol>

***The Server side and the Client side of the MTE Sockets tutorial should now be ready for use on your device.***


<div style="page-break-after: always; break-after: page;"></div>

# Contact Eclypses

<img src="Eclypses.png" style="width:8in;"/>

<p align="center" style="font-weight: bold; font-size: 22pt;">For more information, please contact:</p>
<p align="center" style="font-weight: bold; font-size: 22pt;"><a href="mailto:info@eclypses.com">info@eclypses.com</a></p>
<p align="center" style="font-weight: bold; font-size: 22pt;"><a href="https://www.eclypses.com">www.eclypses.com</a></p>
<p align="center" style="font-weight: bold; font-size: 22pt;">+1.719.323.6680</p>

<p style="font-size: 8pt; margin-bottom: 0; margin: 300px 24px 30px 24px; " >
<b>All trademarks of Eclypses Inc.</b> may not be used without Eclypses Inc.'s prior written consent. No license for any use thereof has been granted without express written consent. Any unauthorized use thereof may violate copyright laws, trademark laws, privacy and publicity laws and communications regulations and statutes. The names, images and likeness of the Eclypses logo, along with all representations thereof, are valuable intellectual property assets of Eclypses, Inc. Accordingly, no party or parties, without the prior written consent of Eclypses, Inc., (which may be withheld in Eclypses' sole discretion), use or permit the use of any of the Eclypses trademarked names or logos of Eclypses, Inc. for any purpose other than as part of the address for the Premises, or use or permit the use of, for any purpose whatsoever, any image or rendering of, or any design based on, the exterior appearance or profile of the Eclypses trademarks and or logo(s).
</p>