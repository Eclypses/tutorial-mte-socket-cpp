#ifndef MteSetupInfo_h
#define MteSetupInfo_h
#include "EcdhP256.h"
#include <cstring>

class MteSetupInfo
{
private:
  EcdhP256 ecdhManager;
  byte_array personalization{};
  byte_array nonce{};
  byte_array publicKey{};
  byte_array peerKey{};

public:
  /// <summary>
  /// Class constructor. Creates the ECDH keys.
  /// </summary>
  MteSetupInfo();

  /// <summary>
  /// Class destructor.
  /// </summary>
  virtual ~MteSetupInfo();

  /// <summary>
  /// Retrieves the public key.
  /// </summary>
  /// <returns>The public key.</returns>
  byte_array getPublicKey() const;

  /// <summary>
  /// Creates and retrieves the shared secret. Must have set the peer public key
  /// with setPeerPublicKey().
  /// </summary>
  /// <returns>The shared secret.</returns>
  byte_array getSharedSecret();

  /// <summary>
  /// Sets the personalization.
  /// </summary>
  /// <param name="data">The personalization data.</param>
  /// <param name="size">The size in bytes.</param>
  void setPersonalization(uint8_t* data, size_t size);

  /// <summary>
  /// Gets the personalization.
  /// </summary>
  /// <returns>The personalization byte array.</returns>
  byte_array getPersonalization() const;

  /// <summary>
  /// Sets the nonce.
  /// </summary>
  /// <param name="data">The nonce data.</param>
  /// <param name="size">The size in bytes.</param>
  void setNonce(uint8_t* data, size_t size);

  /// <summary>
  /// Gets the nonce.
  /// </summary>
  /// <returns>The nonce byte array.</returns>
  byte_array getNonce() const;

  /// <summary>
  /// Sets this entity's peer public key.
  /// </summary>
  /// <param name="data">The peer public key data.</param>
  /// <param name="size">The size in bytes.</param>
  void setPeerPublicKey(uint8_t* data, size_t size);

  /// <summary>
  /// Gets the peer public key for this entity.
  /// </summary>
  /// <returns>The peer public key byte array.</returns>
  byte_array getPeerPublicKey() const;

  /// <summary>
  /// Creates a byte array using the given size in bytes. The data will be set to an array
  /// based on the size, or null if size if zero.
  /// </summary>
  /// <param name="size">The size of the data in bytes.</param>
  /// <returns>An empty byte array with the given size (or null if size of zero).</returns>
  static byte_array createByteArray(size_t size);

  /// <summary>
  /// Creates a copy of the source byte array.
  /// </summary>
  /// <param name="source">The byte array to be copied.</param>
  /// <returns>The copied byte array.</returns>
  static byte_array createByteArray(const byte_array source);

  /// <summary>
  /// Creates a byte array with the given data and size.
  /// </summary>
  /// <param name="source">The pointer to the source data.</param>
  /// <param name="size">The size of the data.</param>
  /// <returns>A byte array pointing to the source data.</returns>
  static byte_array createByteArray(uint8_t* source, size_t size);

};
#endif