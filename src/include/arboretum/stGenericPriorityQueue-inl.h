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
//----------------------------------------------------------------------------
// class stGenericPriorityQueue
//----------------------------------------------------------------------------
/**
* This class defines node type of stGenericPriorityQueue.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup util
* @todo Documentation update.
*/

//------------------------------------------------------------------------------
template <class ObjectType>
stGenericPriorityQueue<ObjectType>::~stGenericPriorityQueue(){
   tGenericEntry * temp = this->Head;
   // while there are objects to be removed...
   while (this->Head){
      this->Head = this->Head->GetNext();
      delete temp;
      temp = this->Head;
   }//end while
}//end ~stGenericPriorityQueue

//------------------------------------------------------------------------------
// APPROXIMATENODE
template <class ObjectType>
void  stGenericPriorityQueue<ObjectType>::Add(tObject * object,
   u_int32_t pageID, double distanceRepQuery, double distanceRep,
   double radius, tEntryType type){

   double distance;
   tGenericEntry * newEntry = new tGenericEntry(); // for the new entry.
   tGenericEntry * previousEntry = NULL;  // for the previous entry.
   tGenericEntry * currEntry = this->Head; // for the currenty entry.
   // calculate the distance
   distance = fabs(distanceRepQuery - distanceRep) - radius;
   // if the distance is less than 0, than the distance must be zero.
   if (distance < 0)
      distance = 0;

   // choose the place this entry will be store.
   while ((currEntry!=NULL) && (distance >= currEntry->GetDistance())){
      // browse the list.
      previousEntry = currEntry;
      currEntry = currEntry->GetNext();
   }//end while
   // store the other information about this entry.
   newEntry->SetPageID(pageID);
   newEntry->SetObject(object);
   newEntry->SetMine(true);
   newEntry->SetDistanceRepQuery(distanceRepQuery);
   newEntry->SetDistanceRep(distanceRep);
   newEntry->SetDistance(distance);
   newEntry->SetRadius(radius);
   newEntry->SetType(type);
   newEntry->SetNext(currEntry);
   Occupation++;
   // if the insertion occured in the list head, the Header must point to the
   // new entry.
   if (previousEntry==NULL){
      this->Head = newEntry;
   }else{
      // otherwise, the previous entry must point to this entry.
      previousEntry->SetNext(newEntry);
   }//end if
}//end

//------------------------------------------------------------------------------
// NODE
template <class ObjectType>
void  stGenericPriorityQueue<ObjectType>::Add(tObject * object,
   u_int32_t pageID, double distanceQuery, double radius, tEntryType type){

   double distance;
   tGenericEntry * newEntry = new tGenericEntry(); // for the new entry.
   tGenericEntry * previousEntry = NULL;  // for the previous entry.
   tGenericEntry * currEntry = this->Head; // for the currenty entry.

   // calculate the distance
   distance = distanceQuery - radius;
   // if the distance is less than 0, than the distance must be zero.
   if (distance < 0)
      distance = 0;

   bool stop = (currEntry==NULL);
   // choose the place this entry will be store.
   // this type of entry has less priority than the other types.
   while (!stop){
      if ((currEntry!=NULL) && (distance >= currEntry->GetDistance())){
         if ((distance == currEntry->GetDistance()) &&
             (currEntry->GetType() == APPROXIMATENODE)){
            stop = true;
         }else{
            // browse the list.
            previousEntry = currEntry;
            currEntry = currEntry->GetNext();
         }//end if
      }else{
         // reach the end of the list.
         stop = true;
      }//end if
   }//end while
   // store the other information about this entry.
   newEntry->SetPageID(pageID);
   newEntry->SetObject(object);
   newEntry->SetMine(true);
   newEntry->SetDistanceRepQuery(distanceQuery);
   newEntry->SetDistanceQuery(distanceQuery);
   newEntry->SetDistance(distance);
   newEntry->SetRadius(radius);
   newEntry->SetType(type);
   newEntry->SetNext(currEntry);
   Occupation++;
   // if the insertion occured in the list head, the Header must point to the
   // new entry.
   if (previousEntry==NULL){
      this->Head = newEntry;
   }else{
      // otherwise, the previous entry must point to this entry.
      previousEntry->SetNext(newEntry);
   }//end if
}//end

//------------------------------------------------------------------------------
// APPROXIMATEOBJECT
template <class ObjectType>
void  stGenericPriorityQueue<ObjectType>::Add(tObject * object,
   double distanceRep, double distanceRepQuery, tEntryType type){

   double distance;
   tGenericEntry * newEntry = new tGenericEntry(); // for the new entry.
   tGenericEntry * previousEntry = NULL;  // for the previous entry.
   tGenericEntry * currEntry = this->Head; // for the currenty entry.
   // calculate the distance
   distance = fabs(distanceRepQuery - distanceRep);
   // if the distance is less than 0, than the distance must be zero.
   if (distance < 0)
      distance = 0;

   bool stop = (currEntry==NULL);
   // choose the place this entry will be store.
   // this type of entry has greater priority than the other types,
   // except for the object type.
   while (!stop){
      if ((currEntry!=NULL) && (distance >= currEntry->GetDistance())){
         if ((distance == currEntry->GetDistance()) &&
             ((currEntry->GetType() == APPROXIMATENODE) ||
              (currEntry->GetType() == NODE))){
            stop = true;
         }else{
            // browse the list.
            previousEntry = currEntry;
            currEntry = currEntry->GetNext();
         }//end if
      }else{
         // reach the end of the list.
         stop = true;
      }//end if
   }//end while
   // store the other information about this entry.
   newEntry->SetObject(object);
   newEntry->SetMine(true);
   newEntry->SetDistanceRepQuery(distanceRepQuery);
   newEntry->SetDistanceRep(distanceRep);
   newEntry->SetDistance(distance);
   newEntry->SetType(type);
   newEntry->SetNext(currEntry);
   Occupation++;
   // if the insertion occured in the list head, the Header must point to the
   // new entry.
   if (previousEntry==NULL){
      this->Head = newEntry;
   }else{
      // otherwise, the previous entry must point to this entry.
      previousEntry->SetNext(newEntry);
   }//end if
}//end

//------------------------------------------------------------------------------
// OBJECT
template <class ObjectType>
void  stGenericPriorityQueue<ObjectType>::Add(tObject * object,
   double distanceQuery, tEntryType type){

   double distance;
   tGenericEntry * newEntry = new tGenericEntry(); // for the new entry.
   tGenericEntry * previousEntry = NULL;  // for the previous entry.
   tGenericEntry * currEntry = this->Head; // for the currenty entry.
   distance = distanceQuery;

   // choose the place this entry will be store.
   // this type of entry has greater priority than the other types.
   while ((currEntry!=NULL) && (distance > currEntry->GetDistance())){
      // browse the list.
      previousEntry = currEntry;
      currEntry = currEntry->GetNext();
   }//end while
   // store the other information about this entry.
   newEntry->SetObject(object);
   newEntry->SetMine(true);
   newEntry->SetDistanceQuery(distanceQuery);
   newEntry->SetDistance(distance);
   newEntry->SetType(type);
   newEntry->SetNext(currEntry);
   Occupation++;
   // if the insertion occured in the list head, the Header must point to the
   // new entry.
   if (previousEntry==NULL){
      this->Head = newEntry;
   }else{
      // otherwise, the previous entry must point to this entry.
      previousEntry->SetNext(newEntry);
   }//end if
}//end

//------------------------------------------------------------------------------
template <class ObjectType>
stGenericEntry<ObjectType> * stGenericPriorityQueue<ObjectType>::Get(){
   tGenericEntry * returnEntry = this->Head;
   // get the header
   if (this->Head){
      // the header must point to the next entry.
      this->Head = this->Head->GetNext();
      // decrement one unit from the Occupation
      this->Occupation--;
   }//end if
   return returnEntry;
}//end Get


//----------------------------------------------------------------------------
// class stGenericPriorityHeap
//----------------------------------------------------------------------------
/**
* This class defines node Type of stGenericPriorityHeap.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup util
* @todo Documentation update.
*/

//------------------------------------------------------------------------------
template <class ObjectType>
stGenericPriorityHeap<ObjectType>::~stGenericPriorityHeap(){
   //delete the objects that the queue owns.
   for (int i=0; i<Size; i++){
      if (Entries[i].Mine){
         if (Entries[i].Object != NULL){
            delete Entries[i].Object;
         }//end if
      }//end if
   }//end for
   // delete the rest of the queue.
   delete[] Entries;
}//end ~stGenericPriorityHeap

//----------------------------------------------------------------------------
template < class ObjectType >
void stGenericPriorityHeap<ObjectType>::Add(tObject * object, u_int32_t pageID,
   double distanceQuery, double distanceRep, double distanceRepQuery,
   double radius, u_int32_t height, tType Type){

   int child;
   int parent;
   double distance;

   // Calculate the distance. It depends on the type of the entry.
   switch (Type){
      case OBJECT :
            distance = distanceQuery;
            break;
      case APPROXIMATEOBJECT :
            distance = fabs(distanceRepQuery - distanceRep);
            break;
      case NODE :
            distance = distanceQuery - radius;
            break;
      case APPROXIMATENODE :
            distance = fabs(distanceRepQuery - distanceRep) - radius;
            break;
   }//end switch

   // if the distance is less than 0, than the distance must be zero.
   if (distance < 0){
      distance = 0;
   }//end if

   // Resize me if required.
   if (Size == MaxSize){
      Resize();
   }//end if
   
   child = Size;
   parent = (child - 1) / 2;
   // Do not enable complete boolena evaluation or it will fail.
   while ((child > 0) && (Entries[parent].Distance > distance)){
      // Move parent down.
      // store the other information about this entry.
      Entries[child].PageID = Entries[parent].PageID;
      Entries[child].Object = Entries[parent].Object;
      Entries[child].Mine = Entries[parent].Mine;
      Entries[child].Distance = Entries[parent].Distance;
      Entries[child].DistanceQuery = Entries[parent].DistanceQuery;
      Entries[child].DistanceRep = Entries[parent].DistanceRep;
      Entries[child].DistanceRepQuery = Entries[parent].DistanceRepQuery;
      Entries[child].Radius = Entries[parent].Radius;
      Entries[child].Type = Entries[parent].Type;
      Entries[child].Height = Entries[parent].Height;
      // Next...
      child = parent;
      parent = (child - 1) / 2;
   }//end while
   // If there are some entries that have the same distance from the new entry...
   if ((child > 0) && (Entries[parent].Distance == distance)){
      // If the distance is the same and the type is diferent.... move
      while ((child > 0) && (Entries[parent].Distance == distance) &&
             (Entries[parent].Type > Type)){
         // Move parent down.
         Entries[child].PageID = Entries[parent].PageID;
         Entries[child].Object = Entries[parent].Object;
         Entries[child].Mine = Entries[parent].Mine;
         Entries[child].Distance = Entries[parent].Distance;
         Entries[child].DistanceRep = Entries[parent].DistanceRep;
         Entries[child].DistanceQuery = Entries[parent].DistanceQuery;
         Entries[child].DistanceRepQuery = Entries[parent].DistanceRepQuery;
         Entries[child].Radius = Entries[parent].Radius;
         Entries[child].Type = Entries[parent].Type;
         Entries[child].Height = Entries[parent].Height;
         // Next...
         child = parent;
         parent = (child - 1) / 2;
      }//end while
      // If the distance and the type is the same and the height is diferent....
      // move until the height is less than the new entry
      // The object is the low level in the tree has greater values of Height
      // than the entries in high levels.
      while ((child > 0) && (Entries[parent].Distance == distance) &&
             (Entries[parent].Type == Type) && (Entries[parent].Height < height)){
         // Move parent down.
         Entries[child].PageID = Entries[parent].PageID;
         Entries[child].Object = Entries[parent].Object;
         Entries[child].Mine = Entries[parent].Mine;
         Entries[child].Distance = Entries[parent].Distance;
         Entries[child].DistanceRep = Entries[parent].DistanceRep;
         Entries[child].DistanceQuery = Entries[parent].DistanceQuery;
         Entries[child].DistanceRepQuery = Entries[parent].DistanceRepQuery;
         Entries[child].Radius = Entries[parent].Radius;
         Entries[child].Type = Entries[parent].Type;
         Entries[child].Height = Entries[parent].Height;
         // Next...
         child = parent;
         parent = (child - 1) / 2;
      }//end while
   }//end if

   // Add in the proper position
   Entries[child].PageID = pageID;
   Entries[child].Object = object;
   Entries[child].Mine = true;
   Entries[child].Distance = distance;
   Entries[child].DistanceRep = distanceRep;
   Entries[child].DistanceQuery = distanceQuery;
   Entries[child].DistanceRepQuery = distanceRepQuery;
   Entries[child].Radius = radius;
   Entries[child].Height = height;
   Entries[child].Type = Type;
   // Increment the size.
   Size++;
}//end stGenericPriorityHeap::Add

//------------------------------------------------------------------------------
template < class ObjectType >
bool stGenericPriorityHeap<ObjectType>::Get(tObject * & object, u_int32_t & pageID,
   double & distanceQuery, double & distanceRep, double & distanceRepQuery,
   double & radius, u_int32_t & height, tType & Type){
   int child;
   int parent;

   // If there is a element.
   if (Size > 0){
      // Remove first
      Entries[0].Mine = false;
      pageID = Entries[0].PageID;
      object = Entries[0].Object;
      distanceRep = Entries[0].DistanceRep;
      distanceQuery = Entries[0].DistanceQuery;
      distanceRepQuery = Entries[0].DistanceRepQuery;
      radius = Entries[0].Radius;
      Type = Entries[0].Type;
      height = Entries[0].Height;
      Size--;

      // Reinsert last.
      parent = 0;
      child = GetMinChild(parent);
      // Do not enable complete boolena evaluation or it will fail.
      while ((child >= 0) && (Entries[Size].Distance > Entries[child].Distance)){
         // Move child up
         Entries[parent].PageID = Entries[child].PageID;
         Entries[parent].Object = Entries[child].Object;
         Entries[parent].Mine = Entries[child].Mine;
         Entries[parent].Distance = Entries[child].Distance;
         Entries[parent].DistanceRep = Entries[child].DistanceRep;
         Entries[parent].DistanceQuery = Entries[child].DistanceQuery;
         Entries[parent].DistanceRepQuery = Entries[child].DistanceRepQuery;
         Entries[parent].Radius = Entries[child].Radius;
         Entries[parent].Type = Entries[child].Type;
         Entries[parent].Height = Entries[child].Height;
         parent = child;
         child = GetMinChild(parent);
      }//end while
      // If there are some entries that have the same distance from the new entry...
      if ((child >= 0) && (Entries[Size].Distance == Entries[child].Distance)){
         // If the distance is the same and the type is diferent.... move
         while ((child >= 0) && (Entries[Size].Distance == Entries[child].Distance) &&
                (Entries[Size].Type > Entries[child].Type)){
            // Move child up
            Entries[parent].PageID = Entries[child].PageID;
            Entries[parent].Object = Entries[child].Object;
            Entries[parent].Mine = Entries[child].Mine;
            Entries[parent].Distance = Entries[child].Distance;
            Entries[parent].DistanceRep = Entries[child].DistanceRep;
            Entries[parent].DistanceQuery = Entries[child].DistanceQuery;
            Entries[parent].DistanceRepQuery = Entries[child].DistanceRepQuery;
            Entries[parent].Radius = Entries[child].Radius;
            Entries[parent].Type = Entries[child].Type;
            Entries[parent].Height = Entries[child].Height;
            parent = child;
            child = GetMinChild(parent);
         }//end while
         // If the distance and the type is the same and the height is diferent....
         // move until the height is less than the new entry
         // The object is the low level in the tree has greater values of Height
         // than the entries in high levels.
         while ((child >= 0) && (Entries[Size].Distance == Entries[child].Distance) &&
                (Entries[Size].Type == Entries[child].Type) && (Entries[Size].Height < Entries[child].Height)){
            // Move child up
            Entries[parent].PageID = Entries[child].PageID;
            Entries[parent].Object = Entries[child].Object;
            Entries[parent].Mine = Entries[child].Mine;
            Entries[parent].Distance = Entries[child].Distance;
            Entries[parent].DistanceRep = Entries[child].DistanceRep;
            Entries[parent].DistanceQuery = Entries[child].DistanceQuery;
            Entries[parent].DistanceRepQuery = Entries[child].DistanceRepQuery;
            Entries[parent].Radius = Entries[child].Radius;
            Entries[parent].Type = Entries[child].Type;
            Entries[parent].Height = Entries[child].Height;
            parent = child;
            child = GetMinChild(parent);
         }//end while
      }//end if
      
      // Put it in place.
      Entries[parent].PageID = Entries[Size].PageID;
      Entries[parent].Object = Entries[Size].Object;
      Entries[parent].Mine = Entries[Size].Mine;
      Entries[parent].Distance = Entries[Size].Distance;
      Entries[parent].DistanceRep = Entries[Size].DistanceRep;
      Entries[parent].DistanceQuery = Entries[Size].DistanceQuery;
      Entries[parent].DistanceRepQuery = Entries[Size].DistanceRepQuery;
      Entries[parent].Radius = Entries[Size].Radius;
      Entries[parent].Type = Entries[Size].Type;
      Entries[parent].Height = Entries[Size].Height;
      return true;
   }else{
      // Empty!
      return false;
   }//end if         
}//end stGenericPriorityHeap::Get

//----------------------------------------------------------------------------
template < class ObjectType >
void stGenericPriorityHeap<ObjectType>::Resize(){
   tGenericEntry * newEntries;
   
   // New entry vector
   newEntries = new tGenericEntry[MaxSize + Increment];
   memcpy(newEntries, Entries, sizeof(tGenericEntry) * Size);
   //delete the old entries
   delete[] Entries;
   Entries = newEntries;
   this->MaxSize += Increment;
}//end stGenericPriorityHeap::Resize
