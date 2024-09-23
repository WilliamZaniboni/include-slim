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
* This file defines the nodes of Partition.
*
* @version 1.0
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @todo Review of documentation.
*/

#ifndef __STPARTITIONNODE_H_
#define __STPARTITIONNODE_H_

#include <arboretum/stPage.h>

#include <stdexcept>

#include <cmath>
#include <cstring>

#include <iostream>

//-----------------------------------------------------------------------------
// Class stPartitionGlobalRepresentatives
//-----------------------------------------------------------------------------
/**
* This class defines global representatives of Partition.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @ingroup partition
*/

// This macro will be used to replace the declaration of
// stPartitionGlobalRepresentatives<ObjectType>
#define tmpl_stPartitionGlobalRep stPartitionGlobalRepresentatives<ObjectType, EvaluatorType>

template <class ObjectType, class EvaluatorType>
class stPartitionGlobalRepresentatives{
   public:
      /**
      * Constructor of the stPartitionGlobalRepresentatives
      */
      stPartitionGlobalRepresentatives(EvaluatorType * metric){
         IdCandidate = -1;
         NumberRep = 0;
         myMetric  = metric;
      }

      /**
      * Destructor of the stPartitionGlobalRepresentatives
      */
      ~stPartitionGlobalRepresentatives();

      /**
      * Returns the representative object through index. The first index is zero.
      *
      * @param id The index.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The representative object.
      */
      ObjectType * GetRepObject(int id);

      /*
      * Returns the number of representatives object.
      *
      * @returns The number of representatives object.
      */
      int GetNumberRep(){
         return NumberRep;
      }

      /**
      * Returns true if the distance from any representative object to zero.
      *
      * @param obj The representative object.
      * @return Returns true if the distance from any representative object
      * to zero.
      */
      bool DistanceIsZero(ObjectType * obj);

      /**
      * Add the representative object.
      *
      * @param obj The representative object.
      * @param pos The position where representative object must be insered.
      * @warning The parameter numIns is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return Returns the pointer to item insered.
      */
      void Add(int pos, ObjectType * obj);

      /**
      * Add a candidate representative object. This method remove the previous
      * candidate representative object
      *
      * @param obj The representative object.
      * @param pos The position where representative object must be insered.
      * @warning The parameter numIns is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return Returns the pointer to item insered.
      */
      void AddCandidate(int pos, ObjectType * obj);

      /**
      * Remove the candidate representative object.
      */
      void RemoveCandidate();

      /**
      * Remove all representative object.
      */
      void RemoveAll();

      /**
      * Finds the NumberInsertion and returns its index.
      *
      * @param numIns The NumberInsertion that will be finded.
      * @warning The parameter numIns is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @returns The Index of NumberInsertion that was finded.
      */
      int FindNumberInsertion(unsigned char numIns);

      /**
      * Returns the distance from id of axis to id of object.
      *
      * @param idAxis The id the axis.
      * @param idObj The id the object.
      * @warning The parameter idAxis and idObject are not verified by this
      * implementation unless __stDEBUG__ is defined at compile time.
      * @returns The Distance.
      */
      double GetAxisDistance( int idAxis, int idObj);

      /**
      * Returns the distance lower bound from id of axis to id of object.
      *
      * @param idAxis The id the axis.
      * @param idObj The id the object.
      * @warning The parameter idAxis and idObject are not verified by this
      * implementation unless __stDEBUG__ is defined at compile time.
      * @returns The Distance.
      */
      double GetAxisDistanceLowerBound( int idAxis, int idObj);

      /**
      * Returns the disntance of object far of axis.
      *
      * @param idAxis The id the axis.
      * @warning The parameter idAxis is not verified by this
      * implementation unless __stDEBUG__ is defined at compile time.
      * @returns The Distance.
      */
      double GetAxisDistanceMax( int idAxis );

   private:
      /**
      * The Vector
      */
      ObjectType * Global[16];

      /**
      * The Vector maximum distance.
      */
      double MaxDistance[16];

      /**
      * Number of Insertion
      */
      int NumberInsertion[16];

      /**
      * The Candidate
      */
      int IdCandidate;

      /**
      * The number of representatives
      */
      int NumberRep;

      /**
      * The Metric
      */
      EvaluatorType * myMetric;

      /**
      * The distance matrix that represent the axis
      */
      double DistanceAxis[16][16];

      /**
      * Method that insert a item.
      *
      * @param obj The representative object.
      * @param pos The position where representative object must be insered.
      * @warning The parameter numIns is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return Returns the pointer to item insered.
      */
      void AddItem(int pos, ObjectType * obj);

};//end stPartitionGlobalRepresentatives

//-----------------------------------------------------------------------------
// Class stPartitionRegionDesc
//-----------------------------------------------------------------------------
/**
* This class defines the region descriptors of Partition.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @ingroup partition
*/
class stPartitionRegionDesc {
   public:
      /**
      * This type represents a region descriptor of the Partition.
      */
      #pragma pack(1)
      struct stRegion{
         /**
         * The representative number to region descriptor
         */
         unsigned char NumberRep;

         /**
         * The value that represent the region descriptor.
         */
         u_int32_t long Region;

      }; //end stRegion
      #pragma pack()

      /**
      * Constructor
      * param glob The representatives globals
      */
      stPartitionRegionDesc (){
         Desc.NumberRep = 0;
         Desc.Region = 0;
      }

      /**
      * Constructor by insertion
      */
      stPartitionRegionDesc (unsigned char numRep, u_int32_t long reg){
         #ifdef __stDEBUG__
         if (numRep > 16){
            throw std::logic_error("Maximum representative number is 16.");
         }//end if
         #endif //__stDEBUG__
         Desc.NumberRep = numRep;
         Desc.Region = reg;
      }

      /**
      * Overload in the operator =
      *
      * @param value The value attribution
      */
      void operator = (stPartitionRegionDesc value){
         Desc.NumberRep = value.GetNumberRep();
         Desc.Region = value.GetRegion();
      } //end operator =

      /**
      * This is a method return the amount of representative of region descriptor.
      *
      * @return The amount representative of the region descriptor
      */
      unsigned char GetNumberRep(){
         return Desc.NumberRep;
      } // end GetNumberRep

      /**
      * This is a method return the value of region descriptor.
      *
      * @return The region descriptor
      */
      u_int32_t long GetRegion(){
         return Desc.Region;
      } // end GetRegion

      /**
      * This method change the value of region descriptor.
      *
      * @param reg The region descriptor
      */
      void SetRegion(u_int32_t long reg){
         Desc.Region = reg;
      } // end SetRegion

      /**
      * This method change the amount of representative of region descriptor.
      *
      * @param num The amount representative of the region descriptor
      */
      void SetNumberRepresentatives(unsigned char num){
         #ifdef __stDEBUG__
         if (num > 16){
            throw std::logic_error("Maximum representative number is 16.");
         }//end if
         #endif //__stDEBUG__
         Desc.NumberRep = num;
      } // end SetNumberRepresentatives

      /**
      * Returns the first digit of region descriptor.
      *
      * @returns The first digit of region descriptor.
      */
      int GetFirstDigit();

      /**
      * This method retuns true if the value of descriptor region is different
      * between descriptor regions of minor base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is less.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsDifferentMinorBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is different
      * between descriptor regions of Large base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is less.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsDifferentLargeBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is equal
      * between descriptor regions of minor base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is less.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsEqualMinorBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is equal
      * between descriptor regions of Large base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is less.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsEqualLargeBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is less
      * between descriptor regions of minor base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is less.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsLessMinorBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is less
      * between descriptor regions of large base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is less.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsLessLargeBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is more
      * between descriptor regions of minor base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is bigger or equal.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsMoreMinorBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is more
      * igual between descriptor regions of large base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is bigger or equal.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsMoreLargeBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is more or
      * igual between descriptor regions of minor base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is bigger or equal.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsMoreEqualMinorBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * This method retuns true if the value of descriptor region is more or
      * igual between descriptor regions of large base.
      *
      * @param global The Representatives globals used in the comparation.
      * @param value The compared region descriptor.
      * @return Return true if region descriptor compared is bigger or equal.
      */
      template <class ObjectType, class EvaluatorType>
      bool IsMoreEqualLargeBase (tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc value);

      /**
      * Change the number of representatives setting the region descriptor
      *
      * @param global The Representatives globals used in the change of
      * number of representatives.
      * @param numRep The new number of representatives
      */
      template <class ObjectType, class EvaluatorType>
      void ChangeNumberRep(tmpl_stPartitionGlobalRep * global,
         unsigned char numRep);

   private:
      /**
      * Region Desc
      */
      stRegion Desc;

      /**
      * Convert the region descriptor to a vector of bins.
      * @param base The base.
      * @param result The result of convertion. The result is a vector whith
      * size of base.
      */
      void ConvertToBins(unsigned char base, unsigned char *result);

      /**
      * Returns the numeric value referents to change in number of
      * representatives.
      *
      * @param global The Representatives globals used in the convertion of
      * number of representatives.
      * @param base The base that will be converted
      * @warning The parameter base is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The numeric value converted to base.
      */
      template <class ObjectType, class EvaluatorType>
      u_int32_t long ConvertNumberRep(tmpl_stPartitionGlobalRep * global,
         unsigned char base);

};//end stPartitionRegionDesc

//-----------------------------------------------------------------------------
// Class stPartitionListRegionDesc
//-----------------------------------------------------------------------------
/**
* This class defines a list of Desciptor Region.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @ingroup partition
*/

class stPartitionListRegionDesc{
   public:
      /**
      * Constructor of the stPartitionGlobalRepresentatives
      */
      stPartitionListRegionDesc(){
         Root = Actual = Last = NULL;
         IdActual = -1;
         NumberItem = 0;
      }

      /**
      * Destructor of the stPartitionGlobalRepresentatives
      */
      ~stPartitionListRegionDesc();

      /**
      * Returns the region descriptor through index. The first index is zero.
      *
      * @param id The index.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The region descriptor.
      */
      stPartitionRegionDesc GetRegionDesc(int id);

      /*
      * Returns the number of Item of List.
      *
      * @returns The number of Item of List.
      */
      int GetNumberItem(){
         return NumberItem;
      }

      /**
      * Insert a orded region descriptor.
      *
      * @param regDesc The region descriptor.
      * @return Returns the position where was insered the item.
      */
      int Add(stPartitionRegionDesc regDesc);

   private:
      /**
      * Items of List
      */
      struct stItemRegionDesc{
         /**
         * The representative object.
         */
         stPartitionRegionDesc RegionDesc;

         /**
         * The next item
         */
         stItemRegionDesc * Next;

      }; //end stItemRegionDesc

      /**
      * The first item
      */
      stItemRegionDesc * Root;

      /**
      * The first item
      */
      stItemRegionDesc * Last;

      /**
      * The pointer for the item actual
      */
      stItemRegionDesc * Actual;

      /**
      * The index of the item actual
      */
      int IdActual;

      /**
      * The number of Items
      */
      int NumberItem;
};//end stPartitionListRegionDesc

//-----------------------------------------------------------------------------
// Class stPartitionNode
//-----------------------------------------------------------------------------
/**
* This abstract class is the basic for the bucket of Partition. All
* classes that implement the bucket of Partition must extend this
* class.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @see stPartitionIndexBucket
* @see stPartitionLeafBucket
* @see stPartitionRepresentativeNode
* @ingroup partition
*/
class stPartitionNode{
   public:
      /**
      * Bucket type.
      */
      enum stPartitionNodeType{
         /**
         * ID of an index bucket.
         */
         INDEX = 0x4449, // In little endian "ID"

         /**
         * ID of a leaf bucket.
         */
         LEAF = 0x464C, // In little endian "LF"

         /**
         * ID of a representatives object node.
         */
         REPRESENT = 0x5052 // In little endian "RP"

      };//end stPartitionNodeType

      /**
      * This method will dispose this instance and all associated resources.
      */
      virtual ~stPartitionNode(){
      }//end ~stPartitionNode()

      /**
      * Returns the associated page.
      *
      * @return The associated page.
      */
      stPage * GetPage(){
         return Page;
      }//end GetPage

      /**
      * Returns the ID of the associated page.
      *
      * @return The ID of the associated page.
      */
      u_int32_t GetPageID(){
         return Page->GetPageID();
      }//end GetPage

      /**
      * Returns the type of the bucket of Partition (Leaf or Index).
      *
      * @return the type of bucket.
      * @see stPartitionNodeType
      */
      stPartitionNodeType GetBucketType(){
         return Header->Type;
      }//end GetBucketType

      /**
      * Returns the number of Entries in this bucket
      *
      * @return the number of Entries.
      */
      int GetNumberOfEntries(){
         return Header->Occupation;
      }//end GetNumberOfEntries

      /**
      * Returns the PageId of next bucket.
      *
      * @return The PageId of next bucket.
      */
      u_int32_t GetNextBucket(){
         return Header->NextBucket;
      }//end GetNextBucket

      /**
      * Change the PageId of next bucket.
      *
      * @param The PageId of next bucket.
      */
      u_int32_t SetNextBucket(u_int32_t next){
         Header->NextBucket = next;
      }//end SetNextBucket

      /**
      * Remove the n-last entries in the node.
      *
      * @param number The n-last entries that will be removed.
      */
      void RemoveNLast(int number){
         Header->Occupation = Header->Occupation - number;
      }

      /**
      * Remove all entries in the node.
      */
      void RemoveAll(){
         Header->Occupation = 0;
      }

      /**
      * This is a virtual method that defines a interface for the instantiate
      * the correct specialization of this class.
      *
      * @param page The instance of stPage.
      */
      static stPartitionNode *CreateBucket(stPage * page);

   protected:
      /**
      * This is the structure of the Header of a bucket of Partition.
      */
      #pragma pack(1)
      struct stPartitionNodeHeader{
         /**
         * Bucket type.
         */
         stPartitionNodeType Type;

         /**
         * Number of entries.
         */
         u_int32_t Occupation;

         /**
         * Next Bucket
         */
         u_int32_t NextBucket;
      }; //end stPartitionNodeHeader
      #pragma pack()

      /**
      * Header of this page.
      */
      stPartitionNodeHeader * Header;

      /**
      * The page related with this class.
      */
      stPage * Page;

      /**
      * Creates a new instance of this class.
      *
      * @param page An instance of stPage.
      */
      stPartitionNode(stPage * page){
         Page = page;
         Header = (stPartitionNodeHeader *)Page->GetData();
      }//end stPartitionNode
};//end stPartitionNode

//-----------------------------------------------------------------------------
// Class stPartitionIndexBucket
//-----------------------------------------------------------------------------
/**
* This class implements the index bucket of the Partition.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @see stPartitionNode
* @see stPartitionLeafBucket
* @see stPartitionRepresentativeNode
* @ingroup partition
*/
// +-----------------------------------------------------...
// | Type | Occupation | NextBucket | NumberRep0 | Region0 | PageID0 |...
// +-----------------------------------------------------...
// ...--------------------------------------------------+
// ...| NumberRepn | Regionn | PageIDn |<--blankspace-->|
// ...--------------------------------------------------+
class stPartitionIndexBucket: public stPartitionNode{
   public:
      /**
      * This type represents a entry in the index bucket of Partition.
      */
      #pragma pack(1)
      struct stPartitionIndexEntry{
         /**
         * The region descriptor.
         */
         stPartitionRegionDesc Region;

         /**
         * ID of the page.
         */
         u_int32_t PageID;

      }; //end stPartitionIndexEntry
      #pragma pack()

      /**
      * Creates a new instance of this class. The parameter <i>page</i> is an
      * instance of stPage that hold the bucket data.
      *
      * <P>The parameter <i>create</i> tells to stIndexPage what operation will
      * be performed. True means that the page will be initialized and false
      * means that the page will be used as it is. The default value is false.
      *
      * @param page The page that hold the data of this bucket.
      * @param create The operation to be performed.
      */
      stPartitionIndexBucket(stPage * page, bool create = false);

      /**
      * This is a method return the value of region descriptor.
      *
      * @param id  The position in the vector
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The region descriptor
      *
      */
      stPartitionRegionDesc GetRegionDesc(int id){
         #ifdef __stDEBUG__
         if ((id < 0) && (id >= GetNumberOfEntries())){
            throw std::logic_error("id value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[id].Region;
      } // end GetRegionDesc

      /**
      * This is a method change the region descriptor value of a entry.
      *
      * @param global The Representatives globals used in the comparation.
      * @param id  The position in the vector
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @param reg The region descriptor
      */
      template <class ObjectType, class EvaluatorType>
      void SetRegionDesc(tmpl_stPartitionGlobalRep * global, int id,
         stPartitionRegionDesc reg);

      /**
      * This is a method return the PageId of entry.
      *
      * @param id  The position in the vector
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The region descriptor
      *
      */
      u_int32_t GetEntryPageId(int id){
         #ifdef __stDEBUG__
         if ((id < 0) && (id >= GetNumberOfEntries())){
            throw std::logic_error("id value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[id].PageID;
      } // end GetEntryPageId

      /**
      * This is a method change the PageId value of a entry.
      *
      * @param id  The position in the vector
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @param page The page id
      *
      */
      void SetEntryPageId(int id, u_int32_t page){
         #ifdef __stDEBUG__
         if ((id < 0) && (id >= GetNumberOfEntries())){
            throw std::logic_error("id value is out of range.");
         }//end if
         #endif //__stDEBUG__

         Entries[id].PageID = page;
      } // end SetEntryPageId

      /**
      * This is a method insert of a entry in the bucket and return the
      * position its position in the vector of entries. The algorithm is
      * the insert sort.
      *
      * @param global The Representatives globals used in the comparation.
      * @param region The region descriptor.
      * @param pageid The identification of Page.
      * @return The position in the vector Entries.
      */
      template <class ObjectType, class EvaluatorType>
      int AddEntry(tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc region, u_int32_t pageid);

      /**
      * Return the index of entry where the region descriptors must be found.
      *
      * @todo Change the sequencial search for binary search.
      * @param global The Representatives globals used in the comparation.
      * @param findRegion The region descriptor that will be find
      * @param num The number of representatives of the hash function.
      * @warning The number of entries is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The index of entry where the region descriptors must be found.
      */
      template <class ObjectType, class EvaluatorType>
      void ChoiceSubBucket(tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc findRegion, int & idx,
         stPartitionRegionDesc & begin, stPartitionRegionDesc & end);

      /**
      * This method create a copy of bucket from target bucket.
      *
      * @param global The Representatives globals used in the comparation.
      * @param targetBucket The target bucket.
      */
      template <class ObjectType, class EvaluatorType>
      void CopyFromBucket(tmpl_stPartitionGlobalRep * global,
         stPartitionIndexBucket * targetBucket);

   private:
      /**
      * Entry pointer
      */
      stPartitionIndexEntry * Entries;

      /**
      * Returns the amount of the free space in this bucket.
      */
      u_int32_t GetFree();
};//end stPartitionIndexBucket

//-----------------------------------------------------------------------------
// Class stPartitionLeafBucket
//-----------------------------------------------------------------------------
/**
* This class implements the leaf bucket of the Partition.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @todo Documentation review.
* @see stPartitionNode
* @see stPartitionLeafBucket
* @see stPartitionRepresentativeNode
* @ingroup partition
*/
// +------------------------------------------------------------------...
// | Type | Occupation | NextBucket | NumberRep0 | Region0 | OffSet0 |...
// +------------------------------------------------------------------...
// ...--------------------------------------------------------------------+
// ...| NumberRepn | Regionn | OffSetn |<--blankspace-->| Objn |...| Obj0 |
// ...--------------------------------------------------------------------+
class stPartitionLeafBucket: public stPartitionNode{
   public:
      /**
      * This type represents a entry in the leaf bucket of Partition.
      */
      #pragma pack(1)
      struct stPartitionLeafEntry{
         /**
         * The region descriptor
         */
         stPartitionRegionDesc Region;

         /**
         * Offset of the object.
         * @warning NEVER MODIFY THIS FIELD. YOU MAY DAMAGE THE STRUCTURE OF
         * THIS BUCKET.
         */
         u_int32_t Offset;
      }; //end stPartitionLeafEntry
      #pragma pack()

      /**
      * Creates a new instance of this class. The paramenter <i>page</i> is an
      * instance of stPage that hold the bucket data.
      *
      * <P>The parameter <i>create</i> tells to stLeafPage what operation will
      * be performed. True means that the page will be initialized and false
      * that the page will be used as it is. The default value is false.
      *
      * @param page The page that hold the data of this bucket.
      * @param create The operation to be performed.
      */
      stPartitionLeafBucket(stPage * page, bool create = false);

      /**
      * This is a method return the value of region descriptor.
      *
      * @param id  The position in the vector
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The region descriptor
      *
      */
      stPartitionRegionDesc GetRegionDesc(int id){
         #ifdef __stDEBUG__
         if ((id < 0) && (id >= GetNumberOfEntries())){
            throw std::logic_error("id value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[id].Region;
      } // end GetRegion

      /**
      * This is a method change the region descriptor value of a entry.
      *
      * @param global The Representatives globals used in the comparation.
      * @param id  The position in the vector
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @param reg The region descriptor
      */
      template <class ObjectType, class EvaluatorType>
      void SetRegionDesc(tmpl_stPartitionGlobalRep * global, int id,
         stPartitionRegionDesc reg);

      /**
      * This is a method insert of a entry in the bucket and return the
      * position its position in the vector of entries. The algorithm is
      * the insert sort.
      *
      * @param global The Representatives globals used in the comparation.
      * @param region The region descriptor.
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @warning The parameter size is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The position in the vector Entries.
      */
      template <class ObjectType, class EvaluatorType>
      int AddEntry(tmpl_stPartitionGlobalRep * global,
         stPartitionRegionDesc region, u_int32_t size, const unsigned char * object);

      /**
      * Gets the serialized object. Use GetObjectSize() to determine the size of
      * the object.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return A pointer to the serialized object.
      * @see GetObjectSize()
      */
      const unsigned char * GetObject(int id){
         #ifdef __stDEBUG__
         if ((id < 0) && (id >= GetNumberOfEntries())){
            throw std::logic_error("id value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Page->GetData() + Entries[id].Offset;
      }//end GetObject()

      /**
      * Returns the size of the object. Use GetObject() to get the object data.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The size of the serialized object.
      * @see GetObject()
      */
      u_int32_t GetObjectSize(int id);

      /**
      * This method create a copy of bucket from target bucket.
      *
      * @param global The Representatives globals used in the comparation.
      * @param targetBucket The target bucket.
      */
      template <class ObjectType, class EvaluatorType>
      void CopyFromBucket(tmpl_stPartitionGlobalRep * global,
         stPartitionLeafBucket * targetBucket);

   private:
      /**
      * Entry pointer
      */
      stPartitionLeafEntry * Entries;

      /**
      * Returns the amount of the free space in this bucket.
      */
      u_int32_t GetFree();
};//end stPartitionLeafBucket

//-----------------------------------------------------------------------------
// Class stPartitionRepresentativesNode
//-----------------------------------------------------------------------------
/**
* This class implement the block that store the representives objects for the
* hash function of Partition.
*
* @author Enzo Seraphim(seraphim@icmc.usp.br)
* @version 1.0
* @see stPartitionNode
* @see stPartitionLeafBucket
* @see stPartitionIndexBucket
* @todo Documentation review.
* @ingroup partition
*/
// +------------------------------------------------------------------...
// | Type | Occupation | NextBlock | OffSet0 | OffSet1 |...| OffSetn |...
// +------------------------------------------------------------------...
// ...-----------------------------------------+
// ...<--blankspace-->| Objn |...| Obj1 | Obj0 |
// ...----------------------------------+

class stPartitionRepresentativesNode: public stPartitionNode{
   public:
      /**
      * Creates a new instance of this class. The parameter <i>page</i> is an
      * instance of stPage that hold the bucket data.
      *
      * <P>The parameter <i>create</i> tells to stPartitionRepresentativesNode
      * what operation will be performed. True means that the page will be
      * initialized and false means that the page will be used as it is.
      * The default value is false.
      *
      * @param page The page that hold the data of this bucket.
      * @param create The operation to be performed.
      */
      stPartitionRepresentativesNode(stPage * page, bool create = false);

      /**
      * This is a method insert of a entry in the block and return the
      * position its position in the vector of entries.
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @warning The parameter size is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The position in the vector Entries.
      */
      int AddEntry(u_int32_t size, const unsigned char * object);

      /**
      * Gets the serialized object. Use GetObjectSize() to determine the size of
      * the object.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return A pointer to the serialized object.
      * @see GetObjectSize()
      */
      const unsigned char * GetObject(int id){
         #ifdef __stDEBUG__
         if ((id < 0) && (id >= GetNumberOfEntries())){
            throw std::logic_error("id value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Page->GetData() + Entries[id];
      }

      /**
      * Returns the size of the object. Use GetObject() to get the object data.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The size of the serialized object.
      * @see GetObject()
      */
      u_int32_t GetObjectSize(int id);

   private:
      /**
      * Entry pointer
      */
      u_int32_t * Entries;

      /**
      * Returns the amount of the free space in this bucket.
      */
      u_int32_t GetFree();
};//end stPartitionRepresentativesNode

//#include "stPartitionNode-inl.h"

#endif // __STPARTITIONNODE_H_
