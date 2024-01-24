/*
  This file is part of the UNOR4USBBridge project.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_UNOWIFIR4_SSE_H_
#define ARDUINO_UNOWIFIR4_SSE_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Preferences.h>

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define SSE_DEBUG_ENABLED 0

#define SSE_SHA256_LENGTH             32
#define SSE_EC256_R_LENGTH            32
#define SSE_EC256_S_LENGTH            SSE_EC256_R_LENGTH
#define SSE_EC256_SIGNATURE_LENGTH    (SSE_EC256_R_LENGTH + SSE_EC256_S_LENGTH)
#define SSE_EC256_X_LENGTH            32
#define SSE_EC256_Y_LENGTH            SSE_EC256_X_LENGTH
#define SSE_EC256_PUB_KEY_LENGTH      (SSE_EC256_X_LENGTH + SSE_EC256_Y_LENGTH)

/* https://github.com/Mbed-TLS/mbedtls/blob/8bdd8cdc4fc99588f6e3ed47d3dac7b2ddbdb6af/library/pkwrite.h#L82 */
#define SSE_EC256_DER_PUB_KEY_LENGTH  (30 + 2 * SSE_EC256_X_LENGTH)
#define SSE_EC256_DER_PRI_KEY_LENGTH  (29 + 3 * SSE_EC256_X_LENGTH)

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class Arduino_UNOWIFIR4_SSE
{

public:

  /** generateECKeyPair
   *
   * Create a new ECCurve_NIST_P256 keypair. Only public key will be available
   * inside KeyBuf with DER format.
   *
   * | P256 Header (26 bytes)| 0x04 (1 byte)| Public key X Y values (64 bytes) |
   *
   * @param[out] derKey output buffer containing the public key in DER format
   * @param[in] maxLen output buffer max size in bytes
   *
   * @return size of der buffer on Success a negative number on Failure (mbedtls error code)
   */
  static int generateECKeyPair(unsigned char* derKey, int maxLen);

  /** exportECKeyXY
   *
   * Exports public key XY values from a DER formatted ECCurve_NIST_P256 key
   *
   * @param[in] derKey input buffer containing the public key in DER format
   * @param[in] derLen key size in bytes
   * @param[out] pubXY output buffer containing public key XY value. Should be at least 64 bytes
   *
   * @return size of XY buffer on Success a negative number on Failure (mbedtls error code)
   */
  static int exportECKeyXY(const unsigned char* derKey, int derLen, unsigned char* pubXY);

  /** importECKeyXY
   *
   * Imports public key XY values from buffer and writes a DER formatted ECCurve_NIST_P256 public key
   *
   * @param[in] pubXY output buffer containing public key XY value.
   * @param[out] derKey output buffer containing the public key in DER format
   * @param[out] maxLen output buffer max size in bytes
   *
   * @return size of der buffer on Success a negative number on Failure (mbedtls error code)
   */
  static int importECKeyXY(const unsigned char* pubXY, unsigned char* derKey, int maxLen);

  /** sha256
   *
   * One-shot sha256
   *
   * @param[in] message Input data buffer
   * @param[in] len Input data length
   * @param[out] sha256 Output buffer should be at least 32 bytes long
   *
   * @return size of sha256 buffer on Success a negative number on Failure (mbedtls error code)
   */
  static int sha256(const unsigned char* message, int len, unsigned char* sha256);

  /** sign
   *
   * Computes ECDSA signature using private derKey and input sha256
   *
   * @param[in] derKey input buffer containing the private key in DER format
   * @param[in] derLen key size in bytes
   * @param[in] sha256 input sha256
   * @param[out] sigRS output buffer containing signature RS value. Should be at least 64 bytes
   *
   * @return size of RS buffer on Success a negative number on Failure (mbedtls error code)
   */
  static int sign(const unsigned char* derKey, int derLen, const unsigned char* sha256, unsigned char* sigRS);

  /** verify
   *
   * Verify ECDSA signature using public key
   *
   * @param[in] derKey input buffer containing the public key in DER format
   * @param[in] derLen key size in bytes
   * @param[in] sha256 input sha256
   * @param[in] sigRS input buffer containing signature RS value
   *
   * @return 0 on Success a negative number on Failure (mbedtls error code)
   */
  static int verify(const unsigned char* derKey, int derLen, const unsigned char* sha256, const unsigned char* sigRS);

};

extern Preferences sse;

#endif /* Arduino_UNOWIFIR4_SSE */
