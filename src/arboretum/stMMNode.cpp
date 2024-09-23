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
* This file implements the MMTree node.
*
* @version 1.0
* @author Ives Renê Venturini Pola (ives@icmc.sc.usp.br)
*/
#include <arboretum/stMMNode.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// class stMMNode
//-----------------------------------------------------------------------------
stMMNode::stMMNode(stPage * page, bool create){

   this->Page = page;

   // if create is true, we must to zero fill the page
   if (create){
      Page->Clear();
   }//end if

   // Set elements
   this->Header = (stMMNodeHeader *) this->Page->GetData();
   this->Entries = (u_int32_t *)(this->Page->GetData() + sizeof(stMMNodeHeader));

}//end stMMNode::stMMNode

//------------------------------------------------------------------------------
int stMMNode::AddEntry(u_int32_t size, const unsigned char * object){
   u_int32_t totalsize;
   u_int32_t offs;

   totalsize = size + sizeof(u_int32_t);
   if (totalsize <= GetFree()){

      // Object offset
      if (Header->Occupation == 0){
         offs = Page->GetPageSize() - size;
      }else{
         if (Header->Occupation == 1){
            offs = Entries[Header->Occupation-1] - size;
         }else{
            return -1; //there can be no more than two objects per node
         }//end if
      }//end if

      // Update entry
      this->Entries[Header->Occupation] = offs;

      // Write object
      memcpy((void *)(Page->GetData() + Entries[Header->Occupation]),
             (void *)object, size);

      // Update header
      this->Header->Occupation++;
      return ((int )this->Header->Occupation) - 1;
   }else{
      return -1;  // there is no room for the object
   }//end if
}//end stMMNode::AddEntry

//------------------------------------------------------------------------------
const unsigned char * stMMNode::GetObject(int id){

   return (unsigned char *) Page->GetData() + Entries[id];
}//end stMMNode::GetObject

//------------------------------------------------------------------------------
u_int32_t stMMNode::GetObjectSize(int id){

   if (id == 0){
      return Page->GetPageSize() - Entries[0];
   }else{
      return Entries[id-1] - Entries[id];
   }//end if
}//end stMMNode::GetObjectSize

//------------------------------------------------------------------------------
u_int32_t stMMNode::GetFree(){

   if (Header->Occupation == 0){
      return Page->GetPageSize() - sizeof(stMMNodeHeader);
   }else{
      return Page->GetPageSize() - sizeof(stMMNodeHeader) -
             (sizeof(u_int32_t) * Header->Occupation) -
             (Page->GetPageSize() - Entries[Header->Occupation-1]);
   }//end if
}//end stMMNode::GetFree
