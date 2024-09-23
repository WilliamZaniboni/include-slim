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
MetricHistogramDistance<ObjectType>::MetricHistogramDistance() {
}

/**
* Destructor.
*/
template <class ObjectType>
MetricHistogramDistance<ObjectType>::~MetricHistogramDistance() {
}

/**
* @deprecated Use getDistance(ObjectType &obj1, ObjectType &obj2) instead.
*
* @copydoc getDistance(ObjectType &obj1, ObjectType &obj2) .
*/
template <class ObjectType>
double MetricHistogramDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    return getDistance(obj1, obj2);
}

/**
* Calculates similariry between the two metric histograms.
* This calculus is based on the histograms.
*
* @param obj1: The first feature vector.
* @param obj2: The second feature vector.
* @return The distance between feature vector 1 and feature vector 2.
*/
template <class ObjectType>
double MetricHistogramDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    double *new_obj1_gray  = new double[(obj1.size()/2)];
    double *new_obj1_value = new double[(obj1.size()/2)];

    double *new_obj2_gray  = new double[(obj2.size()/2)];
    double *new_obj2_value = new double[(obj2.size()/2)];

    for (size_t x = 0; x < (obj1.size()/2); x++){
        new_obj1_gray[x] = obj1[x];
        new_obj1_value[x] = obj1[(obj1.size()/2) + x];
    }

    for (size_t x = 0; x < (obj2.size()/2); x++){
        new_obj2_gray[x] = obj2[x];
        new_obj2_value[x] = obj2[(obj2.size()/2) + x];
    }

    double d1, d2;
    double m;
    d1 = d2 = 0.0;

    for (size_t x = 0; x < ((obj1.size()/2)-1); x++){
        m = (new_obj1_value[x+1] - new_obj1_value[x])/(new_obj1_gray[x+1] - new_obj1_gray[x]);
        d1 += ((m*new_obj1_gray[x+1]*new_obj1_gray[x+1])/2.0) + (new_obj1_value[x]*new_obj1_gray[x+1]);
        d1 -= ((m*new_obj1_gray[x]*new_obj1_gray[x])/2.0) + (new_obj1_value[x]*new_obj1_gray[x]);
    }

    for (size_t x = 0; x < ((obj2.size()/2)-1); x++){
        m = (new_obj2_value[x+1] - new_obj2_value[x])/(new_obj2_gray[x+1] - new_obj2_gray[x]);
        d2 += ((m*new_obj2_gray[x+1]*new_obj2_gray[x+1])/2.0) + (new_obj2_value[x]*new_obj2_gray[x+1]);
        d2 -= ((m*new_obj2_gray[x]*new_obj2_gray[x])/2.0) + (new_obj2_value[x]*new_obj2_gray[x]);
    }

    this->updateDistanceCount();

    return fabs(d1 - d2);
}
