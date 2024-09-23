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
* This file defines the DBMTree nodes.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Review of documentation.
*/

#ifndef __STDBMNODE_H
#define __STDBMNODE_H

#undef __stDBMHEIGHT__
#undef __stDBMNENTRIES__

#include <arboretum/stPage.h>

#include <stdexcept>

//-----------------------------------------------------------------------------
// Class stDBMNode
//-----------------------------------------------------------------------------

/**
* This class implements the node of the DBMTree.
*
* <P>The DBMTree node...
*
* <P>The structure of DBNode follows:
* @image html dbnode.png "node structure"
*
* <P>The <b>Header</b> holds the information about the node itself.
*     - Occupation: Number of entries in this node.
*
* <P>The <b>Entry</b> holds the information of the link to the other node.
*  - PageID: The identifier of the page which holds the root of the sub tree.
*       - Distance: The distance of this object from the representative object.
*       - NEntries: Number of objects in the sub tree.
*       - Radius: Radius of the sub tree.
*       - Offset: The offset of the object in the page. DO NOT MODIFY ITS VALUE.
*
* <P>The <b>Object</b> is an array of bytes that holds the information required to rebuild
* the original object.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation review.
* @ingroup db
*/
// +--------------------------------------------------------------------------------------------------------+
// | Occupation | PgID0 | Dist0 | OffSet0 |...|PgIDn | Distn | OffSetn | <-- blankspace --> |Objn |...|Obj0 |
// +--------------------------------------------------------------------------------------------------------+
class stDBMNode{

   public:

      #pragma pack(1)
      /**
      * This type represents a DBMTree node entry.
      */
      typedef struct stDBMEntry{
         /**
         * ID of the page.
         */
         u_int32_t PageID;

         /**
         * Distance from the representative.
         */
         double Distance;

         /**
         * Height of the subtree.
         */
         #ifdef __stDBMHEIGHT__
            unsigned char Height;
         #endif //__stDBMHEIGHT__

         /**
         * Number of entries in the sub-tree. This attribute is stored with the object
         * when it is compilided with this option.
         */
         //u_int32_t NEntries;

         /**
         * Radius of the sub-tree. This attribute is stored within the object. 
         */
         //double Radius;

         /**
         * Offset of the object.
         * @warning NEVER MODIFY THIS FIELD. YOU MAY DAMAGE THE STRUCTURE OF
         * THIS NODE.
         */
         u_int32_t Offset;
      }stDBMEntry; //end stDBMEntry
      #pragma pack()

      /**
      * Creates a new instance of this class. The parameter <i>page</i> is an
      * instance of stPage that hold the node data.
      *
      * <P>The parameter <i>create</i> tells to stPage what operation will
      * be performed. True means that the page will be initialized and false
      * means that the page will be used as it is. The default value is false.
      *
      * @param page The page that hold the data of this node.
      * @param create The operation to be performed.
      */
      stDBMNode(stPage * page, bool create = false){
         this->Page = page;
         this->Header = (stDBMNodeHeader *)(this->Page->GetData());

         // Attention to this manouver! It is the brain of this
         // implementation.
         this->Entries = (stDBMEntry *)(page->GetData() + sizeof(stDBMNodeHeader));

         // Initialize page
         if (create){
            #ifdef __stDEBUG__
            Page->Clear();
            #endif //__stDEBUG__
            this->Header->Occupation = 0;
         }//end if
      }//end stDBMNode

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
      * Returns the the Maximum Height of the associated page.
      *
      * @return The Maximum Heigh of subTrees in this node.
      */
      unsigned char GetHeight();

      /**
      * Returns the number of entries in this node.
      *
      * @return the number of entries.
      * @see GetEntry()
      * @see GetObject()
      * @see GetObjectSize()
      */
      u_int32_t GetNumberOfEntries(){
         return this->Header->Occupation;
      }//end GetNumberOfEntries

      /**
      * Returns the reference of the desired entry. You may use this method to
      * read and modify the entry information.
      *
      * @param idx The idx of the entry.
      * @warning The parameter idx is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return stEntry the reference of the desired entry.
      * @see GetNumberOfEntries()
      * @see GetObject()
      * @see GetObjectSize()
      */
      stDBMEntry & GetEntry(u_int32_t idx){
         #ifdef __stDEBUG__
         if (idx >= GetNumberOfEntries()){
            throw std::logic_error("idx value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[idx];
      }//end GetEntry

      /**
      * Returns the number of entries that this subtree has.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @return u_int32_t the number of entries in the subtree.
      */
      u_int32_t GetNEntries(u_int32_t idx);

      /**
      * Sets the number of entries that this subtree has.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @param u_int32_t The new value.
      */
      void SetNEntries(u_int32_t idx, u_int32_t NEntries);

      /**
      * Returns the radius of the subtree.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @return double the radius of this subtree.
      */
      double GetRadius(u_int32_t idx);

      /**
      * Sets the radius of the subtree.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @param double The new radius.
      */
      void SetRadius(u_int32_t idx, double radius);

      /**
      * Adds a new entry to this node. This method will return the idx of the new
      * node or a negative value for failure.
      *
      * <P>This method will fail if there is not enough space to hold the
      * new object.
      *
      * <P>If you have added a new entry successfully, you may edit the entry
      * fields using the method GetEntry().
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @warning The parameter size is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The position in the vector Entries.
      * @see RemoveEntry()
      * @see GetEntry()
      */
      int AddEntry(u_int32_t size, const unsigned char * object, u_int32_t subTree);

      /**
      * Returns the entry idx that hold the representaive object.
      */
      int GetRepresentativeIndex();

      /**
      * Gets the serialized object. Use GetObjectSize to determine the size of
      * the object.
      *
      * @param idx The idx of the entry.
      * @warning The parameter idx is not verified by this implementation.
      * unless __stDEBUG__ is defined at compile time.
      * @return A pointer to the serialized object.
      * @see GetObjectSize()
      */
      const unsigned char * GetObject(u_int32_t idx);

      /**
      * Returns the size of the object. Use GetObject() to get the object data.
      *
      * @param idx The idx of the entry.
      * @warning The parameter idx is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The size of the serialized object.
      * @see GetObject()
      */
      u_int32_t GetObjectSize(u_int32_t idx);

      /**
      * Removes an entry from this object.
      *
      * @param idx The idx of the entry.
      * @warning The parameter idx is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @see GetObjectSize()
      */
      void RemoveEntry(int idx);

      /**
      * Remove All entries.
      */
      void RemoveAll(){
         #ifdef __stDEBUG__
         Page->Clear();
         #else
         this->Header->Occupation = 0;
         #endif //__stDEBUG__
      }//end RemoveAll

      /**
      * Returns the minimum radius of this node.
      */
      double GetMinimumRadius();

      /**
      * Returns the total number of objects in the subtree.
      */
      u_int32_t GetTotalObjectCount();

      /**
      * Returns the index of the farthest object in this node.
      */
      u_int32_t GetFarthestObject();

      /**
      * Returns the number of free objects in this node.
      */
      u_int32_t GetNumberOfFreeObjects();

      /**
      * Returns the amount of the free space in this node.
      */
      u_int32_t GetFree(){
         return Page->GetPageSize() - this->GetUsed();      
      }//end GetFree()

      /**
      * Returns the amount of used space.
      */
      u_int32_t GetUsed();

      /**
      * Returns the global overhead of a DBnode (header size) in bytes.
      */
      static u_int32_t GetGlobalOverhead(){
         return sizeof(stDBMNodeHeader);
      }//end GetGlobalOverhead()

      /**
      * Returns the overhead of each node entry in bytes.
      */
      static u_int32_t GetEntryOverhead(){
         return sizeof(stDBMEntry);
      }//end GetIndexEntryOverhead()

   private:

      /**
      * This is the structure of the Header of a DBMTree node.
      */
      #pragma pack(1)
      typedef struct stDBMNodeHeader{

         /**
         * Number of entries.
         */
         u_int32_t Occupation;
      } stDBMNodeHeader; //end stHeader
      #pragma pack()

      /**
      * Header of this page.
      */
      stDBMNodeHeader * Header;

      /**
      * The page related with this class.
      */
      stPage * Page;

      /**
      * Entry pointer
      */
      stDBMEntry * Entries;

};//end stDBMPage


//-----------------------------------------------------------------------------
// Class stDBMMemNode
//-----------------------------------------------------------------------------
/**
* This class template implements a memory shell for a stDBMNode instance.
* It implements a memory optimized node wich simulates the physical
* occupation of the original node.
*
* <p>Since all objects are stored using their implementation (not the serialized
* form), this class is very useful to optimize heavy object manipulations such
* as MinMax algorithms.
*
* <p>By the other side, this class will unserialize all objects of the node
* without exceptions. Due to that, the use of this class is not recommended
* for procedures with low rate of object unserializations/serializations.
* Another disadvantage of this class is the potential high memory usage.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @see stDBMNode
* @ingroup db
*/
template < class ObjectType >
class stDBMMemNode{
   public:

		/**
		* This struct holds all information required to store a node entry.
		*/
      struct stDBMMemNodeEntry{
         /**
         * Object.
         */
         ObjectType * Object;

         /**
         * ID of the page.
         */
         u_int32_t PageID;

         /**
         * Distance from representative.
         */
         double Distance;

         /**
         * Height of the subtree.
         */
         unsigned char Height;

         /**
         * Number of entries in the sub-tree.
         */
         u_int32_t NEntries;

         /**
         * Radius of the sub-tree.
         */
         double Radius;
      };

      /**
      * Creates a new instance of this class. The parameter <i>page</i> is an
      * instance of stPage that hold the node data.
      *
      * <P>The parameter <i>create</i> tells to stPage what operation will
      * be performed. True means that the page will be initialized and false
      * means that the page will be used as it is. The default value is false.
      *
      * @param page The page that hold the data of this node.
      * @param create The operation to be performed.
      */
      stDBMMemNode(u_int32_t pageSize, u_int32_t numberOfEntries = 32);

      /**
      * Release all resources associate.
      */
      ~stDBMMemNode(){
         this->RemoveAll();
         delete[] Entries;
      }//end ~stDBMMemNode()

      /**
      * Returns the number of entries in this node.
      *
      * @return the number of entries.
      */
      u_int32_t GetNumberOfEntries(){
         return this->NumEntries;
      }//end GetNumberOfEntries

      /**
      * Returns the reference of the desired entry. You may use this method to
      * read and modify the entry information.
      *
      * @param idx The idx of the entry.
      * @warning The parameter idx is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return stEntry the reference of the desired entry.
      * @see GetNumberOfEntries()
      * @see GetObject()
      * @see GetObjectSize()
      */
      stDBMMemNodeEntry & GetEntry(u_int32_t idx){
         #ifdef __stDEBUG__
         if (idx >= GetNumberOfEntries()){
            throw std::logic_error("idx value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[idx];
      }//end GetEntry

      /**
      * Returns the number of entries that this subtree has.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @return u_int32_t the number of entries in the subtree.
      */
      u_int32_t GetNEntries(u_int32_t idx){
         #ifdef __stDEBUG__
         if (idx >= GetNumberOfEntries()){
            throw std::logic_error("idx value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[idx].NEntries;
      }//end GetNEntries
 
      /**
      * Sets the number of entries that this subtree has.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @param u_int32_t The new value.
      */
      void SetNEntries(u_int32_t idx, u_int32_t NEntries){
         #ifdef __stDEBUG__
         if (idx >= GetNumberOfEntries()){
            throw std::logic_error("idx value is out of range.");
         }//end if
         #endif //__stDEBUG__

         Entries[idx].NEntries = NEntries;
      }//end SetNEntries

      /**
      * Returns the radius of the subtree.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @return double the radius of this subtree.
      */
      double GetRadius(u_int32_t idx){
         #ifdef __stDEBUG__
         if (idx >= GetNumberOfEntries()){
            throw std::logic_error("idx value is out of range.");
         }//end if
         #endif //__stDEBUG__

         return Entries[idx].Radius;
      }//end GetRadius

      /**
      * Sets the radius of the subtree.
      *
      * <P>This parameter does not make sense if the object is not a representative
      * object.
      * @param idx The idx of the entry.
      * @param double The new radius.
      */
      void SetRadius(u_int32_t idx, double radius){
         #ifdef __stDEBUG__
         if (idx >= GetNumberOfEntries()){
            throw std::logic_error("idx value is out of range.");
         }//end if
         #endif //__stDEBUG__

         Entries[idx].Radius = radius;
      }//end GetRadius

      /**
      * Adds a new entry to this node. This method will return the idx of the new
      * node or a negative value for failure.
      *
      * <P>This method will fail if there is not enough space to hold the
      * new object.
      *
      * <P>If you have added a new entry successfully, you may edit the entry
      * fields using the method GetEntry().
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @warning The parameter size is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The position in the vector Entries.
      * @see RemoveEntry()
      * @see GetEntry()
      */
      int AddEntry(u_int32_t size, ObjectType * object, u_int32_t subTree);

      /**
      * Sets the fields of the Entry in idx location.
      */
      void SetEntry(u_int32_t idx, double distance
                    #ifdef __stDBMNENTRIES__
                       , u_int32_t nEntries
                    #endif //__stDBMNENTRIES__
                       , double radius
                    #ifdef __stDBMHEIGHT__
                       , unsigned char height
                    #endif //__stDBMHEIGHT__
                    ){
         if (idx < this->NumEntries){
            Entries[idx].Distance = distance;
            #ifdef __stDBMNENTRIES__
               Entries[idx].NEntries = nEntries;
            #endif //__stDBMNENTRIES__
            Entries[idx].Radius = radius;
            #ifdef __stDBMHEIGHT__
               Entries[idx].Height = height;
            #endif //__stDBMHEIGHT__
         }else{
            throw std::logic_error("idx value is out of range.");
         }//end if
      }//end SetEntry

      /**
      * Removes an entry from this object.
      *
      * @param idx The idx of the entry.
      * @warning The parameter idx is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @see GetObjectSize()
      */
      void RemoveEntry(u_int32_t idx);

      /**
      * Remove All entries.
      */
      void RemoveAll();

      /**
      * Returns the minimum radius of this node.
      */
      double GetMinimumRadius();

      /**
      * Returns the amount of the free space in this node.
      */
      u_int32_t GetFree(){
         return (this->MaximumSize - this->UsedSize);
      }//end GetFree

      /**
      * Returns the amount of the space used.
      */
      u_int32_t GetUsed();

   private:

      /**
      * Number of entries in this node.
      */
      u_int32_t NumEntries;

      /**
      * Current capacity of this node.
      */
      u_int32_t Capacity;

      /**
      * Entries of this node.
      */
      stDBMMemNodeEntry * Entries;
      
      /**
      * Maximum size of the node in bytes.
      */
		u_int32_t MaximumSize;

		/**
		* Used size of the node in bytes.
		*/
		u_int32_t UsedSize;
		
		/**
		* Resizes the entries vector to hold more entries. It will at 16
		* positions to the capacity for each call.
		*/
		void Resize(u_int32_t incSize=16);

};//end stDBMMemNode


//-----------------------------------------------------------------------------
// Class stDBMShrinkNode
//-----------------------------------------------------------------------------
/**
* This class template implements a memory shell for a stDBMNode instance.
* It implements a memory optimized node wich simulates the physical
* occupation of the original node.
*
* <p>Since all objects are stored using their implementation (not the serialized
* form), this class is very useful to optimize heavy object manipulations such
* as Shrink and MinMax algorithms.
*
* <p>By the other side, this class will unserialize all objects of the node
* without exceptions. Due to that, the use of this class is not recommended
* for procedures with low rate of object unserializations/serializations.
* Another disadvantage of this class is the potential high memory usage.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @see stDBMNode
* @ingroup dbm
*/
template < class ObjectType >
class stDBMShrinkNode{
   public:

      /**
      * Creates a new stSlimMemLeafNode instance from a stDBMNode.
      *
      * @param node The stDBMNode to be insert.
      * @warning This method will remove entries from node.
      */
      stDBMShrinkNode(stDBMNode * node);
		
      /**
      * Creates a new stSlimMemLeafNode instance from a single object.
      *
      * @param object The object to be insert.
      * @warning This method will share the object.
      */
      stDBMShrinkNode(ObjectType * object);

      /**
      * Release the resources of this class template and construct a new
      * stDBMNode that has all entries in this instance.
      *
      * @note The resulting node will have all objects sorted according to
      * their distance from the representative object (in ascendent order).
      * @return A new instance of stDBMNode.
      */
      stDBMNode * ReleaseNode();

      /**
      * Release the resources of this class template.
      */
      void Release();

      /**
      * Adds an object to this node.
      *
      * <p>All entries are sorted in crescent order of distance to allow complex
      * object manipulation such as Shrink.
      *
      * @param obj An object to be insert.
      * @param distance A correspondent distance of the new entry.
      * @return True for success or false otherwise.
      */
	   bool Add(ObjectType * obj, double distance, u_int32_t pageID, double radius
               #ifdef __stDBMNENTRIES__
                  , u_int32_t nEntries
               #endif //__stDBMNENTRIES__
               #ifdef __stDBMHEIGHT__
                  , unsigned char height
               #endif //__stDBMHEIGHT__
               );

      /**
      * Returns the number of entries.
      */
      u_int32_t GetNumberOfEntries(){
         return this->numEntries;
      }//end GetNumberOfObjects

      /**
      * Returns the object at position idx.
      *
      * @param idx Index of the object.
      * @return An object at position idx.
      * @warning Do not modify/dispose the object using this method.
      */
      ObjectType * ObjectAt(u_int32_t idx){
         #ifdef __stDEBUG__
            if (idx >= this->numEntries){
               throw std::logic_error("idx value is out of range.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[idx].Object;
      }//end ObjectAt

      /**
      * Returns a pointer to the last object of the node. This object is special
      * because it has the largest distance from the representative.
      *
      * @return A pointer to the last object of this node.
      */
      ObjectType * LastObject(){
         #ifdef __stDEBUG__
            if (!this->numEntries){
               throw std::logic_error("there is not a last object.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[this->numEntries-1].Object;
      }//end LastObject

      /**
      * Returns the pageID of the last entry of the node.
      */
      u_int32_t LastPageID(){
         #ifdef __stDEBUG__
            if (!this->numEntries){
               throw std::logic_error("there is not a last pageID.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[this->numEntries-1].PageID;
      }//end LastPageID

      /**
      * Returns the radius of the last entry of the node.
      */
      double LastRadius(){
         #ifdef __stDEBUG__
            if (!this->numEntries){
               throw std::logic_error("there is not a last radius.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[this->numEntries-1].Radius;
      }//end LastRadius

      /**
      * Returns the distance of the last object from the 
      * representative of this node.
      */
      double Ladouble(){
         #ifdef __stDEBUG__
            if (!this->numEntries){
               throw std::logic_error("there is not a last distance.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[this->numEntries-1].Distance;
      }//end Ladouble

      /**
      * Returns the height of the last entry.
      */
      unsigned char LastHeight(){
         #ifdef __stDEBUG__
            if (!this->numEntries){
               throw std::logic_error("there is not a last height.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[this->numEntries-1].Height;
      }//end LastHeight

      /**
      * Returns the NEntries of the last entry.
      */
      u_int32_t LastNEntries(){
         #ifdef __stDEBUG__
            if (!this->numEntries){
               throw std::logic_error("there is not a last nEntries.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[this->numEntries-1].NEntries;
      }//end LastNEntries

      /**
      * Returns a pointer to the representative object of the node.
      */
      ObjectType * RepObject();

      /**
      * Returns the distance associated with a given entry.
      *
      * @param idx Index of the object.
      * @return The distance.
      */
      double DistanceAt(u_int32_t idx){
         #ifdef __stDEBUG__
            if (idx >= this->numEntries){
               throw std::logic_error("idx value is out of range.");
            }//end if
         #endif //__stDEBUG__

         return Entries[idx].Distance;
      }//end DistanceAt

      /**
      * Returns the pageID associated with a given entry.
      *
      * @param idx Index of the object.
      * @return The pageID.
      */
      u_int32_t PageIDAt(u_int32_t idx){
         #ifdef __stDEBUG__
            if (idx >= this->numEntries){
               throw std::logic_error("idx value is out of range.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[idx].PageID;
      }//end PageIDAt

      /**
      * Returns the radius associated with a given entry.
      *
      * @param idx Index of the object.
      * @return The radius.
      */
      double RadiusAt(u_int32_t idx){
         #ifdef __stDEBUG__
            if (idx >= this->numEntries){
               throw std::logic_error("idx value is out of range.");
            }//end if
   	   #endif //__stDEBUG__

         return Entries[idx].Radius;
      }//end RadiusAt

      #ifdef __stDBMNENTRIES__
         /**
         * Returns the NEntries associated with a given entry.
         *
         * @param idx Index of the object.
         * @return The NEntries.
         */
         u_int32_t NEntriesAt(u_int32_t idx){
            #ifdef __stDEBUG__
               if (idx >= this->numEntries){
                  throw std::logic_error("idx value is out of range.");
               }//end if
            #endif //__stDEBUG__

            return Entries[idx].NEntries;
         }//end NEntriesAt
      #endif //__stDBMNENTRIES__

      /**
      * Returns the height associated with a given entry.
      *
      * @param idx Index of the object.
      * @return The height.
      */
      #ifdef __stDBMHEIGHT__
         unsigned char HeightAt(u_int32_t idx){
            #ifdef __stDEBUG__
               if (idx >= this->numEntries){
                  throw std::logic_error("idx value is out of range.");
               }//end if
            #endif //__stDEBUG__

            return Entries[idx].Height;
         }//end HeightAt
      #endif //__stDBMHEIGHT__

      /**
      * Removes an entry in idx from this node.
      *
      * @param idx Index of the object.
      * @return The removed object.
      */
      ObjectType * Remove(u_int32_t idx);

      /**
      * Remove the last object from this tree.
      */
      ObjectType * PopObject();

      /**
      * Returns true if there is enough free space to add the given object.
      *
      * @return True for success or false otherwise.
      */
      bool CanAdd(ObjectType * obj, u_int32_t pageID){
         u_int32_t entrySize;

         // Does it fit ?
         entrySize = obj->GetSerializedSize() + stDBMNode::GetEntryOverhead();
         // for subtrees. 
         if (pageID){
            entrySize += sizeof(double);  // Add the radius
            #ifdef __stDBMNENTRIES__
               entrySize += sizeof(u_int32_t);   // Add the NEntries
            #endif //__stDBMNENTRIES__
         }//end if
         if (entrySize + this->usedSize > this->maximumSize){
            // No, it doesn't.
            return false;
         }//end if
         // yes, it does.
         return true;
      }//end CanAdd

      /**
      * Returns the minimum radius of this node.
      *
      * @return The radius.
      */
      double GetMinimumRadius();

      /**
      * Returns the free space of this node.
      *
      * @return The free space.
      */
      u_int32_t GetFreeSize(){
         return (this->maximumSize - this->usedSize);
      }//end GetFreeSize

   private:
      /**
      * This struct holds all information required to store a node entry.
      */
      struct stDBMShrinkNodeEntry{
         /**
         * Object.
         */
         ObjectType * Object;

         /**
         * Distance from representative.
         */
         double Distance;

         /**
         * ID of the page.
         */
         u_int32_t PageID;

         /**
         * Height of the subtree.
         */
         #ifdef __stDBMHEIGHT__
            unsigned char Height;
         #endif //__stDBMHEIGHT__

         /**
         * Number of entries in the sub-tree. This attribute is stored with the object
         * when it is compilided with this option.
         */
         #ifdef __stDBMNENTRIES__
            u_int32_t NEntries;
         #endif //__stDBMNENTRIES__

         /**
         * Radius of the sub-tree. This attribute is stored within the object.
         */
         double Radius;
      };

      /**
      * Number of entries in this node.
      */
      u_int32_t numEntries;

      /**
      * Current capacity of this node.
      */
      u_int32_t capacity;
      
      /**
      * Entries of this node.
      */
      stDBMShrinkNodeEntry * Entries;
      
      /**
      * Maximum size of the node in bytes.
      */
      u_int32_t maximumSize;

      /**
      * Used size of the node in bytes.
      */
      u_int32_t usedSize;
		
      /**
      * Point to the source leafNode.
      */
      stDBMNode * srcNode;	

      /**
      * Returns the insert position for a given distance.
      *
      * @param distance The distance of the object to be insert.
      * @return The position that will be insert.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      int InsertPosition(double distance); 
		
      /**
      * Resizes the entries vector to hold more entries. It will at 16
      * positions to the capacity for each call.
      */
      void Resize(u_int32_t incSize = 16);
      
};//end stDBMShrinkNode

// Include implementation
#include "stDBMNode-inl.h"

#endif //__STDBMNODE_H
