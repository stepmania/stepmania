include_directories(src/headers)

if(MSVC)
	add_definitions(/D _CRT_SECURE_NO_WARNINGS)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("tomcrypt")
endif(MSVC)

list(APPEND TOMCRYPT_MISC_CRYPT
	"src/misc/crypt/crypt.c"
	"src/misc/crypt/crypt_argchk.c"
	"src/misc/crypt/crypt_cipher_descriptor.c"
	"src/misc/crypt/crypt_cipher_is_valid.c"
	"src/misc/crypt/crypt_find_cipher.c"
	"src/misc/crypt/crypt_find_cipher_any.c"
	"src/misc/crypt/crypt_find_cipher_id.c"
	"src/misc/crypt/crypt_find_hash.c"
	"src/misc/crypt/crypt_find_hash_any.c"
	"src/misc/crypt/crypt_find_hash_id.c"
	"src/misc/crypt/crypt_find_hash_oid.c"
	"src/misc/crypt/crypt_find_prng.c"
	"src/misc/crypt/crypt_fsa.c"
	"src/misc/crypt/crypt_hash_descriptor.c"
	"src/misc/crypt/crypt_hash_is_valid.c"
	"src/misc/crypt/crypt_ltc_mp_descriptor.c"
	"src/misc/crypt/crypt_prng_descriptor.c"
	"src/misc/crypt/crypt_prng_is_valid.c"
	"src/misc/crypt/crypt_register_cipher.c"
	"src/misc/crypt/crypt_register_hash.c"
	"src/misc/crypt/crypt_register_prng.c"
	"src/misc/crypt/crypt_unregister_cipher.c"
	"src/misc/crypt/crypt_unregister_hash.c"
	"src/misc/crypt/crypt_unregister_prng.c"
)

source_group("src\\\\misc\\\\crypt" FILES ${TOMCRYPT_MISC_CRYPT})

set(TOMCRYPT_CIPHERS_AES
	"src/ciphers/aes/aes.c"
)

source_group("src\\\\ciphers\\\\aes" FILES ${TOMCRYPT_CIPHERS_AES})

set(TOMCRYPT_HASHES
	"src/hashes/md5.c"
	"src/hashes/sha1.c"
	"src/hashes/helper/hash_memory.c"
)

source_group("src\\\\hashes" FILES ${TOMCRYPT_HASHES})

list(APPEND TOMCRYPT_MISC
	"src/misc/burn_stack.c"
	"src/misc/error_to_string.c"
	"src/misc/zeromem.c"
)

source_group("src\\\\misc" FILES ${TOMCRYPT_MISC})

set(TOMCRYPT_MISC_BASE64
	"src/misc/base64/base64_decode.c"
	"src/misc/base64/base64_encode.c"
)

source_group("src\\\\misc\\\\base64" FILES ${TOMCRYPT_MISC_BASE64})

set(TOMCRYPT_MISC_PKCS5
	"src/misc/pkcs5/pkcs_5_1.c"
	"src/misc/pkcs5/pkcs_5_2.c"
)

source_group("src\\\\misc\\\\pkcs5" FILES ${TOMCRYPT_MISC_PKCS5})

set(TOMCRYPT_MATH
	"src/math/ltm_desc.c"
	"src/math/fp/ltc_ecc_fp_mulmod.c"
	"src/math/multi.c"
	"src/math/rand_prime.c"
)

source_group("src\\\\math" FILES ${TOMCRYPT_MATH})

list(APPEND TOMCRYPT_PRNGS
	"src/prngs/fortuna.c"
	"src/prngs/rc4.c"
	"src/prngs/rng_get_bytes.c"
	"src/prngs/rng_make_prng.c"
	"src/prngs/sprng.c"
	"src/prngs/yarrow.c"
)

source_group("src\\\\prngs" FILES ${TOMCRYPT_PRNGS})

list(APPEND TOMCRYPT_MODES_CBC
	"src/modes/cbc/cbc_decrypt.c"
	"src/modes/cbc/cbc_done.c"
	"src/modes/cbc/cbc_encrypt.c"
	"src/modes/cbc/cbc_getiv.c"
	"src/modes/cbc/cbc_setiv.c"
	"src/modes/cbc/cbc_start.c"
)

source_group("src\\\\modes\\\\cbc" FILES ${TOMCRYPT_MODES_CBC})

list(APPEND TOMCRYPT_MODES_CFB
	"src/modes/cfb/cfb_decrypt.c"
	"src/modes/cfb/cfb_done.c"
	"src/modes/cfb/cfb_encrypt.c"
	"src/modes/cfb/cfb_getiv.c"
	"src/modes/cfb/cfb_setiv.c"
	"src/modes/cfb/cfb_start.c"
)
source_group("src\\\\modes\\\\cfb" FILES ${TOMCRYPT_MODES_CFB})

list(APPEND TOMCRYPT_MODES_CTR
	"src/modes/ctr/ctr_decrypt.c"
	"src/modes/ctr/ctr_done.c"
	"src/modes/ctr/ctr_encrypt.c"
	"src/modes/ctr/ctr_getiv.c"
	"src/modes/ctr/ctr_setiv.c"
	"src/modes/ctr/ctr_start.c"
	"src/modes/ctr/ctr_test.c"
)

source_group("src\\\\modes\\\\ctr" FILES ${TOMCRYPT_MODES_CTR})

list(APPEND TOMCRYPT_MODES_ECB
	"src/modes/ecb/ecb_decrypt.c"
	"src/modes/ecb/ecb_done.c"
	"src/modes/ecb/ecb_encrypt.c"
	"src/modes/ecb/ecb_start.c"
)

source_group("src\\\\modes\\\\ecb" FILES ${TOMCRYPT_MODES_ECB})

list(APPEND TOMCRYPT_MODES_OFB
	"src/modes/ofb/ofb_decrypt.c"
	"src/modes/ofb/ofb_done.c"
	"src/modes/ofb/ofb_encrypt.c"
	"src/modes/ofb/ofb_getiv.c"
	"src/modes/ofb/ofb_setiv.c"
	"src/modes/ofb/ofb_start.c"
)

source_group("src\\\\modes\\\\ofb" FILES ${TOMCRYPT_MODES_OFB})

list(APPEND TOMCRYPT_PK_RSA
	"src/pk/rsa/rsa_decrypt_key.c"
	"src/pk/rsa/rsa_encrypt_key.c"
	"src/pk/rsa/rsa_export.c"
	"src/pk/rsa/rsa_exptmod.c"
	"src/pk/rsa/rsa_free.c"
	"src/pk/rsa/rsa_import.c"
	"src/pk/rsa/rsa_make_key.c"
	"src/pk/rsa/rsa_sign_hash.c"
	"src/pk/rsa/rsa_verify_hash.c"
)

source_group("src\\\\pk\\\\rsa" FILES ${TOMCRYPT_PK_RSA})

list(APPEND TOMCRYPT_PK_PKCS1
	"src/pk/pkcs1/pkcs_1_i2osp.c"
	"src/pk/pkcs1/pkcs_1_mgf1.c"
	"src/pk/pkcs1/pkcs_1_oaep_decode.c"
	"src/pk/pkcs1/pkcs_1_oaep_encode.c"
	"src/pk/pkcs1/pkcs_1_os2ip.c"
	"src/pk/pkcs1/pkcs_1_pss_decode.c"
	"src/pk/pkcs1/pkcs_1_pss_encode.c"
	"src/pk/pkcs1/pkcs_1_v1_5_decode.c"
	"src/pk/pkcs1/pkcs_1_v1_5_encode.c"
)

source_group("src\\\\pk\\\\pkcs1" FILES ${TOMCRYPT_PK_PKCS1})

list(APPEND TOMCRYPT_PK_DSA
	"src/pk/dsa/dsa_decrypt_key.c"
	"src/pk/dsa/dsa_encrypt_key.c"
	"src/pk/dsa/dsa_export.c"
	"src/pk/dsa/dsa_free.c"
	"src/pk/dsa/dsa_import.c"
	"src/pk/dsa/dsa_make_key.c"
	"src/pk/dsa/dsa_shared_secret.c"
	"src/pk/dsa/dsa_sign_hash.c"
	"src/pk/dsa/dsa_verify_hash.c"
	"src/pk/dsa/dsa_verify_key.c"
)

source_group("src\\\\pk\\\\dsa" FILES ${TOMCRYPT_PK_DSA})

list(APPEND TOMCRYPT_PK_DER
	"src/pk/asn1/der/bit/der_decode_bit_string.c"
	"src/pk/asn1/der/bit/der_encode_bit_string.c"
	"src/pk/asn1/der/bit/der_length_bit_string.c"
	"src/pk/asn1/der/boolean/der_decode_boolean.c"
	"src/pk/asn1/der/boolean/der_encode_boolean.c"
	"src/pk/asn1/der/boolean/der_length_boolean.c"
	"src/pk/asn1/der/choice/der_decode_choice.c"
	"src/pk/asn1/der/ia5/der_decode_ia5_string.c"
	"src/pk/asn1/der/ia5/der_encode_ia5_string.c"
	"src/pk/asn1/der/ia5/der_length_ia5_string.c"
	"src/pk/asn1/der/integer/der_decode_integer.c"
	"src/pk/asn1/der/integer/der_encode_integer.c"
	"src/pk/asn1/der/integer/der_length_integer.c"
	"src/pk/asn1/der/object_identifier/der_decode_object_identifier.c"
	"src/pk/asn1/der/object_identifier/der_encode_object_identifier.c"
	"src/pk/asn1/der/object_identifier/der_length_object_identifier.c"
	"src/pk/asn1/der/octet/der_decode_octet_string.c"
	"src/pk/asn1/der/octet/der_encode_octet_string.c"
	"src/pk/asn1/der/octet/der_length_octet_string.c"
	"src/pk/asn1/der/printable_string/der_decode_printable_string.c"
	"src/pk/asn1/der/printable_string/der_encode_printable_string.c"
	"src/pk/asn1/der/printable_string/der_length_printable_string.c"
	"src/pk/asn1/der/sequence/der_decode_sequence_ex.c"
	"src/pk/asn1/der/sequence/der_decode_sequence_flexi.c"
	"src/pk/asn1/der/sequence/der_decode_sequence_multi.c"
	"src/pk/asn1/der/sequence/der_encode_sequence_ex.c"
	"src/pk/asn1/der/sequence/der_encode_sequence_multi.c"
	"src/pk/asn1/der/sequence/der_length_sequence.c"
	"src/pk/asn1/der/sequence/der_sequence_free.c"
	"src/pk/asn1/der/set/der_encode_set.c"
	"src/pk/asn1/der/set/der_encode_setof.c"
	"src/pk/asn1/der/short_integer/der_decode_short_integer.c"
	"src/pk/asn1/der/short_integer/der_encode_short_integer.c"
	"src/pk/asn1/der/short_integer/der_length_short_integer.c"
	"src/pk/asn1/der/utctime/der_decode_utctime.c"
	"src/pk/asn1/der/utctime/der_encode_utctime.c"
	"Src/pk/asn1/der/utctime/der_length_utctime.c"
	"src/pk/asn1/der/utf8/der_decode_utf8_string.c"
	"src/pk/asn1/der/utf8/der_encode_utf8_string.c"
	"src/pk/asn1/der/utf8/der_length_utf8_string.c"
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
	"src/headers/tomcrypt.h"
	"src/headers/tomcrypt_argchk.h"
	"src/headers/tomcrypt_cfg.h"
	"src/headers/tomcrypt_cipher.h"
	"src/headers/tomcrypt_custom.h"
	"src/headers/tomcrypt_hash.h"
	"src/headers/tomcrypt_mac.h"
	"src/headers/tomcrypt_macros.h"
	"src/headers/tomcrypt_math.h"
	"src/headers/tomcrypt_misc.h"
	"src/headers/tomcrypt_pk.h"
	"src/headers/tomcrypt_pkcs.h"
	"src/headers/tomcrypt_prng.h"
)

source_group("headers" FILES ${TOMCRYPT_HPP})

add_library("tomcrypt" ${TOMCRYPT_SRC} ${TOMCRYPT_HPP})

set_property(TARGET "tomcrypt" PROPERTY FOLDER "External Libraries")