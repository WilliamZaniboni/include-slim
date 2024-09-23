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
* This file defines a Collection of Objects to be promoted in high levels of
* the tree.
*
* @version 1.0
* $Revision: 1.4 $
* $Date: 2004-03-07 19:21:32 $
* $Author: marcos $
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Review of documentation.
*/

#ifndef __STDBMCOLLECTION_H
#define __STDBMCOLLECTION_H


#include <stdexcept>

//----------------------------------------------------------------------------
// Class template stResultPair
//----------------------------------------------------------------------------
/**
* This class defines the pair Object/Distance returned as the result of a query.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup db
*/
template < class ObjectType > class stDBMEntryCollection{

   public:
      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * Creates a new pair DBEntry.
      *
      * <P>This instance will be the owner of the object. In other words,
      * it will not dispose the object when it is no loger necessary.
      *
      * @param object The object.
      * @param height The height of the subtree.
      * @param pageID The pageID of the subtree.
      * @param nEntries The nEntries of the subtree.
      * @param radius The radius of the subtree.
      */
      stDBMEntryCollection(){
         this->Object = NULL;
         this->Mine = false;
         #ifdef __stDBMHEIGHT__
            this->Height = 0;
         #endif //__stDBMHEIGHT__
         this->PageID = 0;
         #ifdef __stDBMNENTRIES__
            this->NEntries = 0;
         #endif //__stDBMNENTRIES__
         this->Radius = 0;
      }//end stDBMEntryCollection

      /**
      * Creates a new pair DBEntry.
      *
      * <P>This instance will be the owner of the object. In other words,
      * it will not dispose the object when it is no loger necessary.
      *
      * @param object The object.
      * @param height The height of the subtree.
      * @param pageID The pageID of the subtree.
      * @param nEntries The nEntries of the subtree.
      * @param radius The radius of the subtree.
      */
      stDBMEntryCollection(tObject * object,
                          #ifdef __stDBMHEIGHT__
                             unsigned char height,
                          #endif //__stDBMHEIGHT__
                          u_int32_t pageID,
                          #ifdef __stDBMNENTRIES__
                             u_int32_t nEntries,
                          #endif //__stDBMNENTRIES__
                          double radius,
                          bool mine = true){
         this->Object = object;
         #ifdef __stDBMHEIGHT__
            this->Height = height;
         #endif //__stDBMHEIGHT__
         this->PageID = pageID;
         #ifdef __stDBMNENTRIES__
            this->NEntries = nEntries;
         #endif //__stDBMNENTRIES__
         this->Radius = radius;
         this->Mine = mine;
      }//end stDBMEntryCollection

      /**
      * This destructor method destructs the Object only if
      * it the owner of object.
      */
      ~stDBMEntryCollection(){
         if (this->Mine && Object!=NULL){
            delete Object;
            Object = NULL;
         }//end if
      }//end ~stDBMEntryCollection

      /**
      * This method set who is the owner of Object.
      *
      * @param owner if true, the owner is not the stDBMEntryCollection.
      */
      void SetMine(bool owner){
         this->Mine = owner;
      }//end SetMine

      /**
      * This method gets who is the owner of Object.
      */
      bool GetMine(){
         return Mine;
      }//end GetMine

      /**
      * This method returns the object.
      */
      tObject * GetObject(){
         return Object;
      }//end GetObject

      /**
      * This method sets the object.
      */
      void SetObject(tObject * object){
         Object = object;
      }//end SetObject

      #ifdef __stDBMHEIGHT__
         /**
         * This method gets the height.
         */
         unsigned char GetHeight(){
            return Height;
         }//end GetHeight

         /**
         * This method sets the height.
         */
         void SetHeight(unsigned char height){
            Height = height;
         }//end SetHeight
      #endif //__stDBMHEIGHT__

      /**
      * This method gets the pageID.
      */
      u_int32_t GetPageID(){
         return PageID;
      }//end GetPageID

      /**
      * This method sets the pageID.
      */
      void SetPageID(u_int32_t pageID){
         PageID = pageID;
      }//end SetPageID

      #ifdef __stDBMNENTRIES__
         /**
         * This method gets the nEntries.
         */
         u_int32_t GetNEntries(){
            return NEntries;
         }//end GetNEntries

         /**
         * This method sets the nEntries.
         */
         void SetNEntries(u_int32_t nEntries){
            NEntries = nEntries;
         }//end SetNEntries
      #endif //__stDBMNENTRIES__

      /**
      * This method gets the radius.
      */
      double GetRadius(){
         return Radius;
      }//end GetRadius

      /**
      * This method sets the radius.
      */
      void SetRadius(double radius){
         Radius = radius;
      }//end SetRadius

   private:

      /**
      * The owner of the object.
      */
      bool Mine;

      /**
      * The object.
      */
      tObject * Object;

      /**
      * Height of the subtree.
      */
      #ifdef __stDBMHEIGHT__
         unsigned char Height;
      #endif //__stDBMHEIGHT__

      /**
      * ID of the page.
      */
      u_int32_t PageID;

      /**
      * Number of entries in the sub-tree.
      */
      #ifdef __stDBMNENTRIES__
         u_int32_t NEntries;
      #endif //__stDBMNENTRIES__

      /**
      * Radius of the sub-tree.
      */
      double Radius;

};//end stDBMEntryCollection

//-----------------------------------------------------------------------------
// Class stDBMCollectionVector
//-----------------------------------------------------------------------------

/**
* This class implements a collection of object based on a vector.
*
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation.
* @ingroup db
*/
template <class ObjectType> class stDBMCollectionVector{

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResult Pair used by this class.
      */
      typedef stDBMEntryCollection < ObjectType > tPair;

      /**
      * This method will create a new instance of this class. The parameter
      * hint is used to prepare this instance to hold at least <i>hint</i>
      * objects (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 32).
      */
      stDBMCollectionVector(int hint = 32){
         // Reserve results
         Pairs.reserve(hint);
      }//end stDBMCollectionVector

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      ~stDBMCollectionVector(){
         RemoveAll();
      }//end ~stDBMCollectionVector
      
      /**
      * This operator allow the access to a pair.
      */
      tPair * operator [] (int idx){
         return Pairs[idx];
      }//end operator []

      /**
      * This method will remove the last entry from the collection.
      */
      void RemoveLast(){
         if (Pairs.size() > 0){
            if (Pairs[Pairs.size() - 1] != NULL){
               delete Pairs[Pairs.size() - 1];
            }//end if
            Pairs.pop_back();
         }//end if
      }//end RemoveLast

      /**
      * This method will remove all entries from the collection.
      */
      void RemoveAll();

      /**
      * This method returns the number of entries in this result.
      */
      u_int32_t GetNumberOfEntries(){
         return Pairs.size();
      }//end GetNumberOfEntries

      /**
      * This method returns the number of entries that it is not a
      * representative of a subtree.
      */
      u_int32_t GetNumberOfFreeObjects();

      /**
      * This method returns the amount of used space for all entries.
      */
      u_int32_t GetUsedSpace();

      /**
      * This method set the Mine option for all objects.
      */
      void SetMineForAllObjects(bool option);

      /**
      * This method adds a entry in the collection.
      *
      * @param object The object.
      * @param height The height of the subtree.
      * @param pageID The pageID of the subtree.
      * @param nEntries The nEntries of the subtree.
      * @param radius The radius of the subtree.
      * @param mine Who is the owner of object.
      */
      void AddEntry(tObject * object,
                    #ifdef __stDBMHEIGHT__
                       unsigned char height,
                    #endif //__stDBMHEIGHT__
                    u_int32_t pageID,
                    #ifdef __stDBMNENTRIES__
                       u_int32_t nEntries,
                    #endif //__stDBMNENTRIES__
                    double radius,
                    bool mine = true){
         Pairs.insert(Pairs.begin(),
                      new tPair(object,
                      #ifdef __stDBMHEIGHT__
                         height,
                      #endif //__stDBMHEIGHT__
                      pageID,
                      #ifdef __stDBMNENTRIES__
                         nEntries,
                      #endif //__stDBMNENTRIES__
                      radius,
                      mine));
      }//end AddPair

   private:

      /**
      * The vector of stDBMEntryCollection.
      */
      vector < tPair * > Pairs;

};//end stDBMCollectionVector


//-----------------------------------------------------------------------------
// Class stDBMCollection
//-----------------------------------------------------------------------------

/**
* This class implements a collection of object.
*
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation.
* @ingroup db
*/
template <class ObjectType> class stDBMCollection{

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResult Pair used by this class.
      */
      typedef stDBMEntryCollection < ObjectType > tPair;

      /**
      * This method will create a new instance of this class. The parameter
      * hint is used to prepare this instance to hold at least <i>hint</i>
      * objects (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 32).
      */
      stDBMCollection(u_int32_t initialSize = 32, u_int32_t increment = 32){
         Capacity = initialSize;
         Increment = increment;
         Size = 0;
         // Reserve results
         Pairs = new tPair[Capacity];
      }//end stDBMCollection

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      ~stDBMCollection(){
         // First remove all entries.
         RemoveAll();
         // Then delete the vector.
			if (Pairs != NULL){
				delete[] Pairs;
            Pairs = NULL;
			}//end if
         Capacity = 0;         
      }//end ~stDBMCollection
      
      /**
      * This operator allow the access to a pair.
      */
      tPair * operator [] (int idx){
         return &Pairs[idx];
      }//end operator []

      /**
      * This method will remove the last entry from the collection.
      */
      void RemoveLast(){
         if (Size > 0){
            if (Pairs[Size - 1].GetMine()){
               delete Pairs[Size - 1].GetObject();
               Pairs[Size - 1].SetObject(NULL);
            }//end if
            Size--;
         }//end if
      }//end RemoveLast

      /**
      * This method will remove all entries from the collection.
      */
      void RemoveAll();

      /**
      * This method returns the number of entries in this result.
      */
      u_int32_t GetNumberOfEntries(){
         return Size;
      }//end GetNumberOfEntries

      /**
      * This method returns the number of entries in this result.
      */
      u_int32_t GetSize(){
         return Size;
      }//end GetSize

      /**
      * This method returns the number of entries that it is not a
      * representative of a subtree.
      */
      u_int32_t GetNumberOfFreeObjects();

      /**
      * This method returns the amount of used space for all entries.
      */
      u_int32_t GetUsedSpace();

      /**
      * This method set the Mine option for all objects.
      */
      void SetMineForAllObjects(bool option);

      /**
      * This method adds a entry in the collection.
      *
      * @param object The object.
      * @param height The height of the subtree.
      * @param pageID The pageID of the subtree.
      * @param nEntries The nEntries of the subtree.
      * @param radius The radius of the subtree.
      */
      void AddEntry(tObject * object,
                    #ifdef __stDBMHEIGHT__
                       unsigned char height,
                    #endif //__stDBMHEIGHT__
                    u_int32_t pageID,
                    #ifdef __stDBMNENTRIES__
                       u_int32_t nEntries,
                    #endif //__stDBMNENTRIES__
                    double radius,
                    bool mine = true);

   private:

      /**
      * The Used Size.
      */
      u_int32_t Size;

      /**
      * The Maximum Capacity.
      */
      u_int32_t Capacity;

		/**
		* Increment.
		*/
      u_int32_t Increment;

      /**
      * The vector of stDBMEntryCollection.
      */
      tPair * Pairs;

		/**
		* Expands the capacity of this heap when necessary by adding increment
		* entries to the current capacity.
		*/
		void Resize();
      
};//end stDBMCollection

// Include implementation
#include "stDBMCollection-inl.h"

#endif //__STDBMCOLLECTION_H
