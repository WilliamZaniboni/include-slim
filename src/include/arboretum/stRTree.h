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
* This file defines the class stRTree.
*/

#ifndef __STRTREE_H
#define __STRTREE_H

#include <arboretum/stUtil.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stGenericPriorityQueue.h>
#include <arboretum/stResult.h>
#include <arboretum/stRNode.h>
// #include <arboretum/deprecated/stBasicMetricEvaluators.h>

// this is used to set the initial size of the dynamic queue
#ifndef STARTVALUEQUEUE
   #define STARTVALUEQUEUE 100
#endif //STARTVALUEQUEUE
// this is used to set the increment size of the dynamic queue
#ifndef INCREMENTVALUEQUEUE
   #define INCREMENTVALUEQUEUE 5
#endif //INCREMENTVALUEQUEUE

//=============================================================================
// Class template stRTree
//-----------------------------------------------------------------------------
/**
* This class defines the behavior of the R*tree (R star).
* This R-tree uses as a base class the stBasicArrayObject, then
* the template receives as parameters the DataType of the array and the OIDType
* of an element.
*
* By definition, each stBasicArrayObject knows its own dimensionality. This tree
* was implemented based on the idea that the user is responsable for adding elements
* with the same dimensionality. Unknown problems may occur if the user does not
* guarantee that.
*
* Based on the textbook by Yannis Manolopoulos, Alexandros Nanopoulos, Apostolos N.
* Papadopoulos, Yannis Theodoridis. "R-Trees: Theory and Applications", Springer, 2006.
* http://www.springeronline.com/sgw/cda/frontpage/0,11855,5-40007-22-45084633-0,00.html
*
* Some methods were based on the source code by Dimistris Papadias available at
* www.rtreeportal.org
*
* @author Humberto Razente
* @version 1.0
*/
template <class DataType, class OIDType = int>
class stRTree {
   public:
      /**
      * This structure defines the Rtree header structure. This type was left
      * public to allow the creation of debug tools.
      */
      typedef struct tRHeader{
         /**
         * Magic number. This is a short string that must contains the magic
         * string "RSTR". It may be used to validate the file.
         */
         char Magic[4];

         /**
         * Split node method.
         */
         int SplitMethod;

         /**
         * The root of the R-tree
         */
         u_int32_t Root;

         /**
         * The height of the tree
         */
         u_int32_t Height;

         /**
         * Total number of objects.
         */
         u_int32_t ObjectCount;

         /**
         * Total number of nodes.
         */
         u_int32_t NodeCount;
      } stRHeader;

      /**
      * Defines the object to be indexed
      */
      typedef stBasicArrayObject <DataType, OIDType> basicArrayObject;

      /**
      * This is the class that abstracs an result set for simple queries.
      */
      typedef stResult <basicArrayObject> tResult;

      /**
      * Defines the abstract metric evaluator
      */
      typedef stBasicMetricEvaluator < basicArrayObject > rBasicMetricEvaluator;

      /**
      * Defines the euclidean metric evaluator
      */
      typedef stBasicEuclideanMetricEvaluator < basicArrayObject > rEuclideanBasicMetricEvaluator;

      /**
      * This type is used by the priority key in NearestQuery.
      */
      typedef stDynamicRPriorityQueue < double, stQueryPriorityQueueValue > tDynamicPriorityQueue;

      /**
      * These constants are used to define the split method.
      */
      enum tSplitMethod {
         /**
         * The linear split chooses two objects as seeds for the two nodes, as far
         * apart from each other as possible. Then consider each remaining object
         * in a random order and assign it to the node requiring the smallest MBR
         * enlargement.
         */
         smLINEAR,

         /**
         * The quadratic split chooses two objects as seeds for the two nodes, where
         * these objects if put together create as much dead space as possible (i.e.
         * the space that remains from the MBR if the areas of the two objects are
         * ignored). Then, until there are no remaining objects, insert the object
         * for which the difference of dead space if assigned to each of the two
         * nodes is maximized in the node that requires less enlargement of its MBR.
         * [Guttman, 1984] suggested using this method as a good compromise to achieve
         * reasonable retrieval performance.
         */
         smQUADRATIC,

         /**
         * In the exponential split all possible groupings are exhaustively tested
         * and the best is chosen with respect to the minimization of the MBR
         * enlargement.
         */
         smEXPONENTIAL
      };//end tSplitMethod

      /**
      * Creates a new R tree using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this R tree.
      */
      stRTree(stPageManager * pageman);

      /**
      * Dispose all used resources, ie, it is the destructor method.
      *
      * @see stRTree()
      */
      virtual ~stRTree() {
          // Flush header page.
          FlushHeader();
      }

      /**
      * Returns the page manager used by this tree.
      */
      stPageManager * GetPageManager(){
         return myPageManager;
      }//end GetPageManager

      /**
      * This method adds an object to the R tree.
      *
      * @param obj The object to be added.
      */
      virtual bool Add(basicArrayObject * newObj);

      /**
      * Returns the height of the tree.
      */
      virtual u_int32_t GetHeight(){
         return Header->Height;
      }//end GetHeight

      /**
      * Returns the number of objetcs of this tree.
      */
      virtual long GetNumberOfObjects(){
         return Header->ObjectCount;
      }//end GetNumberOfObjects

      /**
      * Returns the number of nodes of this tree.
      */
      virtual long GetNodeCount(){
         return Header->NodeCount;
      }//end GetNodeCount

      /**
      * Sets the tree split method.
      *
      * @param method Method name.
      * @see tSplitMethod
      */
      void SetSplitMethod(enum tSplitMethod method){
         Header->SplitMethod = method;
         HeaderUpdate = true;
      }//end SetSplitMethod

      /**
      * Returns the current split method.
      */
      int GetSplitMethod(){
          return Header->SplitMethod;
      }//end GetSplitMethod

      /**
      * Sets the query metric evaluator.
      *
      * @param method metricevaluator the pointer to the metric evaluator
      */
      void SetQueryMetricEvaluator(rBasicMetricEvaluator *metricevaluator){
         queryMetricEvaluator = metricevaluator;
      }//end SetChooseMethod

      /**
      * Gets the query metric evaluator.
      */
      rBasicMetricEvaluator *GetQueryMetricEvaluator(){
         return queryMetricEvaluator;
      }//end SetChooseMethod

      /**
      * Get root page id.
      */
      u_int32_t GetRoot(){
         return this->Header->Root;
      }//end GetRoot

      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * Notice that for the R-tree construction it is not mandatory
      * to inform a metric evaluator, so the user must inform the
      * tree which metric evaluator will be used in similarity
      * queries. Please read SetQueryMetricEvaluator() method.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResult * RangeQuery(basicArrayObject * sample, double range);

      /**
      * This method will perform a K-Nearest Neighbor query using a global priority
      * queue based on chained list to "enhance" its performance. We believe that the
      * use of a priority queue during the search in the index nodes will force the
      * radius to converge faster than the normal nearest query.
      *
      * Notice that for the R-tree construction it is not mandatory
      * to inform a metric evaluator, so the user must inform the
      * tree which metric evaluator will be used in similarity
      * queries. Please read SetQueryMetricEvaluator() method.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void NearestQuery
      */
      tResult * NearestQuery(basicArrayObject * sample, u_int32_t k, bool tie = false);

      /**
      * This method will return the object in the tree that has the distance 0
      * to the query object. In other words, the query object itself.
      *
      * <P>If there are more elements that has distance 0 to the query
      * object, this method will return the first found.
      *
      * <P>The object pointed by <b>sample</b> will not be destroyed by this
      * method.
      *
      * @param sample The sample object.
      * @return The result or NULL if this method is not implemented.
      * @warning This method return only one object that has distance 0 to the
      * query object.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * PointQuery(basicArrayObject * sample) {
          tResult * result = new tResult();
          return result;
      }

      /**
      * Computes the area of a mbr
      * As in method "area" from Constants.java (Papadias R-tree)
      */
      double GetMbrArea(basicArrayObject *mbr) {
          double mbrsize = 1.0;
          u_int32_t dim = mbr->GetSize() / 2;
          for (u_int32_t i = 0; i < dim; i++) {
              mbrsize *= mbr->Get(dim + i) - mbr->Get(i);
          }
          return mbrsize;
      }

      /**
      * Computes the MINDIST between 1 pt and 1 MBR (see [Rous95])
      * the MINDIST ensures that the nearest neighbor from this pt to a
      * rect in this MBR is at at least this distance
      */
      double MinDistance(basicArrayObject *obj, basicArrayObject *mbr) {
          if (this->GetQueryMetricEvaluator() != NULL) {
              this->GetQueryMetricEvaluator()->UpdateDistanceCount();
          }
          double r, sum = 0.0;
          int dim = obj->GetSize();
          for(int i = 0; i < dim; i++) {
              if (obj->Get(i) < mbr->Get(i)) {
                  r = mbr->Get(i);
              }
              else {
                  if (obj->Get(i) > mbr->Get(dim+i))
                      r = mbr->Get(dim+i);
                  else
                      r = obj->Get(i);
              }
              sum += ((obj->Get(i) - r) * (obj->Get(i) - r));
          }
          return sum;
      }

      /**
      * computes the MINMAXDIST between 1 pt and 1 MBR (see [Rous95])
      * the MINMAXDIST ensures that there is at least 1 object in the MBR
      * that is at most MINMAXDIST far away of the point
      */
      double MinMaxDistance(basicArrayObject *obj, basicArrayObject *mbr) {
          double minimum = MAXDOUBLE;
          double S = 0.0;
          double sum = 0.0;
          int dim = obj->GetSize();
          double rmk, rMi;
          for(int i = 0; i < dim; i++) {
              rMi = (obj->Get(i) >= (mbr->Get(i) + mbr->Get(i+dim))/2) ? mbr->Get(i) : mbr->Get(i+dim);
              S += pow( obj->Get(i) - rMi, 2 );
          }
          for(int k = 0; k < dim; k++) {
              rmk = ( obj->Get(k) <=  (mbr->Get(k) + mbr->Get(k+dim)) / 2 ) ? mbr->Get(k) : mbr->Get(k+dim);
              sum = pow(obj->Get(k) - rmk, 2 );
              rMi = (obj->Get(k) >= (mbr->Get(k) + mbr->Get(k+dim)) / 2 ) ? mbr->Get(k) : mbr->Get(k+dim);
              sum += S - pow(obj->Get(k) - rMi, 2);
              if (sum < minimum)
                  minimum = sum;
          }
          return minimum;
      }

      /**
      * computes the MAXDIST between 1 pt and 1 MBR. It is defined as the
      * maximum distance of a MBR vertex against the specified point
      * Used as an upper bound of the furthest rectangle inside an MBR from a specific
      * point
      */
      double MaxDistance(basicArrayObject *obj, basicArrayObject *mbr) {
          double sum = 0.0;
          double maxdiff;
          int dim = obj->GetSize();
          for(int i = 0; i < dim; i++) {
              maxdiff = fabs(obj->Get(i) - mbr->Get(i));
              if (maxdiff < fabs(obj->Get(i) - mbr->Get(i+dim)))
                  maxdiff = fabs(obj->Get(i) - mbr->Get(i+dim));
              sum += maxdiff * maxdiff;
          }
          return sum;
      }

   private:

      /**
      * The Rtree header. This variable points to data in the HeaderPage.
      */
      stRHeader * Header;

      /**
      * Pointer to the header page.
      * The R tree keeps this page in memory for faster access.
      */
      stPage * HeaderPage;

      /**
      * The page manager used by this tree.
      */
      stPageManager * myPageManager;

      /**
      * If true, the header must be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * Pointer to the query metric evaluator
      */
      rBasicMetricEvaluator *queryMetricEvaluator;

      /**
      * Pointer to the euclidean metric evaluator used in the tree construction
      */
      rEuclideanBasicMetricEvaluator *me;

      /**
      * This enumeration defines the actions to be taken after a call of InsertRecursive.
      */
      enum stInsertAction{
         /**
         * No update needed.
         */
         NO_ACT,

         /**
         * Update the mbr in the previous tree level
         */
         UPDATE_MBR,

         /**
         * Split occured. Update upper levels
         */
         PROMOTION

      };//end stInsertAction

      /**
      * This type defines the logic node for this class.
      */
      //typedef stRTreeLogicNode < DataType,OIDType > tRTreeLogicNode;

      /**
      * This structure holds a promotion data. It contains the mbr, the ID of the root and the number of objects of the subtree.
      */
      struct stSubtreeInfo{
         /**
         * The mbr.
         */
         basicArrayObject * Mbr;

         /**
         * The ID of the subtree root.
         */
         u_int32_t RootID;

         /**
         * Number of objects in the subtree.
         */
         u_int32_t NObjects;

         /**
         * True if index entry points to a leaf node or false if not
         */
         bool sonIsLeaf;
      };

      /**
      * This method inserts an object in to the tree recursively.
      * This method is the core of the insertion method. It will manage
      * promotions (splits) and representative changes.
      *
      * <P>For each action, the returning values may assume the following
      * configurations:
      *     - NO_ACT:
      *           - promo1.Radius will have the new subtree radius.
      *           - Other parameters will not be used.
      *     - CHANGE_REP:
      *           - promo1 will contain the information about the changes in the subtree.
      *           - Other parameters will not be used.
      *     - PROMOTION:
      *           - promo1 will contain the information about the choosen subtree.
      *                 - If promo1.Rep is NULL, the representative of the
      *                   subtree will not change.
      *           - promo2 will contain the information about the promoted subtree.
      *
      * @param currNodeID Current node ID.
      * @param newObj The new object to be inserted. This instance will never be destroyed.
      * @param repMBR The MBR object for this node. This instance will never be destroyed.
      * @param promo1 Information about the choosen subtree (returning value).
      * @param promo2 Infromation about the promoted subtree (returning value).
      * @return The action to be taken after the returning. See enum stInsertAction for more details.
      */
      int InsertRecursive(u_int32_t currNodeID, basicArrayObject * newObj, basicArrayObject * mbrObj, stSubtreeInfo & promo1, stSubtreeInfo & promo2);

      /**
      * This method splits a leaf node in 2. This will get 2 nodes and will
      * redistribute the object set between them.
      *
      * <P>The split method will be defined by the current tree configuration.
      *
      * @param oldNode The node to be splited.
      * @param newNode The new node.
      * @param newObj The new object to be added. This instance will be consumed by this method.
      * @param promo1 The promoted subtree. If its mbr are NULL, there is no need to update the mbr
      * @param promo2 The promoted subtree. If its mbr are NULL, there is no need to update the mbr
      */
      void SplitLeaf(stRLeafNode * oldNode, stRLeafNode * newNode, basicArrayObject * newObj, stSubtreeInfo & promo1, stSubtreeInfo & promo2);

      /**
      * This method splits an index node in 2.
      */
      void SplitIndex(stRIndexNode * oldNode, stRIndexNode * newNode, basicArrayObject * newMbr, u_int32_t newNodeID, u_int32_t newNEntries, bool sonIsLeaf);

      /**
      * This method computes an index of an entry where the insertion process
      * of record obj should continue.
      *
      * This procedures was adapted from RTDirNode.java choose_subtree method available
      * in www.rtreeportal.org, from R-star tree source code from Dimitris Papadias
      *
      * ChooseSubTree chooses the best subtree under this node to insert a new mbr
      * There are three cases:
      * Case 1: the new mbr is contained (inside) in only one directory entry mbr.
      * In this case follow this subtree.
      * Case 2: the new mbr is contained (inside) in more than one directory entry mbr.
      * In this case follow the entry whose mbr has the minimum area
      * Case 3: the new mbr is not contained (inside) in any directory entry mbr
      * In this case the criteria are the following:
      * - If the son nodes are data nodes consider as criterion first the minimum overlap
      *   increase if we follow one node with its neighbors, then the minimum area enlargement
      *   and finally the minimum area
      * - In the son nodes are dir nodes consider as criterion first the minimum area enlargement
      *   and finally the minimum area
      * After we choose the subtree, we enlarge the directory entry (if has to be enlarged)
      * and return its index
      *
      * @param rIndexNode the indexNode to be analyzed
      * @param obj The object that will be inserted.
      * @return the minIndex the index of the choose of the subTree
      */
      int ChooseSubTree(stPage * currPage, basicArrayObject * obj);

      /**
      * Sets all header's fields to default values.
      *
      * @warning This method will destroy the tree.
      */
      void DefaultHeader(){
         // Clear header page.
         HeaderPage->Clear();

         // Default values
         Header->Magic[0] = 'R';
         Header->Magic[1] = 'S';
         Header->Magic[2] = 'T';
         Header->Magic[3] = 'R';
         Header->SplitMethod = smLINEAR;
         Header->Root = 0;
         Header->Height = 0;
         Header->ObjectCount = 0;
         Header->NodeCount = 0;

         // Notify modifications
         HeaderUpdate = true;
      }
      /**
      * Loads the header page and set the Header pointer. The previous header
      * page, if exists, will be lost.
      *
      * @exception std::logic_error If the page size is too small.
      */
      void LoadHeader(){
         if (HeaderPage != NULL){
            this->myPageManager->ReleasePage(HeaderPage);
         }//end if

         // Load and set the header.
         HeaderPage = this->myPageManager->GetHeaderPage();
         if (HeaderPage->GetPageSize() < sizeof(stRHeader)){
            #ifdef __stDEBUG__
               cout << "The page size is too small. Increase it!\n";
            #endif //__stDEBUG__
            throw std::logic_error("The page size is too small.");
         }//end if

         Header = (stRHeader *) HeaderPage->GetData();
         HeaderUpdate = false;
      }

      /**
      * Updates the header in the file if required.
      */
      void WriteHeader(){
         if (HeaderUpdate){
            this->myPageManager->WriteHeaderPage(HeaderPage);
            HeaderUpdate = false;
         }//end if
      }//end WriteHeader

      /**
      * Disposes the header page if it exists. It also updates its contents
      * before destroy it.
      *
      * <P>This method is called by the destructor.
      */
      void FlushHeader() {
         if (HeaderPage != NULL){
            if (Header != NULL){
               WriteHeader();
            }//end if
            this->myPageManager->ReleasePage(HeaderPage);
         }//end if
      }

      /**
      * Creates a new empty page and updates the node counter.
      */
      stPage * NewPage(){
         Header->NodeCount++;
         return this->myPageManager->GetNewPage();
      }//end NewPage

      /**
      * Disposes a given page and updates the page counter.
      */
      void DisposePage(stPage * page){
         Header->NodeCount--;
         this->myPageManager->DisposePage(page);
      }//end DisposePage

      /**
      * Checks to see it a given node ID is the current root.
      *
      * @return True it the given nodeID is the root or false otherwise.
      */
      bool IsRoot(u_int32_t nodeID){
         return nodeID == this->Header->Root;
      }//end IsRoot

      /**
      * Sets a new root.
      */
      void SetRoot(u_int32_t root){
         Header->Root = root;
         HeaderUpdate = true;
      }//end SetRoot

      /**
      * Updates the object counter.
      */
      void UpdateObjectCounter(int inc){
         Header->ObjectCount += inc;
         HeaderUpdate = true;
      }//end UpdateObjectCounter

      /**
      * Checks if point obj is inside the entry's MBR
      */
      bool IsInside(basicArrayObject *obj, basicArrayObject *mbr) {
          int i;
          int dim = obj->GetSize();
          for (i = 0; i < dim; i++) {
              if ((obj->Get(i) > mbr->Get(dim+i)) || // upper limit
                  (obj->Get(i) < mbr->Get(i))) // lower limit
              return false;
          }
          return true;
      }

      /**
      * These constants are used to define the section method.
      */
      enum tSection{
         scINSIDE, scOVERLAP, scNONE
      };//end tSection

      /**
      * Tests if mbrA is inside or overlaps mbrB
      * @returns scINSIDE if inside, scOVERLAP if overlap, scNONE if none
      */
      int SectionMbrMbr(basicArrayObject *mbrA, basicArrayObject *mbrB) {
          bool inside;
          bool overlap;
          int i;
          overlap = true;
          inside = true;
          int dim = mbrA->GetSize()/2;
          for (i = 0; i < dim; i++) {
              if ((mbrA->Get(dim+i) > mbrB->Get(dim+i)) || (mbrA->Get(i) < mbrB->Get(i)))
                  overlap = false;
              if ((mbrA->Get(dim+i) < mbrB->Get(dim+i)) || (mbrA->Get(i) > mbrB->Get(i)))
                  inside = false;
          }
          if (inside)
              return scINSIDE;
          else if (overlap)
              return scOVERLAP;
          else
              return scNONE;
      }

      /**
      * Tests if mbrA is inside or overlaps mbrB
      * @returns scINSIDE if inside, scOVERLAP if overlap, scNONE if none
      */
      int SectionMbrObj(basicArrayObject *mbr, basicArrayObject *obj) {
          bool inside;
          bool overlap;
          int i;
          overlap = true;
          inside = true;
          int dim = mbr->GetSize()/2;
          for (i = 0; i < dim; i++) {
              if ((mbr->Get(dim+i) > obj->Get(i)) || (mbr->Get(i) < obj->Get(i)))
                  overlap = false;
              if ((mbr->Get(dim+i) < obj->Get(i)) || (mbr->Get(i) > obj->Get(i)))
                  inside = false;
          }
          if (inside)
              return scINSIDE;
          else if (overlap)
              return scOVERLAP;
          else
              return scNONE;
      }

      /**
      * Creates and updates the new root of the Rtree.
      *
      * @param mbrA1 entry A mbr 1
      * @param mbrA2 entry A mbr 2
      * @param nodeID1 ID of the root page of the sub-tree 1.
      * @param nEntries1 Number of entries in the sub-tree 1.
      * @param mbrB1 entry A mbr 1
      * @param mbrB2 entry A mbr 2
      * @param nodeID2 ID of the root page of the sub-tree 2.
      * @param nEntries2 Number of entries in the sub-tree 2.
      */
      void AddNewRoot(basicArrayObject *mbrA, u_int32_t nodeID1, u_int32_t nEntries1, bool sonIsLeaf1,
                      basicArrayObject *mbrB, u_int32_t nodeID2, u_int32_t nEntries2, bool sonIsLeaf2);

      /**
      * Computes the mbr for the mbrs in an index page.
      */
      basicArrayObject * GetIndexMbr(stRIndexNode *node) {
          basicArrayObject tmp;
          tmp.Unserialize(node->GetObject(0), node->GetObjectSize(0));
          u_int32_t dimensionality = tmp.GetSize() / 2; // tmp is already a mbr
          basicArrayObject *mbr = new basicArrayObject(dimensionality * 2);
          double d, mind, maxd;
          for (u_int32_t j = 0; j < dimensionality; j++) {
              mind = MAXDOUBLE;
              maxd = -MAXDOUBLE;
              for (u_int32_t i = 0; i < node->GetNumberOfEntries(); i++) {
                  tmp.Unserialize(node->GetObject(i), node->GetObjectSize(i));
                  if (tmp.Get(j) < mind)
                      mind = tmp.Get(j);
                  if (tmp.Get(j) > maxd)
                      maxd = tmp.Get(j);

                  if (tmp.Get(dimensionality+j) < mind)
                      mind = tmp.Get(dimensionality+j);
                  if (tmp.Get(dimensionality+j) > maxd)
                      maxd = tmp.Get(dimensionality+j);
              }
              mbr->Set(j,mind);
              mbr->Set(dimensionality + j,maxd);
          }
          return mbr;
      }

      /**
      * Computes the mbr for the objects in a leaf page.
      */
      basicArrayObject * GetLeafMbr(stRLeafNode *node) {
          basicArrayObject tmp;
          tmp.Unserialize(node->GetObject(0), node->GetObjectSize(0));
          u_int32_t dimensionality = tmp.GetSize();
          basicArrayObject *mbr = new basicArrayObject(dimensionality * 2);
          double d, mind, maxd;
          for (u_int32_t j = 0; j < dimensionality; j++) {
              mind = MAXDOUBLE;
              maxd = -MAXDOUBLE;
              for (u_int32_t i = 0; i < node->GetNumberOfEntries(); i++) {
                  tmp.Unserialize(node->GetObject(i), node->GetObjectSize(i));
                  if (tmp.Get(j) < mind)
                      mind = tmp.Get(j);
                  if (tmp.Get(j) > maxd)
                      maxd = tmp.Get(j);
              }
              mbr->Set(j,mind);
              mbr->Set(dimensionality + j,maxd);
          }
          return mbr;
      }

      /**
      * Enlarge parameter mbr to include parameter obj
      */
      void EnlargeMbr(basicArrayObject *mbr, basicArrayObject *obj) {
          u_int32_t dimensionality = obj->GetSize();
          for (u_int32_t i = 0; i < dimensionality; i++) {
              if (obj->Get(i) < mbr->Get(i))
                  mbr->Set(i, obj->Get(i));
              if (obj->Get(i) > mbr->Get(dimensionality+i))
                  mbr->Set(dimensionality+i, obj->Get(i));
          }
      }

      /**
      * Gets the upper bound object of the mbr
      */
      basicArrayObject *GetUpperMbrObject(basicArrayObject *mbr) {
          int dim = mbr->GetSize() / 2;
          basicArrayObject *obj = new basicArrayObject(dim);
          for (int i = 0; i < dim; i++) {
              obj->Set(i,mbr->Get(dim+i));
          }
          return obj;
      }

      /**
      * Gets the lower bound object of the mbr
      */
      basicArrayObject *GetLowerMbrObject(basicArrayObject *mbr) {
          int dim = mbr->GetSize() / 2;
          basicArrayObject *obj = new basicArrayObject(dim);
          for (int i = 0; i < dim; i++) {
              obj->Set(i,mbr->Get(i));
          }
          return obj;
      }

      /**
      * Calcutales the overlapping area of mbr1 and mbr2
      * Calculate overlap in every dimension and multiplicate the values
      * As in method "overlap" from Constants.java (Papadias R-tree)
      */
      double Overlap(basicArrayObject *mbr1, basicArrayObject *mbr2) {
          double sum = 1.0;
          int dimension = mbr1->GetSize() / 2;
          int r1pos = 0, r2pos = 0, r1last = dimension;
          double r1_lb, r1_ub, r2_lb, r2_ub;

          basicArrayObject *UpperMbr1 = GetUpperMbrObject(mbr1);
          basicArrayObject *LowerMbr1 = GetLowerMbrObject(mbr1);
          basicArrayObject *UpperMbr2 = GetUpperMbrObject(mbr2);
          basicArrayObject *LowerMbr2 = GetLowerMbrObject(mbr2);

          while (r1pos < r1last) {
              r1_lb = LowerMbr1->Get(r1pos);
              r1_ub = UpperMbr1->Get(r1pos);
              r2_lb = LowerMbr2->Get(r2pos);
              r2_ub = UpperMbr2->Get(r2pos);
              r1pos++;
              r2pos++;

              // calculate overlap in this dimension
              if (IsInside(UpperMbr1, mbr2)) { // upper bound of r1 is inside r2
                  if (IsInside(LowerMbr1, mbr2)) { // and lower bound of r1 is inside
                      sum *= (r1_ub - r1_lb);
                  }
                  else {
                      sum *= (r1_ub - r2_lb);
                  }
              }
              else {
                  if (IsInside(LowerMbr1, mbr2)) { // and lower bound of r1 is inside
                      sum *= (r2_ub - r1_lb);
                  }
                  else {
                      if ((IsInside(LowerMbr2, mbr1)) && (IsInside(UpperMbr2, mbr1))) {
                          // r1 contains r2
                          sum *= (r2_ub - r2_lb);
                      }
                      else {
                          // r1 and r2 do not overlap
                          sum = 0.0;
                      }
                  }
              }
          }
          delete UpperMbr1;
          delete LowerMbr1;
          delete UpperMbr2;
          delete LowerMbr2;
          return sum;
      }

      /**
      * Computes the distance between two mbrs
      */
      double GetDistanceBetweenMbrs(basicArrayObject *mbr1, basicArrayObject *mbr2) {
          basicArrayObject *mbr1a = GetLowerMbrObject(mbr1);
          basicArrayObject *mbr1b = GetUpperMbrObject(mbr1);
          basicArrayObject *mbr2a = GetLowerMbrObject(mbr2);
          basicArrayObject *mbr2b = GetUpperMbrObject(mbr2);
          double d1, d2, d3, d4, sum = 0.0;
          int i, dim = mbr1a->GetSize();
          // get the sum of the smallest difference for each dimension
          for (i = 0; i < dim; i++) {
               d1 = fabs(mbr1a->Get(i) - mbr2a->Get(i));
               d2 = fabs(mbr1a->Get(i) - mbr2b->Get(i));
               d3 = fabs(mbr1b->Get(i) - mbr2a->Get(i));
               d4 = fabs(mbr1b->Get(i) - mbr2b->Get(i));
               if ( (d1 < d2) && (d1 < d3) && (d1 < d4) )
                   sum += d1;
               else if ( (d2 < d3) && (d2 < d4) )
                   sum += d2;
               else if ( (d3 < d4) )
                   sum += d3;
               else
                   sum += d4;
          }
          delete mbr1a;
          delete mbr1b;
          delete mbr2a;
          delete mbr2b;
          return sum;
      }

      /**
      * computes the MAXDIST between 1 pt and 1 MBR. It is defined as the
      * maximum distance of a MBR vertex against the specified point
      * Used as an upper bound of the furthest rectangle inside an MBR from a specific
      * point
      */
      /*
      public static float MAXDIST(Object pt, float bounces[])
      {
          PPoint point = (PPoint)pt;

          float sum = (float)0.0;
          float maxdiff;
          int i;

          for(i = 0; i < point.dimension; i++)
          {

              maxdiff = max(java.lang.Math.abs(point.data[i] - bounces[2*i]),
                          java.lang.Math.abs(point.data[i] - bounces[2*i+1]));
              sum += java.lang.Math.pow(maxdiff, 2);
          }
          return(sum);
      }
      */

      /**
      * Returns true if there is an intersection between a MBR and a ball and false if not
      * Used in query algorithms
      */
      bool IntersectionBetweenMbrAndBall(basicArrayObject *mbr, basicArrayObject *ballcenter, double radius) {
        // if MBR contains circle center (MINDIST) return true
        if (radius * radius > MinDistance(ballcenter, mbr))
            return true;
        else
            return false;
      }

      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the result.
      * @see tResult * RangeQuery()
      */
      void RangeQuery(u_int32_t pageID, tResult * result, basicArrayObject * sample, double range);

      /**
      * This method will perform a K Nearest Neighbor query using a priority queue.
      *
      * @param result the result set.
      * @param sample The sample object.
      * @param rangeK The range of the results.
      * @param k The number of neighbours.
      * @see tResult * NearestQuery
      */
      void NearestQuery(tResult * result, basicArrayObject * sample, double rangeK, u_int32_t k);

};//end stRTree

// Include implementation
#include "stRTree-inl.h"

#endif //__STRTREE_H
