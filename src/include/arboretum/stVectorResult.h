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
#ifndef __STVECTORRESULT_H
#define __STVECTORRESULT_H

#include <vector>


//----------------------------------------------------------------------------
// Class template stResult
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
class stResult: public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResult Pair used by this class.
      */
      typedef stResultPair< ObjectType > tPair;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1).
      */
      stResult(unsigned int hint = 1){
         // Reserve results
         Pairs.reserve(hint);
         // No info
         SetQueryInfo();
      }//end stResult

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stResult();

      /**
      * This operator allow the access to a pair.
      */
      tPair & operator [] (unsigned int idx){
         return (*Pairs[idx]);
      }//end operator []

      /**
      * This method returns the number of entries in this result.
      */
      unsigned int GetNumOfEntries(){
         return Pairs.size();
      }//end GetNumOfEntries

      /**
      * This method adds a pair Object/Distance to this result list.
      *
      * @param obj The object.
      * @param distance The distance from the sample object.
      * @warning There is no duplicate pair checking. All pairs will be added.
      */
      void AddPair(tObject * obj, double distance);

      /**
      * This method will remove the last object from this result list.
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
      * This method will remove the first object from this result list.
      */
      void RemoveFirst(){

         if (Pairs.size() > 0){
            if (Pairs[0] != NULL){
               delete Pairs[0];
            }//end if
            Pairs.erase(Pairs.begin());
         }//end if
      }//end RemoveFirst

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance(){

         if (Pairs.size() > 0){
            return Pairs[0]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetMinimumDistance

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance(){

         if (Pairs.size() > 0){
            return Pairs[Pairs.size() - 1]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetMaximumDistance

      /**
      * This method will cut out undesired objects. It may be used
      * by k-Nearest Neighbour queries to control the number of results.
      *
      * <P>This implementation also treat...
      *
      * @param limit The desired number of results.
      * @todo Review of this explanation.
      */
      void Cut(unsigned int limit);

      /**
      * This method will cut out undesired objects. It may be used
      * by k-Farthest Neighbour queries to control the number of results.
      *
      * <P>This implementation also treat...
      *
      * @param limit The desired number of results.
      * @todo Review of this explanation.
      */
      void CutFirst(unsigned int limit);

      /**
      * Adds information about the query. It is used by Query methods to add
      * information about the query. Since it is optional, not all results will
      * provide meaningful information about it.
      *
      * @param sample The sample object (a copy of it).
      * @param querytype The query type (UNKNOWN, RANGEQUERY, NEARESTQUERY,
      * KANDRANGEQUERY, KORRANGEQUERY, CROWNQUERY, KANDRANGEQUERYESTIMATE or
      * KORRANGEQUERYESTIMATE)
      * @param k The value of k (if it makes sence).
      * @param radius The value of radius (if it makes sence).
      * @param innerRadius The value of inner radius (if it makes sence).
      * @param tie The tie list. Default false;
      *
      * @warning It do not changes the behavior of this result.
      * @see GetQueryType()
      * @see GetK()
      * @see GetRadius()
      * @see GetSample()
      * @see GetTie()
      */
      void SetQueryInfo(tObject * sample = NULL, int querytype = UNKNOWN,
                        u_int32_t k = 0, double radius = 0.0,
                        double innerRadius = 0.0, bool tie = false){
         this->QueryType = querytype;
         this->K = k;
         this->Radius = radius;
         this->InnerRadius = innerRadius;
         Sample = sample;
         this->Tie = tie;
      }//end SetQueryInfo

      /**
      * Adds information about the query. It is used by Query methods to add
      * information about the query. Since it is optional, not all results will
      * provide meaningful information about it.
      *
      * @param sample The sample object (a copy of it).
      * @param querytype The query type (UNKNOWN, RANGEQUERY, KRANGEQUERY or
      * KNEARESTQUERY)
      * @param k The value of k (if it makes sence).
      * @param radius The value of radius (if it makes sence).
      * @param tie The tie list. Default false;
      *
      * @warning It do not changes the behavior of this result.
      * @see GetQueryType()
      * @see GetK()
      * @see GetRadius()
      * @see GetSample()
      * @see GetTie()
      */
      void SetQueryInfo(tObject * sample, int querytype,
                        u_int32_t k, double radius, bool tie){
         this->SetQueryInfo(sample, querytype, k, radius, 0.0, tie);
      }//end SetQueryInfo

      /**
      * Gets the information about query type. I may assume the values
      * UNKNOWN, RANGEQUERY, KANDRANGEQUERY, KORRANGEQUERY or KNEARESTQUERY.
      */
      int GetQueryType(){
         return this->QueryType;
      }//end GetQueryType

      /**
      * Gets information about k. It makes sense only for KANDRANGEQUERY, KORRANGEQUERY
      * and KNEARESTQUERY.
      */
      unsigned int GetK(){
         return this->K;
      }//end GetK

      /**
      * Gets information about radius. It makes sense only for RANGEQUERY and
      * KRANGEQUERY.
      */
      double GetRadius(){
         return this->Radius;
      }//end GetRadius

      /**
      * Gets information about inner radius. It makes sense only for CROWNQUERY.
      */
      double GetInnerRadius(){
         return this->InnerRadius;
      }//end GetRadius

      /**
      * Gets the sample object if it is available. Since it is an optional
      * information it may not be available.
      *
      * @return The sample object or NULL if it is not available.
      */
      tObject * GetSample(){
         return Sample;
      }//end GetSample

      /**
      * This method tests if two results are equal.
      *
      * @param r1 The second result to be test.
      * @return True if is equal, False otherwise.
      */
      bool IsEqual(const stResult * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
      double Precision(const stResult * r1);


      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
      void Intersection(stResult * result1, stResult * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
      void Union(stResult * result1, stResult * result2);

   private:

      /**
      * The vector of pairs.
      */
      vector < tPair * > Pairs;

      /**
      * Sample object.
      */
      tObject * Sample;

      /**
      * This method locates the insertion position of an object.
      *
      * @param distance The desired distance.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      unsigned int Find(double distance);

};//end stResult

//----------------------------------------------------------------------------
// Class template stResultPaged
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
class stResultPaged: public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResult Pair used by this class.
      */
      typedef stResultPair< ObjectType > tPair;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1).
      */
      stResultPaged(unsigned int hint = 1){
         // Reserve results
         Pairs.reserve(hint);
         // No info
         SetQueryInfo();
      }//end stResultPaged

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stResultPaged();

      /**
      * This operator allow the access to a pair.
      */
      tPair & operator [] (unsigned int idx){
         return (*Pairs[idx]);
      }//end operator []

      /**
      * This method returns the number of entries in this result.
      */
      unsigned int GetNumOfEntries(){
         return Pairs.size();
      }//end GetNumOfEntries

      /**
      * This method adds a pair Object/Distance to this result list.
      *
      * @param obj The object.
      * @param distance The distance from the sample object.
      * @warning There is no duplicate pair checking. All pairs will be added.
      */
      void AddPair(tObject * obj, double distance);

      /**
      * This method will remove the last object from this result list.
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
      * This method will remove the first object from this result list.
      */
      void RemoveFirst(){

         if (Pairs.size() > 0){
            if (Pairs[0] != NULL){
               delete Pairs[0];
            }//end if
            Pairs.erase(Pairs.begin());
         }//end if
      }//end RemoveFirst

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance(){

         if (Pairs.size() > 0){
            return Pairs[0]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetMinimumDistance

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance(){

         if (Pairs.size() > 0){
            return Pairs[Pairs.size() - 1]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetMaximumDistance

      /**
      * This method will cut out undesired objects. It may be used
      * by k-Nearest Neighbour queries to control the number of results.
      *
      * <P>This implementation also treat...
      *
      * @param limit The desired number of results.
      * @todo Review of this explanation.
      */
      void Cut(unsigned int limit);

      /**
      * This method will cut out undesired objects. It may be used
      * by k-Farthest Neighbour queries to control the number of results.
      *
      * <P>This implementation also treat...
      *
      * @param limit The desired number of results.
      * @todo Review of this explanation.
      */
      void CutFirst(unsigned int limit);

      /**
      * Adds information about the query. It is used by Query methods to add
      * information about the query. Since it is optional, not all results will
      * provide meaningful information about it.
      *
      * @param sample The sample object (a copy of it).
      * @param querytype The query type (UNKNOWN, RANGEQUERY, NEARESTQUERY,
      * KANDRANGEQUERY, KORRANGEQUERY, CROWNQUERY, KANDRANGEQUERYESTIMATE or
      * KORRANGEQUERYESTIMATE)
      * @param k The value of k (if it makes sence).
      * @param radius The value of radius (if it makes sence).
      * @param innerRadius The value of inner radius (if it makes sence).
      * @param tie The tie list. Default false;
      *
      * @warning It do not changes the behavior of this result.
      * @see GetQueryType()
      * @see GetK()
      * @see GetRadius()
      * @see GetSample()
      * @see GetTie()
      */
      void SetQueryInfo(tObject * sample = NULL, int querytype = UNKNOWN,
                        u_int32_t k = 0, double radius = 0.0,
                        double innerRadius = 0.0, bool tie = false){
         this->QueryType = querytype;
         this->K = k;
         this->Radius = radius;
         this->InnerRadius = innerRadius;
         Sample = sample;
         this->Tie = tie;
      }//end SetQueryInfo

      /**
      * Adds information about the query. It is used by Query methods to add
      * information about the query. Since it is optional, not all results will
      * provide meaningful information about it.
      *
      * @param sample The sample object (a copy of it).
      * @param querytype The query type (UNKNOWN, RANGEQUERY, KRANGEQUERY or
      * KNEARESTQUERY)
      * @param k The value of k (if it makes sence).
      * @param radius The value of radius (if it makes sence).
      * @param tie The tie list. Default false;
      *
      * @warning It do not changes the behavior of this result.
      * @see GetQueryType()
      * @see GetK()
      * @see GetRadius()
      * @see GetSample()
      * @see GetTie()
      */
      void SetQueryInfo(tObject * sample, int querytype,
                        u_int32_t k, double radius, bool tie){
         this->SetQueryInfo(sample, querytype, k, radius, 0.0, tie);
      }//end SetQueryInfo

      /**
      * Gets the information about query type. I may assume the values
      * UNKNOWN, RANGEQUERY, KANDRANGEQUERY, KORRANGEQUERY or KNEARESTQUERY.
      */
      int GetQueryType(){
         return this->QueryType;
      }//end GetQueryType

      /**
      * Gets information about k. It makes sense only for KANDRANGEQUERY, KORRANGEQUERY
      * and KNEARESTQUERY.
      */
      unsigned int GetK(){
         return this->K;
      }//end GetK

      /**
      * Gets information about radius. It makes sense only for RANGEQUERY and
      * KRANGEQUERY.
      */
      double GetRadius(){
         return this->Radius;
      }//end GetRadius

      /**
      * Gets information about inner radius. It makes sense only for CROWNQUERY.
      */
      double GetInnerRadius(){
         return this->InnerRadius;
      }//end GetRadius

      /**
      * Gets the sample object if it is available. Since it is an optional
      * information it may not be available.
      *
      * @return The sample object or NULL if it is not available.
      */
      tObject * GetSample(){
         return Sample;
      }//end GetSample

      /**
      * This method tests if two results are equal.
      *
      * @param r1 The second result to be test.
      * @return True if is equal, False otherwise.
      */
      bool IsEqual(const stResultPaged * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
      double Precision(const stResultPaged * r1);


      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
      void Intersection(stResultPaged * result1, stResultPaged * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
      void Union(stResultPaged * result1, stResultPaged * result2);

   private:

      /**
      * The vector of pairs.
      */
      vector < tPair * > Pairs;

      /**
      * Sample object.
      */
      tObject * Sample;

      /**
      * This method locates the insertion position of an object.
      *
      * @param distance The desired distance.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      unsigned int Find(double distance);

};//end stResultPaged

//----------------------------------------------------------------------------
// Class template stJoinedResult
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
class stJoinedResult: public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stJoinedResult Triple used by this class.
      */
      typedef stResultTriple< ObjectType > tTriple;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1).
      */
      stJoinedResult(unsigned int hint = 1){
         // Reserve results
         Triples.reserve(hint);
         // No info
         SetQueryInfo();
      }//end stJoinedResult

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stJoinedResult();

      /**
      * This operator allow the access to a pair.
      */
      tTriple & operator [] (unsigned int idx){
         return (*Triples[idx]);
      }//end operator []

      /**
      * This method returns the number of entries in this result.
      */
      unsigned int GetNumOfEntries(){
         return Triples.size();
      }//end GetNumOfEntries

      /**
      * This method adds a joined pair Object/JoinedObject/distance
      * to this result list.
      *
      * @param obj The object.
      * @param joinedObj The joined object.
      * @param distance The distance from the sample object.
      * @warning There is no duplicate pair checking. All pairs will be added.
      */
      void AddJoinedTriple(tObject * obj, tObject * joinedObj, double distance){
         unsigned int pos;

         pos = this->Find(distance);
         Triples.insert(Triples.begin() + pos, new tTriple(obj, joinedObj, distance));
      }//end AddJoinedPair
      
      /**
      * AddLocalResult
      *
      * @param p The pairs.
      */
      void AddLocalResult(stResult<ObjectType> * local);

      /**
      * AddLocalResult
      *
      * @param obj The object.
      * @param p The pairs.
      */
      void AddLocalResultMaxDist(stResult<ObjectType> * local);

      void AddtJoinedResult(stJoinedResult<ObjectType> * res);
      
      /**
      * This method will remove the last object from this result list.
      */
      void RemoveLast(){

         if (Triples.size() > 0){
            if (Triples[Triples.size() - 1] != NULL){
               delete Triples[Triples.size() - 1];
            }//end if
            Triples.pop_back();
         }//end if
      }//end RemoveLast

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst(){

         if (Triples.size() > 0){
            if (Triples[0] != NULL){
               delete Triples[0];
            }//end if
            Triples.erase(Triples.begin());
         }//end if
      }//end RemoveFirst

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance(){

         if (Triples.size() > 0){
            return Triples[0]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetMinimumDistance

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance(){

         if (Triples.size() > 0){
            return Triples[Triples.size() - 1]->GetDistance();
         }else{
            return -1;
         }//end if
      }//end GetMaximumDistance

      /**
      * This method will cut out undesired objects. It may be used
      * by k-Nearest Neighbour queries to control the number of results.
      *
      * <P>This implementation also treat...
      *
      * @param limit The desired number of results.
      * @todo Review of this explanation.
      */
      void Cut(unsigned int limit);

      /**
      * This method will cut out undesired objects. It may be used
      * by k-Farthest Neighbour queries to control the number of results.
      *
      * <P>This implementation also treat...
      *
      * @param limit The desired number of results.
      * @todo Review of this explanation.
      */
      void CutFirst(unsigned int limit);

      /**
      * Adds information about the query. It is used by Query methods to add
      * information about the query. Since it is optional, not all results will
      * provide meaningful information about it.
      *
      * @param sample The sample object (a copy of it).
      * @param querytype The query type (UNKNOWN, RANGEQUERY, NEARESTQUERY,
      * KANDRANGEQUERY, KORRANGEQUERY, CROWNQUERY, KANDRANGEQUERYESTIMATE or
      * KORRANGEQUERYESTIMATE)
      * @param k The value of k (if it makes sence).
      * @param radius The value of radius (if it makes sence).
      * @param innerRadius The value of inner radius (if it makes sence).
      * @param tie The tie list. Default false;
      *
      * @warning It do not changes the behavior of this result.
      * @see GetQueryType()
      * @see GetK()
      * @see GetRadius()
      * @see GetSample()
      * @see GetTie()
      */
      void SetQueryInfo(int querytype = UNKNOWN,
                        u_int32_t k = 0, double radius = 0.0,
                        double innerRadius = 0.0, bool tie = false){
         this->QueryType = querytype;
         this->K = k;
         this->Radius = radius;
         this->InnerRadius = innerRadius;
         this->Tie = tie;
      }//end SetQueryInfo

      /**
      * Adds information about the query. It is used by Query methods to add
      * information about the query. Since it is optional, not all results will
      * provide meaningful information about it.
      *
      * @param sample The sample object (a copy of it).
      * @param querytype The query type (UNKNOWN, RANGEQUERY, KRANGEQUERY or
      * KNEARESTQUERY)
      * @param k The value of k (if it makes sence).
      * @param radius The value of radius (if it makes sence).
      * @param tie The tie list. Default false;
      *
      * @warning It do not changes the behavior of this result.
      * @see GetQueryType()
      * @see GetK()
      * @see GetRadius()
      * @see GetSample()
      * @see GetTie()
      */
      void SetQueryInfo(int querytype, u_int32_t k, double radius, bool tie){
         this->SetQueryInfo(querytype, k, radius, 0.0, tie);
      }//end SetQueryInfo


      /**
      * This method tests if two results are equal.
      *
      * @param r1 The second result to be test.
      * @return True if is equal, False otherwise.
      */
      bool IsEqual(const stJoinedResult * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
      double Precision(const stJoinedResult * r1);


      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
      void Intersection(stJoinedResult * result1, stJoinedResult * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
      void Union(stJoinedResult * result1, stJoinedResult * result2);

   private:

      /**
      * The vector of triples.
      */
      vector < tTriple * > Triples;

      /**
      * This method locates the insertion position of an object.
      *
      * @param distance The desired distance.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      unsigned int Find(double distance);
      
};//end stJoinedResult


/* Copyright 2003-2017 GBDI-ICMC-USP <caetano@icmc.usp.br>
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*   http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file
*
* This file implements the classes stResult and stResults.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/

//----------------------------------------------------------------------------
// Class template stResult
//----------------------------------------------------------------------------
template < class ObjectType >
stResult<ObjectType>::~stResult(){
   unsigned int i;

   for (i = 0; i < Pairs.size(); i++){
      if (Pairs[i] != NULL){
         delete Pairs[i];
      }//end if
   }//end for

   // Dispose sample object.
   if (Sample != NULL){
      delete Sample;
   }//end if
}//end stResult<ObjectType>::~stResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::AddPair(tObject * obj, double distance){
    unsigned int pos;
    pos = this->Find(distance);
    while((pos < Pairs.size()) && (distance == Pairs[pos]->GetDistance()) &&
            (obj->GetOID() > ((tObject *)(Pairs[pos]->GetObject()))->GetOID())) {
        pos++;
    }//end while
    Pairs.insert(Pairs.begin() + pos, new tPair(obj, distance));
}//end AddPair

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->Tie){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < (int)limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < (int)limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->Tie){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stResult<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
unsigned int stResult<ObjectType>::Find(double distance){
   bool stop;
   unsigned int idx;

   idx = 0;
   stop = (idx >= Pairs.size());
   while (!stop){
      if (Pairs[idx]->GetDistance() < distance){
         idx++;
         stop = (idx >= Pairs.size());
      }else{
         stop = true;
      }//end if
   }//end while

   return idx;
}//end stResult<ObjectType>::Find

//----------------------------------------------------------------------------
template < class ObjectType >
bool stResult<ObjectType>::IsEqual(const stResult * r1){
   ObjectType * tmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Pairs[i]->GetDistance() == r1->Pairs[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Pairs[j]->GetObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = Pairs[i]->GetObject()->IsEqual(tmp);
               // delete the object's copy.
               delete tmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stResult<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Intersection(stResult * result1, stResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stResult<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stResult<ObjectType>::Union(stResult * result1, stResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Pairs[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Pairs[i]->GetObject();
            distance = result1->Pairs[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         distance = result2->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stResult<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stResult<ObjectType>::Precision(const stResult * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Pairs[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (i < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stResult<ObjectType>::Precision

//----------------------------------------------------------------------------
// Class template stResultPaged
//----------------------------------------------------------------------------
template < class ObjectType >
stResultPaged<ObjectType>::~stResultPaged(){
   unsigned int i;

   for (i = 0; i < Pairs.size(); i++){
      if (Pairs[i] != NULL){
         delete Pairs[i];
      }//end if
   }//end for

   // Dispose sample object.
   if (Sample != NULL){
      delete Sample;
   }//end if
}//end stResultPaged<ObjectType>::~stResultPaged

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::AddPair(tObject * obj, double distance){
    unsigned int pos;
    pos = this->Find(distance);
    while((pos < Pairs.size()) && (distance == Pairs[pos]->GetDistance()) &&
            (obj->GetOID() > ((tObject *)(Pairs[pos]->GetObject()))->GetOID())) {
        pos++;
    }//end while
    Pairs.insert(Pairs.begin() + pos, new tPair(obj, distance));
}//end AddPair

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->Tie){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < (int)limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < (int)limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stResultPaged<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->Tie){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stResultPaged<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
unsigned int stResultPaged<ObjectType>::Find(double distance){
   bool stop;
   unsigned int idx;

   idx = 0;
   stop = (idx >= Pairs.size());
   while (!stop){
      if (Pairs[idx]->GetDistance() < distance){
         idx++;
         stop = (idx >= Pairs.size());
      }else{
         stop = true;
      }//end if
   }//end while

   return idx;
}//end stResultPaged<ObjectType>::Find

//----------------------------------------------------------------------------
template < class ObjectType >
bool stResultPaged<ObjectType>::IsEqual(const stResultPaged * r1){
   ObjectType * tmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Pairs[i]->GetDistance() == r1->Pairs[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Pairs[j]->GetObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = Pairs[i]->GetObject()->IsEqual(tmp);
               // delete the object's copy.
               delete tmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stResultPaged<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::Intersection(stResultPaged * result1, stResultPaged * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stResultPaged<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stResultPaged<ObjectType>::Union(stResultPaged * result1, stResultPaged * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Pairs[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Pairs[i]->GetObject();
            distance = result1->Pairs[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Pairs[idx]->GetObject();
         distance = result1->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Pairs[idx]->GetObject();
         distance = result2->Pairs[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stResultPaged<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stResultPaged<ObjectType>::Precision(const stResultPaged * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Pairs[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Pairs[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (i < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stResultPaged<ObjectType>::Precision

//----------------------------------------------------------------------------
// Class template stJoinedResult
//----------------------------------------------------------------------------
template < class ObjectType >
stJoinedResult<ObjectType>::~stJoinedResult(){
   unsigned int i;

   for (i = 0; i < Triples.size(); i++){
      if (Triples[i] != NULL){
         delete Triples[i];
      }//end if
   }//end for
}//end stJoinedResult<ObjectType>::~stJoinedResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::AddLocalResult(stResult<ObjectType> * local){
   for (unsigned i=0; i < local->GetNumOfEntries(); i++){
      AddJoinedTriple(local->GetSample()->Clone(),
        ((ObjectType *)(* local)[i].GetObject())->Clone(),(* local)[i].GetDistance());
   }//end for
}//end stJoinedResult<ObjectType>::AddLocalResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::AddLocalResultMaxDist(stResult<ObjectType> * local){
   for (unsigned i=0; (i < local->GetNumOfEntries()) &&
          ((* local)[i].GetDistance() <= GetMaximumDistance()); i++){
      AddJoinedTriple(local->GetSample()->Clone(),
         ((ObjectType *)(* local)[i].GetObject())->Clone(),(* local)[i].GetDistance());
   }//end for
}//end stJoinedResult<ObjectType>::AddLocalResultMaxDist

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::AddtJoinedResult(stJoinedResult<ObjectType> * res){
   for (unsigned i=0; i < res->GetNumOfEntries(); i++){
      AddJoinedTriple( ((ObjectType *)(* res)[i].GetJoinedObject())->Clone(),
         ((ObjectType *)(* res)[i].GetObject())->Clone(),(* res)[i].GetDistance());
   }//end for
}//end stJoinedResult<ObjectType>::AddtJoinedResult

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Cut(unsigned int limit){
   double max;
   bool stop;
   int i;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->Tie){ // if wants tie list
         // What is the max distance ?
         max = (* this)[limit - 1].GetDistance();

         // I'll cut out everybody which has distance greater than max.
         i = GetNumOfEntries() - 1;
         stop = i < (int)limit;
         while (!stop){
            if ((* this)[i].GetDistance() > max){
               // Cut!
               RemoveLast();
               // The next to check is...
               i--;
               stop = (i < (int)limit);
            }else{
               // Oops! I found someone who will not go out.
               stop = true;
            }//end if
         }//end while
      }else{
         RemoveLast();
      }//end if
   }//end if
}//end stJoinedResult<ObjectType>::Cut

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::CutFirst(unsigned int limit){
   double min;
   bool stop;
   int idx;

   // Will I do something ?
   if (GetNumOfEntries() > limit){
      if (this->Tie){ // if wants tie list
         idx = GetNumOfEntries() - limit;
         // What is the min distance?
         min = (* this)[idx].GetDistance();
         // I'll cut out everybody which has distance lesser than min.
         stop = ((GetNumOfEntries() < limit) || (idx < 0));
         while (!stop){
            if ((* this)[idx].GetDistance() < min){
               // Cut!
               RemoveFirst();
            }//end if
            // The next to check is...
            idx--;
            stop = ((GetNumOfEntries() < limit) || (idx < 0));
         }//end while
      }else{
         RemoveFirst();
      }//end if
   }//end if
}//end stJoinedResult<ObjectType>::CutFirst

//----------------------------------------------------------------------------
template < class ObjectType >
unsigned int stJoinedResult<ObjectType>::Find(double distance){
   bool stop;
   unsigned int idx;

   idx = 0;
   stop = (idx >= Triples.size());
   while (!stop){
      if (Triples[idx]->GetDistance() < distance){
         idx++;
         stop = (idx >= Triples.size());
      }else{
         stop = true;
      }//end if
   }//end while

   return idx;
}//end stJoinedResult<ObjectType>::Find

//----------------------------------------------------------------------------
template < class ObjectType >
bool stJoinedResult<ObjectType>::IsEqual(const stJoinedResult * r1){
   ObjectType * tmp, * joinedTmp;
   bool result, result2;
   unsigned int i, j;
   u_int32_t numObj1, numObj2;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // the default answer.
   result = false;
   // test if two results have the same number of entries and maximum
   // distance.
   if ((this->GetMaximumDistance() == r1->GetMaximumDistance()) &&
       (numObj1 == numObj2)){
      // if there are, test one with the other.
      result = true;
      i = 0;
      // for each object in this class.
      while ((i < numObj1) && result) {
         // set the variables.
         result2 = false;
         // test starting with the first object. 
         j = 0;
         // for each object in the r1 set check until find a equal object.
         while ((j < numObj2) && (!result2)) {
            // check the distance first between the two objects.
            if (Triples[i]->GetDistance() == r1->Triples[j]->GetDistance()) {
               // the distance is equal, now test the object.
               tmp = r1->Triples[j]->GetObject()->Clone();
               joinedTmp = r1->Triples[j]->GetJoinedObject()->Clone();
               // set the result2 with the equality of the two objects.
               result2 = (Triples[i]->GetObject()->IsEqual(tmp) &&
                         Triples[i]->GetJoinedObject()->IsEqual(joinedTmp)) ||
                         (Triples[i]->GetObject()->IsEqual(joinedTmp) &&
                         Triples[i]->GetJoinedObject()->IsEqual(tmp));
               // delete the object's copy.
               delete tmp;
               delete joinedTmp;
            }//end if
            // increment the couter of the second set.
            j++;
         }//end while
         // if the object in the first set was not in the second set, then
         // result will be false, otherwise true.
         result = result && result2;
         // increment the couter of the first set.
         i++;
      }//end while
   }//end if
   // return the result.
   return result;
}//end stJoinedResult<ObjectType>::IsEqual

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Intersection(stJoinedResult * result1,
                                              stJoinedResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj1; idx++){

         // get the object from result1.
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         i = 0;
         // check if the object from result1 is in result2.
         do{
            tmpObj2 = (ObjectType *)result2->Triples[i]->GetObject();
            // is it equal to object1?
            result = tmpObj2->IsEqual(tmpObj1);
            // store if the two objects are equal.
            if (result){
              this->AddPair(tmpObj1->Clone(), distance);
            }//end if
            i++;
         }while ((i < numObj2) && (!result));
      }//end for
   }//end if

}//end stJoinedResult<ObjectType>::Intersection

//----------------------------------------------------------------------------
template < class ObjectType >
void stJoinedResult<ObjectType>::Union(stJoinedResult * result1,
                                       stJoinedResult * result2){
   bool result = false;
   ObjectType * tmpObj1, * tmpObj2;
   unsigned int idx, i;
   u_int32_t numObj1, numObj2;
   double distance;

   numObj1 = result1->GetNumOfEntries();
   numObj2 = result2->GetNumOfEntries();

   if ((numObj1 != 0) && (numObj2 != 0)){
      // put all objects in result1 in unionResult.
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for

      // now put all objects in result2 that are not in result1.
      for (idx = 0; idx < numObj2; idx++){
         // put all the objects in result1 and put in unionResult.
         tmpObj2 = (ObjectType *)result2->Triples[idx]->GetObject();
         // it is storage the distance from result2's representative.
         distance = result2->Triples[idx]->GetDistance();
         // check if the tmpObj2 in result2 is in result1.
         i = 0;
         do{
            tmpObj1 = (ObjectType *)result1->Triples[i]->GetObject();
            distance = result1->Triples[i]->GetDistance();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            i++;
         }while ((i < numObj1) && (!result));
         // if the object2 is not in unionResult put it in unionResult.
         if (!result){
           this->AddPair(tmpObj2->Clone(), distance);
         }//end if
      }//end for
   }else if (numObj1 != 0){
      for (idx = 0; idx < numObj1; idx++){
         tmpObj1 = (ObjectType *)result1->Triples[idx]->GetObject();
         distance = result1->Triples[idx]->GetDistance();
         this->AddPair(tmpObj1->Clone(), distance);
      }//end for
   }else{
      for (idx = 0; idx < numObj2; idx++){
         tmpObj2 = (ObjectType *)result2->Triples[idx]->GetObject();
         distance = result2->Triples[idx]->GetDistance();
         this->AddPair(tmpObj2->Clone(), distance);
      }//end for
   }//end if
}//end stJoinedResult<ObjectType>::Union

//----------------------------------------------------------------------------
template < class ObjectType >
double stJoinedResult<ObjectType>::Precision(const stJoinedResult * r1){
   double result = -1;
   ObjectType * tmpObj1, * tmpObj2;
   int idx, i;
   u_int32_t numObj1, numObj2;
   int equal_count = 0;

   numObj1 = this->GetNumOfEntries();
   numObj2 = r1->GetNumOfEntries();

   // test if two results have the same number of entries
   if ((numObj1 != 0) && (numObj2 != 0)){
      for (idx = 0; idx < numObj2; idx++){
         // get the object from r1.
         tmpObj2 = (ObjectType *)r1->Triples[idx]->GetObject();
         i = 0;
         // check if the object from r1 is in "this".
         do{
            tmpObj1 = (ObjectType *)this->Triples[i]->GetObject();
            // is it equal to object1?
            result = tmpObj1->IsEqual(tmpObj2);
            // if the two objects are equal, increment the "equal_count".
            if (result){
               equal_count++;;
            }//end if
            i++;
         }while (i < numObj1); //end do while
      }//end for
   }//end if
   result = ((double)equal_count / numObj2);
   // return the result.
   return result;
}//end stJoinedResult<ObjectType>::Precision


#endif //__STVECTORRESULT_H
