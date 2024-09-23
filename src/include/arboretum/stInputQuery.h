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
* This file defines the classes stInputQuery and stInputQueryPair.
*
* @version 1.0
* $Revision: 1.6 $
* $Date: 2005-03-08 19:43:09 $
* $Author: marcos $
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STINPUTQUERY_H
#define __STINPUTQUERY_H

#include <vector>


//----------------------------------------------------------------------------
// Class template stInputQueryPair
//----------------------------------------------------------------------------
/**
* This class defines the pair Object/Distance returned as the result of a query.
*
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup struct
*/
template < class ObjectType >
class stInputQueryPair{
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
      *
      * @param object The object.
      * @param radius The radius of the query object.
      * @param distance The distance from the representative query.
      */
      stInputQueryPair (tObject * object, double radius,
                        double distance = 0.0){
         this->Object = object;
         this->Radius = radius;
         this->Distance = distance;
      }//end stInputQueryPair

      /**
      * Disposes this instance and release the associated objects.
      */
      ~stInputQueryPair(){

         if (Object != NULL){
            delete this->Object;
         }//end if
      }//end ~stInputQueryPair

      /**
      * This method returns the object query.
      */
      tObject * GetObject(){
         return this->Object;
      }//end GetObject

      /**
      * This method gets the Radius of this entry.
      */
      double GetRadius(){
         return this->Radius;
      }//end GetRadius

      /**
      * This method gets the distance of this entry from the
      * representative query.
      */
      double GetDistance(){
         return this->Distance;
      }//end GetDistance

      /**
      * This method sets the distance of this entry from the
      * representative query.
      */
      void SetDistance(double distance){
         this->Distance = distance;
      }//end SetDistance

   private:

      /**
      * The object query.
      */
      tObject * Object;

      /**
      * The distance from the representative query.
      */
      double Distance;

      /**
      * The radius of this query object.
      */
      double Radius;
};//end stInputQueryPair

//----------------------------------------------------------------------------
// Class template stInputQuery
//----------------------------------------------------------------------------
/**
* This class implements a query input. It will hold a set of pairs
* Object/Radius/Distance.
*
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup struct
*/
template < class ObjectType, class EvaluatorType>
class stInputQuery{
   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stInputQueryPair Pair used by this class.
      */
      typedef stInputQueryPair < ObjectType > tPair;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> inputs
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of inputs (default = 1).
      */
      stInputQuery(int hint = 1){
         // Reserve results
         Pairs.reserve(hint);
      }//end stInputQuery

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stInputQuery();

      /**
      * This operator allow the access to a pair.
      */
      tPair & operator [] (int idx){
         return (*Pairs[idx]);
      }//end operator []

      /**
      * This method sets the EvaluatorType of this input query.
      * This method do not do a copy.
      * @param evaluator The EvaluatorType.
      */
      void BorrowMetricEvaluator(EvaluatorType * evaluator){
         Evaluator = evaluator;
      }//end BorrowMetricEvaluator

      /**
      * This method release the evaluator of this input query.
      */
      void ReleaseMetricEvaluator(){
         Evaluator = NULL;
      }//end ReleaseMetricEvaluator

      /**
      * This method returns the number of entries in this input query.
      */
      int GetNumOfEntries(){
         return Pairs.size();
      }//end GetNumOfEntries

      /**
      * This method get the index of the representative object.
      */
      u_int32_t GetRepresentativeIndex(){
         return RepIndex;
      }//end GetRepresentativeIndex

      /**
      * This method sets the index of the representative object.
      *
      * @param repIndex the index of the representative object.
      */
      void SetRepresentativeIndex(u_int32_t repIndex){
         RepIndex = repIndex;
      }//end SetRepresentativeIndex

      /**
      * This method adds a pair Object/Distance to this result list.
      *
      * @param object The object.
      * @param distance The distance from the .
      * @warning There is no duplicate pair checking. All pairs will be added.
      */
      void AddPair(tObject * object, double radius){
         Pairs.insert(Pairs.end(), new tPair(object, radius));
      }//end AddPair

      /**
      * This method returns the distance of this object from the representative
      * object.
      * If this result is empty, it will return a negative value.
      *
      * @param idx the index of the object.
      */
      double GetDistance(int idx){

         if (Pairs.size() > 0){
            return Pairs[Pairs.begin() + idx]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetDistance

      /**
      * Sets information about the query radius.
      *
      * @param queryRadius the value of the new radius query.
      */
      void SetQueryRadius(double queryRadius){
         QueryRadius = queryRadius;
      }//end SetQueryRadius

      /**
      * Gets information about the query radius.
      * Returns 0 if it is not defined yet!
      */
      double GetQueryRadius(){
         return QueryRadius;
      }//end GetQueryRadius

      /**
      * This method choose a new representative object for this query.
      * The representative is the center object that generate the minimum
      * cover radius.
      *
      * @return the index of the new representative object.
      */
      u_int32_t ChooseRepresentative();

      /**
      * This method update the distance of the object from the new
      * representative object.
      *
      * @param repObj the represetative object.
      * @param repObjIdx the index of the represetative object.
      */
      void UpdateDistances(ObjectType * repObj,
                           u_int32_t repObjIdx);

      /**
      * This method returns the minimum radius query of this query.
      */
      double GetMinimumRadius();

      /**
      * This method ...
      */
      u_int32_t ChooseRepresentativeSmallRadius();
      
   private:

      /**
      * The vector of pairs.
      */
      vector < tPair * > Pairs;

      /**
      * Information about ...
      */
      u_int32_t RepIndex;

      /**
      * Information about radius for the inputQuery.
      */
      double QueryRadius;

      /**
      * The evaluator type. It is used to evaluate the distances.
      */
      EvaluatorType * Evaluator;

};//end stInputQuery

#include <arboretum/stInputQuery-inl.h>

#endif //__STINPUTQUERY
