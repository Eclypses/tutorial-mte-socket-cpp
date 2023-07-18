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

#include "globals.h"
#include "ServerMteHelper.h"

// The help text for this program.
const char* help_text = "This server program receives an encoded message from "
"the client and uses the MTE Decoder to decode it. That message is then "
"securely re-encoded with the MTE Encoder and sent back to the client program, "
"where it will be decoded by the MTE Decoder and sends the encoded message to "
"the matching client program to be decoded by the MTE Decoder. Then the "
"message will be encoded again with the MTE Encoder on the server, and sent "
"back to the client program, where it will be decoded by the MTE Encoder on "
"the client program and compared with the original message. The server program "
"must be started first and ready to accept the connection before the client "
"program begins."
"\nUsage:"
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
/// </summary>
static void displayProgramInfo();

/// <summary>
/// Attempts to run a cross diagnostic test.
/// </summary>
/// <returns>Returns true if client and server Encoder/Decoders are paired properly.</returns>
static bool runDiagnosticTest();

/// <summary>
/// Receives the encoded input from the user via the client. Decodes the message.
/// Then that is re-encoded and sent back to the client to be decoded. The original
/// input and decoded output are compared.
/// </summary>
static void handleUserOutput();

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

static const char *g_port = DEFAULT_PORT;

int main(int argc, char* argv[])
{
  // This tutorial uses Sockets for communication.
  // It should be noted that the MTE can be used with any type of communication. (SOCKETS are not required!).  

  // Evaluate any relevant program arguments.
  // Exit program if the help text is shown.
  if (handleProgramFlags(argc, argv) == false)
  {
    return 1;
  }

  // Display program information.
  displayProgramInfo();

  int socket_creation = SocketManager::createSocket();
  if (socket_creation == 0)
  {
    printf("Unable to create socket.");
    return socket_creation;
  }

  int socket_binding = SocketManager::bindSocket(atoi(g_port));
  if (socket_binding == 0)
  {
    printf("Unable to bind to socket.");
    return socket_binding;
  }

  printf("Listening for new client connection...\n");

  int socket_listening = SocketManager::listenSocket();
  if (socket_listening == 0)
  {
    printf("Unable to listen to socket.");
    return socket_listening;
  }

  int socket_accepting = SocketManager::acceptSocket(const_cast<char*>(g_port));
  if (socket_accepting == 0)
  {
    printf("Unable to accept the socket.");
    return socket_accepting;
  }

  printf("Connected with Client.\n");

  // Init the MTE.
  if (!ServerMteHelper::initMte())
  {
    printf("There was a problem initializing the MTE.");
    return 1;
  }

  // Create the Decoder.
  if (!ServerMteHelper::createDecoder())
  {
    printf("There was a problem creating the Decoder.");
    return 1;
  }

  // Create the Encoder.
  if (!ServerMteHelper::createEncoder())
  {
    printf("There was a problem creating the Encoder.");
    return 1;
  }

  // Run the diagnostic test.
  if (!runDiagnosticTest())
  {
    printf("There was a problem running the diagnostic test.");
    return 1;
  }

  // Handle user output coming from the server.
  handleUserOutput();

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
  char* option_val = &arg[2];
  return option_val;
}

static void displayProgramInfo()
{
  // Display the language and application.
  printf("Starting C++ Socket Server.\n");

  // Display version of MTE and type.
  const char* mte_version = mte_base_version();
#if defined USE_MTE_CORE
  const char* mte_type = "Core";
#endif
#if defined USE_MKE_ADDON
  const char* mte_type = "MKE";
#endif
#if defined USE_FLEN_ADDON
  const char* mte_type = "FLEN";
#endif

  printf("Using MTE Version: %s-%s\n", mte_version, mte_type);
}

static bool runDiagnosticTest()
{
  // Receive and decode the message.
  byte_array decoded{};
  if (receiveAndDecodeMessage(decoded) == false)
  {
    return false;
  }

  char* decodedMessage = (char*)decoded.data;
  // Check that it successfully decoded as "ping".
  if (strncmp("ping", decodedMessage, decoded.size) == 0)
  {
    printf("Server Decoder decoded the message from the client Encoder successfully.\n");
  }
  else
  {
    fprintf(stderr, "Server Decoder DID NOT decode the message from the client Encoder successfully.\n");
    return false;
  }

  // Create "ack" message.
  byte_array message;
  message.size = 3;
  message.data = new uint8_t[message.size];
  message.data = (unsigned char *)("ack");

  // Encode and send message.
  if (encodeAndSendMessage(message) == false)
  {
    return false;
  }

  return true;
}

static void handleUserOutput()
{
  while (true)
  {
    printf("Listening for messages from client...\n");

    // Receive and decode the message from the client.
    byte_array decoded{};
    if (receiveAndDecodeMessage(decoded) == false)
    {
      break;
    }

    // Encode and send the input.
    if (encodeAndSendMessage(decoded) == false)
    {
      break;
    }

    // Free the decoded message.
    delete decoded.data;
  }
  return;
}

static bool encodeAndSendMessage(byte_array message)
{
  // Encode the message.
  byte_array encoded{};

  if (ServerMteHelper::encodeMessage(message, encoded) == false)
  {
    return false;
  }

  // Send the encoded message.
  const size_t res = SocketManager::sendMessage('m', encoded);
  if (res == 0)
  {
    return false;
  }

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
  if (ServerMteHelper::decodeMessage(encoded, message) == false)
  {
    return false;
  }

  return true;
}

static void closeProgram()
{
  // Finish MTE.
  ServerMteHelper::finishMte();

  // Close the socket.
  SocketManager::closeSocket();

  printf("Program stopped.\n");
}