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
* This file implements the class stLineFitting.
*
* @version 1.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Rodrigo Nishihara Adão (adao@grad.icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
*/

//#include <arboretum/stLineFitting.h>

//------------------------------------------------------------------------------
// template class stLineFitting

//------------------------------------------------------------------------------
template <class DataType> void stLineFitting<DataType>::stRegionBasedLineFitting(
      int numberOfPoints, DataType * logR, DataType * logSqR, 
      DataType minimumLeastSquaredError) {

   int i, j, k, auxiliar = 0, numberOfPairs, initialNumberOfPoints, maximumCount, maximumIdX;
   stPointSet ** points;
   DataType * slopeError, fittingError, stepError, maximum, minimum, meanSlopeError, minimumFittingError;
   float maximumError, minimumSlope, add;

   numberOfPairs = numberOfPoints-1;  // number of pairs of points
   initialNumberOfPoints = 2;          // initial number of points in each set
   points = new stPointSet *[numberOfPairs];  // set of points  - to fit the line for each stPointSet

   for (i=0; i<numberOfPairs; i++){
      points[i] = new stPointSet(numberOfPoints);
   }//end for

   if (numberOfPairs == 0){
      return;
   }//end if

   minimumSlope = 0.10; // minimum slope considered
   maximum = 0; // used to find maximum slope error
   minimum = MAXDOUBLE;   // used to find minimum slope error
   slopeError = new DataType[numberOfPairs];  // error considering slopes of consecutive fitting lines
   meanSlopeError = 0; // mean slope error
   maximumError = 0.30; // maximum slope error allowed - < 0.30
   stepError = 0;  // step error - used to evaluate slope error in each step of the process
                   // stepError is incremented (by add) in each step of the process
   fittingError = minimumLeastSquaredError; // for fitting error
   minimumFittingError = minimumLeastSquaredError/10; // minimum fitting error

   // fits the line considering all the points
   for (i=numberOfPoints-1, j=0; i>=0; i--, j++) {  // originally, point 0 is the last on the right of the plot
      points[0]->logR[j]=logR[i];     // here, it is in position 0
      points[0]->logSqR[j]=logSqR[i];
   }//end for
   points[0]->firstPoint = 0;
   points[0]->count = numberOfPoints;
   points[0]->lastPoint = numberOfPoints-1;
   points[0]->error=linerror(points[0]->logR, points[0]->logSqR, numberOfPoints, points[0]->Alpha, points[0]->Beta);
   
   if (points[0]->error == MAXDOUBLE){
      points[0]->error = 0;  // the procedure usually returnns maxdouble when
   }//end if
                                                             // the slope is zero - flat region
   maximumIdX = 0;
   // does the iteractive process if the fitting error for all the points is higher than minimumFittingError (=10% of MLSE)
   if (points[0]->error > minimumFittingError) {
      // fits a line considering each pair of sequencial points and gets the
      // corresponding fitting error  - it will be zero if there are only 2 points
      for (i=numberOfPoints-1, j=0; i>0; i--, j++) {  // point 0 is the last on the right of the plot
         auxiliar=i;
         points[j]->firstPoint = j;
         points[j]->count = initialNumberOfPoints;
         for (k=0; k<initialNumberOfPoints; k++, auxiliar--) {
            points[j]->logR[k]=logR[auxiliar];
            points[j]->logSqR[k]=logSqR[auxiliar];
         }//end for
         points[j]->lastPoint = j+1;
         points[j]->error=linerror(points[j]->logR, points[j]->logSqR, initialNumberOfPoints, points[j]->Alpha, points[j]->Beta);
         if (points[j]->error == MAXDOUBLE) {
            points[j]->error = 0;
         }//end if
      }// end for
      auxiliar = 0;
      // gets the error of slopes of consecutive fitting lines
      for (i=1; i<numberOfPairs; i++){
         if (points[i-1]->Alpha < minimumSlope && points[i]->Alpha < minimumSlope) {  // sequence of points with very low slope
            slopeError[i]=0.0;
            // gets the error to quantify how significant is the difference between the slope of consecutive sets of points
         }else{
            slopeError[i]=(fabs(points[i-1]->Alpha - points[i]->Alpha))/(points[i-1]->Alpha + points[i]->Alpha);
            if(slopeError[i] > maximum){
               maximum = slopeError[i];   // gets the highest error
            }//end if
            if (slopeError[i] < minimum){
               minimum = slopeError[i];  // gets the lowest error
            }//end if
            meanSlopeError += slopeError[i];
            auxiliar++;
         }// end if
      }// end for

      // compute initial parameters
      // excludes minimum and maximum slope errors and computes mean slope error (meanSlopeError), which is used to
      // define the initial step error (stepError)
      if (numberOfPairs >= 4 && auxiliar > 2){  // if there is at least three slopeErrors to consider
         meanSlopeError -= (maximum + minimum);
         meanSlopeError /= (auxiliar - 2);
      }else{
         meanSlopeError /= numberOfPairs -1;
      }//end if

      if (meanSlopeError >= maximumError) {
         meanSlopeError = maximumError;  // if mean slope error exceeds the maximum slope error defined
      }//end if
      
      stepError = meanSlopeError/2;  // step error - 50% of mean slope error
      add = stepError/2; // increment of stepError in each step

      // iterative process to joint sets of points whose fitting lines present close slopes
      // the step error (stepError) increases in each step of the process, until it reachs mean error
      while (stepError <= meanSlopeError){
         j = 0;
         i = 1;
         for (k=0; k<numberOfPairs; k++) {
            slopeError[k] = -1;
         }//end for
         while (j<numberOfPairs-1 && i<numberOfPairs) {
            if (points[i]->count > 0){
               if (points[j]->Alpha < minimumSlope && points[i]->Alpha < minimumSlope) { // consecutive sets of points with very
                  slopeError[i]=0.0;                                   // low slope
               }else{
                  slopeError[i]=(fabs(points[j]->Alpha - points[i]->Alpha))/(points[j]->Alpha + points[i]->Alpha);
               }//end if
               
               if (slopeError[i] < stepError){
                  // joins the sets of points and computes the slope of the new fitting line
                  *points[j] += *points[i];
                  points[j]->error = linerror(points[j]->logR, points[j]->logSqR, points[j]->count, points[j]->Alpha, points[j]->Beta);
                  if (points[j]->error == MAXDOUBLE) {
                     points[j]->error = 0;
                  }//end if
                  // if fitting error is ok ...
                  if (points[j]->error < fittingError){
                     points[i]->Clear();
                     slopeError[i] = -1;
                  }else{// if the fitting error is high, undo join
                     *points[j] -= *points[i] ;
                     points[j]->error=linerror(points[j]->logR, points[j]->logSqR, points[j]->count, points[j]->Alpha, points[j]->Beta);
                     if (points[j]->error == MAXDOUBLE) {
                        points[j]->error = 0;
                     }//end if
                     j = i;
                  }//end if
               }else {
                  j = i;
                  i++;
               }//end if
            }else{
               i++;
            }//end if
         }//end while
         i = 1;
         j = 0;
         // updates slopeError after last iteraction
         while (j<numberOfPairs-1 && i<numberOfPairs){
            while (j<numberOfPairs && points[j]->count==0){
               slopeError[i] = -1;
               j++;
               i++;
            }//end while
            while (i<numberOfPairs && points[i]->count==0){
                slopeError[i] = -1;
                i++;
            }//end while
            if (i<numberOfPairs && j<numberOfPairs-1){
               if (points[j]->Alpha < minimumSlope && points[i]->Alpha < minimumSlope) { // consecutive sets of points with very
                  slopeError[i]=0.0;                                   // low slope
               }else{
                  slopeError[i]=(fabs(points[j]->Alpha - points[i]->Alpha))/(points[j]->Alpha + points[i]->Alpha);
               }//end if
               j = i;
               i++;
            }//end if
         }//end while
         // if there is no slopeError > current stepError, stop the process
         i = 0;
         while (i<numberOfPairs && (slopeError[i] < stepError || slopeError[i] >= meanSlopeError)) {
            i++;
         }//end while
         
         if (i == numberOfPairs) {
            stepError = 1; // stop
         }else{
            stepError += add;  // increment step error and repeat...
         }//end if
      }// end while
      maximumCount = 2;
      maximumIdX = -1;
   }// end if

   // gets the line fitting the higher number of points
   // if there is more than one, gets that whose points corresponds to higher randii
   for (i=0; i<numberOfPairs; i++) {
      if (points[i] && points[i]->count >= maximumCount && points[i]->Alpha >= minimumSlope){
         maximumCount = points[i]->count;
         maximumIdX = i;
      }// end if
   }//end for

   // return Alpha, Beta and error
   if (maximumIdX > -1){
      Alpha = points[maximumIdX]->Alpha;
      Beta = points[maximumIdX]->Beta;
      Error = points[maximumIdX]->error;
   }else{
      Alpha = 0.0;
      Beta = 0.0;
      Error = 0.0;
   }//end if

   for (i=0; i<numberOfPairs; i++) {
      delete points[i];
   }//end for
   delete[] points;
   delete[] slopeError;
};//end stFittng::stRegionBasedLineFitting

//---------------------------------------------------------------------------
template <class DataType> void stLineFitting<DataType>::stLeastSquaredFitting(
      int numberOfPoints, DataType *logR, DataType * logSqR, 
      DataType minimumLeastSquaredError, int minimumNumberOfPoints){

   DataType errorMinusBottom, errorMinusTop, fDA, fDB, lineError, auxerr;
   int inf = 0, sup = numberOfPoints;

   /*
    *  errorclip[ bx_, minlse_:0.015, minlen_:5]
    *   Calculate which part of the points we need to keep.
    *   The idea is that we keep the part that looks closer to a line,
    *    - as judged using the mean squared error
    */

   FirstAlpha = -1.0; 
   FirstBeta = -1.0;
   lineError = linerror(logR,logSqR,(sup - inf + 1), fDA, fDB); //receives the error
   auxerr = lineError;
   MinimumAlpha = fDA; 
   MinimumBeta = fDB;

   //while there are points to be adjusted, try to fit the line based on the error
   while (((sup - inf + 1) > minimumNumberOfPoints) && (logSqR[inf + 1] != logSqR[sup - 1])){
      errorMinusBottom = linerror(&logR[inf + 1],&logSqR[inf + 1],sup - inf,fDA,fDB);
      errorMinusTop = linerror(&logR[inf],&logSqR[inf],sup - inf,fDA,fDB);
      //If the minimum bottom error is less than the top one...
      if (errorMinusBottom < errorMinusTop){
         inf++;
         lineError = errorMinusBottom;//lineError will receive the minimum value
      }else{
         sup--;
         lineError = errorMinusTop;//else, will receive the top value
      }//end if
      if (lineError < auxerr){/*If lineError now is less than the auxerr, that received
                                the lineError value previously, update the auxerr value.*/
         auxerr = lineError;
         MinimumAlpha = fDA;
         MinimumBeta = fDB;
      }//end if
      //If the lineError value is less than the error value required...
      if (lineError < minimumLeastSquaredError && FirstAlpha == -1.){
         FirstAlpha = fDA;
         FirstBeta = fDB;
      }//end if
   }//end while
   Error = lineError;
};//end stLineFitting::stLeastSquaredFitting

//---------------------------------------------------------------------------
template <class DataType> DataType stLineFitting<DataType>::linerror(
      DataType * x, DataType * y, int numberOfPoints, 
      DataType & slope, DataType & yIncrement){
   //Computes the error of the line according tho the values in the graph - x and y
   int i;
   DataType sx = 0, sy = 0, sxy = 0, sxx = 0, syy = 0;
   DataType error;
   slope = yIncrement = error = 0.0;
   for (i = 0; i < numberOfPoints; i++){
      sx += x[i];
      sy += y[i];
      sxy += x[i] * y[i];
      sxx += x[i] * x[i];
      syy += y[i] * y[i];
   }//end for
   if (((numberOfPoints * sxx) - (sx * sx)) <= 0 || ((numberOfPoints * syy) - (sy * sy)) <= 0 || (numberOfPoints * sxx - sx * sx) == 0) {
      return MAXDOUBLE;
   }//end if

   //Now, calculates the new values
   slope = (numberOfPoints * sxy - sx * sy) / (numberOfPoints * sxx - sx * sx);
   yIncrement = (sy - slope * sx) / numberOfPoints;
   error = (numberOfPoints * sxy - sx * sy) / (sqrt((numberOfPoints * sxx) - (sx * sx)) * sqrt((numberOfPoints * syy) - (sy * sy)));
   return (1 - fabs(error));
};//end stLineFitting::linerror
