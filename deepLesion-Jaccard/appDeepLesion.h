
#ifndef appDeepLesion
#define appDeepLesion

#include <chrono>

// Metric Tree includes
#include <arboretum/stMetricTree.h>
#include <arboretum/stPlainDiskPageManager.h>
#include <arboretum/stDiskPageManager.h>
#include <arboretum/stMemoryPageManager.h>
#include <arboretum/stSlimTree.h>
#include <arboretum/stDummyTree.h>
#include <arboretum/stMetricTree.h>

// My object
#include "deepLesion.h"

#include <string.h>
#include <fstream>

#define GEONAMESFILE "files/deepLesionFeatSet22K.txt"
#define QUERYGEONAMESFILE "files/deepLesionFeatSetQuery-1.txt"

//---------------------------------------------------------------------------
// class TApp
//---------------------------------------------------------------------------
class AppDeepLesion
{
public:
   /**
    * This is the type used by the result.
    */
   typedef stResult<DeepLesion> myResult;

   typedef stMetricTree<DeepLesion, DeepLesionDistanceEvaluator> MetricTree;

  
   typedef stSlimTree<DeepLesion, DeepLesionDistanceEvaluator> mySlimTree;

   typedef stDummyTree<DeepLesion, DeepLesionDistanceEvaluator> myDummyTree;

   /**
    * Creates a new instance of this class.
    */
   AppDeepLesion()
   {
      PageManager = NULL;
      SlimTree = NULL;
      DummyTree = NULL;
      PageManagerDummy = NULL;

   } // end TApp

   /**
    * Initializes the application.
    *
    * @param pageSize
    * @param minOccup
    * @param quantidade
    * @param prefix
    */
   void Init()
   {
      // To create it in disk
      CreateDiskPageManager();
      // Creates the tree
      CreateTree();
   } // end Init

   /**
    * Runs the application.
    *
    * @param DataPath
    * @param DataQueryPath
    */
   void Run();

   /**
    * Deinitialize the application.
    */
   void Done();

private:
   /**
    * The Page Manager for SlimTree.
    */
   stPlainDiskPageManager *PageManager;
   stPlainDiskPageManager *PageManagerDummy;

   /**
    * The SlimTree.
    */
   MetricTree *SlimTree;

   MetricTree *DummyTree;

   /**
    * Vector for holding the query objects.
    */
   vector<DeepLesion *> queryObjects;

   /**
    * Creates a disk page manager. It must be called before CreateTree().
    */
   void CreateDiskPageManager();

   /**
    * Creates a tree using the current PageManager.
    */
   void CreateTree();

   /**
    * Loads the tree from file with a set of cities.
    */
   void LoadTree(char *fileName);

   void LoadDummyTree(char *fileName);

   /**
    * Loads the vector for queries.
    */
   void LoadVectorFromFile(char *fileName);

   void LoadSequential(char *fileName);

   /**
    * Performs the queries and outputs its results.
    */
   void PerformQueries();

   void PerformNearestQuery();

   void PerformNearestQueryDummy();

   void PerformRangeQuery();

   void PerformRangeQueryDummy();

   void PerformExistsQuery();
   void PerformExistsQueryDummy();

   void PerformNearestQueryOneCenter();
   void PerformNearestQueryOneCenterDummy();

   void PerformRangeQueryOneCenter();
   void PerformRangeQueryOneCenterDummy();

}; // end TApp

#endif // end appH
