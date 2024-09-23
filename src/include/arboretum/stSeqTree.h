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
* This file defines the class stSeqTree.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/

#ifndef __STSEQTREE_H
#define __STSEQTREE_H

#include <arboretum/stUtil.h>

#include <arboretum/stMetricTree.h>
#include <arboretum/stSeqNode.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stGenericPriorityQueue.h>

// this is used to set the initial size of the dynamic queue
#define STARTVALUEQUEUE 200
// this is used to set the increment size of the dynamic queue
#define INCREMENTVALUEQUEUE 200

#include <string.h>
#include <math.h>
#include <algorithm>

//=============================================================================
// Class template stSeqLogicNode
//-----------------------------------------------------------------------------
/**
* Thic class template represents a SeqTree logic node entry. It is used to
* hold an index node or a leaf node in a memory form which allows better way to
* manipulate entries. It also implements means to distribute its contents
* between 2 nodes.
*
* <P>Each entry of this node can hold all information required by both nodes at
* same time so, the users of this node must control if it holds an index node or
* a leaf node.
*
* @warning This node will take the ownership of all object instances added to this node.
* It means that the object instance will be destroyed by the destructor of this class.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation review.
* @todo Tests!
* @ingroup seq
*/
template <class ObjectType, class EvaluatorType>
class stSeqLogicNode{
   public:
      /**
      * Creates a new instance of this node with no objects.
      *
      * @param maxOccupation The maximum number of entries.
      */
      stSeqLogicNode(u_int32_t maxOccupation);

      /**
      * Disposes this instance and releases all related resources. All instances of
      * object added to this node will also be deleted unless it is not owned by
      * this node (see method BuyObject()).
      */
      ~stSeqLogicNode();

      /**
      * Adds an object to this node. This method will claim the ownership
      * of the object instance.
      *
      * <P>Use SetEntry() to fill the other fields of each entry.
      *
      * @param obj The object to be added.
      * @return The entry id or -1 for error.
      */
      int AddEntry(ObjectType * obj){
         Entries[Count].Object = obj;
         Entries[Count].Mine = true;
         Count++;
         return Count - 1;
      }//end AddEntry

      /**
      * Adds an object to this node.
      *
      * <P>Use SetEntry() to fill the other fields of each entry.
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @return The entry id or -1 for error.
      */
      int AddEntry(u_int32_t size, const unsigned char * object);

      /**
      * Returns the number of entries in this node.
      */
      u_int32_t GetNumberOfEntries(){
         return Count;
      }//end GetNumberOfEntries

      /**
      * Returns the object of a given entry.
      *
      * @param idx The object index.
      */
      ObjectType * GetObject(int idx){
         return Entries[idx].Object;
      }//end GetObject

      /**
      * Returns the object of a given entry.
      *
      * @param idx The object index.
      */
      ObjectType * operator [](int idx){
         return Entries[idx].Object;
      }//end operator []

      /**
      * Returns the PageID of a given entry. Only index nodes will use this field.
      *
      * @param idx The object index.
      */
      u_int32_t GetPageID(int idx){
         return Entries[idx].PageID;
      }//end GetPageID

      /**
      * Returns the radius of the sub-tree of a given entry. Only index nodes will use this field.
      *
      * @param idx The object index.
      */
      double GetRadius(int idx){
         return Entries[idx].Radius;
      }//end GetRadius

      /**
      * Sets the data associated with a given entry.
      *
      * @param idx The object index.
      * @param pageID The pageID.
      * @param radius The radius of the sub-tree.
      */
      void SetEntry(int idx, u_int32_t pageID, double radius){
         Entries[idx].PageID = pageID;
         Entries[idx].Radius = radius;
      }//end GetPageID

      /**
      * Adds all objects of an index node. It will also set the node type to
      * stSeqNode::INDEX.
      *
      * @param node The node.
      */
      void AddIndexNode(stSeqIndexNode * node);

      /**
      * Adds all objects of a leaf node. It will also set the node type to
      * stSeqNode::LEAF.
      *
      * @param node The node.
      */
      void AddLeafNode(stSeqLeafNode * node);

      /**
      * Returns the idx of the representative object.
      *
      * @param idx The representative ID. It may be 0 or 1.
      */
      u_int32_t GetRepresentativeIndex(u_int32_t idx){
         return RepIndex[idx];
      }//end GetRepresentativeIndex

      /**
      * Returns the representative object.
      *
      * @param idx The representative ID. It may be 0 or 1.
      */
      ObjectType * GetRepresentative(u_int32_t idx){
        return Entries[RepIndex[idx]].Object;
      }//end GetRepresentative

      /**
      * Sets both representatives ids. Use UpdateDistances() to update all
      * distances between objects and.
      *
      * @param rep0 Index of representative 0.
      * @param rep1 Index of representative 1.
      */
      void SetRepresentative(u_int32_t rep0, u_int32_t rep1){
         RepIndex[0] = rep0;
         RepIndex[1] = rep1;
      }//end SetRepresentative

      /**
      * Distribute objects between 2 index nodes using the current
      * representatives (see SetRepresentative()). Both nodes must be empty.
      *
      * @param node0 The first node.
      * @param rep0 Representative of node 0.
      * @param node1 The second node.
      * @param rep0 Representative of node 1.
      * @param metricEvaluator The metric evaluator to be used to compute
      * distances.
      * @return The number of computed distances.
      */
      u_int32_t Distribute(stSeqIndexNode * node0, ObjectType * & rep0,
                         stSeqIndexNode * node1, ObjectType * & rep1,
                         EvaluatorType * metricEvaluator){
         u_int32_t result;

         // Distribute...
         result = TestDistribution(node0, node1, metricEvaluator);

         // Representatives
         rep0 = BuyObject(GetRepresentativeIndex(0));
         rep1 = BuyObject(GetRepresentativeIndex(1));

         return result;
      }//end Distribute

      /**
      * Distribute objects between 2 leaf nodes using the current
      * representatives (see SetRepresentative()). Both nodes must be empty.
      *
      * @param node0 The first node.
      * @param rep0 Representative of node 0.
      * @param node1 The second node.
      * @param rep1 Representative of node 1.
      * @param metricEvaluator The metric evaluator to be used to compute
      * distances.
      * @return The number of computed distances.
      */
      u_int32_t Distribute(stSeqLeafNode * node0, ObjectType * & rep0,
                         stSeqLeafNode * node1, ObjectType * & rep1,
                         EvaluatorType * metricEvaluator){
         u_int32_t result;

         // Distribute...
         result = TestDistribution(node0, node1, metricEvaluator);

         // Representatives
         rep0 = BuyObject(GetRepresentativeIndex(0));
         rep1 = BuyObject(GetRepresentativeIndex(1));

         return result;
      }//end Distribute

      /**
      * Tests the distribution of objects between 2 index nodes using the current
      * representatives (see SetRepresentative()). Both nodes must be empty.
      *
      * @param node0 The first node.
      * @param node1 The second node.
      * @param metricEvaluator The metric evaluator to be used to compute
      * distances.
      * @return The number of computed distances.
      * @warning Since it is just a test fnction, it will not inialize the
      * fields of the entries.
      */
      u_int32_t TestDistribution(stSeqIndexNode * node0, stSeqIndexNode * node1,
                               EvaluatorType * metricEvaluator);

      /**
      * Tests the distribution of objects between 2 leaf nodes using the current
      * representatives (see SetRepresentative()).  Both nodes must be empty.
      *
      * @param node0 The first node.
      * @param node1 The second node.
      * @param metricEvaluator The metric evaluator to be used to compute
      * distances.
      * @return The number of computed distances.
      */
      u_int32_t TestDistribution(stSeqLeafNode * node0, stSeqLeafNode * node1,
                               EvaluatorType * metricEvaluator);

      /**
      * Set minimum occupation. This must be the minimum number of objects
      * in a page. This value must be at least 1.
      */
      void SetMinOccupation(u_int32_t min){
         MinOccupation = min;
         // At least the nodes must store 2 objects.
         if ((MinOccupation > (MaxEntries/2)) || (MinOccupation == 0)){
            MinOccupation = 2;
         }//end if
      }//end SetMinOccupation

      /**
      * Returns the node type. It may assume the values stSeqNode::INDEX or
      * stSeqNode::LEAF.
      */
      u_int16_t GetNodeType(){
         return NodeType;
      }//end GetNodeType

      /**
      * Sets the node type.
      */
      void SetNodeType(u_int16_t type){
         NodeType = type;
      }//end SetNodeType

      /**
      * Verifies if a given object is a representative.
      *
      * @param idx Object index.
      * @return True if the object is a representative or false otherwise.
      */
      bool IsRepresentative(u_int32_t idx){
         return ((idx == RepIndex[0]) || (idx == RepIndex[1]));
      }//end IsRepresentative

      /**
      * Gets the ownership of an object associated with a given entry.
      * This method will avoid the automatic destruction of the object
      * instance by the destructor of this node.
      *
      * <P>This method can be used to buy the representative objects to avoid
      * unnecessary replications.
      *
      * @param idx The object index.
      * @return The pointer to the object.
      */
      ObjectType * BuyObject(u_int32_t idx){
         Entries[idx].Mine = false;
         return Entries[idx].Object;
      }//end BuyObject

   private:
      /**
      * This type represents a seq tree logic node entry.
      */
      struct stSeqLogicEntry{
         /**
         * Object.
         */
         ObjectType * Object;

         /**
         * ID of the page.
         */
         u_int32_t PageID;

         /**
         * Radius of the sub-tree.
         */
         double Radius;

         /**
         * Owner flag.
         */
         bool Mine;

         /**
         * Distances between representatives.
         */
         double Distance[2];

         /**
         * Node Map.
         */
         bool Mapped;
      };

      /**
      * Minimum occupation.
      */
      u_int32_t MinOccupation;

      /**
      * Entries.
      */
      struct stSeqLogicEntry * Entries;

      /**
      * Maximum number of entries.
      */
      u_int32_t MaxEntries;

      /**
      * Current number of entries.
      */
      u_int32_t Count;

      /**
      * This vector holds the id of the representative objects.
      */
      u_int32_t RepIndex[2];

      /**
      * Type of this node.
      */
      u_int16_t NodeType;

      /**
      * Updates all distances between representatives and all objects in this
      * node. It returns the number of distances calculated.
      *
      * @param metricEvaluator The metric evaluator to be used.
      */
      u_int32_t UpdateDistances(EvaluatorType * metricEvaluator);
      
};//end stSeqLogicNode

//=============================================================================
// Class template stSeqMSTSpliter
//-----------------------------------------------------------------------------
/**
* This class template implements the SeqTree MST split algorithm.
*
* @version 1.0
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo Documentation review.
* @todo Tests!
* @ingroup seq
*/
template <class ObjectType, class EvaluatorType>
class stSeqMSTSplitter{
   public:
      /**
      * This type defines the logic node for this class.
      */
      typedef stSeqLogicNode < ObjectType, EvaluatorType > tLogicNode;

      /**
      * Builds a new instance of this class. It will claim the ownership of the
      * logic node provided as input.
      */
      stSeqMSTSplitter(tLogicNode * node);

      /**
      * Disposes all associated resources.
      */
      ~stSeqMSTSplitter();

      /**
      * Provides access to the logic node.
      */
      const tLogicNode * GetLogicNode(){
         return Node;
      }//end GetLogicNode

      /**
      * Distributes objects between 2 index nodes.
      *
      */
      int Distribute(stSeqIndexNode * node0, ObjectType * & rep0,
                     stSeqIndexNode * node1, ObjectType * & rep1,
                     EvaluatorType * metricEvaluator);

      /**
      * Distributes objects between 2 leaf nodes.
      */
      int Distribute(stSeqLeafNode * node0, ObjectType * & rep0,
                     stSeqLeafNode * node1, ObjectType * & rep1,
                     EvaluatorType * metricEvaluator);
                     
   protected:
      /**
      * Distance matrix type.
      */
      typedef stGenericMatrix <double> tDistanceMatrix;

      /**
      * Cluster states.
      */
      enum tClusterState{
         ALIVE,
         DEAD,
         DEATH_SENTENCE
      };//end tClusterState

      /**
      * Cluster information type.
      */
      struct tCluster{
         /**
         * If this cluster exists.
         */
         enum tClusterState State;

         /**
         * Number of objects in this cluster.
         */
         int Size;

         /**
         * Minimum distance to the nearest cluster.
         */
         double MinDist;

         /**
         * The object of this cluster that defines the minimum distance.
         */
         int Src;

         /**
         * The object of the nearest cluster that defines the minimum distance.
         */
         int Dst;
      };

      /**
      * The logic node to be used as source.
      */
      tLogicNode * Node;

      /**
      * The distance matrix.
      */
      tDistanceMatrix DMat;

      /**
      * All clusters.
      */
      struct tCluster * Cluster;

      /**
      * The names of the cluster of each object
      */
      int * ObjectCluster;

      /**
      * Total number of objects.
      */
      int N;

      /**
      * Name of the cluster 0.
      */
      int Cluster0;

      /**
      * Name of the cluster 1.
      */
      int Cluster1;

      /**
      * Returns the center of the object in the cluster clus.
      * The.
      *
      * @param clus Cluster id.
      */
      int FindCenter(int clus);

      /**
      * Builds the distance matrix using the given metric evaluator.
      *
      * @param metricEvaluator The metric evaluator.
      * @return The number of computed distances.
      */
      int BuildDistanceMatrix(EvaluatorType * metricEvaluator);

      /**
      * Performs the MST algorithm. This method will split the objects in 2
      * clusters. The result of the processing will be found at the array
      * Cluster.
      *
      * @warning DMat must be initialized.
      */
      void PerformMST();

      /**
      * Joins 2 clusters. This method will insert custer2 into cluster1.
      *
      * <P>The state of cluster2 will change to DEATH_SENTENCE.
      *
      * @param cluster1 Cluster 1.
      * @param cluster2 Cluster 2.
      */
      void JoinClusters(int cluster1, int cluster2);

};//end stSeqMSTSplitter

//=============================================================================
// Class template stSeqTree
//-----------------------------------------------------------------------------
/**
* This class defines all behavior of the SeqTree.
* Probably most of the atributes will be stored in the header page of the used
* stPageManager (stDiskPageManager or stMemoryPageManager).
*
* <P> First implementation is based on original code by Agma Traina, Bernard Seeger
* and Caetano Traina
* <P> Main modifications from original code are intent to turn it an object oriented
* compliant code.
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @todo More documentation.
* @version 1.0
* @ingroup seq
*/
template <class ObjectType, class EvaluatorType>
class stSeqTree: public stMetricTree <ObjectType, EvaluatorType> {

   public:

      /**
      * This structure defines the SeqTree header structure. This type was left
      * public to allow the creation of debug tools.
      */
      typedef struct tSeqHeader{
         /**
         * Magic number. This is a short string that must contains the magic
         * string "SEQ-". It may be used to validate the file (this feature
         * is not implemented yet).
         */
         char Magic[4];
      
         /**
         * Split method.
         */
         int SplitMethod;
      
         /**
         * Choose method
         */
         int ChooseMethod;
      
         /**
         * Correct method.
         */
         int CorrectMethod;
      
         /**
         * The root of the Seq-tree
         */
         u_int32_t Root;

         /**
         * The first leaf node of the Seq-tree
         */
         u_int32_t FirstLeafNode;

         /**
         * The first leaf node of the Seq-tree
         */
         u_int32_t FirstIndexNode;

         /**
         * Minimum percentage of objects in a node.
         */
         double MinOccupation;

         /**
         * Maximum occupation of objects in a node.
         */
         u_int32_t MaxOccupation;

         /**
         * The height of the tree
         */
         int Height;
      
         /**
         * Total number of records
         */
         u_int32_t ObjectCount;
         
         /**
         * Total number of nodes.
         */
         u_int32_t NodeCount;
      } stSeqHeader;   

      /**
      * These constants are used to define the choose sub tree method.
      */
      enum tChooseMethod{
         /**
         * Choose the first of the qualifying nodes.
         */
         cmBIASED,

         /**
         * Choose the node that has the minimum distance from the
         * new object and the representative (center) of the node.
         */
         cmMINDIST,

         /**
         * Unknown.
         * @todo Discover what this method do.
         */
         cmMINGDIST
      };//end tChooseMethod

      /**
      * These constants are used to define the correction method.
      */
      enum tCorrectMethod {
         /**
         * No correction.
         */
         crmOFF,
         /**
         * Use Fat Factor.
         */
         crmFATFACTOR
      };//end tCorrectMethod

      /**
      * These constants are used to define the split method.
      * @todo Update documentation of each constant.
      */
      enum tSplitMethod {

         /**
         * The optimal split method. This algorithm tries all possible
         * distribution configurations and select the optimal distribution as
         * the final result.
         *
         * <p>This is the default method for the M-Tree.
         *
         * @warning This method is very slow.
         */
         smMINMAX,

         /**
         * Split method based on the Minimal Spanning Tree algorithm.
         * This is the Seq-Tree default split method.
         */
         smSPANNINGTREE
      };//end tSplitMethod

      /**
      * This is the class that abstracs an result set for simple queries.
      */
      typedef stResult <ObjectType> tResult;

      /**
      * This is the class that abstracs an result set for joined queries.
      */
      typedef stJoinedResult <ObjectType> tJoinedResult;

      /**
      * This type is used by the priority key.
      */
      typedef stGenericPriorityQueue < ObjectType > tGenericPriorityQueue;

      /**
      * This type is used by the priority key.
      */
      typedef stGenericEntry < ObjectType > tGenericEntry;

      /**
      * This type is used by the priority key.
      */
      typedef stGenericPriorityHeap < ObjectType > tPGenericHeap;

      /**
      * Memory leaf node used by Shrink.
      */
      typedef stSeqMemLeafNode < ObjectType > tMemLeafNode;

      //typedef stHistogram < ObjectType, EvaluatorType > tHistogram;

      /**
      * Creates a new metric tree using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this metric tree.
      */
      stSeqTree(stPageManager * pageman);

      /**
      * Creates a new metric tree using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this metric tree.
      * @param metricEval The shared metric evaluator be used by this metric tree.
      * @warning metricEval will not be delete by this class.
      */
      stSeqTree(stPageManager * pageman, EvaluatorType * metricEval);

      /**
      * Dispose all used resources, ie, it is the destructor method.
      *
      * @see stSeqTree()
      */
      virtual ~stSeqTree();

      /**
      * This method adds an object to the metric tree.
      *
      * @param obj The object to be added.
      */
      virtual bool Add(ObjectType * newObj);

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
      * Returns the MaxOccupation of the nodes.
      */
      u_int32_t GetMaxOccupation(){
         return Header->MaxOccupation;
      }//end GetMaxOccupation

      /**
      * Returns the MaxOccupation of the nodes.
      */
      void SetMaxOccupation(u_int32_t newValue){
         if (newValue > Header->MaxOccupation){
            Header->MaxOccupation = newValue;
         }//end if
      }//end SetMaxOccupation

      /**
      * Returns the MinOccupation of the nodes.
      */
      virtual double GetMinOccupation(){
         return Header->MinOccupation;
      }//end GetMinOccupation

      /**
      * Set the MinOccupation of the nodes.
      */
      virtual void SetMinOccupation(double min){
         Header->MinOccupation = min;
      }//end SetMinOccupation

      /**
      * Returns the number of nodes of this tree.
      */
      virtual long GetNodeCount(){
         return Header->NodeCount;
      }//end GetNodeCount

      long GetIndexNodeCount();

      long GetIndexNodeCount(u_int32_t pageID);
      /**
      * Returns the limit distance between 2 objects in the tree. That is
      * \f$ \forall a,b \in D, d(a,b) \le GetDistanceLimit()\f$. In other
      * words, there is no distance greater than this.
      *
      * <P> This value may be the greatest distance between objects but may
      * return greater values due to implementation optmizations.
      */
      double GetDistanceLimit();

      /**
      * Returns the greatest estimated distance between 2 objects in the second
      * level of the tree.
      *
      * @return the greatest estimated distance.
      */
      double GetGreaterEstimatedDistance();

      /**
      * Returns the greatest distance between 2 objects in the leaf
      * level of the tree.
      *
      * @return the greatest distance.
      */
      double GetGreaterDistance();

      // Tree Configuration
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
      * Sets the Choose Method name.
      *
      * @param method Choose method name.
      * @see tChooseMethod
      */
      void SetChooseMethod(enum tChooseMethod method){
         Header->ChooseMethod = method;
         HeaderUpdate = true;
      }//end SetChooseMethod

      /**
      * Returns the Choose Method name.
      */
      int GetChooseMethod(){
         return Header->ChooseMethod;
      }//end GetChooseMethod

      #ifdef __stDEBUG__
         /**
         * Get root page id.
         *
         * @warning This method is public only if __stDEBUG__ is defined at compile
         * time.
         */
         u_int32_t GetRoot(){
            return this->Header->Root;
         }//end GetRoot
      #endif //__stDEBUG__

      /**
      * Gets the maximum user data size available in this tree.
      *
      * @see WriteUserData()
      * @see ReadUserData()
      * @bug This feature is not implemented yet.
      */
      u_int32_t GetUserDataSize(){
         return 0;
      }//end GetUserDataSize

      /**
      * Writes an additional user data.
      *
      * <P>This feature allows users to write additional information in to the
      * free area of the header page. The available space will deppends on the
      * size of the header page (see the page manager documentation for more
      * details).
      *
      * @param userData A pointer to the user data.
      * @param size The size of the user data.
      * @return True for success or false if the user data doesn't fit in this
      * area.
      * @see GetUserDataSize()
      * @see ReadUserData()
      * @bug This feature is not implemented yet.
      */
      bool WriteUserData(const unsigned char * userData, u_int32_t size){
         return false;
      }//end WriteUserData

      /**
      * Reads an additional user data.
      *
      * <P>This feature allows users to write additional information in to the
      * free area of the header page. The available space will deppends on the
      * size of the header page (see the page manager documentation for more
      * details).
      *
      * @param userData A pointer to the user data.
      * @param size The size of the user data.
      * @return True for success or false if the user data doesn't fit in this
      * area.
      * @see GetUserDataSize()
      * @see WriteUserData()
      * @bug This feature is not implemented yet.
      */
      bool ReadUserData(unsigned char * userData, u_int32_t size){
         return false;
      }//end ReadUserData

      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResult * RangeQuery(ObjectType * sample, double range);

      tResult * SeqLeafRangeQuery(ObjectType * sample, double range);

      tResult * SeqIndexRangeQuery(ObjectType * sample, double range);

      /**
      * This method will perform a K-Nearest Neighbor query using a global priority
      * queue based on chained list to "enhance" its performance. We believe that the
      * use of a priority queue during the search in the index nodes will force the
      * radius to converge faster than the normal nearest query.
      *
      * <p>This implementation may replace the original implementation if it proves
      * to be better.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void NearestQuery
      */
      tResult * NearestQuery(ObjectType * sample, u_int32_t k, bool tie = false);

      tResult * SeqLeafNearestQuery(ObjectType * sample, u_int32_t k, bool tie = false);

      tResult * SeqLeafNearestQuery2(ObjectType * sample, u_int32_t k, bool tie = false);

      tResult * SeqIndexNearestQuery(ObjectType * sample, u_int32_t k, bool tie = false);

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
      tResult * PointQuery(ObjectType * sample);

      /**
      * This method will perform a k-nearest neighbor joined query.
      *
      * @param seqTree The other seqTree to be joined.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      */
      tJoinedResult * NearestJoinQuery(stSeqTree * seqTree, u_int32_t k,
                                       bool tie = false);

      /**
      * This method will perform a range joined query.
      *
      * @param seqTree The tree being joined.
      * @param range The range of the results.
      * @param activeBuffer Active buffer of sub nodes in Joined Tree.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tJoinedResult * RangeJoinQuery(stSeqTree * seqTree, double range);
      tJoinedResult * RangeJoinQueryEnzo(stSeqTree * joinedtree, double range);

      /**
      * Calculates the FatFactor of this tree.
      *
      * @warning This method will update the statistics of the tree.
      */
      double GetFatFactor();

      /**
      * @copydoc stMetricTree::GetTreeInfo()
      */
      virtual stTreeInfoResult * GetTreeInfo();

      /**
      * Optimizes the structure of this tree by executing the Shrink
      * algorithm.
      *
      * <p>The Shrink algorithm can only be performed when the tree has at
       * least 3 levels. Fortunately, this
      */
      virtual void Optimize();

      /**
      * This method travels through the tree gathering information about the
      * number of objects in the tree.
      */
      u_int32_t GetRealIndexObjectsCount();
      u_int32_t GetRealLeafObjectsCount();

      /**
      * This method travels through the tree validating the hole tree.
      */
      bool Consistency();

   private:

      /**
      * This type defines the logic node for this class.
      */
      typedef stSeqLogicNode < ObjectType, EvaluatorType > tLogicNode;

      /**
      * This type defines the MST splitter for this class.
      */
      typedef stSeqMSTSplitter < ObjectType, EvaluatorType > tMSTSplitter;

      /**
      * This type is used by the priority key.
      */
      typedef stRPriorityQueue < double, u_int32_t > tPriorityQueue;

      /**
      * This type is used by the priority key in IncrementalNearest.
      */
      typedef stDynamicRPriorityQueue < double, stQueryPriorityQueueValue > tDynamicPriorityQueue;

      typedef stDynamicRReversedPriorityQueue < double, stQueryPriorityQueueValue > tDynamicReversedPriorityQueue;

      /**
      * This enumeration defines the actions to be taken after an call of
      * InsertRecursive.
      */
      enum stInsertAction{
         /**
         * No action required. Just update the radius.
         */
         NO_ACT,

         /**
         * Replace representative.
         */
         CHANGE_REP,

         /**
         * Split occured. Update subtrees.
         */
         PROMOTION
      };//end stInsertAction

      /**
      * This structure holds a promotion data. It contains the representative
      * object, the ID of the root, the Radius and the number of objects of the subtree.
      */
      struct stSubtreeInfo{
         /**
         * The representative object.
         */
         ObjectType * Rep;

         /**
         * The radius of the subtree.
         */
         double Radius;

         /**
         * The ID root of the root of the subtree.
         */
         u_int32_t RootID;
      };

      /**
      * If true, the header mus be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * The SeqTree header. This variable points to data in the HeaderPage.
      */
      stSeqHeader * Header;

      /**
      * Pointer to the header page.
      * The Seq Tree keeps this page in memory for faster access.
      */
      stPage * HeaderPage;

      /**
      * Sets all header's fields to default values.
      *
      * @warning This method will destroy the tree.
      */
      void DefaultHeader();

      /**
      * Loads the header page and set the Header pointer. The previous header
      * page, if exists, will be lost.
      *
      * @exception std::logic_error If the page size is too small.
      */
      void LoadHeader();

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
      void FlushHeader();
      
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
      * This method computes an index of an entry where the insertion process
      * of record obj should continue.
      *
      * @param seqIndexNode the indexNode to be analyzed
      * @param obj The object that will be inserted.
      * @return the minIndex the index of the choose of the subTree
      */
      int ChooseSubTree(stSeqIndexNode * seqIndexNode, ObjectType * obj);

      /**
      * This is just another strategy of promoting entries of a page p.
      * See the original paper.
      *
      * @param node The node.
      */
      void MinMaxPromote(tLogicNode * node);

      /**
      * This method find a new center for the objects in the node P.
      * It works by finding the objects that minimizes the covering circle.
      * Case the node is a leafNode, the new radius is the new radius plus the radius
      * of the subTree that has all the objects in the all subTrees;
      *
      * @param *node node to compute
      * @param *newCenter the index of the new center
      * @param *newRadius the new radius of this node
      * @todo pq nao usa return para voltar o novo centro ou o novo raio em vez de
      * passar por referencia?
      * this method was revised by Marcos
      */
      void ReDistribute(stSeqNode * node, int & newCenter, double & newRadius);

      /**
      * Checks to see it a given node ID is the current root.
      *
      * @return True it the given nodeID is the root or false otherwise.
      */
      bool IsRoot(u_int32_t nodeID){
         return nodeID == this->Header->Root;
      }//end IsRoot

      #ifndef __stDEBUG__
         /**
         * Get root page id.
         */
         u_int32_t GetRoot(){
            return this->Header->Root;
         }//end GetRoot
      #endif // __stDEBUG__

      /**
      * Sets a new root.
      */
      void SetRoot(u_int32_t root){
         Header->Root = root;
         HeaderUpdate = true;
      }//end SetRoot

      /**
      * Sets a new root.
      */
      void SetFirstIndexNode(u_int32_t first){
         Header->FirstIndexNode = first;
         HeaderUpdate = true;
      }//end SetFirstIndexNode

      /**
      * Get the first sequential node.
      */
      u_int32_t GetFirstIndexNode(){
         return Header->FirstIndexNode;
      }//end GetFirstIndexNode

      /**
      * Sets a new root.
      */
      void SetFirstLeafNode(u_int32_t first){
         Header->FirstLeafNode = first;
         HeaderUpdate = true;
      }//end SetFirstLeafNode

      /**
      * Get the first sequential node.
      */
      u_int32_t GetFirstLeafNode(){
         return Header->FirstLeafNode;
      }//end GetFirstLeafNode

      /**
      * Updates the object counter.
      */
      void UpdateObjectCounter(int inc){
         Header->ObjectCount += inc;
         HeaderUpdate = true;
      }//end UpdateObjectCounter

      /**
      * Used by GetGreaterDistance() to get all objects in the leaf node.
      *
      * @param pageID the pageId to be analyze.
      * @param objects the vector to store all objects.
      * @param size the size of the vector objects.
      * @see GetGreaterDistance()
      */
      void GetGreaterDistance(u_int32_t pageID, ObjectType ** objects,
                              u_int32_t & size);

      /**
      * This method travels through the tree gathering information about the
      * number of objects of the subtree.
      *
      * @param pageID Root of this sub-tree.
      */
      u_int32_t GetRealObjectsCount(u_int32_t pageID);

      /**
      * This method travels through the subtree validating the hole tree.
      *
      * @param pageID Root of this sub-tree.
      */
      bool Consistency(u_int32_t pageID, ObjectType * repObj, double & distance);

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
      *           - promo1 will contains the information about the changes
      *             in the subtree.
      *           - Other parameters will not be used.
      *     - PROMOTION:
      *           - promo1 will contain the information about the choosen subtree.
      *                 - If promo1.Rep is NULL, the representative of the
      *                   subtree will not change.
      *           - promo2 will contain the information about the promoted subtree.
      *
      * @param currNodeID Current node ID.
      * @param newObj The new object to be inserted. This instance will never
      * be destroyed.
      * @param repObj The representative object for this node. This instance
      * will never be destroyed.
      * @param promo1 Information about the choosen subtree (returning value).
      * @param promo2 Infromation about the promoted subtree (returning value).
      * @return The action to be taken after the returning. See enum
      * stInsertAction for more details.
      */
      int InsertRecursive(u_int32_t currNodeID, ObjectType * newObj,
                          ObjectType * repObj, stSubtreeInfo & promo1,
                          stSubtreeInfo & promo2);

      /**
      * Creates and updates the new root of the SeqTree.
      *
      * @param obj1 Object 1.
      * @param radius1 Radius of subtree 1.
      * @param nodeID1 ID of the root page of the sub-tree 1.
      * @param obj2 Object 2.
      * @param radius2 Radius of subtree 2.
      * @param nodeID2 ID of the root page of the sub-tree 2.
      */
      void AddNewRoot(ObjectType *obj1, double radius1, u_int32_t nodeID1,
                      ObjectType *obj2, double radius2,
                      u_int32_t nodeID2);

      /**
      * This method splits a leaf node in 2. This will get 2 nodes and will
      * redistribute the object set between these.
      *
      * <P>The split method will be defined by the current tree configuration.
      *
      * @param oldNode The node to be splited.
      * @param newNode The new node.
      * @param newObj The new object to be added. This instance will be consumed
      * by this method.
      * @param prevRep The previous representative.
      * @param promo1 The promoted subtree. If its representative is NULL,
      * the choosen representative is equal to prevRep.
      * @param promo2 The promoted subtree. The representative of this tree will
      * never be the prevRep.
      * @todo redo the FatFactorPromote method.
      */
      void SplitLeaf(stSeqLeafNode * oldNode, stSeqLeafNode * newNode,
                     ObjectType * newObj, ObjectType * prevRep,
                     stSubtreeInfo & promo1, stSubtreeInfo & promo2);

      /**
      * This method splits an index node in 2. This will get 2 nodes and will
      * redistribute the object set between these.
      *
      * <P>This method may takes one or two new objects to be added to the node
      * before the split.
      *
      * <P>The split method will be defined by the current tree configuration.
      *
      * @param oldNode The node to be splited.
      * @param newNode The new node.
      * @param newObj1 The new object 1 to be added. This instance will be consumed
      * by this method.
      * @param newRadius1 The new object 1 radius.
      * @param newNodeID1 The new object 1 node ID.
      * @param newObj2 The new object 2 to be added or NULL.
      * @param newRadius2 The new object 2 radius if newObj2 is not NULL.
      * @param newNodeID2 The new object 2 node ID  if newObj2 is not NULL..
      * @param prevRep The previous representative.
      * @param promo1 The promoted subtree. If its representative is NULL,
      * the choosen representative is equal to prevRep.
      * @param promo2 The promoted subtree. The representative of this tree will
      * never be the prevRep.
      * @todo redo the FatFactorPromote method.
      */
      void SplitIndex(stSeqIndexNode * oldNode, stSeqIndexNode * newNode,
                      ObjectType * newObj1, double newRadius1,
                      u_int32_t newNodeID1,
                      ObjectType * newObj2, double newRadius2,
                      u_int32_t newNodeID2, 
                      ObjectType * prevRep,
                      stSubtreeInfo & promo1, stSubtreeInfo & promo2);

      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the result.
      * @param distanceRepres The distance of the representative.
      * @see tResult * RangeQuery()
      */
      void RangeQuery(u_int32_t pageID, tResult * result,
                      ObjectType * sample, double range,
                      double distanceRepres);

      /**
      * This method will perform a K Nearest Neighbor query using a priority
      * queue.
      *
      * @param result the result set.
      * @param sample The sample object.
      * @param rangeK The range of the results.
      * @param k The number of neighbours.
      * @see tResult * NearestQuery
      */
      void NearestQuery(tResult * result, ObjectType * sample,
                        double rangeK, u_int32_t k);

      void SeqLeafNearestQuery2(tResult * result, ObjectType * sample, u_int32_t k,
                                double rangeK, u_int32_t prevPageID,
                                u_int32_t nextPageID);

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
      * @see tResult * PointQuery
      */
      void PointQuery(tResult * result, ObjectType * sample);

      /**
      * This method will perform a range join query.
      *
      * @param currNodeIndex The node in index tree to be analyzed.
      * @param objIndex The object representative in index tree.
      * @param radiusObjIndex The object representative in index tree.
      * @param heightIndex Actual height of indexed tree.
      * @param currNodeJoin The node in join tree to be analyzed.
      * @param objJoin The object representative in join tree.
      * @param radiusObjJoin The radius of object representative in join tree.
      * @param heightJoin Actual height of joined tree.
      * @param PageManagerJoin The PageManager of join tree.
      * @param distRepres The distance of the representative of indexed and
      * joined tree.
      * @param range The range of the results.
      * @param globalResult The query result.
      * @param activeBuffer Active buffered of sub nodes in Joined Tree.
      * @warning The instance of tResult returned must be destroied by user.
      * @autor Implemented by Enzo Seraphim
      */
      void JoinedTreeRangeJoinRecursive(ObjectType * objIndex,
            double radiusObjIndex, stSeqNode * currNodeJoin,
            double radiusObjJoin, stPageManager * PageManagerJoin,
            double distRepres, const double range,
            stSeqLeafNode * leafNodeIndex, tJoinedResult * globalResult);

      /**
      * Updates the distances of the objects from the new representative.
      */
      void UpdateDistances(stSeqIndexNode * node, ObjectType * repObj,
                           u_int32_t repObjIdx);

      /**
      * This method travels through the tree gathering information about each level.
      *
      * @param pageID Root of the subtree.
      * @param level Current level (zero for the first call).
      * @param info Tree information.
      */
      void GetTreeInfoRecursive(u_int32_t pageID, int level,
                                stTreeInformation * info);

      /**
      * This method travels through the tree gathering information about the
      * intersections of a given object.
      *
      * @param pageID Root of this sub-tree.
      * @param obj Object.
      * @param level Current level.
      * @param info Tree information.
      */
      void ObjectIntersectionsRecursive(u_int32_t pageID, ObjectType * obj,
                                        int level, stTreeInformation * info);
            

      // Shrink implementaion
      /**
      * This method performs the Shrink in the whole tree recursively. I starts from the
      * root traveling through the tree in post order. Every time it reaches an index node
      * in the second level from bottom (the index node above the leafs) it calls the method
      * Shrink() to perform a local Shrink. The radius of each subtree is updated during
      * the proccess to keep the tree consistent.
      *
      * @param pageID Root of the subtree.
      * @param level Current level (the first call must be 0).
      * @return The new radius of the subtree.
      * @warning This method will not work if the tree has less than 3 levels.
      */
      double ShrinkRecursive(u_int32_t pageID, int level);

      /**
      * This method performs the local Shrink in the given subtree.
      *
      * @param pageID The subtree root.
      * @return The new radius of the subtree.
      */      
      double Shrink(u_int32_t pageID);

      /**
      * Perform the Shrink in a set of stSeqMemLeafNode.
      *
      * @param memLeafNodes Leaf nodes.
      * @param nodeCount Number of nodes in memLeafNodes.
      * @param maxSwaps Swap limit.
      */
      void LocalShrink(tMemLeafNode ** memLeafNodes, int nodeCount, int maxSwaps);

      /**
      * Verifies if the last object of src can be moved to dst. It will test:
      *     - dst covers src's last object
      *     - dst free space
      *
      * @param src Source node.
      * @param dst Destination node.
      * @retval distance Distance from the last object of src to the
      * representative of dst. 
      * @return True if it can be swapped of false otherwise.
      * @warning The occupation of src is never tested. 
      */
      bool ShrinkCanSwap(tMemLeafNode * src, tMemLeafNode * dst,
                         double & distance);
                           
};//end stSeqTree

// Include implementation
#include "stSeqTree-inl.h"

#endif //__STSEQTREE_H
