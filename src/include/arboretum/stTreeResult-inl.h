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
* This file implements the classes stResult and stResults.
*
* @version 1.0
* @author Enzo Seraphim (seraphim@unifei.edu.br)
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/

//----------------------------------------------------------------------------
// Class template stResult
//----------------------------------------------------------------------------
template < class ObjectType >
stResult<ObjectType>::~stResult(){
   while(Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--; //last element
      delete *ite;
      Pairs.erase(ite);
   }//end while
   // Dispose sample object.
   if (Sample != NULL){
      delete Sample;
   }//end if
   // Dispose vector of pairs.
   if (allpairs != NULL)
       delete[] allpairs;
   allpairs = NULL;
}//end stResult<ObjectType>::~stResult

//----------------------------------------------------------------------------
#ifdef _DEPRECATED_ARBORETUM
template < class ObjectType >
stResult<ObjectType>::tPair & stResult<ObjectType>::operator [] (unsigned int idx){
   if (idx < Pairs.size()){
      tItePairs ite = Pairs.begin();
      int i;
      for(i=0; i < idx; i++){
         ite++;
      }//endfor
      return *(*ite);
   }//end if
}//end stResult<ObjectType>::operator []
#endif //_DEPRECATED_ARBORETUM

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::RemoveLast(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--; //last element
      delete *ite;
      Pairs.erase(ite);
   }//end if
}//end RemoveLast

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::RemoveFirst(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.begin();
      delete *ite;
      
      Pairs.erase(ite);
   }//end if
}//end RemoveFirst

//----------------------------------------------------------------------------
template < class ObjectType >
double stResult<ObjectType>::GetMinimumDistance(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.begin();
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMinimumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
double stResult<ObjectType>::GetMaximumDistance(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--;
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMaximumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Cut(unsigned int limit){
   
   double max;

   if(this->Tie){
      
      tItePairs ite = Pairs.begin();
      for (int i = 0; i < (limit-1) && ite!= Pairs.end(); i++, ite++);
      max = (*ite)->GetDistance();

      bool stop = false;
      while(!stop){

         tItePairs lastElem = Pairs.end();
         lastElem--;
         double lastValue = (*lastElem)->GetDistance();

         if (lastValue > max)            
               RemoveLast();
         else
            stop = true;
         }
         
   }else{
      while(Pairs.size() > limit){
        RemoveLast();
      }
   }
}//end stResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::CutFirst(unsigned int limit){
   while(Pairs.size() > limit){
      tItePairs ite = Pairs.begin();
      delete *ite;
      Pairs.erase(ite);
   }//end while
}//end stResult<ObjectType>::CutFirst

/*
//----------------------------------------------------------------------------
template < class ObjectType >
bool stResult<ObjectType>::IsEqual(const stResult * r1){
   ObjectType * tmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Pairs[i]->GetDistance() == r1->Pairs[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Pairs[j]->GetObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = Pairs[i]->GetObject()->IsEqual(tmp);
               // delete the object's copy.
               delete tmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stResult<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Intersection(stResult * result1, stResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stResult<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Union(stResult * result1, stResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Pairs[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Pairs[i]->GetObject();
            distance = result1->Pairs[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         distance = result2->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stResult<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stResult<ObjectType>::Precision(const stResult * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Pairs[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (w < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stResult<ObjectType>::Precision
*/

//----------------------------------------------------------------------------
// Class template stResultPaged
//----------------------------------------------------------------------------

template < class ObjectType >
stResultPaged<ObjectType>::~stResultPaged(){
   while(Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--; //last element
      delete *ite;
      Pairs.erase(ite);
   }//end while
   // Dispose sample object.
   if (Sample != NULL){
      delete Sample;
   }//end if
}//end stResultPaged<ObjectType>::~stResultPaged

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::RemoveLast(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--; //last element
      delete *ite;
      Pairs.erase(ite);
   }//end if
}//end RemoveLast

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::RemoveFirst(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.begin();
      delete *ite;
      Pairs.erase(ite);
   }//end if
}//end RemoveFirst

//----------------------------------------------------------------------------
template < class ObjectType >
double stResultPaged<ObjectType>::GetMinimumDistance(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.begin();
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMinimumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
double stResultPaged<ObjectType>::GetMaximumDistance(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--;
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMaximumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
long stResultPaged<ObjectType>::GetLastOID(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.end();
      ite--;
      return ((ObjectType *)((*ite)->GetObject()))->GetOID();
   }else{
      return -1;
   }//end if
}//end GetLastOID

//----------------------------------------------------------------------------
template < class ObjectType >
long stResultPaged<ObjectType>::GetFirstOID(){
   if (Pairs.size() > 0){
      tItePairs ite = Pairs.begin();
      return ((ObjectType *)((*ite)->GetObject()))->GetOID();
   }else{
      return -1;
   }//end if
}//end GetLastOID

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::Cut(unsigned int limit){
   while(Pairs.size() > limit){
      tItePairs ite = Pairs.end();
      ite--; //last element
      delete *ite;
      Pairs.erase(ite);
   }//end while
}//end stResultPaged<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::CutFirst(unsigned int limit){
   while(Pairs.size() > limit){
      tItePairs ite = Pairs.begin();
      delete *ite;
      Pairs.erase(ite);
   }//end while
}//end stResultPaged<ObjectType>::CutFirst

/*
//----------------------------------------------------------------------------
template < class ObjectType >
bool stResultPaged<ObjectType>::IsEqual(const stResultPaged * r1){
   ObjectType * tmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Pairs[i]->GetDistance() == r1->Pairs[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Pairs[j]->GetObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = Pairs[i]->GetObject()->IsEqual(tmp);
               // delete the object's copy.
               delete tmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stResultPaged<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::Intersection(stResultPaged * result1, stResultPaged * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stResultPaged<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::Union(stResultPaged * result1, stResultPaged * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Pairs[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Pairs[i]->GetObject();
            distance = result1->Pairs[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         distance = result2->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stResultPaged<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stResultPaged<ObjectType>::Precision(const stResultPaged * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Pairs[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (w < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stResultPaged<ObjectType>::Precision
*/

//----------------------------------------------------------------------------
// Class template stJoinedResult
//----------------------------------------------------------------------------
template < class ObjectType >
stJoinedResult<ObjectType>::~stJoinedResult(){
   while(Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--; //last element
      delete *ite;
      Triples.erase(ite);
   }//end while
   // Dispose sample object.
   if (this->Sample != NULL){
      delete this->Sample;
   }//end if
}//end stResult<ObjectType>::~stResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::RemoveLast(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--; //last element
      delete *ite;
      Triples.erase(ite);
   }//end if
}//end RemoveLast

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::RemoveFirst(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.begin();
      delete *ite;
      Triples.erase(ite);
   }//end if
}//end RemoveFirst

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResult<ObjectType>::GetMinimumDistance(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.begin();
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMinimumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResult<ObjectType>::GetMaximumDistance(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--;
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMaximumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Cut(unsigned int limit){
   while(Triples.size() > limit){
      tIteTriples ite = Triples.end();
      ite--; //last element
      delete *ite;
      Triples.erase(ite);
   }//end while
}//end Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::CutFirst(unsigned int limit){
   while(Triples.size() > limit){
      tIteTriples ite = Triples.begin();
      delete *ite;
      Triples.erase(ite);
   }//end while
}//end CutFirst

/*
//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::AddLocalResult(stResult<ObjectType> * local){
   typename stResult<ObjectType>::tItePairs ite;
   for (ite = (local->GetPairs())->begin(); ite != (local->GetPairs())->end(); ++ite){
      Triples.insert(Triples.begin(), new tTriple((local->GetSample())->Clone(),
         ((ObjectType*)(*ite)->GetObject())->Clone(), (*ite)->GetDistance()));
   }//end for
}//end stJoinedResult<ObjectType>::AddLocalResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::AddLocalResultMaxDist(stResult<ObjectType> * local){
   typename stResult<ObjectType>::tItePairs ite;
   for (ite = (local->GetPairs())->begin();
         ((ite != (local->GetPairs())->end()) &&
         ((*ite)->GetDistance() <= GetMaximumDistance())); ++ite){
      Triples.insert(Triples.begin(), new tTriple((local->GetSample())->Clone(),
         ((ObjectType*)(*ite)->GetObject())->Clone(), (*ite)->GetDistance()));
   }//end for
}//end stJoinedResult<ObjectType>::AddLocalResultMaxDist

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::AddtJoinedResult(stJoinedResult<ObjectType> * res){
   tIteTriples ite;
   for (ite = (res->GetTriples())->begin(); ite != (res->GetTriples())->end(); ++ite){
      Triples.insert(Triples.begin(), new tTriple(
         ((ObjectType*)(*ite)->GetJoinedObject())->Clone(),
         ((ObjectType*)(*ite)->GetObject())->Clone(), (*ite)->GetDistance()));
   }//end for
}//end stJoinedResult<ObjectType>::AddtJoinedResult

//----------------------------------------------------------------------------
template < class ObjectType >
bool stJoinedResult<ObjectType>::IsEqual(const stJoinedResult * r1){
   ObjectType * tmp, * joinedTmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Triples[i]->GetDistance() == r1->Triples[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Triples[j]->GetObject()->Clone();
               joinedTmp = r1->Triples[j]->GetJoinedObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = (Triples[i]->GetObject()->IsEqual(tmp) &&
                         Triples[i]->GetJoinedObject()->IsEqual(joinedTmp)) ||
                         (Triples[i]->GetObject()->IsEqual(joinedTmp) &&
                         Triples[i]->GetJoinedObject()->IsEqual(tmp));
               // delete the object's copy.
               delete tmp;
               delete joinedTmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stJoinedResult<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Intersection(stJoinedResult * result1,
                                              stJoinedResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Triples[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stJoinedResult<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Union(stJoinedResult * result1,
                                       stJoinedResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Triples[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Triples[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Triples[i]->GetObject();
            distance = result1->Triples[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Triples[idx]->GetObject();
         distance = result2->Triples[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stJoinedResult<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResult<ObjectType>::Precision(const stJoinedResult * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Triples[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Triples[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (w < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stJoinedResult<ObjectType>::Precision
*/

//----------------------------------------------------------------------------
// Class template stJoinedResultPaged
//----------------------------------------------------------------------------
template < class ObjectType >
stJoinedResultPaged<ObjectType>::~stJoinedResultPaged(){
   while(Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--; //last element
      delete *ite;
      Triples.erase(ite);
   }//end while
   // Dispose sample object.
   if (this->Sample != NULL){
      delete this->Sample;
   }//end if
}//end stResultPaged<ObjectType>::~stResultPaged

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::RemoveLast(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--; //last element
      delete *ite;
      Triples.erase(ite);
   }//end if
}//end RemoveLast

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::RemoveFirst(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.begin();
      delete *ite;
      Triples.erase(ite);
   }//end if
}//end RemoveFirst

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResultPaged<ObjectType>::GetMinimumDistance(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.begin();
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMinimumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResultPaged<ObjectType>::GetMaximumDistance(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--;
      return (*ite)->GetDistance();
   }else{
      return -1;
   }//end if
}//end GetMaximumDistance

//----------------------------------------------------------------------------
template < class ObjectType >
long stJoinedResultPaged<ObjectType>::GetLastOID(){
   if (Triples.size() > 0){
      tIteTriples ite = Triples.end();
      ite--;
      return ((ObjectType *)((*ite)->GetObject()))->GetOID();
   }else{
      return -1;
   }//end if
}//end GetLastOID

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::Cut(unsigned int limit){
   while(Triples.size() > limit){
      tIteTriples ite = Triples.end();
      ite--; //last element
      delete *ite;
      Triples.erase(ite);
   }//end while
}//end Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::CutFirst(unsigned int limit){
   while(Triples.size() > limit){
      tIteTriples ite = Triples.begin();
      delete *ite;
      Triples.erase(ite);
   }//end while
}//end CutFirst

/*
//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::AddLocalResult(stResultPaged<ObjectType> * local){
   typename stResultPaged<ObjectType>::tItePairs ite;
   for (ite = (local->GetPairs())->begin(); ite != (local->GetPairs())->end(); ++ite){
      Triples.insert(Triples.begin(), new tTriple((local->GetSample())->Clone(),
         ((ObjectType*)(*ite)->GetObject())->Clone(), (*ite)->GetDistance()));
   }//end for
}//end stJoinedResultPaged<ObjectType>::AddLocalResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::AddLocalResultMaxDist(stResultPaged<ObjectType> * local){
   typename stResultPaged<ObjectType>::tItePairs ite;
   for (ite = (local->GetPairs())->begin();
         ((ite != (local->GetPairs())->end()) &&
         ((*ite)->GetDistance() <= GetMaximumDistance())); ++ite){
      Triples.insert(Triples.begin(), new tTriple((local->GetSample())->Clone(),
         ((ObjectType*)(*ite)->GetObject())->Clone(), (*ite)->GetDistance()));
   }//end for
}//end stJoinedResultPaged<ObjectType>::AddLocalResultMaxDist

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::AddtJoinedResult(stJoinedResultPaged<ObjectType> * res){
   tIteTriples ite;
   for (ite = (res->GetTriples())->begin(); ite != (res->GetTriples())->end(); ++ite){
      Triples.insert(Triples.begin(), new tTriple(
         ((ObjectType*)(*ite)->GetJoinedObject())->Clone(),
         ((ObjectType*)(*ite)->GetObject())->Clone(), (*ite)->GetDistance()));
   }//end for
}//end stJoinedResultPaged<ObjectType>::AddtJoinedResult

//----------------------------------------------------------------------------
template < class ObjectType >
bool stJoinedResultPaged<ObjectType>::IsEqual(const stJoinedResultPaged * r1){
   ObjectType * tmp, * joinedTmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Triples[i]->GetDistance() == r1->Triples[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Triples[j]->GetObject()->Clone();
               joinedTmp = r1->Triples[j]->GetJoinedObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = (Triples[i]->GetObject()->IsEqual(tmp) &&
                         Triples[i]->GetJoinedObject()->IsEqual(joinedTmp)) ||
                         (Triples[i]->GetObject()->IsEqual(joinedTmp) &&
                         Triples[i]->GetJoinedObject()->IsEqual(tmp));
               // delete the object's copy.
               delete tmp;
               delete joinedTmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stJoinedResultPaged<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::Intersection(stJoinedResultPaged * result1,
                                              stJoinedResultPaged * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Triples[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stJoinedResultPaged<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResultPaged<ObjectType>::Union(stJoinedResultPaged * result1,
                                       stJoinedResultPaged * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Triples[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Triples[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Triples[i]->GetObject();
            distance = result1->Triples[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Triples[idx]->GetObject();
         distance = result2->Triples[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stJoinedResultPaged<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResultPaged<ObjectType>::Precision(const stJoinedResultPaged * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Triples[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Triples[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (w < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stJoinedResultPaged<ObjectType>::Precision
*/

#ifdef __stCKNNQ__
//----------------------------------------------------------------------------
// Class template stConstrainedResult
//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::RemoveLast() {
  if (::stResult<ObjectType>::Pairs.size() <= 0)  {
      throw std::logic_error("Erro stConstrainedResult::RemoveLast() Pairs");
  }

  tItePairs ite = ::stResult<ObjectType>::Pairs.end();
  ite--; //last element

  //DELETE IN SATISFY OR NOTSATISFY LIST
  tIteSPairs tmpS = satisfyPairs.end();
  tIteSPairs tmpNS = notSatisfyPairs.end();

  if (satisfyPairs.size() > 0 && *(--tmpS) == ite) {
    satisfyPairs.erase(tmpS);
  } else if (notSatisfyPairs.size() > 0 && *(--tmpNS) == ite) {
    notSatisfyPairs.erase(tmpNS);
  } else {
    throw std::logic_error("Erro stConstrainedResult::RemoveLast()");
  }

  //DELETE IN PAIRS
  delete *ite;
  ::stResult<ObjectType>::Pairs.erase(ite);
}//end RemoveLast

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::RemoveLastSatisfyPairs() {
//  if (::stResult<ObjectType>::Pairs.size() > 0) {
  if (satisfyPairs.size() <= 0) {
      throw std::logic_error("Erro stConstrainedResult::RemoveLastSatistyPairs()");
  }

  tItePairs ite = *(--satisfyPairs.end());

  //DELETE IN SATISFY LIST
  satisfyPairs.erase(--satisfyPairs.end());

  //DELETE IN PAIRS
  delete *ite;
  ::stResult<ObjectType>::Pairs.erase(ite);
}//end RemoveLast

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::RemoveLastNotSatisfyPairs() {
//    if (::stResult<ObjectType>::Pairs.size() > 0) {
  if (notSatisfyPairs.size() <= 0) {
    throw std::logic_error("Erro stConstrainedResult::RemoveLastNotSatistyPairs()");
  }

  tItePairs ite = *(--notSatisfyPairs.end());

  //DELETE IN SATISFY OR NOTSATISFY LIST
  notSatisfyPairs.erase(--notSatisfyPairs.end());

  //DELETE IN PAIRS
  delete *ite;
  ::stResult<ObjectType>::Pairs.erase(ite);
}//end RemoveLast

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::RemoveSatisfyPairs(tIteSPairs it) {
  if (satisfyPairs.size() <= 0) {
    throw std::logic_error("Erro stConstrainedResult::RemoveSatistyPairs()");
  }

  tItePairs ite = *it;

  //DELETE IN SATISFY LIST
  satisfyPairs.erase(it);

  //DELETE IN PAIRS
  delete *ite;
  ::stResult<ObjectType>::Pairs.erase(ite);
}//end RemoveLast

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::ToNotSatisfyPairs(tIteSPairs it) {
  if (satisfyPairs.size() <= 0) {
    throw std::logic_error("Erro stConstrainedResult::ToNotSatistyPairs()");
  }

  //Remove from Satisfy and insert into notSatisfy
  notSatisfyPairs.insert(*it);
  satisfyPairs.erase(it);
}//end ToNotSatisfyPairs

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::Cut(unsigned int limit) {
  if (this->Tie) {
      throw std::logic_error("Tie support uninmplemented in stConstrainedResult.");
  } else {
    while (::stResult<ObjectType>::Pairs.size() > limit) {
      RemoveLast();
    }
  }
}//end stConstrainedResult<ObjectType>::Cut

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::CutSatisfyPairs(unsigned int limit) {

  if (this->Tie) {
      throw std::logic_error("Tie support uninmplemented in stConstrainedResult.");
  } else {
    while (satisfyPairs.size() > limit) {
      RemoveLastSatisfyPairs();
    }
  }
}//end stConstrainedResult<ObjectType>::CutSatisfyPairs

//----------------------------------------------------------------------------

template < class ObjectType >
void stConstrainedResult<ObjectType>::CutNotSatisfyPairs(unsigned int limit) {

  if (this->Tie) {
      throw std::logic_error("Tie support uninmplemented in stConstrainedResult.");
  } else {
    while (notSatisfyPairs.size() > limit) {
      RemoveLastNotSatisfyPairs();
    }
  }
}//end stConstrainedResult<ObjectType>::CutNotSatisfyPairs

#endif // __STCKNNQ__
