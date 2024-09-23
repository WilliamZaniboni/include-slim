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
* This file defines the class stMetricTree.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STMETRICTREE_H
#define __STMETRICTREE_H

#include <arboretum/stPageManager.h>
#include <arboretum/stMetricAccessMethod.h>
#include <arboretum/stResult.h>
#include <arboretum/stUtil.h>
#include <stdexcept>
#include <arboretum/stQueryHint.h>
#include <arboretum/stTreeInformation.h>

// Include disk access statistics classes
#ifdef __stDISKACCESSSTATS__
   #include <arboretum/stHistogram.h>
   #include <arboretum/stLevelDiskAccess.h>
#endif //__stDISKACCESSSTATS__

//-----------------------------------------------------------------------------
// Class template stMetricTree
//-----------------------------------------------------------------------------
/**
* This class defines the basic interface used by all metric trees implemented by
* this library.
*
* <P>Developers are encouraged to use this class as the interface to manipulate all
* trees in their applications (polymorphism).
*
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Josiel Maimone de Figueiredo (josiel@icmc.usp.br)
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @version 1.0
* @ingroup struct
*/
template <class ObjectType, class EvaluatorType>
class stMetricTree: public stMetricAccessMethod < ObjectType, EvaluatorType >{

   public:

      /**
      * This is the class that abstracts the object used by this metric tree.
      */
      typedef ObjectType tObject;

      /**
      * This is the class that abstracts the metric evaluator used by this
      * metric tree.
      */
      typedef EvaluatorType tMetricEvaluator;

      /**
      * This is the class that abstracs an result set.
      */
      typedef stResult <ObjectType> tResult;

      #ifdef __stDISKACCESSSTATS__
         typedef stHistogram < ObjectType, EvaluatorType > tHistogram;
      #endif //__stDISKACCESSSTATS__

      /**
      * Creates a new metric tree using a given page manager. This instance will
      * not claim the ownership of the given page manager. It means that the
      * application must dispose the page manager when it is no loger necessary.
      *
      * @param pageman The bage manager to be used by this metric tree.
      * @deprecated This constructor will be deprecated soon.
      */
      stMetricTree(stPageManager * pageman):
            stMetricAccessMethod < ObjectType, EvaluatorType >(NULL){

         // Common to all Metric trees
         myPageManager = pageman;
         stMetricAccessMethod < ObjectType, EvaluatorType >::myMetricEvaluator = new tMetricEvaluator();
         sharedMetricEvaluator = false; // Not shared
      }//end stMetricTree

      /**
      * Creates a new metric tree using a given page manager and metric evaluator.
      * This instance will not claim the ownership of the given page manager and
      * metric evaluator. It means that the application must dispose them when
      * they are no loger necessary. It allows the metric evaluator to be
      * shared by other objects.
      *
      * @param pageman The bage manager to be used by this metric tree.
      * @param metricEval A shared metric evaluator to be used.
      */
      stMetricTree(stPageManager * pageman, EvaluatorType * metricEval):
            stMetricAccessMethod < ObjectType, EvaluatorType >(metricEval){

         // Common to all Metric trees
         myPageManager = pageman;
         sharedMetricEvaluator = true; // Shared
      }//end stMetricTree

      /**
      * Disposes this instance and release all associated resources. The page
      * manager will never be disposed by this destructor.
      */
      virtual ~stMetricTree(){

         if (!sharedMetricEvaluator){
            delete stMetricAccessMethod < ObjectType, EvaluatorType >::myMetricEvaluator;
         }//end if
      }//end stMetricTree

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
         virtual double GetEstimateDiskAccesses(double range,
            double fractalDimension, double maxDistance){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end PointQuery

         /**
         * Return the an estimation of the average of disk access for a Range
         * querywith radius range. This is a fast way to return this average.
         * This formula is based on the paper Traina, Traina, Faloutsos
         * and Seeger, TKDE'2000.
         *
         * @param range the radius of the range query.
         * @param fractalDimension the fractal dimension of the index dataset.
         * @param maxDistance the greater distance between two objects indexed.
         * @see GetEstimateDiskAccesses
         */
         virtual double GetFastEstimateDiskAccesses(double range,
            double fractalDimension, double maxDistance){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end PointQuery

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
         virtual double GetFatFactorFastEstimateDiskAccesses(double fatFactor,
               double range, double fractalDimension, double maxDistance){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end PointQuery

         virtual double GetCiacciaEstimateDiskAccesses(double range,
               tHistogram * histogram){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end PointQuery

         virtual double GetCiacciaEstimateDistCalculation(double range,
               tHistogram * histogram){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end PointQuery

         virtual void CalculateLevelStatistics(stLevelDiskAccess * levelDiskAccess){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end CalculateLevelStatistics

         virtual double GetCiacciaLevelEstimateDiskAccesses(
                  stLevelDiskAccess * levelDiskAccess, double range,
                  tHistogram * histogram){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end GetCiacciaLevelEstimateDiskAccesses

         virtual void GenerateLevelHistograms(tHistogram ** histogram){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end GenerateLevelHistograms

         virtual void GenerateSampledLevelHistograms(tHistogram ** histogram){
            throw std::logic_error("Unsupported method! Contact the \
                                            tree author for more details.");
         }//end GenerateSampledLevelHistograms

         virtual double GetTestLevelEstimateDiskAccesses(
                  stLevelDiskAccess * levelDiskAccess, tHistogram ** histogram,
                  double range){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end GenerateSampledLevelHistograms

         virtual void GenerateTestHistograms(tHistogram ** histogram){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end GenerateSampledLevelHistograms

         virtual double TestEstimateRQ(
               tHistogram ** histogram, double range){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end GetTestLevelEstimateDiskAccesses

         virtual double GetTestEstimateDiskAccesses(tHistogram ** histogram,
                                                    double range){
            throw std::logic_error("Unsupported method! Contact the tree \
                                            author for more details.");
         }//end

         virtual double GetSampledLevelEstimateDiskAccesses(
               stLevelDiskAccess * levelDiskAccess,
               tHistogram ** histogram, double range){
            throw std::logic_error("Unsupported method! Contact the tree author for more details.");
         }//end GetSampledLevelEstimateDiskAccesses
      #endif //__stDISKACCESSSTATS__

      /**
      * Returns the page manager used by this slim tree.
      */
      stPageManager * GetPageManager(){
         return myPageManager;
      }//end GetPageManager

      /**
      * Returns the minimum occupation in a node of the tree.
      * If not implemented, returns -1.
      */
      virtual double GetMinOccupation(){
         return -1;
      }//end GetMinOccupation

      /**
      * Returns the maximum occupation in a node of the tree.
      * If not implemented, returns 0.
      */
      virtual u_int32_t GetMaxOccupation(){
         return 0;
      }//end GetMaxOccupation

      /**
      * Returns the height of the tree.
      *
      * @return The height of the tree or -1 if this information is not available.
      * @see GetTreeInfo()
      */
      virtual u_int32_t GetHeight(){
         return 0;
      }//end GetHeight

      /**
      * Returns the number of nodes of this tree.
      *
      * @return The number of nodes used by the tree or -1 if this information
      * is not available.
      */
      virtual long GetNodeCount(){
         return -1;
      }//end GetNodeCount

      /**
      * Returns the limit distance between 2 objects in the tree. That is
      * \f$ \forall a,b \in D, d(a,b) \le GetDistanceLimit()\f$. In other
      * words, there is no distance greater than this.
      * 
      * @return The maximum distance or -1 if this feature is not supported.
      */
      virtual double GetDistanceLimit(){
         return -1;
      }//end GetDistanceLimit()

      /**
      * This method returns information about the tree like FatFactor, mean
      * object size and others.
      *
      * @return An instance of stTreeInfoResult or NULL if this feature is
      * not supported.
      */
      virtual stTreeInfoResult * GetTreeInfo(){
         return NULL;
      }//end GetTreeInfo

   protected:

      /**
      * The page manager used by thismetric tree.
      */
      stPageManager * myPageManager;
      
   private:
      /**
      * If this flag is true, the metric evaluator pointed by myMetricEvaluator
      * is shared.
      */
      bool sharedMetricEvaluator;
};//end stMetricTree

#endif //__STMETRICTREE_H
