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
#ifndef __STDUMMYTREE_H
#define __STDUMMYTREE_H

#include <math.h>
#include <arboretum/stCommon.h>
#include <arboretum/stMetricTree.h>
#include <arboretum/stDummyNode.h>

#include <exception>
#include <iostream>


/**
* This class template implements a Dummy Tree. The Dummy Tree has the same
* external interface and behavior of a Metric Tree but is implemented as
* sequential list.
*
* <P>It can perform both range and k-nearest neighbout queries but without
* the performance associated with other structures. Its algorithms always
* have O(N) (except for insertion which is O(1)). In other words, it will
* take a lot of time to give answers but it will assure their correctness.
*
* <P>This class was developed to generate perfect answers to queries. It
* allows the build of automated test programs for other metric trees
* implemented by this library.
*
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Finish the implementation.
* @todo Compute statistics.
* @version 1.0
* @ingroup dummy
*/
template <class ObjectType, class EvaluatorType>
class stDummyTree: public stMetricTree<ObjectType, EvaluatorType>{

   public:

      /**
      * This is the class that abstracts the object used by this metric tree.
      */
      typedef ObjectType tObject;

      /**
      * This is the class that abstracts the metric evaluator used by this metric
      * tree.
      */
      typedef EvaluatorType tMetricEvaluator;

      /**
      * This is the class that abstracts an result set for simple queries.
      */
      typedef stResult <ObjectType> tResult;
#ifdef __stCKNNQ__
      /**
      * This is the class that abstracts an result set for simple queries.
      */
      typedef stConstrainedResult <ObjectType> tConstrainedResult;
      
#endif // __stCKNNQ__
      /**
      * This is the class that abstracts an result set for joined queries.
      */
      typedef stJoinedResult <ObjectType> tJoinedResult;



     class Iterator{
     public:
       Iterator(){
         pageID_ = 0;
         entryOnPage_ = 0;
         pageManager_ = NULL;
         currPage = NULL;
         currNode = NULL;
         numberOfEntries_ = 0;
         obj_ = new tObject();
       }

       Iterator(u_int32_t pageID, int entryOnPage, stPageManager* pageManager){
         currPage = NULL;
         currNode = NULL;
         numberOfEntries_ = 0;
         obj_ = new tObject();

         pageID_ = pageID;
         entryOnPage_ = entryOnPage;
         pageManager_ = pageManager;

         load();
       }

       virtual ~Iterator(){
         if(currNode)
           delete currNode;
         if(currPage)
           pageManager_->ReleasePage(currPage);
         if(obj_)
           delete obj_;
       }


       u_int32_t getPageID(){
         return pageID_;
       }

       int entryOnPage(){
         return entryOnPage_;
       }

       tObject* getObject(){
         return obj_;
       }

       tObject* operator*(){
         return getObject();
       }


       Iterator& operator++(int){
         entryOnPage_++;
         if(entryOnPage_ < numberOfEntries_){
           loadObject();
         }else{

           pageID_ = currNode->GetNextNode();
           entryOnPage_ = 0;

           delete currNode;
           pageManager_->ReleasePage(currPage);
           currNode = NULL;
           currPage = NULL;

           load();
         }
       }

       bool operator==(const Iterator& it){
         if(pageID_ == it.pageID_ && entryOnPage_ == it.entryOnPage_)
           return true;
         else
           return false;
       }

       bool operator!=(const Iterator& it){
         if(pageID_ == it.pageID_ && entryOnPage_ == it.entryOnPage_)
           return false;
         else
           return true;
       }


       Iterator& operator=(const Iterator& it){
         if(currNode)
           delete currNode;
         if(currPage)
           pageManager_->ReleasePage(currPage);

         currPage = NULL;
         currNode = NULL;
         numberOfEntries_ = 0;

         pageID_ = it.pageID_;
         entryOnPage_ = it.entryOnPage_;
         pageManager_ = it.pageManager_;

         load();
       }
     private:
       bool load(){
         if(pageID_ != 0){

           currPage = pageManager_->GetPage(pageID_);
           currNode = new stDummyNode(currPage);

           numberOfEntries_ = currNode->GetNumberOfEntries();

           return loadObject();
         }
         return false;
       }


       bool loadObject(){
         if(entryOnPage_ < numberOfEntries_){
           obj_->Unserialize(currNode->GetObject(entryOnPage_), currNode->GetObjectSize(entryOnPage_));
           return true;
         }else
           throw;
         return false;
       }

       u_int32_t pageID_;
       int entryOnPage_;
       stPageManager* pageManager_;
       tObject* obj_;


       stPage* currPage;
       stDummyNode* currNode;
       int numberOfEntries_;
     };

     Iterator begin(){
       u_int32_t pageID;
       int entryOnPage;

       pageID = this->GetRoot();
       entryOnPage = 0;
       return Iterator(pageID, entryOnPage, this->myPageManager);
     }

     Iterator end(){
       return Iterator(0, 0, this->myPageManager);
     }





      /**
      * Creates a new instance of the Dummy Tree.
      *
      * @param pageman The Page Manager to be used.
      */
      stDummyTree(stPageManager * pageman);


      stDummyTree(stPageManager * pageman, EvaluatorType* metricEvaluator);

      /**
      * Disposes this tree and releases all associated resources.
      */
      virtual ~stDummyTree(){

         if (HeaderPage != NULL){
            // Release the header page.
            this->myPageManager->ReleasePage(HeaderPage);
         }//end if
      }//end ~stDummyTree

      /**
      * This method adds an object to the metric tree. This method may fail it the object size
      * exceeds the page size - 16.
      *
      * @param obj The object to be added.
      * @return True for success or false otherwise.
      */
      virtual bool Add(tObject * obj);

      /**
      * Returns the number of objetcs of this sequencial scan.
      */
      virtual long GetNumberOfObjects(){
         return Header->ObjectCount;
      }//end GetNumberOfObjects

      /**
      * Returns the number of nodes of this tree.
      */
      virtual long GetNodeCount(){
         return Header->NodeCount;
      }//end GetNodeCount

      /**
      * Returns the MaxOccupation of the nodes.
      */
      virtual u_int32_t GetMaxOccupation(){
         return Header->MaxOccupation;
      }//end GetMaxOccupation

      /**
      * Returns the MaxOccupation of the nodes.
      */
      void SetMaxOccupation(u_int32_t newValue){
         if (newValue > Header->MaxOccupation){
            Header->MaxOccupation = newValue;
         }//end if
      }//end SetMaxOccupation

      /**
      * Returns the root pageID.
      */
      u_int32_t GetRoot(){
         return Header->Root;
      }//end GetRoot

      /**
      * This method will perform a range query. The result will be a set of
      * pairs object/distance.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      virtual tResult * RangeQuery(tObject * sample, double range);

      /**
      * This method will perform a reverse range query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param range The range of the results. All object that are
      *  greater than the range distance will be included in the result set.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      virtual tResult * ReversedRangeQuery(tObject * sample, double range);

      /**
      * This method will perform a k nearest neighbor query.
      *
      * @param sample The sample object.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      virtual tResult * NearestQuery(tObject * sample, u_int32_t k, bool tie = false);

      /**
      * This method will perform a K-Farthest neighbor query.
      *
      * @param sample The sample object.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      virtual tResult * FarthestQuery(tObject * sample, u_int32_t k, bool tie = false);

      /**
      * This method will perform a range query with a limited number of results.
      *
      * <P>This query is a combination of the standard range query and the standard
      * k-nearest neighbour query. All objects which matches both conditions
      * will be included in the result.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @param tie The tie list. This parameter is optional. Default false;
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      virtual tResult * KAndRangeQuery(tObject * sample, double range,
            u_int32_t k, bool tie=false);

      /**
      * This method will perform range query with a limited number of results.
      *
      * <P>This query is a combination of the standard range query and the
      * standard k-nearest neighbour query. All objects which matches with
      * one of two conditions will be included in the result.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @param tie The tie list. This parameter is optional. Default false;
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      virtual tResult * KOrRangeQuery(tObject * sample, double range,
            u_int32_t k, bool tie=false);

      /**
      * This method will perform a ring query. The result will be a set of
      * pairs object/distance.
      *
      * <P>The object pointed by <b>sample</b> will not be destroyed by this
      * method.
      *
      * @param sample The sample object.
      * @param inRange The inner range of the results.
      * @param outRange The outter range of the results.
      * @return The result.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The inRange must be less than the outRange.
      * @exception std::logic_error If this method is not supported by this tree.
      */
      virtual tResult * RingQuery(tObject * sample, double inRange,
            double outRange);

      /**
      * This method will perform a grouping SUM range query. The result will be a set of
      * pairs object/distance.
      *
      * @param sampleList The list of sample objects.
      * @param sampleSize The number of samples of the param sampleList.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @autor Implemented by Humberto Razente
      */
      tResult * SumRangeQuery(tObject ** sampleList, u_int32_t sampleSize,
                              double range);

      /**
      * This method will perform a grouping SUM k nearest neighbor query.
      *
      * @param sampleList The list of sample objects.
      * @param sampleSize The number of samples of the param sampleList.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * SumNearestQuery(tObject ** sampleList, u_int32_t sampleSize,
                                u_int32_t k, bool tie = false);

      /**
      * This method will perform a grouping MAX range query. The result will be a set of
      * pairs object/distance.
      *
      * @param sampleList The list of sample objects.
      * @param sampleSize The number of samples of the param sampleList.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @autor Implemented by Humberto Razente
      */
      tResult * MaxRangeQuery(tObject ** sampleList, u_int32_t sampleSize,
                              double range);

      /**
      * This method will perform a grouping MAX k nearest neighbor query.
      *
      * @param sampleList The list of sample objects.
      * @param sampleSize The number of samples of the param sampleList.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult* MaxNearestQuery(tObject ** sampleList, u_int32_t sampleSize,
                               u_int32_t k, bool tie = false);

      /**
      * This method will perform a grouping ALL range query. The result will be a set of
      * pairs object/distance.
      *
      * @param sampleList The list of sample objects.
      * @param sampleSize The number of samples of the param sampleList.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @autor Implemented by Humberto Razente
      */
      tResult * AllRangeQuery(tObject ** sampleList, u_int32_t sampleSize,
                              double range);

      /**
      * This method will perform a grouping ALL k nearest neighbor query.
      *
      * @param sampleList The list of sample objects.
      * @param sampleSize The number of samples of the param sampleList.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult* AllNearestQuery(tObject ** sampleList, u_int32_t sampleSize,
                               u_int32_t k, bool tie = false);

      /**
      * This method will perform a k-nearest neighbor joined query.
      *
      * @param dummyTree The Dummy tree to be joined.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      * @autor Implemented by Humberto Razente
      */
      tJoinedResult * NearestJoinQuery(stDummyTree * dummyTree, u_int32_t k,
                                       bool tie = false);

      /**
      * This method will perform a range joined query.
      *
      * @param dummyTree The dummyTree to be joined.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      * @autor Implemented by Humberto Razente
      */
      tJoinedResult * RangeJoinQuery(stDummyTree * dummyTree, double range);

      /**
      * This method will perform a k-closest neighbor joined query.
      *
      * @param dummyTree The Dummy tree to be joined.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      * @autor Implemented by Humberto Razente
      */
      tJoinedResult * ClosestJoinQuery(stDummyTree * dummyTree, u_int32_t k,
                                       bool tie = false);

      /**
      * This method will perform a PRE-CONDITION CONSTRAINED k nearest neighbor query.
      *
      * @param sample The sample object.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      template <class TupleType>
      tResult * preConstrainedNearestQuery(tObject * sample, u_int32_t k, u_int32_t idx,
            bool (*compare)(const void *, const void *), const void * value);

      template <class TupleType>
      tResult * intraConstrainedNearestQueryCountGreaterThanOrEqual(tObject * sample,
            u_int32_t k, u_int32_t idx, bool (*compare)(const void *, const void *), const void * value,
            u_int32_t aggValue);

      template <class TupleType>
      tResult * intraConstrainedNearestQueryCountLessThanOrEqual(tObject * sample,
            u_int32_t k, u_int32_t idx, bool (*compare)(const void *, const void *), const void * value,
            u_int32_t aggValue);

      template <class TupleType>
      tResult * intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual(tObject * sample,
            u_int32_t k, u_int32_t idx, bool (*compare)(const void *, const void *), const void * value,
            u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue);

      template <class TupleType>
      tResult * intraConstrainedNearestQueryCountDistinctLessThanOrEqual(tObject * sample,
            u_int32_t k, u_int32_t idx, bool (*compare)(const void *, const void *), const void * value,
            u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue);

   private:

      /**
      * This type defines the header of the Dummy Tree.
      */
      typedef struct DummyTreeHeader{
         /**
         * The root.
         */
         u_int32_t Root;

         /**
         * The number of the objects in this page.
         */
         u_int32_t ObjectCount;

         /**
         * Total number of nodes.
         */
         u_int32_t NodeCount;

         /**
         * Maximum number of objects in a node.
         */
         u_int32_t MaxOccupation;
      } stDummyTreeHeader;

      /**
      * The header page. It will be kept in memory all the time to avoid
      * reads.
      */
      stPage * HeaderPage;

      /**
      * The header of the "tree".
      */
      stDummyTreeHeader * Header;

      /**
      * If true, the header must be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * Creates the header for an empty tree.
      */
      void Create(){

         LoadHeader();
         Header->Root = 0;
         Header->ObjectCount = 0;
         Header->NodeCount = 0;
         Header->MaxOccupation = 0;
         HeaderUpdate = true;
         WriteHeader();
      }//end Create

      /**
      * Loads the header from the page manager.
      */
      void LoadHeader(){
         HeaderPage = this->myPageManager->GetHeaderPage();
         Header = (stDummyTreeHeader *) HeaderPage->GetData();
      }//end LoadHeader

      /**
      * Writes the header into the Page Manager.
      */
      void WriteHeader(){
         if (HeaderUpdate){
            this->myPageManager->WriteHeaderPage(HeaderPage);
            HeaderUpdate = false;
         }//end if
      }//end WriteHeader

      /**
      * Set the new root pageID.
      */
      void SetRoot(u_int32_t newRootPageID){
         Header->Root = newRootPageID;
      }//end SetRoot

      /**
      * Updates the object counter.
      */
      void UpdateObjectCounter(int inc){
         Header->ObjectCount += inc;
         HeaderUpdate = true;
      }//end UpdateObjectCounter

};//end stDummyTree

#include <arboretum/stDummyTree-inl.h>

#endif //__STDUMMYTREE_H
