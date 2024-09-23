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
* This file is the implementation of stMGrid methods.
*
* @version 1.0
* @author Adriano Arantes Paterlini (paterlini@gmail.com) 
*/

//-----------------------------------------------------------------------------
// class stMGridOrigNode
//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMGridOrigNode<ObjectType, EvaluatorType>::stMGridOrigNode(stPage * page, bool create){

   this->Page = page;

   // Will I create it ?
   if (create){
      Page->Clear();
   }//end if

   // Set elements
   this->Header = (stNodeHeader *) Page->GetData();
   this->Entries = (u_int32_t *)(Page->GetData() + sizeof(stNodeHeader));
}//end stMGridOrigNode::stMGridOrigNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMGridOrigNode<ObjectType, EvaluatorType>::AddEntry(u_int32_t size, const unsigned char * object){
   u_int32_t totalsize;
   u_int32_t offs;

   totalsize = size + sizeof(u_int32_t);
   if (totalsize <= GetFree()){
      // Object offset
      if (Header->Occupation == 0){
         offs = Page->GetPageSize() - size;
      }else{
         offs = Entries[Header->Occupation - 1] - size;
      }//end if

      // Write object
      Page->Write((unsigned char*)object, size, offs);

      // Update entry
      Entries[Header->Occupation] = offs;

      // Update header
      Header->Occupation++;
      return Header->Occupation - 1;
   }else{
      return -1;
   }//end if
}//end stMGridOrigNode::AddEntry

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
const unsigned char * stMGridOrigNode<ObjectType, EvaluatorType>::GetObject(int idx){

   return Page->GetData() + Entries[idx];
}//end stMGridOrigNode::GetObject

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stMGridOrigNode<ObjectType, EvaluatorType>::GetObjectSize(int idx){

   if (idx == 0){
      return Page->GetPageSize() - Entries[0];
   }else{
      return Entries[idx - 1] - Entries[idx];
   }//end if
}//end stMGridOrigNode::GetObjectSize

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMGridOrigNode<ObjectType, EvaluatorType>::RemoveEntry(u_int32_t idx){
   u_int32_t i;
   u_int32_t lastID;
   u_int32_t rObjSize;

   // Programmer's note: This procedure is simple but tricky! See the
   // stDBMNode structure documentation for more details.

   #ifdef __stDEBUG__
   if (idx >= (int )GetNumberOfEntries()){
      // Oops! This id doesn't exists.
      throw range_error("Invalid idx!");
   }//end if
   #endif //__stDEBUG__

   // Let's remove
   lastID = Header->Occupation - 1; // The id of the last object. This
                                    // value will be very useful.
   // Do I need to move something ?
   if (lastID != idx){
      // Yes, I do. Save the removed object size
      rObjSize = GetObjectSize(idx);

      // Let's move objects first. We will use memmove() from stdlib because
      // it handles the overlap between src and dst. Remember that src is the
      // offset of the last object and the dst is the offset of the last
      // object plus removed object size.
      memmove(Page->GetData() + Entries[lastID] + rObjSize,
              Page->GetData() + Entries[lastID],
              Entries[idx] - Entries[lastID]);
      // Let's move entries...
      for (i = idx; i < lastID; i++){
         // Copy all fields with memcpy (it's faster than field copy).
         memcpy(Entries + i, Entries + i + 1, sizeof(u_int32_t));
         // Update offset by adding the removed object size to it. It will
         // reflect the previous move operation.
         Entries[i] += rObjSize;
      }//end for
   }//end if

   // Update counter...
   Header->Occupation--;
}//end stMGridOrigNode::RemoveEntry

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stMGridOrigNode<ObjectType, EvaluatorType>::GetFree(){

   if (Header->Occupation == 0){
      return Page->GetPageSize() - sizeof(stNodeHeader);
   }else{
      return Page->GetPageSize() - sizeof(stNodeHeader) -
            (sizeof(u_int32_t) * Header->Occupation) -
            (Page->GetPageSize() - Entries[Header->Occupation - 1]);
   }//end if
}//end stMGridOrigNode::GetFree




//==============================================================================
// Class stMGridLogicNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMGridLogicNode<ObjectType, EvaluatorType>::stMGridLogicNode(stPageManager * pageman){
   int i;

   myPageManager = pageman; 

   if (this->myPageManager->IsEmpty()){
      // Create it
      this->Create();
      // Default values
      Header->ObjectCount = 0;
      Header->NodeCount = 0;
      Header->MaxOccupation = 0;
   }else{
      // Use it
      this->LoadHeader();
   }//end if

   Entries = new stIndexEntry[40000]; //@warning, should not be static

   
}//end stMGridLogicNode<ObjectType, EvaluatorType>::stMGridLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMGridLogicNode<ObjectType, EvaluatorType>::~stMGridLogicNode(){
   int i;

//    if (Entries != NULL){
//       for (i = 0; i < Count; i++){
//          if ((Entries[i].Object != NULL) && (Entries[i].Mine)){
//             delete Entries[i].Object;
//             delete Entries[i].FieldDistance;
//          }//end if
//       }//end for
//    }//end if
//    // Clean before exit.
   delete[] Entries;
}//end stMGridLogicNode<ObjectType, EvaluatorType>::~stMGridLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMGridLogicNode<ObjectType, EvaluatorType>::AddEntry(tObject * obj){
   // The object will be added in the first page.
   // When it is full, it will create a new page and link
   // it in the begining of the list.
   //stPage * currPage;
   //stDummyNode * currNode;
   bool overflow;
   u_int32_t nextPageID;
   int id;

   // Does it fit ?
   if (obj->GetSerializedSize() > this->myPageManager->GetMinimumPageSize() - 16){
      return false;
   }//end if

   // Adding object
   if (this->GetRoot() == 0){
      overflow = true;
      nextPageID = 0;
   }else{
      // Get node
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = new tOrigNode(currPage);

      // Try to add
      id = currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
      if (id >= 0){
         // I was able to add.
         this->myPageManager->WritePage(currPage);
         overflow = false;
         
         Entries[Header->ObjectCount].ObjPageID = currPage->GetPageID();
         Entries[Header->ObjectCount].ObjIdx = id;

         // update the maximum number of entries.
         //this->SetMaxOccupation(currNode->GetNumberOfEntries());
      }else{
         // Oops! Overflow!
         overflow = true;
         nextPageID = currPage->GetPageID();
      }//end if
         
      // Clear the mess
      delete currNode;
      currNode = NULL;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   // Manage overflows
   if (overflow){
      // Oops! New root required !
      currPage = this->myPageManager->GetNewPage();
      currNode = new tOrigNode(currPage, true);
      currNode->SetNextNode(nextPageID);
      // Update the number of nodes.
      Header->NodeCount++;

      // I'll add it here
      id = currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
      
      // Write the new node
      this->myPageManager->WritePage(currPage);

      // Update "tree" state
      this->SetRoot(currPage->GetPageID());
      WriteHeader();
      
      Entries[Header->ObjectCount].ObjPageID = currPage->GetPageID();
      Entries[Header->ObjectCount].ObjIdx = id;

      // Clear the mess again
      delete currNode;
      currNode = NULL;
      this->myPageManager->ReleasePage(currPage);
   }//end overflow
    

   //Update the number of objects.
   UpdateObjectCounter(1);

   // Write Header!
   WriteHeader();

   return true;
}//end stMGridLogicNode<ObjectType><EvaluatorType>::Add

template <class ObjectType, class EvaluatorType>
ObjectType * stMGridLogicNode<ObjectType, EvaluatorType>::GetObject(u_int32_t index){

ObjectType * Object;

Object = new ObjectType();

if (currNode == NULL){
   currPage = this->myPageManager->GetPage(Entries[index].ObjPageID);
   currNode = new tOrigNode(currPage);
}else if (currNode->GetPageID() != Entries[index].ObjPageID){
   delete currNode;
   currNode = NULL;
   this->myPageManager->ReleasePage(currPage);
   currPage = this->myPageManager->GetPage(Entries[index].ObjPageID);
   currNode = new tOrigNode(currPage);
}

Object->Unserialize(currNode->GetObject(Entries[index].ObjIdx), currNode->GetObjectSize(Entries[index].ObjIdx));

return Object;
}//end stMGrid<ObjectType, EvaluatorType>::GetObject


//==============================================================================
// Class stMGridPivot
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMGridPivot<ObjectType, EvaluatorType>::stMGridPivot(u_int32_t nFocus, 
   u_int32_t nRings, EvaluatorType * metricEvaluator){
   
   NumFocusDim = 1;

   // Number of Focus
   NumFocus = nFocus * NumFocusDim;

   NumDim = nFocus;   

   // Number of Rings
   NumRings = nRings;
   
   fociBase = new stMGridPivotEntry[NumFocus];

   

   // To calculate distances
   myMetricEvaluator = metricEvaluator;
}//end stMGridPivot::stMGridPivot

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::FindPivot(tLogicNode * logicNode, u_int32_t numObject, int type){

  //find the pivots using HFFastMap
   if (type == 0){
      execHFFastMap(logicNode, numObject);
   }else{
      execRandomPivots(logicNode, numObject);
   }
}//end stMGridPivot::FindPivot

//------------------------------------------------------------------------------
// execute the HF algorithm to find the foci base based on the FastMap Algorithm
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::execHFFastMap(tLogicNode * logicNode, u_int32_t numObject) {
   double edge, error, bestError;
   long i, nFound, bestCandidate1, bestCandidate2;
   int steps;

   // set the first object as the base object to find the first focus
   nFound = 0;
   bestCandidate1 = 0;
   bestCandidate2 = 0;
   error = 0;
   bestError = 0;
   tObject *tmp, *best1, *best2;

   for (steps = 0; steps < 5; steps++) {
      // traverse dataset and get first foci
      best1 = logicNode->GetObject(bestCandidate1);
      for (i = 0; i < numObject; i++) {
         // compare each object with actual and keep if it is furthest than the
         // actual furthest
         tmp = logicNode->GetObject(i);
         error = myMetricEvaluator->GetDistance(tmp, best1);         
         if (error > bestError) {
            bestCandidate2 = i;
            bestError = error;
         }//end if
         delete tmp;
      }//end for
      // traverse dataset and get first foci
      best2 = logicNode->GetObject(bestCandidate2);
      for (i = 0; i < numObject; i++) {
         // compare first object with actual and keep if it is furthest than the
         // actual furthest
         tmp = logicNode->GetObject(i);
         error = myMetricEvaluator->GetDistance(tmp, best2);
         if (error > bestError) {
            bestCandidate1 = i;
            bestError = error;
         }//end if
         delete tmp;
      }//end for
      delete best1;
      delete best2;
   }//end for
   //std::cout << "\n Pivots: ";
   //@warning I really need this if state here?
   if (nFound < NumFocus) {
      // insert first focus and set it as pivot
      fociBase[nFound].Object = logicNode->GetObject(bestCandidate1);
      fociBase[nFound].idx = bestCandidate1;
      logicNode->SetIsPivot(bestCandidate1);
      nFound++;
      //std::cout << bestCandidate1;
      // insert second focus nd set it as pivot
      fociBase[nFound].Object = logicNode->GetObject(bestCandidate2);
      fociBase[nFound].idx = bestCandidate2;
      logicNode->SetIsPivot(bestCandidate2); 
      nFound++;
      //std::cout << " " << bestCandidate2;
      // find the remaining focus
      edge = myMetricEvaluator->GetDistance(fociBase[0].Object,fociBase[1].Object);
      execHFRemainingFastMap(logicNode, numObject, edge, nFound);
   }//end if
}//end execHFFastMap

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::execHFRemainingFastMap(tLogicNode * logicNode, u_int32_t numObject,
           double edge, int nFound) {
  double error, bestError, delta;
  long i, j, bestCandidate;
  tObject *tmp;
  // find the remaining foci
  while (nFound < NumFocus) {
    bestError = (double) MAXINT;
    bestCandidate = 0;
    for(i = 0; i < numObject; i++) {
      if (logicNode->GetIsPivot(i) == false){
         tmp = logicNode->GetObject(i);
         error = 0;
         // calculates the error
         for(j = 0; j < nFound; j++) {
         delta = edge - myMetricEvaluator->GetDistance(fociBase[j].Object, tmp);
         if (delta < 0){
            delta *= -1;
         }//end if
         error += delta;
         }//end for

         // compares with actual best and keep if it is better
         if (error < bestError) {
         bestCandidate = i;
         bestError = error;
         }
         delete tmp; 
      }
    }

    // insert first focus and set it as pivot
    fociBase[nFound].Object = logicNode->GetObject(bestCandidate);
    fociBase[nFound].idx = bestCandidate;
    logicNode->SetIsPivot(bestCandidate);
    //std::cout << " " << bestCandidate; 
    nFound++;
  }
}//end execHFRemainingFastMap

template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::execRandomPivots(tLogicNode * logicNode, u_int32_t numObject) {
  
   int nFound = 0;
   int iR;
   srand(20); 
   //srand(125); 
   // find the remaining foci
   std::cout << " Pt: " << std::flush;
   while ( nFound < NumFocus ) {
      //bestError = (double) MAXINT;
      //bestCandidate = 0;    
      iR = rand()%numObject;
      //for(i = 0; i < numObject; i++) {
      if (logicNode->GetIsPivot(iR) == false){
         fociBase[nFound].Object = logicNode->GetObject(iR);
         fociBase[nFound].idx = iR;
         logicNode->SetIsPivot(iR);
         nFound++;
         std::cout << iR << " ";
      }//end if
      
   }//end while
}//end execRandomPivots

template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::FindRings(
   tLogicNode * logicNode, u_int32_t numObject, int type){
   tObject *tmp;
   // type == 0 put rings at mean
   if (type == 0){					
      double FocusDistance[numObject];
      u_int32_t numObjectsRing;      
      numObjectsRing = numObject / NumRings;

      for (int k = 0; k < NumFocus; k++){
         fociBase[k].nrings = NumRings;
         fociBase[k].RingDistance = new double[fociBase[k].nrings]; 
         for (int i = 0; i < numObject; i++){   
            tmp = logicNode->GetObject(i);
            FocusDistance[i] = myMetricEvaluator->GetDistance(fociBase[k].Object, tmp);
            delete tmp;
         }
         sort(FocusDistance, 0, numObject-1);
         // get the distances to each of the rings 
         for(int j=1; j < NumRings; j++) {
           fociBase[k].RingDistance[j-1] = FocusDistance[j * numObjectsRing];
         } // end for j
    
         // want to be sure the last ring distance is the largest distance
         fociBase[k].RingDistance[NumRings-1] = FocusDistance[numObject-1];
      }
   // type == 1 put rings equally separated
   }else if (type == 1){
      double MaxDistance, CurrDistance;
      for (int k = 0; k < NumFocus; k++){
         MaxDistance = 0;
         fociBase[k].nrings = NumRings;
         fociBase[k].RingDistance = new double[fociBase[k].nrings];
         for (int i = 0; i < numObject; i++){   
            tmp = logicNode->GetObject(i);
            CurrDistance = myMetricEvaluator->GetDistance(fociBase[k].Object, tmp);
            if ( CurrDistance > MaxDistance){
	            MaxDistance = CurrDistance;
            }
            delete tmp;
         }
         for(int j=1; j <= NumRings; j++) {
            fociBase[k].RingDistance[j-1] = (j * MaxDistance)/NumRings;
         } // end for j
      }//end for k
   // type == 2 histogram based rings
   }else{
      int numbins = 200;
      long Histogram[numbins+1];
      long bin;
      double edge;
      double tmpdist, MaxDistance;
      double binsize;
      double TmpRingDistance[256];
      int LastNegIdx;
      double deriv;
      edge = myMetricEvaluator->GetDistance(fociBase[0].Object,fociBase[1].Object);
      binsize = (double )edge / (double) numbins;
      
      
      for (int k = 0; k < NumFocus; k++){
         //std::cout << "\n Focus " << k << ": "; 
         MaxDistance = 0;
         for (int b = 0; b <= numbins; b++){
            Histogram[b] = 0;
         }
         for (int i = 0; i < numObject; i++){   
            tmp = logicNode->GetObject(i);
            tmpdist = myMetricEvaluator->GetDistance(fociBase[k].Object, tmp);
            if ( tmpdist > MaxDistance){
	            MaxDistance = tmpdist;
            }            
            bin = (long) ((tmpdist * numbins) / edge);
            if (bin > numbins){
               bin = numbins;
               //std::cout << "! ";
            }
            Histogram[bin]++;
            delete tmp;
         }

         //debug
         //for (int b = 0; b <= numbins; b++){
         //   std::cout << Histogram[b] << " ";
         //}
         //std::cout << "\n dev: ";
         LastNegIdx = -1;
         fociBase[k].nrings = 0;
         for (int b = 1; b <= numbins; b++){
            deriv = (Histogram[b] - Histogram[b-1])/binsize;
            if (deriv > 0 && LastNegIdx != -1){
               TmpRingDistance[fociBase[k].nrings] = ((b + LastNegIdx)/2) * binsize;
               fociBase[k].nrings++;
               LastNegIdx = -1;
            }
            if (deriv < 0){
               LastNegIdx = b;   
            }
            //std::cout << deriv << " ";
         }
         TmpRingDistance[fociBase[k].nrings] = MaxDistance;
         fociBase[k].nrings++;
         fociBase[k].RingDistance = new double[fociBase[k].nrings];
         //memcpy(fociBase[k].RingDistance, TmpRingDistance, (fociBase[k].nrings * double) );
         //std::cout << "\n Ring Dist: ";
         for (int r = 0; r < fociBase[k].nrings; r++){
            fociBase[k].RingDistance[r] = TmpRingDistance[r];
            //std::cout << fociBase[k].RingDistance[r] << " ";
         }
         //sort(FocusDistance, 0, numObject-1);
         // get the distances to each of the rings 
         //for(int j=1; j < NumRings; j++) {
         //  fociBase[k].RingDistance[j-1] = FocusDistance[j * numObjectsRing];
         //} // end for j
    
         // want to be sure the last ring distance is the largest distance
         //fociBase[k].RingDistance[NumRings-1] = FocusDistance[numObject-1];
      }
   }//end if-else
}

template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::sort(double *focus_dist, long l, long numObject){
  double pivot = focus_dist[numObject];
  long i = l-1;
  long j = numObject;
  double temp;
  if(numObject > l) {
    do{
      do i++; while (focus_dist[i] < pivot && i < numObject);
	   do j--; while (focus_dist[j] > pivot && j > l);
	   temp = focus_dist[i];
	   focus_dist[i] = focus_dist[j];
	   focus_dist[j] = temp;
    }while (j>i);
    focus_dist[j] = focus_dist[i];
    focus_dist[i] = pivot;
    focus_dist[numObject] = temp;
    sort(focus_dist, l, i-1);
    sort(focus_dist, i+1, numObject);
  }
}

//---------------------------------------------------------------------------


template <class ObjectType, class EvaluatorType>
void stMGridPivot<ObjectType, EvaluatorType>::BuildFieldDistance(
      ObjectType * object, double * fieldDistance) {
   
   double tmpD, minD;
   for (int k = 0; k < NumDim; k++){
      minD = (double) MAXINT;
      for (int d = 0; d < NumFocusDim; d++){
         tmpD = myMetricEvaluator->GetDistance(fociBase[k+d].Object, object);
         if (tmpD < minD){
            minD = tmpD;
         }
      }
      fieldDistance[k] = minD;
   }//end for
}//end stMGridPivot::BuildFieldDistance

//---------------------------------------------------------------------------

//==============================================================================
// Class stMGrid
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMGrid<ObjectType, EvaluatorType>::stMGrid(stPageManager * pageman, stPageManager * pagemanD, 
   stPageManager * pagemanO, u_int32_t nFocus, u_int32_t nRings, u_int32_t nClusters, 
   EvaluatorType * metricEvaluator ){
   
   // Number of Focus
   NumFocus = nFocus;
   // Number of Rings
   NumRings = nRings;
   
   NumClusters = nClusters;
   
   myPageManager = pageman;
   
   myPageManagerD = pagemanD;

   myPageManagerO = pagemanO;      
   
   myMetricEvaluator = metricEvaluator;

   myBasicMetricEvaluator = new tBasicMetricEvaluator;
   
   LogicNode = new tLogicNode(pageman);
   
   OrderNode = new tLogicNode(pagemanO);

   DistanceNode = new tDistanceNode(pagemanD);      
   
   MGridPivot = new tMGridPivot(nFocus, nRings, metricEvaluator);

   //@warning consider nRings + 1 again
   //NumCells = (u_int32_t) pow( (double) (nRings) , (double) nFocus );

   //CellArray = new stMGridCellArray[NumCells];
   
   Clusters = new stMGridCluster[NumClusters];
   
   for (int k = 0 ; k < NumClusters; k++){
      Clusters[k].Sum = new double[NumFocus];
   }
   
}//end stMGridPivot::stMGridPivot

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool stMGrid<ObjectType, EvaluatorType>::Add(tObject * obj){
   //@warning eliminate this function, back compatiblity 
   LogicNode->AddEntry(obj);
}//end stMGrid<ObjectType><EvaluatorType>::Add

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
ObjectType * stMGrid<ObjectType, EvaluatorType>::GetObject(u_int32_t index){

   return LogicNode->GetObject(index);
}//end stMGrid<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double * stMGrid<ObjectType, EvaluatorType>::GetDistance(u_int32_t index){

   return DistanceNode->GetObject(index)->GetData();
}//end stMGrid<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::BuildAllDistance(){
   
   tObject *tmp;
   double * FieldDistance;
   tBasicArrayObject * ObjectD;
   
   FieldDistance = new double[NumFocus];
   
   for (int i = 0; i < this->GetNumberOfObjects(); i++){
      tmp = LogicNode->GetObject(i);
      MGridPivot->BuildFieldDistance(tmp, FieldDistance);

         //debug
         //for(int f = 0; f < NumFocus; f++){
         //  std::cout << FieldDistance[f] << " ";
         //}
         //std::cout << "\n" << std::flush;
         
      ObjectD = new tBasicArrayObject(NumFocus, FieldDistance);
      DistanceNode->AddEntry(ObjectD);
      delete tmp;
   }
   delete FieldDistance;

}//end stMGrid<ObjectType, EvaluatorType>::BuildAllDistance()

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::AllDistance(){
   
   tObject *ObjectO, *ObjO;
   tBasicArrayObject * ObjectD, *ObjD;
   double d1, d2;
   double tmp, c2 = 1, c1 = 1;

   for (int i = 0; i < this->GetNumberOfObjects(); i++){
      ObjectO = LogicNode->GetObject(i);
      ObjectD = DistanceNode->GetObject(i);
      for (int j = 0; j < this->GetNumberOfObjects(); j++){
         ObjO = LogicNode->GetObject(j);
         d1 = myMetricEvaluator->GetDistance(ObjectO, ObjO);
         ObjD = DistanceNode->GetObject(j);
         d2 = myBasicMetricEvaluator->GetDistance(ObjectD, ObjD);
         if (d2 > d1){
            tmp = d2/d1;
            if ( tmp > c2)
               c2 = tmp;
         }else{
            tmp = d1/d2;
            if ( tmp > c1)
               c1 = tmp;
         }
         
         delete ObjO;
         delete ObjD;    
      }
      delete ObjectO;
      delete ObjectD;
   }
   std::cout << "c1: " << c1 << " c2: " << c2 << " c1*c2: " << c1*c2 << std::endl;

}//end stMGrid<ObjectType, EvaluatorType>::BuildAllDistance()

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::Cluster(){

   double * tmp, * mean;
   tBasicArrayObject * ObjectD;
   u_int32_t cluster;
   double minDistance, Distance;
   
   mean = new double[NumFocus];
   //clone first NumCluster objects, to be the initial means.
   for (int k = 0 ; k < NumClusters ; k++){
      int test = 1000;
      int idx = k*test;
      Clusters[k].ClusterMean = DistanceNode->GetObject(idx);
      Clusters[k].maxDist = 0;
   }

   int n = 0;
   while( n < 15){ //@todo acrescentar teste de convergencia
      for (int k = 0; k < NumClusters ; k++){
         Clusters[k].Count = 0;         
         for (int h = 0; h < NumFocus ; h++){
            Clusters[k].Sum[h] = 0;            
         }
      }
      //pass throw all objects and put it into a cluster
      for (int i = 0; i < this->GetNumberOfDistances(); i++){
         minDistance = (double) MAXINT;
         for (int k = 0 ; k < NumClusters ; k++){
            Distance = myBasicMetricEvaluator->GetDistance(Clusters[k].ClusterMean, DistanceNode->GetObject(i));
            if (Distance < minDistance){
               cluster = k;
               minDistance = Distance;
               DistanceNode->SetCluster(i, k);
               LogicNode->SetCluster(i, k);
            }//end if (Distance < minDistance)
            
         }//end for k         
         tmp = this->GetDistance(i);
         Clusters[cluster].Count += 1;
         for (int h = 0; h < NumFocus ; h++){
            Clusters[cluster].Sum[h] += tmp[h];
         }//end for h
      }//end for i
      for (int k = 0 ; k < NumClusters ; k++){
         if (Clusters[k].Count > 1){
            for (int h = 0; h < NumFocus ; h++){
               mean[h] = Clusters[k].Sum[h]/Clusters[k].Count;
            }//end for h
            Clusters[k].ClusterMean->SetData(mean);   
         }//end for if
      }//end for k      
      n++;
   }//end while
   
   for (int i = 0; i < this->GetNumberOfDistances(); i++){
         cluster = DistanceNode->GetCluster(i);
         Distance = myBasicMetricEvaluator->GetDistance(Clusters[cluster].ClusterMean, DistanceNode->GetObject(i));
         if (Distance > Clusters[cluster].maxDist){
            Clusters[cluster].maxDist = Distance;      
         }
   }

  
    
}//end stMGrid<ObjectType, EvaluatorType>::Cluster()

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::BuildCCArray(){
   
   //tObject *tmp;
   int cell;
   
   double Distance, minDistance, cDistance;
   tBasicArrayObject * ObjectD;
  
   contribution = new int[NumFocus];
     
   contribution[0] = 1;
   NumCells = 1;
   for (int h = 1; h < NumFocus; h++){
      contribution[h] = contribution[h-1]*MGridPivot->GetNumRings(h-1);      
      NumCells = NumCells * MGridPivot->GetNumRings(h-1);      
   }
   
   NumCells = NumCells * MGridPivot->GetNumRings(NumFocus-1);
   //std::cout << "\n NumCells: " << NumCells;
   CellArray = new stMGridCellArray[NumCells];

   //std::cout << " contributing " << std::flush;

   for(int c=0; c < NumCells; c++){
      CellArray[c].NumObjects = 0;
      CellArray[c].distance = 0;
   }
  
   for (int i = 0; i < this->GetNumberOfObjects(); i++){
      cell = 0;
      ObjectD = DistanceNode->GetObject(i);
      for (int h = 0; h < NumFocus; h++){
         for (int r = 0; r < (MGridPivot->GetNumRings(h) - 1); r++){
            if (ObjectD->Get(h) > MGridPivot->GetRingDistance(h,r)){
               cell += contribution[h];   
            }//end if
         }//end for r
      }//end for h 
      if ( CellArray[cell].NumObjects == 0){
         this->AllocCell(cell);
      }  
      CellArray[cell].NumObjects += 1;
      DistanceNode->SetCell(i,cell);
      cDistance = myBasicMetricEvaluator->GetDistance(ObjectD, CellArray[cell].CellCenter);
      if (cDistance > CellArray[cell].distance){
         CellArray[cell].distance = cDistance;
      }
      delete ObjectD;
   }//end for i   
   
   
   //std::cout << "Built Cell Array";
 

}//end stMGrid<ObjectType, EvaluatorType>::BuildCCArray()

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::AllocCell(int c){
   int *in;
   double * center;

   in = new int[NumFocus];
   center = new double[NumFocus];

   GetCell(c, in);
   
   for (int h = 0; h < NumFocus; h++){
      if(in[h] == 0) {
	      center[h] = MGridPivot->GetRingDistance(h, in[h])/2;
      }else{
	      center[h] = (MGridPivot->GetRingDistance(h, in[h]) + MGridPivot->GetRingDistance(h, in[h]-1))/2;
      }// end if-else      
   } // end for h
   CellArray[c].CellCenter = new tBasicArrayObject(NumFocus, center);      
   CellArray[c].first = -1;
   CellArray[c].last = -2;   

   delete in;
   delete center;
}

//------------------------------------------------------------------------------


template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::Order(int type){

int o = 0;

switch (type){
   case 0:  //cluster
      for (int k = 0 ; k < NumClusters ; k++){
         for (int i = 0; i < this->GetNumberOfDistances(); i++){
            if (DistanceNode->GetCluster(i) == k){
               OrderNode->AddEntry(LogicNode->GetObject(i));
               OrderNode->SetCluster(o,k);
               //OrderNode->SetCell(o,DistanceNode->GetCell(i));
               o++;                          
            }//end if
         }//end for i         
      }//end for k
      break;
   case 1:  //cell      
      for(int c=0; c < NumCells; c++){
         for (int i = 0; i < this->GetNumberOfDistances(); i++){
            if (DistanceNode->GetCell(i) == c){
               if (CellArray[c].first == -1){
                  CellArray[c].first = o;
               }
               CellArray[c].last = o;
               OrderNode->AddEntry(LogicNode->GetObject(i));
               OrderNode->SetCell(o,c);
               o++;                          
            }//end if
         }//end for i       
      }
      break;
   case 2:  //cluster cell
      for (int k = 0 ; k < NumClusters ; k++){
         for(int c=0; c < NumCells; c++){
            for (int i = 0; i < this->GetNumberOfDistances(); i++){
               if (DistanceNode->GetCluster(i) == k && DistanceNode->GetCell(i) == c){
                  OrderNode->AddEntry(LogicNode->GetObject(i));
                  OrderNode->SetCluster(o,k);
                  OrderNode->SetCell(o,c);
                  //OrderNode->SetCell(o,DistanceNode->GetCell(i));
                  o++;                          
               }//end if
            }//end for i         
         }//end for c
      }//end for k      
      break;
}


}
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::GetCell(int idx, int *in) {

  for(int j=NumFocus-1; j>=0; j--){
    in[j] = idx/contribution[j];
    idx = idx%contribution[j];
  }
}//end stMGrid<ObjectType, EvaluatorType>::GetCell

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::FindCells(int px, int idxc){

   int idxnew;

   if ( minD[px] < MGridPivot->GetRingDistance(px,0) ){
      idxnew = idxc;
      if ((px + 1) < NumFocus){
         FindCells((px + 1), idxnew);   
      }else{
         cclist[countcc] = idxnew;
         countcc++;   
      }
   }   

   for (int r = 1 ; r < NumRings; r++){
      if ( minD[px] < MGridPivot->GetRingDistance(px,r) && maxD[px] > MGridPivot->GetRingDistance(px,r-1) ){
         idxnew = idxc + r*contribution[px];
         if ((px + 1) < NumFocus){
            FindCells(px + 1, idxnew);   
         }else{
            cclist[countcc] = idxnew;
            countcc++;   
         }
      }   
   }

}

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQuery(
                              tObject * sample, double range){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   double *SampleD;
   

   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];

   // Create result
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);


   MGridPivot->BuildFieldDistance(sample, SampleD);

   for (int h = 0; h < NumFocus; h++){
      minD[h] = SampleD[h] - (range * NumFocus); 
      maxD[h] = SampleD[h] + (range * NumFocus);
   }
   

   for (int k = 0 ; k < NumClusters ; k++){
      Clusters[k].status = 0;
   }
   
   countcc = 0;
   FindCells(0, 0); 
   int num = 0;

   for (int f = 0; f < countcc; f++){
      //if you understand that congratulations!	
      Clusters[CellArray[cclist[f]].cluster].status = 1;      
   }

   // Let's search
   for (int i = 0; i < OrderNode->GetNumberOfObjects(); i++){
      if (Clusters[OrderNode->GetCluster(i)].status == 1){
      	 tmp = OrderNode->GetObject(i);
      	 //myMetricEvaluator->GetDistance(tmp, sample);
      	 // Evaluate distance
      	 distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	 // Is it qualified ?
      	 if (distance <= range){
            num++;
            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
         }//end if
         delete tmp;
      }//end if
   }//end for
   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery


template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::NearestQuery(
                              tObject * sample, u_int32_t k, bool tie){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   double *SampleD;
   
   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];
   int qCell, qCluster;

   // Create result
   result = new tResult(k);
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

   MGridPivot->BuildFieldDistance(sample, SampleD);
   
   //Search at nearest cluster
   for (int i = 0; i < OrderNode->GetNumberOfObjects(); i++){
         tmp = OrderNode->GetObject(i);

      	// Evaluate distance
      	distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	if (result->GetNumOfEntries() < k){
            // Unnecessary to check. Just add.
            result->AddPair(tmp->Clone(), distance);
         }else{
            // Will I add ?
            if (distance <= result->GetMaximumDistance()){
               // Yes! I'll.
               result->AddPair(tmp->Clone(), distance);
               result->Cut(k);
            }//end if
         delete tmp;
         }//end if      
   }//end for

   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NearestQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQueryCell(
                              tObject * sample, double range){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   double *SampleD;
   

   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];

   // Create result
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);


   MGridPivot->BuildFieldDistance(sample, SampleD);

   for (int h = 0; h < NumFocus; h++){
      minD[h] = SampleD[h] - (range * NumFocus); 
      maxD[h] = SampleD[h] + (range * NumFocus);
   }
   

   for (int k = 0 ; k < NumCells ; k++){
      CellArray[k].status = 0;      
   }
   
   countcc = 0;
   FindCells(0, 0); 
   int num = 0;   
   for (int f = 0; f < countcc; f++){
      CellArray[cclist[f]].status = 1;      
   }
  
   // Let's search
   for (int i = 0; i < OrderNode->GetNumberOfObjects(); i++){
      if (CellArray[OrderNode->GetCell(i)].status == 1){
          tmp = OrderNode->GetObject(i);
      	 //myMetricEvaluator->GetDistance(tmp, sample);
      	 // Evaluate distance
      	 distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	 // Is it qualified ?
      	 if (distance <= range){
            num++;
            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
         }//end if
         delete tmp;
      }//end if
   }//end for
   //std::cout << "\nObjects in range: " << num;
   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQueryCellCenter(
      tObject * sample, double range, int cost){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   double *SampleD;
   tBasicArrayObject * SampleObject;
   

   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];

   // Create result
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);
   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);    

   for (int k = 0 ; k < NumCells ; k++){
      CellArray[k].status = 0;      
      if (CellArray[k].NumObjects > 0){
         distance = myBasicMetricEvaluator->GetDistance(SampleObject, CellArray[k].CellCenter);
         if (distance < (CellArray[k].distance + (range)) ){
            CellArray[k].status = 1;
         }
      }            
   }  
   // Let's search
   distance = (double) MAXINT;
   for (int i = 0; i < OrderNode->GetNumberOfObjects(); i++){
      if (CellArray[OrderNode->GetCell(i)].status == 1){
         tmp = OrderNode->GetObject(i);
      	//myMetricEvaluator->GetDistance(tmp, sample);
      	// Evaluate distance
         for (int x = 0; x < cost; x++){
      	   distance = this->myMetricEvaluator->GetDistance(tmp, sample);
         }
      	 // Is it qualified ?
      	 if (distance <= range){            
            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
         }//end if
         delete tmp;
      }//end if
   }//end for   
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQueryCluster(
                              tObject * sample, double range){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   tBasicArrayObject * SampleObject;
   double distance;
   double *SampleD;
   

   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];

   // Create result
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);


   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD); 

   
   for (int k = 0 ; k < NumClusters ; k++){
      Clusters[k].status = 0;
      distance = myBasicMetricEvaluator->GetDistance(SampleObject, Clusters[k].ClusterMean);
      if (distance < (Clusters[k].maxDist + range )){
         Clusters[k].status = 1;
      }
   }
  
   // Let's search
   int num;
   for (int i = 0; i < OrderNode->GetNumberOfObjects(); i++){
      if (Clusters[OrderNode->GetCluster(i)].status == 1){
          tmp = OrderNode->GetObject(i);
      	 //myMetricEvaluator->GetDistance(tmp, sample);
      	 // Evaluate distance
      	 distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	 // Is it qualified ?
      	 if (distance <= range){
            num++;
            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
         }//end if
         delete tmp;
      }//end if
   }//end for
   //std::cout << "\nObjects in range: " << num;
   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQueryClCell(
                              tObject * sample, double range){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   tBasicArrayObject * SampleObject;
   double distance;
   double *SampleD;
   

   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];

   // Create result
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);


   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);

   for (int h = 0; h < NumFocus; h++){
      minD[h] = SampleD[h] - (range * NumFocus); 
      maxD[h] = SampleD[h] + (range * NumFocus);
   }
   

   for (int cl = 0 ; cl < NumCells ; cl++){
      CellArray[cl].status = 0;      
   }

   for (int k = 0 ; k < NumClusters ; k++){
      Clusters[k].status = 0;
      distance = myBasicMetricEvaluator->GetDistance(SampleObject, Clusters[k].ClusterMean);
      if (distance < (Clusters[k].maxDist + (range * NumFocus)) ){
         Clusters[k].status = 1;
      }
   }
   
   countcc = 0;
   FindCells(0, 0); 
   int num = 0;   
   for (int f = 0; f < countcc; f++){
      CellArray[cclist[f]].status = 1;      
   }
 
   // Let's search
   for (int i = 0; i < OrderNode->GetNumberOfObjects(); i++){
      if (CellArray[OrderNode->GetCell(i)].status == 1 && Clusters[OrderNode->GetCluster(i)].status == 1){
          tmp = OrderNode->GetObject(i);
      	 //myMetricEvaluator->GetDistance(tmp, sample);
      	 // Evaluate distance
      	 distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	 // Is it qualified ?
      	 if (distance <= range){
            num++;
            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
         }//end if
         delete tmp;
      }//end if
   }//end for
  
   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQuerySeq(
                              tObject * sample, double range){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   for (int i = 0; i < LogicNode->GetNumberOfObjects(); i++){
      tmp = LogicNode->GetObject(i);

      distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      if (distance <= range){
         // Yes! I'm qualified !
         result->AddPair(tmp->Clone(), distance);
      }//end if
      delete tmp;
   }//end for
   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::RangeQueryOmniSeq(
                              tObject * sample, double range){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   tBasicArrayObject * ObjectD, * SampleObject;
   double distance, OmniDistance;
   double *SampleD;
   

   SampleD = new double[NumFocus];
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);
   
   for (int i = 0; i < DistanceNode->GetNumberOfObjects(); i++){
           
      ObjectD = DistanceNode->GetObject(i);
      OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);
      
      //if (OmniDistance <= (range * NumFocus)){
      if (OmniDistance <= range){
         tmp = LogicNode->GetObject(i);
         distance = this->myMetricEvaluator->GetDistance(tmp, sample);

         if (distance <= range){
            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
         }//end if
         delete tmp;
      }//end if
      delete ObjectD;
   }//end for
   delete SampleD;
   delete SampleObject;

   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::NearestQueryCell(
                              tObject * sample, u_int32_t k, bool tie){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   double *SampleD;
   
   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];
   int qCell;

   // Create result
   result = new tResult(k);
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

   MGridPivot->BuildFieldDistance(sample, SampleD);
   
   
   for (int i = 0; i < k; i++){
      tmp = OrderNode->GetObject(i);
    	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      result->AddPair(tmp->Clone(), distance); 
      delete tmp;     
   }//end for

   qCell = 0;
   for (int h = 0; h < NumFocus; h++){
      for (int r = 0; r < (NumRings - 1); r++){
         if (SampleD[h] > MGridPivot->GetRingDistance(h,r)){
            qCell += contribution[h];   
         }//end if
      }//end for r
   }//end for h
   
   for (int i = k; i < OrderNode->GetNumberOfObjects(); i++){
      if (OrderNode->GetCell(i) == qCell){
         tmp = OrderNode->GetObject(i);
      	// Evaluate distance
      	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      	// Will I add ?
         if (distance <= result->GetMaximumDistance()){
            // Yes! I'll.
            result->AddPair(tmp->Clone(), distance);
            result->Cut(k);                          
         }//end if
         delete tmp;
      }//end if           
   }//end for

   for (int h = 0; h < NumFocus; h++){
      minD[h] = SampleD[h] - result->GetMaximumDistance(); 
      maxD[h] = SampleD[h] + result->GetMaximumDistance();   
   }

   for (int c = 0 ; c < NumCells ; c++){      
      CellArray[c].status = 0;      
   } 
   
   countcc = 0;
   FindCells(0, 0); 
   int num = 0;

   for (int f = 0; f < countcc; f++){
      CellArray[cclist[f]].status = 1;      
   }//end for f

   CellArray[qCell].status = 2;

   // Let's search
   //Search at nearest cluster
   for (int i = k; i < OrderNode->GetNumberOfObjects(); i++){
      if (CellArray[OrderNode->GetCell(i)].status == 1){
         tmp = OrderNode->GetObject(i);
      	// Evaluate distance
      	distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	// Will I add ?
         if (distance <= result->GetMaximumDistance()){
            // Yes! I'll.
            result->AddPair(tmp->Clone(), distance);
            result->Cut(k);            
           
            for (int h = 0; h < NumFocus; h++){
               minD[h] = SampleD[h] - result->GetMaximumDistance(); 
               maxD[h] = SampleD[h] + result->GetMaximumDistance();   
            }
            
            for (int c = 0 ; c < NumCells ; c++){
               CellArray[c].status = 0;      
            } 

            countcc = 0;
            FindCells(0, 0); 
            int num = 0;

            for (int f = 0; f < countcc; f++){
               CellArray[cclist[f]].status = 1;      
            }//end for f

            CellArray[qCell].status = 2;

         }//end if
         delete tmp;
      }//end if
   }//end for
   std::cout << " ." << std::flush;
   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NearestQuery

//------------------------------------------------------------------------------



template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>::NearestQueryCellCenter(
                              tObject * sample, u_int32_t k, bool tie){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   double distance;
   double *SampleD;
   tBasicArrayObject * SampleObject;

   SampleD = new double[NumFocus];
   minD = new double[NumFocus];
   maxD = new double[NumFocus];   
   cclist = new int[NumCells];
   int qCell;

   // Create result
   result = new tResult(k);
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);
   
   for (int i = 0; i < k; i++){
      tmp = OrderNode->GetObject(i);
    	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      result->AddPair(tmp->Clone(), distance); 
      delete tmp;     
   }//end for

   qCell = 0;
   for (int h = 0; h < NumFocus; h++){
      for (int r = 0; r < (NumRings - 1); r++){
         if (SampleD[h] > MGridPivot->GetRingDistance(h,r)){
            qCell += contribution[h];   
         }//end if
      }//end for r
   }//end for h
   
   for (int i = k; i < OrderNode->GetNumberOfObjects(); i++){
      if (OrderNode->GetCell(i) == qCell){
         tmp = OrderNode->GetObject(i);
      	// Evaluate distance
      	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      	// Will I add ?
         if (distance <= result->GetMaximumDistance()){
            // Yes! I'll.
            result->AddPair(tmp->Clone(), distance);
            result->Cut(k);                          
         }//end if
         delete tmp;
      }//end if      
   }//end for


   for (int c = 0 ; c < NumCells ; c++){
      CellArray[c].status = 0;      
      if (CellArray[c].NumObjects > 0){
         distance = myBasicMetricEvaluator->GetDistance(SampleObject, CellArray[c].CellCenter);
         if (distance < (CellArray[c].distance + (result->GetMaximumDistance()))){
            CellArray[c].status = 1;
         }
      }            
   }

   CellArray[qCell].status = 2;

   // Let's search
   //Search at nearest cluster
   for (int i = k; i < OrderNode->GetNumberOfObjects(); i++){
      if (CellArray[OrderNode->GetCell(i)].status == 1){
         tmp = OrderNode->GetObject(i);
      	// Evaluate distance
      	distance = this->myMetricEvaluator->GetDistance(tmp, sample);

      	// Will I add ?
         if (distance <= result->GetMaximumDistance()){
            // Yes! I'll.
            result->AddPair(tmp->Clone(), distance);
            result->Cut(k);            
            
            for (int c = 0 ; c < NumCells ; c++){
               CellArray[c].status = 0;      
               if (CellArray[c].NumObjects > 0){
                  distance = myBasicMetricEvaluator->GetDistance(SampleObject, CellArray[c].CellCenter);
                  if (distance < (CellArray[c].distance + (result->GetMaximumDistance()))){
                     CellArray[c].status = 1;
                  }
               }            
            }
            CellArray[qCell].status = 2;

         }//end if
         delete tmp;   
      }//end if
   }//end for
   // Return the result.
   std::cout << " .";
   delete SampleObject;
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NearestQuery

//----------------------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>:: NearestQueryCellRank(
                              tObject * sample, u_int32_t k, bool tie, int cost){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   int i;
   double distance, minDistance;
   double *SampleD;
   tBasicArrayObject * SampleObject;

   SampleD = new double[NumFocus];
   //minD = new double[NumFocus];
   //maxD = new double[NumFocus];   
   cclist = new int[NumCells];
   int pos, qNearest;


   // Create result
   result = new tResult(k);
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);
   //std::cout << " 1" << std::flush;
   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);
   //std::cout << " 2" << std::flush;
   for (i = 0; i < k; i++){
      tmp = OrderNode->GetObject(i);
    	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      result->AddPair(tmp, distance);            
   }//end for

   int qCell = 0;
   for (int h = 0; h < NumFocus; h++){
      for (int r = 0; r < (NumRings - 1); r++){
         if (SampleD[h] > MGridPivot->GetRingDistance(h,r)){
            qCell += contribution[h];   
         }//end if
      }//end for r
   }//end for h
   //std::cout << " 3" << std::flush;
   int minBound;
   if ( CellArray[qCell].first > k ){
      minBound = CellArray[qCell].first;
   }else{
      minBound = k;
   }
   for (int i = minBound; i <= CellArray[qCell].last; i++){
      tmp = OrderNode->GetObject(i);
      // Evaluate distance
      //for (int x = 0; x < cost; x++){
      distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      //}     
      if (distance <= result->GetMaximumDistance()){
         // Yes! I'll.
         result->AddPair(tmp->Clone(), distance);
         result->Cut(k);
      }//end if
      delete tmp;
   }//end for
   //std::cout << " 4" << std::flush;
   int NumCellsSort = 0;
   for (int c = 0 ; c < NumCells ; c++){
      CellArray[c].status = 0;
      CellArray[c].qDist = (double) MAXINT;
      if (CellArray[c].NumObjects > 0 && c != qCell){
         CellArray[c].qDist = myBasicMetricEvaluator->GetDistance(SampleObject, CellArray[c].CellCenter) - CellArray[c].distance;
         if (CellArray[c].qDist <= result->GetMaximumDistance()){
            cclist[NumCellsSort] = c;
            NumCellsSort++;
         }
      }
   }
   sortcell(cclist, 0, NumCellsSort - 1);
   //std::cout << "5" << std::flush;
   qNearest = 0;
   pos = 0;
   while (pos < NumCellsSort && CellArray[cclist[pos]].qDist <= result->GetMaximumDistance()){
      i = CellArray[cclist[pos]].first;  
      if (i < k)
         i = k;    
//    while (i < OrderNode->GetNumberOfObjects() && go){
      while (i <= CellArray[cclist[pos]].last){
         //if (OrderNode->GetCell(i) == cclist[pos]){
            //thiscell = true;
            tmp = OrderNode->GetObject(i);
            // Evaluate distance
            //for (int x = 0; x < cost; x++){
               distance = this->myMetricEvaluator->GetDistance(tmp, sample);
            //}          
            // Will I add ?
            if (distance <= result->GetMaximumDistance()){
               // Yes! I'll.
               result->AddPair(tmp, distance);
               result->Cut(k);
               //std::cout << "i: " << i << "d: " << distance << std::flush;
            }else{
               delete tmp;
            }
         //}
         i++;     
      }//end while i
      pos++;
      //std::cout << pos << std::flush;
   }//end while pos
   //std::cout << "6" << std::flush;

   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NearestQuery

//----------------------------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMGrid<ObjectType, EvaluatorType>::sortcell(int *clist, long l, long numObject){
  int pivot = clist[numObject];
  long i = l-1;
  long j = numObject;
  int temp;
  if(numObject > l) {
    do{
      do i++; while (CellArray[clist[i]].qDist < CellArray[pivot].qDist && i < numObject);
	   do j--; while (CellArray[clist[j]].qDist > CellArray[pivot].qDist && j > l);
	   temp = clist[i];
	   clist[i] = clist[j];
	   clist[j] = temp;
    }while (j>i);
    clist[j] = clist[i];
    clist[i] = pivot;
    clist[numObject] = temp;
    sortcell(clist, l, i-1);
    sortcell(clist, i+1, numObject);
  }
}

//---------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>:: NearestQueryOmniSeq(
                              tObject * sample, u_int32_t k, bool tie){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   tBasicArrayObject * ObjectD, * SampleObject;
   double distance, OmniDistance;
   double *SampleD;
   //std::cout << "! " << std::flush;

   SampleD = new double[NumFocus];
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);
   
   //debug
   /*std::cout << " qdist: ";
   for (int h = 0; h < NumFocus; h++){
      std::cout << SampleD[h] << " ";
   }*/


   for (int i = 0; i < k; i++){
      tmp = LogicNode->GetObject(i);
    	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      result->AddPair(tmp, distance);            
   }//end for

   
   for (int i = k; i < DistanceNode->GetNumberOfObjects(); i++){
      ObjectD = DistanceNode->GetObject(i);
      OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);
      
      if (OmniDistance <= result->GetMaximumDistance()){
         tmp = LogicNode->GetObject(i);
         distance = this->myMetricEvaluator->GetDistance(tmp, sample);
         if (distance <= result->GetMaximumDistance()){
            // Yes! I'll.
            result->AddPair(tmp->Clone(), distance);
            result->Cut(k);
         }//end if
         delete tmp;
      }//end if
      delete ObjectD;
   }//end for
   delete SampleD;
   delete SampleObject;

   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NNOmniSeqQuery

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMGrid<ObjectType, EvaluatorType>:: NearestQueryOmniSeqOt(
                              tObject * sample, u_int32_t k, double irange, bool tie){
   //stPage * currPage;
   //stDummyNode * currNode;
   tResult * result;
   tObject * tmp;
   tBasicArrayObject * ObjectD, * SampleObject;
   double distance, OmniDistance;
   double *SampleD;
   //std::cout << "! " << std::flush;
       
   SampleD = new double[NumFocus];
   result = new tResult();
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

   MGridPivot->BuildFieldDistance(sample, SampleD);
   SampleObject = new tBasicArrayObject(NumFocus, SampleD);
   
   //debug
   /*std::cout << " qdist: ";
   for (int h = 0; h < NumFocus; h++){
      std::cout << SampleD[h] << " ";
   }*/


   for (int i = 0; i < k; i++){
      tmp = LogicNode->GetObject(i);
    	distance = this->myMetricEvaluator->GetDistance(tmp, sample);
      result->AddPair(tmp, distance);            
   }//end for

   
   for (int i = 0; i < DistanceNode->GetNumberOfObjects(); i++){
      ObjectD = DistanceNode->GetObject(i);
      OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);
      
      if (OmniDistance <= irange && OmniDistance <= result->GetMaximumDistance()){
         tmp = LogicNode->GetObject(i);
         distance = this->myMetricEvaluator->GetDistance(tmp, sample);
         if (distance <= result->GetMaximumDistance()){
            // Yes! I'll.
            result->AddPair(tmp->Clone(), distance);
            result->Cut(k);
         }//end if
         delete tmp;
      }//end if
      delete ObjectD;
   }//end for
   delete SampleD;
   delete SampleObject;

   // Return the result.
   return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NNOmniSeqQuery

//------------------------------------------------------------------------------

