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
* This file implements the RTree nodes.
*/

//------------------------------------------------------------------------------
// class stRNode
//------------------------------------------------------------------------------
stRNode * stRNode::CreateNode(stPage * page){
   stRNode::stRNodeHeader * header;

   header = (stRNodeHeader *)(page->GetData());
   switch (header->Type){
      case INDEX:
         // Create an index page
         return new stRIndexNode(page, false);
      case LEAF:
         // Create a leaf page
         return new stRLeafNode(page, false);
      default:
         return NULL;
   }//end switch
}//end stRNode::CreateNode()

//------------------------------------------------------------------------------
// class stRIndexNode
//------------------------------------------------------------------------------
stRIndexNode::stRIndexNode(stPage * page, bool create): stRNode(page){

   // Attention to this manouver! It is the brain of this
   // implementation.
   Entries = (stRIndexEntry *)(page->GetData() + sizeof(stRNodeHeader));

   // Initialize page
   if (create){
      #ifdef __stDEBUG__
      Page->Clear();
      #endif //__stDEBUG__
      this->Header->Type = INDEX;
      this->Header->Occupation = 0;
   }//end if
}//end stRIndexNode::stRIndexNode()

//------------------------------------------------------------------------------
int stRIndexNode::AddEntry(u_int32_t size, const unsigned char * object){
   u_int32_t entrySize;

   // Does it fit ?
   entrySize = size + sizeof(stRIndexEntry);
   if (entrySize > this->GetFree()){
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
   memcpy((void *)(Page->GetData() + Entries[Header->Occupation].Offset), (void *)object, size);
   // Update # of entries
   Header->Occupation++; // One more!

   return Header->Occupation - 1;
}//end stRIndexNode::AddEntry()

//------------------------------------------------------------------------------
const unsigned char * stRIndexNode::GetObject(u_int32_t idx){

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw std::logic_error("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   return Page->GetData() + Entries[idx].Offset;
}//end stRIndexNode::GetObject()

//------------------------------------------------------------------------------
u_int32_t stRIndexNode::GetObjectSize(u_int32_t idx){
   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw std::logic_error("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   if (idx == 0){
      // First object
      return Page->GetPageSize() - Entries[0].Offset;
   }else{
      // Any other
      return Entries[idx - 1].Offset - Entries[idx].Offset;
   }//end if
}//end stRIndexNode::GetObjectSize()

//------------------------------------------------------------------------------
void stRIndexNode::RemoveEntry(u_int32_t idx){
   u_int32_t rObjSize;
   u_int32_t i, lastID;

   // Programmer's note: This procedure is simple but tricky! See the
   // RIndexNode structure documentation for more details.

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
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
         memcpy(Entries + i, Entries + i + 1, sizeof(stRIndexEntry));

         // Update offset by adding the removed object size to it. It will
         // reflect the previous move operation.
         Entries[i].Offset += rObjSize;
      }//end for
   }//end if

   // Update counter...
   Header->Occupation--;
}//end stRIndexNode::RemoveEntry()

//------------------------------------------------------------------------------
u_int32_t stRIndexNode::GetFree(){
   u_int32_t usedsize;

   // Fixed size
   usedsize = sizeof(stRNodeHeader);

   // Entries
   if (GetNumberOfEntries() > 0){
      usedsize +=
         // Total size of entries
         (sizeof(stRIndexEntry) * GetNumberOfEntries()) +
         // Total object size
         (Page->GetPageSize() - Entries[GetNumberOfEntries() - 1].Offset);
   }//end if

   return Page->GetPageSize() - usedsize;
}//end stRIndexNode::GetFree()
//------------------------------------------------------------------------------
u_int32_t stRIndexNode::GetTotalObjectCount(){
   u_int32_t count;
   u_int32_t i;

   count = 0;
   for (i = 0; i < GetNumberOfEntries(); i++){
      count += GetIndexEntry(i).NEntries;
   }//end for

   return count;
}//end stRIndexNode::GetTotalObjectCount

//------------------------------------------------------------------------------
// class stRLeafNode
//------------------------------------------------------------------------------
stRLeafNode::stRLeafNode(stPage * page, bool create):
      stRNode(page){

   // Attention to this manouver! It is the brain of this
   // implementation.
   Entries = (stRLeafEntry*)(page->GetData() + sizeof(stRNodeHeader));

   // Initialize page
   if (create){
      #ifdef __stDEBUG__
      Page->Clear();
      #endif //__stDEBUG__
      this->Header->Type = LEAF;
      this->Header->Occupation = 0;
   }//end if
}//end stRLeafNode::stRLeafNode()

//------------------------------------------------------------------------------
int stRLeafNode::AddEntry(u_int32_t size, const unsigned char * object){
   u_int32_t entrySize;

   #ifdef __stDEBUG__
   if (size == 0){
      throw std::logic_error("The object size is 0.");
   }//end if
   #endif //__stDEBUG__

   // Does it fit ?
   entrySize = size + sizeof(stRLeafEntry);
   if (entrySize > this->GetFree()){
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
   memcpy((void *)(Page->GetData() + Entries[Header->Occupation].Offset),
          (void *)object, size);

   // Update # of entries
   Header->Occupation++; // One more!

   return Header->Occupation - 1;
}//end stRLeafNode::AddEntry()
//------------------------------------------------------------------------------
const unsigned char * stRLeafNode::GetObject(u_int32_t idx){

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw std::logic_error("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   return Page->GetData() + Entries[idx].Offset;
}//end stRLeafNode::GetObject()

//------------------------------------------------------------------------------
u_int32_t stRLeafNode::GetObjectSize(u_int32_t idx){
   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw std::logic_error("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   if (idx == 0){
      // First object
      return Page->GetPageSize() - Entries[0].Offset;
   }else{
      // Any other
      return Entries[idx - 1].Offset - Entries[idx].Offset;
   }//end if
}//end stRLeafIndexNode::GetObjectSize()

//------------------------------------------------------------------------------
void stRLeafNode::RemoveEntry(u_int32_t idx){
   u_int32_t lastID;
   u_int32_t i;
   u_int32_t rObjSize;

   // Programmer's note: This procedure is simple but tricky! See the
   // RIndexNode structure documentation for more details.

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      // Oops! This idx doesn't exists.
      throw range_error("idx value is out of range.");
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
         memcpy(Entries + i, Entries + i + 1, sizeof(stRLeafEntry));

         // Update offset by adding the removed object size to it. It will
         // reflect the previous move operation.
         Entries[i].Offset += rObjSize;
      }//end for
   }//end if

   // Update counter...
   Header->Occupation--;
}//end stRLeafNode::RemoveEntry
//------------------------------------------------------------------------------
u_int32_t stRLeafNode::GetFree(){
   u_int32_t usedSize;

   // Fixed size
   usedSize = sizeof(stRNodeHeader);

   // Entries
   if (GetNumberOfEntries() > 0){

      usedSize +=
         // Total size of entries
         (sizeof(stRLeafEntry) * GetNumberOfEntries()) +
         // Total object size
         (Page->GetPageSize() - Entries[GetNumberOfEntries() - 1].Offset);
   }//end if

   return Page->GetPageSize() - usedSize;
}//end stRLeafNode::GetFree()
