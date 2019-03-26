// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2014, Linaro Limited
 */

#include <assert.h>
#include <crypto/crypto.h>
#include <kernel/panic.h>
#include <stdlib.h>
#include <string_ext.h>
#include <string.h>
#include <tee_api_types.h>
#include <tee_api_defines_extensions.h>
#include <tee/tee_cryp_utl.h>
#include <tomcrypt.h>
#include "tomcrypt_mp.h"
#include <trace.h>
#include <utee_defines.h>
#include <util.h>

#if defined(CFG_WITH_VFP)
#include <tomcrypt_arm_neon.h>
#include <kernel/thread.h>
#endif

/* Random generator */
static int prng_crypto_start(union Prng_state *prng __unused)
{
	return CRYPT_OK;
}

static int prng_crypto_add_entropy(const unsigned char *in __unused,
				   unsigned long inlen __unused,
				   union Prng_state *prng __unused)
{
	/* No entropy is required */
	return CRYPT_OK;
}

static int prng_crypto_ready(union Prng_state *prng __unused)
{
	return CRYPT_OK;
}

static unsigned long prng_crypto_read(unsigned char *out, unsigned long outlen,
				      union Prng_state *prng __unused)
{
	if (crypto_rng_read(out, outlen))
		return 0;

	return outlen;
}

static int prng_crypto_done(union Prng_state *prng __unused)
{
	return CRYPT_OK;
}

static int prng_crypto_export(unsigned char *out __unused,
			      unsigned long *outlen __unused,
			      union Prng_state *prng __unused)
{
	return CRYPT_OK;
}

static int prng_crypto_import(const unsigned char *in  __unused,
			      unsigned long inlen __unused,
			      union Prng_state *prng __unused)
{
	return CRYPT_OK;
}

static int prng_crypto_test(void)
{
	return CRYPT_OK;
}

static const struct ltc_prng_descriptor prng_crypto_desc = {
	.name = "prng_crypto",
	.export_size = 64,
	.start = &prng_crypto_start,
	.add_entropy = &prng_crypto_add_entropy,
	.ready = &prng_crypto_ready,
	.read = &prng_crypto_read,
	.done = &prng_crypto_done,
	.pexport = &prng_crypto_export,
	.pimport = &prng_crypto_import,
	.test = &prng_crypto_test,
};

/*
 * tee_ltc_reg_algs(): Registers
 *	- algorithms
 *	- hash
 *	- prng (pseudo random generator)
 */

static void tee_ltc_reg_algs(void)
{
#if defined(CFG_CRYPTO_AES)
	register_cipher(&aes_desc);
#endif
#if defined(CFG_CRYPTO_DES)
	register_cipher(&des_desc);
	register_cipher(&des3_desc);
#endif
#if defined(CFG_CRYPTO_MD5)
	register_hash(&md5_desc);
#endif
#if defined(CFG_CRYPTO_SHA1)
	register_hash(&sha1_desc);
#endif
#if defined(CFG_CRYPTO_SHA224)
	register_hash(&sha224_desc);
#endif
#if defined(CFG_CRYPTO_SHA256)
	register_hash(&sha256_desc);
#endif
#if defined(CFG_CRYPTO_SHA384)
	register_hash(&sha384_desc);
#endif
#if defined(CFG_CRYPTO_SHA512)
	register_hash(&sha512_desc);
#endif
	register_prng(&prng_crypto_desc);
}

TEE_Result crypto_init(void)
{
	init_mp_tomcrypt();
	tee_ltc_reg_algs();

	return TEE_SUCCESS;
}

#if defined(CFG_WITH_VFP)
void tomcrypt_arm_neon_enable(struct tomcrypt_arm_neon_state *state)
{
	state->state = thread_kernel_enable_vfp();
}

void tomcrypt_arm_neon_disable(struct tomcrypt_arm_neon_state *state)
{
	thread_kernel_disable_vfp(state->state);
}
#endif