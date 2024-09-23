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
* This file implements the MTree nodes.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.sc.usp.br)
*/
#include <arboretum/stMNode.h>

//------------------------------------------------------------------------------
// class stMNode
//------------------------------------------------------------------------------
stMNode * stMNode::CreateNode(stPage * page){
   stMNode::stMNodeHeader * header;

   header = (stMNodeHeader *)(page->GetData());
   switch (header->Type){
      case INDEX:
         // Create an index page
         return new stMIndexNode(page, false);
      case LEAF:
         // Create a leaf page
         return new stMLeafNode(page, false);
      default:
         return NULL;
   }//end switch
}//end stMNode::CreateNode()

//------------------------------------------------------------------------------
// class stMIndexNode
//------------------------------------------------------------------------------
stMIndexNode::stMIndexNode(stPage * page, bool create):
      stMNode(page){

   // Attention to this manouver! It is the brain of this
   // implementation.
   Entries = (stMIndexEntry*)(page->GetData() + sizeof(stMNodeHeader));

   // Initialize page
   if (create){
       #ifdef __stDEBUG__
       Page->Clear();
       #endif //__stDEBUG__
      this->Header->Type = INDEX;
      this->Header->Occupation = 0;
   }//end if
}//end stMIndexNode::stMIndexNode()

//------------------------------------------------------------------------------
int stMIndexNode::AddEntry(u_int32_t size, const unsigned char * object){
   u_int32_t entrysize;

   #ifdef __stDEBUG__
   if (size == 0){
      throw invalid_argument("The object size is 0.");
   }//end if
   #endif //__stDEBUG__

   // Does it fit ?
   entrysize = size + sizeof(stMIndexEntry);
   if (entrysize > this->GetFree()){
      // No, it doesn't.
      return -1;
   }//end if

   // Ok. I can put it. Lets put it in the last position.

   // Adding the object. Take care with these pointers or you will destroy the
   // node. The idea is to put the object of an entry in the reverse order
   // in the data array.
   if (Header->Occupation == 0){
      Entries[Header->Occupation].Offset = Page->GetPageSize() - size;
   }else{
      Entries[Header->Occupation].Offset = Entries[Header->Occupation - 1].Offset - size;
   }//end if
   memcpy((void*)(Page->GetData() + Entries[Header->Occupation].Offset),
         (void*)object, size);
   // Update # of entries
   Header->Occupation++; // One more!

   return Header->Occupation - 1;
}//end stMIndexNode::AddEntry()

//------------------------------------------------------------------------------
int stMIndexNode::GetRepresentativeEntry(){
   u_int32_t i;
   bool stop;

   // Looking for it
   i = 0;
   stop = (i == Header->Occupation);
   while (!stop){
      if (Entries[i].Distance == 0.0){
         // Found!
         stop = true;
      }else{
         // Next...
         i++;
         stop = (i == Header->Occupation);
      }//end if
   }//end while

   // Output
   if (i == Header->Occupation){
      // Empty or not found.
      return -1;
   }else{
      // Found!
      return i;
   }//end if
}//end stMIndexNode::GetRepresentativeEntry()

//------------------------------------------------------------------------------
const unsigned char * stMIndexNode::GetObject(int idx){

   #ifdef __stDEBUG__
   if ((idx < 0) || (idx >= GetNumberOfEntries())){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   return Page->GetData() + Entries[idx].Offset;
}//end stMIndexNode::GetObject()

//------------------------------------------------------------------------------
u_int32_t stMIndexNode::GetObjectSize(int idx){
   #ifdef __stDEBUG__
   if ((idx < 0) || (idx >= GetNumberOfEntries())){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   if (idx == 0){
      // First object
      return Page->GetPageSize() - Entries[0].Offset;
   }else{
      // Any other
      return Entries[idx - 1].Offset - Entries[idx].Offset;
   }//end if
}//end stMIndexNode::GetObjectSize()

//------------------------------------------------------------------------------
void stMIndexNode::RemoveEntry(int idx){
   int i;
   int lastID;
   u_int32_t rObjSize;

   // Programmer's note: This procedure is simple but tricky! See the
   // MIndexNode structure documentation for more details.

   #ifdef __stDEBUG__
   if ((idx >= GetNumberOfEntries()) || (idx < 0)){
      // Oops! This idx doesn't exists.
      throw range_error("idx is out of range.");
   }//end if
   #endif //__stDEBUG__

   // Let's remove
   lastID = Header->Occupation - 1; // The idx of the last object. This
                                    // value will be very useful.
   // Do I need to move something ?
   if (idx != lastID){
      // Yes, I do.
      rObjSize = GetObjectSize(idx);    // Save the removed object size

      // Let's move objects first. We will use memmove() from stdlib because
      // it handles the overlap between src and dst. Remember that src is the
      // offset of the last object and the dst is the offset of the last
      // object plus removed object size.
      memmove(Page->GetData() + Entries[lastID].Offset + rObjSize,
            Page->GetData() + Entries[lastID].Offset,
            Entries[idx].Offset - Entries[lastID].Offset);

      // Let's move entries...
      for (i = idx; i < lastID; i++){
         // Copy all fields with memcpy (it's faster than field copy).
         memcpy(Entries + i, Entries + i + 1, sizeof(stMIndexEntry));

         // Update offset by adding the removed object size to it. It will
         // reflect the previous move operation.
         Entries[i].Offset += rObjSize;
      }//end for
   }//end if

   // Update counter...
   Header->Occupation --;
}//end stMIndexNode::RemoveEntry()

//------------------------------------------------------------------------------
u_int32_t stMIndexNode::GetFree(){
   u_int32_t usedsize;

   // Fixed size
   usedsize = sizeof(stMNodeHeader);

   // Entries
   if (GetNumberOfEntries() > 0){
      usedsize +=
         // Total size of entries
         (sizeof(stMIndexEntry) * GetNumberOfEntries()) +
         // Total object size
         (Page->GetPageSize() - Entries[GetNumberOfEntries() - 1].Offset);
   }//end if

   return Page->GetPageSize() - usedsize;
}//end stMIndexNode::GetFree()

//------------------------------------------------------------------------------
double stMIndexNode::GetMinimumRadius(){
   int i;
   double d = 0;
   double r;

   for (i = 0; i < GetNumberOfEntries(); i++){
      r = GetIndexEntry(i).Distance + GetIndexEntry(i).Radius;
      if (d < r){
         d = r;
      }//end if
   }//end for

   return d;
}//end stMIndexNode::GetMinimumRadius

//------------------------------------------------------------------------------
// class stMLeafNode
//------------------------------------------------------------------------------
stMLeafNode::stMLeafNode(stPage * page, bool create):
      stMNode(page){

   // Attention to this manouver! It is the brain of this
   // implementation.
   Entries = (stMLeafEntry*)(page->GetData() + sizeof(stMNodeHeader));

   // Initialize page
   if (create){
      #ifdef __stDEBUG__
      Page->Clear();
      #endif //__stDEBUG__
      this->Header->Type = LEAF;
      this->Header->Occupation = 0;
   }//end if
}//end stMLeafNode::stMLeafNode()

//------------------------------------------------------------------------------
int stMLeafNode::AddEntry(u_int32_t size, const unsigned char * object){
   u_int32_t entrysize;

   #ifdef __stDEBUG__
   if (size == 0){
      throw invalid_argument("The object size is 0.");
   }//end if
   #endif //__stDEBUG__

   // Does it fit ?
   entrysize = size + sizeof(stMLeafEntry);
   if (entrysize > this->GetFree()){
      // No, it doesn't.
      return -1;
   }//end if

   // Ok. I can put it. Lets put it in the last position.

   // Adding the object. Take care with these pointers or you will destroy the
   // node. The idea is to put the object of an entry in the reverse order
   // in the data array.
   if (Header->Occupation == 0){
      Entries[Header->Occupation].Offset = Page->GetPageSize() - size;
   }else{
      Entries[Header->Occupation].Offset = Entries[Header->Occupation - 1].Offset - size;
   }//end if
   memcpy((void*)(Page->GetData() + Entries[Header->Occupation].Offset),
             (void*)object, size);

   // Update # of entries
   Header->Occupation++; // One more!

   return Header->Occupation - 1;
}//end stMLeafNode::AddEntry()

//------------------------------------------------------------------------------
int stMLeafNode::GetRepresentativeEntry(){
   u_int32_t i;
   bool stop;

   // Looking for it
   i = 0;
   stop = (i == Header->Occupation);
   while (!stop){
      if (Entries[i].Distance == 0.0){
         // Found!
         stop = true;
      }else{
         // Next...
         i++;
         stop = (i == Header->Occupation);
      }//end if
   }//end while

   // Output
   if (i == Header->Occupation){
      // Empty or not found.
      return -1;
   }else{
      // Found!
      return i;
   }//end if
}//end stMLeafNode::GetRepresentativeEntry()

//------------------------------------------------------------------------------
const unsigned char * stMLeafNode::GetObject(int idx){

   #ifdef __stDEBUG__
   if ((idx < 0) || (idx >= GetNumberOfEntries())){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   return Page->GetData() + Entries[idx].Offset;
}//end stMLeafNode::GetObject()

//------------------------------------------------------------------------------
u_int32_t stMLeafNode::GetObjectSize(int idx){
   #ifdef __stDEBUG__
   if ((idx < 0) || (idx >= GetNumberOfEntries())){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   if (idx == 0){
      // First object
      return Page->GetPageSize() - Entries[0].Offset;
   }else{
      // Any other
      return Entries[idx - 1].Offset - Entries[idx].Offset;
   }//end if
}//end stMLeafIndexNode::GetObjectSize()

//------------------------------------------------------------------------------
void stMLeafNode::RemoveEntry(int idx){
   int i;
   int lastID;
   u_int32_t rObjSize;

   // Programmer's note: This procedure is simple but tricky! See the
   // MIndexNode structure documentation for more details.

   #ifdef __stDEBUG__
   if ((idx >= GetNumberOfEntries()) || (idx < 0)){
      // Oops! This idx doesn't exists.
      throw range_error("idx is out of range.");
   }//end if
   #endif //__stDEBUG__

   // Let's remove
   lastID = Header->Occupation - 1; // The idx of the last object. This
                                    // value will be very useful.
   // Do I need to move something ?
   if (idx != lastID){
      // Yes, I do.
      rObjSize = GetObjectSize(idx);    // Save the removed object size

      // Let's move objects first. We will use memmove() from stdlib because
      // it handles the overlap between src and dst. Remember that src is the
      // offset of the last object and the dst is the offset of the last
      // object plus removed object size.
      memmove(Page->GetData() + Entries[lastID].Offset + rObjSize,
            Page->GetData() + Entries[lastID].Offset,
            Entries[idx].Offset - Entries[lastID].Offset);

      // Let's move entries...
      for (i = idx; i < lastID; i++){
         // Copy all fields with memcpy (it's faster than field copy).
         memcpy(Entries + i, Entries + i + 1, sizeof(stMLeafEntry));

         // Update offset by adding the removed object size to it. It will
         // reflect the previous move operation.
         Entries[i].Offset += rObjSize;
      }//end for
   }//end if

   // Update counter...
   Header->Occupation --;
}//end stMLeafNode::RemoveEntry

//------------------------------------------------------------------------------
double stMLeafNode::GetMinimumRadius(){
   int i;
   double d = 0;

   for (i = 0; i < GetNumberOfEntries(); i++){
      if (d < GetLeafEntry(i).Distance){
         d = GetLeafEntry(i).Distance;
      }//end if
   }//end for

   return d;
}//end stMLeafNode::GetMinimumRadius

//------------------------------------------------------------------------------
u_int32_t stMLeafNode::GetFree(){
   u_int32_t usedsize;

   // Fixed size
   usedsize = sizeof(stMNodeHeader);

   // Entries
   if (GetNumberOfEntries() > 0){

      usedsize +=
         // Total size of entries
         (sizeof(stMLeafEntry) * GetNumberOfEntries()) +
         // Total object size
         (Page->GetPageSize() - Entries[GetNumberOfEntries() - 1].Offset);
   }//end if

   return Page->GetPageSize() - usedsize;
}//end stMLeafNode::GetFree()

