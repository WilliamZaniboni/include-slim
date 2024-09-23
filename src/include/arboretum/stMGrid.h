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
#ifndef __STMGRID_H
#define __STMGRID_H

#include <arboretum/stUtil.h>

#include <arboretum/stMetricTree.h>
//#include <arboretum/stDFNode.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stGenericPriorityQueue.h>
// #include <arboretum/deprecated/stBasicMetricEvaluators.h>
// #include <arboretum/deprecated/stBasicObjects.h>

// this is used to set the initial size of the dynamic queue
#define STARTVALUEQUEUE 200
// this is used to set the increment size of the dynamic queue
#define INCREMENTVALUEQUEUE 200
// this is used to set GR Vector
//#define STFOCUS 3

#include <string.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <time.h>

template <class ObjectType, class EvaluatorType>
class stMGridPivot;

//=============================================================================
// Class template stMGridOrigNodecNode
//-----------------------------------------------------------------------------
/**
* Thic class template represents a node which original objects. It is used to
* acess the objects from the pages of the pagemaner in use.
*
* @version 1.0
* $Revision: 1.20 $
* $Date: 2009-02-27 23:04:22 $
* @author Adriano Arantes Paterlini (paterlini@gmail.com)
* @ingroup MGrid
*/
template <class ObjectType, class EvaluatorType>
class stMGridOrigNode{

   public:

      /**
      * Creates a new instance of this node with no objects.
      *
      * @param pageman The maximum number of entries.
      */
      stMGridOrigNode(stPage * page, bool create = false);

      /**
      * Disposes this instance and releases all related resources. All instances of
      * object added to this node will also be deleted unless it is not owned by
      * this node
      */
      ~stMGridOrigNode(){
      }
      /**
      * Returns the associated page.
      *
      * @return The associated page.
      */
      stPage * GetPage(){
         return Page;
      }//end GetPage

      /**
      * Returns the ID of the associated page.
      *
      * @return The ID of the associated page.
      */
      u_int32_t GetPageID(){
         return Page->GetPageID();
      }//end GetPage

      /**
      * Returns the number of entries in this node.
      *
      * @return the number of entries.
      * @see GetEntry()
      * @see GetObject()
      * @see GetObjectSize()
      */
      u_int32_t GetNumberOfEntries(){
         return this->Header->Occupation;
      }//end GetNumberOfEntries

      /**
      * This method adds an object to this node.
      *
      * @param size The size of the object in bytes.
      * @param object The object data.
      * @return The position in the vector Entries or a negative value for failure.
      * @see RemoveObject()
      */
      int AddEntry(u_int32_t size, const unsigned char * object);

      /**
      * Gets the serialized object. Use GetObjectSize to determine the size of
      * the object.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return A pointer to the serialized object.
      * @see GetObjectSize()
      */
      const unsigned char * GetObject(int idx);

      /**
      * Returns the size of the object in bytes. Use GetObject() to get the
      * object data.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @return The size of the serialized object.
      * @see GetObject()
      */
      u_int32_t GetObjectSize(int idx);

      /**
      * Removes an entry from this object.
      *
      * @param id The id of the entry.
      * @warning The parameter id is not verified by this implementation
      * unless __stDEBUG__ is defined at compile time.
      * @see GetObjectSize()
      * @todo This method is not implemented yet.
      */
      void RemoveEntry(u_int32_t idx);

      /**
      * Gets the next node linked to this node.
      */
      u_int32_t GetNextNode(){

         return this->Header->Next;
      }//end GetNextNode

      /**
      * Sets the next node linked to this node.
      *
      * @param id The ID of the next node.
      */
      void SetNextNode(u_int32_t idx){
         this->Header->Next = idx;
      }//end SetNextNode
     
   
   private:

      /**
      * Header of the stDummyNode.
      */
      typedef struct NodeHeader{
         /**
         * Occupation of this node.
         */
         u_int32_t Occupation;
         
         /**
         * The ID of the next page.
         */
         u_int32_t Next;
         
      }stNodeHeader;//end

      /**
      * The associated page.
      */
      stPage * Page;

      /**
      * Header of this node.
      */
      stNodeHeader * Header;

      /**
      * Entries.
      */
      u_int32_t * Entries;

      /**
      * Returns the free space available in this node.
      */
      u_int32_t GetFree();
      
      
};//end stMGridOrigNode


//=============================================================================
// Class template stMGridLogicNode
//-----------------------------------------------------------------------------
/**
* Thic class template represents a MGrid logic node entry. It is used to
* hold the objects in a memory form which allows better way to
* manipulate entries. 
*
* @version 1.0
* $Revision: 1.20 $
* $Date: 2009-02-27 23:04:22 $
* @author Adriano Arantes Paterlini (paterlini@gmail.com)
* @ingroup MGrid
* @todo replace static allocation of Entries to incremental allocator
* @see stDFLogicNode
*/
template <class ObjectType, class EvaluatorType>
class stMGridLogicNode{

   public:

      /**
      * This type implements the MgP.
      */
      //typedef stMGridPivot <ObjectType, EvaluatorType> tMgP;
      
      /**
      * This is the class that abstracts the object used by this metric tree.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the logic node for this class.
      */
      typedef stMGridOrigNode < ObjectType, EvaluatorType > tOrigNode;

      /**
      * Creates a new instance of this node with no objects.
      *
      * @param 
      */
      stMGridLogicNode(stPageManager * pageman);

      /**
      * Disposes this instance and releases all related resources. All instances of
      * object added to this node will also be deleted unless it is not owned by
      * this node (see method BuyObject()).
      */
      ~stMGridLogicNode();
      
      /**
      * The page manager used 
      */
      stPageManager * myPageManager;

      /**
      * Adds an object to this node. This method will claim the ownership
      * of the object instance.
      *
      * <P>Use SetEntry() to fill the other fields of each entry.
      *
      * @param obj The object to be added.
      * @return The entry id or -1 for error.
      */
      int AddEntry(tObject * obj);
      
      /**
      * Returns the number of objetcs of this sequencial scan.
      */
      u_int32_t GetNumberOfObjects(){
         return Header->ObjectCount;
      }//end GetNumberOfObjects
      
      u_int32_t GetNumberOfNodes(){
         return Header->NodeCount;
      }//end GetNumberOfObjects
      
      /**
      * Returns the root pageID.
      */
      u_int32_t GetRoot(){
         return Header->Root;
      }//end GetRoot

      /**
      * Returns the object of a given entry.
      *
      * @param idx The object index.
      */
      ObjectType * GetObject(u_int32_t index);

      /**
      * Returns if a given entry is already a Pivot.
      *
      * @param idx The object index.
      */
      bool GetIsPivot(u_int32_t idx){
         return Entries[idx].IsPivot;
      }//end GetObject

      /**
      * Set as a Pivot.
      *
      * @param idx The object index.
      */
      bool SetIsPivot(u_int32_t idx){
         Entries[idx].IsPivot = true;
      }//end GetObject

      /**
      * Set the Cluster.
      *
      * @param idx The object index.
      * @param cluster The cluster object belong
      */
      void SetCluster(u_int32_t idx, u_int32_t cluster){
         Entries[idx].Cluster = cluster;
      }//end GetObject
      

      int GetCluster(u_int32_t idx){
         return Entries[idx].Cluster;
      }

      /**
      * Set the Cell.
      *
      * @param idx The object index.
      * @param cluster The cluster object belong
      */
      void SetCell(u_int32_t idx, u_int32_t cell){
         Entries[idx].Cell = cell;
      }//end GetObject
      

      int GetCell(u_int32_t idx){
         return Entries[idx].Cell;
      }

      /**
      * Returns the object of a given entry.
      *
      * @param idx The object index.
      */
      //ObjectType * operator [](u_int32_t idx){
      //   return Entries[idx].Object;
      //}//end operator []          

   private:

      /**
      * This type defines the Entry of the Logic Node.
      */
      typedef struct MGridIndexEntry{
         /**
         * PageId where object is
         */
         u_int32_t ObjPageID;

         /**
         * Object Index inside ObjPage.
         */
         u_int32_t ObjIdx;
         
         /**
         * Object Index inside ObjPage.
         */
         bool IsPivot;
         
         /**
         * The Cluster which this point belong
         */
         u_int32_t Cluster;

         /**
         * The Cell which this point belong
         */
         u_int32_t Cell;
         
      } stIndexEntry;
      
      /**
      * Entries.
      */
      stIndexEntry * Entries;
      
      /**
      * Maximum number of entries.
      */
      u_int32_t MaxEntries;

      /**
      * Current number of entries.
      */
      u_int32_t Count;
      
      /**
      * Page in use 
      */
      stPage * currPage;

      /**
      * Node in use
      */      
      tOrigNode * currNode; 
      
     /**
      * This type defines the header of the Dummy Tree.
      */
      typedef struct MGridHeader{
         /**
         * The root.
         */
         u_int32_t Root;

         /**
         * The number of the objects in this page.
         */
         u_int32_t ObjectCount;

         /**
         * Total number of nodes.
         */
         u_int32_t NodeCount;

         /**
         * Maximum number of objects in a node.
         */
         u_int32_t MaxOccupation;
      } stHeader;

      /**
      * The header page. It will be kept in memory all the time to avoid
      * reads.
      */
      stPage * HeaderPage;

      /**
      * The header of the "tree".
      */
      stHeader * Header;
     
      /**
      * If true, the header must be written to the page manager.
      */
      bool HeaderUpdate;

      /**
      * Creates the header for an empty tree.
      */
      void Create(){

         LoadHeader();
         Header->Root = 0;
         Header->ObjectCount = 0;
         Header->NodeCount = 0;
         Header->MaxOccupation = 0;
         HeaderUpdate = true;
         WriteHeader();
      }//end Create

      /**
      * Loads the header from the page manager.
      */
      void LoadHeader(){
         HeaderPage = this->myPageManager->GetHeaderPage();
         Header = (stHeader *) HeaderPage->GetData();
      }//end LoadHeader

      /**
      * Writes the header into the Page Manager.
      */
      void WriteHeader(){
         if (HeaderUpdate){
            this->myPageManager->WriteHeaderPage(HeaderPage);
            HeaderUpdate = false;
         }//end if
      }//end WriteHeader

      /**
      * Set the new root pageID.
      */
      void SetRoot(u_int32_t newRootPageID){
         Header->Root = newRootPageID;
      }//end SetRoot

      /**
      * Updates the object counter.
      */
      void UpdateObjectCounter(int inc){
         Header->ObjectCount += inc;
         HeaderUpdate = true;
      }//end UpdateObjectCounter

      /**
      * Updates all distances between representatives and all objects in this
      * node. It returns the number of distances calculated.
      *
      * @param metricEvaluator The metric evaluator to be used.
      */
      //int UpdateDistances(EvaluatorType * metricEvaluator);
      
      
      
};//end stMGridLogicNode


//=============================================================================
// Class template stMGridPivot
//-----------------------------------------------------------------------------
/**
* This class defines all behavior of the GR.
* $Date: 2009-02-27 23:04:22 $
* @author Adriano Arantes Paterlini (paterlini@gmail.com) 
* @version 1.0
* @ingroup Mgrid
* @todo replace static allocation of focibase using STFOCUS
* @see stDFGlobalRep
*/
template <class ObjectType, class EvaluatorType>
class stMGridPivot {

   public:

      /**
      * This is the class that abstracts the object used by this metric tree.
      */
      typedef ObjectType tObject;

      /**
      * This type defines the logic node for this class.
      */
      typedef stMGridLogicNode < ObjectType, EvaluatorType > tLogicNode;

      /**
      * This type defines the MST splitter for this class.
      */
      //typedef stDFMSTSplitter < ObjectType, EvaluatorType > tMSTSplitter;

      /**
      * Number of focus.
      */
      u_int32_t NumFocus;
      
      /**
      * Number of focus for each Dimension.
      */
      u_int32_t NumDim;

      /**
      * Number of focus for each Dimension.
      */
      u_int32_t NumFocusDim;

      /**
      * Number of ring.
      */
	   u_int32_t NumRings;
      
      

      /**
      * This type represents a GlobalRep.
      */
      struct stMGridPivotEntry{
         
         ObjectType * Object;
         /**
         * Indice do objeto.
         */
         int idx;

         int nrings;
	      
	      /**
	      * Distance to each ring
	      */
         double * RingDistance;
      };//end sstMGridPivotEntry

      double GetRingDistance(u_int32_t foci, u_int32_t ring){
         return fociBase[foci].RingDistance[ring];
      }

      int GetNumRings(u_int32_t foci){
         return fociBase[foci].nrings;
      }

      /**
      * Find Pivots.
      *
      * @param node The node used to find the best focus
      * @param numObject Total number of objects
      * @warning The distance matrix need already have been calculated
      */
      void FindPivot(tLogicNode * logicNode, u_int32_t numObject, int type);

      /**
      * Builds a new instance of this class.
      * @param nFocus Number of focus
      * @param metricEvaluator The metric evaluator to compute distances.
      */
      stMGridPivot(u_int32_t nFocus, u_int32_t nRings, EvaluatorType * metricEvaluator);

      /**
      * Calculate the GR Distances of a given object.
      *
      * @param Object The object.
      * @param FieldDistance The distances to GR from object
      */
      void BuildFieldDistance(ObjectType * object, double * fieldDistance);
      
      /**
      * Find Rings.
      *
      * @param node The node used to find the best focus
      * @param numObject Total number of objects
      * @warning The distance matrix need already have been calculated
      */
      void FindRings(tLogicNode * logicNode, u_int32_t numObject, int Type);
      
      void sort(double *focus_dist, long l, long numObject);
   private:

      /**
      * Keep the Pivots objects
      * @warning static declarition
      */
      stMGridPivotEntry * fociBase;
 
      /**
      * the metric Evaluator of this grid
      */
      EvaluatorType * myMetricEvaluator;

      /**
      * Distance matrix type.
      */
      typedef stGenericMatrix <double> tDistanceMatrix;
      
      /**
      * execute the HF algorithm to find the foci base based on the FastMap Algorithm
      *
      * @param logicnode The node used to find the best focus
      * @param numObject Total number of objects
      */
      void execHFFastMap(tLogicNode * logicNode, u_int32_t numObject);

      /**
      * execute the HF algorithm to find the remaining foci base based on the FastMap Algorithm
      *
      * @param logicnode The node used to find the best focus
      * @param numObject Total number of objects
      */
      void execHFRemainingFastMap(tLogicNode * logicNode, u_int32_t numObject, double edge, int nFound);   
      
      /**
      * execute randomly find the foci base
      *
      * @param logicnode The node used to find the best focus
      * @param numObject Total number of objects
      */
      void execRandomPivots(tLogicNode * logicNode, u_int32_t numObject);

};//end stMGridPivot


//enum OrderType {cluster, cell, clustercell, cellcluster}

//=============================================================================
// Class template stMGrid
//-----------------------------------------------------------------------------
/**
* This class defines all behavior of the MGrid.
* $Date: 2009-02-27 23:04:22 $
* @author Adriano Arantes Paterlini (paterlini@gmail.com) 
* @version 1.0
* @ingroup Mgrid
*/
template <class ObjectType, class EvaluatorType>
class stMGrid {

   public:
      
      /**
      * This is the class that abstracts the object used by this metric tree.
      */
      typedef ObjectType tObject;
      
      /**
      * This is the class that abstracs an result set for simple queries.
      */
      typedef stResult <ObjectType> tResult;

      /**
      * This type defines the logic node for this class.
      */
      //typedef stMGridOrigNode < ObjectType, EvaluatorType > tOrigNode;
   
      typedef stMGridLogicNode < ObjectType, EvaluatorType > tLogicNode;

      /**
       * this types defines the distance node, using basic elements as object type and evaluator type.
       */
      typedef stBasicArrayObject <double> tBasicArrayObject;
      typedef stBasicLInfinityMetricEvaluator <tBasicArrayObject> tBasicMetricEvaluator;
      //typedef stBasicManhatanMetricEvaluator <tBasicArrayObject> tBasicMetricEvaluator;
      //typedef stBasicEuclideanMetricEvaluator <tBasicArrayObject> tBasicMetricEvaluator;
      typedef stMGridLogicNode <tBasicArrayObject, tBasicMetricEvaluator> tDistanceNode;

      /**
      * This type defines the MGridPivot  for this class.
      */
      typedef stMGridPivot < ObjectType, EvaluatorType > tMGridPivot;

      /**
      * Number of focus.
      */
      u_int32_t NumFocus;

      /**
      * Number of rings.
      */
      u_int32_t NumRings;
      
      /**
      * Number of cells.
      */
      u_int32_t NumCells;
      
      /**
      * Number of clusters.
      */
      u_int32_t NumClusters;
      
      /**
      * The page manager used for original objects 
      */
      stPageManager * myPageManager;

      /**
      * The page manager used for original objects, ordered by the clusters 
      */
      stPageManager * myPageManagerO;
      
      /**
      * The page manager used for objects mapped by distances
      */
      stPageManager * myPageManagerD;
      
      EvaluatorType * myMetricEvaluator;       

      tBasicMetricEvaluator * myBasicMetricEvaluator;

      /**
      * my GridPivot
      */
      tMGridPivot *MGridPivot;
      
      /*
      * Logic node to storage Original Objects
      */
      tLogicNode  *LogicNode;

      /*
      * Logic node to storage Original Objects
      */
      tLogicNode  *OrderNode;
      
      /*
      * Logic node to storage, objects distances from pivots
      */
      tDistanceNode *DistanceNode;
  
      /**
      * This method adds an object to the metric tree. This method may fail it the object size
      * exceeds the page size - 16.
      *
      * @param obj The object to be added.
      * @return True for success or false otherwise.
      */
      bool Add(tObject * obj);

      /**
      * Returns the number of objetcs of this sequencial scan.
      */
      long GetNumberOfObjects(){
         return LogicNode->GetNumberOfObjects();
      }//end GetNumberOfObjects

      long GetNumberOfDistances(){
         return DistanceNode->GetNumberOfObjects();
      }//end GetNumberOfObjects
      
      long GetNumberOfNodes(){
         return LogicNode->GetNumberOfNodes();
      }//end GetNumberOfObjects
      
      
      /**
      * Builds a new instance of this class.
      * @param nFocus Number of focus
      * @param metricEvaluator The metric evaluator to compute distances.
      */
      stMGrid(stPageManager * pageman, stPageManager * pagemanD, stPageManager * pagemanO, 
         u_int32_t nFocus, u_int32_t nRings, u_int32_t nClusters, EvaluatorType * metricEvaluator);

      /**
      * Returns the object of a given entry.
      *
      * @param idx The object index.
      */
      ObjectType * GetObject(u_int32_t index);

      /**
      * Returns the object of a given distance.
      *
      * @param idx The object index.
      */
      double * GetDistance(u_int32_t index);
      
      /**
      * Builds the distance from the original objects to pivots.
      *
      */
      void BuildAllDistance();

      void AllDistance();
      
      /**
      * Find Cluster
      */
      void Cluster();

      /**
      * Builds CC-Array
      */
      void BuildCCArray();   
      
      /**
      * create the order node
      * @param type  0: cluster
      *              1: cell
      *              2: cluster + cell
      */
      void Order(int type);
   
      /**
      * This method will perform a range query. The result will be a set of
      * pairs object/distance.
      *
      * @param sample The sample object.
      * @param range The range of the results.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * RangeQuery(tObject * sample, double range);
      tResult * RangeQueryCell(tObject * sample, double range);
      tResult * RangeQueryCluster(tObject * sample, double range);
      tResult * RangeQueryClCell(tObject * sample, double range);
      tResult * RangeQuerySeq(tObject * sample, double range);
      tResult * RangeQueryOmniSeq(tObject * sample, double range);
      tResult * RangeQueryCellCenter(tObject * sample, double range, int cost = 1);

      /**
      * This method will perform a k nearest neighbor query.
      *
      * @param sample The sample object.
      * @param k The number of neighbours.
      * @param tie The tie list. Default false.
      * @return The result or NULL if this method is not implemented.
      * @warning The instance of tResult returned must be destroied by user.
      */
      tResult * NearestQuery(tObject * sample, u_int32_t k, bool tie = false);
      tResult * NearestQueryCell(tObject * sample, u_int32_t k, bool tie = false);   
      tResult * NearestQueryCellCenter(tObject * sample, u_int32_t k, bool tie = false);
      tResult * NearestQueryOmniSeq(tObject * sample, u_int32_t k, bool tie = false);
      tResult * NearestQueryCellRank(tObject * sample, u_int32_t k, bool tie = false, int cost = 1);
      tResult * NearestQueryOmniSeqOt(tObject * sample, u_int32_t k, double irange, bool tie = false);
   private:
    
      /**
      * This type represents a CellArray.
      */
      struct stMGridCellArray{
         /**
         * Mean of the cluster.
         */
         tBasicArrayObject * CellCenter;
         /**
         * Number of cluster.
         */
         int cluster;
         /**
         * Number of objects.
         */
         int NumObjects;
         /**
         *  
         * 0 - not to visit
         * 1 - must visit
         * 2 - already visit
         */
         int status;

	 /*
	 * distance to the farthest object of this cell
         */	      
         double distance;

	 /*
	 * distance to the query center
         */	      
         double qDist;
         int first;
         int last;

      };//end stMGridCellArray

      /**
      * Keep the Pivots objects
      */
      stMGridCellArray *CellArray;     
      
      /**
      * This type represents clusters.
      */
      struct stMGridCluster{
         /**
         * Mean of the cluster.
         */
         tBasicArrayObject * ClusterMean;
	      /**
         * Number of object in this cluster
         */
         u_int32_t Count;         
         /**
         * Sum of objects.
         * @warning this should be a double * or a object?, as a object, think about a sum operator
         */
	      double * Sum;
         
         /*
         * keep the distance to the farthest object
         */
         double maxDist;
         /**
         *  
         * 0 - not to visit
         * 1 - must visit
         * 2 - already visit
         */
         int status;      

      };// end stMGridCluster
      
      /**
      * Keep the cluters means
      */
      stMGridCluster *Clusters;

      /**
      * keep the contribution for each pivots, this is used during cell calculation      * 
      */
      int *contribution;
      
      int *cclist;

      int countcc;

      double *minD;
      double *maxD;

      /* This function takes the cell number as an integer and returns the cell 
      *  number by the rings it lies in for each pivot 
      */
      void GetCell(int idx, int *in);

      void AllocCell(int c);   
   
      void FindCells(int  px, int idxc);

      void sortcell(int *clist, long l, long numObject);
      
         

};

// Include implementation
#include "stMGrid-inl.h"

#endif //__STMGRID_H
