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
* This file defines the VPTree node.
*
* @version 1.0
*/

#ifndef __STVPNODE_H
#define __STVPNODE_H

#include <arboretum/stPage.h>

#include <stdexcept>

/**
* This class implements the node of the VPtree. The VPtree is built by
* partitioning the space in 2 disjoint regions, using one object as
* representative (vantage point) and a radius beeing the median.
* @version 1.0
* @author Ives Rene Venturini Pola (ives@icmc.usp.br)
* @todo Documentation review.
* @ingroup VP
*/

//-----------------------------------------------------------------------------
// Class stVPNode
//-----------------------------------------------------------------------------
/**
*
* +--------------------------------------------------------------+
* | Left Node | Right Node | Radius | Offset | ..blank.. |Object |
* +--------------------------------------------------------------+
*
*/
class stVPNode{

   public:

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
      stVPNode(stPage * page, bool create = false);

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
      * Gets the ID of the left child page.
      *
      * @return The page ID required.
      */
      u_int32_t GetLeftPageID(){
         return Header->LeftPageID;
      }//end GetLeftPageID

      /**
      * Gets the ID of the right child page.
      *
      * @return The page ID required.
      */
      u_int32_t GetRightPageID(){
         return Header->RightPageID;
      }//end GetRightPageID

      /**
      * Sets the ID of the left child page.
      *
      * @return nothing.
      */
      void SetLeftPageID(u_int32_t ID){
         Header->LeftPageID = ID;
      }//end SetLeftPageID

      /**
      * Sets the ID of the right child page.
      *
      * @return nothing.
      */
      void SetRightPageID(u_int32_t ID){
         Header->RightPageID = ID;
      }//end SetRightPageID

      /**
      * Gets the Radius of the vantage point.
      *
      * @return The radius.
      */
      double GetRadius(){
         return Header->Radius;
      }//end GetRadius

      /**
      * Sets the Radius of the vantage point.
      *
      * @return nothing.
      */
      void SetRadius(double radius){
         Header->Radius = radius;
      }//end SetRadius

      /**
      * This method adds an object to this node.
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @return The position in the vector Entries or a negative value for failure.
      */
      bool AddEntry(u_int32_t size, const unsigned char * object);

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
      const unsigned char * GetObject();

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
      u_int32_t GetObjectSize();

   private:

      /**
      * Header of the stMMNode.
      */
      typedef struct VPNodeHeader{
         /**
         *  The radius of the vantage point
         */
         double Radius;

         /**
         * The Left Page ID of the respective region.
         */
         u_int32_t LeftPageID;

         /**
         * The Right Page ID of the respective region.
         */
         u_int32_t RightPageID;
      } stVPNodeHeader;

      /**
      * The associated page.
      */
      stPage * Page;

      /**
      * Header of this node.
      */
      stVPNodeHeader * Header;     

      /**
      * The Offset of the Entry.
      */
      u_int32_t * Entry;

      /**
      * Returns the free space available in this node.
      */
      u_int32_t GetFree();
            
};//end stVPNode

#endif //__STVPNODE_H
