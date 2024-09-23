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
* This file is the implementation of stMTree methods.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/

//==============================================================================
// Class stMLogicNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMLogicNode<ObjectType, EvaluatorType>::stMLogicNode(int maxOccupation){

   // Allocate resources
   MaxEntries = maxOccupation;
   Entries = new stMLogicEntry[MaxEntries];

   // Init Rep
   RepIndex[0] = 0;
   RepIndex[1] = 0;

   // Initialize
   Count = 0;

   // Minimum occupation. 25% is the default of M-tree
   MinOccupation = (int)(0.25 * maxOccupation);
   // At least the nodes must store 2 objects.
   if ((MinOccupation > (maxOccupation/2)) || (MinOccupation == 0)){
      MinOccupation = 2;
   }//end if

}//end stMLogicNode<ObjectType, EvaluatorType>::stMLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMLogicNode<ObjectType, EvaluatorType>::~stMLogicNode(){
   int i;

   if (Entries != NULL){
      for (i = 0; i < Count; i++){
         if ((Entries[i].Object != NULL) && (Entries[i].Mine)){
            delete Entries[i].Object;
         }//end if
      }//end for
   }//end if
   // Clean before exit.
   delete[] Entries;
}//end stMLogicNode<ObjectType, EvaluatorType>::~stMLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMLogicNode<ObjectType, EvaluatorType>::AddEntry(u_int32_t size, const unsigned char * object){
   if (Count < MaxEntries){
      Entries[Count].Object = new ObjectType();
      Entries[Count].Object->Unserialize(object, size);
      Entries[Count].Mine = true;
      Count++;
      return Count - 1;
   }else{
      return -1;
   }//end if
}//end stMLogicNode<ObjectType, EvaluatorType>::AddEntry

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMLogicNode<ObjectType, EvaluatorType>::AddIndexNode(stMIndexNode * node){
   int i;
   int idx;

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      idx = AddEntry(node->GetObjectSize(i), node->GetObject(i));
      SetEntry(idx, node->GetIndexEntry(i).PageID,
                    node->GetIndexEntry(i).Radius);
   }//end for

   // Node type
   NodeType = stMNode::INDEX;
}//end stMLogicNode<ObjectType, EvaluatorType>::AddIndexNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMLogicNode<ObjectType, EvaluatorType>::AddLeafNode(stMLeafNode * node){
   int i;
   int idx;

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      idx = AddEntry(node->GetObjectSize(i), node->GetObject(i));
      SetEntry(idx, 0, 0);
   }//end for

   // Node type
   NodeType = stMNode::LEAF;
}//end stMLogicNode<ObjectType, EvaluatorType>::AddLeafNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMLogicNode<ObjectType, EvaluatorType>::TestDistribution(
      stMIndexNode * node0, stMIndexNode * node1,
      EvaluatorType * metricEvaluator){
   int dCount;
   int idx;
   int i;
   int l0, l1;
   int currObj;
   doubleIndex * ind0, * ind1;

   // Setup Objects
   dCount = UpdateDistances(metricEvaluator);

   // Init Map and Sorting vector
   ind0 = new doubleIndex[Count];
   ind1 = new doubleIndex[Count];
   for (i = 0; i < Count; i++){
      ind0[i].Index = i;
      ind0[i].Distance = Entries[i].Distance[0];
      ind1[i].Index = i;
      ind1[i].Distance = Entries[i].Distance[1];
      Entries[i].Mapped = false;
   }//end for

   // Sorting by distance...
   sort(ind0, ind0 + Count);
   sort(ind1, ind1 + Count);

   // Make one of then get the minimum occupation.
   l0 = l1 = 0;

   // Adds at least MinOccupation objects to each node.
   for (i = 0; i < MinOccupation; i++){
      // Find a candidate for node 0
      while (Entries[ind0[l0].Index].Mapped){
         l0++;
      }//end while
      // Add to node 0
      currObj = ind0[l0].Index;
      Entries[currObj].Mapped = true;
      idx = node0->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());
      node0->GetIndexEntry(idx).Distance = ind0[l0].Distance;
      node0->GetIndexEntry(idx).PageID = Entries[currObj].PageID;
      node0->GetIndexEntry(idx).Radius = Entries[currObj].Radius;

      // Find a candidate for node 1
      while (Entries[ind1[l1].Index].Mapped){
         l1++;
      }//end while
      // Add to node 1
      currObj = ind1[l1].Index;
      Entries[currObj].Mapped = true;
      idx = node1->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());
      node1->GetIndexEntry(idx).Distance = ind1[l1].Distance;
      node1->GetIndexEntry(idx).PageID = Entries[currObj].PageID;
      node1->GetIndexEntry(idx).Radius = Entries[currObj].Radius;
   }//end for

   // Distribute the others.
   for (i = 0; i < Count; i++){
      if (Entries[i].Mapped == false){
         Entries[i].Mapped = true;
         if (Entries[i].Distance[0] < Entries[i].Distance[1]){
            // Try to put on node 0 first
            idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node0->GetIndexEntry(idx).Distance = Entries[i].Distance[0];
               node0->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node0->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }else{
               // Let's put it in the node 1 since it doesn't fit in the node 0
               idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node1->GetIndexEntry(idx).Distance = Entries[i].Distance[1];
               node1->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node1->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }//end if
         }else{
            // Try to put on node 1 first
            idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node1->GetIndexEntry(idx).Distance = Entries[i].Distance[1];
               node1->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node1->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }else{
               // Let's put it in the node 0 since it doesn't fit in the node 1
               idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node0->GetIndexEntry(idx).Distance = Entries[i].Distance[0];
               node0->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node0->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }//end if
         }//end if
      }//end if
   }//end for

   // Clean home before go away...
   delete[] ind0;
   delete[] ind1;

   return dCount;
}//end stMLogicNode<ObjectType, EvaluatorType>::TestDistribution

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMLogicNode<ObjectType, EvaluatorType>::TestDistribution(
      stMLeafNode * node0, stMLeafNode * node1,
      EvaluatorType * metricEvaluator){
   int dCount;
   int idx;
   int i;
   int l0, l1;
   int currObj;
   doubleIndex *ind0, *ind1;

   // Setup Objects
   dCount = UpdateDistances(metricEvaluator);

   // Init Map and Sorting vector
   ind0 = new doubleIndex[Count];
   ind1 = new doubleIndex[Count];
   for (i = 0; i < Count; i++){
      ind0[i].Index = i;
      ind0[i].Distance = Entries[i].Distance[0];
      ind1[i].Index = i;
      ind1[i].Distance = Entries[i].Distance[1];
      Entries[i].Mapped = false;
   }//end for

   // Sorting by distance...
   sort(ind0, ind0 + Count);
   sort(ind1, ind1 + Count);

   // Make one of then get the minimum occupation.
   l0 = l1 = 0;

   // Adds at least MinOccupation objects to each node.
   for (i = 0; i < MinOccupation; i++){
      // Find a candidate for node 0
      while (Entries[ind0[l0].Index].Mapped){
         l0++;
      }//end while
      // Add to node 0
      currObj = ind0[l0].Index;
      Entries[currObj].Mapped = true;
      idx = node0->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());
      node0->GetLeafEntry(idx).Distance = ind0[l0].Distance;

      // Find a candidate for node 1
      while (Entries[ind1[l1].Index].Mapped){
         l1++;
      }//end while
      // Add to node 1
      currObj = ind1[l1].Index;
      Entries[currObj].Mapped = true;
      idx = node1->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());
      node1->GetLeafEntry(idx).Distance = ind1[l1].Distance;
   }//end for

   // Distribute the others.
   for (i = 0; i < Count; i++){
      if (Entries[i].Mapped == false){
         Entries[i].Mapped = true;
         if (Entries[i].Distance[0] < Entries[i].Distance[1]){
            // Try to put on node 0 first
            idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node0->GetLeafEntry(idx).Distance = Entries[i].Distance[0];
            }else{
               // Let's put it in the node 1 since it doesn't fit in the node 0
               idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node1->GetLeafEntry(idx).Distance = Entries[i].Distance[1];
            }//end if
         }else{
            // Try to put on node 1 first
            idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node1->GetLeafEntry(idx).Distance = Entries[i].Distance[1];
            }else{
               // Let's put it in the node 0 since it doesn't fit in the node 1
               idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node0->GetLeafEntry(idx).Distance = Entries[i].Distance[0];
            }//end if
         }//end if
      }//end if
   }//end for

   // Clean home before go away...
   delete []ind0;
   delete []ind1;

   return dCount;
}//end stMLogicNode<ObjectType, EvaluatorType>::TestDistribution

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMLogicNode<ObjectType, EvaluatorType>::UpdateDistances(
      EvaluatorType * metricEvaluator){
   int i;

   for (i = 0; i < Count; i++){
      if (i == RepIndex[0]){
         Entries[i].Distance[0] = 0;
         Entries[i].Distance[1] = MAXDOUBLE;
      }else if (i == RepIndex[1]){
         Entries[i].Distance[0] = MAXDOUBLE;
         Entries[i].Distance[1] = 0;
      }else{
         Entries[i].Distance[0] = metricEvaluator->GetDistance(
               Entries[RepIndex[0]].Object, Entries[i].Object);
         Entries[i].Distance[1] = metricEvaluator->GetDistance(
               Entries[RepIndex[1]].Object, Entries[i].Object);
      }//end if
   }//end for

   return (GetNumberOfEntries() * 2) - 2;
}//end stMLogicNode<ObjectType, EvaluatorType>::UpdateDistances


//==============================================================================
// Class stMTree
//------------------------------------------------------------------------------

// This macro will be used to replace the declaration of
//       stMTree<ObjectType, EvaluatorType>
#define tmpl_stMTree stMTree<ObjectType, EvaluatorType>

template <class ObjectType, class EvaluatorType>
tmpl_stMTree::stMTree(stPageManager * pageman):
   stMetricTree<ObjectType, EvaluatorType>(pageman){

   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   // Load header.
   LoadHeader();

   // Will I create or load the tree ?
   if (this->GetPageManager()->IsEmpty()){
      DefaultHeader();
   }//end if
}//end stMTree::stMTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
tmpl_stMTree::~stMTree(){

   // Flus header page.
   FlushHeader();
}//end stMTree::~stMTree()

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::DefaultHeader(){

   // Clear header page.
   HeaderPage->Clear();

   // Default values
   Header->Magic[0] = 'M';
   Header->Magic[1] = '_';
   Header->Magic[2] = '_';
   Header->Magic[3] = '3';
   Header->SplitMethod = MIN_RAD;
   Header->ChooseMethod = MIN_R_INCR;
   Header->ChooseSplitStrategyMethod = G_HYPERPL;
   Header->Root = 0;
   Header->MinOccupation = 0.25;
   Header->MaxOccupation = 0;
   Header->Height = 0;
   Header->ObjectCount = 0;
   Header->NodeCount = 0;

   // Notify modifications
   HeaderUpdate = true;
}//end stMTree::DefaultHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::LoadHeader(){

   if (HeaderPage != NULL){
      tMetricTree::GetPageManager()->ReleasePage(HeaderPage);
   }//end if

   // Load and set the header.
   HeaderPage = tMetricTree::GetPageManager()->GetHeaderPage();
   if (HeaderPage->GetPageSize() < sizeof(stMHeader)){
      throw std::logic_error("The page size is too small.");
   }//end if

   Header = (stMHeader *) HeaderPage->GetData();
   HeaderUpdate = false;
}//end stMTree::LoadHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::FlushHeader(){

   if (this->HeaderPage != NULL){
      if (this->Header != NULL){
         WriteHeader();
      }//end if
      tMetricTree::GetPageManager()->ReleasePage(HeaderPage);
   }//end if
}//end stMTree::FlushHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::Add(ObjectType * newObj){
   stSubtreeInfo promo1;
   stSubtreeInfo promo2;

   // Is there a root ?
   if (this->GetRoot() == 0){
      // No! We shall create the new node.
      stPage * auxPage  = NewPage();
      stMLeafNode * leafNode = new stMLeafNode(auxPage, true);
      this->SetRoot(auxPage->GetPageID());
      tMetricTree::GetPageManager()->WritePage(auxPage);
      Header->Height++; // Update Height
      delete leafNode;
      delete auxPage;
   }//end if

   // Let's continue our search for the grail!
   if (this->InsertRecursive(GetRoot(), newObj, NULL, promo1, promo2) == PROMOTION){
      // Split occurred! We must create a new root because it is required.
      // The tree will aacquire a new root.
      AddNewRoot(
            promo1.Rep, promo1.Radius, promo1.RootID,
            promo2.Rep, promo2.Radius, promo2.RootID);
      delete promo1.Rep;
      delete promo2.Rep;
   }//end if

   // Update object count.
   UpdateObjectCounter(1);
   // Header updated!
   this->HeaderUpdate = true;

   return true;
}//end stMTree::Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stMTree::ChooseSubTree(
      stMIndexNode * mIndexNode, ObjectType * obj) {
   int idx;
   int j;
   int * cover;
   bool stop;
   int tmpNumberOfEntries;
   int numberOfEntries, minIndex = 0;
   u_int32_t sizeOfObject; // used for save the size of the current object

   ObjectType * objectType = new ObjectType;
   double distance;
   double minDistance = MAXDOUBLE; // Largest magnitude double value
   // Get the total number of entries.
   numberOfEntries = mIndexNode->GetNumberOfEntries();
   idx = 0;

   switch (this->GetChooseMethod()){

      case stMTree::MIN_R_INCR :
         // Find if there is some circle that contains obj
         stop = (idx >= numberOfEntries);
         while (!stop){
            //get out the object from IndexNode
            objectType->Unserialize(mIndexNode->GetObject(idx),
                                    mIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            // find the first subtree that cover the new object.
            if (distance < mIndexNode->GetIndexEntry(idx).Radius) {
               minDistance = distance;     // the gain will be 0
               stop = true;                // stop the search.
               minIndex = idx;
            }else if (distance - mIndexNode->GetIndexEntry(idx).Radius < minDistance) {
               minDistance = distance - mIndexNode->GetIndexEntry(idx).Radius;
               minIndex = idx;
            }//end if
            idx++;
            // if one of the these condicions are true, stop this while.
            stop = stop || (idx >= numberOfEntries);
         }//end while
         // Try to find a better entry.
         while (idx < numberOfEntries) {
            // Get out the object from IndexNode.
            objectType->Unserialize(mIndexNode->GetObject(idx),
                                    mIndexNode->GetObjectSize(idx));
            // Calculate the distance.                                    
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            if ((distance < mIndexNode->GetIndexEntry(idx).Radius) && (distance < minDistance)) {
               minDistance = distance;
               minIndex = idx;
            }//end if
            idx++;
         }//end while
         break; // end stMTree::cmMINDIST

      case stMTree::MIN_OVERLAP :
         // Find if there is some circle that contains obj
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Get out the object from IndexNode.
            objectType->Unserialize(mIndexNode->GetObject(idx),
                                    mIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            if (distance < minDistance) {
               minDistance = distance;
               minIndex = idx;
            }//end if
         }//end for
         break; //end stMTree::cmMINGDIST

   }//end switch

   delete objectType;

   return minIndex;
}//end stMTree::ChooseSubTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::AddNewRoot(
      ObjectType *obj1, double radius1, u_int32_t nodeID1,
      ObjectType *obj2, double radius2, u_int32_t nodeID2){
   stPage * newPage;
   stMIndexNode * newRoot;
   int idx;

   // Debug mode!
   #ifdef __stDEBUG__
      if ((obj1 == NULL) || (obj2 == NULL)){
         throw std::logic_error("Invalid object.");
      }//end if
   #endif //__stDEBUG__

   // Create a new node
   newPage = NewPage();
   newRoot = new stMIndexNode(newPage, true);

   // Add obj1
   idx = newRoot->AddEntry(obj1->GetSerializedSize(), obj1->Serialize());
   newRoot->GetIndexEntry(idx).Distance = 0.0;
   newRoot->GetIndexEntry(idx).PageID = nodeID1;
   newRoot->GetIndexEntry(idx).Radius = radius1;

   // Add obj2
   idx = newRoot->AddEntry(obj2->GetSerializedSize(), obj2->Serialize());
   newRoot->GetIndexEntry(idx).Distance = 0.0;
   newRoot->GetIndexEntry(idx).PageID = nodeID2;
   newRoot->GetIndexEntry(idx).Radius = radius2;

   // Update tree
   Header->Height++;
   SetRoot(newRoot->GetPage()->GetPageID());
   tMetricTree::GetPageManager()->WritePage(newPage);

   // Dispose page
   delete newRoot;
   tMetricTree::GetPageManager()->ReleasePage(newPage);
}//end MTree::AddNewRoot

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stMTree::InsertRecursive(
      u_int32_t currNodeID, ObjectType * newObj, ObjectType * repObj,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2){
   stPage * currPage;      // Current page
   stPage * newPage;       // New page
   stMNode * currNode;  // Current node
   stMIndexNode * indexNode; // Current index node.
   stMIndexNode * newIndexNode; // New index node for splits
   stMLeafNode * leafNode; // Current leaf node.
   stMLeafNode * newLeafNode; // New leaf node.
   int insertIdx;          // Insert index.
   int result;             // Returning value.
   double dist;        // Temporary distance.
   int subtree;            // Subtree
   ObjectType * subRep;    // Subtree representative.

   // Read node...
   currPage = tMetricTree::GetPageManager()->GetPage(currNodeID);
   currNode = stMNode::CreateNode(currPage);

   // What shall I do ?
   if (currNode->GetNodeType() == stMNode::INDEX){
      // Index Node cast.
      indexNode = (stMIndexNode *)currNode;

      // Where do I add it ?
      subtree = ChooseSubTree(indexNode, newObj);

      // Lets get the information about this tree.
      subRep = new ObjectType();
      subRep->Unserialize(indexNode->GetObject(subtree),
                          indexNode->GetObjectSize(subtree));

      // Try to insert...
      switch (InsertRecursive(indexNode->GetIndexEntry(subtree).PageID,
            newObj, subRep, promo1, promo2)){
         case NO_ACT: // Update Radius and count.
            indexNode->GetIndexEntry(subtree).Radius = promo1.Radius;

            // Returning status.
            promo1.Radius = indexNode->GetMinimumRadius();
            result = NO_ACT;
            break;
         case CHANGE_REP: // Replace representative
            // Remove previous entry.
            indexNode->RemoveEntry(subtree);

            // Try to add the new entry...
            insertIdx = indexNode->AddEntry(promo1.Rep->GetSerializedSize(),
                                            promo1.Rep->Serialize());
            if (insertIdx >= 0){
               // Swap OK. Fill data.
               indexNode->GetIndexEntry(insertIdx).Radius = promo1.Radius;
               indexNode->GetIndexEntry(insertIdx).PageID = promo1.RootID;

               // Will it replace the representative ?
               // WARNING: Do not change the order of this checking.
               if ((repObj != NULL) && (repObj->IsEqual(subRep))){
                  // promo1.Rep is the new representative.
                  indexNode->GetIndexEntry(insertIdx).Distance = 0;

                  // Oops! We must propagate the representative change
                  // promo1.Rep will remain the same.
                  promo1.RootID = currNodeID;

                  // update distances between the new representative
                  // and the others
                  UpdateDistances(indexNode, promo1.Rep, insertIdx);

                  result = CHANGE_REP;
               }else{
                  // promo1.Rep is not the new representative.
                  if (repObj != NULL){
                     // Distance from representative is...
                     indexNode->GetIndexEntry(insertIdx).Distance =
                           this->myMetricEvaluator->GetDistance(
                           repObj, promo1.Rep);
                  }else{
                     // It is the root!
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;
                  }//end if

                  // Cut it here
                  delete promo1.Rep; // promo1.rep will never be used again.
                  promo1.Rep = NULL;
                  result = NO_ACT;
               }//end if
               promo1.Radius = indexNode->GetMinimumRadius();
            }else{
               // Split it!
               // New node.
               newPage = NewPage();
               newIndexNode = new stMIndexNode(newPage, true);

               // Split!
               this->SplitIndex(indexNode, newIndexNode,
                     promo1.Rep, promo1.Radius, promo1.RootID,
                     NULL, 0, 0, repObj, promo1, promo2);

               // Write nodes
               tMetricTree::GetPageManager()->WritePage(newPage);
               // Clean home.
               delete newIndexNode;
               tMetricTree::GetPageManager()->ReleasePage(newPage);
               result = PROMOTION; //Report split.
            }//end if
            break;
         case PROMOTION: // Promotion!!!
            if (promo1.Rep == NULL){
               // Update subtree
               indexNode->GetIndexEntry(subtree).Radius = promo1.Radius;
               indexNode->GetIndexEntry(subtree).PageID = promo1.RootID;

               // Try to insert the promo2.Rep
               insertIdx = indexNode->AddEntry(promo2.Rep->GetSerializedSize(),
                                               promo2.Rep->Serialize());
               if (insertIdx >= 0){
                  // Swap OK. Fill data.
                  indexNode->GetIndexEntry(insertIdx).Radius = promo2.Radius;
                  indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;
                  // Update promo2 distance
                  if (repObj != NULL){
                     // Distance from representative is...
                     indexNode->GetIndexEntry(insertIdx).Distance =
                           this->myMetricEvaluator->GetDistance(
                           repObj, promo2.Rep);
                  }else{
                     // It is the root!
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;
                  }//end if

                  //Update radius...
                  promo1.Radius = indexNode->GetMinimumRadius();
                  delete promo2.Rep;
                  result = NO_ACT;
               }else{
                  // Split it!
                  // New node.
                  newPage = NewPage();
                  newIndexNode = new stMIndexNode(newPage, true);

                  // Split!
                  SplitIndex(indexNode, newIndexNode,
                        promo2.Rep, promo2.Radius, promo2.RootID,
                        NULL, 0, 0, repObj, promo1, promo2);

                  // Write nodes
                  tMetricTree::GetPageManager()->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
                  tMetricTree::GetPageManager()->ReleasePage(newPage);
                  result = PROMOTION; //Report split.
               }//end if
            }else{
               // Remove the previous entry.
               indexNode->RemoveEntry(subtree);

               // Try to add the new entry...
               insertIdx = indexNode->AddEntry(promo1.Rep->GetSerializedSize(),
                                               promo1.Rep->Serialize());
               if (insertIdx >= 0){
                  // Swap OK. Fill data.
                  indexNode->GetIndexEntry(insertIdx).Radius = promo1.Radius;
                  indexNode->GetIndexEntry(insertIdx).PageID = promo1.RootID;

                  // Will it replace the representative ?
                  // WARNING: Do not change the order of this checking.
                  if ((repObj != NULL) && (repObj->IsEqual(subRep))){
                     // promo1.Rep is the new representative.
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;

                     // Oops! We must propagate the representative change
                     // promo1.Rep will remains the same,
                     promo1.RootID = currNodeID;

                     // update distances between the new representative
                     // and the others
                     UpdateDistances(indexNode, promo1.Rep, insertIdx);

                     result = CHANGE_REP;
                  }else{
                     // promo1.Rep is not the new representative.
                     if (repObj != NULL){
                        // Distance from representative is...
                        indexNode->GetIndexEntry(insertIdx).Distance =
                              this->myMetricEvaluator->GetDistance(
                              repObj, promo1.Rep);
                     }else{
                        // It is the root!
                        indexNode->GetIndexEntry(insertIdx).Distance = 0;
                     }//end if

                     // Cut it here
                     delete promo1.Rep; // promo1.rep will never be used again.
                     promo1.Rep = NULL;
                     result = NO_ACT;
                  }//end if

                  // Try to add promo2
                  insertIdx = indexNode->AddEntry(promo2.Rep->GetSerializedSize(),
                                                  promo2.Rep->Serialize());
                  if (insertIdx >= 0){
                     // Swap OK. Fill data.
                     indexNode->GetIndexEntry(insertIdx).Radius = promo2.Radius;
                     indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;

                     // The new distance is...
                     if (promo1.Rep != NULL){
                        // Rep. changed...
                        // Distance from representative is...
                        indexNode->GetIndexEntry(insertIdx).Distance =
                              this->myMetricEvaluator->GetDistance(
                              promo1.Rep, promo2.Rep);
                     }else{
                        // No change!
                        if (repObj != NULL){
                           // Distance from representative is...
                           indexNode->GetIndexEntry(insertIdx).Distance =
                                 this->myMetricEvaluator->GetDistance(
                                 repObj, promo2.Rep);
                        }else{
                           // It is the root!
                           indexNode->GetIndexEntry(insertIdx).Distance = 0;
                        }//end if
                     }//end if

                     delete promo2.Rep;
                     // set the radius to high levels.
                     promo1.Radius = indexNode->GetMinimumRadius();
                  }else{
                     // Split it promo2.rep does not fit.
                     // New node.
                     newPage = NewPage();
                     newIndexNode = new stMIndexNode(newPage, true);

                     // Dispose promo1.rep it if exists because it will not be
                     // used again. It happens when result is CHANGE_REP.
                     if (promo1.Rep != NULL){
                        delete promo1.Rep;
                     }//end if

                     // Add promo2 and split!
                     SplitIndex(indexNode, newIndexNode,
                           promo2.Rep, promo2.Radius, promo2.RootID,
                           NULL, 0, 0, // Ignore this object
                           repObj, promo1, promo2);

                     // Write nodes
                     tMetricTree::GetPageManager()->WritePage(newPage);
                     // Clean home.
                     delete newIndexNode;
                     tMetricTree::GetPageManager()->ReleasePage(newPage);
                     result = PROMOTION; //Report split.
                  }//end if
               }else{
                  // Split it because both objects don't fit.
                  // New node.
                  newPage = NewPage();
                  newIndexNode = new stMIndexNode(newPage, true);

                  // Split!
                  SplitIndex(indexNode, newIndexNode,
                        promo1.Rep, promo1.Radius, promo1.RootID,
                        promo2.Rep, promo2.Radius, promo2.RootID, 
                        repObj, promo1, promo2);

                  // Write nodes
                  tMetricTree::GetPageManager()->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
                  tMetricTree::GetPageManager()->ReleasePage(newPage);
                  result = PROMOTION; //Report split.
               }//end if
            }//end if
      };//end switch

      // Clear the mess.
      delete subRep;
   }else{
      // Leaf node cast.
      stMLeafNode * leafNode = (stMLeafNode *) currNode;

      // Try to insert...
      insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),
                                     newObj->Serialize());
      if (insertIdx >= 0){
         // Don't split!
         // Calculate distance and verify if it is a new radius!
         if (repObj == NULL){
            dist = 0;
         }else{
            dist = this->myMetricEvaluator->GetDistance(newObj, repObj);
         }//end if

         // Fill entry's fields
         leafNode->GetLeafEntry(insertIdx).Distance = dist;

         // Write node.
         tMetricTree::GetPageManager()->WritePage(currPage);

         // Returning values
         promo1.Rep = NULL;
         promo1.Radius = leafNode->GetMinimumRadius();
         promo1.RootID = currNodeID;
         result = NO_ACT;
      }else{
         // Split it!
         // New node.
         newPage = NewPage();
         newLeafNode = new stMLeafNode(newPage, true);

         // Split!
         this->SplitLeaf(leafNode, newLeafNode, (ObjectType *)newObj->Clone(),
               repObj, promo1, promo2);

         // Write node.
         tMetricTree::GetPageManager()->WritePage(newPage);
         // Clean home.
         delete newLeafNode;
         tMetricTree::GetPageManager()->ReleasePage(newPage);
         result = PROMOTION; //Report split.
      }//end if
   }//end if

   // Write node.
   tMetricTree::GetPageManager()->WritePage(currPage);
   // Clean home
   delete currNode;
   tMetricTree::GetPageManager()->ReleasePage(currPage);
   return result;
}//end stMTree<ObjectType, EvaluatorType>::InsertRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::RandomPromote(tLogicNode * node) {

   u_int32_t idx1, idx2;
   u_int32_t numberOfEntries = node->GetNumberOfEntries();
   // generate a number between 0 to numberOfEntries-1 fo idx1
   idx1 = random(numberOfEntries - 1);
   // while idx2 == idx1, do the random for idx2
   while ((idx2 = random(numberOfEntries - 1)) == idx1)
      ;
   // Choose representatives
   node->SetRepresentative(idx1, idx2);
}//end stMTree::randomPromote

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::MinMaxPromote(tLogicNode * node) {

   double iRadius, jRadius, min;
   u_int32_t numberOfEntries, idx1, idx2;
   stPage * newPage1 = new stPage(tMetricTree::GetPageManager()->GetMinimumPageSize());
   stPage * newPage2 = new stPage(tMetricTree::GetPageManager()->GetMinimumPageSize());

   numberOfEntries = node->GetNumberOfEntries();
   min = MAXDOUBLE;   // Largest magnitude double value

   // Is it a Index node?
   if (node->GetNodeType() == stMNode::INDEX) {
      stMIndexNode * indexNode1 = new stMIndexNode(newPage1,true);
      stMIndexNode * indexNode2 = new stMIndexNode(newPage2,true);

      for (u_int32_t i = 0; i < numberOfEntries; i++) {
         for (u_int32_t j = i + 1; j < numberOfEntries; j++) {
            node->SetRepresentative(i, j);
            indexNode1->RemoveAll();
            indexNode2->RemoveAll();
            node->TestDistribution(indexNode1, indexNode2, this->myMetricEvaluator);
            iRadius = indexNode1->GetMinimumRadius();
            jRadius = indexNode2->GetMinimumRadius();
            if (iRadius < jRadius)
               iRadius = jRadius;      // take the maximum
            if (iRadius < min) {
               min = iRadius;
               idx1 = i;
               idx2 = j;
            }//end if
         }//end for
      }//end for
      delete indexNode1;
      delete indexNode2;
   }else {//it is a Leaf node
      stMLeafNode * leafNode1 = new stMLeafNode(newPage1,true);
      stMLeafNode * leafNode2 = new stMLeafNode(newPage2,true);

      for (u_int32_t i = 0; i < numberOfEntries; i++) {
         for (u_int32_t j = i + 1; j < numberOfEntries; j++) {
            node->SetRepresentative(i, j);
            leafNode1->RemoveAll();
            leafNode2->RemoveAll();
            node->TestDistribution(leafNode1, leafNode2, this->myMetricEvaluator);
            iRadius = leafNode1->GetMinimumRadius();
            jRadius = leafNode2->GetMinimumRadius();
            if (iRadius < jRadius)
               iRadius = jRadius;      // take the maximum
            if (iRadius < min) {
               min = iRadius;
               idx1 = i;
               idx2 = j;
            }//end if
         }//end for
      }//end for

      delete leafNode1;
      delete leafNode2;
   }//end else

   // Choose representatives
   node->SetRepresentative(idx1, idx2);

   delete newPage1;
   delete newPage2;
}//end stMTree::minMaxPromote

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::SplitLeaf(
      stMLeafNode * oldNode, stMLeafNode * newNode,
      ObjectType * newObj, ObjectType * prevRep,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2) {
   tLogicNode * logicNode;
   ObjectType * lRep;
   ObjectType * rRep;
   u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();

   // Create the new tLogicNode
   logicNode = new tLogicNode(numberOfEntries + 1);

   // Choose the way the split will occur. 
   if ((GetSplitStrategyMethod() == BAL_G_HYPERPL) ||
       (GetSplitStrategyMethod() == BALANCED)){
      logicNode->SetMinOccupation((int)floor((numberOfEntries + 1)/2));
   }else{
      // it is G_HYPERPL.
      logicNode->SetMinOccupation((int)(GetMinOccupation() * (numberOfEntries + 1)));
   }//end if
   
   logicNode->SetNodeType(stMNode::LEAF);

   // update the maximum number of entries.
   this->SetMaxOccupation(numberOfEntries);
   // Add objects
   logicNode->AddLeafNode(oldNode);
   logicNode->AddEntry(newObj);

   // Split it.
   switch (GetSplitMethod()) {
      case stMTree::RANDOM:
         this->RandomPromote(logicNode);

         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
         break; //end stMTree::RANDOM
      case stMTree::MIN_RAD:
         this->MinMaxPromote(logicNode);

         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
         break;  //end stMTree::MIN_RAD

      #ifdef __stDEBUG__
      default:
         throw logic_error("There is no Split method selected.");
      #endif //__stDEBUG__
   };//end switch

   // Update fields. We may need to change lRep and rRep.
   if (prevRep == NULL){
      // This is a root. The order of lRep and rRep is not important.
      promo1.Rep = lRep;
      promo1.Radius = oldNode->GetMinimumRadius();
      promo1.RootID = oldNode->GetPageID();
      promo2.Rep = rRep;
      promo2.Radius = newNode->GetMinimumRadius();
      promo2.RootID = newNode->GetPageID();
   }else{
      // Let's see if it is necessary to change things.
      if (prevRep->IsEqual(lRep)){
         // lRep is the prevRep. Delete it.
         delete lRep;
         promo1.Rep = NULL;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }else if (prevRep->IsEqual(rRep)){
         // rRep is the prevRep. Delete it.
         delete rRep;
         promo2.Rep = lRep;
         promo2.Radius = oldNode->GetMinimumRadius();
         promo2.RootID = oldNode->GetPageID();
         promo1.Rep = NULL;
         promo1.Radius = newNode->GetMinimumRadius();
         promo1.RootID = newNode->GetPageID();
      }else{
         // This is a root. The order of lRep and rRep is not important.
         promo1.Rep = lRep;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }//end if
   }//end if
}//end stMTree::SplitLeaf

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::SplitIndex(
      stMIndexNode * oldNode, stMIndexNode * newNode,
      ObjectType * newObj1, double newRadius1, u_int32_t newNodeID1,
      ObjectType * newObj2, double newRadius2, u_int32_t newNodeID2,
      ObjectType * prevRep,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2){
   tLogicNode * logicNode;
   ObjectType * lRep;
   ObjectType * rRep;
   u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();

   // Create the new tLogicNode
   logicNode = new tLogicNode(numberOfEntries + 2);


   // Choose the way the split will occur.
   if ((GetSplitStrategyMethod() == BAL_G_HYPERPL) ||
       (GetSplitStrategyMethod() == BALANCED)){
      logicNode->SetMinOccupation((int)floor((numberOfEntries + 2)/2));
   }else{
      // it is G_HYPERPL.
      logicNode->SetMinOccupation((int)(GetMinOccupation() * (numberOfEntries + 2)));
   }//end if

   logicNode->SetNodeType(stMNode::INDEX);

   // update the maximum number of entries.
   this->SetMaxOccupation(numberOfEntries);

   // Add objects
   logicNode->AddIndexNode(oldNode);
   // Add newObj1
   logicNode->AddEntry(newObj1);
   logicNode->SetEntry(logicNode->GetNumberOfEntries() - 1,
         newNodeID1, newRadius1);

   // Will I add newObj2 ?
   if (newObj2 != NULL){
      logicNode->AddEntry(newObj2);
      logicNode->SetEntry(logicNode->GetNumberOfEntries() - 1,
            newNodeID2, newRadius2);
   }//end if

   // Split it.
   switch (GetSplitMethod()) {
      case stMTree::RANDOM:
         this->RandomPromote(logicNode);

         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
         break; //end stMTree::RANDOM
      case stMTree::MIN_RAD:
         this->MinMaxPromote(logicNode);

         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
         break;  //end stMTree::MIN_RAD

      #ifdef __stDEBUG__
      default:
         throw logic_error("There is no Split method selected.");
      #endif //__stDEBUG__
   };//end switch

   // Update fields. We may need to change lRep and rRep.
   if (prevRep == NULL){
      // This is a root. The order of lRep and rRep is not important.
      promo1.Rep = lRep;
      promo1.Radius = oldNode->GetMinimumRadius();
      promo1.RootID = oldNode->GetPageID();
      promo2.Rep = rRep;
      promo2.Radius = newNode->GetMinimumRadius();
      promo2.RootID = newNode->GetPageID();
   }else{
      // Let's see if it is necessary to change things.
      if (prevRep->IsEqual(lRep)){
         // lRep is the prevRep. Delete it.
         delete lRep;
         promo1.Rep = NULL;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }else if (prevRep->IsEqual(rRep)){
         // rRep is the prevRep. Delete it.
         delete rRep;
         promo2.Rep = lRep;
         promo2.Radius = oldNode->GetMinimumRadius();
         promo2.RootID = oldNode->GetPageID();
         promo1.Rep = NULL;
         promo1.Radius = newNode->GetMinimumRadius();
         promo1.RootID = newNode->GetPageID();
      }else{
         // This is a root. The order of lRep and rRep is not important.
         promo1.Rep = lRep;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }//end if
   }//end if
}//end stMTree::SplitIndex

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::UpdateDistances(stMIndexNode *node,
            ObjectType * repObj, int repObjIdx){
   int i;
   ObjectType * tempObj = new ObjectType();

   for (i=0; i < node->GetNumberOfEntries(); i++){
      if (i != repObjIdx){
         tempObj->Unserialize(node->GetObject(i), node->GetObjectSize(i));
         node->GetIndexEntry(i).Distance =
            this->myMetricEvaluator->GetDistance(repObj, tempObj);
      }else{
         //it's the representative object
         node->GetIndexEntry(i).Distance = 0.0;
      }//end if
   }//end for

   //clean the house before exit.
   delete tempObj;
}//end stMTree::UpdateDistances

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stMTree::GetDistanceLimit(){
   double distance = 0;
   double distanceTemp = 0;
   int i, j;
   ObjectType * object1 = new ObjectType();
   ObjectType * object2 = new ObjectType();
   stPage * currPage;
   stMNode * currNode;
   stMIndexNode * indexNode;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes.
      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Index Node cast.
      indexNode = (stMIndexNode *)currNode;

      //search every entry in the root node
      for (i=0; i < indexNode->GetNumberOfEntries()-1; i++){
         //get a object
         object1->Unserialize(indexNode->GetObject(i), indexNode->GetObjectSize(i));
         //combine with the others objects
         for (j=i+1; j < indexNode->GetNumberOfEntries(); j++){
            //get the other object
            object2->Unserialize(indexNode->GetObject(j), indexNode->GetObjectSize(j));
            //calculate the distance of the two objects
            distanceTemp = this->myMetricEvaluator->GetDistance(object1, object2);
            //sum the distance with the distance of the two
            distanceTemp = distanceTemp + indexNode->GetIndexEntry(i).Radius +
                                      indexNode->GetIndexEntry(j).Radius;
            //if this sum is greater than the previous...
            if (distanceTemp > distance)
               //store it to return this value
               distance = distanceTemp;
         }//end for
      }//end for
   }//end if

   //cleaning...
   delete object1;
   delete object2;

   //return the maximum distance between 2 objects of this tree
   return distance;
}//end stMTree::GetDistanceLimit

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stMTree::GetIndexNodeCount(){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t idx, numberOfEntries;
   long nodeCount = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX){
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetIndexNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for

      }//end if

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return
   return nodeCount;
}//end stMTree<ObjectType, EvaluatorType>::GetIndexNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stMTree::GetIndexNodeCount(u_int32_t pageID){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   long nodeCount = 0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         nodeCount += 1;
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetIndexNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for
      }//end if

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return nodeCount;
}//end stMTree<ObjectType, EvaluatorType>::GetIndexNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stMTree::GetLeafNodeCount(){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t idx, numberOfEntries;
   long nodeCount = 0;

   // Is there a root ?
   if (this->GetRoot()){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX){
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetLeafNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for

      }//end if

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return
   return nodeCount;
}//end stMTree<ObjectType, EvaluatorType>::GetLeafNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stMTree::GetLeafNodeCount(u_int32_t pageID){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   long nodeCount = 0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetLeafNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for
      }else{
         nodeCount = 1;
      }//end if

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return nodeCount;
}//end stMTree<ObjectType, EvaluatorType>::GetLeafNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stMTree::GetAvgRadiusLeaf(){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t idx, numberOfEntries;
   double avg = 0;
   u_int32_t height = this->GetHeight();

   // Is there a root ?
   if (this->GetRoot()){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX){
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            if (height == 2){ //stMNode::INDEX
               // Analyze this subtree.
               avg += this->GetAvgRadiusLeaf(indexNode->GetIndexEntry(idx).PageID, height-1);
            }else{
               // Analyze this subtree.
               avg += indexNode->GetIndexEntry(idx).Radius;
            }//endif
         }//end for
         //avg
         avg /= numberOfEntries;

      }//end if

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return
   return avg;
}//end stMTree<ObjectType, EvaluatorType>::GetAvgRadiusLeaf

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stMTree::GetAvgRadiusLeaf(u_int32_t pageID, u_int32_t height){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   double avg = 0;

   // Let's search
   if (pageID != 0){
      // Is it an Index node?
      if (height > 1){ //stMNode::INDEX
         // Get Index node
         currPage = this->myPageManager->GetPage(pageID);
         currNode = stMNode::CreateNode(currPage);
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            if (height == 2){ //stMNode::INDEX
               // Analyze this subtree.
               avg += this->GetAvgRadiusLeaf(indexNode->GetIndexEntry(idx).PageID, height-1);
            }else{
               // Analyze this subtree.
               avg += indexNode->GetIndexEntry(idx).Radius;
            }//endif
         }//end for
         //avg
         avg /= numberOfEntries;
         // Free it all
         delete currNode;
         this->myPageManager->ReleasePage(currPage);
      }//end if
   }//end if

   return avg;
}//end stMTree<ObjectType, EvaluatorType>::GetAvgRadiusLeaf

//==============================================================================
// Begin of Queries
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stMTree::ForwardRangeQueryWithoutPriority(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){
   tResultPaged * result = new tResultPaged();  // Create result
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;
   double external;

   external = externalRadius;

   // Set the information.
   result->SetQueryInfo(sample->Clone(), NEXTRANGEQUERY, nObj, externalRadius, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX){
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            // test external qualifies.
            if ( ((distance - indexNode->GetIndexEntry(idx).Radius) <= external ) &&
                  // test internal qualifies.
                  (( distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
               // Yes! Analyze this subtree.
               this->ForwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, nObj, internalRadius, external, distance, oid);
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            // is it a object that qualified?
            if ( ( distance <= external) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
               // there is more than k elements?
               if (result->GetNumOfEntries() >= nObj){
                  //cut if there is more than k elements
                  result->Cut(nObj);
               }//end if
            }//end if

         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end tmpl_stMTree<ObjectType, EvaluatorType>::ForwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::ForwardRangeQueryWithoutPriority(
         u_int32_t pageID, tResultPaged * result, ObjectType * sample,
         u_int32_t nObj, double internalRadius, double & externalRadius,
         double distanceRepres, long oid){
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
            if ( ( (distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                    indexNode->GetIndexEntry(idx).Radius ) <= externalRadius ) &&
                  // test internal qualifies.
                  ( (distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                    indexNode->GetIndexEntry(idx).Radius ) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // test external qualifies.
               if ( (( distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                     // test internal qualifies.
                     ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
                  // Yes! Analyze it!
                  this->ForwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, nObj, internalRadius, externalRadius, distance, oid);
               }//end if
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
             if ( ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius) &&
                  // test internal qualifies.
                  ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // Is this a qualified object?
               if ( ( distance <= externalRadius) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= nObj){
                     //cut if there is more than k elements
                     result->Cut(nObj);
                     //Minimizing Range
                     externalRadius = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for

      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end tmpl_stMTree<ObjectType, EvaluatorType>::ForwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stMTree::ForwardRangeQuery(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){

   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stMNode * currNode;
   tResultPaged * result;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;

   result = new tResultPaged();  // Create result

   // Set the information.
   result->SetQueryInfo(sample->Clone(), NEXTRANGEQUERY, nObj, externalRadius, false);

   // Let's search
   if (this->GetRoot() != 0){

      // Root node
      pqCurrValue.PageID = this->GetRoot();
      pqCurrValue.Radius = 0;

      // Create the Global Priority Queue
      queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

      // Let's search
      while (pqCurrValue.PageID != 0){
         // Read node...
         currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
         currNode = stMNode::CreateNode(currPage);
         // Is it a Index node?
         if (currNode->GetNodeType() == stMNode::INDEX) {
            // Get Index node
            stMIndexNode * indexNode = (stMIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
               if (  ((distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                       indexNode->GetIndexEntry(idx).Radius) <= externalRadius ) &&
                     // test internal qualifies.
                     ((distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                       indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ){
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

                  // test external qualifies.
                  if (  ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                        // test internal qualifies.
                        ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {

                     // Yes! I'm qualified! Put it in the queue.
                     pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                     pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                     queue->Add(distance, pqTmpValue);
                  }//end if
               }//end if
            }//end for
         }else{
            // No, it is a leaf node. Get it.
            stMLeafNode * leafNode = (stMLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
                if ( ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius ) &&
                     // test internal qualifies.
                     ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius ) ){
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  // When this entry is a representative, it does not need to evaluate
                  // a distance, because distanceRepres is iqual to distance.
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
                  // Is this a qualified object?
                  if ( ( distance <= externalRadius) && (
                       ( distance > internalRadius) ||
                       ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                     // Yes! Put it in the result set.
                     result->AddPair(tmpObj.Clone(), distance);
                     // there is more than k elements?
                     if (result->GetNumOfEntries() >= nObj){
                        //cut if there is more than k elements
                        result->Cut(nObj);
                        //Minimizing Range
                        externalRadius = result->GetMaximumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end for

         }//end else

         // Free it all
         delete currNode;
         this->myPageManager->ReleasePage(currPage);

         // Go to next node
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){

               // test external qualifies.
               if ( (distance <= (externalRadius + pqCurrValue.Radius)) &&
                     // test internal qualifies.
                     (( distance + pqCurrValue.Radius) >= internalRadius) ) {
                  // Yes, get the pageID and the distance from the representative
                  // and the query object.
                  distanceRepres = distance;
                  // Break the while.
                  stop = true;
               }//end if
            }else{
               // the queue is empty!
               pqCurrValue.PageID = 0;
               // Break the while.
               stop = true;
            }//end if
         }while (!stop);
      }// end while
      // Release the Global Priority Queue
      delete queue;

   }// end if

   return result;
}//end tmpl_stMTree<ObjectType, EvaluatorType>::ForwardRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stMTree::BackwardRangeQueryWithoutPriority(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){
   tResultPaged * result = new tResultPaged();  // Create result
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;
   double internal;

   internal = internalRadius;

   // Set the information.
   result->SetQueryInfo(sample->Clone(), PREVIOUSRANGEQUERY, nObj, externalRadius, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX){
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            // test external qualifies.
            if ( ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius ) &&
                 // test internal qualifies.
                 ((distance + indexNode->GetIndexEntry(idx).Radius) >= internal) ) {
               // Yes! Analyze this subtree.
               this->BackwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, nObj, internal, externalRadius, distance, oid);
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            // is it a object that qualified?
            if ( ( distance <= externalRadius) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
               // there is more than nObj elements?
               if (result->GetNumOfEntries() >= nObj){
                  //cut if there is more than nObj elements
                  result->CutFirst(nObj);
               }//end if
            }//end if

         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end tmpl_stMTree<ObjectType, EvaluatorType>::BackwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::BackwardRangeQueryWithoutPriority(
         u_int32_t pageID, tResultPaged * result, ObjectType * sample,
         u_int32_t nObj, double & internalRadius, double externalRadius,
         double distanceRepres, long oid){
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
            if ( (( distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                    indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                  // test internal qualifies.
                  ((distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                    indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // test external qualifies.
               if (  ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                     // test internal qualifies.
                     ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
                  // Yes! Analyze it!
                  this->BackwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, nObj, internalRadius, externalRadius, distance, oid);
               }//end if
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
             if ( ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius) &&
                  // test internal qualifies.
                  ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // Is this a qualified object?
               if ( ( distance <= externalRadius) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than nObj elements?
                  if (result->GetNumOfEntries() >= nObj){
                     //cut if there is more than nObj elements
                     result->CutFirst(nObj);
                     //Minimizing Range
                     internalRadius = result->GetMinimumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for

      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end tmpl_stMTree<ObjectType, EvaluatorType>::BackwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stMTree::BackwardRangeQuery(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){

   tDynamicReversedPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stMNode * currNode;
   tResultPaged * result;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;

   // Set the information.
   result = new tResultPaged();  // Create result
   result->SetQueryInfo(sample->Clone(), PREVIOUSRANGEQUERY, nObj, externalRadius, false);

   // Let's search
   if (this->GetRoot() != 0){

      // Root node
      pqCurrValue.PageID = this->GetRoot();
      pqCurrValue.Radius = 0;

      // Create the Global Priority Queue
      queue = new tDynamicReversedPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

      // Let's search
      while (pqCurrValue.PageID != 0){
         // Read node...
         currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
         currNode = stMNode::CreateNode(currPage);
         // Is it a Index node?
         if (currNode->GetNodeType() == stMNode::INDEX) {
            // Get Index node
            stMIndexNode * indexNode = (stMIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
               if (  ((distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                     indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                     // test internal qualifies.
                     ((distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                     indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ){
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

                  // test external qualifies.
                  if (  ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                        // test internal qualifies.
                        ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
                     // Yes! I'm qualified! Put it in the queue.
                     pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                     pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                     queue->Add(distance, pqTmpValue);
                  }//end if
               }//end if
            }//end for
         }else{
            // No, it is a leaf node. Get it.
            stMLeafNode * leafNode = (stMLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
               if (  ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius) &&
                     // test internal qualifies.
                     ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius) ){
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  // When this entry is a representative, it does not need to evaluate
                  // a distance, because distanceRepres is iqual to distance.
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
                  // Is this a qualified object?
                  if ( ( distance <= externalRadius) && (
                    ( distance > internalRadius) ||
                    ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                     // Yes! Put it in the result set.
                     result->AddPair(tmpObj.Clone(), distance);
                     // there is more than nObj elements?
                     if (result->GetNumOfEntries() >= nObj){
                        //cut if there is more than nObj elements
                        result->CutFirst(nObj);
                        //Minimizing Range
                        internalRadius = result->GetMinimumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end for

         }//end else

         // Free it all
         delete currNode;
         this->myPageManager->ReleasePage(currPage);

         // Go to next node
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){

               // test external qualifies.
               if ( (distance <= (externalRadius + pqCurrValue.Radius)) &&
                     // test internal qualifies.
                     (( distance + pqCurrValue.Radius) >= internalRadius) ) {

                  // Yes, get the pageID and the distance from the representative
                  // and the query object.
                  distanceRepres = distance;
                  // Break the while.
                  stop = true;
               }//end if
            }else{
               // the queue is empty!
               pqCurrValue.PageID = 0;
               // Break the while.
               stop = true;
            }//end if
         }while (!stop);
      }// end while

      // Release the Global Priority Queue
      delete queue;
   }//end if
   return result;
}//end stMTree<ObjectType, EvaluatorType>::BackwardRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stMTree::RangeQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;

   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(this->GetRoot());
      currNode = stMNode::CreateNode(currPage);

      // Is it a Index node?
      if (currNode->GetNodeType() == stMNode::INDEX){
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            // test if this subtree qualifies.
            if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
               // Yes! Analyze this subtree.
               this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, range, distance);
            }//end if
         }//end for
         
      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            // is it a object that qualified?
            if (distance <= range){
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      tMetricTree::GetPageManager()->ReleasePage(currPage);
   }//end if

   return result;
}//end stMTree::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::RangeQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double range, double distanceRepres){
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it a index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality to cut a subtree
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      range + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // is this a qualified subtree?
               if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! Analyze it!
                  this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, range, distance);
               }//end if
            }//end if
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      range){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // Is this a qualified object?
               if (distance <= range){
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      tMetricTree::GetPageManager()->ReleasePage(currPage);
   }//end if
}//end stMTree::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMTree<ObjectType, EvaluatorType>::NearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      this->NearestQuery(result, sample, MAXDOUBLE, k);
   }//end if

   return result;
}//end stMTree::NearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMTree<ObjectType, EvaluatorType>::NearestQuery(tResult * result,
         ObjectType * sample, double rangeK, u_int32_t k){
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;

   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(pqCurrValue.PageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      rangeK + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

               if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTmpValue);
               }//end if
            }//end if
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      rangeK){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               //test if the object qualify
               if (distance <= rangeK){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->Cut(k);
                     //may I use this for performance?
                     rangeK = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      tMetricTree::GetPageManager()->ReleasePage(currPage);

      // Go to next node
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            // Qualified if distance <= rangeK + radius
            if (distance <= rangeK + pqCurrValue.Radius){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
}//end stMTree::NearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMTree<ObjectType, EvaluatorType>::PointQuery(
      ObjectType * sample){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), POINTQUERY);
   // Let's search
   if (this->GetRoot() != 0){
      this->PointQuery(result, sample);
   }//end if

   return result;
}//end stMTree::PointQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMTree<ObjectType, EvaluatorType>::PointQuery(
         tResult * result, ObjectType * sample){
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stMNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTMPValue;
   u_int32_t numberOfEntries;
   bool stop;
   bool find = false;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   
   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while ((pqCurrValue.PageID != 0) && (!find)){
      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(pqCurrValue.PageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it a Index node?        
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

               if (distance <= indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTMPValue.PageID =  indexNode->GetIndexEntry(idx).PageID;
                  pqTMPValue.Radius =  indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTMPValue);
               }//end if
            }//end if
         }//end for
      }else { 
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality
            if ( distanceRepres == leafNode->GetLeafEntry(idx).Distance){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance.
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               //test if the object qualify
               if (distance == 0){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // Stop the query because the object was found!
                  find = true;
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      tMetricTree::GetPageManager()->ReleasePage(currPage);

      // Go to next node.
      if (!find){
         // Search... and feed query
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){
               // Qualified if distance <= rangeK + radius
               if (distance <= pqCurrValue.Radius){
                  // Yes, get the pageID and the distance from the representative
                  // and the query object.
                  distanceRepres = distance;
                  // Break the while.
                  stop = true;
               }//end if
            }else{
               // the queue is empty!
               pqCurrValue.PageID = 0;
               // Break the while.
               stop = true;
            }//end if
         }while (!stop);
      }//end if
   }//end while

   // Release the Global Priority Queue
   delete queue;
}//end stMTree::PointQuery

//==============================================================================
// End of Queries
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stTreeInfoResult * tmpl_stMTree::GetTreeInfo(){
   stTreeInformation * info;

   // No cache of information. I think a cahe would be a good idea.
   info = new stTreeInformation(GetHeight(), GetNumberOfObjects());

   // Let's get the information!
   GetTreeInfoRecursive(this->GetRoot(), 0, info);

   // Optimal tree
   if (info->GetMeanObjectSize() != 0){
      info->CalculateOptimalTreeInfo(int(tMetricTree::GetPageManager()->GetMinimumPageSize() /
            info->GetMeanObjectSize()));
   }//end if

   return info;
}//end stMTree::GetTreeInfo

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::GetTreeInfoRecursive(u_int32_t pageID, int level,
      stTreeInformation * info){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t i;
   u_int32_t lv;
   ObjectType tmp;

   // Let's search
   if (pageID != 0){
      // Update node count
      info->UpdateNodeCount(level);

      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;

         // Object count
         info->UpdateObjectCount(level, indexNode->GetNumberOfEntries());

         // Scan all entries
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            GetTreeInfoRecursive(indexNode->GetIndexEntry(i).PageID, level + 1,
                  info);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stMLeafNode * leafNode = (stMLeafNode *)currNode;

         // Object count
         info->UpdateObjectCount(level, leafNode->GetNumberOfEntries());

         // Update object count
         for (i = 0; i < leafNode->GetNumberOfEntries(); i++){
            // Update other statistics
            info->UpdateMeanObjectSize(leafNode->GetObjectSize(i));

            // Compute intersections
            tmp.Unserialize(leafNode->GetObject(i),
                            leafNode->GetObjectSize(i));

            // Compute intersections
            ObjectIntersectionsRecursive(this->GetRoot(), &tmp, 0, info);
         }//end for
      }//end if

      // Free it all
      delete currNode;
      tMetricTree::GetPageManager()->ReleasePage(currPage);
   }//end if
}//end stMTree::GetTreeInfoRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stMTree::ObjectIntersectionsRecursive(u_int32_t pageID,
      ObjectType * obj, int level, stTreeInformation * info){
   stPage * currPage;
   stMNode * currNode;
   u_int32_t i;
   ObjectType tmp;
   double d;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::GetPageManager()->GetPage(pageID);
      currNode = stMNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stMNode::INDEX) {
         // Get Index node
         stMIndexNode * indexNode = (stMIndexNode *)currNode;

         // Scan all entries
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            tmp.Unserialize(indexNode->GetObject(i),
                indexNode->GetObjectSize(i));
            d = this->myMetricEvaluator->GetDistance(&tmp, obj);
            if (d <= indexNode->GetIndexEntry(i).Radius){
               // Intersection !!!!
               info->UpdateIntersections(level);
               ObjectIntersectionsRecursive(indexNode->GetIndexEntry(i).PageID,
                     obj, level + 1, info);
            }//end if
         }//end for
      }//end if

      // Free it all
      delete currNode;
      tMetricTree::GetPageManager()->ReleasePage(currPage);
   }//end if
}//end stMTree::ObjectIntersectionsRecursive





#ifdef __BULKLOAD__
//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(ObjectType **objects, u_int32_t numObj){
   return BulkLoadMemory(objects, numObj, 0.8);  // Default node occupancy
} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(ObjectType **objects, u_int32_t numObj, double nodeOccupancy){

   u_int32_t objSize = objects[0]->GetSerializedSize();   //@TODO: get the medium or max obj size

   return BulkLoadMemory(objects, numObj, nodeOccupancy, objSize);
} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(ObjectType **objects, u_int32_t numObj, double nodeOccupancy, u_int32_t objSize) {

   vector< SampleMTreeSon<ObjectType> > objs; // objects vector

   for(u_int32_t i=0;i<numObj;i++) {
      SampleMTreeSon<ObjectType> son(objects[i],0.0);
      objs.push_back(son);
   } //end for

   return BulkLoadMemory(objs, nodeOccupancy, objSize);

} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory
//-----------------------------------------------------------------------------
// Utils


template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stMTree::setBulkHeight(u_int32_t numObj,u_int32_t objSize,double nodeOccupancy) {

   u_int32_t numIndexNodeObj = getNumIndexNodeObj(objSize)*nodeOccupancy;
   u_int32_t numLeafNodeObj = getNumLeafNodeObj(objSize)*nodeOccupancy;

   if(numObj<=numLeafNodeObj) return 1;

   // Top-down
   u_int32_t aux = numObj/numIndexNodeObj;
   u_int32_t height = 1;
   while(aux>numLeafNodeObj) {
        aux = aux/numIndexNodeObj;
        height++;
   }

   Header->Height = height+1; // root
   return height;
}

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stMTree::getNodeFreeSize() {
   return GetPageManager()->GetMinimumPageSize() - stMNode::GetGlobalOverhead();
} //end stMTree<ObjectType, EvaluatorType>::getNodeFreeSize

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stMTree::getNumIndexNodeObj(u_int32_t objSize) {
   u_int32_t nodeFreeSize = getNodeFreeSize();
   return ((nodeFreeSize)/(objSize + sizeof(stMIndexNode::stMIndexEntry)));
} //end stMTree<ObjectType, EvaluatorType>::getNumIndexNodeObj

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stMTree::getNumLeafNodeObj(u_int32_t objSize) {
   u_int32_t nodeFreeSize = getNodeFreeSize();
   return ((nodeFreeSize)/(objSize + sizeof(stMLeafNode::stMLeafEntry)));
} //end stMTree<ObjectType, EvaluatorType>::getNumLeafNodeObj

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::searchIdx(u_int32_t *array, u_int32_t size, u_int32_t value) {
   u_int32_t tmpIdx = 0;
   for(;tmpIdx<size; tmpIdx++) {
      if(array[tmpIdx] == value) return true;
   } //end for
   return false;
} //end stMTree<ObjectType, EvaluatorType>::searchIdx

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(vector< SampleMTreeSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize){

   stSubtreeInfo firstSub;

   bool ret = BulkLoadMemory(objects, nodeOccupancy, objSize, firstSub);

   this->SetRoot(firstSub.RootID);

   // Update the Height
   setBulkHeight(objects.size(),objSize,nodeOccupancy);

   #ifdef __stPRINTMSG__
      cout << endl << "Height " << Header->Height << endl;
   #endif //__stPRINTMSG__

   // Update object count.
   UpdateObjectCounter(objects.size());

   // Report the modification.
   HeaderUpdate = true;

   return ret;
} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(vector< SampleMTreeSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub){

   u_int32_t numIndexNodeObj = getNumIndexNodeObj(objSize)*nodeOccupancy;
   u_int32_t numLeafNodeObj = getNumLeafNodeObj(objSize)*nodeOccupancy;

   bool insertLeaf = false;

   if(objects.size()<=numLeafNodeObj) insertLeaf = true;

   return BulkLoadMemory(objects, nodeOccupancy, objSize, sub, insertLeaf);
} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(vector< SampleMTreeSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, bool insertLeaf){

   return BulkLoadMemory(objects, -1, nodeOccupancy, objSize, sub, insertLeaf);
} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::BulkLoadMemory(vector< SampleMTreeSon<ObjectType> > objects, int father, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, bool insertLeaf){

   u_int32_t numIndexNodeObj = getNumIndexNodeObj(objSize)*nodeOccupancy;
   u_int32_t numLeafNodeObj = getNumLeafNodeObj(objSize)*nodeOccupancy;

   u_int32_t numObj = objects.size();

   #ifdef __stPRINTMSG__
      cout << "# Index Obj: " << numIndexNodeObj <<
      "# Leaf Obj: " << numLeafNodeObj <<
      "# Obj: " << numObj << endl;
   #endif //__stPRINTMSG__

   if(insertLeaf) { // insert on leaf?

      #ifdef __stPRINTMSG__
         cout << endl << "Insert in leaf node" <<endl;
      #endif //__stPRINTMSG__

      // new leaf node
      stPage * newPage  = this->NewPage();
      stMLeafNode * leafNode = new stMLeafNode(newPage, true);

      u_int32_t repIdx = father;

      // insert all
      for(u_int32_t i=0;i<numObj;i++) {

         ObjectType *newObj = objects[i].getObject();

         u_int32_t insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),
                                                newObj->Serialize());

         // distance calculation
         leafNode->GetLeafEntry(insertIdx).Distance = this->myMetricEvaluator->GetDistance(newObj, objects[repIdx].getObject());

      } //end for

      sub.Radius = leafNode->GetMinimumRadius();
      sub.RootID = newPage->GetPageID();
      sub.Rep =  objects[repIdx].getObject();

      // clean the mess
      delete leafNode;

      // write to disk
      tMetricTree::GetPageManager()->WritePage(newPage);
      delete newPage;


   } else {

      // choose samples
      SampleMTreeSon<ObjectType> *sample = new SampleMTreeSon<ObjectType>[numIndexNodeObj]; // holds sample data
      u_int32_t *sampleIdx = new u_int32_t[numIndexNodeObj]; // holds sample indexes
      u_int32_t tmpIdx = 0;
      if(father>=0) { // put the father as a sample
         sampleIdx[0] = father;
         sample[0] = objects[father];
         tmpIdx = 1;
      }
      for(;tmpIdx<numIndexNodeObj; tmpIdx++) {
         u_int32_t aux = rand()%numObj;
         while(searchIdx(sampleIdx,numIndexNodeObj,aux)) { // avoid duplicated samples
            aux = rand()%numObj;
         }

         sampleIdx[tmpIdx] = aux;
         sample[tmpIdx] = objects[aux];

         #ifdef __stPRINTMSG__
            cout << aux << endl;
         #endif //__stPRINTMSG__
      } //end for
      delete[] sampleIdx;

      vector< vector< SampleMTreeSon<ObjectType> > > samplesVector(numIndexNodeObj);

      // distribute objects
      for(u_int32_t i=0;i<numObj;i++) {
         int choice = -1;
         double dist = 0.0;
         double distOld = 0.0;
	 for(u_int32_t j=0;j<numIndexNodeObj;j++) {
	    dist = this->myMetricEvaluator->GetDistance(sample[j].getObject(), objects[i].getObject());
	    if((choice == -1) || (dist < distOld)) {
	       distOld = dist;
               choice = j;
	    } //end if
         } //end for



         SampleMTreeSon<ObjectType> son(objects[i].getObject(),distOld);
         #ifdef __stPRINTMSG__
            cout << "Sample #: " << choice << " Object #: " << i << " Distance: " << distOld << endl;
         #endif //__stPRINTMSG__

	 samplesVector[choice].push_back(son); // opt
      } //end for

      u_int32_t numberObjBucket = numObj/numIndexNodeObj;
      u_int32_t numberObjRem = numObj - numberObjBucket*numIndexNodeObj;

      #ifdef __stPRINTMSG__
         cout << "Estimate number per bucket: " << numberObjBucket << " # rem: " << numberObjRem << endl;
         u_int32_t total = 0;
         for(u_int32_t i=0;i<numIndexNodeObj;i++) {
            cout << "Sample #: " << i << " Objects #: " << samplesVector[i].size() << endl;
            total +=  samplesVector[i].size();
         }
         cout << "Total: " << total << endl;
      #endif //__stPRINTMSG__

      vector<int> candidates;
      vector< SampleMTreeSon<ObjectType> > newObjs;

      // redistribution phase
      for(u_int32_t i=0;i<numIndexNodeObj;i++) {
         if(samplesVector[i].size()>numberObjBucket) { // need to balance
            while(samplesVector[i].size()>numberObjBucket) {
               // choose the fartest
               u_int32_t fartest = 0;
               double dist = samplesVector[i][0].getDistance();
               for(u_int32_t idx=1;idx<samplesVector[i].size();idx++) {
                  if(samplesVector[i][idx].getDistance()>dist) {
                     dist = samplesVector[i][idx].getDistance();
                     fartest = idx;

                  }
               }

               newObjs.push_back(samplesVector[i][fartest]);
               samplesVector[i].erase(samplesVector[i].begin()+fartest);
            } //end while
         } else { // candidates
            if(samplesVector[i].size()!=numberObjBucket) {
               candidates.push_back(i);
            } //end if
         } //end if
      } //end for

      ObjectType *uniRep = NULL;

      // redistribute objects
      for(u_int32_t i=0;i<newObjs.size();i++) {
         int choice = -1;
         int candidateIdx;
         double dist = 0.0;
         double distOld = 0.0;
	 for(u_int32_t j=0;j<candidates.size();j++) {
	    dist = this->myMetricEvaluator->GetDistance(sample[candidates[j]].getObject(), newObjs[i].getObject());
	    if((choice == -1) || (dist < distOld)) {
	       distOld = dist;
               choice = candidates[j];
               candidateIdx = j;
	    } //end if
         } //end for


         if(choice == -1) { // remaining

            for(u_int32_t j=0;j<numIndexNodeObj;j++) {
               if(samplesVector[j].size()>numberObjBucket) {
                  continue;
               } //end if
	       dist = this->myMetricEvaluator->GetDistance(sample[j].getObject(), newObjs[i].getObject());
	       if((choice == -1) || (dist < distOld)) {
	          distOld = dist;
                  choice = j;
	       } //end if
            } //end for
         } else {
            if(samplesVector[choice].size()+1>=numberObjBucket) {
               candidates.erase(candidates.begin()+candidateIdx);
            } //end if
         } //end if

         SampleMTreeSon<ObjectType> son(newObjs[i].getObject(),distOld);
         #ifdef __stPRINTMSG__
            cout << "Sample #: " << choice << " Object #: " << i << " Distance: " << distOld << endl;
         #endif //__stPRINTMSG__

         samplesVector[choice].push_back(son); // opt

      } //end for


      #ifdef __stPRINTMSG__
         cout << "Estimate number per bucket: " << numberObjBucket << endl;
         u_int32_t total2 = 0;
         for(u_int32_t i=0;i<numIndexNodeObj;i++) {
            cout << "Sample #: " << i << " Objects #: " << samplesVector[i].size() << endl;
            total2 +=  samplesVector[i].size();
         } //end for
         cout << "Total: " << total2 << endl;
      #endif //__stPRINTMSG__



      #ifdef __stPRINTMSG__
         cout << endl << "Insert in index node" <<endl;
      #endif //__stPRINTMSG__

      stPage * newIndexPage  = this->NewPage();
      stMIndexNode * indexNode = new stMIndexNode(newIndexPage, true);

      bool insertLeaf = samplesVector[0].size() < numLeafNodeObj;

      for(u_int32_t i=0;i<numIndexNodeObj;i++) {

         BulkLoadMemory(samplesVector[i], 0, nodeOccupancy, objSize, sub, insertLeaf);

         //@TODO: To reduce memory usage and increase disk access, do the bulk and after load the node and update the information
         ObjectType *newIndexObj = sub.Rep;

         u_int32_t insertIdx = indexNode->AddEntry(newIndexObj->GetSerializedSize(),
                                        newIndexObj->Serialize());

        indexNode->GetIndexEntry(insertIdx).Radius = sub.Radius;
        indexNode->GetIndexEntry(insertIdx).PageID = sub.RootID;
        if((i==father)||(father<0)) {
           uniRep = newIndexObj;
           indexNode->GetIndexEntry(insertIdx).Distance = 0;
        } else {
           indexNode->GetIndexEntry(insertIdx).Distance = this->myMetricEvaluator->GetDistance(newIndexObj, uniRep);
        } //end if

      } //end for

      sub.Radius = indexNode->GetMinimumRadius();
      sub.RootID = newIndexPage->GetPageID();
      sub.Rep =  uniRep;

      // clean the mess
      delete indexNode;

      tMetricTree::GetPageManager()->WritePage(newIndexPage);
      delete newIndexPage;

      delete[] sample;

   } //end if

   return true;
} //end stMTree<ObjectType, EvaluatorType>::BulkLoadMemory

#endif //__BULKLOAD__

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::Consistency(){

   u_int32_t idx;
   stPage * currPage;
   stMNode * currNode;
   u_int32_t numberOfEntries;
   ObjectType subRep;
   double radius;
   u_int32_t maxHeight, minHeight;
   u_int32_t * heights;
   u_int32_t objectCount;
   u_int32_t subtreeObjects;
   bool result = true;

   // Let's search
   if (this->GetRoot() == 0){
      // Problem! The header has wrong value for the root node.
      result = false;
   }else{
      // Read the root node
      currPage = tMetricTree::GetPageManager()->GetPage(this->GetRoot());
      // Test the root pageID.
      if (currPage == NULL){
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in the root!";
         #endif //__stPRINTMSG__
         // Problem!
         result = false;
      }else{
         // Get the node.
         currNode =  stMNode::CreateNode(currPage);
         result = true;
         objectCount = 0;

         // Is it an Index node?
         if (currNode->GetNodeType() == stMNode::INDEX){
            // Get Index node
            stMIndexNode * indexNode = (stMIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();
            heights = new u_int32_t[numberOfEntries];
            // Set the height variables.
            maxHeight = 0;
            minHeight = 0;
            // For each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // set the subtree objects.
               subtreeObjects = 0;
               // Rebuild the object
               subRep.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Call it recursively.
               result = result && this->Consistency(indexNode->GetIndexEntry(idx).PageID,
                                                    &subRep, radius, heights[idx],
                                                    subtreeObjects);
               // Test the subtree radius.
               if (radius != indexNode->GetIndexEntry(idx).Radius){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a radius problem with entry " << idx
                          << " in pageID: " << indexNode->GetIndexEntry(idx).PageID
                          << " radius " << indexNode->GetIndexEntry(idx).Radius
                          << " measured " << radius << ".";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
               // Test the max height
               if (heights[idx] > heights[maxHeight]){
                  maxHeight = idx;
               }//end if
               // Test the min height
               if (heights[idx] < heights[minHeight]){
                  minHeight = idx;
               }//end if
               objectCount += subtreeObjects;
            }//end for
            // Verify the subtrees regarding the heights.
            if (heights[minHeight] != heights[maxHeight]){
               #ifdef __stPRINTMSG__
                  cout << "\nThe subtree pageID " << indexNode->GetIndexEntry(minHeight).PageID
                       << " with height " << heights[minHeight]
                       << " in idx " << minHeight
                       << " and the subtree pageID " << indexNode->GetIndexEntry(maxHeight).PageID
                       << " with height " << heights[maxHeight]
                       << " in idx " << maxHeight
                       << " both in pageID " << this->GetRoot()
                       << " have different heights from the others";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }//end if
            // release memory.
            delete[] heights;
         }else{
            // No, it is a leaf node. There is only the header to test it.
            stMLeafNode * leafNode = (stMLeafNode *)currNode;
            if (leafNode->GetNumberOfEntries() != Header->ObjectCount){
               // Ops, problem with the header.
               result = false;
               #ifdef __stPRINTMSG__
                  cout << "\nThe header is set with: " << Header->ObjectCount
                       << " objects and the unique leaf node with: " << leafNode->GetNumberOfEntries()
                       << ". Both should have the same number.";
               #endif //__stPRINTMSG__
            }//end if
         }//end else

         // Free it all
         delete currNode;
         tMetricTree::GetPageManager()->ReleasePage(currPage);
      }//end if
   }//end if

   return result;
}//end stMTree::Consistency

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stMTree::Consistency(u_int32_t pageID, ObjectType * repObj,
                                  double & radius, u_int32_t & height,
                                  u_int32_t & objectCount){

   u_int32_t idx;
   stPage * currPage;
   stMNode * currNode;
   u_int32_t numberOfEntries;
   double distance;
   ObjectType subRep, localRep, tmpObj;
   u_int32_t maxHeight, minHeight;
   u_int32_t * heights;
   u_int32_t subtreeObjects;
   int idxRep;
   radius = MAXDOUBLE;
   bool result = true;

   // Let's search
   if (pageID == 0){
      // Problem, invalid pageID!
      result = false;
   }else{
      // Read the node.
      currPage = tMetricTree::GetPageManager()->GetPage(pageID);
      // Test the pageID consistency.
      if (currPage == NULL){
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in " << pageID << ".";
         #endif //__stPRINTMSG__
         // Problem!
         result = false;
      }else{
         // Get the node.
         currNode = stMNode::CreateNode(currPage);
         // Is it an Index node?
         if (currNode->GetNodeType() == stMNode::INDEX) {
            // Get Index node
            stMIndexNode * indexNode = (stMIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();
            heights = new u_int32_t[numberOfEntries];
            // Set the height variables.
            maxHeight = 0;
            minHeight = 0;
            // Get the representative entry of this node.
            idxRep = indexNode->GetRepresentativeEntry();
            // Test if there is a representative.
            if ((idxRep < 0) || (idxRep >= numberOfEntries)){
               #ifdef __stPRINTMSG__
                  cout << "\nThere is no representative entry in pageID "
                       << pageID << ".";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }else{
               // Get the representative.
               localRep.Unserialize(indexNode->GetObject(idxRep),
                                    indexNode->GetObjectSize(idxRep));
               // Check the field entries.
               result = true;
               // Test all entries in this node.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Set the subtree number of objects.
                  subtreeObjects = 0;
                  // Get the subtree representative.
                  subRep.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));

                  result = result && this->Consistency(indexNode->GetIndexEntry(idx).PageID,
                                                       &subRep, radius, heights[idx],
                                                       subtreeObjects);
                  // Test the subtree radius with the local field.
                  if (radius != indexNode->GetIndexEntry(idx).Radius){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a radius problem with entry " << idx
                             << " in Index pageID: " << indexNode->GetIndexEntry(idx).PageID
                             << " radius " << indexNode->GetIndexEntry(idx).Radius
                             << " measured " << radius << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if

                  // Calculate the distance.
                  if (idx != (u_int32_t )idxRep){
                     tmpObj.Unserialize(indexNode->GetObject(idx),
                                        indexNode->GetObjectSize(idx));
                     // Evaluate it!
                     distance = this->myMetricEvaluator->GetDistance(repObj, &tmpObj);
                  }else{
                     // The distance is already calculated.
                     distance = 0;
                  }//end if
                  // Test the distance for every entry.
                  if (distance != indexNode->GetIndexEntry(idx).Distance){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a distance problem in entry " << idx
                             << " in Index pageID " << currPage->GetPageID()
                             << " distance: " << indexNode->GetIndexEntry(idx).Distance
                             << " measured " << distance << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if
                  // Test the max height.
                  if (heights[idx] > heights[maxHeight]){
                     maxHeight = idx;
                  }//end if
                  // Test the min height
                  if (heights[idx] < heights[minHeight]){
                     minHeight = idx;
                  }//end if
                  objectCount += subtreeObjects;
               }//end for
               
               // Set the radius for the upper levels.
               radius = indexNode->GetMinimumRadius();
               // Test if the representative is the same to the upper level.
               if (!localRep.IsEqual(repObj)){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a problem in the representative entry " << idxRep
                          << " in Index pageID " << currPage->GetPageID()
                          << ". The representative is not the same of the upper level.";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
            }//end if
            // Verify the subtrees regarding the heights.
            if (heights[minHeight] != heights[maxHeight]){
               #ifdef __stPRINTMSG__
                  cout << "\nThe subtree pageID " << indexNode->GetIndexEntry(minHeight).PageID
                       << " with height " << heights[minHeight]
                       << " in idx " << minHeight
                       << " and the subtree pageID " << indexNode->GetIndexEntry(maxHeight).PageID
                       << " with height " << heights[maxHeight]
                       << " in idx " << maxHeight
                       << " both in pageID " << currPage->GetPageID()
                       << " have different heights from the others";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }//end if
            // return the minimum height.
            height = heights[maxHeight] + 1;
            // release memory.
            delete[] heights;
         }else{
            // It is a leaf node. Get it.
            stMLeafNode * leafNode = (stMLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();
            objectCount += numberOfEntries;
   
            // Get the representative entry of this node.
            idxRep = leafNode->GetRepresentativeEntry();
            // Test if there is a representative.
            if ((idxRep < 0) || (idxRep >= numberOfEntries)){
               #ifdef __stPRINTMSG__
                  cout << "\nThere is no representative entry in pageID "
                       << pageID << ".";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }else{
               // Get the representative.
               localRep.Unserialize(leafNode->GetObject(idxRep),
                                    leafNode->GetObjectSize(idxRep));
               // Check the field entries.
               result = true;
               // Test all entries in this node.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Get the subtree representative.
                  subRep.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
            
                  // Calculate the distance.
                  if (idx != (u_int32_t )idxRep){
                     tmpObj.Unserialize(leafNode->GetObject(idx),
                                        leafNode->GetObjectSize(idx));
                     // Evaluate it!
                     distance = this->myMetricEvaluator->GetDistance(repObj, &tmpObj);
                  }else{
                     // The distance is already calculated.
                     distance = 0;
                  }//end if
                  // Test the distance for every entry.
                  if (distance != leafNode->GetLeafEntry(idx).Distance){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a distance problem in entry " << idx
                             << " in Leaf pageID " << currPage->GetPageID()
                             << " distance: " << leafNode->GetLeafEntry(idx).Distance
                             << " measured " << distance << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if
               }//end for
               // Set the radius for the upper levels.
               radius = leafNode->GetMinimumRadius();
               // Test if the representative is the same to the upper level.
               if (!localRep.IsEqual(repObj)){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a problem in the representative entry " << idxRep
                          << " in Leaf pageID " << currPage->GetPageID()
                          << ". The representative is not the same of the upper level.";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
            }//end if
            // return height.
            height = 1;
         }//end else
   
         // Free it all
         delete currNode;
         tMetricTree::GetPageManager()->ReleasePage(currPage);
      }//end if
   }//end if

   return result;
}//end stMTree::Consistency
//-----------------------------------------------------------------------------




