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
* This file defines a Collection of Objects to be promoted in high levels of
* the tree.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Review of documentation.
*/


//==============================================================================
// Class stDBMCollectionVector
//==============================================================================

//------------------------------------------------------------------------------
template <class ObjectType>
void stDBMCollectionVector<ObjectType>::RemoveAll(){
   int idx;
   int size = Pairs.size();

   // Delete all object that are in the vector.
   for (idx=size; idx > 0; idx--){
      if (Pairs[idx-1] != NULL){
         delete Pairs[idx-1];
      }//end if
   }//end for
   // Clean the vector.
   Pairs.clear();
}//end RemoveAll

//------------------------------------------------------------------------------
template <class ObjectType>
u_int32_t stDBMCollectionVector<ObjectType>::GetNumberOfFreeObjects(){
   u_int32_t idx;
   u_int32_t freeObj = 0;
   // Foa all entries.
   for (idx=0; idx < Pairs.size(); idx++){
      // Test if this is a free object.
      if (!Pairs[idx]->GetPageID())
         freeObj++;
   }//end for
   return freeObj;
}//end GetNumberOfFreeObjects

//------------------------------------------------------------------------------
template <class ObjectType>
u_int32_t stDBMCollectionVector<ObjectType>::GetUsedSpace(){
   u_int32_t idx;
   u_int32_t size = Pairs.size();
   u_int32_t usedSpace = 0;

   usedSpace = size * stDBMNode::GetEntryOverhead();
   // For each entry
   for (idx = 0; idx < size; idx++){
      usedSpace += (Pairs[idx]->GetObject())->GetSerializedSize();
      // If this entry is a subtree.
      if (Pairs[idx]->GetPageID()){
         // Add the radius.
         usedSpace += sizeof(double);
         #ifdef __stDBMNENTRIES__
            usedSpace += sizeof(u_int32_t);
         #endif //__stDBMNENTRIES__
      }//end if
   }//end for

   return usedSpace;
}//end GetUsedSpace

//------------------------------------------------------------------------------
template <class ObjectType>
void stDBMCollectionVector<ObjectType>::SetMineForAllObjects(bool option){
   u_int32_t size = Pairs.size();

   for (u_int32_t idx=0; idx<size; idx++){
      Pairs[idx]->SetMine(option);
   }//end for
}//end SetMineForAllObjects


//==============================================================================
// Class stDBMCollection
//==============================================================================

//------------------------------------------------------------------------------
template <class ObjectType>
void stDBMCollection<ObjectType>::AddEntry(tObject * object,
                    #ifdef __stDBMHEIGHT__
                       unsigned char height,
                    #endif //__stDBMHEIGHT__
                    u_int32_t pageID,
                    #ifdef __stDBMNENTRIES__
                       u_int32_t nEntries,
                    #endif //__stDBMNENTRIES__
                    double radius,
                    bool mine){
	// Resize me if required.
	if (Size == Capacity){
		Resize();
	}//end if
   Pairs[Size].SetObject(object);
   Pairs[Size].SetPageID(pageID);
   Pairs[Size].SetRadius(radius);
   #ifdef __stDBMHEIGHT__
      Pairs[Size].SetHeight(height);
   #endif //__stDBMHEIGHT__
   #ifdef __stDBMNENTRIES__
      Pairs[Size].SetNEntries(nEntries);
   #endif //__stDBMNENTRIES__
   Pairs[Size].SetMine(mine);
	Size++;
}//end AddPair

//------------------------------------------------------------------------------
template <class ObjectType>
void stDBMCollection<ObjectType>::RemoveAll(){
   // Delete the vector.
   if (Pairs != NULL){
      for (u_int32_t idx=0; idx < Size; idx++){
         if (Pairs[idx].GetMine()){
            delete Pairs[idx].GetObject();
            Pairs[idx].SetObject(NULL);
         }//end if  
      }//end for
      Size = 0;
   }//end if
}//end RemoveAll

//------------------------------------------------------------------------------
template <class ObjectType>
u_int32_t stDBMCollection<ObjectType>::GetNumberOfFreeObjects(){
   int idx;
   int freeObj = 0;

   for (idx=0; idx < Size; idx++){
      if (!(Pairs[idx].GetPageID()))
         freeObj++;
   }//end for
   return freeObj;
}//end GetNumberOfFreeObjects

//------------------------------------------------------------------------------
template <class ObjectType>
u_int32_t stDBMCollection<ObjectType>::GetUsedSpace(){
   u_int32_t usedSpace = 0;

   usedSpace = Size * stDBMNode::GetEntryOverhead();
   // For each entry
   for (u_int32_t idx=0; idx<Size; idx++){
      usedSpace += Pairs[idx].GetObject()->GetSerializedSize();
      // If this entry is a subtree.
      if (Pairs[idx].GetPageID()){
         // Add the radius.
         usedSpace += sizeof(double);
         #ifdef __stDBMNENTRIES__
            usedSpace += sizeof(u_int32_t);
         #endif //__stDBMNENTRIES__
      }//end if
   }//end for

   return usedSpace;
}//end GetUsedSpace

//------------------------------------------------------------------------------
template <class ObjectType>
void stDBMCollection<ObjectType>::SetMineForAllObjects(bool option){
   for (u_int32_t idx=0; idx < Size; idx++){
      Pairs[idx].SetMine(option);
   }//end for
}//end SetMineForAllObjects

//------------------------------------------------------------------------------
template <class ObjectType>
void stDBMCollection<ObjectType>::Resize(){
   tPair * newPairs;
   bool mine;

   // store the status of objects.
   mine = Pairs[0].GetMine();
   SetMineForAllObjects(false);
	// New entry vector
	newPairs = new tPair[Capacity + Increment];
	memcpy(newPairs, Pairs, Capacity * sizeof(tPair));
	// Delete the old vector.
	delete[] Pairs;
   // Ajust the pointers.
	Pairs = newPairs;
   // restore the status of objects.
   SetMineForAllObjects(mine);
   // Update the new value of capacity.
	Capacity += Increment;
}//end Resize
