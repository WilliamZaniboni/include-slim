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
* This file has the implementation of the functions defined in the file stUtil.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#include <arboretum/stUtil.h>

//------------------------------------------------------------------------------
// Class stMessageString
//------------------------------------------------------------------------------
void stMessageString::Append(const char * str){
   this->str.append(str);
}//end Append

//------------------------------------------------------------------------------
void stMessageString::Append(double d){
   char tmp[32];

#ifdef _MSC_VER
   _snprintf_s(tmp, sizeof(tmp), sizeof(tmp), "%f", d);
#else
   printf(tmp, "%f", d);
#endif
 Append(tmp);
}//end Append

//------------------------------------------------------------------------------
void stMessageString::Append(int i){
   char tmp[32];

#ifdef _MSC_VER
   _snprintf_s(tmp,  sizeof(tmp), sizeof(tmp), "%d", i);
#else
   sprintf(tmp, "%d", i);
#endif
   Append(tmp);
}//end Append

//------------------------------------------------------------------------------
// struct doubleIndex
//------------------------------------------------------------------------------
/*
struct doubleIndex & doubleIndex::operator = (const struct doubleIndex & x){
   this->Distance = x.Distance;
   this->Index = x.Index;
   return (*this);
}//end doubleIndex::operator =
*/
//------------------------------------------------------------------------------
bool operator < (const doubleIndex & x, const doubleIndex & y){
   return x.Distance < y.Distance;
}//end operator <
