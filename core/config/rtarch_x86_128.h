/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_128_H
#define RT_RTARCH_X86_128_H

#include "rtarch_x86.h"

#define RT_SIMD_REGS        8
#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

#undef  mxcsr_ld

#if defined (RT_128) && (RT_128 != 0)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86_128.h: Implementation of x86:i386 SSE2-128 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdp*_ri - applies [cmd] to [p]acked: [r]egister from [i]mmediate
 * cmdp*_rr - applies [cmd] to [p]acked: [r]egister from [r]egister
 *
 * cmdp*_rm - applies [cmd] to [p]acked: [r]egister from [m]emory
 * cmdp*_ld - applies [cmd] to [p]acked: as above
 *
 * cmdpx_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmdpn_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmdps_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * The cmdp*_** instructions are intended for SPMD programming model
 * and can potentially be configured per target to work with 32-bit/64-bit
 * data-elements (integers/pointers, floating point).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* mandatory escape prefix for some opcodes */
#define ESC                                                                 \
        EMITB(0x66)

/* mandatory escape prefix for some opcodes */
#define xF3                                                                 \
        EMITB(0xF3)

/* fwait instruction for legacy processors (fix for fstcw) */
#define FWT                                                                 \
        EMITB(0x9B)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x03, EMPTY
#define Xmm1    0x01, 0x03, EMPTY
#define Xmm2    0x02, 0x03, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x04, 0x03, EMPTY
#define Xmm5    0x05, 0x03, EMPTY
#define Xmm6    0x06, 0x03, EMPTY
#define Xmm7    0x07, 0x03, EMPTY

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE1)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movpx_st(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
        EMITB(0x8D)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define annpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/**************   packed single precision floating point (SSE1)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define divps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define sqrps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x53)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        addps_rr(W(RG), W(RG))                                              \
        subps_rr(W(RG), W(RM))

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x52)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        subps_ld(W(RM), Mebp, inf_GPC03)                                    \
        mulps_ld(W(RM), Mebp, inf_GPC02)                                    \
        mulps_rr(W(RG), W(RM))

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define minps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define maxps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (SSE1)   *************************/

#if (RT_128 < 2)

#define fpuxs_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xD9)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxs_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xD9)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxn_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xDB)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxn_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xDB)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xD9)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_st(RM, DP) /* not portable, do not use outside */             \
    FWT EMITB(0xD9)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        movxx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnzps_rr(RG, RM)     /* round towards zero */                       \
        cvzps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnzps_ld(RG, RM, DP) /* round towards zero */                       \
        cvzps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
        fpurz_xx()                                                          \
        cvnps_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
        fpurz_xx()                                                          \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        movxx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnpps_rr(RG, RM)     /* round towards +inf */                       \
        cvpps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnpps_ld(RG, RM, DP) /* round towards +inf */                       \
        cvpps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvpps_rr(RG, RM)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnps_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvpps_ld(RG, RM, DP) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        movxx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnmps_rr(RG, RM)     /* round towards -inf */                       \
        cvmps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnmps_ld(RG, RM, DP) /* round towards -inf */                       \
        cvmps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvmps_rr(RG, RM)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnps_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvmps_ld(RG, RM, DP) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR00)

#define rnnps_rr(RG, RM)     /* round towards near */                       \
        cvnps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnnps_ld(RG, RM, DP) /* round towards near */                       \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvnps_rr(RG, RM)     /* round towards near */                       \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuxs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x0C))                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnps_ld(RG, RM, DP) /* round towards near */                       \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvnps_rr(W(RG), W(RG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round towards near */                       \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuxn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuxn_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuxn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuxn_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x0C))                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnpn_ld(RG, RM, DP) /* round towards near */                       \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

/* add */

#define addpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_st(W(RM), Mebp, inf_SCR02(0))                                 \
        movxx_st(Reax,  Mebp, inf_SCR00)                                    \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movxx_ld(Reax,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define addpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        movxx_st(Reax,  Mebp, inf_SCR00)                                    \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addxx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movxx_ld(Reax,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_st(W(RM), Mebp, inf_SCR02(0))                                 \
        movxx_st(Reax,  Mebp, inf_SCR00)                                    \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movxx_ld(Reax,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define subpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        movxx_st(Reax,  Mebp, inf_SCR00)                                    \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movxx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subxx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movxx_ld(Reax,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shlxx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shlxx_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shlxx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shlxx_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shlpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movxx_st(Recx,  Mebp, inf_SCR00)                                    \
        movxx_ld(Recx,  W(RM), W(DP))                                       \
        shlxx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlxx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlxx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlxx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movxx_ld(Recx,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrxx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrxx_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shrxx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrxx_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movxx_st(Recx,  Mebp, inf_SCR00)                                    \
        movxx_ld(Recx,  W(RM), W(DP))                                       \
        shrxx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrxx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrxx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrxx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movxx_ld(Recx,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define shrpn_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrxn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrxn_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shrxn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrxn_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrpn_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movxx_st(Recx,  Mebp, inf_SCR00)                                    \
        movxx_ld(Recx,  W(RM), W(DP))                                       \
        shrxn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrxn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrxn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrxn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movxx_ld(Recx,  Mebp, inf_SCR00)                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/**************************   packed integer (SSE2)   *************************/

#else /* RT_128 >= 2 */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzps_rr(RG, RM)     /* round towards zero */                       \
        cvzps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnzps_ld(RG, RM, DP) /* round towards zero */                       \
        cvzps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
    xF3 EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
    xF3 EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define rnpps_rr(RG, RM)     /* round towards +inf */                       \
        cvpps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnpps_ld(RG, RM, DP) /* round towards +inf */                       \
        cvpps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvpps_rr(RG, RM)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpps_ld(RG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtps_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define rnmps_rr(RG, RM)     /* round towards -inf */                       \
        cvmps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnmps_ld(RG, RM, DP) /* round towards -inf */                       \
        cvmps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvmps_rr(RG, RM)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmps_ld(RG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtps_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define rnnps_rr(RG, RM)     /* round towards near */                       \
        cvnps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnnps_ld(RG, RM, DP) /* round towards near */                       \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvnps_rr(RG, RM)     /* round towards near */                       \
        cvtps_rr(W(RG), W(RM))

#define cvnps_ld(RG, RM, DP) /* round towards near */                       \
        cvtps_ld(W(RG), W(RM), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round towards near */                       \
        cvtpn_rr(W(RG), W(RM))

#define cvnpn_ld(RG, RM, DP) /* round towards near */                       \
        cvtpn_ld(W(RG), W(RM), W(DP))

/* add */

#define addpx_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0xFE)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addpx_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xFE)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subpx_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0xFA)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subpx_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xFA)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlpx_ri(RM, IM)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlpx_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xF2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrpx_ri(RM, IM)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrpx_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xD2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpn_ri(RM, IM)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrpn_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xE2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 2 */

/**************************   helper macros (SSE1)   **************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x0F    /*  all satisfy the condition */

#define movsn_rr(RG, RM) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0x50)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movsn_rr(Reax, W(RG))                                               \
        cmpxx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jeqxx_lb(lb)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode,
 * original FCTRL blocks are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        movxx_mi(Mebp, inf_SCR00, IH(RT_SIMD_MODE_##mode << 13 | 0x1F80))   \
        mxcsr_ld(Mebp, inf_SCR00)                                           \

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#if (RT_128 < 2)

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz */

#define rndps_rr(RG, RM)                                                    \
        cvtps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rndps_ld(RG, RM, DP)                                                \
        cvtps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvtps_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrxx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andxx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrxx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnps_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR00)

#define cvtps_ld(RG, RM, DP)                                                \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvtps_rr(W(RG), W(RG))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrxx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andxx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrxx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnpn_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR00)

#define cvtpn_ld(RG, RM, DP)                                                \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvtpn_rr(W(RG), W(RG))

#else /* RT_128 >= 2 */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz */

#define rndps_rr(RG, RM)                                                    \
        cvtps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rndps_ld(RG, RM, DP)                                                \
        cvtps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvtps_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvtps_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvtpn_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 2 */

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block */

#define rnrps_rr(RG, RM, mode)                                              \
        cvrps_rr(W(RG), W(RM), mode)                                        \
        cvnpn_rr(W(RG), W(RG))

#define cvrps_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
