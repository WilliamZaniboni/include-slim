/* Copyright 2003-2017 GBDI-ICMC-USP <caetano@icmc.usp.br>
* 
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
* @file
*
* This file includes all common headers used by this library. It will help the portability
* of this code.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/                                     
#ifndef __STCOMMON_H
#define __STCOMMON_H

// All compilers
#include <stdlib.h>

// GNU GCC
#ifdef __GNUG__
   #include <string.h> // malloc & cia
   
   // This is an useful function extracted from Windows C Runtime library.
   #define random(x) ((int)((((double)rand()) / ((double)RAND_MAX)) * (x)))
   
	// Mingw does not provide these values so, here they are.
	#ifdef __MINGW32_VERSION
	   #define MAXSHORT    0x7fff
	   #define MAXINT      0x7fffffff
	   #define MAXLONG     0x7fffffff
	   #define MAXDOUBLE   1.7976931348623158E+308
	   #define MAXFLOAT    3.40282347E+38F
	   #define MINDOUBLE   2.2250738585072014E-308
	   #define MINFLOAT    1.17549435E-38F
	   #define MAXLDOUBLE  1.1897314953572317649E+4932L
	   #define MINLDOUBLE  3.362103143112094E-4917L
	#else	
		#ifdef __CYGWIN__
	   	#define MAXSHORT    0x7fff
	   	#define MAXINT      0x7fffffff
	   	#define MAXLONG     0x7fffffff
	   	#define MAXDOUBLE   1.7976931348623158E+308
	   	#ifndef MAXFLOAT
            #define MAXFLOAT    3.40282347E+38F
         #endif
	   	#define MINDOUBLE   2.2250738585072014E-308
	   	#define MINFLOAT    1.17549435E-38F
		   #define MAXLDOUBLE  1.1897314953572317649E+4932L
		   #define MINLDOUBLE  3.362103143112094E-4917L
		#else
			// Other gccs
		   #include <limits.h> 
         #include <float.h>
         #include <limits>
         #define MAXSHORT    0x7fff
         #define MAXINT      0x7fffffff
         #define MAXLONG     0x7fffffff
         #define MAXDOUBLE   1.7976931348623158E+308
         #ifndef MAXFLOAT
            #define MAXFLOAT    3.40282347E+38F
         #endif
         #define MINDOUBLE   2.2250738585072014E-308
         #define MINFLOAT    1.17549435E-38F
         #define MAXLDOUBLE  1.1897314953572317649E+4932L
         #define MINLDOUBLE  3.362103143112094E-4917L
		#endif
	#endif //__MINGW32_VERSION
#endif //__GNUG__

// Microsoft Visual C++
#ifdef _MSC_VER

   #include <limits.h>
   #define MAXLONG LONG_MAX
   #define MAXINT INT_MAX
   #define MAXDOUBLE 1.7E308

   // malloc & cia
   #include <memory.h>  

   // Avoid the deprecated POSIX names. 
   // TODO In the future we must replace the POSIX names by the ISO C++.
   #include <io.h>
   #include <fcntl.h>
   #include <sys/stat.h>
   #define lseek _lseek
   #define read _read
   #define write _write
   #define open _open

   #define O_CREAT _O_CREAT
   #define O_TRUNC _O_TRUNC
   #define O_RDWR _O_RDWR 
   #define O_BINARY _O_BINARY
   // The following constants are not supported by Win32
   #ifndef S_IREAD
      #define S_IREAD 0
   #endif
   #ifndef S_IWRITE
      #define S_IWRITE 0
   #endif

   // M_PI has been deprecated so I'll define it again.
   #ifndef M_PI
      #define M_PI           3.14159265358979323846
   #endif
#endif //_MSC_VER

// Borland C++
#ifdef __BORLANDC__
   #include <mem.h> // malloc & cia
   #include <limits.h> 
   #include <float.h>
#endif //__BORLANDC__

#endif //__STCOMMON_H
