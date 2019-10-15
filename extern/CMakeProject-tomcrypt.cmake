if(WITH_SYSTEM_TOMCRYPT)
  find_library(TOMCRYPT_LIBRARY tomcrypt)
else()
  set(TOMDIR "${SM_SRC_DIR}/libtomcrypt")

  list(APPEND TOMCRYPT_CIPHERS
    "${TOMDIR}/src/ciphers/anubis.c"
    "${TOMDIR}/src/ciphers/blowfish.c"
    "${TOMDIR}/src/ciphers/camellia.c"
    "${TOMDIR}/src/ciphers/cast5.c"
    "${TOMDIR}/src/ciphers/des.c"
    "${TOMDIR}/src/ciphers/kasumi.c"
    "${TOMDIR}/src/ciphers/khazad.c"
    "${TOMDIR}/src/ciphers/kseed.c"
    "${TOMDIR}/src/ciphers/multi2.c"
    "${TOMDIR}/src/ciphers/noekeon.c"
    "${TOMDIR}/src/ciphers/rc2.c"
    "${TOMDIR}/src/ciphers/rc5.c"
    "${TOMDIR}/src/ciphers/rc6.c"
    "${TOMDIR}/src/ciphers/skipjack.c"
    "${TOMDIR}/src/ciphers/xtea.c"
  )

  source_group("ciphers" FILES ${TOMCRYPT_CIPHERS})

  list(APPEND TOMCRYPT_CIPHERS_AES
    "${TOMDIR}/src/ciphers/aes/aes.c"
  )

  source_group("ciphers\\\\aes" FILES ${TOMCRYPT_CIPHERS_AES})

  list(APPEND TOMCRYPT_CIPHERS_SAFER
    "${TOMDIR}/src/ciphers/safer/safer.c"
    "${TOMDIR}/src/ciphers/safer/saferp.c"
  )

  source_group("ciphers\\\\safer" FILES ${TOMCRYPT_CIPHERS_SAFER})

  list(APPEND TOMCRYPT_CIPHERS_TWOFISH
    "${TOMDIR}/src/ciphers/twofish/twofish.c"
  )

  source_group("ciphers\\\\twofish" FILES ${TOMCRYPT_CIPHERS_TWOFISH})

  list(APPEND TOMCRYPT_ENCAUTH_CCM
    "${TOMDIR}/src/encauth/ccm/ccm_add_aad.c"
    "${TOMDIR}/src/encauth/ccm/ccm_add_nonce.c"
    "${TOMDIR}/src/encauth/ccm/ccm_done.c"
    "${TOMDIR}/src/encauth/ccm/ccm_init.c"
    "${TOMDIR}/src/encauth/ccm/ccm_memory.c"
    "${TOMDIR}/src/encauth/ccm/ccm_process.c"
    "${TOMDIR}/src/encauth/ccm/ccm_reset.c"
    "${TOMDIR}/src/encauth/ccm/ccm_test.c"
  )

  source_group("encauth\\\\ccm" FILES ${TOMCRYPT_ENCAUTH_CCM})

  list(APPEND TOMCRYPT_ENCAUTH_CHACHAPOLY
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_add_aad.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_decrypt.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_done.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_encrypt.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_init.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_memory.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_setiv.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_setiv_rfc7905.c"
    "${TOMDIR}/src/encauth/chachapoly/chacha20poly1305_test.c"
  )

  source_group("encauth\\\\chachapoly" FILES ${TOMCRYPT_ENCAUTH_CHACHAPOLY})

  list(APPEND TOMCRYPT_ENCAUTH_EAX
    "${TOMDIR}/src/encauth/eax/eax_addheader.c"
    "${TOMDIR}/src/encauth/eax/eax_decrypt.c"
    "${TOMDIR}/src/encauth/eax/eax_decrypt_verify_memory.c"
    "${TOMDIR}/src/encauth/eax/eax_done.c"
    "${TOMDIR}/src/encauth/eax/eax_encrypt.c"
    "${TOMDIR}/src/encauth/eax/eax_encrypt_authenticate_memory.c"
    "${TOMDIR}/src/encauth/eax/eax_init.c"
    "${TOMDIR}/src/encauth/eax/eax_test.c"
  )

  source_group("encauth\\\\eax" FILES ${TOMCRYPT_ENCAUTH_EAX})

  list(APPEND TOMCRYPT_ENCAUTH_GCM
    "${TOMDIR}/src/encauth/gcm/gcm_add_aad.c"
    "${TOMDIR}/src/encauth/gcm/gcm_add_iv.c"
    "${TOMDIR}/src/encauth/gcm/gcm_done.c"
    "${TOMDIR}/src/encauth/gcm/gcm_gf_mult.c"
    "${TOMDIR}/src/encauth/gcm/gcm_init.c"
    "${TOMDIR}/src/encauth/gcm/gcm_memory.c"
    "${TOMDIR}/src/encauth/gcm/gcm_mult_h.c"
    "${TOMDIR}/src/encauth/gcm/gcm_process.c"
    "${TOMDIR}/src/encauth/gcm/gcm_reset.c"
    "${TOMDIR}/src/encauth/gcm/gcm_test.c"
  )

  source_group("encauth\\\\gcm" FILES ${TOMCRYPT_ENCAUTH_GCM})

  list(APPEND TOMCRYPT_ENCAUTH_OCB
    "${TOMDIR}/src/encauth/ocb/ocb_decrypt.c"
    "${TOMDIR}/src/encauth/ocb/ocb_decrypt_verify_memory.c"
    "${TOMDIR}/src/encauth/ocb/ocb_done_decrypt.c"
    "${TOMDIR}/src/encauth/ocb/ocb_done_encrypt.c"
    "${TOMDIR}/src/encauth/ocb/ocb_encrypt.c"
    "${TOMDIR}/src/encauth/ocb/ocb_encrypt_authenticate_memory.c"
    "${TOMDIR}/src/encauth/ocb/ocb_init.c"
    "${TOMDIR}/src/encauth/ocb/ocb_ntz.c"
    "${TOMDIR}/src/encauth/ocb/ocb_shift_xor.c"
    "${TOMDIR}/src/encauth/ocb/ocb_test.c"
    "${TOMDIR}/src/encauth/ocb/s_ocb_done.c"
  )

  source_group("encauth\\\\ocb" FILES ${TOMCRYPT_ENCAUTH_OCB})

  list(APPEND TOMCRYPT_ENCAUTH_OCB3
    "${TOMDIR}/src/encauth/ocb3/ocb3_add_aad.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_decrypt.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_decrypt_last.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_decrypt_verify_memory.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_done.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_encrypt.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_encrypt_authenticate_memory.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_encrypt_last.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_init.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_int_ntz.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_int_xor_blocks.c"
    "${TOMDIR}/src/encauth/ocb3/ocb3_test.c"
  )

  source_group("encauth\\\\ocb3" FILES ${TOMCRYPT_ENCAUTH_OCB3})

  list(APPEND TOMCRYPT_HASHES
    "${TOMDIR}/src/hashes/blake2b.c"
    "${TOMDIR}/src/hashes/blake2s.c"
    "${TOMDIR}/src/hashes/md2.c"
    "${TOMDIR}/src/hashes/md4.c"
    "${TOMDIR}/src/hashes/md5.c"
    "${TOMDIR}/src/hashes/rmd128.c"
    "${TOMDIR}/src/hashes/rmd160.c"
    "${TOMDIR}/src/hashes/rmd256.c"
    "${TOMDIR}/src/hashes/rmd320.c"
    "${TOMDIR}/src/hashes/sha1.c"
    "${TOMDIR}/src/hashes/sha3.c"
    "${TOMDIR}/src/hashes/sha3_test.c"
    "${TOMDIR}/src/hashes/tiger.c"
  )

  source_group("hashes" FILES ${TOMCRYPT_HASHES})

  list(APPEND TOMCRYPT_HASHES_CHC
    "${TOMDIR}/src/hashes/chc/chc.c"
  )

  source_group("hashes\\\\chc" FILES ${TOMCRYPT_HASHES_CHC})

  list(APPEND TOMCRYPT_HASHES_HELPER
    "${TOMDIR}/src/hashes/helper/hash_file.c"
    "${TOMDIR}/src/hashes/helper/hash_filehandle.c"
    "${TOMDIR}/src/hashes/helper/hash_memory.c"
    "${TOMDIR}/src/hashes/helper/hash_memory_multi.c"
  )

  source_group("hashes\\\\helper" FILES ${TOMCRYPT_HASHES_HELPER})

  list(APPEND TOMCRYPT_HASHES_SHA2
    "${TOMDIR}/src/hashes/sha2/sha224.c"
    "${TOMDIR}/src/hashes/sha2/sha256.c"
    "${TOMDIR}/src/hashes/sha2/sha384.c"
    "${TOMDIR}/src/hashes/sha2/sha512.c"
    "${TOMDIR}/src/hashes/sha2/sha512_224.c"
    "${TOMDIR}/src/hashes/sha2/sha512_256.c"
  )

  source_group("hashes\\\\sha2" FILES ${TOMCRYPT_HASHES_SHA2})

  list(APPEND TOMCRYPT_HASHES_WHIRL
    "${TOMDIR}/src/hashes/whirl/whirl.c"
  )

  source_group("hashes\\\\whirl" FILES ${TOMCRYPT_HASHES_WHIRL})

  list(APPEND TOMCRYPT_MAC_BLAKE2
    "${TOMDIR}/src/mac/blake2/blake2bmac.c"
    "${TOMDIR}/src/mac/blake2/blake2bmac_file.c"
    "${TOMDIR}/src/mac/blake2/blake2bmac_memory.c"
    "${TOMDIR}/src/mac/blake2/blake2bmac_memory_multi.c"
    "${TOMDIR}/src/mac/blake2/blake2bmac_test.c"
    "${TOMDIR}/src/mac/blake2/blake2smac.c"
    "${TOMDIR}/src/mac/blake2/blake2smac_file.c"
    "${TOMDIR}/src/mac/blake2/blake2smac_memory.c"
    "${TOMDIR}/src/mac/blake2/blake2smac_memory_multi.c"
    "${TOMDIR}/src/mac/blake2/blake2smac_test.c"
  )

  source_group("mac\\\\blake2" FILES ${TOMCRYPT_MAC_BLAKE2})

  list(APPEND TOMCRYPT_MAC_F9
    "${TOMDIR}/src/mac/f9/f9_done.c"
    "${TOMDIR}/src/mac/f9/f9_file.c"
    "${TOMDIR}/src/mac/f9/f9_init.c"
    "${TOMDIR}/src/mac/f9/f9_memory.c"
    "${TOMDIR}/src/mac/f9/f9_memory_multi.c"
    "${TOMDIR}/src/mac/f9/f9_process.c"
    "${TOMDIR}/src/mac/f9/f9_test.c"
  )

  source_group("mac\\\\f9" FILES ${TOMCRYPT_MAC_F9})

  list(APPEND TOMCRYPT_MAC_HMAC
    "${TOMDIR}/src/mac/hmac/hmac_done.c"
    "${TOMDIR}/src/mac/hmac/hmac_file.c"
    "${TOMDIR}/src/mac/hmac/hmac_init.c"
    "${TOMDIR}/src/mac/hmac/hmac_memory.c"
    "${TOMDIR}/src/mac/hmac/hmac_memory_multi.c"
    "${TOMDIR}/src/mac/hmac/hmac_process.c"
    "${TOMDIR}/src/mac/hmac/hmac_test.c"
  )

  source_group("mac\\\\hmac" FILES ${TOMCRYPT_MAC_HMAC})

  list(APPEND TOMCRYPT_MAC_OMAC
    "${TOMDIR}/src/mac/omac/omac_done.c"
    "${TOMDIR}/src/mac/omac/omac_file.c"
    "${TOMDIR}/src/mac/omac/omac_init.c"
    "${TOMDIR}/src/mac/omac/omac_memory.c"
    "${TOMDIR}/src/mac/omac/omac_memory_multi.c"
    "${TOMDIR}/src/mac/omac/omac_process.c"
    "${TOMDIR}/src/mac/omac/omac_test.c"
  )

  source_group("mac\\\\omac" FILES ${TOMCRYPT_MAC_OMAC})

  list(APPEND TOMCRYPT_MAC_PELICAN
    "${TOMDIR}/src/mac/pelican/pelican.c"
    "${TOMDIR}/src/mac/pelican/pelican_memory.c"
    "${TOMDIR}/src/mac/pelican/pelican_test.c"
  )

  source_group("mac\\\\pelican" FILES ${TOMCRYPT_MAC_PELICAN})

  list(APPEND TOMCRYPT_MAC_PMAC
    "${TOMDIR}/src/mac/pmac/pmac_done.c"
    "${TOMDIR}/src/mac/pmac/pmac_file.c"
    "${TOMDIR}/src/mac/pmac/pmac_init.c"
    "${TOMDIR}/src/mac/pmac/pmac_memory.c"
    "${TOMDIR}/src/mac/pmac/pmac_memory_multi.c"
    "${TOMDIR}/src/mac/pmac/pmac_ntz.c"
    "${TOMDIR}/src/mac/pmac/pmac_process.c"
    "${TOMDIR}/src/mac/pmac/pmac_shift_xor.c"
    "${TOMDIR}/src/mac/pmac/pmac_test.c"
  )

  source_group("mac\\\\pmac" FILES ${TOMCRYPT_MAC_PMAC})

  list(APPEND TOMCRYPT_MAC_POLY1305
    "${TOMDIR}/src/mac/poly1305/poly1305.c"
    "${TOMDIR}/src/mac/poly1305/poly1305_file.c"
    "${TOMDIR}/src/mac/poly1305/poly1305_memory.c"
    "${TOMDIR}/src/mac/poly1305/poly1305_memory_multi.c"
    "${TOMDIR}/src/mac/poly1305/poly1305_test.c"
  )

  source_group("mac\\\\poly1305" FILES ${TOMCRYPT_MAC_POLY1305})

  list(APPEND TOMCRYPT_MAC_XCBC
    "${TOMDIR}/src/mac/xcbc/xcbc_done.c"
    "${TOMDIR}/src/mac/xcbc/xcbc_file.c"
    "${TOMDIR}/src/mac/xcbc/xcbc_init.c"
    "${TOMDIR}/src/mac/xcbc/xcbc_memory.c"
    "${TOMDIR}/src/mac/xcbc/xcbc_memory_multi.c"
    "${TOMDIR}/src/mac/xcbc/xcbc_process.c"
    "${TOMDIR}/src/mac/xcbc/xcbc_test.c"
  )

  source_group("mac\\\\xcbc" FILES ${TOMCRYPT_MAC_XCBC})

  list(APPEND TOMCRYPT_MATH
    "${TOMDIR}/src/math/gmp_desc.c"
    "${TOMDIR}/src/math/ltm_desc.c"
    "${TOMDIR}/src/math/multi.c"
    "${TOMDIR}/src/math/radix_to_bin.c"
    "${TOMDIR}/src/math/rand_bn.c"
    "${TOMDIR}/src/math/rand_prime.c"
    "${TOMDIR}/src/math/tfm_desc.c"
  )

  source_group("math" FILES ${TOMCRYPT_MATH})

  list(APPEND TOMCRYPT_MATH_FP
    "${TOMDIR}/src/math/fp/ltc_ecc_fp_mulmod.c"
  )

  source_group("math\\\\fp" FILES ${TOMCRYPT_MATH_FP})

  list(APPEND TOMCRYPT_MISC
    "${TOMDIR}/src/misc/adler32.c"
    "${TOMDIR}/src/misc/burn_stack.c"
    "${TOMDIR}/src/misc/compare_testvector.c"
    "${TOMDIR}/src/misc/crc32.c"
    "${TOMDIR}/src/misc/error_to_string.c"
    "${TOMDIR}/src/misc/mem_neq.c"
    "${TOMDIR}/src/misc/pk_get_oid.c"
    "${TOMDIR}/src/misc/zeromem.c"
  )

  source_group("misc" FILES ${TOMCRYPT_MISC})

  list(APPEND TOMCRYPT_MISC_BASE64
    "${TOMDIR}/src/misc/base64/base64_decode.c"
    "${TOMDIR}/src/misc/base64/base64_encode.c"
  )

  source_group("misc\\\\base64" FILES ${TOMCRYPT_MISC_BASE64})

  list(APPEND TOMCRYPT_MISC_CRYPT
    "${TOMDIR}/src/misc/crypt/crypt.c"
    "${TOMDIR}/src/misc/crypt/crypt_argchk.c"
    "${TOMDIR}/src/misc/crypt/crypt_cipher_descriptor.c"
    "${TOMDIR}/src/misc/crypt/crypt_cipher_is_valid.c"
    "${TOMDIR}/src/misc/crypt/crypt_constants.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_cipher.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_cipher_any.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_cipher_id.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_hash.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_hash_any.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_hash_id.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_hash_oid.c"
    "${TOMDIR}/src/misc/crypt/crypt_find_prng.c"
    "${TOMDIR}/src/misc/crypt/crypt_fsa.c"
    "${TOMDIR}/src/misc/crypt/crypt_hash_descriptor.c"
    "${TOMDIR}/src/misc/crypt/crypt_hash_is_valid.c"
    "${TOMDIR}/src/misc/crypt/crypt_inits.c"
    "${TOMDIR}/src/misc/crypt/crypt_ltc_mp_descriptor.c"
    "${TOMDIR}/src/misc/crypt/crypt_prng_descriptor.c"
    "${TOMDIR}/src/misc/crypt/crypt_prng_is_valid.c"
    "${TOMDIR}/src/misc/crypt/crypt_prng_rng_descriptor.c"
    "${TOMDIR}/src/misc/crypt/crypt_register_all_ciphers.c"
    "${TOMDIR}/src/misc/crypt/crypt_register_all_hashes.c"
    "${TOMDIR}/src/misc/crypt/crypt_register_all_prngs.c"
    "${TOMDIR}/src/misc/crypt/crypt_register_cipher.c"
    "${TOMDIR}/src/misc/crypt/crypt_register_hash.c"
    "${TOMDIR}/src/misc/crypt/crypt_register_prng.c"
    "${TOMDIR}/src/misc/crypt/crypt_sizes.c"
    "${TOMDIR}/src/misc/crypt/crypt_unregister_cipher.c"
    "${TOMDIR}/src/misc/crypt/crypt_unregister_hash.c"
    "${TOMDIR}/src/misc/crypt/crypt_unregister_prng.c"
  )

  source_group("misc\\\\crypt" FILES ${TOMCRYPT_MISC_CRYPT})

  list(APPEND TOMCRYPT_MISC_HKDF
    "${TOMDIR}/src/misc/hkdf/hkdf.c"
    "${TOMDIR}/src/misc/hkdf/hkdf_test.c"
  )

  source_group("misc\\\\hkdf" FILES ${TOMCRYPT_MISC_HKDF})

  list(APPEND TOMCRYPT_MISC_PKCS5
    "${TOMDIR}/src/misc/pkcs5/pkcs_5_1.c"
    "${TOMDIR}/src/misc/pkcs5/pkcs_5_2.c"
    "${TOMDIR}/src/misc/pkcs5/pkcs_5_test.c"
  )

  source_group("misc\\\\pkcs5" FILES ${TOMCRYPT_MISC_PKCS5})

  list(APPEND TOMCRYPT_MODES_CBC
    "${TOMDIR}/src/modes/cbc/cbc_decrypt.c"
    "${TOMDIR}/src/modes/cbc/cbc_done.c"
    "${TOMDIR}/src/modes/cbc/cbc_encrypt.c"
    "${TOMDIR}/src/modes/cbc/cbc_getiv.c"
    "${TOMDIR}/src/modes/cbc/cbc_setiv.c"
    "${TOMDIR}/src/modes/cbc/cbc_start.c"
  )

  source_group("modes\\\\cbc" FILES ${TOMCRYPT_MODES_CBC})

  list(APPEND TOMCRYPT_MODES_CFB
    "${TOMDIR}/src/modes/cfb/cfb_decrypt.c"
    "${TOMDIR}/src/modes/cfb/cfb_done.c"
    "${TOMDIR}/src/modes/cfb/cfb_encrypt.c"
    "${TOMDIR}/src/modes/cfb/cfb_getiv.c"
    "${TOMDIR}/src/modes/cfb/cfb_setiv.c"
    "${TOMDIR}/src/modes/cfb/cfb_start.c"
  )

  source_group("modes\\\\cfb" FILES ${TOMCRYPT_MODES_CFB})

  list(APPEND TOMCRYPT_MODES_CTR
    "${TOMDIR}/src/modes/ctr/ctr_decrypt.c"
    "${TOMDIR}/src/modes/ctr/ctr_done.c"
    "${TOMDIR}/src/modes/ctr/ctr_encrypt.c"
    "${TOMDIR}/src/modes/ctr/ctr_getiv.c"
    "${TOMDIR}/src/modes/ctr/ctr_setiv.c"
    "${TOMDIR}/src/modes/ctr/ctr_start.c"
    "${TOMDIR}/src/modes/ctr/ctr_test.c"
  )

  source_group("modes\\\\ctr" FILES ${TOMCRYPT_MODES_CTR})

  list(APPEND TOMCRYPT_MODES_ECB
    "${TOMDIR}/src/modes/ecb/ecb_decrypt.c"
    "${TOMDIR}/src/modes/ecb/ecb_done.c"
    "${TOMDIR}/src/modes/ecb/ecb_encrypt.c"
    "${TOMDIR}/src/modes/ecb/ecb_start.c"
  )

  source_group("modes\\\\ecb" FILES ${TOMCRYPT_MODES_ECB})

  list(APPEND TOMCRYPT_MODES_F8
    "${TOMDIR}/src/modes/f8/f8_decrypt.c"
    "${TOMDIR}/src/modes/f8/f8_done.c"
    "${TOMDIR}/src/modes/f8/f8_encrypt.c"
    "${TOMDIR}/src/modes/f8/f8_getiv.c"
    "${TOMDIR}/src/modes/f8/f8_setiv.c"
    "${TOMDIR}/src/modes/f8/f8_start.c"
    "${TOMDIR}/src/modes/f8/f8_test_mode.c"
  )

  source_group("modes\\\\f8" FILES ${TOMCRYPT_MODES_F8})

  list(APPEND TOMCRYPT_MODES_LRW
    "${TOMDIR}/src/modes/lrw/lrw_decrypt.c"
    "${TOMDIR}/src/modes/lrw/lrw_done.c"
    "${TOMDIR}/src/modes/lrw/lrw_encrypt.c"
    "${TOMDIR}/src/modes/lrw/lrw_getiv.c"
    "${TOMDIR}/src/modes/lrw/lrw_process.c"
    "${TOMDIR}/src/modes/lrw/lrw_setiv.c"
    "${TOMDIR}/src/modes/lrw/lrw_start.c"
    "${TOMDIR}/src/modes/lrw/lrw_test.c"
  )

  source_group("modes\\\\lrw" FILES ${TOMCRYPT_MODES_LRW})

  list(APPEND TOMCRYPT_MODES_OFB
    "${TOMDIR}/src/modes/ofb/ofb_decrypt.c"
    "${TOMDIR}/src/modes/ofb/ofb_done.c"
    "${TOMDIR}/src/modes/ofb/ofb_encrypt.c"
    "${TOMDIR}/src/modes/ofb/ofb_getiv.c"
    "${TOMDIR}/src/modes/ofb/ofb_setiv.c"
    "${TOMDIR}/src/modes/ofb/ofb_start.c"
  )

  source_group("modes\\\\ofb" FILES ${TOMCRYPT_MODES_OFB})

  list(APPEND TOMCRYPT_MODES_XTS
    "${TOMDIR}/src/modes/xts/xts_decrypt.c"
    "${TOMDIR}/src/modes/xts/xts_done.c"
    "${TOMDIR}/src/modes/xts/xts_encrypt.c"
    "${TOMDIR}/src/modes/xts/xts_init.c"
    "${TOMDIR}/src/modes/xts/xts_mult_x.c"
    "${TOMDIR}/src/modes/xts/xts_test.c"
  )

  source_group("modes\\\\xts" FILES ${TOMCRYPT_MODES_XTS})

  list(APPEND TOMCRYPT_PK_ASN1_DER_BIT
    "${TOMDIR}/src/pk/asn1/der/bit/der_decode_bit_string.c"
    "${TOMDIR}/src/pk/asn1/der/bit/der_decode_raw_bit_string.c"
    "${TOMDIR}/src/pk/asn1/der/bit/der_encode_bit_string.c"
    "${TOMDIR}/src/pk/asn1/der/bit/der_encode_raw_bit_string.c"
    "${TOMDIR}/src/pk/asn1/der/bit/der_length_bit_string.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\bit" FILES ${TOMCRYPT_PK_ASN1_DER_BIT})

  list(APPEND TOMCRYPT_PK_ASN1_DER_BOOLEAN
    "${TOMDIR}/src/pk/asn1/der/boolean/der_decode_boolean.c"
    "${TOMDIR}/src/pk/asn1/der/boolean/der_encode_boolean.c"
    "${TOMDIR}/src/pk/asn1/der/boolean/der_length_boolean.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\boolean" FILES ${TOMCRYPT_PK_ASN1_DER_BOOLEAN})

  list(APPEND TOMCRYPT_PK_ASN1_DER_CHOICE
    "${TOMDIR}/src/pk/asn1/der/choice/der_decode_choice.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\choice" FILES ${TOMCRYPT_PK_ASN1_DER_CHOICE})

  list(APPEND TOMCRYPT_PK_ASN1_DER_GENERALIZEDTIME
    "${TOMDIR}/src/pk/asn1/der/generalizedtime/der_decode_generalizedtime.c"
    "${TOMDIR}/src/pk/asn1/der/generalizedtime/der_encode_generalizedtime.c"
    "${TOMDIR}/src/pk/asn1/der/generalizedtime/der_length_generalizedtime.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\generalizedtime" FILES ${TOMCRYPT_PK_ASN1_DER_GENERALIZEDTIME})

  list(APPEND TOMCRYPT_PK_ASN1_DER_IA5
    "${TOMDIR}/src/pk/asn1/der/ia5/der_decode_ia5_string.c"
    "${TOMDIR}/src/pk/asn1/der/ia5/der_encode_ia5_string.c"
    "${TOMDIR}/src/pk/asn1/der/ia5/der_length_ia5_string.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\ia5" FILES ${TOMCRYPT_PK_ASN1_DER_IA5})

  list(APPEND TOMCRYPT_PK_ASN1_DER_INTEGER
    "${TOMDIR}/src/pk/asn1/der/integer/der_decode_integer.c"
    "${TOMDIR}/src/pk/asn1/der/integer/der_encode_integer.c"
    "${TOMDIR}/src/pk/asn1/der/integer/der_length_integer.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\integer" FILES ${TOMCRYPT_PK_ASN1_DER_INTEGER})

  list(APPEND TOMCRYPT_PK_ASN1_DER_OBJECT_IDENTIFIER
    "${TOMDIR}/src/pk/asn1/der/object_identifier/der_decode_object_identifier.c"
    "${TOMDIR}/src/pk/asn1/der/object_identifier/der_encode_object_identifier.c"
    "${TOMDIR}/src/pk/asn1/der/object_identifier/der_length_object_identifier.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\object_identifier" FILES ${TOMCRYPT_PK_ASN1_DER_OBJECT_IDENTIFIER})

  list(APPEND TOMCRYPT_PK_ASN1_DER_OCTET
    "${TOMDIR}/src/pk/asn1/der/octet/der_decode_octet_string.c"
    "${TOMDIR}/src/pk/asn1/der/octet/der_encode_octet_string.c"
    "${TOMDIR}/src/pk/asn1/der/octet/der_length_octet_string.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\octet" FILES ${TOMCRYPT_PK_ASN1_DER_OCTET})

  list(APPEND TOMCRYPT_PK_ASN1_DER_PRINTABLE_STRING
    "${TOMDIR}/src/pk/asn1/der/printable_string/der_decode_printable_string.c"
    "${TOMDIR}/src/pk/asn1/der/printable_string/der_encode_printable_string.c"
    "${TOMDIR}/src/pk/asn1/der/printable_string/der_length_printable_string.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\printable_string" FILES ${TOMCRYPT_PK_ASN1_DER_PRINTABLE_STRING})

  list(APPEND TOMCRYPT_PK_ASN1_DER_SEQUENCE
    "${TOMDIR}/src/pk/asn1/der/sequence/der_decode_sequence_ex.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_decode_sequence_flexi.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_decode_sequence_multi.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_decode_subject_public_key_info.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_encode_sequence_ex.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_encode_sequence_multi.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_encode_subject_public_key_info.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_length_sequence.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_sequence_free.c"
    "${TOMDIR}/src/pk/asn1/der/sequence/der_sequence_shrink.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\sequence" FILES ${TOMCRYPT_PK_ASN1_DER_SEQUENCE})

  list(APPEND TOMCRYPT_PK_ASN1_DER_SET
    "${TOMDIR}/src/pk/asn1/der/set/der_encode_set.c"
    "${TOMDIR}/src/pk/asn1/der/set/der_encode_setof.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\set" FILES ${TOMCRYPT_PK_ASN1_DER_SET})

  list(APPEND TOMCRYPT_PK_ASN1_DER_SHORT_INTEGER
    "${TOMDIR}/src/pk/asn1/der/short_integer/der_decode_short_integer.c"
    "${TOMDIR}/src/pk/asn1/der/short_integer/der_encode_short_integer.c"
    "${TOMDIR}/src/pk/asn1/der/short_integer/der_length_short_integer.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\short_integer" FILES ${TOMCRYPT_PK_ASN1_DER_SHORT_INTEGER})

  list(APPEND TOMCRYPT_PK_ASN1_DER_TELETEX_STRING
    "${TOMDIR}/src/pk/asn1/der/teletex_string/der_decode_teletex_string.c"
    "${TOMDIR}/src/pk/asn1/der/teletex_string/der_length_teletex_string.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\teletex_string" FILES ${TOMCRYPT_PK_ASN1_DER_TELETEX_STRING})

  list(APPEND TOMCRYPT_PK_ASN1_DER_UTCTIME
    "${TOMDIR}/src/pk/asn1/der/utctime/der_decode_utctime.c"
    "${TOMDIR}/src/pk/asn1/der/utctime/der_encode_utctime.c"
    "${TOMDIR}/src/pk/asn1/der/utctime/der_length_utctime.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\utctime" FILES ${TOMCRYPT_PK_ASN1_DER_UTCTIME})

  list(APPEND TOMCRYPT_PK_ASN1_DER_UTF8
    "${TOMDIR}/src/pk/asn1/der/utf8/der_decode_utf8_string.c"
    "${TOMDIR}/src/pk/asn1/der/utf8/der_encode_utf8_string.c"
    "${TOMDIR}/src/pk/asn1/der/utf8/der_length_utf8_string.c"
  )

  source_group("pk\\\\asn1\\\\der\\\\utf8" FILES ${TOMCRYPT_PK_ASN1_DER_UTF8})

  list(APPEND TOMCRYPT_PK_DH
    "${TOMDIR}/src/pk/dh/dh.c"
    "${TOMDIR}/src/pk/dh/dh_check_pubkey.c"
    "${TOMDIR}/src/pk/dh/dh_export.c"
    "${TOMDIR}/src/pk/dh/dh_export_key.c"
    "${TOMDIR}/src/pk/dh/dh_free.c"
    "${TOMDIR}/src/pk/dh/dh_generate_key.c"
    "${TOMDIR}/src/pk/dh/dh_import.c"
    "${TOMDIR}/src/pk/dh/dh_set.c"
    "${TOMDIR}/src/pk/dh/dh_set_pg_dhparam.c"
    "${TOMDIR}/src/pk/dh/dh_shared_secret.c"
  )

  source_group("pk\\\\dh" FILES ${TOMCRYPT_PK_DH})

  list(APPEND TOMCRYPT_PK_DSA
    "${TOMDIR}/src/pk/dsa/dsa_decrypt_key.c"
    "${TOMDIR}/src/pk/dsa/dsa_encrypt_key.c"
    "${TOMDIR}/src/pk/dsa/dsa_export.c"
    "${TOMDIR}/src/pk/dsa/dsa_free.c"
    "${TOMDIR}/src/pk/dsa/dsa_generate_key.c"
    "${TOMDIR}/src/pk/dsa/dsa_generate_pqg.c"
    "${TOMDIR}/src/pk/dsa/dsa_import.c"
    "${TOMDIR}/src/pk/dsa/dsa_make_key.c"
    "${TOMDIR}/src/pk/dsa/dsa_set.c"
    "${TOMDIR}/src/pk/dsa/dsa_set_pqg_dsaparam.c"
    "${TOMDIR}/src/pk/dsa/dsa_shared_secret.c"
    "${TOMDIR}/src/pk/dsa/dsa_sign_hash.c"
    "${TOMDIR}/src/pk/dsa/dsa_verify_hash.c"
    "${TOMDIR}/src/pk/dsa/dsa_verify_key.c"
  )

  source_group("pk\\\\dsa" FILES ${TOMCRYPT_PK_DSA})

  list(APPEND TOMCRYPT_PK_ECC
    "${TOMDIR}/src/pk/ecc/ecc.c"
    "${TOMDIR}/src/pk/ecc/ecc_ansi_x963_export.c"
    "${TOMDIR}/src/pk/ecc/ecc_ansi_x963_import.c"
    "${TOMDIR}/src/pk/ecc/ecc_decrypt_key.c"
    "${TOMDIR}/src/pk/ecc/ecc_encrypt_key.c"
    "${TOMDIR}/src/pk/ecc/ecc_export.c"
    "${TOMDIR}/src/pk/ecc/ecc_free.c"
    "${TOMDIR}/src/pk/ecc/ecc_get_size.c"
    "${TOMDIR}/src/pk/ecc/ecc_import.c"
    "${TOMDIR}/src/pk/ecc/ecc_make_key.c"
    "${TOMDIR}/src/pk/ecc/ecc_shared_secret.c"
    "${TOMDIR}/src/pk/ecc/ecc_sign_hash.c"
    "${TOMDIR}/src/pk/ecc/ecc_sizes.c"
    "${TOMDIR}/src/pk/ecc/ecc_test.c"
    "${TOMDIR}/src/pk/ecc/ecc_verify_hash.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_is_valid_idx.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_map.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_mul2add.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_mulmod.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_mulmod_timing.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_points.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_projective_add_point.c"
    "${TOMDIR}/src/pk/ecc/ltc_ecc_projective_dbl_point.c"
  )

  source_group("pk\\\\ecc" FILES ${TOMCRYPT_PK_ECC})

  list(APPEND TOMCRYPT_PK_KATJA
    "${TOMDIR}/src/pk/katja/katja_decrypt_key.c"
    "${TOMDIR}/src/pk/katja/katja_encrypt_key.c"
    "${TOMDIR}/src/pk/katja/katja_export.c"
    "${TOMDIR}/src/pk/katja/katja_exptmod.c"
    "${TOMDIR}/src/pk/katja/katja_free.c"
    "${TOMDIR}/src/pk/katja/katja_import.c"
    "${TOMDIR}/src/pk/katja/katja_make_key.c"
  )

  source_group("pk\\\\katja" FILES ${TOMCRYPT_PK_KATJA})

  list(APPEND TOMCRYPT_PK_PKCS1
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_i2osp.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_mgf1.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_oaep_decode.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_oaep_encode.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_os2ip.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_pss_decode.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_pss_encode.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_v1_5_decode.c"
    "${TOMDIR}/src/pk/pkcs1/pkcs_1_v1_5_encode.c"
  )

  source_group("pk\\\\pkcs1" FILES ${TOMCRYPT_PK_PKCS1})

  list(APPEND TOMCRYPT_PK_RSA
    "${TOMDIR}/src/pk/rsa/rsa_decrypt_key.c"
    "${TOMDIR}/src/pk/rsa/rsa_encrypt_key.c"
    "${TOMDIR}/src/pk/rsa/rsa_export.c"
    "${TOMDIR}/src/pk/rsa/rsa_exptmod.c"
    "${TOMDIR}/src/pk/rsa/rsa_free.c"
    "${TOMDIR}/src/pk/rsa/rsa_get_size.c"
    "${TOMDIR}/src/pk/rsa/rsa_import.c"
    "${TOMDIR}/src/pk/rsa/rsa_import_pkcs8.c"
    "${TOMDIR}/src/pk/rsa/rsa_import_x509.c"
    "${TOMDIR}/src/pk/rsa/rsa_make_key.c"
    "${TOMDIR}/src/pk/rsa/rsa_set.c"
    "${TOMDIR}/src/pk/rsa/rsa_sign_hash.c"
    "${TOMDIR}/src/pk/rsa/rsa_sign_saltlen_get.c"
    "${TOMDIR}/src/pk/rsa/rsa_verify_hash.c"
  )

  source_group("pk\\\\rsa" FILES ${TOMCRYPT_PK_RSA})

  list(APPEND TOMCRYPT_PRNGS
    "${TOMDIR}/src/prngs/chacha20.c"
    "${TOMDIR}/src/prngs/fortuna.c"
    "${TOMDIR}/src/prngs/rc4.c"
    "${TOMDIR}/src/prngs/rng_get_bytes.c"
    "${TOMDIR}/src/prngs/rng_make_prng.c"
    "${TOMDIR}/src/prngs/sober128.c"
    "${TOMDIR}/src/prngs/sprng.c"
    "${TOMDIR}/src/prngs/yarrow.c"
  )

  source_group("prngs" FILES ${TOMCRYPT_PRNGS})

  list(APPEND TOMCRYPT_STREAM_CHACHA
    "${TOMDIR}/src/stream/chacha/chacha_crypt.c"
    "${TOMDIR}/src/stream/chacha/chacha_done.c"
    "${TOMDIR}/src/stream/chacha/chacha_ivctr32.c"
    "${TOMDIR}/src/stream/chacha/chacha_ivctr64.c"
    "${TOMDIR}/src/stream/chacha/chacha_keystream.c"
    "${TOMDIR}/src/stream/chacha/chacha_setup.c"
    "${TOMDIR}/src/stream/chacha/chacha_test.c"
  )

  source_group("stream\\\\chacha" FILES ${TOMCRYPT_STREAM_CHACHA})

  list(APPEND TOMCRYPT_STREAM_RC4
    "${TOMDIR}/src/stream/rc4/rc4_stream.c"
    "${TOMDIR}/src/stream/rc4/rc4_test.c"
  )

  source_group("stream\\\\rc4" FILES ${TOMCRYPT_STREAM_RC4})

  list(APPEND TOMCRYPT_STREAM_SOBER128
    "${TOMDIR}/src/stream/sober128/sober128_stream.c"
    "${TOMDIR}/src/stream/sober128/sober128_test.c"
  )

  source_group("stream\\\\sober128" FILES ${TOMCRYPT_STREAM_SOBER128})

  list(APPEND TOMCRYPT_SRC
    ${TOMCRYPT_CIPHERS}
    ${TOMCRYPT_CIPHERS_AES}
    ${TOMCRYPT_CIPHERS_SAFER}
    ${TOMCRYPT_CIPHERS_TWOFISH}
    ${TOMCRYPT_ENCAUTH_CCM}
    ${TOMCRYPT_ENCAUTH_CHACHAPOLY}
    ${TOMCRYPT_ENCAUTH_EAX}
    ${TOMCRYPT_ENCAUTH_GCM}
    ${TOMCRYPT_ENCAUTH_OCB}
    ${TOMCRYPT_ENCAUTH_OCB3}
    ${TOMCRYPT_HASHES}
    ${TOMCRYPT_HASHES_CHC}
    ${TOMCRYPT_HASHES_HELPER}
    ${TOMCRYPT_HASHES_SHA2}
    ${TOMCRYPT_HASHES_WHIRL}
    ${TOMCRYPT_MAC_BLAKE2}
    ${TOMCRYPT_MAC_F9}
    ${TOMCRYPT_MAC_HMAC}
    ${TOMCRYPT_MAC_OMAC}
    ${TOMCRYPT_MAC_PELICAN}
    ${TOMCRYPT_MAC_PMAC}
    ${TOMCRYPT_MAC_POLY1305}
    ${TOMCRYPT_MAC_XCBC}
    ${TOMCRYPT_MATH}
    ${TOMCRYPT_MATH_FP}
    ${TOMCRYPT_MISC}
    ${TOMCRYPT_MISC_BASE64}
    ${TOMCRYPT_MISC_CRYPT}
    ${TOMCRYPT_MISC_HKDF}
    ${TOMCRYPT_MISC_PKCS5}
    ${TOMCRYPT_MODES_CBC}
    ${TOMCRYPT_MODES_CFB}
    ${TOMCRYPT_MODES_CTR}
    ${TOMCRYPT_MODES_ECB}
    ${TOMCRYPT_MODES_F8}
    ${TOMCRYPT_MODES_LRW}
    ${TOMCRYPT_MODES_OFB}
    ${TOMCRYPT_MODES_XTS}
    ${TOMCRYPT_PK_ASN1_DER_BIT}
    ${TOMCRYPT_PK_ASN1_DER_BOOLEAN}
    ${TOMCRYPT_PK_ASN1_DER_CHOICE}
    ${TOMCRYPT_PK_ASN1_DER_GENERALIZEDTIME}
    ${TOMCRYPT_PK_ASN1_DER_IA5}
    ${TOMCRYPT_PK_ASN1_DER_INTEGER}
    ${TOMCRYPT_PK_ASN1_DER_OBJECT_IDENTIFIER}
    ${TOMCRYPT_PK_ASN1_DER_OCTET}
    ${TOMCRYPT_PK_ASN1_DER_PRINTABLE_STRING}
    ${TOMCRYPT_PK_ASN1_DER_SEQUENCE}
    ${TOMCRYPT_PK_ASN1_DER_SET}
    ${TOMCRYPT_PK_ASN1_DER_SHORT_INTEGER}
    ${TOMCRYPT_PK_ASN1_DER_TELETEX_STRING}
    ${TOMCRYPT_PK_ASN1_DER_UTCTIME}
    ${TOMCRYPT_PK_ASN1_DER_UTF8}
    ${TOMCRYPT_PK_DH}
    ${TOMCRYPT_PK_DSA}
    ${TOMCRYPT_PK_ECC}
    ${TOMCRYPT_PK_KATJA}
    ${TOMCRYPT_PK_PKCS1}
    ${TOMCRYPT_PK_RSA}
    ${TOMCRYPT_PRNGS}
    ${TOMCRYPT_STREAM_CHACHA}
    ${TOMCRYPT_STREAM_RC4}
    ${TOMCRYPT_STREAM_SOBER128}
  )

  list(APPEND TOMCRYPT_HEADERS
    "${TOMDIR}/src/headers/tomcrypt.h"
    "${TOMDIR}/src/headers/tomcrypt_argchk.h"
    "${TOMDIR}/src/headers/tomcrypt_cfg.h"
    "${TOMDIR}/src/headers/tomcrypt_cipher.h"
    "${TOMDIR}/src/headers/tomcrypt_custom.h"
    "${TOMDIR}/src/headers/tomcrypt_hash.h"
    "${TOMDIR}/src/headers/tomcrypt_mac.h"
    "${TOMDIR}/src/headers/tomcrypt_macros.h"
    "${TOMDIR}/src/headers/tomcrypt_math.h"
    "${TOMDIR}/src/headers/tomcrypt_misc.h"
    "${TOMDIR}/src/headers/tomcrypt_pk.h"
    "${TOMDIR}/src/headers/tomcrypt_pkcs.h"
    "${TOMDIR}/src/headers/tomcrypt_prng.h"
  )

  source_group("headers" FILES ${TOMCRYPT_HEADERS})

  add_library("tomcrypt" STATIC ${TOMCRYPT_SRC} ${TOMCRYPT_HEADERS})

  set_property(TARGET "tomcrypt" PROPERTY FOLDER "External Libraries")

  # Required since building from the source.
  sm_add_compile_definition("tomcrypt" LTC_SOURCE)

  # Required since tommath is a dependency.
  sm_add_compile_definition("tomcrypt" LTM_DESC)

  # This was defined behind an always active block.
  sm_add_compile_definition("tomcrypt" LTC_DEVRANDOM)

  # Common formulas used by our app.
  sm_add_compile_definition("tomcrypt" LTC_SHA1)
  sm_add_compile_definition("tomcrypt" LTC_MD5)

  # Use the full AES encryption items.
  sm_add_compile_definition("tomcrypt" LTC_YARROW)
  sm_add_compile_definition("tomcrypt" LTC_YARROW_AES=3)

  # Other definitions we used in the past, but whose meanings are not clear.
  sm_add_compile_definition("tomcrypt" LTC_NO_PKCS)
  sm_add_compile_definition("tomcrypt" LTC_PKCS_1)
  sm_add_compile_definition("tomcrypt" LTC_DER)
  sm_add_compile_definition("tomcrypt" LTC_NO_MODES)
  sm_add_compile_definition("tomcrypt" LTC_ECB_MODE)
  sm_add_compile_definition("tomcrypt" LTC_CBC_MODE)
  sm_add_compile_definition("tomcrypt" LTC_CTR_MODE)
  sm_add_compile_definition("tomcrypt" LTC_NO_HASHES)
  sm_add_compile_definition("tomcrypt" LTC_NO_MACS) # no MAC (message authentication code) support
  sm_add_compile_definition("tomcrypt" LTC_NO_PRNGS)
  sm_add_compile_definition("tomcrypt" LTC_RNG_GET_BYTES)
  sm_add_compile_definition("tomcrypt" LTC_RNG_MAKE_PRNG)
  sm_add_compile_definition("tomcrypt" LTC_TRY_URANDOM_FIRST)
  sm_add_compile_definition("tomcrypt" LTC_NO_PK)
  sm_add_compile_definition("tomcrypt" LTC_MRSA)
  sm_add_compile_definition("tomcrypt" LTC_NO_PROTOTYPES)

  if(WITH_PORTABLE_TOMCRYPT)
    sm_add_compile_definition("tomcrypt" LTC_NO_ASM)
  endif()
  if(WITH_NO_ROLC_TOMCRYPT)
    sm_add_compile_definition("tomcrypt" LTC_NO_ROLC)
  endif()

  if(APPLE)
    sm_append_simple_target_property("tomcrypt"
                 XCODE_ATTRIBUTE_GCC_NO_COMMON_BLOCKS "YES")
  endif()
  if(MSVC)
    sm_add_compile_definition("tomcrypt" _CRT_SECURE_NO_WARNINGS)
  endif()

  disable_project_warnings("tomcrypt")

  add_dependencies("tomcrypt" "tommath")

  list(APPEND TOMCRYPT_INCLUDE_DIRS
    "${TOMDIR}/src/headers"
    "${SM_SRC_DIR}/libtommath"
  )

  target_include_directories("tomcrypt" PUBLIC ${TOMCRYPT_INCLUDE_DIRS})
endif()
