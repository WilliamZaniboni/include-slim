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
#ifndef __stOmni_H
#define __stOmni_H

#include <arboretum/stUtil.h>

#include <arboretum/stMetricTree.h>
#include <arboretum/stSlimTree.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stGenericPriorityQueue.h>
#include <arboretum/stMetricEvaluators.h>
#include <arboretum/stBasicObjects.h>

#include <string.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <time.h>
#include <cstring>

//#define FASTMAPER 1

template <class ObjectType, class EvaluatorType>
class stOmniPivot;

//=============================================================================
// Class template stOmniOrigNodecNode
//-----------------------------------------------------------------------------

/**
 * Thic class template represents a node which original objects. It is used to
 * acess the objects from the pages of the pagemaner in use.
 *
 * @version 1.0
 * $Revision: 1.3 $
 * $Date: 2009-02-27 23:08:44 $
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @ingroup Omni
 */
template <class ObjectType, class EvaluatorType>
class stOmniOrigNode {
public:

    /**
     * Creates a new instance of this node with no objects.
     *
     * @param pageman The maximum number of entries.
     */
    stOmniOrigNode(stPage * page, bool create = false);

    /**
     * Disposes this instance and releases all related resources. All instances of
     * object added to this node will also be deleted unless it is not owned by
     * this node
     */
    ~stOmniOrigNode() {
    }

    /**
     * Returns the associated page.
     *
     * @return The associated page.
     */
    stPage * GetPage() {
        return Page;
    }//end GetPage

    /**
     * Returns the ID of the associated page.
     *
     * @return The ID of the associated page.
     */
    u_int32_t GetPageID() {
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
    u_int32_t GetNumberOfEntries() {
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
    int AddEntry(u_int32_t size, const unsigned char * object, double maxD = 0);

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
    u_int32_t GetNextNode() {

        return this->Header->Next;
    }//end GetNextNode

    /**
     * Gets the next node linked to this node.
     */
    double GetMax() {

        return this->Header->Max;
    }//end GetNextNode

    /**
     * Sets the next node linked to this node.
     *
     * @param id The ID of the next node.
     */
    void SetNextNode(u_int32_t idx) {
        this->Header->Next = idx;
    }//end SetNextNode


private:

    /**
     * Header of the stDummyNode.
     */
    typedef struct NodeHeader {
        /**
         * Occupation of this node.
         */
        u_int32_t Occupation;

        /**
         * The ID of the next page.
         */
        u_int32_t Next;

        /**
         * Distance between the first element and all others in this node
         */
        double Max;

    } stNodeHeader; //end

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


}; //end stOmniOrigNode


//=============================================================================
// Class template stOmniLogicNode
//-----------------------------------------------------------------------------

/**
 * Thic class template represents a Omni logic node entry. It is used to
 * hold the objects in a memory form which allows better way to
 * manipulate entries.
 *
 * @version 1.0
 * $Revision: 1.3 $
 * $Date: 2009-02-27 23:08:44 $
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @ingroup Omni
 * @todo replace static allocation of Entries to incremental allocator
 * @see stDFLogicNode
 */
template <class ObjectType, class EvaluatorType>
class stOmniLogicNode {
public:

    /**
     * This is the class that abstracts the object used by this metric tree.
     */
    typedef ObjectType tObject;

    /**
     * This type defines the logic node for this class.
     */
    typedef stOmniOrigNode < ObjectType, EvaluatorType > tOrigNode;

    /**
     * Creates a new instance of this node with no objects.
     *
     * @param
     */
    stOmniLogicNode(stPageManager * pageman, EvaluatorType * metricEvaluator);

    /**
     * Disposes this instance and releases all related resources. All instances of
     * object added to this node will also be deleted unless it is not owned by
     * this node (see method BuyObject()).
     */
    ~stOmniLogicNode();

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
    u_int32_t GetNumberOfObjects() {
        return Header->ObjectCount;
    }//end GetNumberOfObjects

    u_int32_t GetNumberOfNodes() {
        return Header->NodeCount;
    }//end GetNumberOfObjects

    /**
     * Returns the root pageID.
     */
    u_int32_t GetRoot() {
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
    bool GetIsPivot(u_int32_t idx) {
        return Entries[idx].IsPivot;
    }//end GetObject

    /**
     * Set as a Pivot.
     *
     * @param idx The object index.
     */
    bool SetIsPivot(u_int32_t idx) {
        Entries[idx].IsPivot = true;
    }//end GetObject

      /**
      * Set the Cluster.
      *
      * @param idx The object index.
      * @param cluster The cluster object belong
      */
      void SetCluster(u_int32_t idx, u_int32_t cluster){
         Infos[idx].Cluster = cluster;
      }//end GetObject
      

      int GetCluster(u_int32_t idx){
         return Infos[idx].Cluster;
      }

      void AddInfo(){
          stInfoEntry info;
          Infos.insert(Infos.end(), info);          
      }
            /**
      * Set the Cluster.
      *
      * @param idx The object index.
      * @param cluster The cluster object belong
      */
      void ResetCost(u_int32_t idx){
         Infos[idx].Cost = 0;
      }//end GetObject

      void AddCost(u_int32_t idx, double cost){
         Infos[idx].Cost += cost;
      }//end GetObject


      double GetCost(u_int32_t idx){
         return Infos[idx].Cost;
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
    typedef struct OmniIndexEntry {
        /**
         * PageId where object is
         */
        u_int32_t ObjPageID;

        /**
         * Object Index inside ObjPage.
         */
        u_int32_t ObjIdx;

        bool IsPivot;

    } stIndexEntry;


    typedef struct OmniInfoEntry {

        int Cluster;
        
        double Cost;

    } stInfoEntry;

    /**
     * Entries.
     */
    vector <stIndexEntry> Entries;

    vector <stInfoEntry> Infos;

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
     * the metric Evaluator of this grid
     */
    EvaluatorType * myMetricEvaluator;

    /**
     * This type defines the header of the Dummy Tree.
     * @todo add more informations!!!
     */
    typedef struct OmniHeader {
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
    void Create() {

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
    void LoadHeader() {
        HeaderPage = this->myPageManager->GetHeaderPage();
        Header = (stHeader *) HeaderPage->GetData();
    }//end LoadHeader

    /**
     * Writes the header into the Page Manager.
     */
    void WriteHeader() {
        if (HeaderUpdate) {
            this->myPageManager->WriteHeaderPage(HeaderPage);
            HeaderUpdate = false;
        }//end if
    }//end WriteHeader

    /**
     * Set the new root pageID.
     */
    void SetRoot(u_int32_t newRootPageID) {
        Header->Root = newRootPageID;
    }//end SetRoot

    /**
     * Updates the object counter.
     */
    void UpdateObjectCounter(int inc) {
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



}; //end stOmniLogicNode


//=============================================================================
// Class template stOmniPivot
//-----------------------------------------------------------------------------

/**
 * This class defines all behavior of the GR.
 * $Date: 2009-02-27 23:08:44 $
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @version 1.0
 * @ingroup Omni
 * @see stDFGlobalRep
 */
template <class ObjectType, class EvaluatorType>
class stOmniPivot {
public:

    /**
     * This is the class that abstracts the object used by this metric tree.
     */
    typedef ObjectType tObject;

    /**
     * This type defines the logic node for this class.
     */
    typedef stOmniLogicNode < ObjectType, EvaluatorType > tLogicNode;

    /**
     * Number of focus.
     */
    u_int32_t NumFocus;

    /**
     * This type represents a GlobalRep.
     */
    struct stOmniPivotEntry {
        ObjectType * Object;

        /**
         * Indice do objeto.
         */
        int idx;

        /**
         * coordenadas do pivot, usado pelo fastmap
         */
        double * coords;

    }; //end stOmniPivotEntry

    /**
     * Find Pivots.
     *
     * @param node The node used to find the best focus
     * @param numObject Total number of objects
     * @param type find type
     * @param part only partial imersion
     * @warning The distance matrix need already have been calculated
     */
    void FindPivot(tLogicNode * logicNode, u_int32_t numObject, int type = 0, int part = 0);

    /**
     * Builds a new instance of this class.
     * @param nFocus Number of focus
     * @param metricEvaluator The metric evaluator to compute distances.
     */
    stOmniPivot(u_int32_t nFocus, EvaluatorType * metricEvaluator);

    /**
     * Calculate the GR Distances of a given object.
     *
     * @param Object The object.
     * @param FieldDistance The distances to GR from object
     */
    double BuildFieldDistance(ObjectType * object, double * fieldDistance);

    void BuildFieldDistancePartial(ObjectType * object, double * fieldDistance, int part);

private:

    /**
     * Keep the Pivots objects
     * @warning static declarition
     */
    stOmniPivotEntry * fociBase;

    /**
     * the metric Evaluator of this grid
     */
    EvaluatorType * myMetricEvaluator;


    /**
     * A cache of pivot distances raised by the power of 2.
     */
    double * PivotDist;

    /**
     * A cache of pivot distances raised by the power of 2.
     */
    double * PivotDist2;

    /**
     * execute the HF algorithm to find the foci base based on the FastMap Algorithm
     *
     * @param logicnode The node used to find the best focus
     * @param numObject Total number of objects
     */
    void execHFFastMap(tLogicNode * logicNode, u_int32_t numObject);
    void execHFFastMapPartial(tLogicNode * logicNode, u_int32_t numObject, int part);
    void execHFFastMapCluster(tLogicNode * logicNode, u_int32_t numObject, int part);
    /**
     * execute the HF algorithm to find the remaining foci base based on the FastMap Algorithm
     *
     * @param logicnode The node used to find the best focus
     * @param numObject Total number of objects
     */
    void execHFRemainingFastMap(tLogicNode * logicNode, u_int32_t numObject, double edge, int nFound);
    void execHFRemainingFastMapPartial(tLogicNode * logicNode, u_int32_t numObject, double edge, int nFound, int part);
    void execHFRemainingFastMapCluster(tLogicNode * logicNode, u_int32_t numObject, double edge, int nFound, int part);
    /**
     * execute randomly find the foci base
     *
     * @param logicnode The node used to find the best focus
     * @param numObject Total number of objects
     */
    void execRandomPivots(tLogicNode * logicNode, u_int32_t numObject);
    void execRandomCluster(tLogicNode * logicNode, u_int32_t numObject, int part);



    double FMProject(ObjectType * obj, double * map, int axis);

    double GetFMDistance2(ObjectType * o1, double * map1, ObjectType * o2, double * map2, int axis);

    void UpdatePivotMaps();


}; //end stOmniPivot


//enum OrderType {cluster, cell, clustercell, cellcluster}

//=============================================================================
// Class template stOmni
//-----------------------------------------------------------------------------

/**
 * This class defines all behavior of the Omni.
 * $Date: 2009-02-27 23:08:44 $
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @version 1.0
 * @ingroup Omni
 */
template <class ObjectType, class EvaluatorType>
class stOmni {
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
    typedef stOmniOrigNode < ObjectType, EvaluatorType > tOrigNode;

    typedef stOmniLogicNode < ObjectType, EvaluatorType > tLogicNode;


    /**
     * this types defines the distance node, using basic elements as object type and evaluator type.
     */
    typedef stBasicArrayObject <double> tBasicArrayObject;

#ifdef FASTMAPER
    typedef stEuclideanMetricEvaluator <double> tBasicMetricEvaluator;
#else
    typedef stLInfinityMetricEvaluator <double> tBasicMetricEvaluator;
#endif

    //typedef stSlimTree < tBasicArrayObject, tBasicMetricEvaluator > mySlimTree;

    typedef stOmniLogicNode <tBasicArrayObject, tBasicMetricEvaluator> tDistanceNode;

    /**
     * This type defines the OmniPivot  for this class.
     */
    typedef stOmniPivot < ObjectType, EvaluatorType > tOmniPivot;

    /**
     * Number of focus.
     */
    u_int32_t NumFocus;

    /**
     * The page manager used for original objects
     */
    stPageManager * myPageManager;

    /**
     * The page manager used for objects mapped by distances
     */
    stPageManager * myPageManagerD;

    EvaluatorType * myMetricEvaluator;

    tBasicMetricEvaluator * myBasicMetricEvaluator;

    /**
     * my GridPivot
     */
    tOmniPivot * OmniPivot;

    /*
     * Logic node to storage Original Objects
     */
    tLogicNode *LogicNode;

    /*
     * Logic node to storage, objects distances from pivots
     */
    tDistanceNode *DistanceNode;

    //mySlimTree * SlimTree;

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
    long GetNumberOfObjects() {
        return LogicNode->GetNumberOfObjects();
    }//end GetNumberOfObjects

    long GetNumberOfDistances() {
        return DistanceNode->GetNumberOfObjects();
    }//end GetNumberOfObjects

    long GetNumberOfNodes() {
        return LogicNode->GetNumberOfNodes();
    }//end GetNumberOfObjects


    /**
     * Builds a new instance of this class.
     * @param nFocus Number of focus
     * @param metricEvaluator The metric evaluator to compute distances.
     */
    stOmni(stPageManager * pageman, stPageManager * pagemanD, u_int32_t nFocus, EvaluatorType * metricEvaluator);

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
     * This method will perform a range query. The result will be a set of
     * pairs object/distance.
     *
     * @param sample The sample object.
     * @param range The range of the results.
     * @return The result or NULL if this method is not implemented.
     * @warning The instance of tResult returned must be destroied by user.
     */
    tResult * RangeQuery(tObject * sample, double range);
    tResult * RangeQueryOmniSeq(tObject * sample, double range);
    tResult * RangeQueryOmniSeqB(tObject * sample, double range);

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
    tResult * NearestQueryOmniSeq(tObject * sample, u_int32_t k, bool tie = false);

private:


};


//=============================================================================
// Class template stOmni
//-----------------------------------------------------------------------------

/**
 * This class defines all behavior of the Omni.
 * $Date: 2009-02-27 23:08:44 $
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @version 1.0
 * @ingroup Omni
 */
template <class ObjectType, class EvaluatorType>
class stMOmni {
public:

    /**
     * This is the class that abstracts the object used by this metric tree.
     */
    typedef ObjectType tObject;

    /**
     * This is the class that abstracs an result set for simple queries.
     */
    typedef stResult <ObjectType> tResult;


    typedef stOmniOrigNode < ObjectType, EvaluatorType > tOrigNode;

    /**
     * This type defines the logic node for this class.
     */
    typedef stOmniLogicNode < ObjectType, EvaluatorType > tLogicNode;

    /**
     * this types defines the distance node, using basic elements as object type and evaluator type.
     */
    typedef stBasicArrayObject <double> tBasicArrayObject;

    typedef stLInfinityMetricEvaluator <double> tBasicMetricEvaluator;

    typedef stOmniLogicNode <tBasicArrayObject, tBasicMetricEvaluator> tDistanceNode;

    /**
     * This type defines the OmniPivot  for this class.
     */
    typedef stOmniPivot < ObjectType, EvaluatorType > tOmniPivot;



    /**
     * Number of focus.
     */
    u_int32_t NumFocus;


    u_int32_t NumOmni;

    /**
     * The page manager used for original objects
     */
    stPageManager * myPageManager;

    /**
     * The page manager used for objects mapped by distances
     */
    vector <stPlainDiskPageManager *> PageManagers;

    EvaluatorType * myMetricEvaluator;

    tBasicMetricEvaluator * myBasicMetricEvaluator;

    /**
     * my GridPivot
     */
    vector <tOmniPivot *> OmniPivot;

    /*
     * Logic node to storage Original Objects
     */
    tLogicNode * LogicNode;

    /*
     * Logic node to storage, objects distances from pivots
     */
    vector <tDistanceNode *> DistanceNode;

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
    long GetNumberOfObjects() {
        return LogicNode->GetNumberOfObjects();
    }//end GetNumberOfObjects

    long GetNumberOfDistances() {
        return DistanceNode[0]->GetNumberOfObjects();
    }//end GetNumberOfObjects

    long GetNumberOfNodes() {
        return LogicNode->GetNumberOfNodes();
    }//end GetNumberOfObjects

    /**
     * Builds a new instance of this class.
     * @param nFocus Number of focus
     * @param metricEvaluator The metric evaluator to compute distances.
     */
    stMOmni(stPageManager * pageman, u_int32_t nOmni, u_int32_t nFocus, EvaluatorType * metricEvaluator);

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


    void FindPivot();

    /**
     * Builds the distance from the original objects to pivots.
     *
     */
    void BuildAllDistance();

    void AllDistance();

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
    tResult * RangeQueryOmniSeqB(tObject * sample, double range);

    /**
     * This method will perform a k nearest neighbor query.
     *
     * @param sample The sample object.
     * @param k The number of neighbours.
     * @param tie The tie list. Default false.
     * @return The result or NULL if this method is not implemented.
     * @warning The instance of tResult returned must be destroied by user.
     */
    tResult * NearestQueryOmniSeq(tObject * sample, u_int32_t k, bool tie = false);


private:


}; //stMOmni


//=============================================================================
// Class template stClOmni
//-----------------------------------------------------------------------------

/**
 * This class defines all behavior of the Omni.
 * $Date: 2009-02-27 23:08:44 $
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @version 1.0
 * @ingroup Omni
 */
template <class ObjectType, class EvaluatorType>
class stClOmni {
public:

    /**
     * This is the class that abstracts the object used by this metric tree.
     */
    typedef ObjectType tObject;

    /**
     * This is the class that abstracs an result set for simple queries.
     */
    typedef stResult <ObjectType> tResult;


    typedef stOmniOrigNode < ObjectType, EvaluatorType > tOrigNode;

    /**
     * This type defines the logic node for this class.
     */
    typedef stOmniLogicNode < ObjectType, EvaluatorType > tLogicNode;

    /**
     * this types defines the distance node, using basic elements as object type and evaluator type.
     */
    typedef stBasicArrayObject <double> tBasicArrayObject;

    typedef stLInfinityMetricEvaluator <double> tBasicMetricEvaluator;

    typedef stOmniLogicNode <tBasicArrayObject, tBasicMetricEvaluator> tDistanceNode;

    /**
     * This type defines the OmniPivot  for this class.
     */
    typedef stOmniPivot < ObjectType, EvaluatorType > tOmniPivot;

    /**
     * The page manager used for original objects
     */
    stPageManager * myPageManager;

    /**
     * The page manager used for objects mapped by distances
     */
    vector <stPlainDiskPageManager *> PageManagers;

    EvaluatorType * myMetricEvaluator;

    tBasicMetricEvaluator * myBasicMetricEvaluator;

    /**
     * my GridPivot
     */
    vector <tOmniPivot *> OmniPivot;

    /*
     * Logic node to storage Original Objects
     */
    tLogicNode * LogicNode;

    /*
     * Logic node to storage, objects distances from pivots
     */
    vector <tDistanceNode *> DistanceNode;

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
    long GetNumberOfObjects() {
        return LogicNode->GetNumberOfObjects();
    }//end GetNumberOfObjects

    long GetNumberOfDistances() {
        return DistanceNode[0]->GetNumberOfObjects();
    }//end GetNumberOfObjects

    long GetNumberOfNodes() {
        return LogicNode->GetNumberOfNodes();
    }//end GetNumberOfObjects

    /**
     * Builds a new instance of this class.
     * @param nFocus Number of focus
     * @param metricEvaluator The metric evaluator to compute distances.
     */
    stClOmni(stPageManager * pageman, u_int32_t nCluster, u_int32_t nFocus, EvaluatorType * metricEvaluator);

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


    void Cluster();

    void FindPivot();

    /**
     * Builds the distance from the original objects to pivots.
     *
     */
    void BuildAllDistance();

    void AllDistance();

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
    tResult * RangeQueryOmniSeqB(tObject * sample, double range);

    /**
     * This method will perform a k nearest neighbor query.
     *
     * @param sample The sample object.
     * @param k The number of neighbours.
     * @param tie The tie list. Default false.
     * @return The result or NULL if this method is not implemented.
     * @warning The instance of tResult returned must be destroied by user.
     */
    tResult * NearestQueryOmniSeq(tObject * sample, u_int32_t k, bool tie = false);


private:

    /**
     * Number of focus.
     */
    u_int32_t NumFocus;

    //u_int32_t NumOmni;

    u_int32_t NumClusters;


    /**
     * This type represents clusters.
     */
    struct stCluster {
        /**
         * Mean of the cluster.
         */
        ObjectType * ClusterMedoid;
        /**
         * Number of object in this cluster
         */
        u_int32_t Count;
        /**
         *
         */
        long clOID;
        /*
         * keep the distance to the farthest object
         */
        double maxDist;
        /**
         * 0 - not to visit
         * 1 - must visit
         * 2 - already visit
         */
        int status;

        double Cost;
    }; // end stMGridCluster

    stCluster *Clusters;

}; //stMOmni


// Include implementation
#include "stOmni-inl.h"

#endif //__stOmni_H
