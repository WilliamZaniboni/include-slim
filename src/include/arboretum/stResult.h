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
* This file defines the classes stResult and stResults.
*
* @version 1.0
* @author Enzo Seraphim (seraphim@unifei.edu.br)
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STRESULT_H
#define __STRESULT_H

#include <vector>



/**
* Define constants to query type.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
enum tQueryType{
   /**
   * Indicates that there is no information about the query.
   */
   UNKNOWN = 0,

   /**
   * This is a result of a range query.
   */
   RANGEQUERY = 1,

   /**
   * This is a result of a k nearest neighbour query.
   */
   KNEARESTQUERY = 2,

   /**
   * This is a result of a estimated k nearest neighbour query.
   */
   ESTIMATEKNEARESTQUERY = 3,

   /**
   * This is a result of a ring query.
   */
   RINGQUERY = 4,

   /**
   * This is a result of a k ring query.
   */
   KRINGQUERY = 5,

   /**
   * This is a result of a k AND range query.
   */
   KANDRANGEQUERY = 6,

   /**
   * This is a result of a k OR range query.
   */
   KORRANGEQUERY = 7,

   /**
   * This is a result of a Point query.
   */
   POINTQUERY = 8,

   /**
   * This is a result of a Reverse of Range query.
   */
   REVERSEDRANGEQUERY = 9,

   /**
   * This is a result of a K-Farthest query.
   */
   KFARTHESTQUERY = 10,

   /**
   * This is a result of a Range Join query.
   */
   KCLOSESTJOINQUERY = 11,

   /**
   * This is a result of a Range Join query.
   */
   KNEARESTJOINQUERY = 12,

   /**
   * This is a result of a Nearest Join query.
   */
   RANGEJOINQUERY = 13,

   /**
   * This is a result of a sum nearest query.
   */
   SUMKNEARESTQUERY = 14,

   /**
   * This is a result of a sum range query.
   */
   SUMRANGEQUERY = 15,

   /**
   * This is a result of a lazy range query.
   */
   LAZYRANGEQUERY = 16,

   /**
   * This is a result of a next range query.
   */
   NEXTRANGEQUERY = 17,

   /**
   * This is a result of a previous range query.
   */
   PREVIOUSRANGEQUERY = 18,

   /**
   * This is a result of a total order less than query.
   */
   TO_LESSTHANQUERY = 19,

   /**
   * This is a result of a total order less than or equal to query.
   */
   TO_LESSTHANOREQUALQUERY = 20,

   /**
   * This is a result of a total order equal to query.
   */
   TO_EQUALQUERY = 21,

   /**
   * This is a result of a total order greater than query.
   */
   TO_GREATERTHANQUERY = 22,

   /**
   * This is a result of a total order greater than or equal to query.
   */
   TO_GREATERTHANOREQUALQUERY = 23,

   /**
   * This is a result of a total order between query.
   */
   TO_BETWEENQUERY = 24
};//end tQueryType

//----------------------------------------------------------------------------
// Class template stResultPair
//----------------------------------------------------------------------------
/**
* This class defines the pair Object/Distance returned as the result of a query.
* It is used by class template stResult to store pairs of results and their
* associated distance from the center of the query. Do not create instances of
* this class template directly.
*
* @author Enzo Seraphim (seraphim@unifei.edu.br)
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup struct
* @see stResult
*/
template < class ObjectType, class KeyType = double >
class stResultPair{

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
       * Type of the key.
       */
      typedef KeyType tKey;

      /**
      * Creates a new pair Object/Distance.
      *
      * <P>This instance will claim the ownership of the object. In other words,
      * it will dispose the object when it is no loger necessary.
      */
      stResultPair(){
         this->Object = NULL;
         this->Key = -1;
      }//end stResultPair

      /**
      * Creates a new pair Object/Distance.
      *
      * <P>This instance will claim the ownership of the object. In other words,
      * it will dispose the object when it is no loger necessary.
      *
      * @param obj The result object.
      * @param distance The distance from the sample.
      */
      stResultPair(tObject * obj, tKey key){
         this->Object = obj;
         this->Key = key;
      }//end stResultPair

      /**
      * Disposes this instance and release all associated resources including
      * the result object.
      */
      virtual ~stResultPair(){
         if (Object != NULL){
            delete Object;
         }//end if
      }//end ~stResultPair

      /**
      * This method sets the object.
      */
      void SetObject(tObject * obj){
         Object = obj;
      }//end SetObject

      /**
      * This method returns the object.
      */
      tObject * GetObject() const{
         return Object;
      }//end GetObject

      /**
      * This method sets the key.
      */
      void SetKey(tKey key){
         this->Key = key;
      }//end SetDistance

      /**
      * This method sets the distance.
      * It is maintained for backward compatibility issues.
      */
      void SetDistance(double dist){
         SetKey(dist);
      }//end SetDistance

      /**
      * This method gets the distance.
      */
      tKey GetKey(){
         return Key;
      }//end GetDistance

      /**
      * This method gets the distance.
      * It is maintained for backward compatibility issues.
      */
      double GetDistance(){
         return GetKey();
      }//end GetDistance

   private:

      /**
      * The object.
      */
      tObject * Object;

      /**
      * The key employed to build the result. Regarding similarity queries, it
      *  is the distance from the query object.
      */
      tKey Key;
      
};//end stResultPair

//----------------------------------------------------------------------------
// Class template stResultTriple
//----------------------------------------------------------------------------
/**
* This class defines the triple Object/JoinedObject/Distance returned as
* the result of a joined query.
* It is used by class template stResult to store pairs of results and their
* associated distance from the center of the query. Do not create instances of
* this class template directly. 
*
* @author Enzo Seraphim (seraphim@unifei.edu.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup struct
* @see stResultPair
* @see stResult
*/
template < class ObjectType >
class stResultTriple: public stResultPair < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * Creates a new pair Object/Distance.
      *
      * <P>This instance will claim the ownership of the object. In other words,
      * it will dispose the object when it is no loger necessary.
      */
      stResultTriple():stResultPair< ObjectType >(){

         this->JoinedObject = NULL;
      }//end stResultTriple

      /**
      * Creates a new pair Object/Distance with a JoinedObject.
      *
      * <P>This instance will claim the ownership of the object. In other words,
      * it will dispose the object when it is no loger necessary.
      *
      * @param obj The result object.
      * @param joined obj The result joined object.
      * @param distance The distance from the sample.
      */
      stResultTriple(tObject * obj, tObject * joinedobj, double distance):
         stResultPair< ObjectType >(obj, distance){

         this->JoinedObject = joinedobj;
      }//end stResultTriple

      /**
      * Disposes this instance and release all associated resources including
      * the result object.
      */
      virtual ~stResultTriple(){

         if (JoinedObject != NULL){
            delete JoinedObject;
         }//end if
      }//end ~stResultTriple

      /**
      * This method sets the object.
      */
      void SetJoinedObject(tObject * obj){
         JoinedObject = obj;
      }//end SetJoinedObject

      /**
      * This method returns the object.
      */
      tObject * GetJoinedObject() const{
         return JoinedObject;
      }//end GetObject

   private:

      /**
      * The joined object.
      */
      tObject * JoinedObject;

};//end stResultTriple

//----------------------------------------------------------------------------
// Class template stBasicResult
//----------------------------------------------------------------------------
/**
* This class implements a query result. It will hold a set of pairs
* Object/Distance ordered by distance which constitutes the answer
* to a query. All query methods of all metric trees implemented by
* this library will return instances of this class.
*
* <P>In nearest neigbour queries and k-range queries, the result set may contain
* more than k results. It means that the greatest distance from sample to result
* has more than 1 object. In such cases, all objects whose distance from sample
* is equal to GetMaximumDistance() constitute the draw list.
*
* <P>As an extra capability, it can store information about the query but they
* will only be available if the query method supports this feature (this is an
* optional capability). See SetQueryInfo() for more details.
*
* <P> It also performs basic operations that allows the construction of
* the result set by the query procedures.
*
* @author Enzo Seraphim (seraphim@unifei.edu.br)
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @version 1.1
* @ingroup struct
*/
template < class ObjectType >
class stBasicResult{

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stBasicResult(){
      }//end ~stBasicResult

      /**
      * Gets the information about query type. I may assume the values
      * UNKNOWN, RANGEQUERY, KANDRANGEQUERY, KORRANGEQUERY or KNEARESTQUERY.
      */
      int GetQueryType(){
         return QueryType;
      }//end GetQueryType

      /**
      * Gets information about k. It makes sense only for KANDRANGEQUERY, KORRANGEQUERY
      * and KNEARESTQUERY.
      */
      unsigned int GetK(){
         return K;
      }//end GetK

      /**
      * Gets information about radius. It makes sense only for RANGEQUERY and
      * KRANGEQUERY.
      */
      double GetRadius(){
         return Radius;
      }//end GetRadius

      /**
      * Gets information about inner radius. It makes sense only for CROWNQUERY.
      */
      double GetInnerRadius(){
         return InnerRadius;
      }//end GetInnerRadius

      /**
      * Gets the tie list to the query. It is an optional
      * information.
      *
      * @return True if there is tie list or False otherwise.
      */
      bool GetTie(){
         return Tie;
      }//end GetTie

   protected:

      /**
      * Information about QueryType.
      */
      int QueryType;

      /**
      * Information about k for KANDRANGEQUERY, KORRANGEQUERY and
      * KNEARESTQUERY (Optional).
      */
      unsigned int K;

      /**
      * Information about tie list for KANDRANGEQUERY, KORRANGEQUERY
      * and KNEARESTQUERY .
      */
      bool Tie;

      /**
      * Information about radius for KANDRANGEQUERY, KORRANGEQUERY and
      * RANGEQUERY (Optional).
      */
      double Radius;

      /**
      * Information about inner radius for CROWNQUERY (Optional).
      */
      double InnerRadius;

};//end stBasicResult

#ifdef _DEPRECATED_VECTOR_RESULT
   #include "stVectorResult.h"
#else
   #ifdef _DEPRECATED_LIST_RESULT
      #include "stListResult.h"
   #else
      #include "stTreeResult.h"
   #endif 
#endif 

#endif //__STRESULT_H
