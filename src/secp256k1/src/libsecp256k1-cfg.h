#pragma once

#ifdef _MSC_VER
#	include <vc-inc.h>
#endif

#ifndef _MSC_VER
#	define EXT_GMP_HEADER <gmp.h>
#	define EXT_GMP_LIB "gmp"
#else
#	define EXT_GMP_HEADER <mpir.h>
#	define EXT_GMP_LIB "mpir"
#endif

#define USE_NUM_GMP
#define USE_FIELD_GMP
#define USE_FIELD_INV_BUILTIN
#define USE_SCALAR_INV_NUM

#if UCFG_64 && (!UCFG_MSC_VERSION || defined(USE_ASM_X86_64))
#	define USE_FIELD_5X52 1
#	if UCFG_MSC_VERSION
#		define USE_SCALAR_8X32 1
#	else
#		define HAVE___INT128 1
#		define USE_SCALAR_4X64 1
#	endif
#else
#	define USE_FIELD_10X26 1
#	define USE_SCALAR_8X32 1
#endif


#pragma warning(disable: 4146 4242 4244 4505)

//SFX
#define ENABLE_MODULE_ECDH
//#define ENABLE_MODULE_SCHNORR
#define ENABLE_MODULE_RECOVERY
