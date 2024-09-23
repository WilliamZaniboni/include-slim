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
* This file defines the class stVPTree.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/


#ifndef __STVPTREE_H
#define __STVPTREE_H

#include <math.h>
#include <algorithm>
#include <arboretum/stCommon.h>
#include <arboretum/stMetricTree.h>
#include <arboretum/stVPNode.h>

/**
* This class template implements a VP-tree. The VP-tree has the same
* external interface and behavior of a Metric Tree.
*
* <P>It can perform both range and k-nearest neighbour queries, and so its
* combinations and variations.
*
* This is a binary static memory metric tree
*
* <P>This class was developed to generate perfect answers to queries. It
* allows the build of automated test programs for other metric trees
* implemented by this library.
*
* @author Ives RenÃª Venturini Pola (ives@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation review.
* @version 1.0
* @ingroup VP
*/
template <class ObjectType, class EvaluatorType>
class stVPTree: public stMetricTree<ObjectType, EvaluatorType>{

   public:

      /**
      * This type defines the header of the VPTree.
      */
      typedef struct VPTreeHeader{
         /**
         * Magic number. This is a short string that must contains the magic
         * string "VP-3". It may be used to validate the file (this feature
         * is not implemented yet).
         */
         char Magic[4];
      
         /**
         * The root.
         */
         u_int32_t Root;

         /**
         * The height of the tree
         */
         u_int32_t Height;

         /**
         * Total number of objects.
         */
         u_int32_t ObjectCount;

         /**
         * The number of the nodes
         */
         u_int32_t NodeCount;
      } stVPTreeHeader;

      /**
      * This is the class that abstracts the object.
      */
      typedef ObjectType tObject;

      /**
      * This is the class that abstracts the metric evaluator.
      */
      typedef EvaluatorType tMetricEvaluator;

      /**
      * This is the class that abstracts an result set.
      */
      typedef stResult <ObjectType> tResult;

      /**
      * Creates a new instance of the VPTree.
      *
      * @param pageman The Page Manager to be used.
      */
      stVPTree(stPageManager * pageman);

      /**
      * Disposes this tree and releases all associated resources.
      */
      virtual ~stVPTree();

      /**
      * This method adds an object to the VPTree.
      * This method may fail it he object size exceeds the page size.
      *
      * @param obj The object to be added.
      * @return True for success or false otherwise.
      */
      bool Add(tObject ** objects, long listSize);

      /**
      * This method implements the Add method to insert object one-by-one in
      * the VP-Tree.
      *
      * @return success for the VPTree build.
      */
      bool Add(tObject * obj);

      /**
      * This method build the VP-tree after some Add calls.
      *
      * @return success for the VPTree build.
      */
      bool MakeVPTree();

      #ifdef __stDEBUG__
         /**
         * Get root page id.
         *
         * @warning This method is public only if __stDEBUG__ is defined at compile
         * time.
         */
         u_int32_t GetRoot(){
            return this->Header->Root;
         }//end GetRoot
      #endif //__stDEBUG__

      /**
      * Returns the height of the tree.
      */
      virtual u_int32_t GetHeight(){
         return Header->Height;
      }//end GetHeight

      /**
      * Returns the number of objetcs of this tree.
      */
      virtual long GetNumberOfObjects(){
         return Header->ObjectCount;
      }//end GetNumberOfObjects

      /**
      * This method will perform a range query. The result will be a set of
      * pairs object/distance.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * RangeQuery(tObject * sample, double range);

      /**
      * This method will perform a k nearest neighbor query.
      *
      * @param sample The sample object.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * NearestQuery(tObject * sample, u_int32_t k, bool tie = false);

   private:

      /**
      * The header page. It will be kept in memory all the time to avoid
      * reads.
      */
      stPage * HeaderPage;

      /**
      * The header of the "tree".
      */
      stVPTreeHeader * Header;

      /**
      * If true, the header mus be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * To add object one-by-one.
      */
      tObject ** Objects;

      /**
      * The Used Size.
      */
      long Size;

      /**
      * The Maximum Capacity.
      */
      long Capacity;

		/**
		* Increment.
		*/
      long Increment;

      #ifndef __stDEBUG__
         /**
         * Get root page id.
         */
         u_int32_t GetRoot(){
            return this->Header->Root;
         }//end GetRoot
      #endif // __stDEBUG__

      /**
      * Sets a new root.
      */
      void SetRoot(u_int32_t root){
         this->Header->Root = root;
         this->HeaderUpdate = true;
      }//end SetRoot

      /**
      * Loads the header from the page manager.
      */
      void LoadHeader();

      /**
      * Sets all header's fields to default values.
      *
      * @warning This method will destroy the tree.
      */
      void DefaultHeader();

      /**
      * Updates the header in the file if required.
      */
      void WriteHeader(){
         if (this->HeaderUpdate){
            this->myPageManager->WriteHeaderPage(HeaderPage);
            this->HeaderUpdate = false;
         }//end if
      }//end WriteHeader

      /**
      * Disposes the header page if it exists. It also updates its contents
      * before destroy it.
      *
      * <P>This method is called by the destructor.
      */
      void FlushHeader(){
         if (HeaderPage != NULL){
            if (Header != NULL){
               this->WriteHeader();
            }//end if
            this->myPageManager->ReleasePage(HeaderPage);
         }//end if
      }//end FlushHeader
      
      /**
      * Creates a new empty page and updates the node counter.
      */
      stPage * NewPage(){
         this->Header->NodeCount++;
         return this->myPageManager->GetNewPage();
         this->HeaderUpdate = true;
      }//end NewPage

      /**
      * Disposes a given page and updates the page counter.
      */
      void DisposePage(stPage * page){
         Header->NodeCount--;
         this->myPageManager->DisposePage(page);
         this->HeaderUpdate = true;
      }//end DisposePage

      /**
      * This method recursively build the VP-tree based on the object list
      *
      * @return success for the VPTree build.
      */
      u_int32_t MakeVPTree(tObject ** objects, doubleIndex * & selected,
                          long size);

      /**
      * Used by MakeVPTree, selects a vantage point from a dataset from the
      * corner of the space.
      */
      long SelectVP(tObject ** objects, doubleIndex * selected, long size);

      /**
      *  This method calculates the median element, sorting the "sample" using
      *  the "vantage" as vantage point (index of the object in "obj".
      */
      long GetMedian(tObject ** objects, doubleIndex * sample, long sampleSize,
                    long vantage);

      /**
      *  Calculates the 2nd=Momento of the sample
      */
      double GetSecondMoment(doubleIndex * sample, long sampleSize, long median);

      /**
      *  This method calculate the sample size given the actual size
      */
      long PickSampleSize(long size){
         long sampleSize;

         // Try to get 10%
         sampleSize = (long) ceil(0.09 * (double )size);
         if (sampleSize < 50){
            sampleSize = 50;
            if (size < sampleSize){
               sampleSize = size;
            }//end if
         }//end if
         if (sampleSize > 500){
            sampleSize = 500;
            if (size < sampleSize){
               sampleSize = size;
            }//end if
         }//end if
         return sampleSize;
      }//end PickSampleSize

      /**
      *  Used to make a sample of the vector "selected"
      */
      void MakeSample(doubleIndex * sample, long sampleSize,
                      doubleIndex * selected, long size);

		/**
		* Expands the capacity of the object vector by adding increment
		* entries to the current capacity.
		*/
      void Resize();
      
      /**
      * Support for the RangeQuery, recursive code for searching.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param result The result object to add the pairs <object, distance> found
      * @param page The page (node) to search in.
      */
      void RangeQuery(tObject * sample, double range,
                      tResult * result, u_int32_t page);

      /**
      * Support for the NearestQuery, recursive code for searching.
      * @param sample The sample object.
      * @param k The number of nearest neighbors to retrieve.
      * @param result The result object to add the pairs <object, distance> found
      * @param page The page (node) to search in.
      */
      void NearestQuery(tObject * sample, u_int32_t k, tResult * result,
                        u_int32_t page);

};//end stVPTree

/* Copyright 2003-2017 GBDI-ICMC-USP <caetano@icmc.usp.br>
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*   http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file
*
* This file is the implementation of stVPTree methods.
*
* @author Ives Renê Venturini Pola (ives@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/                     
                                                                       
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stVPTree<ObjectType, EvaluatorType>::stVPTree(stPageManager * pageman):
stMetricTree<ObjectType, EvaluatorType>(pageman){
   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;
   // Load header.
   LoadHeader();
   Size = 0;
   Capacity = 500;
   Increment = 500;
   Objects = new tObject * [500];

   // Will I create or load the tree ?
   if (this->myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if
}//end stVPTree<ObjectType><EvaluatorType>::stVPTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stVPTree<ObjectType, EvaluatorType>::~stVPTree(){
   long i;
   for (i = 0; i < Size; i++){
      delete Objects[i];
   }//end for
   delete[] Objects;
   // Flus header page.
   FlushHeader();
}//end ~stVPTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stVPTree<ObjectType, EvaluatorType>::LoadHeader(){
   if (HeaderPage != NULL){
      this->myPageManager->ReleasePage(HeaderPage);
   }//end if

   // Load and set the header.
   HeaderPage = this->myPageManager->GetHeaderPage();
   if (HeaderPage->GetPageSize() < sizeof(stVPTreeHeader)){
      #ifdef __stDEBUG__
         throw std::logic_error("The page size is too small. Increase it!\n");
      #endif //__stDEBUG__
   }//end if

   this->Header = (stVPTreeHeader *) HeaderPage->GetData();
   HeaderUpdate = false;
}//end stVPTree<ObjectType, EvaluatorType>::LoadHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stVPTree<ObjectType, EvaluatorType>::DefaultHeader(){
   // Clear header page.
   HeaderPage->Clear();
   // Default values
   Header->Magic[0] = 'V';
   Header->Magic[1] = 'P';
   Header->Magic[2] = '-';
   Header->Magic[3] = '3';
   Header->Root = 0;
   Header->Height = 0;
   Header->ObjectCount = 0;
   Header->NodeCount = 0;
   // Notify modifications
   HeaderUpdate = true;
}//end stVPTree<ObjectType, EvaluatorType>::DefaultHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stVPTree<ObjectType, EvaluatorType>::Add(tObject ** objects, long listSize){
   long i;
   doubleIndex * selected;

   // First test for wrong values.
   if (listSize > 0){
      // Make the initial set, select every object
      selected = new doubleIndex[listSize];
      // Init map.
      for (i = 0; i < listSize; i++){
         selected[i].Index = i;
         selected[i].Distance = 0.0;
      }//end for
      // Build the tree and set the root node.
      this->SetRoot(MakeVPTree(objects, selected, listSize));
      // Update the header.
      this->Header->ObjectCount = listSize;
      // Write Header!
      WriteHeader();

      // Clean.
      delete[] selected;
      // return sucess!
      return true;
   }else{
      // It is not possible to insert negative or zero values.
      return false;
   }//end if
}//end stVPTree<ObjectType><EvaluatorType>::Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stVPTree<ObjectType, EvaluatorType>::Add(tObject * obj){
   // Resize me if required.
   if (Size == Capacity){
      Resize();
   }//end if
   Objects[Size] = obj->Clone();
   // Update the size of Objects.
   Size++;
   return true;
}//end Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stVPTree<ObjectType, EvaluatorType>::MakeVPTree(){
   long i;
   doubleIndex * selected;

   // First test for wrong values.
   if (Size > 0){
      // Make the initial set, select every object
      selected = new doubleIndex[Size];
      // Init map.
      for (i = 0; i < Size; i++){
         selected[i].Index = i;
         selected[i].Distance = 0.0;
      }//end for
      // Build the tree and set the root node.
      this->SetRoot(MakeVPTree(Objects, selected, Size));
      // Update the header.
      this->Header->ObjectCount = Size;
      // Write Header!
      WriteHeader();

      // Clean.
      delete[] selected;
      // return sucess!
      return true;
   }else{
      // It is not possible to insert negative or zero values.
      return false;
   }//end if
}//end MakeVPTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stVPTree<ObjectType, EvaluatorType>::MakeVPTree(tObject ** objects,
                               doubleIndex * & selected, long size){
   stPage * currPage;
   stPage * newPage;
   stVPNode * currNode;
   stVPNode * newNode;
   doubleIndex * left, * right;
   long i, w;
   long vp, median;

   currPage = this->NewPage();
   currNode = new stVPNode(currPage, true);
   //left = NULL;
   //right = NULL;

   if (size > 2){
      //SelectVP must return the index on obj list, directly
      vp = SelectVP(objects, selected, size);

      // Add the selected object.
      currNode->AddEntry(objects[vp]->GetSerializedSize(),
                         objects[vp]->Serialize());
      // median is the median index on selected list, this method
      // must sort the selected to get it....
      median = GetMedian(objects, selected, size, vp);
      currNode->SetRadius(selected[median].Distance);

      // Build the left and right object lists
      if (median > 0){
         left = new doubleIndex[median];

         for (i = 0; i < median; i++){
            left[i] = selected[i+1];
         }//end for
         // Make the left subtree.
         currNode->SetLeftPageID(MakeVPTree(objects, left, median));
         // Clean.
         delete[] left;
      }//end if
      if ((size - median - 1) > 0){
         right = new doubleIndex[size - median - 1];
         for (i = median + 1; i < size; i++){
            right[i-median-1] = selected[i];
         }//end for
         // Make the right subtree.
         currNode->SetRightPageID(MakeVPTree(objects, right, size - median - 1));
         // Clean.
         delete[] right;
      }//end if

      // update the height.
      Header->Height++;
      // Write node.
      this->myPageManager->WritePage(currPage);
      this->myPageManager->ReleasePage(currPage);
      // Clean.
      delete currNode;
   }else{
      if (size == 2){
         // Add the entry.
         currNode->AddEntry(objects[selected[0].Index]->GetSerializedSize(),
                            objects[selected[0].Index]->Serialize());
         // Set the radius.
         currNode->SetRadius(this->myMetricEvaluator->GetDistance(*objects[selected[0].Index],
                                                                  *objects[selected[1].Index]));

         // Alocate a new page.
         newPage = this->NewPage();
         newNode = new stVPNode(newPage, true);
         // Add the other entry.
         newNode->AddEntry(objects[selected[1].Index]->GetSerializedSize(),
                           objects[selected[1].Index]->Serialize());
         // Link the two nodes.
         currNode->SetLeftPageID(newPage->GetPageID());
         // Set the right children.
         currNode->SetRightPageID(0);
         // update the height.
         Header->Height++;

         // Write node.
         this->myPageManager->WritePage(newPage);
         this->myPageManager->WritePage(currPage);
         // Clean.
         delete currNode;
         delete newNode;
         this->myPageManager->ReleasePage(newPage);
         this->myPageManager->ReleasePage(currPage);
      }else{
         // There is only one entry.
         // Add the entry.
         currNode->AddEntry(objects[selected[0].Index]->GetSerializedSize(),
                            objects[selected[0].Index]->Serialize());
         // Set the children.
         currNode->SetLeftPageID(0);
         currNode->SetRightPageID(0);

         // Write node.
         this->myPageManager->WritePage(currPage);
         this->myPageManager->ReleasePage(currPage);
         // Clean.
         delete currNode;
      }//end if
   }//end if

   // return the pageID of the created node.
   return currPage->GetPageID();
}//end stVPTree<ObjectType, EvaluatorType>::MakeVPTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long stVPTree<ObjectType, EvaluatorType>::SelectVP(tObject ** objects,
      doubleIndex * selected, long size){

   long sampleSize, median, bestVP;
   double spread, bestSpread;
   doubleIndex * sample1;
   doubleIndex * sample2;
   long i;

   // Init variables.
   bestSpread = -1;
   bestVP = 0;

   // Create a sample.
   sampleSize = PickSampleSize(size);
   sample1 = new doubleIndex[sampleSize];
   MakeSample(sample1, sampleSize, selected, size);

   for (i = 0; i < sampleSize; i++){
      // Create another sample.
      sample2 = new doubleIndex[sampleSize];
      MakeSample(sample2, sampleSize, selected, size);

      median = GetMedian(objects, sample2, sampleSize, sample1[i].Index);
      spread = GetSecondMoment(sample2, sampleSize, median);

      // Is this a better configuration?
      if (spread > bestSpread){
         // Yes, save the status.
         bestSpread = spread;
         bestVP = sample1[i].Index;
      }//end if
      // Clean.
      delete[] sample2;
   }//end for
   
   // Return the best vp.
   return bestVP;
}//end stVPTree<ObjectType, EvaluatorType>::SelectVP

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long stVPTree<ObjectType, EvaluatorType>::GetMedian(tObject ** objects,
      doubleIndex * sample, long sampleSize, long vantage){
   long i;
   bool stop;
   long median = 0;
   doubleIndex tmp;

   // Calculate the distances.
   for (i = 0; i < sampleSize; i++){
      sample[i].Distance =
         this->myMetricEvaluator->GetDistance(*objects[vantage], 
                                              *objects[sample[i].Index]);
   }//end for
   // Sort them.
   std::sort(sample, sample + sampleSize);

   // Get the median index.
   median = sampleSize / 2;
   while ((median + 1 < sampleSize) &&
          (sample[median + 1].Distance == sample[median].Distance)){
      median++;
   }//end while

   // Confirm the right location for vantage (first position).
   if (sample[0].Index != vantage){
      i = 1;
      stop = i >= sampleSize;
      while (!stop){
         if (sample[i].Index == vantage){
            tmp = sample[i];
            sample[i] = sample[0];
            sample[0] = tmp;
            // stop it!
            stop = true;
         }//end if
         i++;
         stop = stop || (i >= sampleSize);
      }//end while
   }//end if

   return median;
}//end stVPTree<ObjectType, EvaluatorType>::GetMedian

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double stVPTree<ObjectType, EvaluatorType>::GetSecondMoment(
         doubleIndex * sample, long sampleSize, long median){
   long i;
   double moment = 0;

   for (i = 0; i < sampleSize; i++){
      moment += (sample[i].Distance - sample[median].Distance) *
                (sample[i].Distance - sample[median].Distance);
   }//end for
   return moment;
}//end stVPTree<ObjectType, EvaluatorType>::GetSecondMoment

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stVPTree<ObjectType, EvaluatorType>::MakeSample(doubleIndex * sample,
      long sampleSize, doubleIndex * selected, long size){

   long increment, current, idxSample, chose;

   increment = (size / sampleSize) - 1; //zero position counts..
   current = 0;
   idxSample = 0;
   chose = 0;

   while ((idxSample <= (sampleSize-1)) && (chose < (size-1))){
      chose = (long )rngrand(current, current + increment);
      if (chose > (size-1)){
         chose = size-1;
      }//end if
      sample[idxSample++] = selected[chose];
      current += increment + 1;
   }//end while
}//end stVPTree<ObjectType, EvaluatorType>::MakeSample

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stVPTree<ObjectType, EvaluatorType>::Resize(){
   tObject ** newObjs;
   long i;

	// New entry vector
	newObjs = new tObject * [Capacity + Increment];
   // Copy the vector in a single step.
	memcpy(newObjs, Objects, Size * sizeof(tObject *));
	// Delete the old vector.
	delete[] Objects;
   // Ajust the pointers.
	Objects = newObjs;
   // Update the new value of capacity.
	Capacity += Increment;
}//end Resize

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stVPTree<ObjectType, EvaluatorType>::RangeQuery(
                              tObject * sample, double range){
   tResult * result = new tResult();  // Create result
   // Set the information.
   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   // Let's search
   if (this->GetRoot() != 0){
      // Calls the routine to search the nodes recursively.
      RangeQuery(sample, range, result, this->GetRoot());
   }//end if
   // Return the result set.
   return result;
}//end stMMTree<ObjectType><EvaluatorType>::RangeQuery
                                           
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stVPTree<ObjectType, EvaluatorType>::RangeQuery(tObject * sample,
                       double range, tResult * result, u_int32_t pageID){

   stPage * currPage;
   stVPNode * currNode;
   double distance;
   tObject tmpObj;

   // Let's search
   if (pageID != 0){
      currPage = this->myPageManager->GetPage(pageID);
      currNode = new stVPNode(currPage);

      // Rebuild the first object
      tmpObj.Unserialize(currNode->GetObject(), currNode->GetObjectSize());
      // Evaluate distance
      distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

      // is it a object that qualified?
      if (distance <= range){
         // Yes! Put it in the result set.
         result->AddPair(tmpObj.Clone(), distance);
      }//end if

      // Analize the subtrees.
      // The left subtree.
      if (distance - range <= currNode->GetRadius()){
         RangeQuery(sample, range, result, currNode->GetLeftPageID());
      }//end if
      // The right subtree.
      if (distance + range > currNode->GetRadius()){
         RangeQuery(sample, range, result, currNode->GetRightPageID());
      }//end if

      // Free it all.
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end stVPTree<ObjectType, EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stVPTree<ObjectType, EvaluatorType>::NearestQuery(
                     tObject * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   // Set the information.
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

   // Let's search
   if (this->GetRoot() != 0){
      //calls the routine to search the nodes recursively
      NearestQuery(sample, k, result, this->GetRoot());
   }//end if
   // Return the result set.
   return result;
}//end stVPTree<ObjectType, EvaluatorType>::NearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stVPTree<ObjectType, EvaluatorType>::NearestQuery(tObject * sample,
                       u_int32_t k, tResult * result, u_int32_t pageID){
   stPage * currPage;
   stVPNode * currNode;
   double distance;
   tObject tmpObj;

   // Let's search
   if (pageID != 0){
      currPage = this->myPageManager->GetPage(pageID);
      currNode = new stVPNode(currPage);

      // Rebuild the first object
      tmpObj.Unserialize(currNode->GetObject(), currNode->GetObjectSize());
      // Evaluate the distance
      distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
      // Is there more than k elements?
      if (result->GetNumOfEntries() < k){
         // No. Add the object.
         result->AddPair(tmpObj.Clone(), distance);
      }else{
         // Yes, there is more than k elements.
         if (distance < result->GetMaximumDistance()){
            // Add the object.
            result->AddPair(tmpObj.Clone(), distance);
            // Cut if there is more than k elements.
            result->Cut(k);
         }//end if
      }//end if

      // Analize the subtrees.
      if (distance <= currNode->GetRadius()){
         //first the left side
         NearestQuery(sample, k, result, currNode->GetLeftPageID());
         //try to prune the right side
         if (distance + result->GetMaximumDistance() > currNode->GetRadius()){
            NearestQuery(sample, k, result, currNode->GetRightPageID());
         }//end if
      }else{
         //first the right side
         NearestQuery(sample, k, result, currNode->GetRightPageID());
         //try to prune the left side
         if (distance - result->GetMaximumDistance() <= currNode->GetRadius()){
            NearestQuery(sample, k, result, currNode->GetLeftPageID());
         }//end if
      }//end if

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end stVPTree<ObjectType, EvaluatorType>::NearestQuery


#endif //__STVPTREE_H

