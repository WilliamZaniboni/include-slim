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
* This file implements stPage and stLockablePage.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
// Copyright (c) 2002-2003 GBDI-ICMC-USP

#include <arboretum/stPage.h>

//------------------------------------------------------------------------------
// class stPage
//------------------------------------------------------------------------------
stPage::stPage (u_int32_t size, u_int32_t pageid){

   this->BufferSize = size;
   this->Buffer = new unsigned char[size];
   this->SetPageID(pageid);
}//end stPage::stPage

//------------------------------------------------------------------------------
stPage::~stPage(){

   if (Buffer != 0){
      delete[] Buffer;
      Buffer = 0;
   }//end if
}//end stPage::~stPage

//------------------------------------------------------------------------------
void stPage::Write(unsigned char * buff, u_int32_t n, u_int32_t offset){

   #ifdef __stDEBUG__
   if ((offset + n) > this->BufferSize){
      throw invalid_argument("Offset out of bounds.");
   }//end if
   #endif //__stDEBUG__

   memcpy((void *)(this->Buffer + offset), (void*)buff, n);
}//end stPage::Write

//------------------------------------------------------------------------------
u_int32_t stPage::GetPageSize(){

   return this->BufferSize;
}//end stPage::GetPageSize

//------------------------------------------------------------------------------
unsigned char * stPage::GetData(){

   return this->Buffer;
}//end stPage::GetData

//------------------------------------------------------------------------------
void stPage::Copy(stPage * page){

   #ifdef __stDEBUG__
   if (page->GetPageSize() != GetPageSize()){
      throw invalid_argument("Both pages must have the same size.");
   }//end if
   #endif //__stDEBUG__

   memcpy(this->GetData(), page->GetData(), this->GetPageSize());
}//end stPage::Copy

//------------------------------------------------------------------------------
void stPage::Clear(){

   memset(this->GetData(), 0, this->GetPageSize());
}//end stPage::Clear

//------------------------------------------------------------------------------
// class stLockablePage
//------------------------------------------------------------------------------
void stLockablePage::Write(unsigned char * buff, u_int32_t n, u_int32_t offset){

   #ifdef __stDEBUG__
   if ((LockSize + offset + n) > this->BufferSize){
      throw invalid_argument("Offset out of bounds.");
   }//end if
   #endif //__stDEBUG__

   memcpy((void *)(this->Buffer + offset + LockSize),
         (void*)buff, n);
}//end stLockablePage::Write

//------------------------------------------------------------------------------
u_int32_t stLockablePage::GetPageSize(){

   return this->BufferSize - this->LockSize;
}//end stLockablePage::GetPageSize

//------------------------------------------------------------------------------
unsigned char * stLockablePage::GetData(){

   return this->Buffer + this->LockSize;
}//end stLockablePage::GetData

