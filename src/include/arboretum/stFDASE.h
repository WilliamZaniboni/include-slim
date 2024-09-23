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
* This file defines the class stFDASE.
*
* @version 1.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
*/

#ifndef __STFDASE_H
#define __STFDASE_H

#include <arboretum/stFractalDimension.h>
#include <time.h>

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
template <class DataType, class ObjectType> class stFDASE{
   public:

      /**
       * Constructor method.
       *
       * @param NumberOfObjects Number of objects in vector ObjectsArray[]
       * @param ObjectsArray[] Vector of pointers to the objects to be processed
       * @param q power to raise the count of occupancies in the BOPS plot. This is usually set to 1 if generalized is true, and 2 if false.
       * @param generalized If true, uses S=C*(C-1)/2 else use s=C^q in the calculus of log(S)/(q-1)log(r), where C is the occupancy count.
       * @param Dimension The dimension of the dataset.
       * @param NumberOfPointsInInterval Number of points in the chosen interval.
       * @param PointSelect Determines if there were selection of point or not.
       * @param FittingAlgorithmFactor It can be 0 (LeastSquaredFitting) or 1 (RegionBasedLineFitting).
       * @param NormalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param XInterval Stores the values of the chosen interval.
       * @param YInterval Stores the values of the chosen interval.
       * @param Threshold The choosen threshold.
       */
      stFDASE(int NumberOfObjects, ObjectType **ObjectsArray,
             TRange q, int generalized, int Dimension, int NumberOfPointsInInterval,
             int PointSelect, int NormalizeFactor, int FittingAlgorithmFactor,
             double XInterval[30], double YInterval[30], double Threshold);

   private:
      /**
       * Number of groups found.
       */
      int groupCount;

      /*
       * Defines what hapenned in each iteration:
       *    0 - nothing yet
       *    1 - itFormed a group
       *    2 - does not itFormed because base and group are equal
       *    3 - control (itFormed a group)
       */
      int itFormed;

      /**
       * Indicates the level os recursion.
       */
      int level;

      /**
       * It is used to determine whether to stopo or not.
       */
      int stop;

      /**
       * Used to determine which attribute was the last inserted into MaskFDASE.
       */
      int last;

      /**
       * To store partial dimensions.
       */
      double A,B,C,D;

      /**
      * Vector with the number of each attribute and each Individual Contribution.
      */
      struct stObjects *Attributes;

      /**
       * This mask defines which attributes will be used in each iteration.
       * For instance, in IndividualContribution, this vector will have "1" only in
       * the index that corresponds to the attribute being calculated. For the others,
       * this value will be "0"
       */
      unsigned int *MaskFDASE;

      /**
       * This mask will define wich attribute was used and which was not.
       * If the value is 0, the attribute must not be used in any process.
       * If it is 1, the attribute must be used. Is it is 2, the attribute should or not
       * be used.
       */
      unsigned int *MaskUSE;

      /**
       * This mask is used to store the attributes that formed a group.
       */
      unsigned int *MaskANSW;

      /**
       * Stores which attributes form the base.
       */
      unsigned int *MaskBASE;

      /**
       * Stores the attributes that form the Correlation Base.
       */
      unsigned int *MaskCB;

      /**
       * Calculates de individual contribution of the attributes.
       *
       * @param NumberOfObjects Number of objects in vector ObjectsArray[]
       * @param ObjectsArray[] Vector of pointers to the objects to be processed
       * @param q power to raise the count of occupancies in the BOPS plot. This is usually set to 1 if generalized is true, and 2 if false.
       * @param generalized If true, uses S=C*(C-1)/2 else use s=C^q in the calculus of log(S)/(q-1)log(r), where C is the occupancy count.
       * @param Dimension The dimension of the dataset.
       * @param NormalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param XInterval Stores the values of the chosen interval.
       * @param YInterval Stores the values of the chosen interval.
       * @param FittingAlgorithmFactor It can be 0 (LeastSquaredFitting) or 1 (RegionBasedLineFitting).
       * @param NumberOfPointsInInterval Number of points in the chosen interval.
       * @param PointSelect Determines if there were selection of point or not.
       * @param Attributes Stores the inforamtion about every attribute.
       * @param MaskFDASE Determines which attributes have to be considered in the calculus.
       */
      void IndividualContribution(int NumberOfObjects, ObjectType **ObjectsArray, TRange q,
                     int generalized, int Dimension, int NormalizeFactor,
                     double XInterval[30], double YInterval[30], int FittingAlgorithmFactor,
                     int NumberOfPointsInInterval, int PointSelect, struct stObjects *Attributes,
                     unsigned int *MaskFDASE);

      /**
       * Sort the attributes in the descending order of individual contribution.
       *
       * @param Vector The vector that will be sorted.
       * @param start The start of the vector to be considered in each iteration.
       * @param Dimension The dimension to be considered in each iteration.
       */
      void SortAttributes(struct stObjects *Vector, int start, int Dimension);

      /**
       * Calculates de parcial dimension of the attributes.
       *
       * @param NumberOfObjects Number of objects in vector ObjectsArray[]
       * @param ObjectsArray[] Vector of pointers to the objects to be processed
       * @param q power to raise the count of occupancies in the BOPS plot. This is usually set to 1 if generalized is true, and 2 if false.
       * @param generalized If true, uses S=C*(C-1)/2 else use s=C^q in the calculus of log(S)/(q-1)log(r), where C is the occupancy count.
       * @param Dimension The dimension of the dataset.
       * @param NormalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param XInterval Stores the values of the chosen interval.
       * @param YInterval Stores the values of the chosen interval.
       * @param FittingAlgorithmFactor It can be 0 (LeastSquaredFitting) or 1 (RegionBasedLineFitting).
       * @param NumberOfPointsInInterval Number of points in the chosen interval.
       * @param PointSelect Determines if there were selection of point or not.
       * @param Attributes Stores the inforamtion about every attribute.
       * @param MaskFDASE Determines which attributes have to be considered in the calculus.
       * @param Threshold The choosen threshold.
       * @param MaskUSE determines which attributes should or not be used.
       */
      void PartialDimension(int NumberOfObjects, ObjectType **ObjectsArray,
                TRange q, int generalized, int Dimension, int NormalizeFactor,
                double XInterval[30], double YInterval[30], int FittingAlgorithmFactor,
                int NumberOfPointsInInterval, int PointSelect, struct stObjects *Attributes,
                unsigned int *MaskFDASE, double Threshold, unsigned int *MaskUSE);

      /**
       * Find the groups of attributes correlated.
       *
       * @param NumberOfObjects Number of objects in vector ObjectsArray[]
       * @param ObjectsArray[] Vector of pointers to the objects to be processed
       * @param q power to raise the count of occupancies in the BOPS plot. This is usually set to 1 if generalized is true, and 2 if false.
       * @param generalized If true, uses S=C*(C-1)/2 else use s=C^q in the calculus of log(S)/(q-1)log(r), where C is the occupancy count.
       * @param Dimension The dimension of the dataset.
       * @param NormalizeFactor It can be 0 (Independent) or 1 (Proportional).
       * @param XInterval Stores the values of the chosen interval.
       * @param YInterval Stores the values of the chosen interval.
       * @param FittingAlgorithmFactor It can be 0 (LeastSquaredFitting) or 1 (RegionBasedLineFitting).
       * @param NumberOfPointsInInterval Number of points in the chosen interval.
       * @param PointSelect Determines if there were selection of point or not.
       * @param Attributes Stores the inforamtion about every attribute.
       * @param MaskFDASE Determines which attributes have to be considered in the calculus.
       * @param Threshold The choosen threshold.
       * @param MaskUSE determines which attributes should or not be used.
       * @param MaskANSW stores the group.
       * @param level The level of recursion.
       */
      unsigned int * FindGroup(int NumberOfObjects, ObjectType **ObjectsArray,
                TRange q, int generalized, int Dimension, int NormalizeFactor,
                double XInterval[30], double YInterval[30], int FittingAlgorithmFactor,
                int NumberOfPointsInInterval, int PointSelect, struct stObjects *Attributes,
                unsigned int *MaskFDASE, double Threshold, unsigned int *MaskUSE,
                unsigned int *MaskANSW, int level);

      /**
       * This typedef were included to use the class template stFractalDimension.
       */
      stFractalDimension<DataType,ObjectType> *sFD;
};

// Include Template source
#include <arboretum/stFDASE-inl.h>

#endif __STFDASE
