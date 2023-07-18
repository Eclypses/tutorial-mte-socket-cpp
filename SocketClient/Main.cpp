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

//---------------------------------------------------
// MKE and Fixed length add-ons are NOT in all SDK
// MTE versions. If the name of the SDK includes
// "-MKE" then it will contain the MKE add-on. If the
// name of the SDK includes "-FLEN" then it contains
// the Fixed length add-on.
//---------------------------------------------------

#include <string>

#include "globals.h"
#include "ClientMteHelper.h"
#include "EcdhP256.h"
#include "SocketManager.h"
#include "MteBase.h"

// The help text for this program.
const char* help_text =
"This client program takes an input message, uses the MTE Encoder to "
"securely encode it, and sends the encoded message to the matching server "
"program to be decoded by the MTE Decoder. Then the message will be encoded "
"again with the MTE Encoder on the server, and sent back to the client "
"program, where it will be decoded by the MTE Decoder on the client program "
"and compared with the original message. The server program must be started "
"first and ready to accept the connection before the client program begins."
"\nUsage:"
"\n\t[-s]: The IP address of the running server program. Optional. Default is " DEFAULT_SERVER_IP "."
"\n\t[-p]: The port number the running server program will listen to. Optional. Default is " DEFAULT_PORT ".";

/// <summary>
/// Loops through all possible program flags and assigns values as needed.
/// </summary>
/// <param name="argc">The argument count.</param>
/// <param name="argv">The argument values.</param>
/// <returns>Returns true if program can continue.</returns>
static bool handleProgramFlags(int argc, char* argv[]);

/// <summary>
/// Gets the option flag (the character) from an argument.
/// </summary>
/// <param name="arg">The program argument.</param>
/// <returns>The option character.</returns>
static const char getOptionFlag(char* arg);

/// <summary>
/// Gets the option value from an argument.
/// </summary>
/// <param name="arg">The program argument.</param>
/// <returns>The value for that option.</returns>
static const char* getOptionValue(char* arg);

/// <summary>
/// Displays the program language and type. Displays the version and the current type of MTE being used.
/// </summary>*
static void displayProgramInfo();

/// <summary>
/// Attempts to run a cross diagnostic test.
/// </summary>
/// <returns>Returns true if client and server Encoder/Decoders are paired properly.</returns>
static bool runDiagnosticTest();

/// <summary>
/// Main program loop. Prompts for user input. It is then encoded and sent to the server.
/// It will then be decoded, encoded again, and sent back.
/// That will then be decoded here and compared to the original input.
/// </summary>
static void handleUserInput();

/// <summary>
/// Encodes the message with the MTE and then sends it.
/// </summary>
/// <param name="message">The message to be encoded and sent.</param>
/// <returns>True if encoded and sent successfully, false otherwise.</returns>
static bool encodeAndSendMessage(byte_array message);

/// <summary>
/// Receives the incoming message and then decode it with the MTE.
/// </summary>
/// <param name="message">The decoded message.</param>
/// <returns>True if received and decoded successfully, false otherwise.</returns>
static bool receiveAndDecodeMessage(byte_array& message);

/// <summary>
/// Finishes the program. Closes the socket connection.
/// </summary>
static void closeProgram();

static const char* g_ip_address = DEFAULT_SERVER_IP;
static const char* g_port = DEFAULT_PORT;

int main(int argc, char* argv[])
{
  // Evaluate any relevant program arguments.
 // Exit program if the help text is shown.
  if (handleProgramFlags(argc, argv) == false)
  {
    return 1;
  }

  // Evaluate any relevant program arguments.
  // Exit program if the help text is shown.
  if (handleProgramFlags(argc, argv) == false)
  {
    return 1;
  }

  // Display program information.
  displayProgramInfo();

   // Initialize socket
#if defined _WIN32
  //Sleep(1000);
#else
  sleep(1);
#endif
 
  int socketCreation = SocketManager::createSocket();
  if (socketCreation == 0)
  {
    printf("Unable to create socket.");
    return socketCreation;
  }

  int socketConnection = SocketManager::connectSocket(g_ip_address, (uint16_t)atoi(g_port));
  if (socketConnection == 0)
  {
    printf("Unable to connect to socket.");
    return socketConnection;
  }

  printf("Client connected to Server.\n");

  // Init the MTE.
  if (!ClientMteHelper::initMte())
  {
    printf("There was a problem initializing the MTE.");
    return 1;
  }

  // Create the Encoder.
  if (!ClientMteHelper::createEncoder())
  {
    printf("There was a problem creating the Encoder.");
    return 1;
  }

  // Create the Decoder.
  if (!ClientMteHelper::createDecoder())
  {
    printf("There was a problem creating the Decoder.");
    return 1;
  }

  // Run the diagnostic test.
  if (!runDiagnosticTest())
  {
    printf("There was a problem running the diagnostic test.");
    return 1;
  }

  // Handle user input.
  handleUserInput();

  // End the program.
  closeProgram();

  return 0;
}

static bool handleProgramFlags(int argc, char* argv[])
{
  // Loop through each argument to check if it starts with a flag.
  for (int i = 0; i < argc; i++)
  {
    // Get the option.
    const char flag = getOptionFlag(argv[i]);

    // Determine if there is a value adjacent to the flag (e.g., -ftext.txt
    // where -f is the flag, there is no space, and text.txt is the value).
    const char* val = getOptionValue(argv[i]);

    // If the adjacent value is not present, determine if the next argument
    // is a value (e.g., -f text.txt where -f is the flag, there is a space,
    // and text.txt is the value).
    if (strlen(val) == 0)
    {
      val = argv[i + 1];
    }

    // If there is the help flag, display the help text and then exit.
    if (flag == 'h')
    {
      printf("%s\n", help_text);

      // Immediately exit program.
      return false;
    }

    // Assign server ip address.
    if (flag == 's' && strlen(val) > 0)
    {
      g_ip_address = (char*)(val);
    }

    // Assign port number.
    if (flag == 'p' && strlen(val) > 0)
    {
      g_port = (char*)(val);
    }
  }
  return true;
}

static const char getOptionFlag(char* arg)
{
  char option = 0;
  // Determine if first character is hyphen.
  if (arg[0] == '-')
  {
    // Set option to the character after.
    option = arg[1];
  }
  return option;
}

static const char* getOptionValue(char* arg)
{
  // Get substring starting after hyphen and option character.
  char* optionVal = &arg[2];
  return optionVal;
}

static void displayProgramInfo()
{
  // Display the language and application.
  printf("Starting C++ Socket Client.\n");

  // Display version of MTE and type.
  const char* mteVersion = MteBase::getVersion();
#if defined USE_MTE_CORE
  const char* mteType = "Core";
#endif
#if defined USE_MKE_ADDON
  const char* mteType = "MKE";
#endif
#if defined USE_FLEN_ADDON
  const char* mteType = "FLEN";
#endif

  printf("Using MTE Version: %s-%s\n", mteVersion, mteType);
}

static bool runDiagnosticTest()
{
  // Create ping message.
  byte_array message;
  message.size = 4;
  message.data = new uint8_t[message.size];
  message.data = (unsigned char*)("ping");

  // Encode and send message.
  if (encodeAndSendMessage(message) == false)
  {
    return false;
  }

  // Receive and decode the message.
  byte_array decoded{};
  if (receiveAndDecodeMessage(decoded) == false)
  {
    return false;
  }

  // Check that it successfully decoded as "ack".
  char* decodedMessage = (char*)decoded.data;
  if (strncmp("ack", decodedMessage, decoded.size) == 0)
  {
    printf("Client Decoder decoded the message from the server Encoder successfully.\n");
  }
  else
  {
    fprintf(stderr, "Client Decoder DID NOT decode the message from the server Encoder successfully.\n");
    return false;
  }

  return true;
}

static void handleUserInput()
{
  while (true)
  {
    // Prompt user for input to send to other side.
    char input[MAX_INPUT_BYTES];

    printf("Please enter text up to %i bytes to send: (To end please type 'quit')\n", MAX_INPUT_BYTES);
    fflush(stdout);
    (void)fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';
    if (strcasecmp(input, "quit") == 0)
    {
      break;
    }

    byte_array message;
    message.size = strlen(input);
    message.data = new uint8_t[message.size];
    memcpy((void*)message.data, input, message.size);

    // Encode and send the input.
    if (encodeAndSendMessage(message) == false)
    {
      break;
    }

    // Receive and decode the returned data.
    byte_array decoded{};
    if (receiveAndDecodeMessage(decoded) == false)
    {
      break;
    }    

    // Compare the decoded message to the original.
    char* decodedMessage = (char*)decoded.data;
    if (strncmp(input, decodedMessage, decoded.size) == 0)
    {
      printf("The original input and decoded return match.\n");
    }
    else
    {
      fprintf(stderr, "The original input and decoded return DO NOT match.\n");
      break;
    }
  }

  return;
}

static bool encodeAndSendMessage(byte_array message)
{
  // Encode the message.
  byte_array encoded{};

  if (ClientMteHelper::encodeMessage(message, encoded) == false)
  {
    return false;
  }

  // Send the encoded message.
  const size_t res = SocketManager::sendMessage('m', encoded);
  if (res == 0)
  {
    return false;
  }

  // Free the encoded message.
  delete encoded.data;

  return true;
}

static bool receiveAndDecodeMessage(byte_array& message)
{
  // Wait for return message.
  struct recv_msg msg_struct = SocketManager::receiveMessage();
  if (msg_struct.success == false || msg_struct.message.size == 0 || msg_struct.header != 'm')
  {
    fprintf(stderr, "Error received from server.\n");
    return false;
  }

  // Decode the message.
  byte_array encoded = MteSetupInfo::createByteArray(msg_struct.message);
  if (ClientMteHelper::decodeMessage(encoded, message) == false)
  {
    return false;
  }

 delete encoded.data;

  return true;
}

static void closeProgram()
{
  // Finish MTE.
  ClientMteHelper::finishMte();

  // Close the socket.
  SocketManager::closeSocket();

  printf("Program stopped.\n");
}