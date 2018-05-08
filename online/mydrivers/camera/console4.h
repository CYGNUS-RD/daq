// console4.h

// ----------------------------------------------------------------

#ifdef _WIN32

// Windows

#include	<windows.h>

#elif defined( LINUX )

// Linux

#include	<string.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<pthread.h>

#elif defined( MACOSX ) || __ppc64__ || __i386__ || __x86_64__

// Mac

// No carbon

#endif


// common headers

#include	<stdio.h>

// DCAM-API headers

#ifndef _NO_DCAMAPI

#ifndef DCAMAPI_VER
#define	DCAMAPI_VER		4000
#endif

#ifndef DCAMAPI_VERMIN
#define	DCAMAPI_VERMIN	4000
#endif

//#if	defined( LINUX )
#include			"dcamapi.h"
#include			"dcamprop.h"
//#else
//#include			"../../inc/dcamapi.h"
//#include			"../../inc/dcamprop.h"
//#endif

#if defined( _WIN64 )
#pragma comment(lib,"../../../lib/win64/dcamapi.lib")
#elif defined( _WIN32 )
#pragma comment(lib,"../../../lib/win32/dcamapi.lib")
#endif

#endif // _NO_DCAMAPI

// ----------------

#ifdef _USE_DCIMGAPI

#include			"../../inc/dcimgapi.h"

#if defined( _WIN64 )
#pragma comment(lib,"../../../lib/win64/dcimgapi.lib")
#elif defined( _WIN32 )
#pragma comment(lib,"../../../lib/win32/dcimgapi.lib")
#endif

#endif // _USE_DCIMGAPI

// ----------------------------------------------------------------

// define common macro

#ifndef ASSERT
#define	ASSERT(c)
#endif

// absorb different function

#ifdef _WIN32

#define	strcmpi	_strcmpi

#if defined(UNICODE) || defined(_UNICODE)
#define	_T(str)	L##str
#else
#define	_T(str)	str
#endif

#elif defined( MACOSX ) || __ppc64__ || __i386__ || __x86_64__ || defined( LINUX )

#define	strcmpi	strcasecmp
#define	_T(str)	str

#endif

// absorb Visual Studio 2005 and later

#if defined( WIN32 ) && _MSC_VER >= 1400

#define	_secure_buf(buf)		buf,sizeof( buf )
#define	_secure_ptr(ptr,size)	ptr,size

#else

#define	memcpy_s				memcpy
#define	sprintf_s				sprintf
#define	strcat_s				strcat
#define	_secure_buf(buf)		buf
#define	_secure_ptr(ptr,size)	ptr

#endif

// ----------------------------------------------------------------
