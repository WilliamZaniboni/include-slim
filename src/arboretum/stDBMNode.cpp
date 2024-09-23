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
* This file implements the dbmTree nodes.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.sc.usp.br)
*/
#include <arboretum/stDBMNode.h>

//------------------------------------------------------------------------------
// class stDBMNode
//------------------------------------------------------------------------------

int stDBMNode::AddEntry(u_int32_t size, const unsigned char * object, u_int32_t subTree){
   u_int32_t entrySize;

   #ifdef __stDEBUG__
   if (size == 0){
      throw invalid_argument("The object size is 0.");
   }//end if
   #endif //__stDEBUG__

   // Does it fit ?
   entrySize = size + sizeof(stDBMEntry);

   // Is this a subtree entry?
   if (subTree){
      // Add the radius size.
      entrySize += sizeof(double);
      #ifdef __stDBMNENTRIES__
         entrySize += sizeof(u_int32_t);
      #endif //__stDBMNENTRIES__
   }//end if

   if (entrySize > this->GetFree()){
      // No, it doesn't.
      return -1;
   }//end if

   // Mark this entry as...
   Entries[Header->Occupation].PageID = subTree;  // subtree

   // Ok. I can put it. Lets put it in the last position.

   // Adding the object. Take care with these pointers or you will destroy the
   // node. The idea is to put the object of an entry in the reverse order
   // in the data array.
   if (Header->Occupation == 0){
      Entries[0].Offset = Page->GetPageSize() - size;
   }else{
      Entries[Header->Occupation].Offset = Entries[Header->Occupation - 1].Offset - size;
   }//end if

   // Discount the Radius if this is a subtree.
   if (subTree){
      Entries[Header->Occupation].Offset -= sizeof(double);
      #ifdef __stDBMNENTRIES__
         Entries[Header->Occupation].Offset -= sizeof(u_int32_t);
      #endif //__stDBMNENTRIES__
   }//end if

   memcpy((void *)(Page->GetData() + Entries[Header->Occupation].Offset),
          (void *)object, size);
   // Update # of entries
   Header->Occupation++; // One more!

   return Header->Occupation - 1;
}//end stDBMNode::AddEntry

//------------------------------------------------------------------------------
int stDBMNode::GetRepresentativeIndex(){
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
}//end stDBMNode::GetRepresentativeIndex

//------------------------------------------------------------------------------
const unsigned char * stDBMNode::GetObject(u_int32_t idx){

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   return Page->GetData() + Entries[idx].Offset;
}//end stDBMNode::GetObject

//------------------------------------------------------------------------------
u_int32_t stDBMNode::GetObjectSize(u_int32_t idx){
   u_int32_t entrySize;

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   if (idx == 0){
      // First object
      entrySize = Page->GetPageSize() - Entries[0].Offset;
   }else{
      // Any other
      entrySize = Entries[idx - 1].Offset - Entries[idx].Offset;
   }//end if

   // Is this a subtree entry?
   if (Entries[idx].PageID){
      // Discount the radius
      entrySize -= sizeof(double);
      #ifdef __stDBMNENTRIES__
         entrySize -= sizeof(u_int32_t);
      #endif //__stDBMNENTRIES__
   }//end if

   return entrySize;
}//end stDBMNode::GetObjectSize

//------------------------------------------------------------------------------
void stDBMNode::RemoveEntry(int idx){
   u_int32_t i;
   u_int32_t lastID;
   u_int32_t rObjSize;

   // Programmer's note: This procedure is simple but tricky! See the
   // stDBMNode structure documentation for more details.

   #ifdef __stDEBUG__
   if ((idx < 0) || (idx >= (int )GetNumberOfEntries())){
      // Oops! This id doesn't exists.
      throw range_error("Invalid idx!");
   }//end if
   #endif //__stDEBUG__

   // Let's remove
   lastID = Header->Occupation - 1; // The id of the last object. This
                                    // value will be very useful.
   // Do I need to move something ?
   if (lastID != (unsigned int)idx){
      // Yes, I do. Save the removed object size
      rObjSize = GetObjectSize(idx);

      // If this is a subtree, add the size of Radius.
      if (Entries[idx].PageID){
         rObjSize += sizeof(double);
         #ifdef __stDBMNENTRIES__
            rObjSize += sizeof(u_int32_t);
         #endif //__stDBMNENTRIES__
         }//end if

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
         memcpy(Entries + i, Entries + i + 1, sizeof(stDBMEntry));

         // Update offset by adding the removed object size to it. It will
         // reflect the previous move operation.
         Entries[i].Offset += rObjSize;
      }//end for
   }//end if

   // Update counter...
   Header->Occupation--;
}//end stDBMNode::RemoveEntry

//------------------------------------------------------------------------------
u_int32_t stDBMNode::GetUsed(){
   u_int32_t usedSize;

   // Fixed size
   usedSize = sizeof(stDBMNodeHeader);

   // Entries
   if (GetNumberOfEntries() > 0){
      usedSize +=
         // Total size of entries
         (sizeof(stDBMEntry) * GetNumberOfEntries()) +
         // Total object size
         (Page->GetPageSize() - Entries[GetNumberOfEntries() - 1].Offset);
   }//end if

   return usedSize;
}//end stDBMNode::GetUsed

//------------------------------------------------------------------------------
unsigned char stDBMNode::GetHeight(){
   u_int32_t i;
   unsigned char maxHeight = 0;

   #ifdef __stDBMHEIGHT__
   for (i = 0; i < GetNumberOfEntries(); i++){
      if (maxHeight < GetEntry(i).Height){
         maxHeight = GetEntry(i).Height;
      }//end if
   }//end for
   #endif //__stDBMHEIGHT__

   return maxHeight;
}//end stDBMNode::GetHeight

//------------------------------------------------------------------------------
double stDBMNode::GetMinimumRadius(){
   u_int32_t i;
   double maxDistance = 0;
   double distance;

   for (i = 0; i < GetNumberOfEntries(); i++){
      distance = GetEntry(i).Distance;
      // is this a representative subtree?
      if (GetEntry(i).PageID){
         // Yes, add the Radius of this subtree.
         distance += GetRadius(i);
      }//end if
      if (maxDistance < distance){
         maxDistance = distance;
      }//end if
   }//end for

   return maxDistance;
}//end stDBMNode::GetMinimumRadius

//------------------------------------------------------------------------------
u_int32_t stDBMNode::GetTotalObjectCount(){
   u_int32_t i;
   u_int32_t count;

   #ifdef __stDBMNENTRIES__
      count = 0;
      for (i = 0; i < GetNumberOfEntries(); i++){
         // is this a representative subtree?
         if (GetEntry(i).PageID){
            // Yes, add the NEntries of this subtree.
            count += GetNEntries(i);
         }else{
            count++;
         }//end if
      }//end for
   #else
      count = GetNumberOfEntries();
   #endif //__stDBMNENTRIES__

   //return the total object count.
   return count;
}//end stDBMNode::GetTotalObjectCount

//------------------------------------------------------------------------------
u_int32_t stDBMNode::GetNumberOfFreeObjects(){
   u_int32_t i;
   u_int32_t count = 0;

   for (i = 0; i < GetNumberOfEntries(); i++){
      if (!GetEntry(i).PageID){
         count++;
      }//end if
   }//end for
   // return the number of free objects.
   return count;
}//end stDBMNode::GetNumberOfFreeObjects

//------------------------------------------------------------------------------
u_int32_t stDBMNode::GetFarthestObject(){
   u_int32_t i, idx = 0;
   double maxDistance = 0;

   for (i = 0; i < GetNumberOfEntries(); i++){
      if (GetEntry(i).Distance > maxDistance){
         maxDistance = GetEntry(i).Distance;
         idx = i;
      }//end if
   }//end for

   return idx;
}//end stDBMNode::GetFarthestObject

//------------------------------------------------------------------------------
u_int32_t stDBMNode::GetNEntries(u_int32_t idx){
   #ifdef __stDBMNENTRIES__
      u_int32_t * NEntries;
      #ifdef __stDEBUG__
      if (idx >= GetNumberOfEntries()){
         throw invalid_argument("idx value is out of range.");
      }//end if
      #endif //__stDEBUG__

      // if this entry does not have subtree.
      if (!Entries[idx].PageID){
         return 0;
      }//end if

      if (idx == 0){
         // First object
         NEntries = (u_int32_t *)(Page->GetData() + Page->GetPageSize() - sizeof(u_int32_t) - sizeof(double));
      }else{
         // Any other
         NEntries = (u_int32_t *)(Page->GetData() + Entries[idx - 1].Offset - sizeof(u_int32_t) - sizeof(double));
      }//end if
      // return NEntries.
      return *NEntries;
   #else
      return 0;
   #endif //__stDBMNENTRIES__
}//end stDBMNode::GetNEntries

//------------------------------------------------------------------------------
void stDBMNode::SetNEntries(u_int32_t idx, u_int32_t NEntries){
   #ifdef __stDBMNENTRIES__
      u_int32_t * nEntries;

      #ifdef __stDEBUG__
      if (idx >= GetNumberOfEntries()){
         throw invalid_argument("idx value is out of range.");
      }//end if
      #endif //__stDEBUG__

      // Adding the object. Take care with these pointers or you will destroy the
      // node. The idea is to put the object of an entry in the reverse order
      // in the data array.
      if (idx == 0){
         // First object
         nEntries = (u_int32_t *)(Page->GetData() + Page->GetPageSize() - sizeof(u_int32_t) - sizeof(double));
      }else{
         // Any other
         nEntries = (u_int32_t *)(Page->GetData() + Entries[idx - 1].Offset - sizeof(u_int32_t) - sizeof(double));
      }//end if
      // set the new NEntries.
      *nEntries = NEntries;
   #endif //__stDBMNENTRIES__
}//end stDBMNode::SetNEntries

//------------------------------------------------------------------------------
double stDBMNode::GetRadius(u_int32_t idx){
   double * Radius;
   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   // if this entry does not have subtree.
   if (!Entries[idx].PageID){
      return 0.0;
   }//end if

   if (idx == 0){
      // First object
      Radius = (double *)(Page->GetData() + Page->GetPageSize() - sizeof(double));
   }else{
      // Any other
      Radius = (double *)(Page->GetData() + Entries[idx - 1].Offset - sizeof(double));
   }//end if
   // return the Radius.
   return *Radius;
}//end stDBMNode::GetRadius

//------------------------------------------------------------------------------
void stDBMNode::SetRadius(u_int32_t idx, double radius){
   double * distance;

   #ifdef __stDEBUG__
   if (idx >= GetNumberOfEntries()){
      throw invalid_argument("idx value is out of range.");
   }//end if
   #endif //__stDEBUG__

   // if this entry has a space to store NEntries.
   if (GetEntry(idx).PageID){
      // Adding the object. Take care with these pointers or you will destroy the
      // node. The idea is to put the object of an entry in the reverse order
      // in the data array.
      if (idx == 0){
         // First object
         distance = (double *)(Page->GetData() + Page->GetPageSize() - sizeof(double));
      }else{
         // Any other
         distance = (double *)(Page->GetData() + Entries[idx - 1].Offset - sizeof(double));
      }//end if
      // set the new Distance.
      *distance = radius;
   }//end if
}//end stDBMNode::SetRadius
