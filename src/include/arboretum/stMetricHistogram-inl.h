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
* This file defines a generic array object that implements all methods required
* by the stObject interface. This object may be used in combination with the
* metric evaluators defined in the file stBasicMetricEvaluator.h.
*
* @version 2.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/

//----------------------------------------------------------------------------
// Class template dlGenericMatrix
//----------------------------------------------------------------------------
template < class Type >
void dlGenericMatrix<Type>::SetSize(int cols, int rows){
   int w;

 	if (Data != NULL){
      delete [] Data;
    	delete [] PRows;
   }//end if

   Rows = rows;
   Cols = cols;
   Data = new Type[cols * rows];
   PRows = new Type * [rows];
   PRows[0] = Data;
   for (w = 1; w < rows; w++){
      PRows[w] = PRows[w - 1] + cols;
   }//end for
}//end dlGenericMatrix<Type>::SetSize

//----------------------------------------------------------------------------
// Class template dlGenericMatrix
//----------------------------------------------------------------------------
template < class Type >
dlHugeGenericMatrix<Type>::~dlHugeGenericMatrix(){

   DisposeRows();
}//end stHugeGenericMatrix<Type>::~stHugeGenericMatrix

//----------------------------------------------------------------------------
template < class Type >
void dlHugeGenericMatrix<Type>::SetSize(int cols, int rows){
   int w;

   if ((cols != Cols) || (rows != Rows)){
      // Kill previows matrix
      DisposeRows();

      Rows = rows;
      Cols = cols;
      PRows = new Type * [rows];
      for (w = 0; w < rows; w++){
         PRows[w] = new Type[cols];
      }//end for      
   //}else{
      // Do nothing. He he he...
   }//end if
}//end dlGenericMatrix<Type>::SetSize

//----------------------------------------------------------------------------
template < class Type >
void dlHugeGenericMatrix<Type>::DisposeRows(){
   Type ** i;

 	if (PRows != NULL){
      for (i = PRows; i < PRows + Rows; i++){
         delete [] *i;
      }//end for
      delete [] PRows;
   }//end if
}//end dlGenericMatrix<Type>::DisposeRows 
