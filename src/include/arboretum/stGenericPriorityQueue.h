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
#ifndef __STGPRIORITYQ_H
#define __STGPRIORITYQ_H


#include <arboretum/stCommon.h>
#include <math.h>

//----------------------------------------------------------------------------
// class template stGenericEntry
//----------------------------------------------------------------------------
/**
* This class defines the node type of stGenericEntry.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup util
* @todo Review the documentation.
*/
enum tType {
   /**
   * The entry is a node.
   */
   NODE,
   /**
   * The entry is a approximate node.
   */
   APPROXIMATENODE,
   /**
   * The entry is a object.
   */
   OBJECT,
   /**
   * The entry is a approximate object.
   */
   APPROXIMATEOBJECT
};

template < class ObjectType >
class stGenericEntry {
   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * Disposes this instance and release all associated resources including
      * the result object.
      */
      ~stGenericEntry(){
         if (Mine){
            if (this->Object != NULL){
               delete this->Object;
            }//end if
         }//end if
      }//end ~stGenericEntry

      /**
      * Set the PageID of this entry if is a node.
      *
      * @param pageID the pageID.
      */
      void SetPageID(u_int32_t pageID){
         this->PageID = pageID;
      }//end SetPageID

      /**
      * Returns the PageID if it is a node.
      */
      u_int32_t GetPageID(){
         return this->PageID;
      }//end GetPageID

      /**
      * Set the Distance between this entry and the query object.
      *
      * @param distance distance of this entry.
      */
      void SetDistanceQuery(double distanceQuery){
         this->DistanceQuery = distanceQuery;
      }//end SetDistanceQuery

      /**
      * Returns the Distance between this entry and the query object.
      */
      double GetDistanceQuery(){
         return this->DistanceQuery;
      }//end GetDistanceQuery

      /**
      * Set the Distance between this entry and the representative of
      * this entry.
      *
      * @param distance distance of this entry.
      */
      void SetDistanceRep(double distanceRep){
         this->DistanceRep = distanceRep;
      }//end SetDistanceRep

      /**
      * Returns the Distance between this entry and the representative
      * of this entry.
      */
      double GetDistanceRep(){
         return this->DistanceRep;
      }//end GetDistanceRep

      /**
      * Set the Distance from the representative of this entry and the
      * query object.
      *
      * @param distance distance of this entry.
      */
      void SetDistanceRepQuery(double distanceRepQuery){
         this->DistanceRepQuery = distanceRepQuery;
      }//end SetDistanceRepQuery

      /**
      * Returns the Distance from the representative of this entry
      * and the query object.
      */
      double GetDistanceRepQuery(){
         return this->DistanceRepQuery;
      }//end GetDistanceRepQuery

      /**
      * Set the Distance used to order this entry in the list.
      *
      * @param distance distance of this entry.
      */
      void SetDistance(double distance){
         this->Distance = distance;
      }//end SetDistance

      /**
      * Returns the Distance used to order this entry in the list.
      */
      double GetDistance(){
         return this->Distance;
      }//end GetDistance

      /**
      * Set the Radius of this entry if this is a node.
      *
      * @param radius subtree radius of this entry.
      */
      void SetRadius(double radius){
         this->Radius = radius;
      }//end SetRadius

      /**
      * Returns the Radius of this entry.
      */
      double GetRadius(){
         return this->Radius;
      }//end GetRadius

      /**
      * Set the Next entry.
      *
      * @param next next entry.
      */
      void SetNext(stGenericEntry * next){
         this->Next = next;
      }//end SetNext

      /**
      * Returns the next Entry.
      */
      stGenericEntry * GetNext(){
         return this->Next;
      }//end GetNext

      /**
      * Set the type of this Entry.
      */
      void SetType(enum tType type){
         this->Type = type;
      }//end SetType

      /**
      * Returns the type of this Entry.
      */
      int GetType(){
         return this->Type;
      }//end GetType

      /**
      * This method sets the object.
      */
      void SetObject(tObject * obj){
         this->Object = obj;
      }//end SetObject

      /**
      * This method returns the object.
      */
      tObject * GetObject(){
         return this->Object;
      }//end GetObject

      /**
      * This method sets the mine.
      */
      void SetMine(bool mine){
         this->Mine = mine;
      }//end SetMine

   private:

      /**
      * The object.
      */
      tObject * Object;

      /**
      * The flag to set if this instance owns the object.
      */
      bool Mine;

      /**
      * The ID of a Page.
      */
      u_int32_t PageID;

      /**
      * The Distance between this object and the query object.
      */
      double DistanceQuery;

      /**
      * The Distance between this object and the representative object.
      */
      double DistanceRep;

      /**
      * The Distance between the object representative and the query object.
      */
      double DistanceRepQuery;

      /**
      * The Distance used to order the list.
      */
      double Distance;

      /**
      * The subtree radius of this representative.
      */
      double Radius;

      /**
      * The type of this entry.
      */
      tType Type;

      /**
      * Link for the other Entry in the List.
      */
      stGenericEntry * Next;
      
};//end stGenericEntry


//----------------------------------------------------------------------------
// class template stGenericPriorityQueue
//----------------------------------------------------------------------------
/**
* This class implements a global priority queue based on chained list used for
* the Incremental Nearest Neighbor.
*
* <p>This priority queue is implemented using a simple chained list.
* This implementation of priority queue is based on the paper
* "Incremental Similarity Search in Multimedia Databases" - G. R. Hjaltason &
*  Hanan Samet - 2000.
*
* @author Marcos Rodrigues Vieira
* @version 1.0
* @ingroup util
*/
template < class ObjectType >
class stGenericPriorityQueue{
   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;
      
      /**
      * This type defines the stGenericEntry used by this class.
      */
      typedef stGenericEntry < ObjectType > tGenericEntry;

      /**
      * This type defines the enum tType used by this class.
      */
      #if defined (__BORLANDC__)
         typedef tType tEntryType;
      #else
         typedef enum tType tEntryType;
      #endif

      /**
      * Creates a new priority queue.
      */
      stGenericPriorityQueue(){
         this->Occupation = 0;
         this->Head = NULL;
      }//end stGenericPriorityQueue

      /**
      * Disposes this queue.
      */
      ~stGenericPriorityQueue();

      /**
      * Adds a new approximate node to the queue.
      *
      * @param object The object to be inserted.
      * @param pageID The pageID to be inserted.
      * @param distanceRepQuery The distance from the representative and the query object.
      * @param distanceRep The distance from the object and the representative.
      * @param radius The radius value of the subtree to be inserted.
      * @param type The type of this entry.
      */
      #if defined(__BORLANDC__)
         void Add(tObject * object, u_int32_t pageID, double distanceRepQuery,
            double distanceRep, double radius, tType type);
      #else
         void Add(tObject * object, u_int32_t pageID, double distanceRepQuery,
            double distanceRep, double radius, tEntryType type);
      #endif

      /**
      * Adds a new node to the queue.
      *
      * @param object The object to be inserted.
      * @param pageID The pageID to be inserted.
      * @param distanceQuery The distance from this entry and the query object.
      * @param radius The radius value of the subtree to be inserted.
      * @param type The type of this entry.
      */
      void Add(tObject * object, u_int32_t pageID, double distanceQuery,
         double radius, tEntryType type);

      /**
      * Adds a new approximate object to the queue.
      *
      * @param object The object to be inserted.
      * @param distanceRep The distance from the object and the representative.
      * @param distanceRepQuery The distance from the representative and the query object.
      * @param type The type of this entry.
      */
      void Add(tObject * object, double distanceRep,
         double distanceRepQuery, tEntryType type);

      /**
      * Adds a new object to the queue.
      *
      * @param object The object to be inserted.
      * @param distanceQuery The distance from this entry and the query object.
      * @param type The type of this entry.
      */
      void Add(tObject * object, double distanceQuery, tEntryType type);

      /**
      * Gets the next node. This node is removed from the queue.
      *
      * @return The Head node for success or NULL if the queue is empty.
      */
      tGenericEntry * Get();

      /**
      * Returns the size of this queue.
      */
      u_int32_t GetSize(){
         return this->Occupation;
      }//end GetSize
      
      /**
      * Returns if this queue is empty.
      */
      bool IsEmpty(){
         return (this->Head==NULL);
      }//end IsEmpty

   private:

      /**
      * Pointer to beginning of this queue.
      */
      tGenericEntry * Head;

      /**
      * Current occupation of this queue.
      */
      u_int32_t Occupation;

};//end stGenericPriorityQueue

//----------------------------------------------------------------------------
// class template stGenericPriorityHeap
//----------------------------------------------------------------------------
/**
* This class implements a global priority queue based on a heap used for
* the Incremental Nearest Neighbor.
*
* <p>This priority queue is implemented using a dynamic heap.
* This implementation of priority queue is based on the paper
* "Incremental Similarity Search in Multimedia Databases" - G. R. Hjaltason &
*  Hanan Samet - 2000.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup util
* @todo review the documentation.
* @see stDynamicRPriorityQueue
*/
template <class ObjectType>
class stGenericPriorityHeap{

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * Creates a new reverse priority queue. The initialSize is used to
      * reserve a certain number of entries in this queue while increment
      * is the number of entries to add to the capacity when the addition of
      * a new entry overflows the current capacity (resize).
      *
      * <p>To increase the performance of this class it is necessary set
      * the paramenters initialSize and increment to optimal values which can
      * be chosen according to the nature of the problem.
      *
      * <p>Usually large values for initialSize and increment tend to enhance
      * the performance in exchange of memory usage and small values will
      * optimize the memory usage in exchange of performance.
      *
      * @param initialSize Initial capacity of this queue.
      * @param increment The number of entries to add to the capacity when
      * required.
      * @note If the value of initialSize is optimal (a number greater or equal
      * to the real maximum size of the queue, the performance of this
      * implementation will be almost as fast as stRPriorityQueue.
      */
      stGenericPriorityHeap(int initialSize, int increment = 32){
         this->Increment = increment;
         this->MaxSize = initialSize;
         this->Size = 0;
         Entries = new tGenericEntry[MaxSize];
      }//end stGenericPriorityHeap

      /**
      * Disposes this queue.
      */
      ~stGenericPriorityHeap();

      /**
      * Adds a new entry to the queue. This method will fail if the number of
      * objects exceeds the maximum number of positions.
      *
      * @param object The object to be inserted.
      * @param pageID The pageID to be inserted.
      * @param distanceQuery The distance from this entry and the query object.
      * @param distanceRep The distance from the object and the representative.
      * @param distanceRepQuery The distance from the representative and the query object.
      * @param radius The radius value of the subtree to be inserted.
      * @param height The height of this entry to be inserted.
      * @param type The type of this entry.
      */
      void Add(tObject * object, u_int32_t pageID,
               double distanceQuery, double distanceRep, double distanceRepQuery,
               double radius, u_int32_t height, tType Type);

      /**
      * Gets the next entry with the minimum key value. This entry
      * is removed from the queue.
      *
      * @param object The object to be inserted.
      * @param pageID The pageID to be inserted.
      * @param distanceQuery The distance from this entry and the query object.
      * @param distanceRep The distance from the object and the representative.
      * @param distanceRepQuery The distance from the representative and the query object.
      * @param radius The radius value of the subtree to be inserted.
      * @param height The height of this entry.
      * @param type The type of this entry.
      * @return True for success or false it the queue is empty.
      */
      bool Get(tObject * & object, u_int32_t & pageID,
               double & distanceQuery, double & distanceRep, double & distanceRepQuery,
               double & radius, u_int32_t & height, tType & type);

      /**
      * Returns the size of this queue.
      */
      int GetSize(){
         return Size;
      }//end GetSize
      
   private:

      /**
      * Structure of this entry
      */
      struct tGenericEntry{
         /**
         * The object.
         */
         tObject * Object;

         /**
         * The flag to set if this instance owns the object.
         */
         bool Mine;

         /**
         * The ID of a Page.
         */
         u_int32_t PageID;

         /**
         * The Height of this entry.
         */
         u_int32_t Height;

         /**
         * The Distance used to order the list.
         * This distance depends on the type of the entry.
         */
         double Distance;

         /**
         * The Distance between this object and the query object.
         */
         double DistanceQuery;

         /**
         * The Distance between this object and the representative object.
         */
         double DistanceRep;

         /**
         * The Distance between the object representative and the query object.
         */
         double DistanceRepQuery;

         /**
         * The subtree radius of this representative.
         */
         double Radius;

         /**
         * The type of this entry.
         */
         tType Type;
      };

      /**
      * The entries' array.
      */
      tGenericEntry * Entries;

      /**
      * Maximum size of this queue.
      */
      int MaxSize;

      /**
      * Current size of this queue.
      */
      int Size;

      /**
      * Increment.
      */
      int Increment;
      
      /**
      * Returns the id of the child which has the smaller key value.
      *
      * @param parent The id of the parent.
      * @return The id of the child or -1 if the parent has no child.
      */
      int GetMinChild(int parent){
         int c = (parent * 2) + 1;

         if ((c + 1 < Size) && (Entries[c + 1].Distance < Entries[c].Distance)){
            return c + 1;
         }else if (c + 1 < Size){
            if ((Entries[c + 1].Distance == Entries[c].Distance) &&
                (Entries[c + 1].Type < Entries[c].Type)){
               return c + 1;
            }else if ((Entries[c + 1].Distance == Entries[c].Distance) &&
                (Entries[c + 1].Type == Entries[c].Type) &&
                (Entries[c + 1].Height > Entries[c].Height)){
               return c + 1;
            }else{
               return c;
            }//end if
         }else if (c < Size){
            return c;
         }else{
            return -1;
         }//end if
      }//end GetMinChild
   
      /**
      * Expands the capacity of this heap when necessary by adding increment
      * entries to the current capacity.
      */
      void Resize();

};//end stGenericPriorityHeap

// Include implementation
#include "stGenericPriorityQueue-inl.h"

#endif //__STGPRIORITYQ_H
