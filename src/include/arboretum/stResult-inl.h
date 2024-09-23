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
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/

#ifndef __stRESULTNEWALLOC__
//----------------------------------------------------------------------------
// Class template stResult
//----------------------------------------------------------------------------
template < class ObjectType >
stResult<ObjectType>::~stResult(){
   unsigned int i;

   for (i = 0; i < Pairs.size(); i++){
      if (Pairs[i] != NULL){
         delete Pairs[i];
      }//end if
   }//end for

   // Dispose sample object.
   if (Sample != NULL){
      delete Sample;
   }//end if
}//end stResult<ObjectType>::~stResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stResult<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
unsigned int stResult<ObjectType>::Find(double distance){
   bool stop;
   unsigned int idx;

   idx = 0;
   stop = (idx >= Pairs.size());
   while (!stop){
      if (Pairs[idx]->GetDistance() < distance){
         idx++;
         stop = (idx >= Pairs.size());
      }else{
         stop = true;
      }//end if
   }//end while

   return idx;
}//end stResult<ObjectType>::Find

//----------------------------------------------------------------------------
template < class ObjectType >
bool stResult<ObjectType>::IsEqual(const stResult * r1){
   ObjectType * tmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = ((stResult *)r1)->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == ((stResult *)r1)->GetMaximumDistance()) &&
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
            if (Pairs[i]->GetDistance() == ((stResult *)r1)->Pairs[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = ((ObjectType *)((stResult *)r1)->Pairs[j]->GetObject())->Clone();
               // set the result2 with the equality of the two objects.
               result2 = ((ObjectType *)Pairs[i]->GetObject())->IsEqual(tmp);
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
         }while (i < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stResult<ObjectType>::Precision

//----------------------------------------------------------------------------
// Class template stJoinedResult
//----------------------------------------------------------------------------
template < class ObjectType >
stJoinedResult<ObjectType>::~stJoinedResult(){
   unsigned int i;

   for (i = 0; i < Triples.size(); i++){
      if (Triples[i] != NULL){
         delete Triples[i];
      }//end if
   }//end for
}//end stJoinedResult<ObjectType>::~stJoinedResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stJoinedResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stJoinedResult<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
unsigned int stJoinedResult<ObjectType>::Find(double distance){
   bool stop;
   unsigned int idx;

   idx = 0;
   stop = (idx >= Triples.size());
   while (!stop){
      if (Triples[idx]->GetDistance() < distance){
         idx++;
         stop = (idx >= Triples.size());
      }else{
         stop = true;
      }//end if
   }//end while

   return idx;
}//end stJoinedResult<ObjectType>::Find

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
         }while (i < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stJoinedResult<ObjectType>::Precision

#else
//----------------------------------------------------------------------------
// Class template stResult
//----------------------------------------------------------------------------
template < class ObjectType >
stResult<ObjectType>::~stResult(){
   tNode * node = First;
   // Clean.
   while (node != NULL){
      First = First->Next;
      delete node;
      node = First;
   }//end while
   Size = 0;
   First = NULL;

   // Dispose sample object.
   if (Sample != NULL){
      delete Sample;
   }//end if
}//end stResult<ObjectType>::~stResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stResult<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
stResult<ObjectType>::tNode * stResult<ObjectType>::Find(double distance){
   bool stop;
   tNode * prev = NULL;
   tNode * next = First;

   stop = (next == NULL);
   while (!stop){
      if (next->Pair.GetDistance() < distance){
         prev = next;
         next = next->Next;
         stop = (next == NULL);
      }else{
         stop = true;
      }//end if
   }//end while

   return prev;
}//end stResult<ObjectType>::Find

//----------------------------------------------------------------------------
template < class ObjectType >
bool stResult<ObjectType>::IsEqual(const stResult * r1){
   ObjectType * tmp;
   bool result, result2;
   tNode * node1, * node2;
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
      node1 = First;
      node2 = r1->First;
      // for each object in this class.
      while (node1 && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object.
         // for each object in the r1 set check until find a equal object.
         while (node2 && (!result2)) {
            // check the distance first between the two objects.
            if (node1->Pair.GetDistance() == node2->Pair.GetDistance()) {
               // return true;
               result2 = true;
               /*
               // the distance is equal, now test the object.
               tmp = node2->Pair.GetObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = node1->Pair.GetObject()->IsEqual(tmp);
               // delete the object's copy.
               delete tmp;
               */
            }//end if
            // increment the couter of the second set.
            node2 = node2->Next;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         node1 = node1->Next;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stResult<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Intersection(stResult * result1,
                                        stResult * result2){
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
void stResult<ObjectType>::Union(stResult * result1,
                                 stResult * result2){
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
// Class template stJoinedResult
//----------------------------------------------------------------------------
template < class ObjectType >
stJoinedResult<ObjectType>::~stJoinedResult(){
   tNode * node = First;
   // Clean.
   while (node != NULL){
      First = First->Next;
      delete node;
      node = First;
   }//end while
   Size = 0;
   First = NULL;
}//end stJoinedResult<ObjectType>::~stJoinedResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stJoinedResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->GetTie()){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stJoinedResult<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
stJoinedResult<ObjectType>::tNode * stJoinedResult<ObjectType>::Find(double distance){
   bool stop;
   tNode * prev = NULL;
   tNode * next = First;

   stop = (next == NULL);
   while (!stop){
      if (next->Triple.GetDistance() < distance){
         prev = next;
         next = next->Next;
         stop = (next == NULL);
      }else{
         stop = true;
      }//end if
   }//end while

   return prev;
}//end stJoinedResult<ObjectType>::Find

//----------------------------------------------------------------------------
template < class ObjectType >
bool stJoinedResult<ObjectType>::IsEqual(const stJoinedResult * r1){
   ObjectType * tmp;
   bool result, result2;
   tNode * node1, * node2;
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
      node1 = First;
      node2 = r1->First;
      // for each object in this class.
      while (node1 && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object.
         // for each object in the r1 set check until find a equal object.
         while (node2 && (!result2)) {
            // check the distance first between the two objects.
            if (node1->Triple.GetDistance() == node2->Triple.GetDistance()) {
               // return true;
               result2 = true;
               /*
               // the distance is equal, now test the object.
               tmp = node2->Triple.GetObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = node1->Triple.GetObject()->IsEqual(tmp);
               // delete the object's copy.
               delete tmp;
               */
            }//end if
            // increment the couter of the second set.
            node2 = node2->Next;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         node1 = node1->Next;
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
   ObjectType * tmpObj1, * tmpJoinedObj1, * tmpObj2, * tmpJoinedObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Triple[idx]->GetObject();
         tmpJoinedObj1 = (ObjectType *)result1->Triple[idx]->GetJoinedObject();
         distance = result1->Triple[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Triple[i]->GetObject();
            tmpJoinedObj2 = (ObjectType *)result2->Triple[i]->GetJoinedObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            joinedResult = tmpJoinedObj2->IsEqual(tmpJoinedObj1);
            // store if the two objects are equal.
            if (result && joinedResult){
              this->AddTriple(tmpObj1->Clone(), tmpJoinedObj1->Clone(), distance);
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
         tmpObj1 = (ObjectType *)result1->Triple[idx]->GetObject();
         distance = result1->Triple[idx]->GetDistance();
         this->AddTriple(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Triple[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Triple[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Triple[i]->GetObject();
            distance = result1->Triple[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddTriple(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triple[idx]->GetObject();
         distance = result1->Triple[idx]->GetDistance();
         this->AddTriple(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Triple[idx]->GetObject();
         distance = result2->Triple[idx]->GetDistance();
         this->AddTriple(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stJoinedResult<ObjectType>::Union

#endif //__stRESULTNEWALLOC__
