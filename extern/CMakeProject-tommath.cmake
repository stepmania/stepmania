set(TOMDIR "${SM_SRC_DIR}/libtommath")

list(APPEND TOMMATH_SRC
            "${TOMDIR}/bn_error.c"
            "${TOMDIR}/bn_fast_mp_invmod.c"
            "${TOMDIR}/bn_fast_mp_montgomery_reduce.c"
            "${TOMDIR}/bn_fast_s_mp_mul_digs.c"
            "${TOMDIR}/bn_fast_s_mp_mul_high_digs.c"
            "${TOMDIR}/bn_fast_s_mp_sqr.c"
            "${TOMDIR}/bn_mp_2expt.c"
            "${TOMDIR}/bn_mp_abs.c"
            "${TOMDIR}/bn_mp_add.c"
            "${TOMDIR}/bn_mp_addmod.c"
            "${TOMDIR}/bn_mp_add_d.c"
            "${TOMDIR}/bn_mp_and.c"
            "${TOMDIR}/bn_mp_clamp.c"
            "${TOMDIR}/bn_mp_clear.c"
            "${TOMDIR}/bn_mp_clear_multi.c"
            "${TOMDIR}/bn_mp_cmp.c"
            "${TOMDIR}/bn_mp_cmp_d.c"
            "${TOMDIR}/bn_mp_cmp_mag.c"
            "${TOMDIR}/bn_mp_cnt_lsb.c"
            "${TOMDIR}/bn_mp_copy.c"
            "${TOMDIR}/bn_mp_count_bits.c"
            "${TOMDIR}/bn_mp_div.c"
            "${TOMDIR}/bn_mp_div_2.c"
            "${TOMDIR}/bn_mp_div_2d.c"
            "${TOMDIR}/bn_mp_div_3.c"
            "${TOMDIR}/bn_mp_div_d.c"
            "${TOMDIR}/bn_mp_dr_is_modulus.c"
            "${TOMDIR}/bn_mp_dr_reduce.c"
            "${TOMDIR}/bn_mp_dr_setup.c"
            "${TOMDIR}/bn_mp_exch.c"
            "${TOMDIR}/bn_mp_expt_d.c"
            "${TOMDIR}/bn_mp_exptmod.c"
            "${TOMDIR}/bn_mp_exptmod_fast.c"
            "${TOMDIR}/bn_mp_exteuclid.c"
            "${TOMDIR}/bn_mp_fread.c"
            "${TOMDIR}/bn_mp_fwrite.c"
            "${TOMDIR}/bn_mp_gcd.c"
            "${TOMDIR}/bn_mp_get_int.c"
            "${TOMDIR}/bn_mp_grow.c"
            "${TOMDIR}/bn_mp_init.c"
            "${TOMDIR}/bn_mp_init_copy.c"
            "${TOMDIR}/bn_mp_init_multi.c"
            "${TOMDIR}/bn_mp_init_set.c"
            "${TOMDIR}/bn_mp_init_set_int.c"
            "${TOMDIR}/bn_mp_init_size.c"
            "${TOMDIR}/bn_mp_invmod.c"
            "${TOMDIR}/bn_mp_invmod_slow.c"
            "${TOMDIR}/bn_mp_is_square.c"
            "${TOMDIR}/bn_mp_jacobi.c"
            "${TOMDIR}/bn_mp_karatsuba_mul.c"
            "${TOMDIR}/bn_mp_karatsuba_sqr.c"
            "${TOMDIR}/bn_mp_lcm.c"
            "${TOMDIR}/bn_mp_lshd.c"
            "${TOMDIR}/bn_mp_mod.c"
            "${TOMDIR}/bn_mp_mod_2d.c"
            "${TOMDIR}/bn_mp_mod_d.c"
            "${TOMDIR}/bn_mp_montgomery_calc_normalization.c"
            "${TOMDIR}/bn_mp_montgomery_reduce.c"
            "${TOMDIR}/bn_mp_montgomery_setup.c"
            "${TOMDIR}/bn_mp_mul.c"
            "${TOMDIR}/bn_mp_mul_2.c"
            "${TOMDIR}/bn_mp_mul_2d.c"
            "${TOMDIR}/bn_mp_mul_d.c"
            "${TOMDIR}/bn_mp_mulmod.c"
            "${TOMDIR}/bn_mp_n_root.c"
            "${TOMDIR}/bn_mp_neg.c"
            "${TOMDIR}/bn_mp_or.c"
            "${TOMDIR}/bn_mp_prime_fermat.c"
            "${TOMDIR}/bn_mp_prime_is_divisible.c"
            "${TOMDIR}/bn_mp_prime_is_prime.c"
            "${TOMDIR}/bn_mp_prime_miller_rabin.c"
            "${TOMDIR}/bn_mp_prime_next_prime.c"
            "${TOMDIR}/bn_mp_prime_rabin_miller_trials.c"
            "${TOMDIR}/bn_mp_prime_random_ex.c"
            "${TOMDIR}/bn_mp_radix_size.c"
            "${TOMDIR}/bn_mp_radix_smap.c"
            "${TOMDIR}/bn_mp_rand.c"
            "${TOMDIR}/bn_mp_read_radix.c"
            "${TOMDIR}/bn_mp_read_signed_bin.c"
            "${TOMDIR}/bn_mp_read_unsigned_bin.c"
            "${TOMDIR}/bn_mp_reduce.c"
            "${TOMDIR}/bn_mp_reduce_2k.c"
            "${TOMDIR}/bn_mp_reduce_2k_setup.c"
            "${TOMDIR}/bn_mp_reduce_2k_setup_l.c"
            "${TOMDIR}/bn_mp_reduce_2k_l.c"
            "${TOMDIR}/bn_mp_reduce_is_2k.c"
            "${TOMDIR}/bn_mp_reduce_is_2k_l.c"
            "${TOMDIR}/bn_mp_reduce_setup.c"
            "${TOMDIR}/bn_mp_rshd.c"
            "${TOMDIR}/bn_mp_set.c"
            "${TOMDIR}/bn_mp_set_int.c"
            "${TOMDIR}/bn_mp_shrink.c"
            "${TOMDIR}/bn_mp_signed_bin_size.c"
            "${TOMDIR}/bn_mp_sqr.c"
            "${TOMDIR}/bn_mp_sqrmod.c"
            "${TOMDIR}/bn_mp_sqrt.c"
            "${TOMDIR}/bn_mp_sub.c"
            "${TOMDIR}/bn_mp_sub_d.c"
            "${TOMDIR}/bn_mp_submod.c"
            "${TOMDIR}/bn_mp_to_signed_bin.c"
            "${TOMDIR}/bn_mp_to_signed_bin_n.c"
            "${TOMDIR}/bn_mp_to_unsigned_bin.c"
            "${TOMDIR}/bn_mp_to_unsigned_bin_n.c"
            "${TOMDIR}/bn_mp_toom_mul.c"
            "${TOMDIR}/bn_mp_toom_sqr.c"
            "${TOMDIR}/bn_mp_toradix.c"
            "${TOMDIR}/bn_mp_toradix_n.c"
            "${TOMDIR}/bn_mp_unsigned_bin_size.c"
            "${TOMDIR}/bn_mp_xor.c"
            "${TOMDIR}/bn_mp_zero.c"
            "${TOMDIR}/bn_prime_tab.c"
            "${TOMDIR}/bn_reverse.c"
            "${TOMDIR}/bn_s_mp_add.c"
            "${TOMDIR}/bn_s_mp_exptmod.c"
            "${TOMDIR}/bn_s_mp_mul_digs.c"
            "${TOMDIR}/bn_s_mp_mul_high_digs.c"
            "${TOMDIR}/bn_s_mp_sqr.c"
            "${TOMDIR}/bn_s_mp_sub.c"
            "${TOMDIR}/bncore.c")

list(APPEND TOMMATH_HPP
            "${TOMDIR}/tommath.h"
            "${TOMDIR}/tommath_class.h"
            "${TOMDIR}/tommath_superclass.h")

source_group("" FILES ${TOMMATH_SRC})
source_group("" FILES ${TOMMATH_HPP})

add_library("tommath" STATIC ${TOMMATH_SRC} ${TOMMATH_HPP})

set_property(TARGET "tommath" PROPERTY FOLDER "External Libraries")

disable_project_warnings("tommath")

target_include_directories("tommath" PUBLIC ${TOMDIR})
