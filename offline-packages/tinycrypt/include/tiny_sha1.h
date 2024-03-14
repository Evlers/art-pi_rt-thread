/**
 * \file sha1.h
 *
 *  Based on TropicSSL: Copyright (C) 2017 Shanghai Real-Thread Technology Co., Ltd
 *
 *  Based on XySSL: Copyright (C) 2006-2008  Christophe Devine
 *
 *  Copyright (C) 2009  Paul Bakker <polarssl_maintainer at polarssl dot org>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of PolarSSL or XySSL nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef TINY_CRYPT_SHA1_H__
#define TINY_CRYPT_SHA1_H__

/**
 * \brief          SHA-1 context structure
 */
typedef struct {
    uint32_t total[2]; /*!< number of bytes processed  */
    uint32_t state[5]; /*!< intermediate digest state  */
    uint8_t buffer[64];   /*!< data block being processed */

    uint8_t ipad[64]; /*!< HMAC: inner padding        */
    uint8_t opad[64]; /*!< HMAC: outer padding        */
} tiny_sha1_context;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * \brief          SHA-1 context setup
     *
     * \param ctx      context to be initialized
     */
    void tiny_sha1_starts(tiny_sha1_context * ctx);

    /**
     * \brief          SHA-1 process buffer
     *
     * \param ctx      SHA-1 context
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     */
    void tiny_sha1_update(tiny_sha1_context * ctx, uint8_t *input, int ilen);

    /**
     * \brief          SHA-1 final digest
     *
     * \param ctx      SHA-1 context
     * \param output   SHA-1 checksum result
     */
    void tiny_sha1_finish(tiny_sha1_context * ctx, uint8_t output[20]);

    /**
     * \brief          Output = SHA-1( input buffer )
     *
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     * \param output   SHA-1 checksum result
     */
    void tiny_sha1(uint8_t *input, int ilen, uint8_t output[20]);

    /**
     * \brief          SHA-1 HMAC context setup
     *
     * \param ctx      HMAC context to be initialized
     * \param key      HMAC secret key
     * \param keylen   length of the HMAC key
     */
    void tiny_sha1_hmac_starts(tiny_sha1_context * ctx, uint8_t *key,
                  int keylen);

    /**
     * \brief          SHA-1 HMAC process buffer
     *
     * \param ctx      HMAC context
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     */
    void tiny_sha1_hmac_update(tiny_sha1_context * ctx, uint8_t *input,
                  int ilen);

    /**
     * \brief          SHA-1 HMAC final digest
     *
     * \param ctx      HMAC context
     * \param output   SHA-1 HMAC checksum result
     */
    void tiny_sha1_hmac_finish(tiny_sha1_context * ctx, uint8_t output[20]);

    /**
     * \brief          Output = HMAC-SHA-1( hmac key, input buffer )
     *
     * \param key      HMAC secret key
     * \param keylen   length of the HMAC key
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     * \param output   HMAC-SHA-1 result
     */
    void tiny_sha1_hmac(uint8_t *key, int keylen,
               uint8_t *input, int ilen,
               uint8_t output[20]);

#ifdef __cplusplus
}
#endif
#endif              /* sha1.h */
