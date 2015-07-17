set(TOMCRYPT_DIR "libtomcrypt-1.17")

set(TOMCRYPT_CIPHERS_AES
  "${TOMCRYPT_DIR}/src/ciphers/aes/aes.c"
)

source_group("ciphers\\aes" FILES ${TOMCRYPT_CIPHERS_AES})

set(TOMCRYPT_HASHES
  "${TOMCRYPT_DIR}/src/hashes/md5.c"
  "${TOMCRYPT_DIR}/src/hashes/sha1.c"
)

source_group("hashes" FILES ${TOMCRYPT_HASHES})

set(TOMCRYPT_HASHES_HELPER
  "${TOMCRYPT_DIR}/src/hashes/helper/hash_memory.c"
)

source_group("hashes\\helper" FILES ${TOMCRYPT_HASHES_HELPER})

set(TOMCRYPT_MATH
  "${TOMCRYPT_DIR}/src/math/ltm_desc.c"
  "${TOMCRYPT_DIR}/src/math/multi.c"
  "${TOMCRYPT_DIR}/src/math/rand_prime.c"
)

source_group("math" FILES ${TOMCRYPT_MATH})

set(TOMCRYPT_MATH_FP
  "${TOMCRYPT_DIR}/src/math/fp/ltc_ecc_fp_mulmod.c"
)

source_group("math\\fp" FILES ${TOMCRYPT_MATH_FP})

set(TOMCRYPT_MISC
  "${TOMCRYPT_DIR}/src/misc/burn_stack.c"
  "${TOMCRYPT_DIR}/src/misc/error_to_string.c"
  "${TOMCRYPT_DIR}/src/misc/zeromem.c"
)

source_group("misc" FILES ${TOMCRYPT_MISC})

set(TOMCRYPT_MISC_BASE64
  "${TOMCRYPT_DIR}/src/misc/base64/base64_decode.c"
  "${TOMCRYPT_DIR}/src/misc/base64/base64_encode.c"
)

source_group("misc\\base64" FILES ${TOMCRYPT_MISC_BASE64})

set(TOMCRYPT_MISC_CRYPT
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_argchk.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_cipher_descriptor.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_cipher_is_valid.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_cipher.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_cipher_any.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_cipher_id.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_hash.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_hash_any.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_hash_id.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_hash_oid.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_find_prng.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_fsa.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_hash_descriptor.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_hash_is_valid.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_ltc_mp_descriptor.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_prng_descriptor.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_prng_is_valid.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_register_cipher.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_register_hash.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_register_prng.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_unregister_cipher.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_unregister_hash.c"
  "${TOMCRYPT_DIR}/src/misc/crypt/crypt_unregister_prng.c"
)

source_group("misc\\crypt" FILES ${TOMCRYPT_MISC_CRYPT})

set(TOMCRYPT_MISC_PKCS5
  "${TOMCRYPT_DIR}/src/misc/pkcs5/pkcs_5_1.c"
  "${TOMCRYPT_DIR}/src/misc/pkcs5/pkcs_5_2.c"
)

source_group("misc\\pkcs5" FILES ${TOMCRYPT_MISC_PKCS5})

set(TOMCRYPT_MODES_CBC
  "${TOMCRYPT_DIR}/src/modes/cbc/cbc_decrypt.c"
  "${TOMCRYPT_DIR}/src/modes/cbc/cbc_done.c"
  "${TOMCRYPT_DIR}/src/modes/cbc/cbc_encrypt.c"
  "${TOMCRYPT_DIR}/src/modes/cbc/cbc_getiv.c"
  "${TOMCRYPT_DIR}/src/modes/cbc/cbc_setiv.c"
  "${TOMCRYPT_DIR}/src/modes/cbc/cbc_start.c"
)

source_group("modes\\cbc" FILES ${TOMCRYPT_MODES_CBC})

set(TOMCRYPT_MODES_CFB
  "${TOMCRYPT_DIR}/src/modes/cfb/cfb_decrypt.c"
  "${TOMCRYPT_DIR}/src/modes/cfb/cfb_done.c"
  "${TOMCRYPT_DIR}/src/modes/cfb/cfb_encrypt.c"
  "${TOMCRYPT_DIR}/src/modes/cfb/cfb_getiv.c"
  "${TOMCRYPT_DIR}/src/modes/cfb/cfb_setiv.c"
  "${TOMCRYPT_DIR}/src/modes/cfb/cfb_start.c"
)

source_group("modes\\cfb" FILES ${TOMCRYPT_MODES_CFB})

set(TOMCRYPT_MODES_CTR
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_decrypt.c"
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_done.c"
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_encrypt.c"
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_getiv.c"
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_setiv.c"
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_start.c"
  "${TOMCRYPT_DIR}/src/modes/ctr/ctr_test.c"
)

source_group("modes\\ctr" FILES ${TOMCRYPT_MODES_CTR})

set(TOMCRYPT_MODES_ECB
  "${TOMCRYPT_DIR}/src/modes/ecb/ecb_decrypt.c"
  "${TOMCRYPT_DIR}/src/modes/ecb/ecb_done.c"
  "${TOMCRYPT_DIR}/src/modes/ecb/ecb_encrypt.c"
  "${TOMCRYPT_DIR}/src/modes/ecb/ecb_start.c"
)

source_group("modes\\ecb" FILES ${TOMCRYPT_MODES_ECB})

set(TOMCRYPT_MODES_OFB
  "${TOMCRYPT_DIR}/src/modes/ofb/ofb_decrypt.c"
  "${TOMCRYPT_DIR}/src/modes/ofb/ofb_done.c"
  "${TOMCRYPT_DIR}/src/modes/ofb/ofb_encrypt.c"
  "${TOMCRYPT_DIR}/src/modes/ofb/ofb_getiv.c"
  "${TOMCRYPT_DIR}/src/modes/ofb/ofb_setiv.c"
  "${TOMCRYPT_DIR}/src/modes/ofb/ofb_start.c"
)

source_group("modes\\ofb" FILES ${TOMCRYPT_MODES_OFB})

set(TOMCRYPT_PK_ASN1_DER_BIT
  "${TOMCRYPT_DIR}/src/pk/asn1/der/bit/der_decode_bit_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/bit/der_encode_bit_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/bit/der_length_bit_string.c"
)

source_group("pk\\asn1\\der\\bit" FILES ${TOMCRYPT_PK_ASN1_DER_BIT})

set(TOMCRYPT_PK_ASN1_DER_BOOLEAN
  "${TOMCRYPT_DIR}/src/pk/asn1/der/boolean/der_decode_boolean.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/boolean/der_encode_boolean.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/boolean/der_length_boolean.c"
)

source_group("pk\\asn1\\der\\boolean" FILES ${TOMCRYPT_PK_ASN1_DER_BOOLEAN})

set(TOMCRYPT_PK_ASN1_DER_CHOICE
  "${TOMCRYPT_DIR}/src/pk/asn1/der/choice/der_decode_choice.c"
)

source_group("pk\\asn1\\der\\choice" FILES ${TOMCRYPT_PK_ASN1_DER_CHOICE})

set(TOMCRYPT_PK_ASN1_DER_IA5
  "${TOMCRYPT_DIR}/src/pk/asn1/der/ia5/der_decode_ia5_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/ia5/der_encode_ia5_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/ia5/der_length_ia5_string.c"
)

source_group("pk\\asn1\\der\\ia5" FILES ${TOMCRYPT_PK_ASN1_DER_IA5})

set(TOMCRYPT_PK_ASN1_DER_INTEGER
  "${TOMCRYPT_DIR}/src/pk/asn1/der/integer/der_decode_integer.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/integer/der_encode_integer.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/integer/der_length_integer.c"
)

source_group("pk\\asn1\\der\\integer" FILES ${TOMCRYPT_PK_ASN1_DER_INTEGER})

set(TOMCRYPT_PK_ASN1_DER_OBJECT_IDENTIFIER
  "${TOMCRYPT_DIR}/src/pk/asn1/der/object_identifier/der_decode_object_identifier.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/object_identifier/der_encode_object_identifier.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/object_identifier/der_length_object_identifier.c"
)

source_group("pk\\asn1\\der\\object_identifier" FILES ${TOMCRYPT_PK_ASN1_DER_OBJECT_IDENTIFIER})

set(TOMCRYPT_PK_ASN1_DER_OCTET
  "${TOMCRYPT_DIR}/src/pk/asn1/der/octet/der_decode_octet_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/octet/der_encode_octet_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/octet/der_length_octet_string.c"
)

source_group("pk\\asn1\\der\\octet" FILES ${TOMCRYPT_PK_ASN1_DER_OCTET})

set(TOMCRYPT_PK_ASN1_DER_PRINTABLE_STRING
  "${TOMCRYPT_DIR}/src/pk/asn1/der/printable_string/der_decode_printable_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/printable_string/der_encode_printable_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/printable_string/der_length_printable_string.c"
)

source_group("pk\\asn1\\der\\printable_string" FILES ${TOMCRYPT_PK_ASN1_DER_PRINTABLE_STRING})

set(TOMCRYPT_PK_ASN1_DER_SEQUENCE
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_decode_sequence_ex.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_decode_sequence_flexi.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_decode_sequence_multi.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_encode_sequence_ex.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_encode_sequence_multi.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_length_sequence.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/sequence/der_sequence_free.c"
)

source_group("pk\\asn1\\der\\sequence" FILES ${TOMCRYPT_PK_ASN1_DER_SEQUENCE})

set(TOMCRYPT_PK_ASN1_DER_SET
  "${TOMCRYPT_DIR}/src/pk/asn1/der/set/der_encode_set.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/set/der_encode_setof.c"
)

source_group("pk\\asn1\\der\\set" FILES ${TOMCRYPT_PK_ASN1_DER_SET})

set(TOMCRYPT_PK_ASN1_DER_SHORT_INTEGER
  "${TOMCRYPT_DIR}/src/pk/asn1/der/short_integer/der_decode_short_integer.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/short_integer/der_encode_short_integer.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/short_integer/der_length_short_integer.c"
)

source_group("pk\\asn1\\der\\short_integer" FILES ${TOMCRYPT_PK_ASN1_DER_SHORT_INTEGER})

set(TOMCRYPT_PK_ASN1_DER_UTCTIME
  "${TOMCRYPT_DIR}/src/pk/asn1/der/utctime/der_decode_utctime.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/utctime/der_encode_utctime.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/utctime/der_length_utctime.c"
)

source_group("pk\\asn1\\der\\utctime" FILES ${TOMCRYPT_PK_ASN1_DER_UTCTIME})

set(TOMCRYPT_PK_ASN1_DER_UTF8
  "${TOMCRYPT_DIR}/src/pk/asn1/der/utf8/der_decode_utf8_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/utf8/der_encode_utf8_string.c"
  "${TOMCRYPT_DIR}/src/pk/asn1/der/utf8/der_length_utf8_string.c"
)

source_group("pk\\asn1\\der\\utf8" FILES ${TOMCRYPT_PK_ASN1_DER_UTF8})

set(TOMCRYPT_PK_DSA
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_decrypt_key.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_encrypt_key.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_export.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_free.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_import.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_make_key.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_shared_secret.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_sign_hash.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_verify_hash.c"
  "${TOMCRYPT_DIR}/src/pk/dsa/dsa_verify_key.c"
)

source_group("pk\\dsa" FILES ${TOMCRYPT_PK_DSA})

set(TOMCRYPT_PK_PKCS1
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_i2osp.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_mgf1.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_oaep_decode.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_oaep_encode.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_os2ip.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_pss_decode.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_pss_encode.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_v1_5_decode.c"
  "${TOMCRYPT_DIR}/src/pk/pkcs1/pkcs_1_v1_5_encode.c"
)

source_group("pk\\pkcs1" FILES ${TOMCRYPT_PK_PKCS1})

set(TOMCRYPT_PK_RSA
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_decrypt_key.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_encrypt_key.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_export.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_exptmod.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_free.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_import.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_make_key.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_sign_hash.c"
  "${TOMCRYPT_DIR}/src/pk/rsa/rsa_verify_hash.c"
)

source_group("pk\\rsa" FILES ${TOMCRYPT_PK_RSA})

set(TOMCRYPT_PRNGS
  "${TOMCRYPT_DIR}/src/prngs/fortuna.c"
  "${TOMCRYPT_DIR}/src/prngs/rc4.c"
  "${TOMCRYPT_DIR}/src/prngs/rng_get_bytes.c"
  "${TOMCRYPT_DIR}/src/prngs/rng_make_prng.c"
  "${TOMCRYPT_DIR}/src/prngs/sprng.c"
  "${TOMCRYPT_DIR}/src/prngs/yarrow.c"
)

source_group("prngs" FILES ${TOMCRYPT_PRNGS})

set(TOMCRYPT_SRC
  ${TOMCRYPT_CIPHERS_AES}
  ${TOMCRYPT_HASHES}
  ${TOMCRYPT_HASHES_HELPER}
  ${TOMCRYPT_MATH}
  ${TOMCRYPT_MATH_FP}
  ${TOMCRYPT_MISC}
  ${TOMCRYPT_MISC_BASE64}
  ${TOMCRYPT_MISC_CRYPT}
  ${TOMCRYPT_MISC_PKCS5}
  ${TOMCRYPT_MODES_CBC}
  ${TOMCRYPT_MODES_CFB}
  ${TOMCRYPT_MODES_CTR}
  ${TOMCRYPT_MODES_ECB}
  ${TOMCRYPT_MODES_OFB}
  ${TOMCRYPT_PK_ASN1_DER_BIT}
  ${TOMCRYPT_PK_ASN1_DER_BOOLEAN}
  ${TOMCRYPT_PK_ASN1_DER_CHOICE}
  ${TOMCRYPT_PK_ASN1_DER_IA5}
  ${TOMCRYPT_PK_ASN1_DER_INTEGER}
  ${TOMCRYPT_PK_ASN1_DER_OBJECT_IDENTIFIER}
  ${TOMCRYPT_PK_ASN1_DER_OCTET}
  ${TOMCRYPT_PK_ASN1_DER_PRINTABLE_STRING}
  ${TOMCRYPT_PK_ASN1_DER_SEQUENCE}
  ${TOMCRYPT_PK_ASN1_DER_SET}
  ${TOMCRYPT_PK_ASN1_DER_SHORT_INTEGER}
  ${TOMCRYPT_PK_ASN1_DER_UTCTIME}
  ${TOMCRYPT_PK_ASN1_DER_UTF8}
  ${TOMCRYPT_PK_DSA}
  ${TOMCRYPT_PK_PKCS1}
  ${TOMCRYPT_PK_RSA}
  ${TOMCRYPT_PRNGS}
)

set(TOMCRYPT_HPP
  "${TOMCRYPT_DIR}/include/tomcrypt.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_argchk.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_cfg.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_cipher.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_custom.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_hash.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_mac.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_macros.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_math.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_misc.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_pk.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_pkcs.h"
  "${TOMCRYPT_DIR}/include/tomcrypt_prng.h"
)

source_group("headers" FILES ${TOMCRYPT_HPP})

add_library("tomcrypt" ${TOMCRYPT_SRC} ${TOMCRYPT_HPP})

set_property(TARGET "tomcrypt" PROPERTY FOLDER "External Libraries")

if (WITH_PORTABLE_TOMCRYPT)
  sm_add_compile_definition("tomcrypt" LTC_NO_ASM)
elseif(WITH_NO_ROLC_TOMCRYPT AND NOT APPLE)
  sm_add_compile_definition("tomcrypt" LTC_NO_ROLC)
endif()

if (APPLE)
  sm_add_compile_definition("tomcrypt" LTC_NO_ROLC)
  sm_append_simple_target_property("tomcrypt" XCODE_ATTRIBUTE_GCC_NO_COMMON_BLOCKS "YES")
  sm_append_simple_target_property("tomcrypt" XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS[variant=Debug]
  "'CMAKE_INTDIR=\"Debug\"' LTC_NO_ROLC DEBUG=1")
elseif (MSVC)
  sm_add_compile_definition("tomcrypt" _CRT_SECURE_NO_WARNINGS)
endif()

disable_project_warnings("tomcrypt")

target_include_directories("tomcrypt" PRIVATE "${TOMMATH_DIR}/include")
target_include_directories("tomcrypt" PUBLIC "${TOMCRYPT_DIR}/include")
