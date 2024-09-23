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
QuiSquareDistance<ObjectType>::QuiSquareDistance(){
}

/**
* Destructor.
*/
template <class ObjectType>
QuiSquareDistance<ObjectType>::~QuiSquareDistance(){
}

/**
* @deprecated Use getDistance() instead.
*
* @copydoc getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error)
*/
template <class ObjectType>
double QuiSquareDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error) {

    if (obj1.size() != obj2.size()) {
        throw std::length_error("The feature vectors do not have the same size.");
    }

    return getDistance(obj1, obj2);
}

/**
* Calculates the Qui-Square distance between two feature vectors.
* This calculus is based on the math form sum( (feature_1[i]-feature_2[i])^2 / (feature_1[i]+feature_2[i]) ) / 2.
* To make this computations both feature vectors should have the same size().
*
* @param obj1: The first feature vector.
* @param obj2: The second feature vector.
* @throw Exception If the computation is not possible.
* @return The QuiSquare distance between feature vector 1 and feature vector 2.
*/
template <class ObjectType>
double QuiSquareDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    if (obj1.size() != obj2.size()) {
        throw std::length_error("The feature vectors do not have the same size.");
    }

    double d = 0;
    double tmp;

    for (size_t i = 0; i < obj1.size(); i++){
        tmp = obj1[i] + obj2[i];

        if (!(tmp == 0.0)){
            tmp = ((obj1[i] - obj2[i]) * (obj1[i] - obj2[i]))/tmp;
            d = d + tmp;
        }
    }
    d /= (double)2;

    // Statistic support
    this->updateDistanceCount();

    return d;
}
