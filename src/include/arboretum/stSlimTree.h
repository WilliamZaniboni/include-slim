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
* This file defines the class stSlimTree.
*
* @version 1.1
* @author Fabio Jun Takada Chino(chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Josiel Maimone de Figueiredo (josiel@icmc.usp.br)
* @author Thiago Galbiatti Vespa (thiago@icmc.sc.usp.br)
*/

#ifndef __STSLIMTREE_H
#define __STSLIMTREE_H

#include <stdlib.h>
#include <arboretum/stUtil.h>
#include <arboretum/stMetricTree.h>
#include <arboretum/stSlimNode.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stGenericPriorityQueue.h>

// this is used to set the initial size of the dynamic queue
#ifndef STARTVALUEQUEUE
   #define STARTVALUEQUEUE 100
#endif //STARTVALUEQUEUE
// this is used to set the increment size of the dynamic queue
#ifndef INCREMENTVALUEQUEUE
   #define INCREMENTVALUEQUEUE 5
#endif //INCREMENTVALUEQUEUE

#ifdef __stFRACTALQUERY__
   #define SIZERINGCALLS 10
#endif //__stFRACTALQUERY__

#ifndef SECUREVALUE
   #define SECUREVALUE 1.2
#endif //SECUREVALUE

#include <string.h>
#include <math.h>
//#include <values.h>
#include <algorithm>
#include <arboretum/stMAMView.h> // Visualization support

#include <stack>
#include <vector>

// Include disk access statistics classes
#ifdef __stDISKACCESSSTATS__
   #include <arboretum/stHistogram.h>
   #include <arboretum/stLevelDiskAccess.h>
#endif //__stDISKACCESSSTATS__


#ifdef __BULKLOAD__
//=============================================================================
// Class template SampleSon
//-----------------------------------------------------------------------------
/**
* @version 1.0
* @author Thiago Galbiatti Vespa (thiago@icmc.sc.usp.br)
* @todo Documentation review.
* @ingroup slim
*/
template <class ObjectType>
class SampleSon {
  ObjectType* object;
  double distance;
public:
  SampleSon() {
     object = NULL;
     distance = 0.0;
  }
  SampleSon(ObjectType* object, u_int32_t size) {
     this->object = object;
     distance = 0.0;
     this->size = size;
  }
  SampleSon(ObjectType* object, double dist) {
     this->object = object;
     this->distance = dist;
  }

  SampleSon &operator=(ObjectType* x) {
     object = x;
     distance = 0.0;
     return *this;
  }

  ObjectType* getObject() {
    return object;
  }

  double getDistance() {
    return distance;
  }

  void setDistance(double distance) {
    this->distance = distance;
  }

};

#endif //__BULKLOAD__

//=============================================================================
// Class template stSlimLogicNode
//-----------------------------------------------------------------------------
/**
* Thic class template represents a SlimTree logic node entry. It is used to
* hold an index node or a leaf node in a memory form which allows better way to
* manipulate entries. It also implements means to distribute its contents
* between 2 nodes.
*
* <P>Each entry of this node can hold all information required by both nodes at
* same time so, the users of this node must control if it holds an index node or
* a leaf node.
*
* @warning This node will take the ownership of all object instances added to
* this node. It means that the object instance will be destroyed by the
* destructor of this class.
*
* @version 1.1
* @author Fabio Jun Takada Chino (chino@icmc.sc.usp.br)
* @author Thiago Galbiatti Vespa (thiago@icmc.sc.usp.br)
* @todo Documentation review.
* @todo Tests!
* @ingroup slim
*/

#ifdef __stCKNNQ__

typedef struct {
    u_int32_t PageId;
    u_int32_t OffSet;
    
} RowId;

typedef struct {
    bool operator()(RowId rowID1, RowId rowID2)
    {
    if(rowID1.PageId < rowID2.PageId)
        return true;
    if ((rowID1.PageId == rowID2.PageId) && (rowID1.OffSet < rowID2.OffSet))
        return true;
    return false;
    }
} rowidComparator;

#endif // __stCKNNQ__

template <class ObjectType, class EvaluatorType>
class stSlimLogicNode{
   public:
      /**
      * Creates a new instance of this node with no objects.
      *
      * @param maxOccupation The maximum number of entries.
      */
      stSlimLogicNode(u_int32_t maxOccupation);

      /**
      * Disposes this instance and releases all related resources. All instances of
      * object added to this node will also be deleted unless it is not owned by
      * this node (see method BuyObject()).
      */
      ~stSlimLogicNode();

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


      int AddEntryForIndex(u_int32_t size, const unsigned char * object);

      /**
      * Returns the number of entries in this node.
      */
      u_int32_t GetNumberOfEntries(){
         return Count;
      }//end GetNumberOfEntries

      /**
      * Returns the maximum number of entries in this node.
      */
      u_int32_t GetMaxEntries() {
         return MaxEntries;
      }//end GetMaxEntries

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
      * Returns the PageID of a given entry. 
      * Only index nodes will use this field.
      *
      * @param idx The object index.
      */
      u_int32_t GetPageID(int idx){
         return Entries[idx].PageID;
      }//end GetPageID

      /**
      * Returns the number of entries in the sub-tree of a 
      * given entry. Only index nodes will use this field.
      *
      * @param idx The object index.
      */
      u_int32_t GetNEntries(int idx){
         return Entries[idx].NEntries;
      }//end GetNEntries

      /**
      * Returns the radius of the sub-tree of a given entry. 
      * Only index nodes will use this field.
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
      * @param nEntries The number of entries in the sub-tree.
      * @param radius The radius of the sub-tree.
      */
      void SetEntry(int idx, u_int32_t pageID, u_int32_t nEntries, double radius){
         Entries[idx].PageID = pageID;
         Entries[idx].NEntries = nEntries;
         Entries[idx].Radius = radius;
      }//end GetPageID

      /**
      * Adds all objects of an index node. It will also set the node type to
      * stSlimNode::INDEX.
      *
      * @param node The node.
      */
      void AddIndexNode(stSlimIndexNode * node);

      /**
      * Adds all objects of a leaf node. It will also set the node type to
      * stSlimNode::LEAF.
      *
      * @param node The node.
      */
      void AddLeafNode(stSlimLeafNode * node);

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
      u_int32_t Distribute(stSlimIndexNode * node0, ObjectType * & rep0,
                         stSlimIndexNode * node1, ObjectType * & rep1,
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
      u_int32_t Distribute(stSlimLeafNode * node0, ObjectType * & rep0,
                         stSlimLeafNode * node1, ObjectType * & rep1,
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
      u_int32_t TestDistribution(stSlimIndexNode * node0, stSlimIndexNode * node1,
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
      u_int32_t TestDistribution(stSlimLeafNode * node0, stSlimLeafNode * node1,
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
      * Returns the node type. It may assume the values stSlimNode::INDEX or
      * stSlimNode::LEAF.
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
      * This type represents a slim tree logic node entry.
      */
      struct stSlimLogicEntry{
         /**
         * Object.
         */
         ObjectType * Object;

         /**
         * ID of the page.
         */
         u_int32_t PageID;

         /**
         * Number of entries in the sub-tree.
         */
         u_int32_t NEntries;

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
      struct stSlimLogicEntry * Entries;

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
      
};//end stSlimLogicNode

//=============================================================================
// Class template stSlimMSTSpliter
//-----------------------------------------------------------------------------
/**
* This class template implements the SlimTree MST split algorithm.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.sc.usp.br)
* @todo Documentation review.
* @todo Tests!
* @ingroup slim
*/
template <class ObjectType, class EvaluatorType>
class stSlimMSTSplitter{
   public:
      /**
      * This type defines the logic node for this class.
      */
      typedef stSlimLogicNode < ObjectType, EvaluatorType > tLogicNode;

      /**
      * Builds a new instance of this class. It will claim the ownership of the
      * logic node provided as input.
      */
      stSlimMSTSplitter(tLogicNode * node);

      /**
      * Disposes all associated resources.
      */
      ~stSlimMSTSplitter();

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
      int Distribute(stSlimIndexNode * node0, ObjectType * & rep0,
                     stSlimIndexNode * node1, ObjectType * & rep1,
                     EvaluatorType * metricEvaluator);

      /**
      * Distributes objects between 2 leaf nodes.
      */
      int Distribute(stSlimLeafNode * node0, ObjectType * & rep0,
                     stSlimLeafNode * node1, ObjectType * & rep1,
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

};//end stSlimMSTSplitter

//=============================================================================
// Class template stSlimTree
//-----------------------------------------------------------------------------
/**
* This class defines all behavior of the SlimTree.
* Probably most of the atributes will be stored in the header page of the used
* stPageManager (stDiskPageManager or stMemoryPageManager).
*
* <P> First implementation is based on original code by Agma Traina, Bernard Seeger
* and Caetano Traina
* <P> Main modifications from original code are intent to turn it an object oriented
* compliant code.
*
* @author Fabio Jun Takada Chino (chino@icmc.sc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.sc.usp.br)
* @author Josiel Maimone de Figueiredo (josiel@icmc.sc.usp.br)
* @todo More documentation.
* @version 1.0
* @ingroup slim
*/
template <class ObjectType, class EvaluatorType>
class stSlimTree: public stMetricTree <ObjectType, EvaluatorType> {
   public:
      /**
      * This is the class that abstracts the object used by this metric tree.
      */
      typedef ObjectType tObject;

      long maxQueue;
      long minQueue;
      long avgQueue;
      long sumOperationsQueue;
      int plotSplitSequence;

      /**
      * This structure defines the SlimTree header structure. This type was left
      * public to allow the creation of debug tools.
      */
      typedef struct tSlimHeader{
         /**
         * Magic number. This is a short string that must contains the magic
         * string "SLIM". It may be used to validate the file (this feature
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
         * The root of the Slim-tree
         */
         u_int32_t Root;
      
         /**
         * Minimum percentage of objects in a node.
         */
         double MinOccupation;

         /**
         * Maximum number of objects in a node.
         */
         u_int32_t MaxOccupation;

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
      }stSlimHeader;   

      /**
      * These constants are used to define the choose sub tree method.
      */
      enum tChooseMethod{
         /**
         * Choose the first of the qualifying nodes.
         */
         cmBIASED,
         /**
         * Randomly choose one of the qualifying nodes.
         */
         cmRANDOM,
         /**
         * Choose the node that has the minimum distance from the
         * new object and the representative (center) of the node.
         */
         cmMINDIST,

         /**
         * Choose the node that has the minimum occupancy among the
         * qualifying ones. This is the default method due to its better
         * performance.
         */
         cmMINOCCUPANCY,
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
         * This method peeks 2 random objects as the representatives and
         * distribute the other objects around them.
         *
         * <p>This is the fastest split method but the result is undetermined.
         */
         smRANDOM,

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
         * This is the Slim-Tree default split method.
         */
         smSPANNINGTREE
      };//end tSplitMethod

     
      //typedef stTOResult <ObjectType, KeyType> myIntResult;
      /**
      * This is the class that abstracts an result set for simple queries.
      */
      typedef stResult <ObjectType> tResult;
      
#ifdef __stCKNNQ__
      
      typedef stConstrainedResult <ObjectType> tConstrainedResult;
      
#endif // __stCKNNQ__
      /**
      * This is the class that abstracts an result paged set for queries.
      */
      typedef stResultPaged <ObjectType> tResultPaged;

      /**
      * This is the class that abstracts an result set for joined queries.
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

      typedef stMetricTree <ObjectType, EvaluatorType> tMetricTree;

      /**
      * Memory leaf node used by Slim-Down.
      */
      typedef stSlimMemLeafNode < ObjectType > tMemLeafNode;

      #ifdef __stDISKACCESSSTATS__
         typedef stHistogram < ObjectType, EvaluatorType > tHistogram;
      #endif //__stDISKACCESSSTATS__

      /**
      * Creates a new metric tree using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this metric tree.
      */
      stSlimTree(stPageManager * pageman);

      /**
      * Creates a new metric tree using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this metric tree.
      * @param metricEval The shared metric evaluator be used by this metric tree.
      * @warning metricEval will not be delete by this class.
      */
      stSlimTree(stPageManager * pageman, EvaluatorType * metricEval);

      /**
      * Dispose all used resources, ie, it is the destructor method.
      *
      * @see stSlimTree()
      */
      virtual ~stSlimTree();

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
      virtual u_int32_t GetMaxOccupation(){
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
       * Computes the number of elements that an index node can hold.
       * The function considers every object is the same size.
       * @return index node capacity
       */
      int GetIndexCapacity() {
            stPage * currPage;
            stSlimNode * currNode;
            ObjectType tmpObj;
            u_int32_t idx, numberOfEntries;
            u_int32_t pageID = this->GetRoot();
            int capacity = 0;
            if (pageID != 0){
                currPage = this->myPageManager->GetPage(pageID);
                currNode = stSlimNode::CreateNode(currPage);
                if (currNode->GetNodeType() == stSlimNode::INDEX){
                    stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
                    int objSize = indexNode->GetObjectSize(0);
                    int header = indexNode->GetGlobalOverhead();
                    int pagesize = currPage->GetPageSize();
                    capacity = (pagesize - header) / objSize;
                }
                else {
                    capacity = GetLeafCapacity();
                }
                delete currNode;
                this->myPageManager->ReleasePage(currPage);
            }
            return capacity;
      }
      
      /**
       * Computes the number of elements that a leaf node can hold.
       * The function considers every object is the same size.
       * @return leaf node capacity
       */
      int GetLeafCapacity() {
            stPage * currPage;
            stSlimNode * currNode;
            ObjectType tmpObj;
            u_int32_t idx, numberOfEntries;
            u_int32_t pageID = this->GetRoot();
            int capacity = 0;
            while (pageID != 0){
                currPage = this->myPageManager->GetPage(pageID);
                currNode = stSlimNode::CreateNode(currPage);
                if (currNode->GetNodeType() == stSlimNode::INDEX){
                    stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
                    pageID = indexNode->GetIndexEntry(0).PageID;
                }
                else {
                    stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
                    int objSize = leafNode->GetObjectSize(0);
                    int header = leafNode->GetGlobalOverhead();
                    int pagesize = currPage->GetPageSize();
                    capacity = (pagesize - header) / objSize;
                    pageID = 0;
                }
                delete currNode;
                this->myPageManager->ReleasePage(currPage);
            }
            return capacity;
      }
      
      /**
      * Returns the number of nodes of this tree.
      */
      virtual long GetNodeCount(){
         return Header->NodeCount;
      }//end GetNodeCount

      /**
      * Returns the total number of index nodes of this tree.
      */
      long GetIndexNodeCount();
      
      /**
      * Returns the total number of leaf nodes of this tree.
      */
      long GetLeafNodeCount();

      /**
      * Returns the average radius in leaf nodes of this tree.
      */
      double GetAvgRadiusLeaf();

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

      #ifdef __stDISKACCESSSTATS__
         /**
         * Return the an estimation of the average of disk access for a Range query
         * with radius range.
         * This formula is based on the paper Traina, Traina, Faloutsos and Seeger,
         * TKDE'2000.
         *
         * @param range the radius of the range query.
         * @param fractalDimension the fractal dimension of the index dataset.
         * @param maxDistance the greater distance between two objects indexed.
         * @see GetEstimateDiskAccesses
         */
         double GetEstimateDiskAccesses(double range,
            double fractalDimension, double maxDistance);

         /**
         * Return the an estimation of the average of disk access for a Range query
         * with radius range. This is a fast way to return this average.
         * This formula is based on the paper Traina, Traina, Faloutsos and Seeger,
         * TKDE'2000.
         *
         * @param range the radius of the range query.
         * @param fractalDimension the fractal dimension of the index dataset.
         * @param maxDistance the greater distance between two objects indexed.
         * @see GetEstimateDiskAccesses
         */
         double GetFastEstimateDiskAccesses(double range,
            double fractalDimension, double maxDistance);

         /**
         * Return the an estimation of the average of disk access for a Range query
         * with radius range. This is a fast way to return this average based on the
         * FatFactor.
         * This formula is based on the paper Traina, Traina, Faloutsos and Seeger,
         * TKDE'2000.
         *
         * @param range the radius of the range query.
         * @param fractalDimension the fractal dimension of the index dataset.
         * @param maxDistance the greater distance between two objects indexed.
         * @see GetEstimateDiskAccesses
         */
         double GetFatFactorFastEstimateDiskAccesses(double fatFactor,
            double range, double fractalDimension, double maxDistance);

         double GetCiacciaEstimateDiskAccesses(double range,
            tHistogram * histogram);

         double GetCiacciaEstimateDistCalculation(double range,
            tHistogram * histogram);

         void CalculateLevelStatistics(stLevelDiskAccess * levelDiskAccess);

         void CalculateLevelStatistics(stLevelDiskAccess * levelDiskAccess,
                  u_int32_t pageID, u_int32_t height);

         double GetCiacciaLevelEstimateDiskAccesses(
                  stLevelDiskAccess * levelDiskAccess, double range,
                  tHistogram * histogram);

         void GenerateLevelHistograms(tHistogram ** histogram);

         void GenerateSampledLevelHistograms(tHistogram ** histogram);

         double GetTestLevelEstimateDiskAccesses(stLevelDiskAccess * levelDiskAccess,
               tHistogram ** histogram, double range);

         double GetSampledLevelEstimateDiskAccesses(stLevelDiskAccess * levelDiskAccess,
               tHistogram ** histogram, double range);
      #endif //__stDISKACCESSSTATS__

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
      * This method will perform a Forward range query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param nObj The number of objects of the query.
      * @param internalRadius The internal radius of the query.
      * @param externalRadius The internal radius of the query.
      * @param oid The last OID  of the previous query.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResultPaged * ForwardRangeQueryWithoutPriority(ObjectType * sample, u_int32_t nObj,
              double internalRadius = 0, double externalRadius = MAXDOUBLE,
              long oid = MAXLONG);

      /**
      * This method will perform a Forward range query using list of priority.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param nObj The number of objects of the query.
      * @param internalRadius The internal radius of the query.
      * @param externalRadius The internal radius of the query.
      * @param oid The last OID  of the previous query.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResultPaged * ForwardRangeQuery(ObjectType * sample, u_int32_t nObj,
              double internalRadius = 0, double externalRadius = MAXDOUBLE,
              long oid = MAXLONG);

      /**
      * This method will perform a Backward range query without list of the priority.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param nObj The number of objects of the query.
      * @param internalRadius The internal radius of the query.
      * @param externalRadius The internal radius of the query.
      * @param oid The last OID  of the previous query.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResultPaged * BackwardRangeQueryWithoutPriority(ObjectType * sample, u_int32_t nObj,
              double internalRadius = 0, double externalRadius = MAXDOUBLE,
              long oid = MAXLONG);

      /**
      * This method will perform a Backward range query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param nObj The number of objects of the query.
      * @param internalRadius The internal radius of the query.
      * @param externalRadius The internal radius of the query.
      * @param oid The last OID  of the previous query.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResultPaged * BackwardRangeQuery(ObjectType * sample, u_int32_t nObj,
              double internalRadius = 0, double externalRadius = MAXDOUBLE,
              long oid = MAXLONG);

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

      tResult * GetEmptyResult();

      tResult * ExistsQuery(ObjectType * sample, double range);


      /**
      * This method will perform a reverse of range query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param range The range of the results. All object that are
      * greater than the range distance will be included in the result set.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void RangeQuery()
      */
      tResult * ReversedRangeQuery(ObjectType * sample, double range);

      /**
      * This method will perform a K-Nearest Neighbor query using a priority
      * queue to "enhance" its performance. We believe that the use of a priority
      * queue during the search in the index nodes will force the radius to converge
      * faster than the normal nearest query.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void NearestQuery
      */
      tResult * LocalNearestQuery(ObjectType * sample, u_int32_t k, bool tie = false);

      /**
      * This method will perform a K-Nearest Neighbor query using a link list.
      * We believe that the use of a priority queue during the search in the
      * index nodes will force the radius to converge faster than the normal
      * nearest query.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void NearestQuery
      */
      tResult * ListNearestQuery(ObjectType * sample, u_int32_t k, bool tie = false);

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
      tResult * NearestQuery(ObjectType * sample, u_int32_t k, bool tie = false, bool tiebreaker = false);

      /**
      * This method will perform a K-Farthest Neighbor query using a global priority
      * queue based on chained list to "enhance" its performance. We believe that the
      * use of a priority queue during the search in the index nodes will force the
      * radius to converge faster than the normal farthest query.
      *
      * <p>This implementation may replace the original implementation if it proves
      * to be better.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void FarthestQuery
      */
      tResult * FarthestQuery(ObjectType * sample, u_int32_t k, bool tie = false);

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
      * This method will inicializate the parameters to perform a
      * K-Nearest Neighbor query using a global priority queue based
      * on a dynamic link list.
      *
      * <P>This method is based on Hjaltson and Samet (ACM TODS-2000). It is
      * possible to get k+1 object after retrieve the k's objects.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void IncrementalNearestQuery
      */
      tResult * IncrementalListNearestQuery(ObjectType * sample, u_int32_t k,
                                            bool tie = false);

      /**
      * This method will inicializate the parameters to perform a
      * K-Nearest Neighbor query using a global priority queue based
      * on a dynamic heap to "enhance" its performance.
      *
      * <P>This method is based on Hjaltson and Samet (ACM TODS-2000). It is
      * possible to get k+1 object after retrieve the k's objects.
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void IncrementalNearestQuery
      */
      tResult * IncrementalNearestQuery(ObjectType * sample, u_int32_t k,
                                        bool tie = false);

      /**
      * This method will inicializate the parameters to perform a
      * K-Nearest Neighbor query using a global priority queue based
      * on a dynamic heap to "enhance" its performance.
      *
      * <P>This method is based on Hjaltson and Samet (ACM TODS-2000). It is
      * possible to get k+1 object after retrieve the k's objects.
      *
      * <P>This is possible because the global priority is not delete and
      * the subtrees are not pruned. 
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param The result.
      * @param globalQueue a queue to be managed.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The instance of tPGenericHeap must be created and destroied by user.
      * @see void IncrementalNearestQuery
      */
      void InitializeIncrementalNearestQuery(ObjectType * sample, u_int32_t k,
                                             tResult * result,
                                             tPGenericHeap * globalQueue);

      /**
      * This method will inicializate the parameters to perform a
      * K-Nearest Neighbor query using a global priority queue based
      * on a dynamic list.
      *
      * <P>This method is based on Hjaltson and Samet (ACM TODS-2000). It is
      * possible to get k+1 object after retrieve the k's objects.
      *
      * <P>This is possible because the global priority is not delete and
      * the subtrees are not pruned. 
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param The result.
      * @param globalQueue a queue to be managed.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The instance of tPGenericHeap must be created and destroied by user.
      * @see void IncrementalNearestQuery
      */
      void InitializeIncrementalNearestQuery(ObjectType * sample, u_int32_t k,
                                             tResult * result,
                                             tGenericPriorityQueue * globalQueue);

      /**
      * This method will perform a K-Nearest Neighbor query using a global
      * priority queue based on a dynamic heap to "enhance" its performance.
      *
      * <P>This method is based on Hjaltson and Samet (ACM TODS-2000). It is
      * possible to get k+1 object after retrieve the k's objects.
      *
      * <P>This is possible because the global priority is not delete and
      * the subtrees are not pruned. 
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param The result.
      * @param globalQueue a queue to be managed.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The instance of tPGenericHeap must be created and destroied by user.
      * @warning This method must be called after InitializeIncrementalNearestQuery.
      * @see void InitializeIncrementalNearestQuery
      */
      void IncrementalNearestQuery(ObjectType * sample, u_int32_t k,
                                   tResult * result,
                                   tPGenericHeap * globalQueue);

      /**
      * This method will perform a K-Nearest Neighbor query using a global
      * priority queue based on a dynamic list.
      *
      * <P>This method is based on Hjaltson and Samet (ACM TODS-2000). It is
      * possible to get k+1 object after retrieve the k's objects.
      *
      * <P>This is possible because the global priority is not delete and
      * the subtrees are not pruned. 
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param The result.
      * @param globalQueue a queue to be managed.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The instance of tPGenericHeap must be created and destroied by user.
      * @warning This method must be called after InitializeIncrementalNearestQuery.
      * @see void InitializeIncrementalNearestQuery
      */
      void IncrementalNearestQuery(ObjectType * sample, u_int32_t k,
                                   tResult * result,
                                   tGenericPriorityQueue * globalQueue);

      /**
      * This method will perform a K-Nearest Neighbor query with the use of
      * a Correlation Dimension Fractal.
      *
      * @param sample The sample object.
      * @param fractalDimension The Correlation Dimension Fractal of the data sets.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see tResult * NearestQuery
      */
      tResult * EstimateNearestQuery(ObjectType * sample, double fractalDimension,
                                     long totalNroObjects, double maxDistance,
                                     u_int32_t k, bool tie = false);

      /**
      * This method will perform a K-Nearest Neighbor query with the use of
      * a Correlation Dimension Fractal based on a local chained list.
      *
      * @param sample The sample object.
      * @param fractalDimension The Correlation Dimension Fractal of the data sets.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see tResult * NearestQuery
      */
      tResult * LocalEstimateNearestQuery(ObjectType * sample, double fractalDimension,
                                          long nroObjects, double radiusTree,
                                          u_int32_t k, bool tie = false);

      #ifdef __stFRACTALQUERY__

         /**
         * Reset all the fractal statistics.
         */
         void  ResetFractalStatistics();

         /**
         * Return the number of sucessfull guesses.
         */
         int GetNumberOfGoodGuesses(){
            return GoodGuesses;
         }//end GetNumberOfGoodGuesses

         /**
         * Return the number of ring calls.
         */
         int GetNumberOfRingCalls(int idx){
            if (idx < SIZERINGCALLS){
               return RingCalls[idx];
            }else{
               return -1;
            }//end if
         }//end GetNumberOfRingCalls
         
      #endif //__stFRACTALQUERY__

      /**
      * This method will perform a range query with a limited number of results.
      *
      * <P>This query is a combination of the standard Range Query and the standard
      * K-Nearest Neighbor Query. All objects which matches both conditions
      * will be included in the result. The K-Nearest Neighbor Query is implemented
      * with a local priority queue.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @param tie The tie list. This parameter is optional. Default false;
      * @warning The instance of tResult returned must be destroied by user.
      * @see void KAndRangeQuery
      * @warning This method does not work for trees with only one node.
      */
      tResult * KAndRangeQuery(ObjectType * sample, double range,
                               u_int32_t k, bool tie = false);

      /**
      * This method will perform range query with a limited number of results.
      *
      * <P>This query is a combination of the standard range query and the standard
      * K-Nearest Neighbor Query. All objects which matches with one of two conditions
      * will be included in the result. The K-Nearest Neighbor Query is implemented
      * with a local priority queue.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @param tie The tie list. This parameter is optional. Default false;
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void KOrRangeQuery
      * @warning This method does not work for trees with only one node.
      */
      tResult * KOrRangeQuery(ObjectType * sample, double range,
                              u_int32_t k, bool tie = false);

      /**
      * This method will perform the disjunctive complex similarity query between
      * range and k-nearest neighbor operator. However, in this method, this is
      * done through of range queries.  
      *
      * <P>This query is a combination of the standard range query and the standard
      * K-Nearest Neighbor Query. All objects which matches with one of two conditions
      * will be included in the result. The K-Nearest Neighbor Query is implemented
      * with a global priority queue.
      *
      * @param sample The sample object.
      * @param fractalDimension The fractal dimension of the data set.
      * @param nroObjects The total number of objects of the data set.
      * @param radiusKOrRange Th radius of the range operator.
      * @param k The maximum number of results.
      * @param tie The tie list. This parameter is optional. Default false;
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void KOrRangeQuery
      */
      tResult * SimpleKOrRangeQuery(ObjectType * sample, double fractalDimension,
                                    double radiusKOrRange, u_int32_t k,
                                    bool tie = false);

      /**
      * This method will perform a ring query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param inRange The outter range of the results.
      * @param outRange The inner range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The value of outRange must be higher than inRange.
      * @see void RingQuery()
      */
      tResult * RingQuery(ObjectType * sample, double inRange,
                          double outRange);

      /**
      * This method will perform a ring query with K-Nearest Neighbor query.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param inRange The outter range of the results.
      * @param outRange The inner range of the results.
      * @param k The number of nearest neighbor.
      * @param tie The tie list. This parameter is optional. Default false;
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The value of outRange must be higher than inRange.
      * @see void RingKQuery()
      */
      tResult * KRingQuery(ObjectType * sample, double inRange,
                           double outRange, u_int32_t k, bool tie = false);
      void KRingQuery(tResult * result, ObjectType * sample, double inRange,
                      double & outRange, u_int32_t k);

      /**
      * This method will perform a ring query with K-Nearest Neighbor query
      * based on a local chained list.
      * The result will be a set of pairs object/distance.
      *
      * @param sample The sample object.
      * @param inRange The outter range of the results.
      * @param outRange The inner range of the results.
      * @param k The number of nearest neighbor.
      * @param tie The tie list. This parameter is optional. Default false;
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @warning The value of outRange must be higher than inRange.
      * @see void RingKQuery()
      */
      tResult * LocalKRingQuery(ObjectType * sample, double inRange,
                                double outRange, u_int32_t k, bool tie = false);

      /**
      * This method will perform a range query with a limited number of results.
      * <P>A lazy range query recovers the k elements within a radius, independently
      * if the k elements are the k nearest elements of query object.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @param tie The tie list. This parameter is optional. Default false;
      * @warning The instance of tResult returned must be destroied by user.
      * @see void LazyRangeQuery
      */
      tResult * LazyRangeQuery(ObjectType * sample, double range,
                               u_int32_t k, bool tie = false);

      /**
      * This method will perform a aproximate K-Nearest Neighbor query
      *
      * @param sample The sample object.
      * @param k The number of neighbors.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      * @see void AproximateNearestQuery
      */
      tResult * AproximateNearestQuery(ObjectType * sample, u_int32_t k,
                                       bool tie = false);

      /**
      * This method will perform a k-nearest neighbor join query.
      *
      * @param slimTree The tree being joined.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      * @autor Implemented by Enzo Seraphim
      */
      tJoinedResult * NearestJoinQuery(stSlimTree * slimTree, u_int32_t k,
                                       bool tie = false);

      /**
      * This method will perform a range joined query.
      *
      * @param slimTree The tree being joined.
      * @param range The range of the results.
      * @param activeBuffer Active buffer of sub nodes in Joined Tree.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      */
      tJoinedResult * RangeJoinQuery(stSlimTree * slimTree, double range,
                                     bool buffer = true);

      /**
      * This method will perform a generic range join query with a metric tree.
      * It will perform a range query for each object in the first tree.
      *
      * @param joinedTree A metric tree to be joined.
      * @param range The radius of the query.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      */
      tJoinedResult * DummyRangeJoinQuery(
                           stMetricTree<ObjectType, EvaluatorType> * joinedTree,
                           double range);

      /**
      * Aggregate Range Query.
      * @param numerator grid power g numerator. g = numerator/denominator
      * @param denominator grid power g denominator. g = numerator/denominator
      * @param sampleList list of query centers
      * @param sampleSize number of objects in sampleList
      * @param range aggregate query radius
      */
      tResult * AggregateRangeQuery(double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, double range, double *weights = NULL);

      /**
      * Aggregate Nearest Query.
      * @param numerator grid power g numerator. g = numerator/denominator
      * @param denominator grid power g denominator. g = numerator/denominator
      * @param sampleList list of query centers
      * @param sampleSize number of objects in sampleList
      * @param k number of nearest neighbors
      */
      tResult * AggregateNearestQuery(double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, u_int32_t k, bool tie = false, double *weights = NULL);

	  /**
      * Calculates the FatFactor of this tree.
      *
      * @warning This method will update the statistics of the tree.
      */
      double GetFatFactor();


      #ifdef __BULKLOAD__

         enum tBulkMethod{
            /**
            * Choose the first object as representative.
            */
            bulkBIASED,
            /**
            * Randomly choose one representative.
            */
            bulkRANDOM,
            /**
            * Creates a node that has the minimum distance from the
            * new object and the representative (center) of the node.
            */
            bulkMINDIST
         };//end tBulkMethod

         enum tBulkType{
            /**
            * Fixed number of objects in a node
            */
            bulkFIXED,
            /**
            * Calculated number of object in a node.
            */
            bulkFUNCTION,
            /**
            * Number of object in a range
            */
            bulkRANGE
         };//end tBulkType



         /**
         * This method bulk insert objects into this metric tree.
         *
         * @param obj The objects to be added.
         */
         bool BulkLoadOrdered(ObjectType **newObj, u_int32_t numObj);
         bool BulkLoadOrdered(ObjectType **newObj, u_int32_t numObj, enum tBulkMethod method);
         bool BulkLoadOrdered(ObjectType **newObj, u_int32_t numObj, double nodeOccupancy, enum tBulkMethod method);
         bool BulkLoadOrdered(ObjectType **newObj, u_int32_t numObj, double leafNodeOccupancy, double indexNodeOccupancy, enum tBulkMethod method);

         bool BulkLoadMemory(ObjectType **objects, u_int32_t numObj, enum tBulkType type);
         bool BulkLoadMemory(ObjectType **objects, u_int32_t numObj, double nodeOccupancy, enum tBulkType type);
         bool BulkLoadMemory(ObjectType **objects, u_int32_t numObj, double nodeOccupancy, u_int32_t objSize, enum tBulkType type);
         bool BulkLoadMemory(vector< SampleSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, enum tBulkType type);

         
         
      #endif //__BULKLOAD__

      /**
      * This method checks for inconsistencies in the Slim-tree.
      * Basically, this methods finds radius, distances, and
      * other information that may be set wrong in the algorithm.
      * This method is very useful in the developement processes to track
      * implementation errors that a developer may introduce in his/her 
      * implementation.
      *
      * @see Consistency(u_int32_t pageID, ObjectType * repObj, double & radius)
      */
      bool Consistency();

      // Visualization support
      #ifdef __stMAMVIEW__
         /**
         * Sets the visualization output directory.
         *
         * @param dir The directory name.
         * @warning This method does not exist if __stMANVIEW__ is not defined.
         */
         void MAMViewSetOutputDir(const char * dir){

            MAMViewer->SetOutputDir(dir);
         }//end MAMViewSetOutputDir

         /**
         * Initializes the MAMView module. It will restart he
         *
         * @param dir The directory name.
         * @warning This method does not exist if __stMANVIEW__ is not defined.
         */
         void MAMViewInit();

         /**
         * Creates a dump of the tree which can be viewed by MAMViewer.
         *
         * @warning This method does not exist if __stMANVIEW__ is not defined.
         */
         void MAMViewDumpTree();
      #endif //__stMAMVIEW__

      /**
      * @copydoc stMetricTree::GetTreeInfo()
      */
      virtual stTreeInfoResult * GetTreeInfo();

      /**
      * Optimizes the structure of this tree by executing the Slim-Down
      * algorithm.
      *
      * <p>The Slim-Down algorithm can only be performed when the tree has at
       * least 3 levels. Fortunately, this
      */
      virtual void Optimize();

#ifdef __stCKNNQ__

     /**
       * OBS: constrained kNN that accesses data blocks already output the tuples that satisfy the constraint, not the rowids.
       */
      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * preConstrainedNearestQuery(tObject * sample, u_int32_t k, u_int32_t idxConstraint,
          bool (*compare)(const void *, const void *), const void * value,
          DataBlockManagerType& dataBlockManager);



      
      template <class TupleTypeIndex, class RowId>
      tResult * BConstrainedNearestQuery(std::multiset<RowId,rowidComparator> m, tObject * sample, u_int32_t k);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountGreaterThanOrEqual(//Obs: CountGreaterThan(5) can be CountGreaterThanOrEqual(6)
          tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
          u_int32_t aggValue, DataBlockManagerType& dataBlockManager);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountLessThanOrEqual(//Obs: CountLessThan(5) can be CountLessThanOrEqual(4)
          tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
          u_int32_t aggValue, DataBlockManagerType& dataBlockManager);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual(
        tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
        u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue, DataBlockManagerType& dataBlockManager);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountDistinctLessThanOrEqual(
        tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
        u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue, DataBlockManagerType& dataBlockManager);


      /**
       * OBS: constrained kNN over covering indexes output the rowids.
       */
      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * preConstrainedNearestQuery(tObject * sample, u_int32_t k, u_int32_t idxConstraint,
          bool (*compare)(const void *, const void *), const void * value);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountGreaterThanOrEqual(//Obs: CountGreaterThan(5) can be CountGreaterThanOrEqual(6)
          tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
          u_int32_t aggValue);


      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountLessThanOrEqual(//Obs: CountLessThan(5) can be CountLessThanOrEqual(4)
          tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
          u_int32_t aggValue);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual(
        tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
        u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue);


      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * intraConstrainedNearestQueryCountDistinctLessThanOrEqual(
        tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
        u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * BSlimIntraConstrainedNearestQueryCountGreaterThanOrEqual(
        std::multiset<RowId, rowidComparator> m,tObject * sample, u_int32_t k,
        u_int32_t aggValue, DataBlockManagerType& dataBlockManager);

      template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
      tResult * BSlimIntraConstrainedNearestQueryCountLessThanOrEqual(
        std::multiset<RowId, rowidComparator> m,tObject * sample, u_int32_t k,
        u_int32_t aggValue, DataBlockManagerType& dataBlockManager);
#endif // __stCKNNQ__
      

   private:

      /**
      * This method compute the grouping distance from an object to a set of objects.
      */
      double GroupDistance(double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, ObjectType *obj, double *weights = NULL);

      /**
      * Computes the aggregate distance to a Slim-tree subtree
      */
      double AggregateDistanceToSlimNode(double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, ObjectType *representativeObject, double nodeRadius, double *weights);

      /**
      * Recursion of AggregateRangeQuery.
      */
      void AggregateRangeQuery(u_int32_t pageID, tResult * result, double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, double range, double *weights);
       
      /**
      * This type defines the logic node for this class.
      */
      typedef stSlimLogicNode < ObjectType, EvaluatorType > tLogicNode;

      /**
      * This type defines the MST splitter for this class.
      */
      typedef stSlimMSTSplitter < ObjectType, EvaluatorType > tMSTSplitter;

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

         /**
         * Number of objects in the subtree.
         */
         u_int32_t NObjects;
      };

      // Visualization support
      #ifdef __stMAMVIEW__
         /**
         * Type of the object sample array. Used by the visualization module.
         *
         * @warning This method does not exist if __stMAMVIEW__ is not defined.
         */
         typedef stMAMViewObjectSample <ObjectType> tObjectSample;

         /**
         * Type of the MAM view extractor for this tree. Used by the visualization
         * module.
         *
         * @warning This method does not exist if __stMAMVIEW__ is not defined.
         */
         typedef stMAMViewExtractor <ObjectType, EvaluatorType> tViewExtractor;

         /**
         * The MAM Viewer extractor for this tree.
         */
         tViewExtractor * MAMViewer;
      #endif //__stMAMVIEW__

      #ifdef __stFRACTALQUERY__

         /**
         * Used to hold the total number of good guesses that the estimateNearest did.
         */
         int GoodGuesses;

         /**
         * To hold all the number of ring calls that the EstimateNearest did.
         */
         int RingCalls[SIZERINGCALLS];

      #endif //__stFRACTALQUERY__

      #ifdef __BULKLOAD__
         stack<stPage*, vector<stPage*> > rightPathEntries;
      #endif  //__BULKLOAD__

      /**
      * If true, the header mus be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * The SlimTree header. This variable points to data in the HeaderPage.
      */
      stSlimHeader * Header;

      /**
      * Pointer to the header page.
      * The Slim Tree keeps this page in memory for faster access.
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
            tMetricTree::myPageManager->WriteHeaderPage(HeaderPage);
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
         return tMetricTree::myPageManager->GetNewPage();
      }//end NewPage
      
      /**
      * Disposes a given page and updates the page counter.
      */
      void DisposePage(stPage * page){
         Header->NodeCount--;
         tMetricTree::myPageManager->DisposePage(page);
      }//end DisposePage

      /**
      * Recursevely calculates the total number of index nodes of this tree.
      */
      long GetIndexNodeCount(u_int32_t pageID);

      /**
      * Recursevely calculates the total number of leaf nodes of this tree.
      */
      long GetLeafNodeCount(u_int32_t pageID);

      /**
      * Recursevely calculates the average radius in leaf nodes of this tree.
      */
      double GetAvgRadiusLeaf(u_int32_t pageID, u_int32_t height);

      /**
      * This method computes an index of an entry where the insertion process
      * of record obj should continue.
      *
      * @param slimIndexNode the indexNode to be analyzed
      * @param obj The object that will be inserted.
      * @return the minIndex the index of the choose of the subTree
      */
      int ChooseSubTree(stSlimIndexNode * slimIndexNode, ObjectType * obj);

      /**
      * Compute two elements from the page and use them for being the center
      * of the index entries in the parent page of the SlimTree.
      * This is the most simple and inexpensive method for promoting entries
      *
      * @param node The node.
      */
      void RandomPromote(tLogicNode * node);

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
      void ReDistribute(stSlimNode * node, int & newCenter, double & newRadius);

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

      #ifdef __stDISKACCESSSTATS__
         /**
         * Return the an estimation of the average of disk access for a Range query
         * with radius range.
         * This formula is based on the paper Traina, Traina, Faloutsos and Seeger,
         * TKDE'2000.
         * This is a recursive method called only by GetEstimateDiskAccesses
         *
         * @param pageID the pageID to be analyze.
         * @param range the radius of the range query.
         * @param fractalDimension the fractal dimension of the index dataset.
         * @see GetEstimateDiskAccesses
         */
         double GetEstimateDiskAccesses(u_int32_t pageID, double range,
            double fractalDimension);

         double GetCiacciaEstimateDiskAccesses(u_int32_t pageID, double range,
                                               tHistogram * histogram);

         double GetCiacciaEstimateDistCalculation(u_int32_t pageID, double range,
                                               tHistogram * histogram);

         double GetCiacciaLevelEstimateDiskAccesses(u_int32_t pageID, double range,
                                                    tHistogram * histogram);

         void GenerateLevelHistograms(tHistogram ** histogram, u_int32_t pageID,
                                u_int32_t height);

         void GenerateSampledLevelHistograms(tHistogram ** histogram, u_int32_t pageID,
                                             u_int32_t height);
      #endif //__stDISKACCESSSTATS__

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
      * Creates and updates the new root of the SlimTree.
      *
      * @param obj1 Object 1.
      * @param radius1 Radius of subtree 1.
      * @param nodeID1 ID of the root page of the sub-tree 1.
      * @param nEntries1 Number of entries in the sub-tree 1.
      * @param obj2 Object 2.
      * @param radius2 Radius of subtree 2.
      * @param nodeID2 ID of the root page of the sub-tree 2.
      * @param nEntries2 Number of entries in the sub-tree 2.
      */
      void AddNewRoot(ObjectType *obj1, double radius1, u_int32_t nodeID1,
                      u_int32_t nEntries1, ObjectType *obj2, double radius2,
                      u_int32_t nodeID2, u_int32_t nEntries2);

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
      void SplitLeaf(stSlimLeafNode * oldNode, stSlimLeafNode * newNode,
                     ObjectType * newObj, ObjectType * prevRep,
                     stSubtreeInfo & promo1, stSubtreeInfo & promo2);

      void MapSplit(stSlimLeafNode *oldNode, ObjectType *newObj);
      void MapSplit(stSlimLeafNode *oldNode, ObjectType *lRep, stSlimLeafNode *newNode, ObjectType *rRep);
      
      /**
      * This method splits an index node in 2. This will get 2 nodes and will
      * redistribute the object set between these.
      *
      * <P>This method may take one or two new objects to be added to the node
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
      * @param newNEntries1 The new object 1 number of entries.
      * @param newObj2 The new object 2 to be added or NULL.
      * @param newRadius2 The new object 2 radius if newObj2 is not NULL.
      * @param newNodeID2 The new object 2 node ID  if newObj2 is not NULL..
      * @param newNEntries2 The new object 2 number of entries if newObj2 is
      * not NULL.
      * @param prevRep The previous representative.
      * @param promo1 The promoted subtree. If its representative is NULL,
      * the choosen representative is equal to prevRep.
      * @param promo2 The promoted subtree. The representative of this tree will
      * never be the prevRep.
      * @todo redo the FatFactorPromote method.
      */
      void SplitIndex(stSlimIndexNode * oldNode, stSlimIndexNode * newNode,
                      ObjectType * newObj1, double newRadius1,
                      u_int32_t newNodeID1, u_int32_t newNEntries1,
                      ObjectType * newObj2, double newRadius2,
                      u_int32_t newNodeID2, u_int32_t newNEntries2,
                      ObjectType * prevRep,
                      stSubtreeInfo & promo1, stSubtreeInfo & promo2);
	  void MapSplit(stSlimIndexNode *oldNode, ObjectType *lRep, stSlimIndexNode *newNode, ObjectType *rRep);
      
      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param nObj The number of objects of the query.
      * @param internalRadius The internal radius of the query.
      * @param externalRadius The internal radius of the query.
      * @param distanceRepres The distance of the representative.
      * @param oid The last OID  of the previous query.
      * @see tResult * ForwardRangeQueryWithoutPriority()
      */
      void ForwardRangeQueryWithoutPriority(u_int32_t pageID, tResultPaged * result,
                      ObjectType * sample, u_int32_t nObj,
                      double internalRadius,  double & externalRadius,
                      double distanceRepres, long oid);

      /**
      * This method will perform a range query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param nObj The number of objects of the query.
      * @param internalRadius The internal radius of the query.
      * @param externalRadius The internal radius of the query.
      * @param distanceRepres The distance of the representative.
      * @param oid The last OID  of the previous query.
      * @see tResult * BackwardRangeQueryWithoutPriority()
      */
      void BackwardRangeQueryWithoutPriority(u_int32_t pageID, tResultPaged * result,
                      ObjectType * sample, u_int32_t nObj,
                      double & internalRadius,  double externalRadius,
                      double distanceRepres, long oid);

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

      void ExistsQuery(u_int32_t pageID, tResult * result,
                      ObjectType * sample, double range,
                      double distanceRepres);

      /**
      * This method will perform a reverse range query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the result. All object that are
      * greater than the range distance will be included in the result set.
      * @param distanceRepres The distance of the representative.
      * @see tResult * RangeQuery()
      */
      void ReversedRangeQuery(u_int32_t pageID, tResult * result,
                              ObjectType * sample, double range,
                              double distanceRepres);

      /**
      * This method will perform a K Nearest Neighbor query using a global priority
      * queue based on chained list.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The number of neighbours.
      * @param globalQueue The global priority queue.
      * @see tResult * NearestQuery
      */
      void LocalNearestQuery(u_int32_t pageID, tResult * result,
                             ObjectType * sample, double & rangeK,
                             u_int32_t k, double distanceRepres);

      /**
      * This method will perform a K Nearest Neighbor query using a global priority
      * queue based on chained list.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The number of neighbours.
      * @param globalQueue The global priority queue.
      * @see tResult * NearestQuery
      */
      void ListNearestQuery(tResult * result, ObjectType * sample, 
                            double rangeK, u_int32_t k);

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
                        double rangeK, u_int32_t k, bool tiebreaker);

      /**
      * This method will perform a K-Farthest Neighbor query using a priority
      * queue.
      *
      * @param result the result set.
      * @param sample The sample object.
      * @param rangeK The range of the results.
      * @param k The number of farthest neighbours.
      * @see tResult * NearestQuery
      */
      void FarthestQuery(tResult * result, ObjectType * sample,
                         double rangeK, u_int32_t k);

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
      * This method will perform a range query with a limited number of results.
      *
      * <P>This query is a combination of the standard range query and the standard
      * k-nearest neighbour query. All objects which matches both conditions
      * will be included in the result. The K-Nearest Neighbor Query is implemented
      * with a local priority queue.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @see tResult * KAndRangeQuery
      * @warning This method does not work for trees with only one node.
      */
      void KAndRangeQuery(tResult * result, ObjectType * sample,
                          double range, u_int32_t k);

      /**
      * This method will perform range query with a limited number of results.
      *
      * <P>This query is a combination of the standard range query and the standard
      * K-Nearest Neighbor query. All objects which matches with one of two conditions
      * will be included in the result. The K-Nearest Neighbor Query is implemented
      * with a local priority queue.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param dk The maximum distance.
      * @param k The maximum number of results.
      * @see tResult * KOrRangeQuery
      * @warning This method does not work for trees with only one node.
      */
      void KOrRangeQuery(tResult * result, ObjectType * sample,
                         double range, u_int32_t k);

      /**
      * This method will perform a ring query.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param inRange The outter range of the results.
      * @param outRange The inner range of the results.
      * @return The result or NULL if this method is not implemented.
      * @param distanceRepres The distance of the representative.
      * @warning The value of outRange must be higher than inRange.
      * @see tResult * RingQuery()
      */
      void RingQuery(u_int32_t pageID, tResult * result,
                     ObjectType * sample, double inRange,
                     double outRange, double distanceRepres);

      /**
      * This method will perform a ring query with K-Nearest Neighbor based on
      * a global chained list.
      * The result will be a set of pairs object/distance.
      *
      * @param result the result set.
      * @param sample The sample object.
      * @param inRange The outter range of the results.
      * @param outRange The inner range of the results.
      * @param k The number of nearest neighbor.
      * @warning The value of outRange must be higher than inRange.
      * @see tResult * RingKQuery()
      */
      //void KRingQuery(tResult * result, ObjectType * sample, stDistance inRange,
      //                stDistance & outRange, stCount k);

      /**
      * This method will perform a ring query with K-Nearest Neighbor based on
      * a local chained list.
      * The result will be a set of pairs object/distance.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param inRange The outter range of the results.
      * @param outRange The inner range of the results.
      * @param k The number of nearest neighbor.
      * @return The result or NULL if this method is not implemented.
      * @param distanceRepres The distance of the representative.
      * @param globalQueue The global chained list.
      * @warning The value of outRange must be higher than inRange.
      * @see tResult * RingKQuery()
      */
      void LocalKRingQuery(u_int32_t pageID, tResult * result, ObjectType * sample,
                           double inRange, double & outRange, u_int32_t k,
                           double distanceRepres);

      /**
      * This method will perform a range query with a limited number of results.
      * <P>A lazy range query recovers the k elements within a radius, independently
      * if the k elements are the k nearest elements of query object.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The maximum number of results.
      * @see tResult * LazyRangeQuery
      */
      void LazyRangeQuery(u_int32_t pageID, tResult * result,
                          ObjectType * sample, double & range, u_int32_t k,
                          double distanceRepres, bool & stop);

      /**
      * This method will perform a Aproximate K Nearest Neighbor query.
      *
      * @param pageID the page to be analyzed.
      * @param result the result set.
      * @param sample The sample object.
      * @param range The range of the results.
      * @param k The number of neighbours.
      * @see tResult * NearestQuery
      */
      void AproximateNearestQuery(u_int32_t pageID, tResult * result,
                                  ObjectType * sample, double & rangeK,
                                  u_int32_t k);

      /**
      * This method will perform a range joined query.
      *
      * @param currIndexNode The node in index tree to be analyzed.
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
      * @param result The query result.
      * @param activeBuffer Active buffered of sub nodes in Joined Tree.
      * @warning The instance of tJoinedResult returned must be destroied by user.
      * @autor Implemented by Enzo Seraphim
      */
      void RangeJoinQueryRecursive(stSlimNode * currIndexNode, double radiusObjIndex,
                  stSlimNode * currNodeJoin, double radiusObjJoin,
                  stPageManager * PageManagerJoin, double distRepres,
                  const double range, tJoinedResult * result,
                  bool buffer);

      void RangeJoinRecursive(u_int32_t heightIndex, stSlimNode * currNode,
                  stSlimTree * slimTree, double range, bool buffer,
                  tJoinedResult * result);

      /**
      * Recursive algoritm for navegate Joined Tree when Index Tree went in leaf
      * node.
      *
      * @param PageManagerJoin
      * @param currNodeJoin
      * @param objIndex
      * @param distRepres
      * @param range
      * @param result
      */
      void JoinedTreeRangeJoinRecursive(stPageManager * PageManagerJoin,
                  stSlimNode * currNodeJoin, ObjectType * objIndex,
                  double distRepres, double range,
                  tJoinedResult * result);

      /**
      * For each entry in the leaf entry call the range query in the
      * joined tree.
      *
      * @param joinedTree the tree to be joined.
      * @param pageID the pageID to be analyzed.
      * @param range the range distance.
      * @param result the final result.
      */
      void DummyRangeJoinQueryRecursive(tJoinedResult * globalResult,
                           stMetricTree<ObjectType, EvaluatorType> * joinedTree,
                           u_int32_t pageID, double range);

      /**
      * Updates the distances of the objects from the new representative.
      */
      void UpdateDistances(stSlimIndexNode * node, ObjectType * repObj,
                           u_int32_t repObjIdx);

      // Visualization support
      #ifdef __stMAMVIEW__

         /**
         * This method gets a sample of random objects from the tree. It is used
         * by the visualization module to initializes the fastmapper.
         *
         * @param Size of the sample.
         * @warning This method does not exist if __stMANVIEW__ is not defined.
         */
         tObjectSample * GetSample(int sampleSize);

         /**
         * This method travels by the tree structure picking random objects stored
         * in the leaf nodes.
         *
         * @param pageID The ID of the root page.
         * @param sample The sample of objects.
         * @warning This method does not exist if __stMAMVIEW__ is not defined.
         */
         void GetSampleRecursive(u_int32_t pageID, tObjectSample * sample);

         /**
         * This method travels through the tree structure and creates a dump of the
         * tree.
         *
         * @param pageID The ID of the root page.
         * @warning This method does not exist if __stMAMVIEW__ is not defined.
         */
         void MAMViewDumpTreeRecursive(u_int32_t pageID, ObjectType * rep,
                                       double radius, u_int32_t parent);
                                       
      #endif //__stMAMVIEW__

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
            

      // Slimdown implementaion
      /**
      * This method performs the Slim-Down in the whole tree recursively. I starts from the
      * root traveling through the tree in post order. Every time it reaches an index node
      * in the second level from bottom (the index node above the leafs) it calls the method
      * SlimDown() to perform a local slim down. The radius of each subtree is updated during
      * the proccess to keep the tree consistent.
      *
      * @param pageID Root of the subtree.
      * @param level Current level (the first call must be 0).
      * @return The new radius of the subtree.
      * @warning This method will not work if the tree has less than 3 levels.
      */
      double SlimDownRecursive(u_int32_t pageID, int level);

      /**
      * This method performs the local slim down in the given subtree.
      *
      * @param pageID The subtree root.
      * @return The new radius of the subtree.
      */      
      double SlimDown(u_int32_t pageID);

      /**
      * Perform the SlimDown in a set of stSlimMemLeafNode.
      *
      * @param memLeafNodes Leaf nodes.
      * @param nodeCount Number of nodes in memLeafNodes.
      * @param maxSwaps Swap limit.
      */
      void LocalSlimDown(tMemLeafNode ** memLeafNodes, int nodeCount, int maxSwaps);

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
      bool SlimDownCanSwap(tMemLeafNode * src, tMemLeafNode * dst,
                           double & distance);

      #ifdef __BULKLOAD__
         /**
         * This methods are used by BulkLoad
         */
         bool BulkInsert(stSubtreeInfo & sub1, stSubtreeInfo & sub, double indexNodeOccupancy, enum tBulkMethod method);
         int  BulkLoadSimple (ObjectType **objects, u_int32_t numObj, double leafNodeOccupancy, stPage *& auxPage, int currObj);
         int  BulkLoadSampled(ObjectType **objects, u_int32_t numObj, double leafNodeOccupancy, stPage *& auxPage, int currObj);

         bool BulkLoadMemory(vector< SampleSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, enum tBulkType type);
         bool BulkLoadMemory(vector< SampleSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, bool insertLeaf, enum tBulkType type);
         bool BulkLoadMemory(vector< SampleSon<ObjectType> > objects, int father, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, bool insertLeaf, enum tBulkType type);

         /**
         * Utilities
         */
        u_int32_t  getNodeFreeSize();
        u_int32_t getNumIndexNodeObj(u_int32_t objSize);
        u_int32_t getNumLeafNodeObj(u_int32_t objSize);
        u_int32_t  setBulkHeight(u_int32_t numObj,u_int32_t objSize,double nodeOccupancy);
        u_int32_t  getBulkHeight(u_int32_t numObj,u_int32_t objSize,double nodeOccupancy);


      #endif //__BULKLOAD__

      /**
      * This method recursively checks for inconsistencies in the Slim-tree.
      * Basically, this methods finds radius, distances, and
      * other information that may be set wrong in the algorithm.
      * This method is very useful in the developement processes to track
      * implementation errors that a developer may introduce in his/her 
      * implementation.
      *
      * @see Consistency()
      */
      bool Consistency(u_int32_t pageID, ObjectType * repObj, 
                       double & radius, u_int32_t & height, 
                       u_int32_t & objectCount);

};//end stSlimTree


// Include implementation
#include "stSlimTree-inl.h"

#endif //__STSLIMTREE_H
