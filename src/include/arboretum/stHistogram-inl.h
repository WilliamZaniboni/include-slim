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
//---------------------------------------------------------------------------
// stHistogram.h ...
//
// Author: Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
//---------------------------------------------------------------------------
#include "stHistogram.h"

//---------------------------------------------------------------------------
// class stHistogram
//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stHistogram<ObjectType, EvaluatorType>::stHistogram(unsigned int numberOfBins){
   NumberOfBins = numberOfBins;
   Values = new double [NumberOfBins];
   Bins = new double [NumberOfBins];
}//end stHistogram

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stHistogram<ObjectType, EvaluatorType>::~stHistogram(){
   // Delete the resources.
   delete[] Values;
   delete[] Bins;
   // Delete the objects.
   for (unsigned int i = 0; i < Objects.size(); i++){
      delete Objects.at(i);
   }//end for
}//end ~stHistogram

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stHistogram<ObjectType, EvaluatorType>::GetNumberOfObjects(){
   return Objects.size();
}//end GetNumberOfObjects

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stHistogram<ObjectType, EvaluatorType>::Build(EvaluatorType * metricEvaluator){
   unsigned int i, j, k;
   double maxDist, minDist, incDist;
   double ** dist;
   double tempDist;
   double count;
   unsigned int numberOfObjects;

   numberOfObjects = Objects.size();
   maxDist = 0;
   minDist = MAXDOUBLE;

   if (numberOfObjects > 0){
      // Allocate the matrix.
      dist = new double * [numberOfObjects];
      for (i = 0; i < numberOfObjects; i++){
         dist[i] = new double[numberOfObjects];
      }//end for

      // Build the distance matrix.
      for (i = 0; i < numberOfObjects; i++){
         // diagonal.
         dist[i][i] = 0;
         for (j = i + 1; j < numberOfObjects; j++){
            dist[i][j] = metricEvaluator->GetDistance(Objects[i], Objects[j]);
            dist[j][i] = dist[i][j];
            if (dist[i][j] > maxDist){
               maxDist = dist[i][j];
            }//end if
            if (dist[i][j] < minDist){
               minDist = dist[i][j];
            }//end if
         }//end for
      }//end for

      incDist = (maxDist - minDist)/this->NumberOfBins;

      // test if there is more than one object to build the matrix of distances.
      if (numberOfObjects > 2){
         // To measure the bins.
         for (i = 0; i < this->NumberOfBins; i++){
            count = 0;
            Bins[i] = minDist + incDist;
            minDist = Bins[i];
            for (j = 0; j < numberOfObjects - 1; j++){
               for (k = j + 1; k < numberOfObjects; k++){
                  if (dist[j][k] <= Bins[i]){
                     count++;
                  }//end if
               }//end for
            }//end for
            Values[i] = (double )count / (numberOfObjects * (numberOfObjects - 1) / 2.0);
         }//end for
      }else{
         for (i = 0; i < this->NumberOfBins; i++){
            Values[i] = 1;
            Bins[i] = maxDist;
         }//end for
      }//end if

      // clean.
      for (i = 0; i < numberOfObjects; i++){
         delete dist[i];
      }//end for
      delete[] dist;
   }else{
      //There is not any objects.
      for (i = 0; i < this->NumberOfBins; i++){
         Values[i] = 0;
         Bins[i] = MAXDOUBLE;
      }//end for
   }//end if
}//end Build

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stHistogram<ObjectType, EvaluatorType>::Add(ObjectType * obj){
   // Add a new object.
   Objects.insert(Objects.end(), obj->Clone());
}//end Add

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double stHistogram<ObjectType, EvaluatorType>::GetBin(unsigned int idx){
   if (idx < NumberOfBins){
      return Bins[idx];
   }//end if
   return 0;
}//end GetBin

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double stHistogram<ObjectType, EvaluatorType>::GetValue(unsigned int idx){
   if (idx < NumberOfBins){
      return Values[idx];
   }//end if
   return 0;
}//end GetValue

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double stHistogram<ObjectType, EvaluatorType>::GetValueOfBin(double value){
   unsigned int idx;

   idx = 0;
   while (idx < NumberOfBins){
      if (value < Bins[idx]){
         return Values[idx];
      }//end if
      idx++;
   }//end while
   return Values[NumberOfBins-1];
}//end GetValue
