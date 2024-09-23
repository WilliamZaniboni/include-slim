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
* This file defines the classes stCountingTree, stPairCellLowDimensional and
* stPairCellHighDimensional.
*
* @version 2.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
*
* @ingroup fastmap
*/

#ifndef __STCOUNTINGTREE_H
#define __STCOUNTINGTREE_H

#include <arboretum/stCellId.h>

//----------------------------------------------------------------------------
// Structure stPairCellLowDimensional
//----------------------------------------------------------------------------
/**
* This structure is used in arrays, to store data from low dimensional datasets,
* using vector.
*
* @version 2.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
* @ingroup fastmap
*/
//---------------------------------------------------------------------------
struct stPairCellLowDimensional{
   
   public:
   
      /**
      * Accumulated sum of points hitting this cell.
      */
      int SumOfPoints;
      
      /**
      * Pointer to the next level.
      */
      stPairCellLowDimensional *NextLevel;
      
};//end stPairCellLowDimensional
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// class stPairCellHighDimensional
//----------------------------------------------------------------------------
/**
* This class is used in arrays, to store data from high dimensional datasets,
* using linked list.
*
* @version 2.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
* @ingroup fastmap
*/
//---------------------------------------------------------------------------
class stPairCellHighDimensional {
   
   public:
   
      /**
      * Creates a new dinamic structure to store the cells for high dimensional
      * datasets, using linked list.
      *
      * @param stNumberOfDimensions The number os dimensions of the dataset.
      * @param stNumberOfGridLevels The number os levels where the point are gathered in.
      *
      */
      stPairCellHighDimensional(int dimension) {
         if (dimension < 33) {
            index = new stCellIdLowDimensional();
         }else{
            index = new stCellIdHighDimensional(dimension);
         }//end if
         SumOfPoints = 1;          // if the cell exists, then at least one point exists.
         NextLevel = 0;
         NextCell = 0;
         //stCountingTree::blocks += (double)20/1024;
      }//end stPairCellHighDimensional

      /**
      * Disposes this structure.
      */
      ~stPairCellHighDimensional() {
         delete index;  /*stCountingTree::blocks -= (double)20/1024; */
      }//end ~stPairCellHighDimensional

      /**
      * Address of the cell in the space.
      */
      stCellId * index;

      /**
      * Accumulated sum of points hitting the cell.
      */
      int SumOfPoints;

      /**
      * Pointer to the next level.
      */
      stPairCellHighDimensional * NextLevel;

      /**
      * Pointer for the next cell in the linked list.
      */
      stPairCellHighDimensional * NextCell;

};//end stPairCellHighDimensional
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Class stCountingTree
//----------------------------------------------------------------------------
/**
* This class implements a structure to gather the points of a dataset and 
* manipulates them.
*
* @version 2.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
* @ingroup fastmap
*/
//----------------------------------------------------------------------------
class stCountingTree{
   
   public:
   
      /**
      * Constructor method to initialize one memory structure.
      *
      * @param NumberOfDimensions The number os dimensions of the dataset.
      * @param NumberOfGridLevels The number os levels where the point are gathered in.
      */
      stCountingTree (int dimensions = 2, int gridLevels = 10);
           
      /**
      * Disposes this grid.
      */
      ~stCountingTree();

      /**
      * After the structure had been concluded, executes the BoxCounting procedure,
      * which will count the points hitting each cell of the grid.
      *
      * @param VectorNumberOfPoints Has the number of points in each level.
      * @param VectorSq Has the result of ((number of points * number of points)-1)/2
      * @param SelfCount Counts the recursive iterations of this method.
      */
      void Count(int * vectorNumberOfPoints, double * vectorSq, int selfCount);
      
      /**
      * After the structure had been concluded, executes the BoxCounting procedure,
      * which will count the points hitting each cell of the grid.
      * Top is used to calculate the normgem factor to normalize every gen plot to
      * the not generalized (q=2) one.
      *
      * @param VectorNumberOfPoints Has the number of points in each level.
      * @param VectorSq Has the result of ((number of points * number of points)-1)/2.
      * @param SelfCount Counts the recursive iterations of this method.
      */
      void GenericCount(int * vetorNumberOfPoints, double * vetorSq,
                        int selfCount, double q, double & top);
      
      /**
      * Sets the Array of normalization constants to be applied to each point,
      * as each one will be being inserted in the memory structure.
      *
      * @param Slope Vector with the values of X points.
      * @param YInc  Vector with the values of Y points.
      */
      void Normalize(double * slope, double * yInc);

      /**
      * Inserts one point into the memory structure.
      *
      * @param Ponto The point.
      * @param NormPoint The normalized point.
      * @return A value that determines if the method should continue or not.
      */
      int Point(double * ponto, double * normPoint);
      
      /**
      * Returns the number of memory blocks used in each refinement level and the
      * total number of blocks used in all levels.
      *
      * @param Total The total number of blocks used.
      * @param Block Vector that stores the values of the global vector UseBlock.
      */
      void Uses(int * total, int * block);
      
      /**
      * After the structure had been concluded, executes the BoxCounting procedure to
      * generate the Histogram File.
      *
      * @param biggest The biggest value.
      * @param NumberOfPointsInGrid
      * @param HgFile The output file of the Histogram.
      */
      void Histogram(double biggest, int numberOfPointsInGrid, char * hgFile);
  
      /**
      * After the Structure had been concluded, print every node.
      */
      void Print();

      /**
      * Internal, recursive, print every node in array structure.
      *
      * @param GridLevel The level of the grid.
      * @param PairCell Structure that stores the index and sum of points hitting every cell.
      */
      void CellPrint(int gridLevel, stPairCellLowDimensional * pairCell);

      /**
      * Internal, recursive, print every node in list structure.
      *
      * @param GridLevel The level of the grid.
      * @param PairCell Structure that stores the index and sum of points hitting every cell.
      */
      void CellPrintHighDimensional(int gridLevel, stPairCellHighDimensional * pairCell);

      /**
      * Internal, recursive, print one node in list structure.
      *
      * @param GridLevel The level of the grid.
      * @param PairCell Structure that stores the index and sum of points hitting every cell.
      */
      void PrintOneCellHighDimensional(int gridLevel, stPairCellHighDimensional * pairCell);

      /**
      * Created to help statistic process,identifies the position of the point
      * in a grid divided "ndiv" times. For example: if the point falls in cell
      * 1 in the first level,in cell 2 in the 2nd level, in cell 1 in the 3rd
      * level and in cell 3 in the 4th level, then cellId[0]=1, cellId[1]=2,
      * cellId[2]=1, cellId[3]=3.
      *
      * @param Point The point to be processed.
      *
      */
      void ProcessPoint(double * point);

   private:
   
      /**
      * Number of Dimensions.
      */
      int NumberOfDimensions;
      
      /**
      * Number of Cells with this dimension.
      */
      int NumberOfCells;

      /**
      * Number of different square sizes or grid levels.
      */
      int NumberOfGridLevels;
      
      /**
      * To determine if it is a low dimensional or a high dimensional dataset.
      */
      int DimensionLessThanFour;
      
      /**
      * Vector of objects from stPairCellLowDimensional class.
      */
      stPairCellLowDimensional * FirstLevel;

      /**
      * Vector of objects from stPairCellHighDimensional class.
      */
      stPairCellHighDimensional * FirstLevelHighDimensional;

      /**
      * Vector that will be used to store the normalization constants to be
      * applied to each point as one will be being inserted in the memory structure.
      */
      double * NormalizeSlope;

      /**
      * Vector that will be used to store the normalization constants to be 
      * applied to each point as one will be being inserted in the memory structure.
      */
      double * NormalizeYInc;

      /**
      * Vector that will be used to store the number of memory blocks used in 
      * each refinement level.
      * applied to each point as one will be being inserted in the memory structure.
      */
      int * UseBlock;
      
      /**
      * Vector that will be used to store the total number of memory blocks 
      * used in all levels of refinement. 
      * applied to each point as one will be being inserted in the memory structure.
      */ 
      int TotalUseBlock;
      
      /**
      * Delete the memory structure.
      */
      void Reset(void * x);

      /**
      * Internal Recursive procedure to insert one point into one level of
      * refinement in the memory structure
      */
      void CellLowDimensional_process(int gridLevel, stPairCellLowDimensional * pairCell,
                                      double * min, double * max, double * normPoint);

      /**
      * Internal Recursive procedure to insert one point into one level of
      * refinement in the memory structure
      */
      void CellHighDimensional_process(int gridLevel, stPairCellHighDimensional ** pPC,
                                       double * min, double * max, double * normPoint);

      /**
      * Internal, recursive, BoxCounting procedure.
      */
      void CellCount(int gridLevel, stPairCellLowDimensional * pairCell,
                     int * vetorSq, double * vetorNPoints, int selfCount);

      /**
      * Internal, recursive, BoxCounting procedure.
      */
      void CellCountHighDimensional(int gridLevel, stPairCellHighDimensional * pairCell,
                                    int * vetorNPoints, double * vetorSq, int countself);

      /**
      * Internal, recursive, BoxCounting procedure to not generalized
      * normalization factor (q=2).
      */
      void CellCountGen(int gridLevel, stPairCellLowDimensional * pairCell,
                        int * vetorNPoints, double * vetorSq, int countself, double q);

      /**
      * Internal, recursive, BoxCounting procedure to not generalized
      * normalization factor (q=2).
      */
      void CellCountGenHighDimensional(int gridLevel, stPairCellHighDimensional * pairCell,
                                       int * vetorNPoints, double * vetorSq, int countself, double q);

      /**
      * Internal, recursive, BoxCounting procedure to make the histogram.
      */
      void HistogramRecursive(int level, int numberOfPointsInGrid, stPairCellLowDimensional * pairCell,
                              FILE * hgFile);
      
};//end stCountingTree

#endif
