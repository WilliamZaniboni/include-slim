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
* This file defines Partition.
*
* @version 1.0
* @author Enzo Seraphim(seraphim@icmc.usp.br)
*/

#ifndef __STPARTITIONHASH_H
#define __STPARTITIONHASH_H

#include <arboretum/stUtil.h>

#include <arboretum/stMetricTree.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stPartitionNode.h>

//=============================================================================
// Class template stPartitionHash
//-----------------------------------------------------------------------------
/**
* This class defines all behavior of the Partition Hash.
* Probably most of the atributes will be stored in the header page of the used
* stPageManager (stDiskPageManager or stMemoryPageManager).
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @ingroup partition
*/
template <class ObjectType, class EvaluatorType>
class stPartitionHash: public stMetricTree< ObjectType, EvaluatorType> {
   public:
      /**
      * This structure defines the Partition Hash header structure. This type
      * was left public to allow the creation of debug tools.
      */
      struct stPartitionHeader{
         /**
         * Magic number. This is a short string that must contains the magic
         * string "PART". It may be used to validate the file (this feature
         * is not implemented yet).
         */
         char Magic[4];

         /**
         * Representative choose method.
         */
         int ChooseMethod;

         /**
         * The root of the Index Bucket of Partition Hash
         */
         u_int32_t RootIndex;

         /**
         * The root of the Leaf Bucket of Partition Hash
         */
         u_int32_t RootLeaf;

         /**
         * The root page that store the representative objects
         * of Partition Hash
         */
         u_int32_t RootRep;

         /**
         * The representative number of partition
         */
         u_int32_t NumberRep;

         /**
         * The height of the partition
         */
         int Height;

         /**
         * Total number of records
         */
         u_int32_t ObjectCount;

         /**
         * Total number of index bucket.
         */
         u_int32_t IndexBucketCount;

         /**
         * Total number of leaft bucket.
         */
         u_int32_t LeafBucketCount;

         /**
         * Total number of representative objects node.
         */
         u_int32_t RepNodeCount;
      };

      /**
      * These constants are used to define the representative choose method.
      */
      enum tChooseMethod{
         /**
         * This method choice randomly a representative object
         */
         cmRANDOM,
         /**
         * This method choice a representative object that have the more variance
         */
         cmVARIANCE,
         /**
         * This method choice a representative object that have the more amount of
         * differ region descritor.
         */
         cmDIFFER
      };//end tChooseMethod

      /**
      * This is the class that abstracs an result set.
      */
      typedef stResult <ObjectType> tResult;

      /**
      * Creates a new extensible hash using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this metric tree.
      */
      stPartitionHash (stPageManager * pageman);

      /**
      * Dispose all used resources, ie, it is the destructor method.
      *
      * @see stPartitionHash()
      */
      virtual ~stPartitionHash();

      /**
      * Sets the representative choose method. The default method is cmVARIANCE.
      *
      * @param method Choose method name.
      * @see tChooseMethod
      */
      void SetChooseMethod(enum tChooseMethod method){
         Header->ChooseMethod = method;
         HeaderUpdate = true;
      }//end SetChooseMethod

      /**
      * Returns the representative choose method.
      */
      int GetChooseMethod(){
         return Header->ChooseMethod;
      }//end GetChooseMethod

      /**
      * This method adds an object to the Hash Partition
      *
      * @param obj The object to be added.
      * @return Return true if the object was inserted.
      */
      virtual bool Add(ObjectType * newObj);

      /**
      * Returns the represeheight of the tree.
      */
      int GetGlobalNumberRep(){
         return RepObjects->GetNumberRep();
      }//end GetGlobalNumberRep

      /**
      * Returns the height of the tree.
      */
      virtual int GetHeight(){
         return Header->Height;
      }//end GetHeight

      /**
      * Returns the number of objetcs of this tree.
      */
      virtual long GetNumberOfObjects(){
         return Header->ObjectCount;
      }//end GetNumberOfObjects

      /**
      * Returns the number of node.
      *
      * @return The sum index bucket + leaf bucket +
      * representative node
      */
      virtual long GetNodeCount(){
         return (Header->IndexBucketCount + Header->LeafBucketCount +
            Header->RepNodeCount);
      }//end GetBucketCount

      /**
      * Returns the number of index bucket.
      *
      * @return The number of index bucket.
      */
      long GetIndexBucketCount(){
         return Header->IndexBucketCount;
      }//end GetIndexBucketCountt

      /**
      * Returns the number of leaf bucket.
      *
      * @return The number of leaf bucket.
      */
      long GetLeafBucketCount(){
         return Header->LeafBucketCount;
      }//end GetLeafBucketCount

      /**
      * Returns the number of representative objects node.
      *
      * @return The number of representative objects node.
      */
      long GetRepNodeCount(){
         return Header->RepNodeCount;
      }//end GetRepNodeCount

      /**
      * Gets the maximum user data size available in this tree.
      *
      * @see WriteUserData()
      * @see ReadUserData()
      * @bug This feature is not implemented yet.
      */
      u_int32_t GetUserDataSize(){
         return 0;
      }//end GetUserDataSize

      /**Petrus Henrique Ribeiro dos Anjos
      * Writes an additional user data.
      *
      * <P>This feature allows users to write additional information in to the
      * free area of the header page. The available space will deppends on the
      * size of the header page (see the page manager documentation for more
      * details).
      *
      * @param userData A pointer to the user data.
      * @param size The size of the user data.
      * @return True for success or false if the user data doesn't fit in this
      * area.
      * @see GetUserDataSize()
      * @see ReadUserData()
      * @bug This feature is not implemented yet.
      */
      bool WriteUserData(const unsigned char * userData, u_int32_t size){
         return false;
      }//end WriteUserData

      /**
      * Reads an additional user data.
      *
      * <P>This feature allows users to write additional information in to the
      * free area of the header page. The available space will deppends on the
      * size of the header page (see the page manager documentation for more
      * details).
      *
      * @param userData A pointer to the user data.
      * @param size The size of the user data.
      * @return True for success or false if the user data doesn't fit in this
      * area.
      * @see GetUserDataSize()
      * @see WriteUserData()
      * @bug This feature is not implemented yet.
      */
      bool ReadUserData(unsigned char * userData, u_int32_t size){
         return false;
      }//end ReadUserData

      /**
      * Returns the global representative objects through index. The first index
      * is zero.
      *
      * @param id The index.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The representative object.
      */
      ObjectType * GetGlobalRepObject (int id){
         return RepObjects->GetRepObject(id);
      }//end GetGlobalRepObject

      /**
      * Returns the less value of descriptor region of Partition.
      *
      * @return The less value.
      */
      stPartitionRegionDesc GetBeginRegion();

      /**
      * Returns the more value of descriptor region of Partition.
      *
      * @return The more value.
      */
      stPartitionRegionDesc GetEndRegion();

      /**
      * Apply the hash function in a object and returns a region descriptor.
      *
      * @param obj The object.
      * @param reg The region descriptor (returning value).
      */
      void ObjectHashFunction(ObjectType * obj, stPartitionRegionDesc & reg);

      /**
      * Apply the hash function in a object with a radius and returns a list
      * of the region descriptor.
      *
      * @param obj The object.
      * @param radius The Radius.
      * @param list The list of descriptor region (returning value).
      */
      void RadiusHashFunction(ObjectType * obj, double radius, stPartitionListRegionDesc & list);

      /**
      * This method will perform a range query. The result will be a set of
      * pairs object/distance.
      *
      * <P>The object pointed by <b>sample</b> will not be destroyed by this
      * method.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @return The result.
      * @warning The instance of tResult returned must be destroied by user.
      * @exception std::logic_error If this method is not supported by this tree.
      */
       tResult * RangeQuery(ObjectType * sample, double range);

      /**
      * This method will perform a k nearest neighbour query. It is a new
      * version that do a range query with a dynamic radius.
      *
      * <P>The object pointed by <b>sample</b> will not be destroyed by this
      * method.
      *
      * @param sample The sample object.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
//    tResult * NearestQuery(ObjectType * sample, int k, bool tie = false);

      /**
      * This method will return the object in the tree that has the distance 0
      * to the query object. In other words, the query object itself.
      *
      * <P>If there are more elements that has distance 0 to the query
      * object, this method will return the first found.
      *
      * <P>The object pointed by <b>sample</b> will not be destroyed by this
      * method.
      *
      * @param sample The sample object.
      * @return The result or NULL if this method is not implemented.
      * @warning This method return only one object that has distance 0 to the
      * query object.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * PointQuery(ObjectType * sample);

      /**
      * This method will return the object in the tree that has the distance 0
      * to the query object. In other words, the query object itself.
      *
      * <P>If there are more elements that has distance 0 to the query
      * object, this method will return the first found.
      *
      * <P>The object pointed by <b>sample</b> will not be destroyed by this
      * method.
      *
      * @param sample The sample object.
      * @return The result or NULL if this method is not implemented.
      * @warning This method return only one object that has distance 0 to the
      * query object.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * SequencialPointQuery(ObjectType * sample);


private:
      /**
      * This structure holds a promotion data.
      */
      struct stPartitionSubtreeInfo{
         /**
        * The region descriptor of the subBucket.
        */
         stPartitionRegionDesc Region;

         /**
        * The pageID of the subBucket.
        */
         u_int32_t PageID;
      };

      /**
      * This enumeration defines the actions to be taken after an call of
      * InsertRecursive.
      */
      enum stInsertAction{
         /**
         * No action required.
         */
         NO_ACT,

         /**
         * Split occured. Update subtrees.
         */
         PROMOTION
      };//end stInsertAction

      /**
      * If true, the header mus be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * The Partition Hash header. This variable points to data in the HeaderPage.
      */
      stPartitionHeader * Header;

      /**
      * Pointer to the header page.
      * The Partition Hash keeps this page in memory for faster access.
      */
      stPage * HeaderPage;

      /**
      * The representative objects used in the hash function.
      */
      tmpl_stPartitionGlobalRep * RepObjects;

      /**
      * Sets all header's fields to default values.
      *
      * @warning This method will destroy the tree.
      */
      void DefaultHeader();

      /**
      * Loads the header page and set the Header pointer. The previous header
      * page, if exists, will be lost.
      *
      * @exception std::logic_error If the page size is too small.
      */
      void LoadHeader();

      /**
      * Updates the header in the file if required.
      */
      void WriteHeader(){
         if (HeaderUpdate){
            myPageManager->WriteHeaderPage(HeaderPage);
            HeaderUpdate = false;
         }//end if
      }//end WriteHeader

      /**
      * Disposes the header page if it exists. It also updates its contents
      * before destroy it.
      *
      * <P>This method is called by the destructor.
      */
      void FlushHeader();

      /**
      * Read representative objects from nodes
      */
      void LoadRepresentatives();

      /**
      * Save representative objects to nodes
      */
      void SaveRepresentatives();

      /**
      * Creates a new empty page and updates the bucket counter.
      */
      stPage * NewPage(stPartitionNode::stPartitionNodeType type){
         switch (type) {
         case stPartitionNode::INDEX:
            Header->IndexBucketCount++;
            break;
         case stPartitionNode::LEAF:
            Header->LeafBucketCount++;
            break;
         case stPartitionNode::REPRESENT:
            Header->RepNodeCount++;
            break;
         }
         HeaderUpdate = true;
         return myPageManager->GetNewPage();
      }//end NewPage

      /**
      * Disposes a given page and updates the page counter.
      */
      void DisposePage(stPage * page, stPartitionNode::stPartitionNodeType type){
         switch (type) {
         case stPartitionNode::INDEX:
            Header->IndexBucketCount--;
            break;
         case stPartitionNode::LEAF:
            Header->LeafBucketCount--;
            break;
         case stPartitionNode::REPRESENT:
            Header->RepNodeCount--;
            break;
         }
         myPageManager->DisposePage(page);
         HeaderUpdate = true;
      }//end DisposePage

      /**
      * Get root of page id.
      */
      u_int32_t GetRoot(){
         //if exists root
         if (GetRootIndex()!=0){
            return Header->RootIndex;
         }else{
            return Header->RootLeaf;
         }//end if
      }//end GetRootIndex

      /**
      * Get root of Index Bucket page id.
      */
      u_int32_t GetRootIndex(){
         return Header->RootIndex;
      }//end GetRootIndex

      /**
      * Sets a new root of Index Bucket.
      */
      void SetRootIndex(u_int32_t root){
         Header->RootIndex = root;
         HeaderUpdate = true;
      }//end SetRootIndex

      /**
      * Get root of Leaf Bucket page id.
      */
      u_int32_t GetRootLeaf(){
         return Header->RootLeaf;
      }//end GetRootIndex

      /**
      * Sets a new root of Leaf Bucket.
      */
      void SetRootLeaf(u_int32_t root){
         Header->RootLeaf = root;
         HeaderUpdate = true;
      }//end SetRootIndex

      /**
      * Get root page of representative objects.
      */
      u_int32_t GetRootRep(){
         return Header->RootRep;
      }//end GetRootRep

      /**
      * Sets a new root page of representative objects
      */
      void SetRootRep(u_int32_t root){
         Header->RootRep = root;
         HeaderUpdate = true;
      }//end SetRootRep

      /**
      * Updates the object counter.
      */
      void UpdateObjectCounter(int inc){
         Header->ObjectCount += inc;
         HeaderUpdate = true;
      }//end UpdateObjectCounter

      /**
      * This method inserts an object in the Partition recursively.
      * This method is the core of the insertion method. It will manage
      * promotions and splits.
      * <P>For each action, the returning values may assume the following
      * configurations:
      *     - NO_ACT:
      *           - promo1.Radius will have the new subtree radius.
      *           - Other parameters will not be used.
      *     - PROMOTION:
      *           - promo1 will contain the information about the choosen subtree.
      *                 - If promo1.Rep is NULL, the representative of the
      *                   subtree will not change.
      *           - promo2 will contain the information about the promoted subtree.
      *
      * @param curBucketID The current bucket ID.
      * @param regionIns The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param objIns The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param beginRegion The region begin that the bucket represent.
      * @param endRegion The end begin that the bucket represent.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      * @return The action to be taken after the returning. See enum
      * stInsertAction for more details.
      */
      int InsertRecursive( u_int32_t curBucketID, stPartitionRegionDesc regionIns,
      ObjectType * objIns, stPartitionRegionDesc beginRegion, stPartitionRegionDesc endRegion,
      stPartitionSubtreeInfo & splited, stPartitionSubtreeInfo & promoted);

      /**
      * Creates and updates the new root of the Partition Hash.
      *
      * @param splited Information about the rootBucket splited.
      * @param promoted Infromation about the subBucket promoted.
      */
      void AddNewRoot(stPartitionSubtreeInfo splited, stPartitionSubtreeInfo promoted);

      /**
      * This method splits an index bucket in 2. This will get 2 buckets and will
      * redistribute the object set between these.
      *
      * @param fullBucket The bucket to be splited.
      * @param promoteBucket The new index bucket.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitIndex(stPartitionIndexBucket * fullBucket,
      stPartitionIndexBucket * promoteBucket, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket choicing randomly representative objects.
      * This will redistribute the objects between these buckets.
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafChoiceTwoRepRandom(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket choicing two representative objects that have
      * the more variance. This will redistribute the objects between these buckets.
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafChoiceTwoRepByVariance(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket choicing two representative objects that have
      * the more amount of different region descritor. This will redistribute the objects
      * between these buckets.
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafChoiceTwoRepByDiffer(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket choicing a randomly representative
      * object. This will redistribute the objects between these buckets.
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafChoiceOneRepRandom(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket choicing a representative object that have
      * the more variance. This will redistribute the objects between these buckets.
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param beginRegion The region begin that the bucket represent.
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafChoiceOneRepByVariance(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket choicing a representative object that have
      * the more amount of differ region descritor. This will redistribute the objects
      * between these buckets.
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafChoiceOneRepByDiffer(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * This method splits a leaf bucket without choice representative objects.
      * This will redistribute the objects between these buckets .
      *
      * @param fullBucket The leaft bucket to be splited.
      * @param promoteBucket The new leaf bucket.
      * @param newReg The region descriptor of object that will be inserted in
      * the leaf bucket.
      * @param newObj The new object to be inserted in a leaf bucket. This
      * instance will never be destroyed.
      * @param splited Information about the choosen subBucket splited (returning value).
      * @param promoted Infromation about the promoted subBucket promoted (returning value).
      */
      void SplitLeafWithoutChoiceRep(stPartitionLeafBucket * fullBucket,
      stPartitionLeafBucket * promoteBucket, stPartitionRegionDesc newReg,
      ObjectType * newObj, stPartitionSubtreeInfo & splited,
      stPartitionSubtreeInfo & promoted);

      /**
      * Update descriptor regions in the full leaf bucket.
      *
      * @param fullBucket The leaft full bucket.
      */
      void UpdateRegionLeaf(stPartitionLeafBucket * fullBucket);

      /**
      * Region desciptor generator.
      *
      * @param countAxis Counter of qualify coordenate in the axis (vector).
      * @param CoordAxis Qualify coordenate in the axis (matrix).
      * @param list The list of region descriptor(returning value).
      */
      void GeneratorRegionDesc(unsigned char * countCoordAxis, unsigned char ** CoordAxis,
      stPartitionListRegionDesc & list);

      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the result.
      * @param distanceRepres The distance of the representative.
      * @see tResult * RangeQuery()
      */
//       void RangeQuery(u_int32_t pageID, tResult * result,
//       ObjectType * sample, double range,
//       double distanceRepres);

      /**
      * This method will perform a K Nearest Neighbor query using a priority
      * queue.
      *
      * @param result the result set.
      * @param sample The sample object.
      * @param rangeK The range of the results.
      * @param k The number of neighbours.
      * @see tResult * NearestQuery
      */
//       void NearestQuery(tResult * result, ObjectType * sample, double rangeK, int k);




};//end stPartitionHash

#include "stPartitionHash-inl.h"

#endif //__STPARTITIONHASH_H
