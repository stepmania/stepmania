set(TOMDIR "${SM_EXTERN_DIR}/tomcrypt")

if (NOT IS_DIRECTORY "${TOMDIR}")
  message(ERROR "Submodule for tomcrypt missing. Run git submodule init && git submodule update first.")
  return()
endif()

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

source_group("src\\\\misc\\\\crypt" FILES ${TOMCRYPT_MISC_CRYPT})

set(TOMCRYPT_CIPHERS_AES
  "${TOMDIR}/src/ciphers/aes/aes.c"
)

source_group("src\\\\ciphers\\\\aes" FILES ${TOMCRYPT_CIPHERS_AES})

set(TOMCRYPT_HASHES
  "${TOMDIR}/src/hashes/md5.c"
  "${TOMDIR}/src/hashes/sha1.c"
  "${TOMDIR}/src/hashes/helper/hash_memory.c"
)

source_group("src\\\\hashes" FILES ${TOMCRYPT_HASHES})

list(APPEND TOMCRYPT_MISC
  "${TOMDIR}/src/misc/burn_stack.c"
  "${TOMDIR}/src/misc/error_to_string.c"
  "${TOMDIR}/src/misc/zeromem.c"
  "${TOMDIR}/src/misc/pk_get_oid.c"
  "${TOMDIR}/src/misc/mem_neq.c"
  "${TOMDIR}/src/misc/compare_testvector.c"
)

source_group("src\\\\misc" FILES ${TOMCRYPT_MISC})

set(TOMCRYPT_MISC_BASE64
  "${TOMDIR}/src/misc/base64/base64_decode.c"
  "${TOMDIR}/src/misc/base64/base64_encode.c"
)

source_group("src\\\\misc\\\\base64" FILES ${TOMCRYPT_MISC_BASE64})

set(TOMCRYPT_MISC_PKCS5
  "${TOMDIR}/src/misc/pkcs5/pkcs_5_1.c"
  "${TOMDIR}/src/misc/pkcs5/pkcs_5_2.c"
)

source_group("src\\\\misc\\\\pkcs5" FILES ${TOMCRYPT_MISC_PKCS5})

set(TOMCRYPT_MATH
  "${TOMDIR}/src/math/ltm_desc.c"
  "${TOMDIR}/src/math/fp/ltc_ecc_fp_mulmod.c"
  "${TOMDIR}/src/math/multi.c"
  "${TOMDIR}/src/math/rand_prime.c"
)

source_group("src\\\\math" FILES ${TOMCRYPT_MATH})

list(APPEND TOMCRYPT_PRNGS
  "${TOMDIR}/src/prngs/fortuna.c"
  "${TOMDIR}/src/prngs/rc4.c"
  "${TOMDIR}/src/prngs/rng_get_bytes.c"
  "${TOMDIR}/src/prngs/rng_make_prng.c"
  "${TOMDIR}/src/prngs/sprng.c"
  "${TOMDIR}/src/prngs/yarrow.c"
)

source_group("src\\\\prngs" FILES ${TOMCRYPT_PRNGS})

list(APPEND TOMCRYPT_MODES_CBC
  "${TOMDIR}/src/modes/cbc/cbc_decrypt.c"
  "${TOMDIR}/src/modes/cbc/cbc_done.c"
  "${TOMDIR}/src/modes/cbc/cbc_encrypt.c"
  "${TOMDIR}/src/modes/cbc/cbc_getiv.c"
  "${TOMDIR}/src/modes/cbc/cbc_setiv.c"
  "${TOMDIR}/src/modes/cbc/cbc_start.c"
)

source_group("src\\\\modes\\\\cbc" FILES ${TOMCRYPT_MODES_CBC})

list(APPEND TOMCRYPT_MODES_CFB
  "${TOMDIR}/src/modes/cfb/cfb_decrypt.c"
  "${TOMDIR}/src/modes/cfb/cfb_done.c"
  "${TOMDIR}/src/modes/cfb/cfb_encrypt.c"
  "${TOMDIR}/src/modes/cfb/cfb_getiv.c"
  "${TOMDIR}/src/modes/cfb/cfb_setiv.c"
  "${TOMDIR}/src/modes/cfb/cfb_start.c"
)

source_group("src\\\\modes\\\\cfb" FILES ${TOMCRYPT_MODES_CFB})

list(APPEND TOMCRYPT_MODES_CTR
  "${TOMDIR}/src/modes/ctr/ctr_decrypt.c"
  "${TOMDIR}/src/modes/ctr/ctr_done.c"
  "${TOMDIR}/src/modes/ctr/ctr_encrypt.c"
  "${TOMDIR}/src/modes/ctr/ctr_getiv.c"
  "${TOMDIR}/src/modes/ctr/ctr_setiv.c"
  "${TOMDIR}/src/modes/ctr/ctr_start.c"
  "${TOMDIR}/src/modes/ctr/ctr_test.c"
)

source_group("src\\\\modes\\\\ctr" FILES ${TOMCRYPT_MODES_CTR})

list(APPEND TOMCRYPT_MODES_ECB
  "${TOMDIR}/src/modes/ecb/ecb_decrypt.c"
  "${TOMDIR}/src/modes/ecb/ecb_done.c"
  "${TOMDIR}/src/modes/ecb/ecb_encrypt.c"
  "${TOMDIR}/src/modes/ecb/ecb_start.c"
)

source_group("src\\\\modes\\\\ecb" FILES ${TOMCRYPT_MODES_ECB})

list(APPEND TOMCRYPT_MODES_OFB
  "${TOMDIR}/src/modes/ofb/ofb_decrypt.c"
  "${TOMDIR}/src/modes/ofb/ofb_done.c"
  "${TOMDIR}/src/modes/ofb/ofb_encrypt.c"
  "${TOMDIR}/src/modes/ofb/ofb_getiv.c"
  "${TOMDIR}/src/modes/ofb/ofb_setiv.c"
  "${TOMDIR}/src/modes/ofb/ofb_start.c"
)

source_group("src\\\\modes\\\\ofb" FILES ${TOMCRYPT_MODES_OFB})

list(APPEND TOMCRYPT_PK_RSA
  "${TOMDIR}/src/pk/rsa/rsa_decrypt_key.c"
  "${TOMDIR}/src/pk/rsa/rsa_encrypt_key.c"
  "${TOMDIR}/src/pk/rsa/rsa_export.c"
  "${TOMDIR}/src/pk/rsa/rsa_exptmod.c"
  "${TOMDIR}/src/pk/rsa/rsa_free.c"
  "${TOMDIR}/src/pk/rsa/rsa_import.c"
  "${TOMDIR}/src/pk/rsa/rsa_make_key.c"
  "${TOMDIR}/src/pk/rsa/rsa_sign_hash.c"
  "${TOMDIR}/src/pk/rsa/rsa_verify_hash.c"
)

source_group("src\\\\pk\\\\rsa" FILES ${TOMCRYPT_PK_RSA})

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

source_group("src\\\\pk\\\\pkcs1" FILES ${TOMCRYPT_PK_PKCS1})

list(APPEND TOMCRYPT_PK_DSA
  "${TOMDIR}/src/pk/dsa/dsa_decrypt_key.c"
  "${TOMDIR}/src/pk/dsa/dsa_encrypt_key.c"
  "${TOMDIR}/src/pk/dsa/dsa_export.c"
  "${TOMDIR}/src/pk/dsa/dsa_free.c"
  "${TOMDIR}/src/pk/dsa/dsa_import.c"
  "${TOMDIR}/src/pk/dsa/dsa_make_key.c"
  "${TOMDIR}/src/pk/dsa/dsa_shared_secret.c"
  "${TOMDIR}/src/pk/dsa/dsa_sign_hash.c"
  "${TOMDIR}/src/pk/dsa/dsa_verify_hash.c"
  "${TOMDIR}/src/pk/dsa/dsa_verify_key.c"
)

source_group("src\\\\pk\\\\dsa" FILES ${TOMCRYPT_PK_DSA})

list(APPEND TOMCRYPT_PK_DER
  # bit
  "${TOMDIR}/src/pk/asn1/der/bit/der_decode_bit_string.c"
  "${TOMDIR}/src/pk/asn1/der/bit/der_decode_raw_bit_string.c"
  "${TOMDIR}/src/pk/asn1/der/bit/der_encode_bit_string.c"
  "${TOMDIR}/src/pk/asn1/der/bit/der_encode_raw_bit_string.c"
  "${TOMDIR}/src/pk/asn1/der/bit/der_length_bit_string.c"
  # boolean
  "${TOMDIR}/src/pk/asn1/der/boolean/der_decode_boolean.c"
  "${TOMDIR}/src/pk/asn1/der/boolean/der_encode_boolean.c"
  "${TOMDIR}/src/pk/asn1/der/boolean/der_length_boolean.c"
  # choice
  "${TOMDIR}/src/pk/asn1/der/choice/der_decode_choice.c"
  # generalizedtime
  "${TOMDIR}/src/pk/asn1/der/generalizedtime/der_decode_generalizedtime.c"
  "${TOMDIR}/src/pk/asn1/der/generalizedtime/der_encode_generalizedtime.c"
  "${TOMDIR}/src/pk/asn1/der/generalizedtime/der_length_generalizedtime.c"
  # ia5
  "${TOMDIR}/src/pk/asn1/der/ia5/der_decode_ia5_string.c"
  "${TOMDIR}/src/pk/asn1/der/ia5/der_encode_ia5_string.c"
  "${TOMDIR}/src/pk/asn1/der/ia5/der_length_ia5_string.c"
  # integer
  "${TOMDIR}/src/pk/asn1/der/integer/der_decode_integer.c"
  "${TOMDIR}/src/pk/asn1/der/integer/der_encode_integer.c"
  "${TOMDIR}/src/pk/asn1/der/integer/der_length_integer.c"
  # object_identifier
  "${TOMDIR}/src/pk/asn1/der/object_identifier/der_decode_object_identifier.c"
  "${TOMDIR}/src/pk/asn1/der/object_identifier/der_encode_object_identifier.c"
  "${TOMDIR}/src/pk/asn1/der/object_identifier/der_length_object_identifier.c"
  # octect
  "${TOMDIR}/src/pk/asn1/der/octet/der_decode_octet_string.c"
  "${TOMDIR}/src/pk/asn1/der/octet/der_encode_octet_string.c"
  "${TOMDIR}/src/pk/asn1/der/octet/der_length_octet_string.c"
  # printable_string
  "${TOMDIR}/src/pk/asn1/der/printable_string/der_decode_printable_string.c"
  "${TOMDIR}/src/pk/asn1/der/printable_string/der_encode_printable_string.c"
  "${TOMDIR}/src/pk/asn1/der/printable_string/der_length_printable_string.c"
  # sequence
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
  # set
  "${TOMDIR}/src/pk/asn1/der/set/der_encode_set.c"
  "${TOMDIR}/src/pk/asn1/der/set/der_encode_setof.c"
  # short_integer
  "${TOMDIR}/src/pk/asn1/der/short_integer/der_decode_short_integer.c"
  "${TOMDIR}/src/pk/asn1/der/short_integer/der_encode_short_integer.c"
  "${TOMDIR}/src/pk/asn1/der/short_integer/der_length_short_integer.c"
  # teletex_string
  "${TOMDIR}/src/pk/asn1/der/teletex_string/der_decode_teletex_string.c"
  "${TOMDIR}/src/pk/asn1/der/teletex_string/der_length_teletex_string.c"
  # utctime
  "${TOMDIR}/src/pk/asn1/der/utctime/der_decode_utctime.c"
  "${TOMDIR}/src/pk/asn1/der/utctime/der_encode_utctime.c"
  "${TOMDIR}/src/pk/asn1/der/utctime/der_length_utctime.c"
  # utf8
  "${TOMDIR}/src/pk/asn1/der/utf8/der_decode_utf8_string.c"
  "${TOMDIR}/src/pk/asn1/der/utf8/der_encode_utf8_string.c"
  "${TOMDIR}/src/pk/asn1/der/utf8/der_length_utf8_string.c"
)

source_group("src\\\\pk\\\\ans1" FILES ${TOMCRYPT_PK_DER})

list(APPEND TOMCRYPT_SRC
  ${TOMCRYPT_CIPHERS_AES}
  ${TOMCRYPT_HASHES}
  ${TOMCRYPT_MISC}
  ${TOMCRYPT_MISC_BASE64}
  ${TOMCRYPT_MISC_CRYPT}
  ${TOMCRYPT_MISC_PKCS5}
  ${TOMCRYPT_MATH}
  ${TOMCRYPT_MODES_CBC}
  ${TOMCRYPT_MODES_CFB}
  ${TOMCRYPT_MODES_CTR}
  ${TOMCRYPT_MODES_ECB}
  ${TOMCRYPT_MODES_OFB}
  ${TOMCRYPT_PRNGS}
  ${TOMCRYPT_PK_RSA}
  ${TOMCRYPT_PK_PKCS1}
  ${TOMCRYPT_PK_DSA}
  ${TOMCRYPT_PK_DER}
)

list(APPEND TOMCRYPT_HPP
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

source_group("headers" FILES ${TOMCRYPT_HPP})

add_library("tomcrypt" STATIC ${TOMCRYPT_SRC} ${TOMCRYPT_HPP})

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

if (WITH_PORTABLE_TOMCRYPT)
  sm_add_compile_definition("tomcrypt" LTC_NO_ASM)
elseif(WITH_NO_ROLC_TOMCRYPT AND NOT APPLE)
  sm_add_compile_definition("tomcrypt" LTC_NO_ROLC)
endif()

if (APPLE)
  sm_add_compile_definition("tomcrypt" LTC_NO_ROLC)
  sm_append_simple_target_property("tomcrypt" XCODE_ATTRIBUTE_GCC_NO_COMMON_BLOCKS "YES")
elseif (MSVC)
  sm_add_compile_definition("tomcrypt" _CRT_SECURE_NO_WARNINGS)
endif()

disable_project_warnings("tomcrypt")

add_dependencies("tomcrypt" "tommath")

list(APPEND TOMCRYPT_INCLUDE_DIRS
  "${TOMDIR}/src/headers"
  "${SM_EXTERN_DIR}/tommath"
)

target_include_directories("tomcrypt" PUBLIC ${TOMCRYPT_INCLUDE_DIRS})

