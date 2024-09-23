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
* Constructor.
*/
template <class ObjectType>
DTWDistance<ObjectType>::DTWDistance(){
}

/**
* Destructor.
*/
template <class ObjectType>
DTWDistance<ObjectType>::~DTWDistance(){
}

template <class ObjectType>
double DTWDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    return getDistance(obj1, obj2);
}

/**
* Calculates the Dynamic Time Warping distance between two feature vectors.
*
* @param obj1: The first feature vector.
* @param obj2: The second feature vector.
* @throw Exception If the computation is not possible.
* @return The DTW distance between feature vector 1 and feature vector 2.
*/
template <class ObjectType>
double DTWDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    size_t rows = obj1.size();
    size_t cols = obj2.size();
    double d = 0.0;
    double **dtw; //Matrix to be used.

    // Allocates the memory for matrix M
    dtw = new double*[rows + 1];
    for (size_t i = 0; i < rows + 1; i++){
        dtw[i] = new double[cols + 1];
    }

    // Initialize
    dtw[0][0] = pow(obj1[0] - obj2[0], 2.0);
    // Initialize first col
    for (size_t i = 1; i < rows; i++)
        dtw[i][0] = pow(obj1[i] - obj2[0], 2.0) + dtw[i-1][0];
    // Initialize first row
    for (size_t i = 1; i < cols; i++)
        dtw[0][i] = pow(obj1[0] - obj2[i], 2.0) + dtw[0][i-1];

    // Execute the recurrence
    for (size_t i = 1; i < rows; i++) {
        for (size_t j = 1; j < cols; j++) {
            dtw[i][j] = pow(obj1[i] - obj2[j], 2.0) + fmin(dtw[i-1][j-1], fmin(dtw[i][j-1], dtw[i-1][j]));
        }
    }

    // stores the result
    d = sqrt(dtw[rows-1][cols-1]);

    // Statistic support
    this->updateDistanceCount();

    return d;
}

