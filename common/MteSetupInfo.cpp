#include "MteSetupInfo.h"

MteSetupInfo::MteSetupInfo()
{
  // Set public key size.
  publicKey = createByteArray(SZ_ECDH_P256_PUBLIC_KEY);

  // Create the private and public keys.
  const int res = ecdhManager.createKeyPair(publicKey);
  if (res < 0)
  {
    throw res;
  }
}

MteSetupInfo::~MteSetupInfo()
{
  // Zeroize public key.
  ecdh_p256_zeroize(publicKey.data, publicKey.size);
}

byte_array MteSetupInfo::getPublicKey() const
{
  // Create temp byte array.
  const byte_array temp = createByteArray(publicKey);
  return temp;
}

byte_array MteSetupInfo::getSharedSecret()
{
  if (peerKey.size == 0)
  {
    throw ECDH_P256_MEMORY_FAIL;
  }

  // Create temp byte array.
  byte_array temp = createByteArray(SZ_ECDH_P256_SECRET_DATA);

  // Create shared secret.
  const int res = ecdhManager.getSharedSecret(peerKey, temp);
  if (res < 0)
  {
    throw ECDH_P256_MEMORY_FAIL;
  }

  return temp;
}

void MteSetupInfo::setPersonalization(uint8_t* data, size_t size)
{
  // Check if personalization already set.
  if (personalization.data != nullptr)
  {
    // Delete data.
    delete personalization.data;
  }

  // Create personalization again.
  personalization = createByteArray(data, size);
}

byte_array MteSetupInfo::getPersonalization() const
{
  if (personalization.size != 0)
  {
    // Create temp byte array and use personal byte array.
    return createByteArray(personalization);
  }
  else
  {
    return createByteArray(nullptr, 0);
  }
}

void MteSetupInfo::setNonce(uint8_t* data, size_t size)
{
  // Check if nonce already set.
  if (nonce.data != nullptr)
  {
    // Delete data.
    delete nonce.data;
  }

  // Create nonce again.
  nonce = createByteArray(data, size);
}

byte_array MteSetupInfo::getNonce() const
{
  if (nonce.size != 0)
  {
    // Create temp byte array and use nonce byte array.
    return createByteArray(nonce);
  }
  else
  {
    return createByteArray(nullptr, 0);
  }
}

void MteSetupInfo::setPeerPublicKey(uint8_t* data, size_t size)
{
  // Check if peer already set.
  if (peerKey.data != nullptr)
  {
    // Delete data.
    delete peerKey.data;
  }

  // Create peer again.
  peerKey = createByteArray(data, size);
}

byte_array MteSetupInfo::getPeerPublicKey() const
{
  if (peerKey.size != 0)
  {
    // Create temp byte array and use peer byte array.
    return createByteArray(peerKey);
  }
  else
  {
    return createByteArray(nullptr, 0);
  }
}

byte_array MteSetupInfo::createByteArray(size_t size)
{
  // Create temp byte array with size based on parameter.
  byte_array temp;
  temp.size = size;

  if (size == 0)
  {
    // Set data to null.
    temp.data = nullptr;
  }
  else
  {
    // Create new uint8_t byte array based on size.
    temp.data = new uint8_t[size];
  }
  return temp;
}

byte_array MteSetupInfo::createByteArray(const byte_array source)
{
  // If size is zero or data is null, return empty byte array.
  if (source.size == 0 || source.data == nullptr)
  {
    return createByteArray(0);
  }

  // Copy the data from the source into the destination temp.
  byte_array temp;
  temp.size = source.size;
  temp.data = new uint8_t[source.size];
  memcpy((void*)temp.data, source.data, temp.size);

  return temp;
}

byte_array MteSetupInfo::createByteArray(uint8_t* source, size_t size)
{
  // If size is zero or data is null, return empty byte array.
  if (size == 0 || source == nullptr)
  {
    return createByteArray(0);
  }

  // Copy the data from the source into the destination temp.
  byte_array temp;
  temp.size = size;
  temp.data = source;

  return temp;
}