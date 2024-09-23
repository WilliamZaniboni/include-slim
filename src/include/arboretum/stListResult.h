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
#ifndef __STLISTRESULT_H
#define __STLISTRESULT_H



//----------------------------------------------------------------------------
// Class template stResult
//----------------------------------------------------------------------------
/**
* This class implements a query result using a Link List instead of
* a vector like the stResultNewAlloc. It will hold a set of pairs
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
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.1
* @ingroup struct
*/
template < class ObjectType >
class stResult : public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResultPair used by this class.
      */
      typedef stResultPair < ObjectType > tPair;

      /**
      * This type defines the stResultPair used by this class.
      */
      typedef stResultTriple < ObjectType > tTriple;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1). This parameter
      * is used only to standard proposes.
      */
      stResult(unsigned int hint = 1){
         // Reserve results
         First = NULL;
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
         tNode * temp;
         if (idx < Size){
            temp = First;
            for (int i = 0; i < idx; i++){
               temp = temp->Next;
            }//end if
            return temp->Pair;
         }//end if
      }//end operator []

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
         // Is there one entry?
         if (Size > 0){
            // If there is only one entry.
            if (Size == 1){
               delete First;
               First = NULL;
               // Update the MaximumDistance.
               MaximumDistance = -1.0;
            }else{
               tNode * node = First;
               while (node->Next->Next){
                  node = node->Next;
               }//end if
               delete node->Next;
               node->Next = NULL;
               // Update the MaximumDistance.
               MaximumDistance = node->Pair.GetDistance();
            }//end if
            Size--;
         }//end if
      }//end RemoveLast

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst(){
         if (Size > 0){
            if (Size == 1){
               delete First;
               First = NULL;
               // Update the MinimumDistance.
               MinimumDistance = MAXDOUBLE;
            }else{
               tNode * temp = First;
               First = First->Next;
               // Update the MinimumDistance.
               MinimumDistance = First->Pair.GetDistance();
               delete temp;
            }//end if
            Size--;
         }//end if
      }//end RemoveFirst

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
      * KANDRANGEQUERY, KORRANGEQUERY, RINGQUERY, KANDRANGEQUERYESTIMATE or
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
      * This structure is a node of the link list.
      */
      typedef struct stNode{
         /**
         * The Pair.
         */
         tPair Pair;

         /**
         * The pointer for a pair.
         */
         stNode * Next;
      }tNode;

      /**
      * Head of the link list.
      */
      tNode * First;

      /**
      * Sample object (Optional).
      */
      tObject * Sample;

      /**
      * This method locates the insertion position of an object.
      *
      * @param distance The desired distance.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      tNode * Find(double distance);
      
};//end stResult

//----------------------------------------------------------------------------
// Class template stResultPaged
//----------------------------------------------------------------------------
/**
* This class implements a query result using a Link List instead of
* a vector like the stResultNewAlloc. It will hold a set of pairs
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
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.1
* @ingroup struct
*/
template < class ObjectType >
class stResultPaged : public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResultPair used by this class.
      */
      typedef stResultPair < ObjectType > tPair;

      /**
      * This type defines the stResultPair used by this class.
      */
      typedef stResultTriple < ObjectType > tTriple;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1). This parameter
      * is used only to standard proposes.
      */
      stResultPaged(unsigned int hint = 1){
         // Reserve results
         First = NULL;
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
         tNode * temp;
         if (idx < Size){
            temp = First;
            for (int i = 0; i < idx; i++){
               temp = temp->Next;
            }//end if
            return temp->Pair;
         }//end if
      }//end operator []

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
         // Is there one entry?
         if (Size > 0){
            // If there is only one entry.
            if (Size == 1){
               delete First;
               First = NULL;
               // Update the MaximumDistance.
               MaximumDistance = -1.0;
            }else{
               tNode * node = First;
               while (node->Next->Next){
                  node = node->Next;
               }//end if
               delete node->Next;
               node->Next = NULL;
               // Update the MaximumDistance.
               MaximumDistance = node->Pair.GetDistance();
            }//end if
            Size--;
         }//end if
      }//end RemoveLast

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst(){
         if (Size > 0){
            if (Size == 1){
               delete First;
               First = NULL;
               // Update the MinimumDistance.
               MinimumDistance = MAXDOUBLE;
            }else{
               tNode * temp = First;
               First = First->Next;
               // Update the MinimumDistance.
               MinimumDistance = First->Pair.GetDistance();
               delete temp;
            }//end if
            Size--;
         }//end if
      }//end RemoveFirst

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
      * KANDRANGEQUERY, KORRANGEQUERY, RINGQUERY, KANDRANGEQUERYESTIMATE or
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
      * This structure is a node of the link list.
      */
      typedef struct stNode{
         /**
         * The Pair.
         */
         tPair Pair;

         /**
         * The pointer for a pair.
         */
         stNode * Next;
      }tNode;

      /**
      * Head of the link list.
      */
      tNode * First;

      /**
      * Sample object (Optional).
      */
      tObject * Sample;

      /**
      * This method locates the insertion position of an object.
      *
      * @param distance The desired distance.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      tNode * Find(double distance);
      
};//end stResultPaged

//----------------------------------------------------------------------------
// Class template stJoinedResult
//----------------------------------------------------------------------------
/**
* This class implements a query result using a Link List instead of
* a vector like the stResultNewAlloc. It will hold a set of pairs
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
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.1
* @ingroup struct
*/
template < class ObjectType >
class stJoinedResult : public stBasicResult < ObjectType > {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the stResultPair used by this class.
      */
      typedef stResultTriple < ObjectType > tTriple;

      /**
      * This method will create a new instance of this class. The parameter hint
      * is used to prepare this instance to hold at least <i>hint</i> results
      * (it is not a upper bound limit).
      *
      * @param hint The projected number of results (default = 1). This parameter
      * is used only to standard proposes.
      */
      stJoinedResult(unsigned int hint = 1){
         // Reserve results
         First = NULL;
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
         tNode * temp;
         if (idx < Size){
            temp = First;
            for (int i = 0; i < idx; i++){
               temp = temp->Next;
            }//end if
            return temp->Triple;
         }//end if
      }//end operator []

      /**
      * This method adds a pair Object/Distance to this result list.
      *
      * @param obj The object.
      * @param distance The distance from the sample object.
      * @warning There is no duplicate pair checking. All pairs will be added.
      */
      void AddJoinedTriple(tObject * obj, tObject * joinedObj, double distance){
         tNode * pos;
         tNode * newNode;
         unsigned int i;
         bool stop;

         pos = this->Find(distance);
         // Add the node.
         newNode = new tNode;
         // Add the new triple.
         newNode->Triple.SetObject(obj);
         newNode->Triple.SetJoinedObject(joinedObj);
         newNode->Triple.SetDistance(distance);
         // set the pointers.
         if (pos == NULL){
            newNode->Next = First;
            First = newNode;
         }else{
            newNode->Next = pos->Next;
            pos->Next = newNode;
         }//end if
         // Update the distances.
         if (distance < MinimumDistance){
            MinimumDistance = distance;
         }//end if
         if (distance > MaximumDistance){
            MaximumDistance = distance;
         }//end if
         // update the counter.
      	Size++;
      }//end AddJoinedTriple

      /**
      * This method will remove the last object from this result list.
      */
      void RemoveLast(){
         // Is there one entry?
         if (Size > 0){
            // If there is only one entry.
            if (Size == 1){
               delete First;
               First = NULL;
               // Update the MaximumDistance.
               MaximumDistance = -1.0;
            }else{
               tNode * node = First;
               while (node->Next->Next){
                  node = node->Next;
               }//end if
               delete node->Next;
               node->Next = NULL;
               // Update the MaximumDistance.
               MaximumDistance = node->Triple.GetDistance();
            }//end if
            Size--;
         }//end if
      }//end RemoveLast

      /**
      * This method will remove the first object from this result list.
      */
      void RemoveFirst(){
         if (Size > 0){
            if (Size == 1){
               delete First;
               First = NULL;
               // Update the MinimumDistance.
               MinimumDistance = MAXDOUBLE;
            }else{
               tNode * temp = First;
               First = First->Next;
               // Update the MinimumDistance.
               MinimumDistance = First->Triple.GetDistance();
               delete temp;
            }//end if
            Size--;
         }//end if
      }//end RemoveFirst

      /**
      * This method returns the minimum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMinimumDistance(){
         if (Size > 0){
            return MinimumDistance;
         }else{
            return -1;
         }//end if
      }//end GetMinimumDistance

      /**
      * This method returns the maximum distance of the objects in this result
      * list. If this result is empty, it will return a negative value.
      */
      double GetMaximumDistance(){
         if (Size > 0){
            return MaximumDistance;
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
      * KANDRANGEQUERY, KORRANGEQUERY, RINGQUERY, KANDRANGEQUERYESTIMATE or
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
      void SetQueryInfo(int querytype = UNKNOWN, u_int32_t k = 0,
                        double radius = 0.0, double innerRadius = 0.0,
                        bool tie = false){
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
      * Gets information about inner radius. It makes sense only for RINGQUERY.
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
      * Gets the tie list to the query. It is an optional
      * information.
      *
      * @return True if there is tie list or False otherwise.
      */
      bool GetTie(){
         return this->Tie;
      }//end GetTie

      /**
      * This method tests if two results are equal.
      *
      * @param r1 The second result to be test.
      * @return True if is equal, False otherwise.
      */
      bool IsEqual(const stJoinedResult * r1);

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
      * This structure is a node of the link list.
      */
      typedef struct stNode{
         /**
         * The Triple.
         */
         tTriple Triple;

         /**
         * The pointer for a pair.
         */
         stNode * Next;
      }tNode;

      /**
      * Head of the link list.
      */
      tNode * First;

      /**
      * This method locates the insertion position of an object.
      *
      * @param distance The desired distance.
      * @todo This code needs optimizations. I suggest a binary search
      * implementation.
      */
      tNode * Find(double distance);
      
};//end stJoinedResult

#include "stListResult-inl.h"

#endif //__STLISTRESULT_H
