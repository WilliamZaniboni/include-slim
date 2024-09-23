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
**/

#ifndef __STTREERESULT_H
#define __STTREERESULT_H

#include <set>
#include <vector>


//----------------------------------------------------------------------------
// Class stLessResultPair
//----------------------------------------------------------------------------
/**
* This class implements the less value of two resultPair
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @ingroup struct
*/
template < class ObjectType >
struct stLessResultPair {
  bool operator()(stResultPair < ObjectType > * r1,
                  stResultPair < ObjectType > * r2) const {
    return (r1->GetDistance() < r2->GetDistance());
  }
};

//----------------------------------------------------------------------------
// Class stLessResultPagedPair
//----------------------------------------------------------------------------
/**
* This class implements the less value of two resultPair
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @ingroup struct
*/
template < class ObjectType >
struct stLessResultPagedPair {
  bool operator()(stResultPair < ObjectType > * r1,
                  stResultPair < ObjectType > * r2) const {
    return ( (r1->GetDistance() < r2->GetDistance()) ||
           ( (r1->GetDistance() == r2->GetDistance()) &&
           ( ( (ObjectType *)(r1->GetObject()))->GetOID() < ((ObjectType *)(r2->GetObject()))->GetOID() ) ) );
  }
};

//----------------------------------------------------------------------------
// Class stLessResultTriple
//----------------------------------------------------------------------------
/**
* This class implements the less value of two resultTriple
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @ingroup struct
*/
template < class ObjectType >
struct stLessResultTriple {
  bool operator()(stResultTriple < ObjectType > * r1,
                  stResultTriple < ObjectType > * r2) const {
    return (r1->GetDistance() < r2->GetDistance());
  }
};

//----------------------------------------------------------------------------
// Class stLessResultPagedTriple
//----------------------------------------------------------------------------
/**
* This class implements the less value of two resultTriple
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @ingroup struct
*/
template < class ObjectType >
struct stLessResultPagedTriple {
  bool operator()(stResultTriple < ObjectType > * r1,
                  stResultTriple < ObjectType > * r2) const {
    return ( (r1->GetDistance() < r2->GetDistance()) ||
           ( (r1->GetDistance() == r2->GetDistance()) &&
           ( ( (ObjectType *)(r1->GetObject()))->GetOID() < ((ObjectType *)(r2->GetObject()))->GetOID() ) ) );
  }
};

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

      typedef std::multiset < tPair *, stLessResultPair< ObjectType > > tPairs;
      
      typedef typename tPairs::iterator tItePairs;

      /**
      * This method will create a new instance of this class.
      */
      stResult(){
         // No info
         SetQueryInfo();
         Sample = NULL;
         allpairs = NULL;
      }//end stResult

      /**
      * This method will create a new instance of this class.
      *
      * @warning Deprecated Method.
      */
      stResult(int k){
         // No info
         SetQueryInfo();
         Sample = NULL;
         this->K = k;
         allpairs = NULL;
      }//end stResult

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stResult();

      /**
      * Iterator to begin of list.
      */
      tItePairs beginPairs(){
         return Pairs.begin();
      }//end iteratorPair
      
      /**
      * Iterator to end of list.
      *
      * @warning Decrement the iterator for get the last element.
      */
      tItePairs endPairs(){
         return Pairs.end();
      }//end iteratorPair

      /**
      * Return the container Pairs
      */
      tPairs * GetPairs(){
         return &Pairs;
      }// end GetPairs

      /**
      * Gets a pair by its position. Users should use function GetPair instead!
      *
      * @return The stResultPair pair
      */
      tPair & operator [] (unsigned int idx) {
          if (allpairs == NULL) {
              allpairs = new tPair*[GetNumOfEntries()];
              int pos = 0;
              for (tItePairs ite = this->beginPairs(); ite != this->endPairs(); ite++) {
                  allpairs[pos] = (*ite);
                  pos++;
              }
          }
          if (idx < GetNumOfEntries())
              return *allpairs[idx];
//          else
//              return NULL;
      }

      /**
      * Gets a pair by its position. It replaces the deprecated operator [].
      *
      * @return The stResultPair pair
      */
      tPair *GetPair(int idx) {
          if (allpairs == NULL) {
              allpairs = new tPair*[GetNumOfEntries()];
              int pos = 0;
              for (tItePairs ite = this->beginPairs(); ite != this->endPairs(); ite++) {
                  allpairs[pos] = (*ite);
                  pos++;
              }
          }
          if (idx < GetNumOfEntries())
              return allpairs[idx];
          else
              return NULL;
      }

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
      void AddPair(tObject * obj, double distance){
         Pairs.insert(Pairs.begin(), new tPair(obj, distance));
         if (allpairs != NULL)
             delete[] allpairs;
         allpairs = NULL;
      }//end AddPair

      /**
      * This method will remove the last object from this result list.
      */
      void RemoveLast();

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst();

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance();

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance();

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
         this->Sample = sample;
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
//      bool IsEqual(const stResult * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
//      double Precision(const stResult * r1);

      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Intersection(stResult * result1, stResult * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Union(stResult * result1, stResult * result2);

   private:

      /**
      * The red-black tree of pairs.
      */
      tPairs Pairs;

      /**
      * Sample object.
      */
      tObject * Sample;

      /**
      * The vector of pairs
      */
      stResultPair<tObject> **allpairs;
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
* @author Enzo Seraphim (seraphim@icmc.usp.br)
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

      typedef std::multiset < tPair *, stLessResultPagedPair< ObjectType > > tPairs;
      
      typedef typename tPairs::iterator tItePairs;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1).
      */
      stResultPaged(){
         // No info
         SetQueryInfo();
         Sample = NULL;
      }//end stResult

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stResultPaged();

      /**
      * Iterator to begin of list.
      */
      tItePairs beginPairs(){
         return Pairs.begin();
      }//end beginPairs
      
      /**
      * Iterator to end of list.
      *
      * @warning Decrement the iterator for get the last element.
      */
      tItePairs endPairs(){
         return Pairs.end();
      }//end endPairs

      /**
      * Return the container Pairs
      */
      tPairs * GetPairs(){
         return &Pairs;
      }// end GetPairs

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
      void AddPair(tObject * obj, double distance){
         Pairs.insert(Pairs.begin(), new tPair(obj, distance));
      }//end AddPair

      /**
      * This method will remove the last object from this result list.
      */
      void RemoveLast();

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst();

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance();

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance();

      /**
      * This method returns the last oid of the objects in this result
      * list. 
      */
      long GetLastOID();

      /**
      * This method returns the first oid of the objects in this result
      * list.
      */
      long GetFirstOID();

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
         this->Sample = sample;
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
//      bool IsEqual(const stResult * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
//      double Precision(const stResult * r1);

      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Intersection(stResult * result1, stResult * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Union(stResult * result1, stResult * result2);

   private:

      /**
      * The vector of pairs.
      */
      tPairs Pairs;

      /**
      * Sample object.
      */
      tObject * Sample;

};//end stResultPaged

//----------------------------------------------------------------------------
// Class template stJoinedResult
//----------------------------------------------------------------------------
/**
* This class implements a query result. It will hold a set of triples
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
* @author Enzo Seraphim (seraphim@icmc.usp.br)
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

      typedef std::multiset < tTriple *, stLessResultTriple< ObjectType > > tTriples;

      typedef typename tTriples::iterator tIteTriples;
      
      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1).
      */
      stJoinedResult(){
         // No info
         SetQueryInfo();
      }//end stJoinedResult

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stJoinedResult();

      /**
      * Iterator to begin of list.
      */
      tIteTriples beginTriples(){
         return Triples.begin();
      }//end beginTriples
      
      /**
      * Iterator to end of list.
      *
      * @warning Decrement the iterator for get the last element.
      */
      tIteTriples endTriples(){
         return Triples.end();
      }//end endTriples

      /**
      * Return the container Triples
      */
      tTriples * GetTriples(){
         return &Triples;
      }// end GetTriples

      /**
      * This method returns the number of entries in this result.
      */
      unsigned int GetNumOfEntries(){
         return Triples.size();
      }//end GetNumOfEntries

      /**
      * This method adds a joined triple Object/JoinedObject/distance
      * to this result list.
      *
      * @param obj The object.
      * @param joinedObj The joined object.
      * @param distance The distance from the sample object.
      * @warning There is no duplicate triple checking. All triples will be added.
      */
      void AddJoinedTriple(tObject * obj, tObject * joinedObj, double distance){
         Triples.insert(Triples.begin(), new tTriple(obj, joinedObj, distance));
      }//end AddJoinedTriple

      /**
      * AddLocalResult
      *
      */
//      void AddLocalResult(stResult<ObjectType> * local);

      /**
      * AddLocalResult
      *
      * @param p The pairs.
      */
//      void AddLocalResultMaxDist(stResult<ObjectType> * local);

//      void AddtJoinedResult(stJoinedResult<ObjectType> * res);

      /**
      * This method will remove the last object from this result list.
      */
      void RemoveLast();

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst();

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance();

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance();

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
  void SetQueryInfo(int querytype, u_int32_t k, double radius, bool tie) {
    this->SetQueryInfo(querytype, k, radius, 0.0, tie);
  }//end SetQueryInfo

      /**
      * This method tests if two results are equal.
      *
      * @param r1 The second result to be test.
      * @return True if is equal, False otherwise.
      */
//      bool IsEqual(const stJoinedResult * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
//      double Precision(const stJoinedResult * r1);

      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Intersection(stJoinedResult * result1, stJoinedResult * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Union(stJoinedResult * result1, stJoinedResult * result2);

   private:

      /**
      * The vector of triples.
      */
      tTriples Triples;

};//end stJoinedResult

//----------------------------------------------------------------------------
// Class template stJoinedResultPaged
//----------------------------------------------------------------------------
/**
* This class implements a query result. It will hold a set of triples
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
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @version 1.1
* @ingroup struct
*/
template < class ObjectType >
class stJoinedResultPaged: public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stJoinedResult Triple used by this class.
      */
      typedef stResultTriple< ObjectType > tTriple;

      typedef std::multiset < tTriple *, stLessResultPagedTriple< ObjectType > > tTriples;

      typedef typename tTriples::iterator tIteTriples;
      
      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1).
      */
      stJoinedResultPaged(){
         // No info
         SetQueryInfo();
      }//end stJoinedResult

      /**
      * This method disposes this instance and releases all allocated resources.
      */
      virtual ~stJoinedResultPaged();

      /**
      * Iterator to begin of list.
      */
      tIteTriples beginTriples(){
         return Triples.begin();
      }//end beginTriples
      
      /**
      * Iterator to end of list.
      *
      * @warning Decrement the iterator for get the last element.
      */
      tIteTriples endTriples(){
         return Triples.end();
      }//end endTriples

      /**
      * Return the container Triples
      */
      tTriples * GetTriples(){
         return &Triples;
      }// end GetTriples

      /**
      * This method returns the number of entries in this result.
      */
      unsigned int GetNumOfEntries(){
         return Triples.size();
      }//end GetNumOfEntries

      /**
      * This method adds a joined triple Object/JoinedObject/distance
      * to this result list.
      *
      * @param obj The object.
      * @param joinedObj The joined object.
      * @param distance The distance from the sample object.
      * @warning There is no duplicate pair checking. All pairs will be added.
      */
      void AddJoinedTriple(tObject * obj, tObject * joinedObj, double distance){
         Triples.insert(Triples.begin(), new tTriple(obj, joinedObj, distance));
      }//end AddJoinedTriple

      /**
      * AddLocalResult
      *
      */
//      void AddLocalResult(stResult<ObjectType> * local);

      /**
      * AddLocalResult
      *
      */
//      void AddLocalResultMaxDist(stResult<ObjectType> * local);

//      void AddtJoinedResult(stJoinedResult<ObjectType> * res);

      /**
      * This method will remove the last object from this result list.
      */
      void RemoveLast();

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst();

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance();

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance();

      /**
      * This method returns the last oid of the objects in this result
      * list. 
      */
      long GetLastOID();

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
//      bool IsEqual(const stJoinedResult * r1);

      /**
      * This method tests the similarity between two results .
      *
      * @param r1 The second result to be test.
      * @return the percent-similarity of the two results.
      */
//      double Precision(const stJoinedResult * r1);

      /**
      * This method implements the intersection operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Intersection(stJoinedResult * result1, stJoinedResult * result2);

      /**
      * This method implements the union operator between two results.
      * @param r1 The first result set.
      * @param r2 The second result set.
      */
//      void Union(stJoinedResult * result1, stJoinedResult * result2);

   private:

      /**
      * The vector of triples.
      */
      tTriples Triples;

};//end stJoinedResultPaged



template < class ObjectType >
struct stLessIteResultPair {
  bool operator()(typename std::multiset < stResultPair< ObjectType > *, stLessResultPair< ObjectType > >::iterator r1,
      typename std::multiset < stResultPair< ObjectType > *, stLessResultPair< ObjectType > >::iterator r2) const {
    return ((*r1)->GetDistance() < (*r2)->GetDistance());
  }
};

#ifdef __stCKNNQ__
//----------------------------------------------------------------------------
// Class template stResultConstrained
//----------------------------------------------------------------------------

/**
 * This class implements a constrained query result. It will hold a set of pairs
 * Object/Distance ordered by distance which constitutes the answer
 * to a query. All query methods of all metric trees implemented by
 * this library will return instances of this class.
 *
 * @author Willian Dener de Oliveira (willian@icmc.usp.br)
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @version 1.0
 * @ingroup struct
 */
template < class ObjectType >
class stConstrainedResult : public stResult < ObjectType > {
public:

  /**
   * Type of the object.
   */
  typedef ObjectType tObject;

  /**
   * This type defines the stResult Pair used by this class.
   */
  typedef stResultPair< ObjectType > tPair;

  typedef std::multiset < tPair *, stLessResultPair< ObjectType > > tPairs;

  typedef typename tPairs::iterator tItePairs;

  typedef typename std::multiset< tItePairs, stLessIteResultPair< ObjectType > >::iterator tIteSPairs;

  /**
   * This method will create a new instance of this class.
   */
  stConstrainedResult():stResult<ObjectType>() {
      addPairCount = 0;
  }//end stResult

  /**
   * This method will create a new instance of this class.
   *
   * @warning Deprecated Method.
   */
  stConstrainedResult(int k):stResult<ObjectType>(k){
      addPairCount = 0;
  }//end stResult

  u_int32_t getAddPairCount() {
      return addPairCount;
  }

  /**
   * This method disposes this instance and releases all allocated resources.
   */
  virtual ~stConstrainedResult(){};

  tIteSPairs beginSatisfyPairs(){
    return satisfyPairs.begin();
  }

  tIteSPairs endSatisfyPairs(){
    return satisfyPairs.end();
  }

  tPair* GetLastSatisfyPair(){
    return *(*(satisfyPairs.rbegin()));
  }

  tPair* GetLastNotSatisfyPair(){
    return *(*(notSatisfyPairs.rbegin()));
  }

    /**
   * This method returns the number of entries in this result.
   */
  unsigned int GetNumOfEntriesSatisfyPairs() {
    return satisfyPairs.size();
  }//end GetNumOfEntries

    /**
   * This method returns the number of entries in this result.
   */
  unsigned int GetNumOfEntriesNotSatisfyPairs() {
    return notSatisfyPairs.size();
  }//end GetNumOfEntries

  /**
   * This method adds a pair Object/Distance to this result list.
   *
   * @param obj The object.
   * @param distance The distance from the sample object.
   * @warning There is no duplicate pair checking. All pairs will be added.
   */
  void AddSatisfyPair(tObject * obj, double distance) {
    tItePairs tmp = ::stResult<ObjectType>::Pairs.insert(::stResult<ObjectType>::Pairs.begin(), new tPair(obj, distance));
    satisfyPairs.insert(satisfyPairs.begin(),tmp);
    if (::stResult<ObjectType>::allpairs != NULL)
      delete[] ::stResult<ObjectType>::allpairs;
    ::stResult<ObjectType>::allpairs = NULL;

    addPairCount++;
  }//end AddPair

    /**
   * This method adds a pair Object/Distance to this result list.
   *
   * @param obj The object.
   * @param distance The distance from the sample object.
   * @warning There is no duplicate pair checking. All pairs will be added.
   */
  void AddNotSatisfyPair(tObject * obj, double distance) {
    tItePairs tmp = ::stResult<ObjectType>::Pairs.insert(::stResult<ObjectType>::Pairs.begin(), new tPair(obj, distance));
    notSatisfyPairs.insert(notSatisfyPairs.begin(),tmp);
    if (::stResult<ObjectType>::allpairs != NULL)
      delete[] ::stResult<ObjectType>::allpairs;
    ::stResult<ObjectType>::allpairs = NULL;

    addPairCount++;
  }//end AddPair



  /**
   * This method will remove the last object from this result list.
   */
  void RemoveLast();

  void RemoveLastSatisfyPairs();

  void RemoveLastNotSatisfyPairs();

  void RemoveSatisfyPairs(tIteSPairs it);

  void ToNotSatisfyPairs(tIteSPairs it);


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

  void CutSatisfyPairs(unsigned int limit);


  void CutNotSatisfyPairs(unsigned int limit);

private:

  std::multiset<tItePairs, stLessIteResultPair< ObjectType > >  satisfyPairs;
  std::multiset<tItePairs, stLessIteResultPair< ObjectType > > notSatisfyPairs;

  u_int32_t addPairCount;

}; //end stConstrainedResult
#endif //__stCKNNQ__


//----------------------------------------------------------------------------
// Comparation function employed in class stTOResult
//----------------------------------------------------------------------------

template < class ObjectType, class KeyType, class Comparator = std::less<KeyType> >
struct stLessTOResultPair {
  bool operator()(stResultPair< ObjectType, KeyType > * r1,
                  stResultPair< ObjectType, KeyType > * r2) const {
    return Comparator()(r1->GetKey(), r2->GetKey());
  }
};

//----------------------------------------------------------------------------
// Class template stTOResult
//----------------------------------------------------------------------------

/**
 * This class implements a total order query result. It will hold a set of pairs
 * Object/Key ordered by the key which constitutes the answer
 * to a query. All query methods of all total order access methods implemented by
 * this library will return instances of this class.
 *
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @version 1.0
 * @ingroup struct
 */
template < class ObjectType, class KeyType, class Comparator = std::less<KeyType> >
class stTOResult {
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
   * This type defines the stResult Pair used by this class.
   */
  typedef stResultPair< tObject, tKey > tPair;

  typedef std::multiset < tPair *, stLessTOResultPair<ObjectType, KeyType, Comparator> > tPairs;

  typedef typename tPairs::iterator tItePairs;

  /**
   * This method will create a new instance of this class.
   */
  stTOResult() {
    // No info
    // SetQueryInfo(); /* Gives an error to B+-Tree implementation */
    allpairs = NULL;
  }//end stResult

  /**
   * This method disposes this instance and releases all allocated resources.
   */
  virtual ~stTOResult() {
    while (Pairs.size() > 0) {
      tItePairs ite = Pairs.end();
      ite--; //last element
      delete *ite;
      Pairs.erase(ite);
    }//end while
    // Dispose vector of pairs.
    if (allpairs != NULL)
      delete[] allpairs;
    allpairs = NULL;
  }

  /**
   * Iterator to begin of list.
   */
  tItePairs beginPairs() {
    return Pairs.begin();
  }//end iteratorPair

  /**
   * Iterator to end of list.
   *
   * @warning Decrement the iterator for get the last element.
   */
  tItePairs endPairs() {
    return Pairs.end();
  }//end iteratorPair

  /**
   * Return the container Pairs
   */
  tPairs * GetPairs() {
    return &Pairs;
  }// end GetPairs

  /**
   * Gets a pair by its position. Users should use function GetPair instead!
   *
   * @return The stResultPair pair
   */
  tPair & operator [] (u_int32_t idx) {
      return GetPair(idx);
  }

  /**
   * Gets a pair by its position. It replaces the deprecated operator [].
   *
   * @return The stResultPair pair
   */
  tPair *GetPair(u_int32_t idx) {
    if (allpairs == NULL) {
      allpairs = new tPair*[GetNumOfEntries()];
      u_int32_t pos = 0;
      for (tItePairs ite = this->beginPairs(); ite != this->endPairs(); ite++) {
        allpairs[pos] = (*ite);
        pos++;
      }
    }
    if (idx < GetNumOfEntries())
      return allpairs[idx];
    else
      return NULL;
  }

  /**
   * This method returns the number of entries in this result.
   */
  unsigned int GetNumOfEntries() {
    return Pairs.size();
  }//end GetNumOfEntries

  /**
   * This method adds a pair Object/Key to this result list.
   *
   * @param obj The object.
   * @param key The search key.
   * @warning There is no duplicate pair checking. All pairs will be added.
   */
  tItePairs AddPair(tObject * obj, tKey key) {
    tItePairs it;
    it = Pairs.insert(Pairs.begin(), new tPair(obj, key));
    if (allpairs != NULL)
      delete[] allpairs;
    allpairs = NULL;
    return it;
  }//end AddPair

  /**
   * This method adds a pair Object/Key to this result list. It allows providing
   * an iterator to perform the insertion in amortized constant time.
   *
   * @param obj The object.
   * @param key The search key.
   * @param key The search key.
   * @warning There is no duplicate pair checking. All pairs will be added.
   */
  tItePairs AddPair(tObject * obj, tKey key, tItePairs it) {
    tItePairs outIt;
    outIt = Pairs.insert(it, new tPair(obj, key));
    if (allpairs != NULL)
      delete[] allpairs;
    allpairs = NULL;
    return outIt;
  }//end AddPair

  /**
   * Adds information about the query. It is used by Query methods to add
   * information about the query. Since it is optional, not all results will
   * provide meaningful information about it.
   *
   * @param op The query type (UNKNOWN, TO_LESSTHAN, TO_LESSTHANOREQUAL,
   * TO_EQUAL, TO_GREATERTHANOREQUAL, TO_GREATERTHAN, TO_BETWEEN)
   * @param lowerBound The query lower bound (if it makes sense).
   * @param upperBound The query upper bound (if it makes sense).
   *
   * @warning It do not changes the behavior of this result.
   * @see GetOperator()
   * @see GetLowerBound()
   * @see GetUpperBound()
   */
  void SetQueryInfo(int queryType = UNKNOWN, tKey lowerBound = 0, tKey upperBound = 0) {
    this->QueryType = queryType;
    this->LowerBound = lowerBound;
    this->UpperBound = upperBound;
  }//end SetQueryInfo

  /**
   * Gets the query operator if it is available. Since it is an optional
   * information it may not be available.
   *
   * @return The query operator or UNKNOWN if it is not available.
   */
  int GetQueryType() {
      return QueryType;
  }

  /**
   * Gets the lower bound if it is available. Since it is an optional
   * information it may not be available.
   *
   * @return The lower bound or NULL if it is not available.
   */
  tKey GetLowerBound() {
    return LowerBound;
  }//end GetSample

  /**
   * Gets the upper bound if it is available. Since it is an optional
   * information it may not be available.
   *
   * @return The upper bound or NULL if it is not available.
   */
  tKey GetUpperBound() {
    return UpperBound;
  }//end GetSample


protected:

  /**
   * The red-black tree of pairs.
   */
  tPairs Pairs;

  /**
   * Query lower bound.
   */
  tKey LowerBound;

  /**
   * Query upper bound.
   */
  tKey UpperBound;

  /**
   * Information about QueryType. It is UNKNOWN or TO_* (see tQueryType in file
   * stResult.h).
   */
  int QueryType;

  /**
   * The vector of pairs
   */
  tPair **allpairs;
}; //end stResult


#include "stTreeResult-inl.h"

#endif //__STTREERESULT_H
