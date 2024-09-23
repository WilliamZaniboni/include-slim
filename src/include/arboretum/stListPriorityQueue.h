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
#ifndef __STLIST_H
#define __STLIST_H


#include <arboretum/stCommon.h>

//----------------------------------------------------------------------------
// class stEntry
//----------------------------------------------------------------------------
/**
* This class defines node type of stListPriorityQueue.
*
*
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @version 1.0
* @ingroup util
* @todo Tests.
* @todo Documentation update.
*/
class stEntry {
   public:

      /**
      * Set the PageID of this entry.
      *
      * @param pageID the pageID.
      */
      void SetPageID(u_int32_t pageID){
         PageID = pageID;
      }//end SetPageID

      /**
      * Returns the PageID.
      */
      u_int32_t GetPageID(){
         return PageID;
      }//end GetPageID

      /**
      * Set the Distance of this entry.
      *
      * @param distance distance of this entry.
      */
      void SetDistance(double distance){
         Distance = distance;
      }//end SetDistance

      /**
      * Returns the Distance.
      */
      double GetDistance(){
         return Distance;
      }//end GetDistance

      /**
      * Set the Radius of this entry.
      *
      * @param radius subtree radius of this entry.
      */
      void SetRadius(double radius){
         Radius = radius;
      }//end SetRadius

      /**
      * Returns the Radius.
      */
      double GetRadius(){
         return Radius;
      }//end GetRadius

      /**
      * Set the Next entry.
      *
      * @param next next entry.
      */
      void SetNext(stEntry * next){
         Next = next;
      }//end SetNext

      /**
      * Returns the next Entry.
      */
      stEntry * GetNext(){
         return Next;
      }//end GetNext

   private:

      /**
      * The ID of a Page.
      */
      u_int32_t PageID;

      /**
      * The Distance between this object and the query object.
      */
      double Distance;

      /**
      * The subtree radius of this representative.
      */
      double Radius;

      /**
      * Link for the other Entry in the List.
      */
      stEntry * Next;
};//end stEntry


//----------------------------------------------------------------------------
// class stListPriorityQueue
//----------------------------------------------------------------------------
/**
* This class implements a priority queue based on chained list.
*
* <p>This priority queue is implemented using a simple chained list which.
*
* @author Marcos Rodrigues Vieira
* @version 1.0
* @ingroup util
* @todo Documentation update.
*/
class stListPriorityQueue{
   public:

      /**
      * Creates a new priority queue.
      *
      */
      stListPriorityQueue(){
         capacity = 0;
         head = NULL;
      }//end stListPriorityQueue

      /**
      * Disposes this queue.
      */
      ~stListPriorityQueue();

      /**
      * Adds a new entry to the queue.
      *
      * @param pageID The pageID to be inserted.
      * @param w  The node value in pageID to be inserted.
      * @param dist The distance value to be inserted.
      */
      void Add(u_int32_t pageID, double distance, double radius);

      /**
      * Gets the next node. This node is removed from the queue.
      *
      * @return The head node for success or NULL if the queue is empty.
      */
      stEntry * Get();

      /**
      * Returns the size of this queue.
      */
      u_int32_t GetSize(){
         return capacity;
      }//end GetSize
      
   private:

      /**
      * Pointer to beginning of this queue.
      */
      stEntry * head;

      /**
      * Current size of this queue.
      */
      u_int32_t capacity;

};//end stListPriorityQueue

#endif //__STLIST_H
