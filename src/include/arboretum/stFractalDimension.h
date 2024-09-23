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
* This file defines the class stLineFitting.
*
* @version 1.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Rodrigo Nishihara Adï¿½o (adao@grad.icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
*/

#ifndef __STFRACTALDIMENSION_H
#define __STFRACTALDIMENSION_H

#include <arboretum/stCountingTree.h>
#include <arboretum/stLineFitting.h>

//----------------------------------------------------------------------------
// template class stFractalDimension
//----------------------------------------------------------------------------
/**
* This class is used to calculate de fractal dimension.
*
* @version 1.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
* @ingroup fastmap
*/
//---------------------------------------------------------------------------

/**
* Type to store the desired range
*/
typedef struct stRange {
    double min;
    double max;
    double step;
} TRange;

/**
* Determines the kind of scale that will be used.
*/
enum SetScale {
   ScaleLinLin = 0,
   ScaleLogLog = 3
};

/**
* Struct to store information about each attribute
*/
struct stObjects{
   int attribute;
   double IndividualContribution;
};

template <class DataType, class ObjectType> class stFractalDimension{
   
   public:
   
      /**
       * Constructor method.
       *
       * @param numberOfObjects Number of objects in vector objectsArray[]
       * @param objectsArray[] Vector of pointers to the objects to be processed
       * @param q power to raise the count of occupancies in the BOPS plot. This is usually set to 1 if generalized is true, and 2 if false.
       * @param generalized If true, uses S=C*(C-1)/2 else use s=C^q in the calculus of log(S)/(q-1)log(r), where C is the occupancy count.
       * @param dimension The dimension of the dataset.
       * @param normalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param xInterval Stores the values of the chosen interval.
       * @param yInterval Stores the values of the chosen interval.
       * @param fittingAlgorithmFactor It can be 0 (LeastSquaredFitting) or 1 (RegionBasedLineFitting).
       * @param numberOfPointsInInterval Number of points in the chosen interval.
       * @param pointSelect Determines if there were selection of point or not.
       * @param mask Determines which attributes have to be considered in the calculus.
       * @param attributes Vector with the number of each attribute and each Individual Contribution.
       */
      stFractalDimension(int numberOfObjects, ObjectType ** objectsArray, TRange q,
                         int generalized, int dimension, int numberOfPointsInInterval,
                         int pointSelect, int normalizeFactor, int fittingAlgorithmFactor,
                         double xInterval[30], double yInterval[30], unsigned int * mask,
                         struct stObjects * attributes);

      /**
       * Method to get Alpha value.
       *
       * @return Alpha The obtained value of Alpha.
       */
      DataType GetAlpha() {
         return this->Alpha;
      }//end GetAlpha


      /**
       * Method to get Beta value.
       *
       * @return Beta The obtained alue of Beta.
       */
      DataType GetBeta() {
         return this->Beta;
      };//end GetBeta

   private:
   
      /**
       * The slope of the adjusted line.
       */
      DataType Alpha;

      /**
       * The increment in the Y-axis of the adjusted line.
       */
      DataType Beta;

      /**
       * Determines that the generated plots must be normalized.
       */
      int NormalizeGeneratedPlots;

      /**
       * Determines that the generalized plots must be normalized.
       */
      int NormalizeGeneralizedPlots;

      /**
       * Determines which scale must be used in the graphic.
       */
      SetScale DefScale;

      /**
      * Number of different square sizes or grid levels.
      */
      int NumberOfGridLevels;

      /**
       * Used to do the Histogram, provenient from stCountingTree
       */
      int NPGrid;

      /**
       * Variable to determine wether the slow or fast calculation must be proceeded.
       * @todo Slow calculation will be implemented later.
       */
      int SetFastDE;

      /**
       * Variable to determine wether the slow or fast calculation must be proceeded.
       * @todo Slow calculation will be implemented later.
       */
      bool Fastcalculation;

      /**
       * Process the objects to find out the fractal dimension
       *
       * @param numberOfObjects Number of objects in vector objectsArray[]
       * @param objectsArray[] Vector of pointers to the objects to be processed
       * @param q power to raise the count of occupancies in the BOPS plot. This is usually set to 1 if generalized is true, and 2 if false.
       * @param generalized If true, uses S=C*(C-1)/2 else use s=C^q in the calculus of log(S)/(q-1)log(r), where C is the occupancy count.
       * @param dimension The dimension of the dataset.
       * @param normalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param xInterval Stores the values of the chosen interval.
       * @param yInterval Stores the values of the chosen interval.
       * @param fittingAlgorithmFactor It can be 0 (LeastSquaredFitting) or 1 (RegionBasedLineFitting).
       * @param numberOfPointsInInterval Number of points in the chosen interval.
       * @param pointSelect Determines if there were selection of point or not.
       * @param mask Determines which attributes have to be considered in the calculus.
       * @param attributes Vector with the number of each attribute and each Individual Contribution.
       */
      void ProcessDE(int numberOfObjects, ObjectType ** objectsArray, TRange q,
                     int generalized, int dimension, int normalizeFactor,
                     double xInterval[30], double yInterval[30], int fittingAlgorithmFactor,
                     int numberOfPointsInInterval, int pointSelect, unsigned int * mask,
                     struct stObjects * attributes);

      /**
       * Process all the objects of the input file, count them and retrieve
       * their longest size. Then create the grid structure.
       *
       * @param numberOfObjects Number of objects in the dataset.
       * @param objectsArray Vector of objects read.
       * @param calcTree The structure where objects are placed to be counted.
       * @param arrayX The values of r, where r is the lenght of the side of a grid cell.
       * @param dimension The dimension of the dataset.
       * @param normalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param mask Determines which attributes have to be considered in the calculus.
       * @param countAttributes The number of attributes to be considered.
       * @param attributes Vector with the number of each attribute and each Individual Contribution.
       *
       * @return The number of objects processed.
       */
      int FastDistExponent(int numberOfObjects, ObjectType **objectsArray, stCountingTree *calcTree,
                           double *arrayX, int dimension, int normalizeFactor, unsigned int *mask,
                           int countAttributes, struct stObjects *attributes);

      /**
       * Count the number of points that intersects each region in the structure
       * and apply the formula PCa^q.
       *
       * @param q Range of variation for q in the formula: PCa^q
       * @param generalized Flag to say whether the generic(true) or specific (false) to D2 routine is to be used.
       * @param calcTree The structure where objects are placed to be counted.
       * @param constant Constant to do the normalization.
       * @param arrayY The values of SqR, where r is the lenght of the side of a grid cell.
       */
      double GenerateBOPS(TRange q, int generalized, stCountingTree * calcTree,
                          double & constant, double * arrayY);

      /**
       * Calculates in destX and destY the log of the points in origX and origY.
       * The result in destY is multiplyed by "constant".
       *
       * @param numberOfObjects Number of objects.
       * @param constant Constant to do the normalization.
       * @param origX Original vector.
       * @param origX Original vector.
       * @return destX The values of log(origX).
       * @return destY The values of log(origY).
       */
      void ToLog(int numberOfObjects, double constant, double * destX, double * destY,
                 double * origX, double * origY);

      /**
       * Retrieve the minimum and maximum coordinate values for a given dataset
       * "objectsArray" with "numberOfObjects" stObjects.
       *
       * @param numberOfObjects Number of objects.
       * @param objectsArray Vector of objects read.
       * @param dimension The dimension of the dataset.
       * @param mask Determines which attributes have to be considered in the calculus.
       * @param attributes Vector with the number of each attribute and each Individual Contribution.
       *
       * @return min The minimum value.
       * @return max The maximum value.
       */
      void MinMax(int numberOfObjects, ObjectType ** objectsArray, DataType * min,
                  DataType * max, int dimension, unsigned int * mask, int countAttributes,
                  struct stObjects * attributes);

      /**
       * Do not normalize only generalized plots to the one withp=q=1. Use Log or Sum instead.
       *
       * @param q Maximum value of q.
       * @return The constant to be used.
       */
      double SelfConstant(double q);

      /**
       * This typedef were included to use the class template stLineFitting.
       */
      typedef stLineFitting<double> stFitting;
      stFitting sFit;
};

// Include Template source
#include <arboretum/stFractalDimension-inl.h>

#endif __STFRACTALDIMENSION
