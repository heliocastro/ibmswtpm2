/********************************************************************************/
/*										*/
/*	 		TPM to OpenSSL BigNum Shim Layer			*/
/*			     Written by Ken Goldman				*/
/*		       IBM Thomas J. Watson Research Center			*/
/*            $Id: TpmToOsslMath.h 1519 2019-11-15 20:43:51Z kgoldman $		*/
/*										*/
/*  Licenses and Notices							*/
/*										*/
/*  1. Copyright Licenses:							*/
/*										*/
/*  - Trusted Computing Group (TCG) grants to the user of the source code in	*/
/*    this specification (the "Source Code") a worldwide, irrevocable, 		*/
/*    nonexclusive, royalty free, copyright license to reproduce, create 	*/
/*    derivative works, distribute, display and perform the Source Code and	*/
/*    derivative works thereof, and to grant others the rights granted herein.	*/
/*										*/
/*  - The TCG grants to the user of the other parts of the specification 	*/
/*    (other than the Source Code) the rights to reproduce, distribute, 	*/
/*    display, and perform the specification solely for the purpose of 		*/
/*    developing products based on such documents.				*/
/*										*/
/*  2. Source Code Distribution Conditions:					*/
/*										*/
/*  - Redistributions of Source Code must retain the above copyright licenses, 	*/
/*    this list of conditions and the following disclaimers.			*/
/*										*/
/*  - Redistributions in binary form must reproduce the above copyright 	*/
/*    licenses, this list of conditions	and the following disclaimers in the 	*/
/*    documentation and/or other materials provided with the distribution.	*/
/*										*/
/*  3. Disclaimers:								*/
/*										*/
/*  - THE COPYRIGHT LICENSES SET FORTH ABOVE DO NOT REPRESENT ANY FORM OF	*/
/*  LICENSE OR WAIVER, EXPRESS OR IMPLIED, BY ESTOPPEL OR OTHERWISE, WITH	*/
/*  RESPECT TO PATENT RIGHTS HELD BY TCG MEMBERS (OR OTHER THIRD PARTIES)	*/
/*  THAT MAY BE NECESSARY TO IMPLEMENT THIS SPECIFICATION OR OTHERWISE.		*/
/*  Contact TCG Administration (admin@trustedcomputinggroup.org) for 		*/
/*  information on specification licensing rights available through TCG 	*/
/*  membership agreements.							*/
/*										*/
/*  - THIS SPECIFICATION IS PROVIDED "AS IS" WITH NO EXPRESS OR IMPLIED 	*/
/*    WARRANTIES WHATSOEVER, INCLUDING ANY WARRANTY OF MERCHANTABILITY OR 	*/
/*    FITNESS FOR A PARTICULAR PURPOSE, ACCURACY, COMPLETENESS, OR 		*/
/*    NONINFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS, OR ANY WARRANTY 		*/
/*    OTHERWISE ARISING OUT OF ANY PROPOSAL, SPECIFICATION OR SAMPLE.		*/
/*										*/
/*  - Without limitation, TCG and its members and licensors disclaim all 	*/
/*    liability, including liability for infringement of any proprietary 	*/
/*    rights, relating to use of information in this specification and to the	*/
/*    implementation of this specification, and TCG disclaims all liability for	*/
/*    cost of procurement of substitute goods or services, lost profits, loss 	*/
/*    of use, loss of data or any incidental, consequential, direct, indirect, 	*/
/*    or special damages, whether under contract, tort, warranty or otherwise, 	*/
/*    arising in any way out of use or reliance upon this specification or any 	*/
/*    information herein.							*/
/*										*/
/*  (c) Copyright IBM Corp. and others, 2016 - 2019				*/
/*										*/
/********************************************************************************/

/* B.2.2.1. TpmToOsslMath.h */
/* B.2.2.1.1. Introduction */
/* This file contains the structure definitions used for ECC in the LibTopCrypt() version of the
   code. These definitions would change, based on the library. The ECC-related structures that cross
   the TPM interface are defined in TpmTypes.h */

#ifndef MATH_LIB_DEFINED
#define MATH_LIB_DEFINED
#define MATH_LIB_OSSL
#include <openssl/evp.h>
#include <openssl/ec.h>

#if OPENSSL_VERSION_NUMBER >= 0x10200000L
// Check the bignum_st definition in crypto/bn/bn_lcl.h and either update the
// version check or provide the new definition for this version.
#   error Untested OpenSSL version
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
// from crypto/bn/bn_lcl.h
struct bignum_st {
    BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit
				 * chunks. */
    int top;                    /* Index of last used d +1. */
    /* The next are internal book keeping for bn_expand. */
    int dmax;                   /* Size of the d array. */
    int neg;                    /* one if the number is negative */
    int flags;
};
#endif // OPENSSL_VERSION_NUMBER

#include <openssl/bn.h>

/* B.2.2.2.2. Macros and Defines */
/* Make sure that the library is using the correct size for a crypt word */

#if    defined THIRTY_TWO_BIT && (RADIX_BITS != 32)			\
    || ((defined SIXTY_FOUR_BIT_LONG || defined SIXTY_FOUR_BIT)		\
	&& (RADIX_BITS != 64))
#   error Ossl library is using different radix
#endif

/* Allocate a local BIGNUM value. For the allocation, a bigNum structure is created as is a local
   BIGNUM. The bigNum is initialized and then the BIGNUM is set to reference the local value. */

#define BIG_VAR(name, bits)						\
    BN_VAR(name##Bn, (bits));						\
    BIGNUM          _##name;						\
    BIGNUM          *name = BigInitialized(&_##name,			\
					   BnInit(name##Bn,		\
						  BYTES_TO_CRYPT_WORDS(sizeof(_##name##Bn.d))))

/* Allocate a BIGNUM and initialize with the values in a bigNum initializer */

#define BIG_INITIALIZED(name, initializer)				\
    BIGNUM           _##name;						\
    BIGNUM          *name = BigInitialized(&_##name, initializer)

typedef struct
{
    const ECC_CURVE_DATA    *C;     // the TPM curve values
    EC_GROUP                *G;     // group parameters
    BN_CTX                  *CTX;   // the context for the math (this might not be
    // the context in which the curve was created>;
} OSSL_CURVE_DATA;
typedef OSSL_CURVE_DATA      *bigCurve;
#define AccessCurveData(E)  ((E)->C)

#include "TpmToOsslSupport_fp.h"

#define OSSL_ENTER()     BN_CTX      *CTX = OsslContextEnter()
#define OSSL_LEAVE()     OsslContextLeave(CTX)

/* Start and end a context that spans multiple ECC functions. This is used so that the group for the
   curve can persist across multiple frames. */

#define CURVE_INITIALIZED(name, initializer)				\
    OSSL_CURVE_DATA     _##name;					\
    bigCurve            name =  BnCurveInitialize(&_##name, initializer)

#define CURVE_FREE(name)               BnCurveFree(name)

/* Start and end a local stack frame within the context of the curve frame */
#define ECC_ENTER()     BN_CTX         *CTX = OsslPushContext(E->CTX)
#define ECC_LEAVE()     OsslPopContext(CTX)
#define BN_NEW()        BnNewVariable(CTX)


/* This definition would change if there were something to report */
#define MathLibSimulationEnd()
#endif // MATH_LIB_DEFINED

