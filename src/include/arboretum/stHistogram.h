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
#ifndef stHistogramH
#define stHistogramH

#include <vector>

//---------------------------------------------------------------------------
// class stHistogram
//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
class stHistogram {

   public:

      /**
      * Type of the object.
      */
      typedef ObjectType tObject;

      /**
      * Creates a new instance of this class.
      */
      stHistogram(unsigned int numberOfBins = 100);

      /**
      * Destruct of this class.
      */
      ~stHistogram();

      int Size(){
         return NumberOfBins;
      }//end Size

      int GetNumberOfObjects();

      void Build(EvaluatorType * metricEvaluator);

      void Add(ObjectType * obj);

      double GetBin(unsigned int idx);

      double GetValue(unsigned int idx);

      double GetValueOfBin(double value);

   private:

      std::vector <tObject *> Objects;

      double * Values;

      double * Bins;

      unsigned int NumberOfBins;

};//end stHistogram

#include "stHistogram-inl.h"

#endif //end stHistogramH
