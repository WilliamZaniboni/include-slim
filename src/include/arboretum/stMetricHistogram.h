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
#ifndef __STMETRICHISTOGRAM_H
#define __STMETRICHISTOGRAM_H


#include <arboretum/stCommon.h>
#include <arboretum/stCommonIO.h>
#include <arboretum/stObject.h>
#include <stdexcept>

// #include <arboretum/deprecated/stUserLayerUtil.h>

#include <algorithm>
#include <math.h>
#include <assert.h>
//#include <vector.h>

//------------------------------------------------------------------------------
// tMetricHistogramBin Class
//==============================================================================
/**
* This class implements the basic unit of the a Metric Histogram. It has two
* components, the Gray value and the value itself. It also implements the
* operators =, ==, < and >.
*
* @author Fabio Jun Takada Chino
* @author Josiane M. Bueno
* @version 1.0.2001.11.29
* @ingroup metrichisto
*/
class tMetricHistogramBin{
   public:
      /**
      * Creates a new instance of this class.
      */
      tMetricHistogramBin(){
         Gray = 0; // X axis
         Value = 0; // Y axis
      }//end tMetricHistogramBin

      /**
      * Set operator.
      */
      tMetricHistogramBin & operator = (tMetricHistogramBin & src){

         this->Gray = src.Gray;
         this->Value = src.Value;
         return *this;
      }//end operator =

      /**
      * @returns the gray value.
      */
      int GetGray(){
         return Gray;
      }//end GetGray

      /**
      * @returns the value.
      */
      double GetValue(){
         return Value;
      }//end GetValue

      /**
      * @returns the gray value in X.
      */
      int GetX(){
         return Gray;
      }//end GetX

      /**
      * @returns the value in Y.
      */
      double GetY(){
         return Value;
      }//end GetY

      /**
      * Sets the pair value.
      * @param gray
      * @param value
      */
      void Set(int gray, double value){
         Gray = gray;
         Value = value;
      }//end Set

      /**
      * Sets the gray value.
      * @param gray
      */
      void SetGray(int gray){ this->Gray = gray; }

      /**
      * Sets the value.
      * @param v (new value)
      */
      void SetValue(double v){
         this->Value = v;
      }

   protected:

      /**
      * The gray value.
      */
      int Gray;

      /**
      * The value of the bin.
      */
      double Value;
};//end tMetricHistogramBin

//------------------------------------------------------------------------------
// tMetricHistogram Class
//==============================================================================
/**
* This class implements the Metric Histogram.
*
* @author Fabio Jun Takada Chino
* @author Josiane M. Bueno
* @version 1.0.2001.11.29
* @ingroup metrichisto
*/
class tMetricHistogram {

   public:

      /**
      * Creates a new empty metric histogram.
      */
      tMetricHistogram(){
         Serialized = NULL;
         Size = 0;
         Bins = NULL;
         maxBucketLen = 0;
      }//end tMetricHistogram

      /**
      * Creates a new metric histogram from a given initialization data.
      *
      * @param size Number of Bins.
      * @param bins The bins.
      * @return a new metric histogram from a given initialization data
      */
      tMetricHistogram(int size, tMetricHistogramBin * bins);

      /**
      * Creates a new metric histogram from a Serialized data.
      *
      * @param SerializedData The Data serialized by Serialize() function.
      * @warning In order to keep the backward compatibility, this method does
      * not support the tag value even when it is present.
      * @return a new metric histogram.
      */
      tMetricHistogram(unsigned char * SerializedData);

      /**
      * Creates a new metric histogram by coping another metric
      * histogram.
      *
      * @param mhistogram Another metric histogram.
      * @returns a new metric histogram by coping another metric
      * histogram.
      */
      tMetricHistogram(tMetricHistogram & mhistogram);

      /**
      * Disposes this instance and releases all associated resources.
      */
      ~tMetricHistogram(){
         if (Bins != NULL) {
            delete[] Bins;
         }//end if
         if (Serialized != NULL){
            delete [] Serialized;
         }//end if
      }//end ~tMetricHistogram

      /**
      * Returns the number of entries in this mï¿½tric histogram. Since metric
      * histograms have variable number of entries, use this method to
      * @returns the number of entries in this metric histogram.
      */
      int GetSize(){
         return Size;
      }//end GetSize

      /**
      * Gets an entry (bin) of this metric histogram.
      *
      * @param idx The index of the bin.
      * @return The bin pair.
      * @warning Do not forgert to call Refresh() if you use this method to
      * modify the contents of this metric histogram.
      * @see Refresh()
      */
      tMetricHistogramBin & Get(int idx){
         return Bins[idx];
      }//end Get

      /**
      * Dumps the contents of this metric histogram to a stream.
      * <P>It will write a 16 bit integer to tell how many bins will
      * be written. After this, it will write all bins in sequence. Each
      * bin will have a 16 bits integer and a 64 bit floating point.
      *
      * @param out The output stream.
      */
      // void Dump(tDataOutputStream * out); // deletado

      /**
      * Returns the serialized version of this object. This method is required
      * by <i>stObject</i> interface.
      *
      * Serialize this Metric Histogram.
      *
      * <p><code>
      * +-------------------------------------------+--------------+<br>
      * | NBuckets |  bucket[0] | ... | bucket[n-1] |      tag     |<br>
      * |    int   | int|double | ... | int|double  | unsigned int |<br>
      * +-------------------------------------------+--------------+<br>
      * </code>
      * @returns the serialized version of this object.
      */
      const unsigned char * Serialize();

      /**
      * Rebuilds a serialized object.
      *
      * @param data The serialized object.
      * @warning In order to keep the backward compatibility, this method does
      * not support the tag value even when it is present.
      */
      void Unserialize(const unsigned char * data);

      // The following methods are required by the stObject interface.
		/**
      * @copydoc tMetricHistogram::Unserialize()
      *
      * @param SData The serialized data.
      * @param datasize The size of the serialized object in bytes.
      */
      void Unserialize(const unsigned char * data, u_int32_t datasize);

      /**
      * @returns the serialized size.
      */
      u_int32_t GetSerializedSize(){
         return (sizeof(int) + ((sizeof(int) + sizeof(double)) * GetSize()) +
               sizeof(tag));
      }//end GetSerializedSize

      /**
      * Creates a perfect clone of this object. This method is required by
      * stObject interface.
      *
      * @return A new instance of tMetricHistogram wich is a perfect clone
      * of the original instance.
      */
      tMetricHistogram * Clone(){
         return new tMetricHistogram(*this);
      }//end Clone

      /**
      * Checks to see if this object is equal to other. This method is required
      * by stObject interface.
      *
      * @param obj Another instance of tMetricHistogram.
      * @return True if they are equal or false otherwise.
      */
      bool IsEqual(tMetricHistogram * obj);

      /**
      * Return the tag associated with this metric histogram. This value can be
      * used to associate this metric histogram to a data base entry.
      *
      * @return the tag associated with this metric histogram.
      */
      u_int32_t GetTag(){ return tag; }

      /**
      * Sets the tag associated with this metric histogram. This value can be
      * used to associate this metric histogram to a data base entry.
      *
      * @param tag The tag value.
      */
      void SetTag(u_int32_t tag){ this->tag = tag;}

      /**
      * Refreshes all internal buffers.
      */
      void Refresh();
      
   private:
   
      /**
      * Number of Bins in the metric histogram.
      */
      int Size;

      /**
      * The array of Bins of the metric histogram.
      */
      tMetricHistogramBin * Bins;

      /**
      * Zero threshold. Used by Signal.
      */
      double zeroThres;

      /**
      * Tangent threshold. Used by AcceptTan.
      */
      double tanThres;

      /**
      * Maximum size of a bucket.
      */
      int maxBucketLen;

      /**
      * This array holds the serialized version of this histogram.
      */
      unsigned char * Serialized;

      /**
      * Tag value. It may be used to associate this metric histogram to a
      * data base entry.
      */
      u_int32_t tag;

      /**
      * Returns the signal of a given value. Any value between -zeroThres and
      * zeroThres is computed as zero.
      *
      * param value The value to be verified.
      * return 0 for zero, 1 for positive and -1 for negative.
      */
      int GetSignal(double value){

         if (fabs(value) < zeroThres){
            return 0;
         }else if (value < 0){
            return -1;
         }else{
            return 1;
         }//end if
      }//end GetSignal

      /**
      * Tests to see if we will accept a given tangent. The tangent
      * will be accepted if the difference between angles exceeds
      * the tanThres. The angles are mesuared in radians.
      *
      * param l
      * param r
      */
      bool AcceptTan(double l, double r){
         double a = fabs(atan(l) - atan(r));

         if (a > M_PI){
            a = 2 * M_PI - a ;
         }//end if

         return (a < tanThres);
         //return (fabs(l - r) > tanThres);
      }//end AcceptTan

      /**
      * Metric histogram generator version 1.
      *
      * warning This method will be deprecated soon.
      */
      // void ProcessHistogramOld(tHistogram & src); // deletado

      /**
      * Creates a metric histogram from a given histogram.
      * This implementation has been changed to avoid work with no variation
      * of brightness. This is not the original aproach of processing histogram.
      */
      // void ProcessHistogram(tHistogram & src); // deletado

};//end tMetricHistogram

//------------------------------------------------------------------------------
// tMetricHistogramEvaluator Class
//==============================================================================
/**
* This class implements the Metric Evaluator.
*
* @author Fabio Jun Takada Chino
* @author Josiane M. Bueno
* @version 1.0.2001.11.29
* @ingroup metrichisto
*/
class tMetricHistogramEvaluator : public stMetricEvaluatorStatistics{

   public:

      /**
      * Returns the distance between 2 metricHistogram. This method is
      * required by stMetricEvaluator interface in stMetricTree.
      *
      * @param one Histogram 1.
      * @param other Histogram 2.
      * @return The distance between 2 metricHistogram.
      */
      double GetDistance(tMetricHistogram * one, tMetricHistogram * other){
         // Update Statistics.
         UpdateDistanceCount();
         // Call MetricDistance.
         double d = GetMetricDistance(one, other);
         return d;
      }//end GetDistance

      double GetDistance2(tMetricHistogram * one, tMetricHistogram * other){
         double d = GetDistance(one, other);
         return d * d;
      }//end GetDistance2

      /**
      * Compares two histograms and return the DM distance between them.
      *
      * @param one Histogram 1.
      * @param other Histogram 2.
      * @return The metric distance between them.
      */
      double GetMetricDistance(tMetricHistogram * one, tMetricHistogram * other);

      /**
      *
      * @param one Histogram 1.
      * @param other Histogram 2.
      * @return The Warp Distance.
      */
      double GetWarpDistance(tMetricHistogram * one, tMetricHistogram * other);

};//end tMetricHistogramEvaluator


//----------------------------------------------------------------------------
// Class template dlGenericMatrix
//----------------------------------------------------------------------------
/**
* This class template implements a generic nxm matrix.
*
* @author Fabio Jun Takada Chino
* @version 1.0
* @ingroup util
*/
template < class Type > class dlGenericMatrix{

   public:

      /**
      * Creates a new Matrix with 0 cols and 0 rows. Use SetSize() to change the
      * size of this matrix.
      */
      dlGenericMatrix(){
         Rows = 0;
         Cols = 0;
         Data = NULL;
      }//end stGenericMatrix

      /**
      * Creates a new matrix with a given size.
      *
      * @param col Columns.
      * @param row Rows.
      */
      dlGenericMatrix(int cols, int rows){
         Data = NULL;
         SetSize(cols, rows);
      }//end dlGenericMatrix

      /**
      * Disposes this matrix.
      */
      ~dlGenericMatrix(){

         if (Data != NULL){
            delete [] Data;
            delete [] PRows;
         }//end if
      }//end ~dlGenericMatrix

      /**
      * Sets the size of this matrix. All stored data will be lost!
      *
      * @param col Columns.
      * @param row Rows.
      */
      void SetSize(int cols, int rows);

      /**
      * Returns the number of rows.
      *
      * @return The number of rows.
      */
      int GetRows(){
         return Rows;
      }//end GetRows

      /**
      * Returns the number of columns.
      *
      * @return Returns the number of Columns.
      */
      int GetCols(){
         return Cols;
      }//end GetCols

      /**
      * Return a position of this matrix.
      *
      * @param col Column.
      * @param row Row.
      * @return Position of this matrix
      */
      Type & Get(int row, int col){
         return (*this)[row][col];
      }//end Get

      /**
      * This operator returns a row of this matrix. You may
      * use this operator as matrix[row][col].
      *
      * @param row Row.
      * @return Position of the row of this matrix
      */
      Type * operator [] (int row){
         return PRows[row];
      }//end operator []

   private:
   
      /**
      * Number of rows.
      */
      int Rows;

      /**
      * Number of columns.
      */
      int Cols;

      /**
      * Data vector.
      */
      Type * Data;

      /**
      * Speed up pointers.
      */
      Type ** PRows;
};//end dlGenericMatrix

//----------------------------------------------------------------------------
// Class template dlHugeGenericMatrix
//----------------------------------------------------------------------------
/**
* This class template implements is a variant of dlGenericMatrix which is able
* to store a very large matrix.
*
* @author Fabio Jun Takada Chino
* @version 1.0
* @ingroup util
*/
template < class Type > class dlHugeGenericMatrix{

   public:

      /**
      * Creates a new Matrix with 0 cols and 0 rows. Use SetSize() to change the
      * size of this matrix.
      */
      dlHugeGenericMatrix(){
         Rows = 0;
         Cols = 0;
         PRows = NULL;
      }//end dlHugeGenericMatrix

      /**
      * Creates a new matrix with a given size.
      *
      * @param col Columns.
      * @param row Rows.
      */
      dlHugeGenericMatrix(int cols, int rows){
         PRows = NULL;
         SetSize(cols, rows);
      }//end dlHugeGenericMatrix

      /**
      * Disposes this matrix.
      */
      virtual ~dlHugeGenericMatrix();

      /**
      * Sets the size of this matrix. All stored data will be lost!
      *
      * @param col Columns.
      * @param row Rows.
      */
      void SetSize(int cols, int rows);

      /**
      * Returns the number of rows.
      *
      * @return The number of Rows
      */
      int GetRows(){
         return Rows;
      }//end GetRows

      /**
      * Returns the number of Columns.
      *
      * @return The number of Columns
      */
      int GetCols(){
         return Cols;
      }//end GetCols

      /**
      * Return a position of this matrix.
      *
      * @param col Column.
      * @param row Row.
      */
      Type & Get(int row, int col){
         return (*this)[row][col];
      }//end Get

      /**
      * This operator returns a row of this matrix. You may
      * use this operator as matrix[row][col].
      *
      * @param row Row.
      */
      Type * operator [] (int row){
         return PRows[row];
      }//end operator []

   private:

      /**
      * Number of rows.
      */
      int Rows;

      /**
      * Number of columns.
      */
      int Cols;

      /**
      * Data pointers.
      */
      Type ** PRows;

      /**
      * Disposes all rows.
      */
      void DisposeRows();
};//end dlHugeGenericMatrix

// Include implementation
#include "stMetricHistogram-inl.h"

#endif //end __STMETRICHISTOGRAM_H
