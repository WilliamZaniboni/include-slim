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
* This file implements the class stFractalDimension.
*
* @version 1.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Rodrigo Nishihara Adão (adao@grad.icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
* @ingroup fastmap
*/

//#include "stFractalDimension.h"

//------------------------------------------------------------------------------
// template class stFractalDimension
//------------------------------------------------------------------------------
template <class DataType, class ObjectType> stFractalDimension<DataType,ObjectType>::stFractalDimension
        (int numberOfObjects, ObjectType ** objectsArray, TRange q, int generalized,
        int dimension, int numberOfPointsInInterval, int pointSelect,
        int normalizeFactor, int fittingAlgorithmFactor, double xInterval[30],
        double yInterval[30], unsigned int * mask, struct stObjects * attributes){
   NormalizeGeneratedPlots = false;
   NormalizeGeneralizedPlots = 0;
   DefScale = ScaleLogLog;

   /**
    * @todo Slow Calculation will be fixed later.
    */
   Fastcalculation = true;
   if (Fastcalculation == true) {
      SetFastDE = 1;
      NumberOfGridLevels = 20;
   }else{
      SetFastDE = 0;
      //NumberOfGridLevels = DefMDESlots();
   }//end if

   ProcessDE(numberOfObjects, objectsArray, q, false, dimension, normalizeFactor,
             xInterval, yInterval, fittingAlgorithmFactor, numberOfPointsInInterval,
             pointSelect, mask, attributes);
};//end stFractalDimension::stFractalDimension
//---------------------------------------------------------------------------
template <class DataType, class ObjectType> void stFractalDimension<DataType,ObjectType>::ProcessDE
        (int numberOfObjects, ObjectType ** objectsArray, TRange q,
        int generalized, int dimension, int normalizeFactor, double xInterval[30],
        double yInterval[30], int fittingAlgorithmFactor, int numberOfPointsInInterval,
        int pointSelect, unsigned int * mask, struct stObjects * attributes){

   Huge_int ndistcal_loc;
   DataType minD, MaxD;
   int minimuNumberOfPoints, no1Aux, auxNvc, i;
   DataType fracMin, bMin, err, constant;
   double * arrayX, * arrayY;
   double * logR, * logSqR;
   Huge_int * vetCount;

   fracMin = 0;
   bMin = 0;

   double minimumLeastSquaredError = 0.015;
   double minimumLenght = 5;
   int topAligned = false;
   double topAlignedFactor = 0;

   int countAttributes=0;
   for (i=0; i<dimension; i++) {
      if (mask[i]==1) {
         countAttributes++; //counting the number of attributes to be consideres in each iteration
      }//end if
   }//end for

   //for (int k=0; k<run; k++){

   //ClockStart=clock();
   //ndistcal_loc=ndistcal;

   //ndistcal_loc = ndistcal;

   if (SetFastDE == 0){ // Old lines to do Slow calculation
      /*vetCount= (Huge_int *) malloc ((1+NumberOfGridLevels)*sizeof(Huge_int));
      arrayX= (double *) malloc ((1+NumberOfGridLevels)*sizeof(double));
      arrayY = (double *)malloc (sizeof(double)*(NumberOfGridLevels+1));
      //SumDistExponent(numberOfObjects,&objectsArray,,&minD,&MaxD,arrayX,vetCount);
      SumDistExponent(numberOfObjects,&(*(objectsArray)),&minD,&MaxD,arrayX,vetCount);
      arrayY[0] = double(vetCount[0]);
      for (i = 1; i <= NumberOfGridLevels; i++)
         arrayY[i] = arrayY[i - 1] + double(vetCount[i]);
	 if (NormalizeGeneratedPlots) // Normalize every generated plots to y=<0:1>
	    constant = 1. / log(arrayY[0]);
	 else
	    constant = 1.; // Do not normalize*/
      ShowMessage("Slow Calculation");
   }
   else{ // fast calculation
      arrayX = (double *)calloc ((1 + NumberOfGridLevels),sizeof(double));
      arrayY = (double *)calloc ((2 + NumberOfGridLevels),sizeof(double));

      // Construct the List
      u_int32_tingTree CalcTree(countAttributes, NumberOfGridLevels);
      FastDistExponent(numberOfObjects, objectsArray, &CalcTree, arrayX, dimension,
                       normalizeFactor,mask,countAttributes, attributes);
      GenerateBOPS(q,generalized,&CalcTree,constant, arrayY);

      //NPGrid = 0; // Once written, this archive is done. /***NPGrid is now in FastDistExponent, where it´s been used***/
   }//end if
   logR = (double *)calloc ((1 + NumberOfGridLevels),sizeof(double));
   logSqR = (double *)calloc ((2 + NumberOfGridLevels),sizeof(double));

   /*If auxNvc doesn´t receive 0, in the next loop, if the condition were not
   satisfied, auxNvc would be "garbage".*/
   auxNvc=0;

//Carol 13Jul - substituido: for (i = 1; i < NumberOfGridLevels; i++){
   for (i = 0; i <= NumberOfGridLevels; i++){
      if (arrayY[i] != arrayY[i - 1]){
         auxNvc = i;
      }//end if
   }//end for

   if (minimumLenght < NumberOfGridLevels * 0.05) {
      minimuNumberOfPoints = NumberOfGridLevels * 0.05;
   }else{
      minimuNumberOfPoints = minimumLenght;
   }//end if

   ToLog(auxNvc,constant,logR,logSqR,arrayX,arrayY); //Converts the values to log values

   /*FILE *teste = fopen("TesteCarolFastMap.txt","a");
        fprintf(teste,"\n*****************************************\n");
        for (int h=0;h<=NumberOfGridLevels;h++)
           fprintf(teste," %f - %f \n",logR[h],logSqR[h]);
        fclose (teste);*/


   if (topAligned && topAlignedFactor != 0.0) {
      float shift;
      if (SetFastDE == 0) { //slow calculation
         shift = topAlignedFactor - logSqR[NumberOfGridLevels - 1];
      }else{ //fast calculation
         shift = topAlignedFactor - logSqR[0];
      }//end if
      for (int i = 0; i <= auxNvc; i++) {
         logSqR[i] += shift;
      }//end for
   }//end if

   //Fitting options act here.
   if(pointSelect == 1 && fittingAlgorithmFactor==0){ //Least square fitting
      sFit.stLeastSquaredFitting(auxNvc, logR, logSqR, minimumLeastSquaredError, minimuNumberOfPoints);
      fracMin = sFit.GetMinimumAlpha();
      bMin = sFit.GetMinimumBeta();
      Alpha = fracMin;
      Beta = bMin;
   }else if(pointSelect == 0 && fittingAlgorithmFactor==0) { //Least square fitting
      sFit.stLeastSquaredFitting(auxNvc,logR,logSqR,minimumLeastSquaredError,minimuNumberOfPoints);
      Alpha = sFit.GetMinimumAlpha();
      bMin = sFit.GetMinimumBeta();
      pointSelect = 1;
   }//end if

   if(pointSelect == 1 && fittingAlgorithmFactor==1) { //Region based line fitting
      sFit.stRegionBasedLineFitting (auxNvc+1,logR,logSqR,minimumLeastSquaredError);
      //why auxNvc+1?Because auxNvc is the position of the last point, and the method requires the number of points
      Alpha = sFit.GetAlpha();
      Beta = sFit.GetBeta();
   }else if(pointSelect == 0 && fittingAlgorithmFactor==1) { //Region based line fitting
      sFit.stRegionBasedLineFitting (numberOfPointsInInterval+1,xInterval,yInterval,minimumLeastSquaredError);
      //why numberOfPointsInInterval+1?Because numberOfPointsInInterval is the position of the last point, and the method requires the number of points
      Alpha = sFit.GetAlpha();
      Beta = sFit.GetBeta();
      pointSelect = 1;
   }//end if

   if (SetFastDE == 0) {
      delete[]vetCount;
   }//end if
      
   delete[] logR;
   delete[] logSqR;
   delete[] arrayX;
   delete[] arrayY;
};//end stFractalDimension::ProcessDE

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> int stFractalDimension<DataType,ObjectType>::FastDistExponent(
        int numberOfObjects, ObjectType ** objectsArray, u_int32_tingTree * calcTree,
        double * arrayX, int dimension, int normalizeFactor, unsigned int * mask,
        int countAttributes, struct stObjects * attributes){
           
   int i, j, no1Aux, attribute; //attribute is the index to manipulate the ordered vector of attributes.
   DataType * minD, * maxD, biggest;
   DataType * onePoint, * resultPoint, * a, * b; // y=Ax+B to normalize each dataset.
   char hgFile[120];
   double NormalizationFactor = 1.0;

   NPGrid = 0; // Once written, this archive is done.

   minD = (DataType *) calloc ((1+countAttributes),sizeof(DataType));
   maxD = (DataType *) calloc ((1+countAttributes),sizeof(DataType));
   a = (DataType *) calloc(countAttributes,sizeof(DataType));
   b = (DataType *) calloc(countAttributes,sizeof(DataType));
   onePoint = (DataType *) calloc(countAttributes,sizeof(DataType));
   resultPoint = (DataType *) calloc(countAttributes,sizeof(DataType));

   no1Aux = 0;

   // Normalize the data
   MinMax(numberOfObjects, objectsArray, minD, maxD, dimension, mask, countAttributes, attributes);
   biggest = 0; // for Normalize==0, 1 or 3
   // Normalise=0->Independent, =1->mantain proportion, =2->Clip
   //          =3->Geo Referenced
   if (normalizeFactor == 2) {
      biggest = MAXDOUBLE;
   }//end if

   for (i = 0; i < countAttributes; i++) {
      a[i] = (maxD[i] - minD[i]) * NormalizationFactor; //A takes the range of each dimension
      b[i] = minD[i];
      if (a[i] == 0) {
         a[i] = 1;
      }//end if
   }//end for

   for (i = 0; i < countAttributes; i++) {
      if ((normalizeFactor < 2 || normalizeFactor == 3) && biggest < a[i]) {
         biggest = a[i];
      }//end if
      if (normalizeFactor == 2 && biggest > a[i]) {
         biggest = a[i];
      }//end if
   }//end for

   if (normalizeFactor != 0) {
      for (int i = 0; i < countAttributes; i++) {
         a[i] = biggest; // normalized keeping proportion
      }//end for
      /* When we have the proportional normalization, every A[i] are gonna receive
      the biggest range.*/
   }//end if

   if (normalizeFactor >=0) {
      calcTree->Normalize(a,b); // if there is some normalization
   }//end if

   // Process each point
   no1Aux = 0;

   int l;
   //Process each point of objectArray
   for (i = 0; i < numberOfObjects; i++){
      j = 0;
      l = 0;
      while (j < countAttributes && l < dimension) {
         /*This process was adapted to deal with orderly vectors, because the
         ordinance is stored in "attributes", but objectsArray continues disordered.*/
         attribute = attributes[l].attribute;
         if (mask[l]!=0) {
            onePoint[j] = ((FmdbDoubleType *)objectsArray[i]->GetAttribute(attribute))->data;
            j++;
            l++;
         }else{
            l++;
         }//end if
      }//end while
      if (calcTree->Point(onePoint,resultPoint)) {//And add to the grid structure
         no1Aux++;
      }//end if
   }//end for
   
   if (NPGrid > 0){
      calcTree->Histogram(biggest,NPGrid,hgFile);
   }//end if

   // now the calcTree structure has the values for all points in both datasets.
   // Calculate the Xarray vector.
   for (int i = 0; i <= NumberOfGridLevels; i++) {
      arrayX[i] = biggest / pow(2.,1 + i);
   }//end for

   delete[] onePoint;
   delete[] resultPoint;
   delete[] a;
   delete[] b;
   delete[] minD;
   delete[] maxD;
   return no1Aux;
};//end stFractalDimension::FastDistExponent

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> double stFractalDimension<DataType,ObjectType>::GenerateBOPS(
      TRange q, int generalized, u_int32_tingTree * calcTree, double & constant, double * arrayY){
   double total = 0, top;
   int * numPoints;
   numPoints = (int *) calloc((NumberOfGridLevels+2),sizeof(int));

   //Based on the normalization factor, do the count of points in each grid cell
   if(generalized) {
      calcTree->GenericCount(numPoints,arrayY,false,q.max,top);
   }else {
      calcTree->Count(numPoints,arrayY,false);
   }//end if

   if(NormalizeGeneratedPlots) { // Normalize every generated plots to y=<0:1>
      constant = 1.0 / log(arrayY[0]);
   }else{ // Normalize only generalized plots to the one withp=q=1 ?
      if(generalized && NormalizeGeneralizedPlots == 0) { // yes
         constant = log(top) / log(arrayY[0]);
      }else{
         constant = SelfConstant(q.max); // No. Use Log or Sum instead
      }//end if
   }//end if
   for (int i = 0; i <= NumberOfGridLevels; i++){
      total += arrayY[i];
   }//end for

   delete[] numPoints;
   return total;
};//end stFractalDimension::GenerateBOPS

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> void stFractalDimension<DataType,ObjectType>::ToLog(
        int numberOfObjects, double constant, double * destX, double * destY,
        double * origX, double * origY){
   //If the scale of the graph is log-log...
   if (DefScale == ScaleLogLog) {
      for (int i = 0; i <= numberOfObjects; i++) {
         if (origX[i] != 0){
            destX[i] = log(origX[i]); 
         }else {
            destX[i] = 0;
         }//end if
         if (origY[i] != 0) {
   	    destY[i] = log(origY[i]);
         }else{
   	    destY[i] = 0;
	 }//end if
	 destY[i] *= constant;
      }//end for
   }else{
      for (int i = 0; i <= numberOfObjects; i++){
         destX[i] = origX[i];
         destY[i] = origY[i];
      }//end for
   }//end if
};//end stFractalDimension::ToLog

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> void stFractalDimension<DataType,ObjectType>::MinMax(
        int numberOfObjects, ObjectType**objectsArray, DataType * min,
        DataType * max, int dimension, unsigned int * mask, int countAttributes,
        struct stObjects * attributes){
   int i, j, l, attribute;
   for (j = 0; j < countAttributes; j++){ //set the values to the minimum/maximum possible here
      min[j] = MAXDOUBLE;
      max[j] = -MAXDOUBLE;
   }//end for

   //Looking for the minimum and maximum values
   for (i = 0; i < numberOfObjects; i++){
      j = 0; 
      l = 0;
      while (j<countAttributes && l<dimension){
         if (mask[l]==1){
            attribute = attributes[l].attribute; //Dealing with the organized vector.
            if (((FmdbDoubleType *)objectsArray[i]->GetAttribute(attribute))->data < min[j]) {
	       min[j] = ((FmdbDoubleType *)objectsArray[i]->GetAttribute(attribute))->data;
            }//end if
            if (((FmdbDoubleType *)objectsArray[i]->GetAttribute(attribute))->data > max[j]) {
	       max[j] = ((FmdbDoubleType *)objectsArray[i]->GetAttribute(attribute))->data;
            }//end if
            j++;
            l++;
         }else{
            l++;
         }//end if
      }//end while
   }//end for
};//end stFractalDimension::MinMax

//---------------------------------------------------------------------------
template <class DataType, class ObjectType>
double stFractalDimension<DataType,ObjectType>::SelfConstant(double q){
   if (NormalizeGeneralizedPlots == 1){ // Sum
      if (q != 1.) {
         return 1.0 / (q - 1.0);
      }else{
         return 1.0;
      }//end if
   }else { // Log
      if (q != 1.0){
         return 1.0 / (q - 1.0);
      }else{
         return 1.0;
      }//end if
   }//end if
};//end stFractalDimension::SelfConstant
