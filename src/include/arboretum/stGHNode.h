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
* This file defines the GHTree node.
*
* @version 1.0
* @author Ives Renê Venturini Pola (ives@icmc.usp.br) $
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br) $
*/

#ifndef __STGHNODE_H
#define __STGHNODE_H

#include <arboretum/stPage.h>

#include <stdexcept>

/**
* This class implements the node of the GHTree. Since the GHTree is based on
* hiperplanes, this node holds one or two objects, the distance between them,
* and the left and right links to the children nodes.
* @version 1.0
* @author Ives Rene Venturini Pola (ives@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation review.
* @ingroup GH
*/
// +----------------------------------------------------------------------+
// | Occupation | Distance | Offset0 | Offset1 | ..blank.. | Obj1 | obj0 |
// +----------------------------------------------------------------------+
class stGHNode{

   public:

      typedef struct GHNodeEntry {

         /**
         * ID of the sub-page.
         */
         u_int32_t PageID;

         /**
         * Offset of the object.
         */
         u_int32_t Offset;
      } stGHNodeEntry;

      /**
      * Creates a new instance of this class. The parameter <i>page</i> is an
      * instance of stPage that hold the node data.
      *
      * <P>The parameter <i>create</i> tells to stPage what operation will
      * be performed. True means that the page will be initialized and false
      * means that the page will be used just as it is. The default value is
      * false.
      *
      * @param page The page that hold the data of this node.
      * @param create The operation to be performed.
      */
      stGHNode(stPage * page, bool create = false);

      /**
      * Gets the associated page.
      *
      * @return The associated page.
      */
      stPage * GetPage(){
         return Page;
      }//end GetPage

      /**
      * Gets the ID of the associated page.
      *
      * @return The ID of the associated page.
      */
      u_int32_t GetPageID(){
         return Page->GetPageID();
      }//end GetPageID

      /**
      * Gets the ID of a child page.
      *
      * @return The ID of the child page.
      */
      u_int32_t GetChildPageID(u_int32_t idx){
         return Entries[idx].PageID;
      }//end GetPageID

      /**
      * Sets the ID of a child page.
      *
      * @return nothing.
      */
      void SetChildPageID(u_int32_t idx, u_int32_t ID){
         Entries[idx].PageID = ID;
      }//end SetChildPageID

      /**
      * Gets the Distance of the objects.
      *
      * @return The Distance.
      */
      double GetDistance(){
         return this->Header->Distance;
      }//end GetDistance

      /**
      * Sets the Distance of the objects.
      *
      * @return nothing.
      */
      void SetDistance(double dist){
         Header->Distance = dist;
      }//end SetDistance

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
      * This method adds an object to this node.
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @return The position in the vector Entries or a negative value for failure.
      * @see RemoveObject()
      */
      int AddEntry(u_int32_t size, const unsigned char * object);

      /**
      * Gets the serialized object. Use GetObjectSize to determine the size of
      * the object.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      *  unless __stDEBUG__ is defined at compile time.
      * @return A pointer to the serialized object.
      * @see GetObjectSize()
      */
      const unsigned char * GetObject(u_int32_t idx);
     
      /**
      * Returns the size of the object in bytes. Use GetObject() to get the
      * object data.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      *  unless __stDEBUG__ is defined at compile time.
      * @return The size of the serialized object.
      * @see GetObject()
      */
      u_int32_t GetObjectSize(u_int32_t idx);

      /**
      * Removes an entry from this Node.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      *  unless __stDEBUG__ is defined at compile time.
      * @see AddEntry()
      */
      void RemoveEntry(u_int32_t idx);

   private:
   
      /**
      * Header of the stGHNode.
      */
      typedef struct GHNodeHeader{
         /**
         * Occupation of this node.
         */
         u_int32_t Occupation;

         /**
         * The Distance between the two objects
         */
         double Distance;
      } stGHNodeHeader;//end

      /**
      * The associated page.
      */
      stPage * Page;

      /**
      * Header of this node.
      */
      stGHNodeHeader * Header;

      /**
      * Entries.
      */
      stGHNodeEntry * Entries;

      /**
      * Returns the free space available in this node.
      */
      u_int32_t GetFree();

};//end stGHNode

#endif //__STGHNODE_H
