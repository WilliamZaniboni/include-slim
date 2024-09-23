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
* This file is the implementation of stRTree methods.
*
*/

//==============================================================================
// Class stRTree
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
stRTree<DataType,OIDType>::stRTree(stPageManager * pageman){
   this->myPageManager = pageman;

   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   // Load header.
   LoadHeader();

   // Will I create or load the tree ?
   if (this->myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if

   queryMetricEvaluator = NULL;
   me = new rEuclideanBasicMetricEvaluator();

   // initialize default parameters
   this->SetSplitMethod(smQUADRATIC);
}
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
bool stRTree<DataType,OIDType>::Add(basicArrayObject * newObj){
   int insertIdx;
   stSubtreeInfo promo1; promo1.Mbr = NULL;
   stSubtreeInfo promo2; promo2.Mbr = NULL;
   // Is there a root ?
   if (this->GetRoot() == 0){
      // No! We shall create the new node.
      stPage * auxPage  = this->NewPage();
      stRLeafNode * leafNode = new stRLeafNode(auxPage, true);
      this->SetRoot(auxPage->GetPageID());

      // Insert the new object.
      insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),newObj->Serialize());
      if (insertIdx < 0){
         return false;
      }
      else{
         // The new object was inserted on the newly created root node.
         // Update the Height
         Header->Height++;
         // Write the root node.
         this->myPageManager->WritePage(auxPage);
      }//end if
      delete leafNode;
      delete auxPage;
   }
   // yes, there is already a root
   else{
      // Let's continue our search for the grail!
      if (InsertRecursive(GetRoot(), newObj->Clone(), NULL, promo1, promo2) == PROMOTION){
         // Split occurred! We must create a new root because it is required.
         // The tree will aacquire a new root.
         AddNewRoot(promo1.Mbr, promo1.RootID, promo1.NObjects, promo1.sonIsLeaf,
                    promo2.Mbr, promo2.RootID, promo2.NObjects, promo2.sonIsLeaf);
      }//end if
      // Clean memory
      if (promo1.Mbr != NULL) {
          delete promo1.Mbr;
      }
      if (promo2.Mbr != NULL) {
          delete promo2.Mbr;
      }
   }//end if

   // Update object count.
   UpdateObjectCounter(1);

   // Report the modification.
   HeaderUpdate = true;

   // Ok. The new object was inserted. Return success!
   return true;
}
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
int stRTree<DataType,OIDType>::InsertRecursive(
      u_int32_t currNodeID, basicArrayObject * newObj, basicArrayObject * mbrObj, stSubtreeInfo & promo1, stSubtreeInfo & promo2){

   stPage * currPage;      // Current page
   stPage * newPage;       // New page
   stRNode * currNode;     // Current node
   stRIndexNode * indexNode, * newIndexNode; // Current index node and new index node for splits
   stRLeafNode * leafNode, * newLeafNode;    // Current leaf node and new leaf node
   int insertIdx;          // Insert index.
   int result;             // Returning value.
   int subtree;            // Subtree

   // Read node...
   currPage = this->myPageManager->GetPage(currNodeID);
   currNode = stRNode::CreateNode(currPage);

   // What shall I do ?
   if (currNode->GetNodeType() == stRNode::INDEX){
      // Index Node cast.
      indexNode = (stRIndexNode *)currNode;

      // Where do I add it ?
      subtree = ChooseSubTree(currPage, newObj);

      // Lets get the information about this tree.
      basicArrayObject * subMBR = new basicArrayObject();
      subMBR->Unserialize(indexNode->GetObject(subtree), indexNode->GetObjectSize(subtree));

      // Try to insert...
      switch (InsertRecursive(indexNode->GetIndexEntry(subtree).PageID, newObj, subMBR, promo1, promo2)){
         case NO_ACT:
            // Update count.
            indexNode->GetIndexEntry(subtree).NEntries++;
            // Returning status.
            promo1.NObjects = indexNode->GetTotalObjectCount();
            promo1.sonIsLeaf = false;
            result = NO_ACT;
            break;

         case UPDATE_MBR:
            // Remove previous entry.
            indexNode->RemoveEntry(subtree);
            // Try to add the new entry...
            insertIdx = indexNode->AddEntry(promo1.Mbr->GetSerializedSize(), promo1.Mbr->Serialize());
            if (insertIdx >= 0){
               // Swap OK. Fill data.
               indexNode->GetIndexEntry(insertIdx).NEntries = promo1.NObjects;
               indexNode->GetIndexEntry(insertIdx).PageID = promo1.RootID;
               indexNode->GetIndexEntry(insertIdx).sonIsLeaf = promo1.sonIsLeaf;

               // Do I need to continue updating mbr in the upper levels?
               delete promo1.Mbr;
               promo1.Mbr = this->GetIndexMbr(indexNode);
               if (this->GetMbrArea(subMBR) != this->GetMbrArea(promo1.Mbr)) {
                  // Oops! We must propagate the mbr change in the upper levels
                  promo1.RootID = currNodeID;
                  promo1.NObjects = indexNode->GetTotalObjectCount();
                  promo1.sonIsLeaf = false;
                  result = UPDATE_MBR;
               }
               else{
                  // No, no need to update mbr in the upper levels
                  delete promo1.Mbr;
                  promo1.Mbr = NULL;
                  promo1.NObjects = indexNode->GetTotalObjectCount();
                  promo1.sonIsLeaf = false;
                  result = NO_ACT;
               }//end if
            }
            else{
               // it should never got here, as the old entry was removed and the new entry should fit in the available space
               insertIdx = -1;
            }
            break;

         case PROMOTION:
            // Remove the previous entry.
            indexNode->RemoveEntry(subtree);

            // Try to add the new entry...
            insertIdx = indexNode->AddEntry(promo1.Mbr->GetSerializedSize(), promo1.Mbr->Serialize());
            if (insertIdx >= 0){
               // Swap OK. Fill data.
               indexNode->GetIndexEntry(insertIdx).NEntries = promo1.NObjects;
               indexNode->GetIndexEntry(insertIdx).PageID = promo1.RootID;
               indexNode->GetIndexEntry(insertIdx).sonIsLeaf = promo1.sonIsLeaf;

               // Try to add promo2
               insertIdx = indexNode->AddEntry(promo2.Mbr->GetSerializedSize(), promo2.Mbr->Serialize());
               if (insertIdx >= 0){
                  // Swap OK. Fill data.
                  indexNode->GetIndexEntry(insertIdx).NEntries = promo2.NObjects;
                  indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;
                  indexNode->GetIndexEntry(insertIdx).sonIsLeaf = promo2.sonIsLeaf;
                  delete promo2.Mbr;
                  promo2.Mbr = NULL;

                  // Oops! We must propagate the mbr change in the upper levels
                  delete promo1.Mbr;
                  promo1.Mbr = this->GetIndexMbr(indexNode);
                  promo1.RootID = currNodeID;
                  promo1.NObjects = indexNode->GetTotalObjectCount();
                  promo1.sonIsLeaf = false;
                  result = UPDATE_MBR;
               }
               else{
                  // Split it as promo2.mbr does not fit
                  // New node.
                  newPage = this->NewPage();
                  newIndexNode = new stRIndexNode(newPage, true);

                  // Dispose promo1.mbr it if exists
                  if (promo1.Mbr != NULL){
                     delete promo1.Mbr;
                     promo1.Mbr = NULL;
                  }//end if

                  // Add promo2 and split!
                  SplitIndex(indexNode, newIndexNode, promo2.Mbr, promo2.RootID, promo2.NObjects, promo2.sonIsLeaf);

                  delete promo1.Mbr;
                  promo1.Mbr = this->GetIndexMbr(indexNode);
                  promo1.RootID = indexNode->GetPageID();
                  promo1.NObjects = indexNode->GetTotalObjectCount();
                  promo1.sonIsLeaf = false;

                  delete promo2.Mbr;
                  promo2.Mbr = this->GetIndexMbr(newIndexNode);
                  promo2.RootID = newIndexNode->GetPageID();
                  promo2.NObjects = newIndexNode->GetTotalObjectCount();
                  promo2.sonIsLeaf = false;

                  // Write nodes
                  this->myPageManager->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
                  this->myPageManager->ReleasePage(newPage);
                  result = PROMOTION; //Report split.
               }//end if
            }
            else {
               // it should not get here!!!
               result = NO_ACT;
            }//end if

      };//end switch

      // Clear the mess.
      delete subMBR;
   }
   else{
      // Leaf node cast.
      leafNode = (stRLeafNode *) currNode;

      basicArrayObject *tmpmbr = this->GetLeafMbr(leafNode);
      double tmpwidth = this->GetMbrArea(tmpmbr);
      delete tmpmbr;

      // Try to insert...
      insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(), newObj->Serialize());
      if (insertIdx >= 0) { // Don't split!
         // Write node.
         this->myPageManager->WritePage(currPage);

         // Returning values
         promo1.Mbr = this->GetLeafMbr(leafNode);
         promo1.RootID = currNodeID;
         promo1.NObjects = leafNode->GetNumberOfEntries();
         promo1.sonIsLeaf = true;

         double newtmpwidth = this->GetMbrArea(promo1.Mbr);

         // is it needed to update the mbr after returning?
         if (newtmpwidth > tmpwidth)
             result = UPDATE_MBR;
         else
             result = NO_ACT;
      }
      else{
         // Split it!
         // New node.
         newPage = this->NewPage();
         newLeafNode = new stRLeafNode(newPage, true);

         // Split!
         SplitLeaf(leafNode, newLeafNode, newObj, promo1, promo2);

         // Write node.
         this->myPageManager->WritePage(newPage);
         // Clean home.
         delete newLeafNode;
         this->myPageManager->ReleasePage(newPage);
         result = PROMOTION; //Report split.
      }//end if
   }//end if

   // Write node.
   this->myPageManager->WritePage(currPage);
   // Clean home
   delete currNode;
   this->myPageManager->ReleasePage(currPage);
   return result;
}//end stRTree::InsertRecursive
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
int stRTree<DataType,OIDType>::ChooseSubTree(stPage * currPage, basicArrayObject * obj) {
    /**
    * This method computes an index of an entry where the insertion process
    * of record obj should continue.
    *
    * This procedures was adapted from RTDirNode.java choose_subtree method available
    * in www.rtreeportal.org, from R-star tree source code from Dimitris Papadias
    *
    * ChooseSubTree chooses the best subtree under this node to insert a new mbr
    * There are three cases:
    * Case 1: the new mbr is contained (inside) in only one directory entry mbr.
    * In this case follow this subtree.
    * Case 2: the new mbr is contained (inside) in more than one directory entry mbr.
    * In this case follow the entry whose mbr has the minimum area
    * Case 3: the new mbr is not contained (inside) in any directory entry mbr
    * In this case the criteria are the following:
    * - If the son nodes are data nodes consider as criterion first the minimum overlap
    *   increase if we follow one node with its neighbors, then the minimum area enlargement
    *   and finally the minimum area
    * - In the son nodes are dir nodes consider as criterion first the minimum area enlargement
    *   and finally the minimum area
    * After we choose the subtree, we enlarge the directory entry (if has to be enlarged)
    * and return its index
    */
    stRNode * currNode = stRNode::CreateNode(currPage);
    stRIndexNode * rIndexNode = (stRIndexNode *)currNode;

    int numberOfEntries = rIndexNode->GetNumberOfEntries();
    int minIndex = 0, i, j, follow;
    int insideCount = 0;   // this variable holds the number of entries whose mbr contains the new mbr to be inserted
    int *inside = new int[numberOfEntries]; // this array holds the indices of entries whose mbr contains the new mbr to be inserted

    // compute inside[]
    for (i = 0; i < numberOfEntries; i++) {
        basicArrayObject tmpMbr;
        tmpMbr.Unserialize(rIndexNode->GetObject(i), rIndexNode->GetObjectSize(i));
        switch (SectionMbrObj(&tmpMbr, obj)) {
            case scINSIDE:
                // mbr is inside entries[i] mbr
                inside[insideCount++] = i;
                break;
        }
    }

    if (insideCount == 1) {
        // Case 1: There is exactly one mbr entry that contains the object
        follow = inside[0];
    }
    else if (insideCount > 1) {
        // Case 2: there are many mbr entries that contain the object.
        // Choose the one for which insertion causes the minimun area enlargement
        double f, fmin = MAXDOUBLE;
        basicArrayObject tmpMbr;
        for (i = 0; i < insideCount; i++) {
            tmpMbr.Unserialize(rIndexNode->GetObject(inside[i]), rIndexNode->GetObjectSize(inside[i]));
            f = GetMbrArea(&tmpMbr);
            if (f < fmin) {
                minIndex = i;
                fmin = f;
            }
        }
        follow = inside[minIndex];
    }
    else {
        // Case 3: There are no dir_mbrs that contain mbr
        // choose the one for which insertion causes the minimun overlap if sonIsLeaf
        // else choose the one for which insertion causes the minimun area enlargement
        if (rIndexNode->GetIndexEntry(0).sonIsLeaf) {
            double omin = MAXDOUBLE;
            double fmin = MAXDOUBLE;
            double amin = MAXDOUBLE;
            basicArrayObject bmbr, mbr_i, mbr_j;
            for (i = 0; i < numberOfEntries; i++) {
                bmbr.Unserialize(rIndexNode->GetObject(i), rIndexNode->GetObjectSize(i));
                mbr_i.Unserialize(rIndexNode->GetObject(i), rIndexNode->GetObjectSize(i));

                // calculate area
                double a = GetMbrArea(&bmbr);

                // enlarge bmbr to include obj
                EnlargeMbr(&bmbr, obj);

                // calculate area enlargement
                double f = GetMbrArea(&bmbr) - a;

                // calculate overlap before enlarging entry_i
                double o = 0.0, old_o = 0.0;
                for (int j = 0; j < numberOfEntries; j++) {
                    if (j != i) {
                        mbr_j.Unserialize(rIndexNode->GetObject(j), rIndexNode->GetObjectSize(j));
                        old_o += Overlap(&mbr_i, &mbr_j);
                        o += Overlap(&bmbr, &mbr_j);
                    }
                }
                o -= old_o;

                // is this entry better than the former optimum ?
                if ((o < omin) || ((o == omin) && (f < fmin)) || ((o == omin) && (f == fmin) && (a < amin))) {
                    minIndex = i;
                    omin = o;
                    fmin = f;
                    amin = a;
                }
            }
        }
        else { // son is index
            double fmin = MAXDOUBLE;
            double amin = MAXDOUBLE;
            basicArrayObject bmbr, mbr_i;
            for (i = 0; i < numberOfEntries; i++) {
                bmbr.Unserialize(rIndexNode->GetObject(i), rIndexNode->GetObjectSize(i));
                mbr_i.Unserialize(rIndexNode->GetObject(i), rIndexNode->GetObjectSize(i));

                // calculate area
                double a = GetMbrArea(&bmbr);

                // enlarge bmbr to include obj
                EnlargeMbr(&bmbr, obj);

                // calculate area enlargement
                double f = GetMbrArea(&bmbr) - a;

                // is this entry better than the former optimum ?
                if ((f < fmin) || ((f == fmin) && (a < amin))) {
                    minIndex = i;
                    fmin = f;
                    amin = a;
                }
            }
        }
        // enlarge the boundaries of the directoty entry we will follow
        basicArrayObject bmbr;
        bmbr.Unserialize(rIndexNode->GetObject(minIndex), rIndexNode->GetObjectSize(minIndex));
        EnlargeMbr(&bmbr, obj);

        follow = minIndex;

        // write bmbr as mbr of index node pointer of position minIndex
        u_int32_t tmpRootID = rIndexNode->GetIndexEntry(minIndex).PageID;
        u_int32_t tmpNObjects = rIndexNode->GetIndexEntry(minIndex).NEntries;
        bool tmpSonIsLeaf = rIndexNode->GetIndexEntry(minIndex).sonIsLeaf;
        // remove the minIndex entry
        rIndexNode->RemoveEntry(minIndex);
        // add the new entry
        int insertIdx = rIndexNode->AddEntry(bmbr.GetSerializedSize(), bmbr.Serialize());
        if (insertIdx >= 0){
            // Swap OK. Fill data.
            rIndexNode->GetIndexEntry(insertIdx).PageID = tmpRootID;
            rIndexNode->GetIndexEntry(insertIdx).NEntries = tmpNObjects;
            rIndexNode->GetIndexEntry(insertIdx).sonIsLeaf = tmpSonIsLeaf;
        }
        else {
            // it should never get here
            follow = -1;
        }

        // Write node.
        this->myPageManager->WritePage(currPage);
    }

    // Clean memory
    delete inside;
    delete currNode;

    return follow;
}//end stRTree::ChooseSubTree
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
void stRTree<DataType,OIDType>::SplitIndex(stRIndexNode * oldNode, stRIndexNode * newNode, basicArrayObject * newMbr, u_int32_t newNodeID, u_int32_t newNEntries, bool sonIsLeaf) {
   basicArrayObject *mbr;
   u_int32_t i, j, k, position1, position2, idx, numberOfEntries = oldNode->GetNumberOfEntries();
   double d, mindist, maxdist;
   basicArrayObject ** allobjects;
   u_int32_t *allPageIDs;
   u_int32_t *allNEntries;
   bool *allSonIsLeaf;

   numberOfEntries = oldNode->GetNumberOfEntries() + 1;
   allobjects = new basicArrayObject*[numberOfEntries];
   allPageIDs = new u_int32_t[numberOfEntries];
   allNEntries = new u_int32_t[numberOfEntries];
   allSonIsLeaf = new bool[numberOfEntries];
   // copy objects to allobjects
   for (i = 0; i < numberOfEntries - 1; i++) {
       allobjects[i] = new basicArrayObject();
       allobjects[i]->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
       allPageIDs[i] = oldNode->GetIndexEntry(i).PageID;
       allNEntries[i] = oldNode->GetIndexEntry(i).NEntries;
       allSonIsLeaf[i] = oldNode->GetIndexEntry(i).sonIsLeaf;
   }
   allobjects[numberOfEntries-1] = newMbr->Clone();
   allPageIDs[numberOfEntries-1] = newNodeID;
   allNEntries[numberOfEntries-1] = newNEntries;
   allSonIsLeaf[numberOfEntries-1] = sonIsLeaf;

   // Split it.
   switch (this->GetSplitMethod()) {
      case stRTree::smLINEAR:
         /**
         * The linear split chooses two mbrs as seeds for the two nodes, as far
         * apart from each other as possible. Then consider each remaining object
         * in a random order and assign it to the node requiring the smallest MBR
         * enlargement.
         */

         // get the two nodes as far apart from each other as possible
         position1 = -1, position2 = -1;
         maxdist = -1;
         for (i = 0; i < numberOfEntries; i++) {
             for (j = i+1; j < numberOfEntries; j++) {
                 d = this->GetDistanceBetweenMbrs(allobjects[i], allobjects[j]);
                 if (d > maxdist) {
                     maxdist = d;
                     position1 = i;
                     position2 = j;
                 }
             }
         }

         // now allobjects[position1] and allobjects[position2] is the pair of objects with greater distance from one another
         oldNode->RemoveAll();
         newNode->RemoveAll();

         // add position1
         idx = oldNode->AddEntry(allobjects[position1]->GetSerializedSize(), allobjects[position1]->Serialize());
         oldNode->GetIndexEntry(idx).PageID = allPageIDs[position1];
         oldNode->GetIndexEntry(idx).NEntries = allNEntries[position1];
         oldNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[position1];

         // add position2
         idx = newNode->AddEntry(allobjects[position2]->GetSerializedSize(), allobjects[position2]->Serialize());
         newNode->GetIndexEntry(idx).PageID = allPageIDs[position2];
         newNode->GetIndexEntry(idx).NEntries = allNEntries[position2];
         newNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[position2];

         // consider each remaining object assigning it to the node requiring the smallest enlargement of its respective mbr
         for (i = 0; i < numberOfEntries; i++) {
             if ((i != position1) && (i != position2)) {
                 // add entry and measure resultant mbr to oldNode
                 mbr = this->GetIndexMbr(oldNode);
                 d = this->GetMbrArea(mbr);
                 delete mbr;
                 idx = oldNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                 oldNode->GetIndexEntry(idx).PageID = allPageIDs[i];
                 oldNode->GetIndexEntry(idx).NEntries = allNEntries[i];
                 oldNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[i];
                 mbr = this->GetIndexMbr(oldNode);
                 mindist = this->GetMbrArea(mbr) - d;
                 delete mbr;

                 // add entry and measure resultant mbr to newNode
                 mbr = this->GetIndexMbr(newNode);
                 d = this->GetMbrArea(mbr);
                 delete mbr;
                 idx = newNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                 newNode->GetIndexEntry(idx).PageID = allPageIDs[i];
                 newNode->GetIndexEntry(idx).NEntries = allNEntries[i];
                 newNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[i];
                 mbr = this->GetIndexMbr(newNode);
                 maxdist = this->GetMbrArea(mbr) - d;
                 delete mbr;

                 // leave the element in the node with less enlargment
                 if (mindist < maxdist)
                     newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                 else
                     oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
             }
         }
         break; //end stRTree::smLINEAR

      case stRTree::smQUADRATIC:
         /**
         * The quadratic split chooses two objects as seeds for the two nodes, where
         * these objects if put together create as much dead space as possible (i.e.
         * the space that remains from the MBR if the areas of the two objects are
         * ignored). Then, until there are no remaining objects, insert the object
         * for which the difference of dead space if assigned to each of the two
         * nodes is maximized in the node that requires less enlargement of its MBR.
         */

         // get the two nodes as far apart from each other as possible
         position1 = -1, position2 = -1;
         maxdist = -1;
         for (i = 0; i < numberOfEntries; i++) {
             oldNode->RemoveAll();
             for (j = i+1; j < numberOfEntries; j++) {
                 oldNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                 oldNode->AddEntry(allobjects[j]->GetSerializedSize(), allobjects[j]->Serialize());
                 mbr = this->GetIndexMbr(oldNode);
                 d = this->GetMbrArea(mbr) + this->Overlap(allobjects[i],allobjects[j]) -
                     this->GetMbrArea(allobjects[i]) - this->GetMbrArea(allobjects[j]);
                 delete mbr;
                 if (d > maxdist) {
                     maxdist = d;
                     position1 = i;
                     position2 = j;
                 }
             }
         }

         // now allobjects[position1] and allobjects[position2] is the pair of objects with greater distance from one another
         oldNode->RemoveAll();
         newNode->RemoveAll();

         // add position1
         idx = oldNode->AddEntry(allobjects[position1]->GetSerializedSize(), allobjects[position1]->Serialize());
         oldNode->GetIndexEntry(idx).PageID = allPageIDs[position1];
         oldNode->GetIndexEntry(idx).NEntries = allNEntries[position1];
         oldNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[position1];

         // add position2
         idx = newNode->AddEntry(allobjects[position2]->GetSerializedSize(), allobjects[position2]->Serialize());
         newNode->GetIndexEntry(idx).PageID = allPageIDs[position2];
         newNode->GetIndexEntry(idx).NEntries = allNEntries[position2];
         newNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[position2];

         // Then, until there are no remaining objects, insert the object
         // for which the difference of dead space if assigned to each of the two
         // nodes is maximized in the node that requires less enlargement of its MBR.
         for (i = 0; i < numberOfEntries; i++) {
             if ((i != position1) && (i != position2)) {
                 // add entry and measure resultant mbr to oldNode
                 mbr = this->GetIndexMbr(oldNode);
                 d = this->GetMbrArea(mbr);
                 delete mbr;
                 idx = oldNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                 oldNode->GetIndexEntry(idx).PageID = allPageIDs[i];
                 oldNode->GetIndexEntry(idx).NEntries = allNEntries[i];
                 oldNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[i];
                 mbr = this->GetIndexMbr(oldNode);
                 mindist = this->GetMbrArea(mbr) - d;
                 delete mbr;

                 // add entry and measure resultant mbr to newNode
                 mbr = this->GetIndexMbr(newNode);
                 d = this->GetMbrArea(mbr);
                 delete mbr;
                 idx = newNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                 newNode->GetIndexEntry(idx).PageID = allPageIDs[i];
                 newNode->GetIndexEntry(idx).NEntries = allNEntries[i];
                 newNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[i];
                 mbr = this->GetIndexMbr(newNode);
                 maxdist = this->GetMbrArea(mbr) - d;
                 delete mbr;

                 // leave the element in the node with less enlargment
                 if (mindist < maxdist)
                     newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                 else
                     oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
             }
         }
         break; //end stRTree::smQUADRATIC

      case stRTree::smEXPONENTIAL:
         /**
         * In the exponential split all possible groupings are exhaustively tested
         * and the best is chosen with respect to the minimization of the MBR
         * enlargement.
         */
         double totalMbrEnlargement = MAXDOUBLE;
         int oldNodeBestEntry = -1;
         int newNodeBestEntry = -1;
         // try all possible groupings
         for (i = 0; i < numberOfEntries; i++) {
             for (j = i; j < numberOfEntries; j++) {
                 if (i != j) {
                     oldNode->RemoveAll();
                     oldNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                     newNode->RemoveAll();
                     newNode->AddEntry(allobjects[j]->GetSerializedSize(), allobjects[j]->Serialize());

                     // consider each remaining object assigning it to the node requiring the smallest enlargement of its respective mbr
                     for (k = 0; k < numberOfEntries; k++) {
                         if ((k != i) && (k != j)) {
                             mbr = this->GetIndexMbr(oldNode);
                             d =  this->GetMbrArea(mbr);
                             delete mbr;
                             idx = oldNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                             mbr = this->GetIndexMbr(oldNode);
                             mindist = this->GetMbrArea(mbr) - d;
                             delete mbr;

                             mbr = this->GetIndexMbr(newNode);
                             d =  this->GetMbrArea(mbr);
                             delete mbr;
                             idx = newNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                             mbr = this->GetIndexMbr(newNode);
                             maxdist = this->GetMbrArea(mbr) - d;
                             delete mbr;

                             // leave the element in the node with less enlargment
                             if (mindist < maxdist) {
                                 newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                             }
                             else {
                                 oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
                             }
                         }
                     }
                     mbr = this->GetIndexMbr(oldNode);
                     d = this->GetMbrArea(mbr);
                     delete mbr;
                     mbr = this->GetIndexMbr(newNode);
                     d = d + this->GetMbrArea(mbr);
                     delete mbr;
                     if (d < totalMbrEnlargement) {
                         totalMbrEnlargement = d;
                         oldNodeBestEntry = i;
                         newNodeBestEntry = j;
                     }
                 }
             }
         }

         oldNode->RemoveAll();
         idx = oldNode->AddEntry(allobjects[oldNodeBestEntry]->GetSerializedSize(), allobjects[oldNodeBestEntry]->Serialize());
         oldNode->GetIndexEntry(idx).PageID = allPageIDs[oldNodeBestEntry];
         oldNode->GetIndexEntry(idx).NEntries = allNEntries[oldNodeBestEntry];
         oldNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[oldNodeBestEntry];

         newNode->RemoveAll();
         idx = newNode->AddEntry(allobjects[newNodeBestEntry]->GetSerializedSize(), allobjects[newNodeBestEntry]->Serialize());
         newNode->GetIndexEntry(idx).PageID = allPageIDs[newNodeBestEntry];
         newNode->GetIndexEntry(idx).NEntries = allNEntries[newNodeBestEntry];
         newNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[newNodeBestEntry];

         // consider each remaining object assigning it to the node requiring the smallest enlargement of its respective mbr
         for (k = 0; k < numberOfEntries; k++) {
             if ((k != oldNodeBestEntry) && (k != newNodeBestEntry)) {
                 mbr = this->GetIndexMbr(oldNode);
                 d = this->GetMbrArea(mbr);
                 delete mbr;
                 idx = oldNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                 oldNode->GetIndexEntry(idx).PageID = allPageIDs[k];
                 oldNode->GetIndexEntry(idx).NEntries = allNEntries[k];
                 oldNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[k];
                 mbr = this->GetIndexMbr(oldNode);
                 mindist = this->GetMbrArea(mbr) - d;
                 delete mbr;

                 mbr = this->GetIndexMbr(newNode);
                 d =  this->GetMbrArea(mbr);
                 delete mbr;
                 idx = newNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                 newNode->GetIndexEntry(idx).PageID = allPageIDs[k];
                 newNode->GetIndexEntry(idx).NEntries = allNEntries[k];
                 newNode->GetIndexEntry(idx).sonIsLeaf = allSonIsLeaf[k];
                 mbr = this->GetIndexMbr(newNode);
                 maxdist = this->GetMbrArea(mbr) - d;
                 delete mbr;

                 // leave the element in the node with less enlargment
                 if (mindist < maxdist) {
                     newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                 }
                 else {
                     oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
                 }
             }
         }
         break; //end stRTree::smEXPONENTIAL

   };//end switch

   // Clean memory
   for (i = 0; i < numberOfEntries; i++) {
       delete allobjects[i];
   }
   delete allobjects;
   delete allPageIDs;
   delete allNEntries;
   delete allSonIsLeaf;
}
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
void stRTree<DataType,OIDType>::SplitLeaf(stRLeafNode * oldNode, stRLeafNode * newNode, basicArrayObject * newObj,
                                          stSubtreeInfo & promo1, stSubtreeInfo & promo2) {
   double d, mindist, maxdist;
   u_int32_t position1, position2, i, j, k, numberOfEntries;

   numberOfEntries = oldNode->GetNumberOfEntries() + 1;
   basicArrayObject ** allobjects = new basicArrayObject *[numberOfEntries];
   // copy objects to allobjects
   for (i = 0; i < numberOfEntries-1; i++) {
       allobjects[i] = new basicArrayObject();
       allobjects[i]->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
   }
   allobjects[numberOfEntries-1] = newObj->Clone();

   // Split it.
   switch (GetSplitMethod()) {
      case stRTree::smQUADRATIC:
         /**
         * For leaf nodes the quadratic split uses the same procedure as the linear
         * split. Please se the quadratic split at method SplitIndex().
         */
      case stRTree::smLINEAR:
         /**
         * The linear split chooses two objects as seeds for the two nodes, as far
         * apart from each other as possible. Then consider each remaining object
         * in a random order and assign it to the node requiring the smallest MBR
         * enlargement.
         */
         // get the 2 most distant objects
         maxdist = 0;
         position1 = 0;
         for (i = 1; i < numberOfEntries; i++) {
              d = me->GetDistance(allobjects[0],allobjects[i]);
              if (d > maxdist) {
                  maxdist = d;
                  position1 = i;
              }
         }
         maxdist = 0;
         position2 = 0;
         for (i = 0; i < numberOfEntries; i++) {
              if (i != position1) {
                 d = me->GetDistance(allobjects[position1],allobjects[i]);
                 if (d > maxdist) {
                     maxdist = d;
                     position2 = i;
                 }
              }
         }
         maxdist = 0;
         position1 = 0;
         for (i = 0; i < numberOfEntries; i++) {
              if (i != position2) {
                 d = me->GetDistance(allobjects[position2],allobjects[i]);
                 if (d > maxdist) {
                     maxdist = d;
                     position1 = i;
                 }
              }
         }
         // now allobjects[position1] and allobjects[position2] is the pair of objects with greater distance from one another

         oldNode->RemoveAll();
         oldNode->AddEntry(allobjects[position1]->GetSerializedSize(), allobjects[position1]->Serialize());

         newNode->RemoveAll();
         newNode->AddEntry(allobjects[position2]->GetSerializedSize(), allobjects[position2]->Serialize());

         // consider each remaining object assigning it to the node requiring the smallest enlargement of its respective mbr
         for (i = 0; i < numberOfEntries; i++) {
            if ((i != position1) && (i != position2)) {
                promo1.Mbr = this->GetLeafMbr(oldNode);
                d =  this->GetMbrArea(promo1.Mbr);
                oldNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                delete promo1.Mbr;
                promo1.Mbr = this->GetLeafMbr(oldNode);
                mindist = this->GetMbrArea(promo1.Mbr) - d;

                promo2.Mbr = this->GetLeafMbr(newNode);
                d =  this->GetMbrArea(promo2.Mbr);
                newNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                delete promo2.Mbr;
                promo2.Mbr = this->GetLeafMbr(newNode);
                maxdist = this->GetMbrArea(promo2.Mbr) - d;

                // leave the element in the node with less enlargment
                if (mindist < maxdist) {
                    newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                    promo2.Mbr = this->GetLeafMbr(newNode);
                }
                else {
                    oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
                    promo1.Mbr = this->GetLeafMbr(oldNode);
                }
            }
         }
         promo1.RootID = oldNode->GetPageID();
         promo1.NObjects = oldNode->GetTotalObjectCount();
         promo1.sonIsLeaf = true;

         promo2.RootID = newNode->GetPageID();
         promo2.NObjects = newNode->GetTotalObjectCount();
         promo2.sonIsLeaf = true;

         break; //end stRTree::smLINEAR

      case stRTree::smEXPONENTIAL:
         /**
         * In the exponential split all possible groupings are exhaustively tested
         * and the best is chosen with respect to the minimization of the MBR
         * enlargement.
         */
         double totalMbrEnlargement = MAXDOUBLE;
         int oldNodeBestEntry = -1;
         int newNodeBestEntry = -1;
         // try all possible groupings
         for (i = 0; i < numberOfEntries; i++) {
             for (j = i; j < numberOfEntries; j++) {
                 if (i != j) {
                     oldNode->RemoveAll();
                     oldNode->AddEntry(allobjects[i]->GetSerializedSize(), allobjects[i]->Serialize());
                     newNode->RemoveAll();
                     newNode->AddEntry(allobjects[j]->GetSerializedSize(), allobjects[j]->Serialize());

                     // consider each remaining object assigning it to the node requiring the smallest enlargement of its respective mbr
                     for (k = 0; k < numberOfEntries; k++) {
                         if ((k != i) && (k != j)) {
                             promo1.Mbr = this->GetLeafMbr(oldNode);
                             d = this->GetMbrArea(promo1.Mbr);
                             oldNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                             delete promo1.Mbr;
                             promo1.Mbr = this->GetLeafMbr(oldNode);
                             mindist = this->GetMbrArea(promo1.Mbr) - d;

                             promo2.Mbr = this->GetLeafMbr(newNode);
                             d = this->GetMbrArea(promo2.Mbr);
                             newNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                             delete promo2.Mbr;
                             promo2.Mbr = this->GetLeafMbr(newNode);
                             maxdist = this->GetMbrArea(promo2.Mbr) - d;

                             // leave the element in the node with less enlargment
                             if (mindist < maxdist) {
                                 newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                                 promo2.Mbr = this->GetLeafMbr(newNode);
                             }
                             else {
                                 oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
                                 promo1.Mbr = this->GetLeafMbr(oldNode);
                             }
                         }
                     }
                     d = this->GetMbrArea(promo1.Mbr) + this->GetMbrArea(promo2.Mbr);
                     if (d < totalMbrEnlargement) {
                         totalMbrEnlargement = d;
                         oldNodeBestEntry = i;
                         newNodeBestEntry = j;
                     }
                 }
             }
         }

         oldNode->RemoveAll();
         oldNode->AddEntry(allobjects[oldNodeBestEntry]->GetSerializedSize(), allobjects[oldNodeBestEntry]->Serialize());
         newNode->RemoveAll();
         newNode->AddEntry(allobjects[newNodeBestEntry]->GetSerializedSize(), allobjects[newNodeBestEntry]->Serialize());

         // consider each remaining object assigning it to the node requiring the smallest enlargement of its respective mbr
         for (k = 0; k < numberOfEntries; k++) {
             if ((k != oldNodeBestEntry) && (k != newNodeBestEntry)) {
                 promo1.Mbr = this->GetLeafMbr(oldNode);
                 d = this->GetMbrArea(promo1.Mbr);
                 oldNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                 delete promo1.Mbr;
                 promo1.Mbr = this->GetLeafMbr(oldNode);
                 mindist = this->GetMbrArea(promo1.Mbr) - d;

                 promo2.Mbr = this->GetLeafMbr(newNode);
                 d = this->GetMbrArea(promo2.Mbr);
                 newNode->AddEntry(allobjects[k]->GetSerializedSize(), allobjects[k]->Serialize());
                 delete promo2.Mbr;
                 promo2.Mbr = this->GetLeafMbr(newNode);
                 maxdist = this->GetMbrArea(promo2.Mbr) - d;

                 // leave the element in the node with less enlargment
                 if (mindist < maxdist) {
                     newNode->RemoveEntry(newNode->GetNumberOfEntries()-1);
                     delete promo2.Mbr;
                     promo2.Mbr = this->GetLeafMbr(newNode);
                 }
                 else {
                     oldNode->RemoveEntry(oldNode->GetNumberOfEntries()-1);
                     delete promo1.Mbr;
                     promo1.Mbr = this->GetLeafMbr(oldNode);
                 }
             }
         }

         promo1.RootID = oldNode->GetPageID();
         promo1.NObjects = oldNode->GetTotalObjectCount();
         promo1.sonIsLeaf = true;

         promo2.RootID = newNode->GetPageID();
         promo2.NObjects = newNode->GetTotalObjectCount();
         promo2.sonIsLeaf = true;

         break; //end stRTree::smEXPONENTIAL
   };//end switch

   // Clean memory
   for (i = 0; i < numberOfEntries; i++) {
       delete allobjects[i];
   }
   delete allobjects;
}//end stRTree::SplitLeaf
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
void stRTree<DataType,OIDType>::AddNewRoot(basicArrayObject *mbrA, u_int32_t nodeID1, u_int32_t nEntries1, bool sonIsLeaf1,
                                           basicArrayObject *mbrB, u_int32_t nodeID2, u_int32_t nEntries2, bool sonIsLeaf2) {
   stPage * newPage;
   stRIndexNode * newRoot;
   int idx;

   // Create a new node
   newPage = this->NewPage();
   newRoot = new stRIndexNode(newPage, true);

   // Add mbrA
   idx = newRoot->AddEntry(mbrA->GetSerializedSize(), mbrA->Serialize());
   newRoot->GetIndexEntry(idx).PageID = nodeID1;
   newRoot->GetIndexEntry(idx).NEntries = nEntries1;
   newRoot->GetIndexEntry(idx).sonIsLeaf = sonIsLeaf1;

   // Add mbrB
   idx = newRoot->AddEntry(mbrB->GetSerializedSize(), mbrB->Serialize());
   newRoot->GetIndexEntry(idx).PageID = nodeID2;
   newRoot->GetIndexEntry(idx).NEntries = nEntries2;
   newRoot->GetIndexEntry(idx).sonIsLeaf = sonIsLeaf2;

   // Update tree
   Header->Height++;
   SetRoot(newRoot->GetPage()->GetPageID());
   this->myPageManager->WritePage(newPage);

   // Dispose page
   delete newRoot;
   this->myPageManager->ReleasePage(newPage);
}
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
stResult< stBasicArrayObject<DataType,OIDType> > * stRTree<DataType,OIDType>::NearestQuery(basicArrayObject * sample, u_int32_t k, bool tie) {
    // Create the result
    tResult * result = new tResult();  // Create result
    result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

    // compute the query only if the used assigned a metric evaluator through SetQueryMetricEvaluator()
    if (queryMetricEvaluator != NULL) {
        // Let's search
        if (this->GetRoot() != 0){
           this->NearestQuery(result, sample, MAXDOUBLE, k);
        }//end if
    }

    return result;
}
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
void stRTree<DataType,OIDType>::NearestQuery(tResult * result, basicArrayObject * sample, double rangeK, u_int32_t k) {
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stRNode * currNode;
   basicArrayObject tmpObj;
   double distance;
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
      currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stRNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stRNode::INDEX) {
         // Get Index node
         stRIndexNode * indexNode = (stRIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = MinDistance(sample, &tmpObj);
            if (distance <= rangeK) {
               // Yes! I'm qualified! Put it in the queue.
               pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
               queue->Add(distance, pqTmpValue);
            }//end if
         }//end for
      }
      else{
         // No, it is a leaf node. Get it.
         stRLeafNode * leafNode = (stRLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
             // Evaluate distance
            distance = queryMetricEvaluator->GetDistance(&tmpObj, sample);
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
         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);

      // Go to next node
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            // Qualified if distance <= rangeK
            if (distance <= rangeK){
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
}
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
stResult< stBasicArrayObject<DataType,OIDType> > * stRTree<DataType,OIDType>::RangeQuery(basicArrayObject * sample, double range) {
    // Create the result
    tResult * result = new tResult();
    result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

    // compute the query only if the used assigned a metric evaluator through SetQueryMetricEvaluator()
    if (queryMetricEvaluator != NULL) {
        stPage * currPage;
        stRNode * currNode;
        basicArrayObject tmpObj;
        u_int32_t idx, numberOfEntries;
        double distance;

        // Evaluate the root node.
        if (this->GetRoot() != 0){
           // Read node...
           currPage = this->myPageManager->GetPage(this->GetRoot());
           currNode = stRNode::CreateNode(currPage);

           // Is it an Index node?
           if (currNode->GetNodeType() == stRNode::INDEX){
              // Get Index node
              stRIndexNode * indexNode = (stRIndexNode *)currNode;
              numberOfEntries = indexNode->GetNumberOfEntries();

              // For each entry...
              for (idx = 0; idx < numberOfEntries; idx++) {
                 // Rebuild the object
                 tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
                 // test if this subtree qualifies.
                 if (IntersectionBetweenMbrAndBall(&tmpObj, sample, range)) {
                    // Yes! Analyze this subtree.
                    this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result, sample, range);
                 }//end if
              }//end for
           }
           else{
              // No, it is a leaf node. Get it.
              stRLeafNode * leafNode = (stRLeafNode *)currNode;
              numberOfEntries = leafNode->GetNumberOfEntries();

              // For each entry...
              for (idx = 0; idx < numberOfEntries; idx++) {
                 // Rebuild the object
                 tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
                 // Evaluate distance
                 distance = queryMetricEvaluator->GetDistance(&tmpObj, sample);
                 // is it a object that qualified?
                 if (distance <= range){
                    // Yes! Put it in the result set.
                    result->AddPair(tmpObj.Clone(), distance);
                 }//end if
              }//end for
           }//end else

           // Free it all
           delete currNode;
           this->myPageManager->ReleasePage(currPage);
        }//end if
    }
    return result;
}//end stRTree::RangeQuery
//------------------------------------------------------------------------------
template <class DataType, class OIDType>
void stRTree<DataType,OIDType>::RangeQuery(u_int32_t pageID, tResult * result, basicArrayObject * sample, double range) {
   stPage * currPage;
   stRNode * currNode;
   basicArrayObject tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stRNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stRNode::INDEX) {
         // Get Index node
         stRIndexNode * indexNode = (stRIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
               // test if this subtree qualifies.
               if (IntersectionBetweenMbrAndBall(&tmpObj, sample, range)) {
                  // Yes! Analyze it!
                  this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result, sample, range);
               }//end if
         }//end for
      }
      else{
         // No, it is a leaf node. Get it.
         stRLeafNode * leafNode = (stRLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));

               // Evaluate distance
               distance = queryMetricEvaluator->GetDistance(&tmpObj, sample);

               // Is this a qualified object?
               if (distance <= range){
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
               }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end stRTree::RangeQuery
//------------------------------------------------------------------------------

