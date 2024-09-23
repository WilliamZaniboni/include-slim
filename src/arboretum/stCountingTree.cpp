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
* This file implements the classes stCountingTree and stPairCellHighDimensional.
*
* @version 2.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
*/

#include <arboretum/stCountingTree.h>

//------------------------------------------------------------------------------
// class stCountingTree
//------------------------------------------------------------------------------
stCountingTree::stCountingTree (int dimensions, int gridLevels) {
   NumberOfDimensions = dimensions;
   NumberOfCells = 2;

   /*NumberOfCells are twice NumberOfDimensions because we're gonna store the
   data and the pointer to the next structure.*/
   for (int i=1; i<NumberOfDimensions; i++) {
      NumberOfCells *= 2;
   }//end for
   NormalizeSlope = new double[NumberOfDimensions];
   NormalizeYInc = new double[NumberOfDimensions];
   NumberOfGridLevels = gridLevels;
   UseBlock = new int[NumberOfGridLevels+1];

   //Initializing
   for(int i=0; i<NumberOfDimensions; i++){
      NormalizeSlope[i] = 1.0;
      NormalizeYInc[i] = 0.0;
   }//end for

   /*Cases that have less than four attributes are treated with static structures,
   being faster.*/
   DimensionLessThanFour = NumberOfDimensions < 4;
   if (DimensionLessThanFour){
      FirstLevel = new stPairCellLowDimensional[NumberOfCells];
      for (int j=0; j<NumberOfCells; j++){
         FirstLevel[j].NextLevel = 0;
         FirstLevel[j].SumOfPoints = 0;
      }//end for
   }else{
      FirstLevelHighDimensional = 0;
   }//end if

   //Initializing
   for (int i=0; i<=NumberOfGridLevels; i++) {
      UseBlock[i] = 0;
   }//end for
   TotalUseBlock = 1;
}//end stCountingTree::stCountingTree

//---------------------------------------------------------------------------
stCountingTree::~stCountingTree() {
   if (DimensionLessThanFour) {
      Reset(FirstLevel);
   }else{
      Reset(FirstLevelHighDimensional);
   }//end for
   delete NormalizeSlope;
   delete NormalizeYInc;
   delete UseBlock;
}//end stCountingTree::~stCountingTree

//---------------------------------------------------------------------------
void stCountingTree::Reset(void * x) {
   if (x==0) {
      return;
   }if (DimensionLessThanFour) {
      stPairCellLowDimensional * Y = (stPairCellLowDimensional *)x;
      for (int i=0; i<NumberOfCells; i++){
         Reset (Y[i].NextLevel);
      }//end for
      delete Y;
   }else{
      stPairCellHighDimensional * Y = (stPairCellHighDimensional *)x;
      while (Y!=0){         // while there are cells in this level
         Reset(Y->NextLevel);    //   reset the sub-tree
         stPairCellHighDimensional * aux = Y;  //   hold this cell
         Y = Y->NextCell;          //   get next cell
         delete aux;        //   delete the holded cell
      }//end while
   }//end if
}//end stCountingTree::Reset

//---------------------------------------------------------------------------
void stCountingTree::Normalize(double * slope, double * yInc) {
   for (int i=0; i<NumberOfDimensions; i++) {
      NormalizeSlope[i] = *slope++;
      NormalizeYInc[i] = *yInc++;
   }//end for
}//end stCountingTree::Normalize

//---------------------------------------------------------------------------
int stCountingTree::Point(double * ponto, double * normPoint){
   double * min, * max;
   int out;
   min = new double[NumberOfDimensions];
   max = new double[NumberOfDimensions];

   // Normalize the point
   out=0;
   for (int i=0; i<NumberOfDimensions; i++){
      normPoint[i] = ((*ponto++)-NormalizeYInc[i])/NormalizeSlope[i];
      if (normPoint[i]>1) {
         out = 1;
      }//end if
      min[i] = 0;
      max[i] = 1.0;
   }//end for
   // recursively process this points deeply in all levels of refinements
   if (out==0){
      if (DimensionLessThanFour) {
         CellLowDimensional_process(0, FirstLevel, min, max, normPoint);
      }else {
         CellHighDimensional_process(0, &FirstLevelHighDimensional, min, max, normPoint);
      }//end if
   }//end if
   delete min;
   delete max;
   return !out;
}//end stCountingTree::Point

//---------------------------------------------------------------------------
void stCountingTree::CellLowDimensional_process(int gridLevel, 
      stPairCellLowDimensional * pairCell, double * min, 
      double * max, double * normPoint){
         
   int i, cell;
   double middle;

   //Binary process
   for (i=0, cell=0; i<NumberOfDimensions; i++ ){
      cell <<= 1;
      middle = (max[i]-min[i])/2+min[i];
      if (normPoint[i] > middle) {
         cell += 1;
         min[i] = middle;
      }//end if
      else
         max[i] = middle;
   }//end for

   //Creating the grid structure recursively
   pairCell[cell].SumOfPoints++;
   if (gridLevel<NumberOfGridLevels){
      if (pairCell[cell].NextLevel==0){
         stPairCellLowDimensional * Aux = new stPairCellLowDimensional[NumberOfCells];
         pairCell[cell].NextLevel = Aux;
         for (int j=0; j<NumberOfCells; j++){
            Aux[j].NextLevel = 0;
            Aux[j].SumOfPoints = 0;
         }//end for
         UseBlock[gridLevel]++;
         TotalUseBlock++;
      }//end if
      CellLowDimensional_process(gridLevel+1,pairCell[cell].NextLevel, min, max, normPoint);
   }//end if
}//end stCountingTree::CellLowDimensional_process

//---------------------------------------------------------------------------
void stCountingTree::CellHighDimensional_process(int gridLevel, 
      stPairCellHighDimensional ** pPC, double * min,
      double * max, double * normPoint) {
         
   short i, comparing;
   stCellId * cell;
   double middle;
   stPairCellHighDimensional * pairCell;
   pairCell = *pPC;

   if (NumberOfDimensions<33) {
      cell = new stCellIdLowDimensional();
   }else {
      cell = new stCellIdHighDimensional(NumberOfDimensions);
   }//end if

   for (i=0; i<NumberOfDimensions; i++){
   //cell<<=1;          // OBS about using cell as an int (32 bits):
                        // each bit of the integer corresponds to a dimension
                        // (for bit 0 to bit 31, from left to right), and if the
                        // point has E dimensions such that E<32, only the E bits on the
                        // right are considered -
                        // in each pass bit 31 is set to 0 or 1, depending on the value of
                        // dimension i  - the bitwise shift done in each pass
                        // garantee that, at the end, each bit from 0 to 31 has the correct
                        // value (0 or 1) according to the values of each dimension of
                        // the point being processed  (bit is 0 for values in the first
                        // half of the dimension interval; and bit is 1 otherwise)

      cell->LeftShift(i+1);   // why i+1???
                              // see commentaries on the method definition

      middle=(max[i]-min[i])/2+min[i];
      if (normPoint[i] > middle) {
         *(cell)+=1;  // set bit 31 to 1 - the shifts of the loop put if on the right place
                         // according to the corresponding dimension.
         min[i] = middle;
      }else{
         max[i] = middle;
      }//end if
   }//end for
   if (pairCell==0){  // First cell in this subtree
      pairCell = new stPairCellHighDimensional(NumberOfDimensions);
      *pPC = pairCell;
      *(pairCell->index) = *cell;
      UseBlock[gridLevel]++;
      TotalUseBlock++;
   }else{      // The list already exists. Follow it.
      int done = false;
      stPairCellHighDimensional * auxCell = 0;
      while (pairCell != 0 && !done){ // searching the "horizontal list", if this cell was already allocated.
         //if (pairCell->index==cell) { // found!
         comparing = (*(pairCell->index)==*cell);
         if (comparing == 0){ // found!
            done = true;
            pairCell->SumOfPoints++;
            //} else if (pairCell->index<=cell) { // not yet! continue searching horizontally...
         }else{
            if(comparing < 0){
               auxCell=pairCell;
               pairCell=pairCell->NextCell;
            }else{   // oops: dont exists. Need to inser a new cell before this
               if (auxCell==0){  // a new cell inserted in the beguinning of the list
                  auxCell = new stPairCellHighDimensional(NumberOfDimensions);
                  *pPC = auxCell;
                  auxCell->NextCell = pairCell;
                  pairCell = auxCell;
               }else{        // a new cell inserted in the middle of the list
                  pairCell = new stPairCellHighDimensional(NumberOfDimensions);
                  pairCell->NextCell = auxCell->NextCell;
                  auxCell->NextCell = pairCell;
               }//end if
               *(pairCell->index) = *cell;
               done = true;
            }//end if
         }//end if
      } // end while - searching horizontal list
      if (!done){  // The whole horizontal list was travelled without find this cell.
         pairCell = new stPairCellHighDimensional(NumberOfDimensions); // insert in the end of the list
         auxCell->NextCell = pairCell;
         *(pairCell->index) = *cell;
      }//end if
   }//end if
   
   if (gridLevel<NumberOfGridLevels) {
      CellHighDimensional_process(gridLevel+1,&(pairCell->NextLevel), min, max, normPoint);
   }//end if

   delete cell;
}//end stCountingTree::CellHighDimensional_process

//---------------------------------------------------------------------------
void stCountingTree::Count(int *vectorNumberOfPoints, double *vectorSq, int selfCount){
   //Starts the count of points
   for (int i=0; i<=NumberOfGridLevels; i++) {
      vectorSq[i] = 0;
      vectorNumberOfPoints[i] = 0;
   }//end for
   if (DimensionLessThanFour) {
      CellCount(1, FirstLevel, vectorNumberOfPoints, vectorSq, selfCount);
   }else{
      CellCountHighDimensional(1, FirstLevelHighDimensional, vectorNumberOfPoints, vectorSq, selfCount);
   }//end if
   
   vectorSq[0] = double(vectorNumberOfPoints[1])*double(vectorNumberOfPoints[1]-1)/2.0;
   vectorNumberOfPoints[0]=vectorNumberOfPoints[1];

}//end stCountingTree::Count

//---------------------------------------------------------------------------
void stCountingTree::CellCount (int gridLevel, stPairCellLowDimensional * pairCell, 
      int * vectorNumberOfPoints, double * vectorSq, int selfCount){
   //Recursively count
   for (int i=0; i<NumberOfCells; i++){
      if (pairCell[i].NextLevel!=0) {
         CellCount(gridLevel+1, pairCell[i].NextLevel, vectorNumberOfPoints, vectorSq, selfCount);
      }//end if
      if (selfCount||(pairCell[i].SumOfPoints>1)) {
         vectorSq[gridLevel] += double(pairCell[i].SumOfPoints)*double(pairCell[i].SumOfPoints-1)/2.;
      }//end if
      vectorNumberOfPoints[gridLevel] += pairCell[i].SumOfPoints;
   }//end for
}//end stCountingTree::CellCount

//---------------------------------------------------------------------------
void stCountingTree::CellCountHighDimensional(int gridLevel, 
      stPairCellHighDimensional * pairCell, int * vectorNumberOfPoints,
      double * vectorSq, int selfCount) {
   while (pairCell != 0){ //If there is a list...
      if (pairCell->NextLevel!=0) {
         CellCountHighDimensional(gridLevel+1, pairCell->NextLevel, vectorNumberOfPoints, vectorSq, selfCount);
      }//end if
      if (selfCount || (pairCell->SumOfPoints)>1) {
         vectorSq[gridLevel] += double(pairCell->SumOfPoints)*double(pairCell->SumOfPoints-1)/2.;
      }//end if
      vectorNumberOfPoints[gridLevel] += pairCell->SumOfPoints;
      pairCell = pairCell->NextCell; //Goes to the next level.
   }//end while
}//end stCountingTree::CellCountHighDimensional

//---------------------------------------------------------------------------
void stCountingTree::GenericCount(int * vectorNumberOfPoints, double * vectorSq,
      int selfCount, double q, double & top){
   for (int i=0; i<=NumberOfGridLevels; i++){
      vectorSq[i] = 0;
      vectorNumberOfPoints[i] = 0;
   }//end for
   if (DimensionLessThanFour){
      CellCountGen(1, FirstLevel, vectorNumberOfPoints, vectorSq, selfCount, q);
   }else{
      CellCountGenHighDimensional(1, FirstLevelHighDimensional, vectorNumberOfPoints, vectorSq, selfCount, q);
   }//end if
   top = double(vectorNumberOfPoints[1]);
   vectorSq[0] = pow(top, q);
   vectorNumberOfPoints[0] = (int)top;
   top *= top;
}//end stCountingTree::GenericCount

//---------------------------------------------------------------------------
void stCountingTree::CellCountGen (int gridLevel, stPairCellLowDimensional * pairCell, 
      int * vectorNumberOfPoints, double * vectorSq, int selfCount, double q){
   for (int i=0; i<NumberOfCells; i++){
      if (pairCell[i].NextLevel!=0) {
         CellCountGen(gridLevel+1, pairCell[i].NextLevel, vectorNumberOfPoints, vectorSq, selfCount, q);
      }//end if
      if (selfCount||(pairCell[i].SumOfPoints>1)) {
         vectorSq[gridLevel] += pow (double(pairCell[i].SumOfPoints), q);
      }//end if
      vectorNumberOfPoints[gridLevel] += pairCell[i].SumOfPoints;
   }//end for
}//end stCountingTree::CellCountGen

//---------------------------------------------------------------------------
void stCountingTree::CellCountGenHighDimensional(int gridLevel, 
      stPairCellHighDimensional * pairCell, int * vectorNumberOfPoints,
      double * vectorSq, int selfCount, double q){
   while (pairCell != 0) {
      if (pairCell->NextLevel!=0) {
         CellCountGenHighDimensional(gridLevel+1, pairCell->NextLevel, vectorNumberOfPoints, vectorSq, selfCount, q);
      }//end if
      if (selfCount||(pairCell->SumOfPoints>1)) {
         vectorSq[gridLevel] += pow (double(pairCell->SumOfPoints), q);
      }//end if
      vectorNumberOfPoints[gridLevel] += pairCell->SumOfPoints;
      pairCell=pairCell->NextCell;
   }//end while
}//end stCountingTree::CellCountGenHighDimensional*/

//---------------------------------------------------------------------------
void stCountingTree::Uses(int *total, int *block){
   for (int i=0; i<=NumberOfGridLevels; i++) {
      block[i] = UseBlock[i];
   }//end for
   *total = TotalUseBlock;
}//end stCountingTree::Uses

//---------------------------------------------------------------------------
void stCountingTree::Histogram(double biggest, int numberOfPointsInGrid,
      char * hgFileName){
   if (!DimensionLessThanFour){
      printf("Histogram not implemented for high dimmensions!\n");
      return;
   }//end if
   FILE * hgFile = fopen(hgFileName, "w");
   if (hgFile == NULL){
      printf ("Error opening File %s\n", hgFileName);
      return;
   }//end if
   fprintf(hgFile, "%f %d %d\n", biggest, NumberOfDimensions, numberOfPointsInGrid);
   HistogramRecursive(1, numberOfPointsInGrid, FirstLevel, hgFile);
   fclose(hgFile);
}//end stCountingTree::Histogram

//---------------------------------------------------------------------------
void stCountingTree::HistogramRecursive (int level, 
      int numberOfPointsInGrid, stPairCellLowDimensional * pairCell,
      FILE * hgFile){
   for (int i=0; i<NumberOfCells; i++){
      if (pairCell[i].NextLevel!=0){
         if (level<numberOfPointsInGrid) {
            HistogramRecursive(level+1, numberOfPointsInGrid, pairCell[i].NextLevel, hgFile);
         }//end if            
         if (level==numberOfPointsInGrid) {
            fprintf(hgFile, "%d\n", pairCell[i].SumOfPoints);
         }//end if
      }else{
         int n = (int) (pow(pow (2., numberOfPointsInGrid-level), NumberOfDimensions)+.0001);
         for (int i=0; i<n; i++) {
            fprintf (hgFile, "0\n");
         }//end for
      }//end if
   }//end for
}//end stCountingTree::HistogramRecursive

//---------------------------------------------------------------------------
void stCountingTree::Print(){
   if (DimensionLessThanFour) {
      CellPrint(1, FirstLevel);
   }else{
      CellPrintHighDimensional(1, FirstLevelHighDimensional);
   }//end if
   printf("----------------------------------\n\n");
}//end stCountingTree::Print

//---------------------------------------------------------------------------
void stCountingTree::CellPrint (int gridLevel, stPairCellLowDimensional *pairCell){
   for (int i=0; i<NumberOfCells; i++){
      printf ("[%d, %d] ", i, pairCell[i].SumOfPoints);
   }//end for
   printf("\n");
   for (int i=0; i<NumberOfCells; i++){
      if (pairCell[i].NextLevel!=0){
         for (int j=0; j<gridLevel; j++) {
            printf("   ");
         }//end for
         printf("%d: ", i);
         CellPrint(gridLevel+1, pairCell[i].NextLevel);
      }//end if
   }//end for
}//end stCountingTree::CellPrint

//---------------------------------------------------------------------------
void stCountingTree::CellPrintHighDimensional (int gridLevel, 
      stPairCellHighDimensional *pairCell){
   while (pairCell != 0){
      PrintOneCellHighDimensional (gridLevel, pairCell); printf ("\n");
      if (pairCell->NextLevel!=0) {
         CellPrintHighDimensional(gridLevel+1, pairCell->NextLevel);
      }//end if
      pairCell=pairCell->NextCell;
   }//end while
}//end stCountingTree::CellPrintHighDimensional

//---------------------------------------------------------------------------
void stCountingTree::PrintOneCellHighDimensional (int gridLevel, 
      stPairCellHighDimensional *pairCell){
   if (pairCell != 0){
      for (int j=0; j<gridLevel; j++) printf ("   ");
      printf ("%d:Ix= ", gridLevel);
      pairCell->index->print();
      printf("\n");
      printf ("\tS=%d ", pairCell->SumOfPoints);
   }//end if
}//end stCountingTree::PrintOneCellHighDimensional

//---------------------------------------------------------------------------
void stCountingTree::ProcessPoint(double *point){
   double * min, * max;
   int * cellId;
   min = new double[NumberOfDimensions];
   max = new double[NumberOfDimensions];
   for (int i=0; i<NumberOfDimensions; i++){
      min[i] = 0;
      max[i] = 1.0;
   }//end for
   if (DimensionLessThanFour) {
      CellLowDimensional_process(0, FirstLevel, min, max, point);
   }else{
      CellHighDimensional_process(0, &FirstLevelHighDimensional, min, max, point);
   }//end if
   delete min;
   delete max;
   delete cellId;
}//end stCountingTree::ProcessPoint
