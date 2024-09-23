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
* This file is the implementation of Partition.
*
* @version 1.0
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @todo Review of documentation.
*/
//==============================================================================
// Class stPartitionHash
//------------------------------------------------------------------------------

#include <iostream>
#include <cstdlib>

// This macro will be used to replace the declaration of
// stPartitionHash<ObjectType, EvaluatorType>
#define tmpl_stPartitionHash stPartitionHash<ObjectType, EvaluatorType>

template <class ObjectType, class EvaluatorType>
tmpl_stPartitionHash::stPartitionHash(stPageManager * pageman):
    stMetricTree<ObjectType, EvaluatorType>(pageman){
   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   //creating Global Representatives
   RepObjects = new tmpl_stPartitionGlobalRep(myMetricEvaluator);

   // Load header.
   LoadHeader();

   // Will I create or load the tree ?
   if (myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if

   //load representatives
   LoadRepresentatives();
}//end stPartitionHash::stPartitionHash

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
tmpl_stPartitionHash::~stPartitionHash(){

   //clean home
   delete RepObjects;

   //Flush header page.
   FlushHeader();
}//end stPartitionHash::~stPartitionHash

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::DefaultHeader(){
   // Clear header page.
   HeaderPage->Clear();

   // Default values
   Header->Magic[0] = 'P';
   Header->Magic[1] = 'A';
   Header->Magic[2] = 'R';
   Header->Magic[3] = 'T';
   Header->ChooseMethod = cmVARIANCE;
   Header->RootIndex = 0;
   Header->RootLeaf = 0;
   Header->RootRep = 0;
   Header->NumberRep = 0;
   Header->Height = 0;
   Header->ObjectCount = 0;
   Header->IndexBucketCount = 0;
   Header->LeafBucketCount = 0;
   Header->RepNodeCount = 0;

   // Notify modifications
   HeaderUpdate = true;
}//end stPartitionHash::DefaultHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::LoadHeader(){
   if (HeaderPage != NULL){
      myPageManager->ReleasePage(HeaderPage);
   }//end if

   // Load and set the header.
   HeaderPage = myPageManager->GetHeaderPage();
   if (HeaderPage->GetPageSize() < sizeof(stPartitionHeader)){
      throw std::logic_error("The page size is too small.");
   }//end if

   Header = (stPartitionHeader *) HeaderPage->GetData();
   HeaderUpdate = false;
}//end stPartitionHash::LoadHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::FlushHeader(){
   if (HeaderPage != NULL){
      if (Header != NULL){
         WriteHeader();
      }//end if
      myPageManager->ReleasePage(HeaderPage);
   }//end if
}//end stPartitionHash::FlushHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::LoadRepresentatives(){

}//end stPartitionHash::LoadRepresentatives

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SaveRepresentatives()
{

}//end stPartitionHash::SaveRepresentatives

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stPartitionHash::Add(ObjectType *newObj){

   stPartitionRegionDesc begin = GetBeginRegion();
   stPartitionRegionDesc end = GetEndRegion();
   stPartitionSubtreeInfo split;
   stPartitionSubtreeInfo promo;
   stPartitionRegionDesc regObj;
   ObjectHashFunction(newObj, regObj);
   cout << "objReg = " << regObj.GetRegion() << "[" << (int)regObj.GetNumberRep() << "] ";


   // Is there a root in leaf bucket?
   if (GetRootLeaf() == 0){
      // No! We shall create the new node.
      stPage * auxPage = NewPage(stPartitionNode::LEAF);
      stPartitionLeafBucket * leafBucket = new stPartitionLeafBucket(auxPage, true);
      SetRootLeaf(auxPage->GetPageID());
      Header->Height++; // Update height
      // Write nodes
      myPageManager->WritePage(auxPage);
      //Clean home.
      myPageManager->ReleasePage(auxPage);
      delete leafBucket;
   }//end if

   // Recursive Insertion
   if (InsertRecursive(GetRoot(),regObj,newObj,begin,end,split,promo) == PROMOTION){
      AddNewRoot(split, promo);
   }//end if

   // Update object count
   UpdateObjectCounter(1);

   // Write Header!
   WriteHeader();

   cout << endl;

   return true;
}//end stPartitionHash::Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stPartitionHash::InsertRecursive(
      u_int32_t curBucketID, stPartitionRegionDesc regionIns, ObjectType * objIns,
      stPartitionRegionDesc beginRegion, stPartitionRegionDesc endRegion,
      stPartitionSubtreeInfo & splited, stPartitionSubtreeInfo & promoted){
   int result;

   // Read node...
   stPage * curPage = myPageManager->GetPage(curBucketID);
   stPartitionNode * curBucket = stPartitionNode::CreateBucket(curPage);

   // What shall I do ?
   if (curBucket->GetBucketType() == stPartitionNode::INDEX){
      // variables
      stPartitionIndexBucket *indexBucket = (stPartitionIndexBucket *)curBucket;
      int idxSub;
      indexBucket->ChoiceSubBucket(RepObjects, regionIns, idxSub, beginRegion, endRegion);

      // if ocorred a promotion
      if (InsertRecursive(indexBucket->GetEntryPageId(idxSub), regionIns,
            objIns, beginRegion, endRegion, splited, promoted) == PROMOTION){

         //updating splited bucket
         //indexBucket->SetEntryPageId(idxSub, splited.PageID);
         //indexBucket->SetRegionDesc(RepObjects, idxSub, splited.Region);

         // if obj was not added in the bucket
         if (indexBucket->AddEntry(RepObjects, promoted.Region, promoted.PageID) < 0){

            cout << "split index | ";
            // Slip index
            stPage * newIndexPage  = NewPage(stPartitionNode::INDEX);
            stPartitionIndexBucket * newIndexBucket = new stPartitionIndexBucket(newIndexPage, true);
            SplitIndex(indexBucket, newIndexBucket, splited, promoted);
            //Write node
            myPageManager->WritePage(newIndexPage);
            //Clean home
            myPageManager->ReleasePage(newIndexPage);
            delete newIndexBucket;
            result = PROMOTION;

/*         cout << "split.reg [" << (int)splited.Region.GetNumberRep() <<
            "] = " << splited.Region.GetRegion() << " | ";
         cout << "promo.reg [" << (int)promoted.Region.GetNumberRep() <<
            "] = " << promoted.Region.GetRegion() << " | ";
*/
         }else{
            result = NO_ACT;
         }//end if
         //Write node
         myPageManager->WritePage(curPage);
      }else{
         result = NO_ACT;
      }//end if
   }else{

      // Leaf Node cast.
      stPartitionLeafBucket *leafBucket = (stPartitionLeafBucket *)curBucket;

      cout << "beg = " << beginRegion.GetRegion() << " | ";
      cout << "end = " << endRegion.GetRegion() << " | ";

      // if obj was not added in the bucket
      if (leafBucket->AddEntry(RepObjects, regionIns, objIns->GetSerializedSize(), objIns->Serialize()) < 0){
         // Slip leaf
         stPage * newLeafPage = NewPage(stPartitionNode::LEAF);
         stPartitionLeafBucket * newLeafBucket = new stPartitionLeafBucket(newLeafPage, true);

         //If not exists representatives
         if (GetGlobalNumberRep() == 0){
            cout << "split leaft two rep | ";

            switch (Header->ChooseMethod){
               case cmRANDOM:
                  SplitLeafChoiceTwoRepRandom(leafBucket, newLeafBucket,
                     regionIns, objIns, splited, promoted);
                  break;
               case cmVARIANCE:
                  SplitLeafChoiceTwoRepByVariance(leafBucket, newLeafBucket,
                     regionIns, objIns, splited, promoted);
                  break;
               case cmDIFFER:
                  SplitLeafChoiceTwoRepByDiffer(leafBucket, newLeafBucket,
                     regionIns, objIns, splited, promoted);
                  break;
            }
         }else{

            //if can be update region in the full leaf bucket
            if ((leafBucket->GetRegionDesc(0).IsEqualLargeBase(RepObjects,
                  leafBucket->GetRegionDesc(leafBucket->GetNumberOfEntries()-1))) &&
                  (leafBucket->GetRegionDesc(0).GetNumberRep() < GetGlobalNumberRep())){
               cout << "**********************";
               UpdateRegionLeaf(leafBucket);
            }//end if

            //Add new global representative
            if ((leafBucket->GetRegionDesc(0).IsEqualLargeBase(RepObjects,
                  leafBucket->GetRegionDesc(leafBucket->GetNumberOfEntries()-1)))){
               cout << "split leaft one rep | ";
               switch (Header->ChooseMethod){
                  case cmRANDOM:
                     SplitLeafChoiceOneRepRandom(leafBucket, newLeafBucket,
                        regionIns, objIns, splited, promoted);
                     break;
                  case cmVARIANCE:
                     SplitLeafChoiceOneRepByVariance(leafBucket, newLeafBucket,
                        regionIns, objIns, splited, promoted);
                     break;
                  case cmDIFFER:
                     SplitLeafChoiceOneRepByDiffer(leafBucket, newLeafBucket,
                        regionIns, objIns, splited, promoted);
                     break;
               }//end switch
            }else{
               cout << "split leaf without rep | ";
               SplitLeafWithoutChoiceRep(leafBucket, newLeafBucket,
                  regionIns, objIns, splited, promoted);
            }//end if
         }//end if

/*         cout << "split.reg [" << (int)splited.Region.GetNumberRep() <<
            "] = " << splited.Region.GetRegion() << " | ";
         cout << "promo.reg [" << (int)promoted.Region.GetNumberRep() <<
            "] = " << promoted.Region.GetRegion() << " | ";
*/
         // Write node
         myPageManager->WritePage(newLeafPage);
         // Clean home
         myPageManager->ReleasePage(newLeafPage);
         delete newLeafBucket;
         result = PROMOTION;
      }else{
         result = NO_ACT;
      }//end if
      //Write node
      myPageManager->WritePage(curPage);
   }//end if
   //Clean home
   myPageManager->ReleasePage(curPage);
   delete curBucket;
   return result;
}//end stPartitionHash::InsertRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::AddNewRoot(
      stPartitionSubtreeInfo splited, stPartitionSubtreeInfo promoted){
   stPage * newPage = NewPage(stPartitionNode::INDEX);
   stPartitionIndexBucket * newRoot = new stPartitionIndexBucket(newPage, true);

   // Add obj1
   newRoot->AddEntry(RepObjects, splited.Region, splited.PageID);

   // Add obj2
   newRoot->AddEntry(RepObjects, promoted.Region, promoted.PageID);

   // Update tree
   Header->Height++;
   SetRootIndex(newRoot->GetPageID());
   //Write node
   myPageManager->WritePage(newPage);

   // Clean home
   myPageManager->ReleasePage(newPage);
   delete newRoot;
}//end SlimTree::AddNewRoot

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitIndex (stPartitionIndexBucket * fullBucket,
      stPartitionIndexBucket * promoteBucket, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted){

   //this method divide in a half the number of entries of a index bucket
   for (int i = (int)(fullBucket->GetNumberOfEntries()/2);
         i < fullBucket->GetNumberOfEntries(); i++){
      promoteBucket->AddEntry(RepObjects,
         fullBucket->GetRegionDesc(i), fullBucket->GetEntryPageId(i));
   }//end for

   // update the occupation of bucket splited
   fullBucket->RemoveNLast(promoteBucket->GetNumberOfEntries());

   // inserting promoted bucket
   if (promoted.Region.IsMoreEqualLargeBase(RepObjects, promoteBucket->GetRegionDesc(0))){
      promoteBucket->AddEntry(RepObjects, promoted.Region, promoted.PageID);
   }else{
      fullBucket->AddEntry(RepObjects, promoted.Region, promoted.PageID);
   }//end if

   //update next bucket of new index
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //update next bucket splited index bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating information about splited bucket
   splited.Region = fullBucket->GetRegionDesc(0);
   splited.PageID = fullBucket->GetPageID();

   // updating information about promoted bucket
   promoted.Region = promoteBucket->GetRegionDesc(0);
   promoted.PageID = promoteBucket->GetPageID();
}//end stPartitionHash::SplitIndex

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafChoiceTwoRepRandom(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   ObjectType * objRep1 = new ObjectType();
   ObjectType * objRep2 = new ObjectType();
   ObjectType * insObj = new ObjectType();
   stPartitionRegionDesc insReg;
   int idx1, idx2;

   //sorting first representative
   idx1 = random(RAND_MAX) % fullBucket->GetNumberOfEntries();
   //Adding first representative
   objRep1->Unserialize(fullBucket->GetObject(idx1), fullBucket->GetObjectSize(idx1));
   RepObjects->Add(0, objRep1);

   do {
      //remove all entries of prometed and splited bucket
      promoteBucket->RemoveAll();
      splitBucket->RemoveAll();

      //sorting candidate to second representative
      do {
         idx2 = random(RAND_MAX) % fullBucket->GetNumberOfEntries();
      } while(idx1 == idx2);
      //Adding Candidate
      objRep2->Unserialize(fullBucket->GetObject(idx2), fullBucket->GetObjectSize(idx2));
      RepObjects->AddCandidate(1, objRep2);

      // updating region about promoted bucket (region 2 and 3)
      promoted.Region.SetNumberRepresentatives(GetGlobalNumberRep());
      promoted.Region.SetRegion(2);
      promoted.PageID = promoteBucket->GetPageID();

      for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
         insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
         ObjectHashFunction(insObj, insReg);

         // insering in prometed bucket or splited bucket
         if (insReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
            promoteBucket->AddEntry(RepObjects, insReg, insObj->GetSerializedSize(), insObj->Serialize());
         }else{
            splitBucket->AddEntry(RepObjects, insReg, insObj->GetSerializedSize(), insObj->Serialize());
         }//end if
      }//end for

      //removing candidate representative object
      RepObjects->RemoveCandidate();

   // checking if
   } while ((promoteBucket->GetNumberOfEntries() == fullBucket->GetNumberOfEntries()) ||
      (splitBucket->GetNumberOfEntries() == fullBucket->GetNumberOfEntries()));

   //Adding second representative
   objRep2->Unserialize(fullBucket->GetObject(idx2), fullBucket->GetObjectSize(idx2));
   RepObjects->Add(1, objRep2);

   //Adding responsible for splited
   ObjectHashFunction(newObj, newReg);
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects,fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket (region 0 and 1)
   splited.Region.SetNumberRepresentatives(GetGlobalNumberRep());
   splited.Region.SetRegion(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
   delete objRep1;
   delete objRep2;

}//end stPartitionHash::SplitLeafChoiceTwoRepRandom

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafChoiceTwoRepByVariance(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   ObjectType * canRep = new ObjectType();
   ObjectType * insObj = new ObjectType();
   int divider=(int)(fullBucket->GetNumberOfEntries()/2);
   int moreDistribI=1;
   int moreDistribJ=0;
   int aux;
   double ** vetMedia = new double * [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      vetMedia[i] = new double [i];
   }//end for
   double ** vetVariance = new double * [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      vetVariance[i] = new double [i];
   }//end for
   stPartitionRegionDesc *** matReg = new stPartitionRegionDesc ** [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      matReg[i] = new stPartitionRegionDesc * [i];
      for(int j=0;j<i;j++){
         matReg[i][j] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
      }//end for
   }//end for
   stPartitionRegionDesc *** matRegOrd = new stPartitionRegionDesc ** [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      matRegOrd[i] = new stPartitionRegionDesc * [i];
      for(int j=0;j<i;j++){
         matRegOrd[i][j] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
      }//end for
   }//end for

   // calculating matReg
   for (int i=1;i < fullBucket->GetNumberOfEntries();i++){
      for (int j=0;j < i; j++){
         vetVariance[i][j] = 0;
         vetMedia[i][j] = 0;
         //adding representative object
         canRep->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
         RepObjects->Add(0, canRep);
         canRep->Unserialize(fullBucket->GetObject(j), fullBucket->GetObjectSize(j));
         RepObjects->Add(1, canRep);

         //Calculating the HashFunction for object i and j
         for (int k=0;k < fullBucket->GetNumberOfEntries();k++){
            insObj->Unserialize(fullBucket->GetObject(k), fullBucket->GetObjectSize(k));
            ObjectHashFunction(insObj, matReg[i][j][k]);
            //Orded Insertion
            aux=k;
            while ((aux>0) && (matReg[i][j][k].IsLessLargeBase(RepObjects, matRegOrd[i][j][aux-1]))){
               matRegOrd[i][j][aux]=matRegOrd[i][j][aux-1];
               aux--;
            }
            matRegOrd[i][j][aux] = matReg[i][j][k];
            //Sum item in media
            vetMedia[i][j] += matReg[i][j][k].GetRegion();
         }//end for
         //Calcule media
         vetMedia[i][j] = vetMedia[i][j] / fullBucket->GetNumberOfEntries();
         //Calculating the variance for object i
         for (int k=0;k < fullBucket->GetNumberOfEntries();k++){
            vetVariance[i][j] += pow((double)matReg[i][j][k].GetRegion()-vetMedia[i][j],2);
         }//end for
         vetVariance[i][j] = vetVariance[i][j] / (fullBucket->GetNumberOfEntries() - 1);
         if (vetVariance[i][j] > vetVariance[moreDistribI][moreDistribJ]){
            moreDistribI = i;
            moreDistribJ = j;
         }//end if
         //removing all representative object
         RepObjects->RemoveAll();
      }//end for
   }//end for

   // the best divider
   aux=1;
   // searching the descriptor region that best divide the bucket
   while ((divider-aux>=0) && (divider+aux < fullBucket->GetNumberOfEntries()) &&
      (matRegOrd[moreDistribI][moreDistribJ][divider].GetRegion()==
      matRegOrd[moreDistribI][moreDistribJ][divider-aux].GetRegion()) &&
      (matRegOrd[moreDistribI][moreDistribJ][divider].GetRegion()==
      matRegOrd[moreDistribI][moreDistribJ][divider+aux].GetRegion())){
      aux++;
   }//end while

   //Adding in global representative objects
   insObj->Unserialize(fullBucket->GetObject(moreDistribI), fullBucket->GetObjectSize(moreDistribI));
   RepObjects->Add(0, insObj);
   insObj->Unserialize(fullBucket->GetObject(moreDistribJ), fullBucket->GetObjectSize(moreDistribJ));
   RepObjects->Add(1, insObj);

   //updating region about promoted bucket
   //if promotion value is big than divider
   if((divider+aux < fullBucket->GetNumberOfEntries()) &&
         (matRegOrd[moreDistribI][moreDistribJ][divider].GetRegion()!=
         matRegOrd[moreDistribI][moreDistribJ][divider+aux].GetRegion())){
      promoted.Region = matRegOrd[moreDistribI][moreDistribJ][divider+aux];
   }else{
      //if promotion value is less than divider
      promoted.Region = matRegOrd[moreDistribI][moreDistribJ][divider];
   }//end if
   promoted.PageID = promoteBucket->GetPageID();

   for (int i=1;i < fullBucket->GetNumberOfEntries();i++){
      for (int j=0;j < i;j++){
         cout << endl << "i=" << i << "j=" << j << " | ";
         for (int k=1;k < fullBucket->GetNumberOfEntries();k++){
            cout << matRegOrd[i][j][k].GetRegion() << " | ";
         }//end for
         cout << "med=" << vetMedia[i][j] << " | var=" << vetVariance[i][j];
      }//end for
   }//end for
   cout << endl << endl;
   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      cout << matRegOrd[moreDistribI][moreDistribJ][i].GetRegion() << " | ";
   }//end for

   //adding in promoteBucket or splitBucket
   for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
      insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      // insering in prometed bucket or splited bucket
      if (matReg[moreDistribI][moreDistribJ][i].IsMoreEqualLargeBase(RepObjects, promoted.Region)){
         promoteBucket->AddEntry(RepObjects, matReg[moreDistribI][moreDistribJ][i], insObj->GetSerializedSize(), insObj->Serialize());
      }else{
         splitBucket->AddEntry(RepObjects, matReg[moreDistribI][moreDistribJ][i], insObj->GetSerializedSize(), insObj->Serialize());
      }//end if
   }//end for

   //Adding responsible for splited
   ObjectHashFunction(newObj, newReg);
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects,fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket
   splited.Region.SetNumberRepresentatives(GetGlobalNumberRep());
   splited.Region.SetRegion(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
   delete canRep;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      delete [] vetMedia[i];
   }//end for
   delete [] vetMedia;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      delete [] vetVariance[i];
   }//end for
   delete [] vetVariance;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      for(int j=0;i<j;j++){
         delete [] matReg[i][j];
      }
      delete [] matReg[i];
   }//end for
   delete [] matReg;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      for(int j=0;i<j;j++){
         delete [] matRegOrd[i][j];
      }
      delete [] matRegOrd[i];
   }//end for
   delete [] matRegOrd;
}//end stPartitionHash::SplitLeafChoiceTwoRepByVariance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafChoiceTwoRepByDiffer(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   ObjectType * canRep = new ObjectType();
   ObjectType * insObj = new ObjectType();
   int divider=(int)(fullBucket->GetNumberOfEntries()/2);
   int moreDistribI=1;
   int moreDistribJ=0;
   int aux;
   double ** vetDiff = new double * [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      vetDiff[i] = new double [i];
   }//end for
   stPartitionRegionDesc *** matReg = new stPartitionRegionDesc ** [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      matReg[i] = new stPartitionRegionDesc * [i];
      for(int j=0;j<i;j++){
         matReg[i][j] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
      }//end for
   }//end for
   stPartitionRegionDesc *** matRegOrd = new stPartitionRegionDesc ** [fullBucket->GetNumberOfEntries()];
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      matRegOrd[i] = new stPartitionRegionDesc * [i];
      for(int j=0;j<i;j++){
         matRegOrd[i][j] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
      }//end for
   }//end for

   // calculating matReg
   for (int i=1;i < fullBucket->GetNumberOfEntries();i++){
      for (int j=0;j < i; j++){
         vetDiff[i][j] = 0;
         //adding representative object
         canRep->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
         RepObjects->Add(0, canRep);
         canRep->Unserialize(fullBucket->GetObject(j), fullBucket->GetObjectSize(j));
         RepObjects->Add(1, canRep);

         //Calculating the HashFunction for object i and j
         for (int k=0;k < fullBucket->GetNumberOfEntries();k++){
            insObj->Unserialize(fullBucket->GetObject(k), fullBucket->GetObjectSize(k));
            ObjectHashFunction(insObj, matReg[i][j][k]);
            //Orded Insertion
            aux=k;
            while ((aux>0) && (matReg[i][j][k].IsLessLargeBase(RepObjects, matRegOrd[i][j][aux-1]))){
               matRegOrd[i][j][aux]=matRegOrd[i][j][aux-1];
               aux--;
            }
            matRegOrd[i][j][aux] = matReg[i][j][k];
            // if exist region descriptor
            if ((aux==0) || ((aux > 0) && (matRegOrd[i][j][aux].GetRegion() !=
                  matRegOrd[i][j][aux-1].GetRegion()))){
               // increment Count
               vetDiff[i][j]++;
            }
         }//end for
         if (vetDiff[i][j] > vetDiff[moreDistribI][moreDistribJ]){
            moreDistribI = i;
            moreDistribJ = j;
         }//end if
         //removing all representative object
         RepObjects->RemoveAll();
      }//end for
   }//end for

   // the best divider
   aux=1;
   // searching the descriptor region that best divide the bucket
   while ((divider-aux>=0) && (divider+aux < fullBucket->GetNumberOfEntries()) &&
      (matRegOrd[moreDistribI][moreDistribJ][divider].GetRegion()==
      matRegOrd[moreDistribI][moreDistribJ][divider-aux].GetRegion()) &&
      (matRegOrd[moreDistribI][moreDistribJ][divider].GetRegion()==
      matRegOrd[moreDistribI][moreDistribJ][divider+aux].GetRegion())){
      aux++;
   }//end while

   //Adding in global representative objects
   insObj->Unserialize(fullBucket->GetObject(moreDistribI), fullBucket->GetObjectSize(moreDistribI));
   RepObjects->Add(0, insObj);
   insObj->Unserialize(fullBucket->GetObject(moreDistribJ), fullBucket->GetObjectSize(moreDistribJ));
   RepObjects->Add(1, insObj);

   //updating region about promoted bucket
   //if promotion value is big than divider
   if((divider+aux < fullBucket->GetNumberOfEntries()) &&
         (matRegOrd[moreDistribI][moreDistribJ][divider].GetRegion()!=
         matRegOrd[moreDistribI][moreDistribJ][divider+aux].GetRegion())){
      promoted.Region = matRegOrd[moreDistribI][moreDistribJ][divider+aux];
   }else{
      //if promotion value is less than divider
      promoted.Region = matRegOrd[moreDistribI][moreDistribJ][divider];
   }//end if
   promoted.PageID = promoteBucket->GetPageID();

   for (int i=1;i < fullBucket->GetNumberOfEntries();i++){
      for (int j=0;j < i;j++){
         cout << endl << "i=" << i << "j=" << j << " | ";
         for (int k=1;k < fullBucket->GetNumberOfEntries();k++){
            cout << matRegOrd[i][j][k].GetRegion() << " | ";
         }//end for
         cout << "diff=" << vetDiff[i][j];
      }//end for
   }//end for
   cout << endl << endl;
   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      cout << matRegOrd[moreDistribI][moreDistribJ][i].GetRegion() << " | ";
   }//end for

   //adding in promoteBucket or splitBucket
   for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
      insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      // insering in prometed bucket or splited bucket
      if (matReg[moreDistribI][moreDistribJ][i].IsMoreEqualLargeBase(RepObjects, promoted.Region)){
         promoteBucket->AddEntry(RepObjects, matReg[moreDistribI][moreDistribJ][i], insObj->GetSerializedSize(), insObj->Serialize());
      }else{
         splitBucket->AddEntry(RepObjects, matReg[moreDistribI][moreDistribJ][i], insObj->GetSerializedSize(), insObj->Serialize());
      }//end if
   }//end for

   //Adding responsible for splited
   ObjectHashFunction(newObj, newReg);
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects,fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket
   splited.Region.SetNumberRepresentatives(GetGlobalNumberRep());
   splited.Region.SetRegion(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
   delete canRep;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      delete [] vetDiff[i];
   }//end for
   delete [] vetDiff;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      for(int j=0;i<j;j++){
         delete [] matReg[i][j];
      }
      delete [] matReg[i];
   }//end for
   delete [] matReg;
   for(int i=1;i<fullBucket->GetNumberOfEntries();i++){
      for(int j=0;i<j;j++){
         delete [] matRegOrd[i][j];
      }
      delete [] matRegOrd[i];
   }//end for
   delete [] matRegOrd;
}//end stPartitionHash::SplitLeafChoiceTwoRepByDiffer

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafChoiceOneRepRandom(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   int idx, aux;
   stPartitionRegionDesc insReg;
   int divider = (int)(fullBucket->GetNumberOfEntries()/2);
   ObjectType * canRep = new ObjectType();
   ObjectType * insObj = new ObjectType();
   stPartitionRegionDesc * matReg = new stPartitionRegionDesc [fullBucket->GetNumberOfEntries()];

   do {
      //remove all entries of prometed and splited bucket
      promoteBucket->RemoveAll();
      splitBucket->RemoveAll();

      //sorting a candidate
      do {
         idx = random(RAND_MAX) % fullBucket->GetNumberOfEntries();
         canRep->Unserialize(fullBucket->GetObject(idx), fullBucket->GetObjectSize(idx));
      } while (RepObjects->DistanceIsZero(canRep));

      //Adding in global representative objects
      RepObjects->AddCandidate(newReg.GetFirstDigit(), canRep);

      // calculating matReg
      for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
         insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
         ObjectHashFunction(insObj, matReg[i]);
      }//end for

      // the best divider
      aux=1;
      // searching the descriptor region that best divide the bucket
      while ((divider-aux>=0) && (divider+aux < fullBucket->GetNumberOfEntries()) &&
         (matReg[divider].GetRegion()==matReg[divider-aux].GetRegion()) &&
         (matReg[divider].GetRegion()==matReg[divider+aux].GetRegion())){
         aux++;
      }//end while

      //updating region about promoted bucket
      //if promotion value is big than divider
      if((divider+aux < fullBucket->GetNumberOfEntries()) &&
            (matReg[divider].GetRegion()!=matReg[divider+aux].GetRegion())){
         promoted.Region = matReg[divider+aux];
      }else{
         //if promotion value is less than divider
         promoted.Region = matReg[divider];
      }//end if
      promoted.PageID = promoteBucket->GetPageID();

      //adding in promoteBucket or splitBucket
      for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
         insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
         // insering in prometed bucket or splited bucket
         if (matReg[i].IsMoreEqualLargeBase(RepObjects, promoted.Region)){
            promoteBucket->AddEntry(RepObjects, matReg[i], insObj->GetSerializedSize(), insObj->Serialize());
         }else{
            splitBucket->AddEntry(RepObjects, matReg[i], insObj->GetSerializedSize(), insObj->Serialize());
         }//end if
      }//end for

      //removing candidate representative object
      RepObjects->RemoveCandidate();

   // checking if
   } while ((promoteBucket->GetNumberOfEntries() == fullBucket->GetNumberOfEntries()) ||
      (splitBucket->GetNumberOfEntries() == fullBucket->GetNumberOfEntries()));

   //Adding second representative
   RepObjects->Add(newReg.GetFirstDigit(), canRep);

   //Adding responsible for splited
   ObjectHashFunction(newObj, newReg);
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects,fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket
   splited.Region = fullBucket->GetRegionDesc(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
   delete canRep;
   delete [] matReg;
}//end stPartitionHash::SplitLeafChoiceOneRepRandom

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafChoiceOneRepByVariance(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   ObjectType * canRep = new ObjectType();
   ObjectType * insObj = new ObjectType();
   int divider=(int)(fullBucket->GetNumberOfEntries()/2);
   int moreDistrib=0;
   int aux;
   double * vetMedia = new double [fullBucket->GetNumberOfEntries()];
   double * vetVariance = new double [fullBucket->GetNumberOfEntries()];
   stPartitionRegionDesc ** matReg = new stPartitionRegionDesc * [fullBucket->GetNumberOfEntries()];
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      matReg[i] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
   }//end for
   stPartitionRegionDesc ** matRegOrd = new stPartitionRegionDesc * [fullBucket->GetNumberOfEntries()];
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      matRegOrd[i] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
   }//end for

   // calculating matReg
   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      vetVariance[i] = 0;
      vetMedia[i] = 0;
      //adding candidate representative object
      canRep->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      RepObjects->AddCandidate(newReg.GetFirstDigit(), canRep);
      //Calculating the HashFunction for object i
      for (int j=0;j < fullBucket->GetNumberOfEntries();j++){
         insObj->Unserialize(fullBucket->GetObject(j), fullBucket->GetObjectSize(j));
         ObjectHashFunction(insObj, matReg[i][j]);
         //Orded Insertion
         aux=j;
         while ((aux>0) && (matReg[i][j].IsLessLargeBase(RepObjects, matRegOrd[i][aux-1]))){
            matRegOrd[i][aux]=matRegOrd[i][aux-1];
            aux--;
         }
         matRegOrd[i][aux] = matReg[i][j];
         //Sum item in media
         vetMedia[i] += matReg[i][j].GetRegion();
      }//end for
      //Calcule media
      vetMedia[i] = vetMedia[i] / fullBucket->GetNumberOfEntries();
      //Calculating the variance for object i
      for (int k=0;k < fullBucket->GetNumberOfEntries();k++){
         vetVariance[i] += pow((double)matReg[i][k].GetRegion()-vetMedia[i],2);
      }//end for
      vetVariance[i] = vetVariance[i] / (fullBucket->GetNumberOfEntries() - 1);
      if (vetVariance[i] > vetVariance[moreDistrib]){
         moreDistrib = i;
      }//end if
      //removing candidate representative object
      RepObjects->RemoveCandidate();
   }//end for

   // the best divider
   aux=1;
   // searching the descriptor region that best divide the bucket
   while ((divider-aux>=0) && (divider+aux < fullBucket->GetNumberOfEntries()) &&
      (matRegOrd[moreDistrib][divider].GetRegion()==matRegOrd[moreDistrib][divider-aux].GetRegion()) &&
      (matRegOrd[moreDistrib][divider].GetRegion()==matRegOrd[moreDistrib][divider+aux].GetRegion())){
      aux++;
   }//end while

   //Adding in global representative objects
   insObj->Unserialize(fullBucket->GetObject(moreDistrib), fullBucket->GetObjectSize(moreDistrib));
   RepObjects->Add(newReg.GetFirstDigit(), insObj);

   //updating region about promoted bucket
   //if promotion value is big than divider
   if((divider+aux < fullBucket->GetNumberOfEntries()) &&
         (matRegOrd[moreDistrib][divider].GetRegion()!=matRegOrd[moreDistrib][divider+aux].GetRegion())){
      promoted.Region = matRegOrd[moreDistrib][divider+aux];
   }else{
      //if promotion value is less than divider
      promoted.Region = matRegOrd[moreDistrib][divider];
   }//end if
   promoted.PageID = promoteBucket->GetPageID();

   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      cout << endl;
      for (int j=0;j < fullBucket->GetNumberOfEntries();j++){
         cout << matRegOrd[i][j].GetRegion() << " | ";
      }//end for
      cout << "med=" << vetMedia[i] << " | var=" << vetVariance[i];
   }//end for
   cout << endl << endl;
   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      cout << matRegOrd[moreDistrib][i].GetRegion() << " | ";
   }//end for

   //adding in promoteBucket or splitBucket
   for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
      insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      // insering in prometed bucket or splited bucket
      if (matReg[moreDistrib][i].IsMoreEqualLargeBase(RepObjects, promoted.Region)){
         promoteBucket->AddEntry(RepObjects, matReg[moreDistrib][i], insObj->GetSerializedSize(), insObj->Serialize());
      }else{
         splitBucket->AddEntry(RepObjects, matReg[moreDistrib][i], insObj->GetSerializedSize(), insObj->Serialize());
      }//end if
   }//end for

   //Adding responsible for splited
   ObjectHashFunction(newObj, newReg);
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects,fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket
   splited.Region = fullBucket->GetRegionDesc(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
   delete canRep;
   delete [] vetMedia;
   delete [] vetVariance;
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      delete [] matReg[i];
   }//end for
   delete [] matReg;
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      delete [] matRegOrd[i];
   }//end for
   delete [] matRegOrd;
}//end stPartitionHash::SplitLeafChoiceOneRepByVariance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafChoiceOneRepByDiffer(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   ObjectType * canRep = new ObjectType();
   ObjectType * insObj = new ObjectType();
   int divider, moreDistrib, aux;
   int * vetDiff = new int [fullBucket->GetNumberOfEntries()];
   stPartitionRegionDesc ** matReg = new stPartitionRegionDesc * [fullBucket->GetNumberOfEntries()];
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      matReg[i] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
   }//end for
   stPartitionRegionDesc ** matRegOrd = new stPartitionRegionDesc * [fullBucket->GetNumberOfEntries()];
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      matRegOrd[i] = new stPartitionRegionDesc[fullBucket->GetNumberOfEntries()];
   }//end for

   moreDistrib=0;
   // calculating matReg
   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      vetDiff[i] = 0;
      //adding candidate representative object
      canRep->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      RepObjects->AddCandidate(newReg.GetFirstDigit(), canRep);
      //Calculating the HashFunction for object i
      for (int j=0;j < fullBucket->GetNumberOfEntries();j++){
         insObj->Unserialize(fullBucket->GetObject(j), fullBucket->GetObjectSize(j));
         ObjectHashFunction(insObj, matReg[i][j]);
         //Orded Insertion
         aux=j;
         while ((aux>0) && (matReg[i][j].IsLessLargeBase(RepObjects, matRegOrd[i][aux-1]))){
            matRegOrd[i][aux]=matRegOrd[i][aux-1];
            aux--;
         }
         matRegOrd[i][aux] = matReg[i][j];
         // if exist region descriptor
         if ((aux==0) || ((aux > 0) && (matRegOrd[i][aux].GetRegion() != matRegOrd[i][aux-1].GetRegion()))){
            // increment Count
            vetDiff[i]++;
         }
      }//end for
      if (vetDiff[i] > vetDiff[moreDistrib]){
         moreDistrib = i;
      }//end if
      //removing candidate representative object
      RepObjects->RemoveCandidate();
   }//end for

   // the best divider
   divider=(int)(fullBucket->GetNumberOfEntries()/2);
   aux=1;
   // searching the descriptor region that best divide the bucket
   while ((divider-aux>=0) && (divider+aux < fullBucket->GetNumberOfEntries()) &&
      (matRegOrd[moreDistrib][divider].GetRegion()==matRegOrd[moreDistrib][divider-aux].GetRegion()) &&
      (matRegOrd[moreDistrib][divider].GetRegion()==matRegOrd[moreDistrib][divider+aux].GetRegion())){
      aux++;
   }//end while

   //Adding in global representative objects
   insObj->Unserialize(fullBucket->GetObject(moreDistrib), fullBucket->GetObjectSize(moreDistrib));
   RepObjects->Add(newReg.GetFirstDigit(), insObj);

   cout << "first=" << newReg.GetFirstDigit()+1 << " | ";

   //updating region about promoted bucket
   //if promotion value is big than divider
   if((divider+aux < fullBucket->GetNumberOfEntries()) &&
         (matRegOrd[moreDistrib][divider].GetRegion()!=matRegOrd[moreDistrib][divider+aux].GetRegion())){
      promoted.Region = matRegOrd[moreDistrib][divider+aux];
   }else{
      //if promotion value is less than divider
      promoted.Region = matRegOrd[moreDistrib][divider];
   }//end if
   promoted.PageID = promoteBucket->GetPageID();

   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      cout << endl;
      for (int j=0;j < fullBucket->GetNumberOfEntries();j++){
         cout << matRegOrd[i][j].GetRegion() << " | ";
      }//end for
      cout << "diff=" << vetDiff[i];
   }//end for
   cout << endl << endl;
   for (int i=0;i < fullBucket->GetNumberOfEntries();i++){
      cout << matRegOrd[moreDistrib][i].GetRegion() << " | ";
   }//end for

   //adding in promoteBucket or splitBucket
   for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
      insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      // insering in prometed bucket or splited bucket
      if (matReg[moreDistrib][i].IsMoreEqualLargeBase(RepObjects, promoted.Region)){
         promoteBucket->AddEntry(RepObjects, matReg[moreDistrib][i], insObj->GetSerializedSize(), insObj->Serialize());
      }else{
         splitBucket->AddEntry(RepObjects, matReg[moreDistrib][i], insObj->GetSerializedSize(), insObj->Serialize());
      }//end if
   }//end for

   //Adding responsible for splited
   ObjectHashFunction(newObj, newReg);
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects,fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket
   splited.Region = fullBucket->GetRegionDesc(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
   delete canRep;
   delete [] vetDiff;
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      delete [] matReg[i];
   }//end for
   delete [] matReg;
   for(int i=0;i<fullBucket->GetNumberOfEntries();i++){
      delete [] matRegOrd[i];
   }//end for
   delete [] matRegOrd;
}//end stPartitionHash::SplitLeafChoiceOneRepByDiffer

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::SplitLeafWithoutChoiceRep(
      stPartitionLeafBucket * fullBucket, stPartitionLeafBucket * promoteBucket,
      stPartitionRegionDesc newReg,  ObjectType * newObj,
      stPartitionSubtreeInfo & splited,  stPartitionSubtreeInfo & promoted){

   stPage * splitPage = NewPage(stPartitionNode::LEAF);
   stPartitionLeafBucket * splitBucket = new stPartitionLeafBucket(splitPage, true);
   ObjectType * insObj = new ObjectType();
   stPartitionRegionDesc insReg;

   // the best divider
   int divider=(int)(fullBucket->GetNumberOfEntries()/2);
   int aux=1;
   // searching the descriptor region that best divide the bucket
   while ((divider-aux>=0) &&
      (divider+aux < fullBucket->GetNumberOfEntries()) &&
      (fullBucket->GetRegionDesc(divider).IsEqualLargeBase(RepObjects,
      fullBucket->GetRegionDesc(divider-aux))) &&
      (fullBucket->GetRegionDesc(divider).IsEqualLargeBase(RepObjects,
      fullBucket->GetRegionDesc(divider+aux)))){
      aux++;
   }//end while

   //updating region about promoted bucket
   //if promotion value is big than divider
   if((divider+aux < fullBucket->GetNumberOfEntries()) &&
         (fullBucket->GetRegionDesc(divider).IsDifferentLargeBase(RepObjects,
         fullBucket->GetRegionDesc(divider+aux)))){
      promoted.Region = fullBucket->GetRegionDesc(divider+aux);
   }else{
      //if promotion value is less than divider
      promoted.Region = fullBucket->GetRegionDesc(divider);
   }//end if
   promoted.PageID = promoteBucket->GetPageID();

   //adding in promoteBucket or splitBucket
   for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){

      insObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      insReg = fullBucket->GetRegionDesc(i);

      // insering in prometed bucket or splited bucket
      if (insReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
         promoteBucket->AddEntry(RepObjects, insReg, insObj->GetSerializedSize(), insObj->Serialize());
      }else{
         splitBucket->AddEntry(RepObjects, insReg, insObj->GetSerializedSize(), insObj->Serialize());
      }//end if
   }//end for

   //Adding responsible for splited
   if (newReg.IsMoreEqualLargeBase(RepObjects, promoted.Region)){
      promoteBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }else{
      splitBucket->AddEntry(RepObjects, newReg, newObj->GetSerializedSize(), newObj->Serialize());
   }//end if

   //update next bucket of new leaf
   promoteBucket->SetNextBucket(fullBucket->GetNextBucket());
   //copying splitBuck into leafBucket
   splitBucket->CopyFromBucket(RepObjects, fullBucket);
   //update next bucket splited leaf bucket
   fullBucket->SetNextBucket(promoteBucket->GetPageID());

   // updating region about splited bucket
   splited.Region = fullBucket->GetRegionDesc(0);
   splited.PageID = fullBucket->GetPageID();

   //clean home
   DisposePage(splitPage, stPartitionNode::LEAF);
   delete splitBucket;
   delete insObj;
}//end stPartitionHash::SplitLeafWithoutChoiceRep

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::UpdateRegionLeaf(stPartitionLeafBucket * fullBucket){
   ObjectType * updObj = new ObjectType();
   stPartitionRegionDesc updReg;
   //updating fullBucket
   for (int i = 0; i < fullBucket->GetNumberOfEntries(); i++){
      updObj->Unserialize(fullBucket->GetObject(i), fullBucket->GetObjectSize(i));
      updReg = fullBucket->GetRegionDesc(i);
      if (updReg.GetNumberRep() < GetGlobalNumberRep()){
         ObjectHashFunction(updObj, updReg);
      }//end if
      fullBucket->SetRegionDesc(RepObjects, i, updReg);
   }//end for
}//end stPartitionHash::UpdateRegionLeaf

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stPartitionRegionDesc tmpl_stPartitionHash::GetBeginRegion(){
   stPartitionRegionDesc reg(GetGlobalNumberRep(), 0);
   return reg;
}//end stPartitionHash::GetBeginRegin

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stPartitionRegionDesc tmpl_stPartitionHash::GetEndRegion(){
   if (GetGlobalNumberRep()>0){
      stPartitionRegionDesc reg(GetGlobalNumberRep(),
         ((u_int32_t long)(pow(GetGlobalNumberRep(), (long double)GetGlobalNumberRep())-1)));
      return reg;
   }else{
      stPartitionRegionDesc reg(0, 0);
      return reg;
   }//end if
}//end stPartitionHash::GetEndRegion

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::ObjectHashFunction(ObjectType * obj,
      stPartitionRegionDesc & reg){
   if (GetGlobalNumberRep() > 0){
      int j;
      unsigned char pos;
      unsigned char * base = new unsigned char[GetGlobalNumberRep()];
      double distAxis;
      u_int32_t long calcreg = 0;

      //for each representative object (each axis)
      for (int i = 0; i < GetGlobalNumberRep(); i++){
         j = pos = 0;
         distAxis = myMetricEvaluator->GetDistance(obj, RepObjects->GetRepObject(i));
         //what is its region?
         while ((j < GetGlobalNumberRep()) && ((i == j) ||
                !((distAxis < RepObjects->GetAxisDistance(i, j)) &&
                (distAxis >= RepObjects->GetAxisDistanceLowerBound(i, j))))){
            if (i != j){
               //increment coordenate
               pos++;
            }//end if
            j++;
         }//end while
         base[i] = pos;
      }//end for
      //transform in decimal
      for(int k = 0; k < GetGlobalNumberRep(); k++){
         calcreg += (u_int32_t long)(base[k] * pow(GetGlobalNumberRep(), (long double)GetGlobalNumberRep() - k - 1));
      }//end for

      reg.SetNumberRepresentatives(GetGlobalNumberRep());
      reg.SetRegion(calcreg);
      delete [] base;
   }else{
      reg.SetNumberRepresentatives(0);
      reg.SetRegion(0);
   }//end if
}//end stPartitionHash::ObjectHashFunction

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::RadiusHashFunction (ObjectType * obj,
      double radius, stPartitionListRegionDesc & list){
   if (GetGlobalNumberRep() > 0){

      unsigned char pos;
      double distAxis;
      //vetor of sequencial list (list is small)
      unsigned char ** resultAxis = new unsigned char*[GetGlobalNumberRep()];
      for(int i=0;i<GetGlobalNumberRep();i++){
         resultAxis[i] = new unsigned char[GetGlobalNumberRep()];
      }//end for
      unsigned char * countResultAxis = new unsigned char[GetGlobalNumberRep()];

      //what were qualified coordinates?
      for (int i = 0; i < GetGlobalNumberRep(); i++){
         //initiation
         countResultAxis[i] = 0;
         pos = 0;
         distAxis = myMetricEvaluator->GetDistance(obj, RepObjects->GetRepObject(i));
         for (int j = 0; j < GetGlobalNumberRep(); j++){
            if (i != j){
               //adding coordenate
               if ((( distAxis - radius < RepObjects->GetAxisDistance(i, j)) &&
                     (distAxis - radius >= RepObjects->GetAxisDistanceLowerBound(i, j))) ||
                    ((distAxis + radius > RepObjects->GetAxisDistance(i, j)) &&
                     (distAxis - radius <= RepObjects->GetAxisDistanceLowerBound(i, j))) ||
                    ((distAxis + radius < RepObjects->GetAxisDistance(i, j)) &&
                     (distAxis + radius >= RepObjects->GetAxisDistanceLowerBound(i, j)))){
                  resultAxis[i][countResultAxis[i]] = pos;
                  countResultAxis[i]++;
               }//end if
               //increment coordenate
               pos++;
            }//end if
         }//end for
         //if upper bound is less the disntance of object far
         if (distAxis + radius >= RepObjects->GetAxisDistanceMax(i)){
            resultAxis[i][countResultAxis[i]] = pos;
            countResultAxis[i]++;
         }//end if
      }//end for

      for (int i = 0; i < GetGlobalNumberRep(); i++){
         cout << "DistAxis [" << i << "]= ";
         for (int j = 0; j < countResultAxis[i]; j++){
            cout << (int) resultAxis[i][j] << " | ";
         }//end for
         cout << endl;
      }//end for

      //genering the region descriptor
      GeneratorRegionDesc(countResultAxis, resultAxis, list);

      delete [] countResultAxis;
      for (int i=0;i<GetGlobalNumberRep();i++){
         delete [] resultAxis[i];
      }//end for
      delete [] resultAxis;
   }//end if
}//end stPartitionHash::RadiusHashFunction

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stPartitionHash::GeneratorRegionDesc(unsigned char * countCoordAxis,
      unsigned char ** CoordAxis, stPartitionListRegionDesc & list){

   unsigned char * base = new unsigned char[GetGlobalNumberRep()];
   u_int32_t long calcreg;
   stPartitionRegionDesc reg;
   unsigned char * vectPosAxis = new unsigned char[GetGlobalNumberRep()];
   //generating list of Desciptor Region
   //initiating vector of position
   for (int i = 0; i < GetGlobalNumberRep(); i++){
      vectPosAxis[i]=0;
   }//end for

   int i , j, prev;
   i = GetGlobalNumberRep()-1;
   while (i >= 0) {
      //reset vector of position
      for(int k=i+1; k < GetGlobalNumberRep(); k++){
         vectPosAxis[k]=0;
      }//end for
      while (vectPosAxis[i] < countCoordAxis[i]) {
         //reset vector of position
         for(int k=i+1; k < GetGlobalNumberRep(); k++){
            vectPosAxis[k]=0;
         }//end for

         j = GetGlobalNumberRep()-1;
         do {
            //reset vector of position
            for(int k=j+1; k < GetGlobalNumberRep(); k++){
               vectPosAxis[k]=0;
            }//end for
            //if vector of position is more than counter
            while (vectPosAxis[j] < countCoordAxis[j]) {
               //creating descriptor region
               cout << "base: ";
               for(int l=0; l < GetGlobalNumberRep(); l++){
                  base[l]=CoordAxis[l][vectPosAxis[l]];
                  cout << (int)base[l] << " ";
               }//end for
               //transform in decimal
               calcreg = 0;
               for(int k = 0; k < GetGlobalNumberRep(); k++){
                  calcreg += (u_int32_t long)(base[k] * pow(GetGlobalNumberRep(), (long double)GetGlobalNumberRep() - k - 1));
               }//end for
               reg.SetNumberRepresentatives(GetGlobalNumberRep());
               reg.SetRegion(calcreg);
               //adding in the list of region descriptors
               list.Add(reg);
               cout << " [" << (int)reg.GetNumberRep() << "] " << reg.GetRegion() << " | " << endl;
               //incrementing vector of position in j
               vectPosAxis[j]++;
            }//end while
            //decrement
            j--;
            if (j > i){
               //incrementing vector of position in j
               vectPosAxis[j]++;
            }//end if
         } while (j > i);
         //incrementing vector of position in j
         vectPosAxis[i]++;
      }//end while
      //decrement
      i--;
      if (i >= 0) {
         //incrementing vector of position in j
         vectPosAxis[i]++;
      }//end if
   }//end if

   //clean home
   delete [] vectPosAxis;
   delete [] base;

}//end stPartitionHash::GeneratorRegionDesc

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stPartitionHash::RangeQuery(
      ObjectType * sample, double range){

}//end stPartitionHash::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stPartitionHash::PointQuery(ObjectType * sample){

   stPartitionNode * curBucket;
   stPage * curPage;
   stPartitionIndexBucket *indexBucket;
   stPartitionLeafBucket *leafBucket;
   stPartitionRegionDesc regSample;
   stPartitionRegionDesc regFind;
   stPartitionRegionDesc begin = GetBeginRegion();
   stPartitionRegionDesc end = GetEndRegion();
   int idxSub, idxFind;
   tResult * result = new tResult();  // Create result
   ObjectType * resultObj = new ObjectType();
   double distance;

   std::string * cid;

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), tResult::POINTQUERY);

   //applying Hash Function
   ObjectHashFunction(sample, regSample);

   //if not exits root
   if (GetRoot()!=0){
      curPage = myPageManager->GetPage(GetRoot());
      curBucket = stPartitionNode::CreateBucket(curPage);
      while (curBucket->GetBucketType() == stPartitionNode::INDEX){
         //converting bucket in indexcbucket
         indexBucket = (stPartitionIndexBucket *)curBucket;
         //choice sub-bucket
         indexBucket->ChoiceSubBucket(RepObjects, regSample, idxSub, begin, end);

         cout << "new bucket occup=" << indexBucket->GetNumberOfEntries() << endl;
         stPartitionRegionDesc test;
         for (int i=0; i < indexBucket->GetNumberOfEntries(); i++){
            test = indexBucket->GetRegionDesc(i);
            test.ChangeNumberRep(RepObjects, RepObjects->GetNumberRep());
            cout << "[" << (int)test.GetNumberRep() << "] = " << test.GetRegion() << endl;
         }

         //Clean home
         myPageManager->ReleasePage(curPage);
         //restrive bucket
         curPage = myPageManager->GetPage(indexBucket->GetEntryPageId(idxSub));
         delete curBucket;
         curBucket = stPartitionNode::CreateBucket(curPage);
      }//end while
      //converting bucket in leafbucket
      leafBucket = (stPartitionLeafBucket *)curBucket;

      //seach desciptor region
      idxFind = 0;
      regFind = leafBucket->GetRegionDesc(idxFind);

      // while
      while (idxFind < leafBucket->GetNumberOfEntries()){
         //regFind.ChangeNumberRep(RepObjects, RepObjects->GetNumberRep());
         resultObj->Unserialize(leafBucket->GetObject(idxFind), leafBucket->GetObjectSize(idxFind));

         regFind.ChangeNumberRep(RepObjects, GetGlobalNumberRep());

         cid = (std::string*)resultObj;
         cout << "[" << (int)regFind.GetNumberRep() << "] " << regFind.GetRegion();
         cout << " " << cid->c_str() << endl;
         idxFind++;
         regFind = leafBucket->GetRegionDesc(idxFind);
      }//end while

/*

      cout << "numEntries=" << leafBucket->GetNumberOfEntries() << endl;
      cout << "first[" << (int)regFind.GetNumberRep() << "] " << regFind.GetRegion() << endl;

      // while
      while ((idxFind < leafBucket->GetNumberOfEntries()) && (regFind.IsLessMinorBase(RepObjects, regSample))){
         idxFind++;
         regFind = leafBucket->GetRegionDesc(idxFind);

         cout << "isLess[" << (int)regFind.GetNumberRep() << "] " << regFind.GetRegion() << endl;

      }//end while
      //if exists the region
      while ((idxFind < leafBucket->GetNumberOfEntries()) && regFind.IsEqualMinorBase(RepObjects, regSample)){

         // Rebuild the object
         resultObj->Unserialize(leafBucket->GetObject(idxFind), leafBucket->GetObjectSize(idxFind));
         // When this entry is a representative, it does not need to evaluate
         // a distance, because distanceRepres is iqual to distance.
         // Evaluate distance.

         cid = (std::string*)resultObj;
         cout << "isEqual[" << (int)regFind.GetNumberRep() << "] " << regFind.GetRegion();
         cout << "cid= " << cid->c_str() << endl;

         distance = myMetricEvaluator->GetDistance(resultObj, sample);
         //test if the object qualify
         if (distance == 0){
            cout << "added" << endl;
            // Add the object.
            result->AddPair(resultObj->Clone(), distance);
         }//end if
         //next region and object
         idxFind++;
         regFind = leafBucket->GetRegionDesc(idxFind);
      }//end while

      //teste
      resultObj->Unserialize(leafBucket->GetObject(idxFind), leafBucket->GetObjectSize(idxFind));
      cid = (std::string*)resultObj;
      cout << "isEqual[" << (int)regFind.GetNumberRep() << "] " << regFind.GetRegion();
      cout << "cid= " << cid->c_str() << endl;
*/
      //Clean home
      myPageManager->ReleasePage(curPage);
      delete curBucket;

   }//end if

   delete resultObj;

   return result;
}//end stPartitionHash::PointQuery
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stPartitionHash::SequencialPointQuery(
      ObjectType * sample){

   stPage * curPage;
   stPartitionNode * curBucket;
   stPartitionLeafBucket *leafBucket;
   tResult * result = new tResult();  // Create result
   ObjectType * resultObj = new ObjectType();
   u_int32_t actualBucket=GetRootLeaf();;
   double distance;
   stPartitionRegionDesc reg;
   std::string * cid;


   // Set information for this query
   result->SetQueryInfo(sample->Clone(), tResult::POINTQUERY);

   //if not exits root
   if (GetRootLeaf()!=0){
      int cont = 0;
      //while not is last bucket
      while (actualBucket != 0){
         curPage = myPageManager->GetPage(actualBucket);
         curBucket = stPartitionNode::CreateBucket(curPage);
         //converting bucket in indexcbucket
         leafBucket = (stPartitionLeafBucket *)curBucket;
         for (int i=0; i < leafBucket->GetNumberOfEntries(); i++){
            resultObj->Unserialize(leafBucket->GetObject(i), leafBucket->GetObjectSize(i));
            distance = myMetricEvaluator->GetDistance(resultObj, sample);
            //test if the object qualify
            if (distance == 0){
               result->AddPair(resultObj->Clone(), distance);

               for (int j=0; j < leafBucket->GetNumberOfEntries(); j++){
                  resultObj->Unserialize(leafBucket->GetObject(j), leafBucket->GetObjectSize(j));
                  reg = leafBucket->GetRegionDesc(j);
                  reg.ChangeNumberRep(RepObjects, GetGlobalNumberRep());
                  cout << "[" << (int)reg.GetNumberRep() << "] " << reg.GetRegion();
                  cid = (std::string*)resultObj;
                  cout << " cid= " << cid->c_str() << endl;
                  // Add the object.
               }//end for
            }//end if
         }//end for
         //next bucket
         actualBucket = leafBucket->GetNextBucket();
         //Clean home
         myPageManager->ReleasePage(curPage);
         delete curBucket;

         cont++;
         cout << "bucket[" << cont << "] | ";

      }//end while
   }//end if
   delete resultObj;

   return result;

}//end stPartitionHash::SequencialPointQuery
//------------------------------------------------------------------------------
