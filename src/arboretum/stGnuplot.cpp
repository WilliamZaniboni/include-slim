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
* This file implements the blocks of ParitionGlobal.
*
* @version 1.0
* @author Enzo Seraphim(seraphim@icmc.usp.br)
*/

#include <arboretum/stGnuplot3D.h>
#include <cstring>
#include <fstream>

//------------------------------------------------------------------------------
// Class stGnuplot
//------------------------------------------------------------------------------
stGnuplot::~stGnuplot(){
   if (title){
      delete [] title;
   }
   if (xlabel){
      delete [] xlabel;
   }
}// ~stGnuplot

//------------------------------------------------------------------------------
void stGnuplot::SetTitle(const char * str) {
   if (title){
      delete [] title;
   }
   title = new char[strlen(str)+1];
   strcpy(title, str);
}// SetTitle

//------------------------------------------------------------------------------
void stGnuplot::SetXLabel(const char * label) {
   if (xlabel){
      delete [] xlabel;
   }
   xlabel = new char[strlen(label)+1];
   strcpy(xlabel, label);
}// SetXLabel
//------------------------------------------------------------------------------

