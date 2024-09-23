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
template <class ObjectType>
MorositaDistance<ObjectType>::MorositaDistance(){
}

template <class ObjectType>
MorositaDistance<ObjectType>::~MorositaDistance(){
}

template <class ObjectType>
double MorositaDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error) {

    return getDistance(obj1, obj2);
}

template <class ObjectType>
double MorositaDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    if (obj1.size() != obj2.size()) {
        throw std::length_error("The feature vectors do not have the same size.");
    }

    double sumx, sumy, sumxy, lambda1, lambda2;
    double sumaux1, sumaux2, sumaux3, sumaux4;

    sumx = sumy = sumxy = 0.0;
    sumaux1 = sumaux2 = sumaux3 = sumaux4 = 0.0;

    for (size_t x = 0; x < obj1.size(); x++){
        sumaux2 += obj1[x];
        sumaux1 += obj1[x] - 1;
        sumaux3 += obj1[x]*(obj1[x] - 1);
    }
    lambda1 = sumaux3/(sumaux1*sumaux2);

    sumaux1 = sumaux3 = 0.0;
    for (size_t x = 0; x < obj1.size(); x++){
        sumaux4 += obj2[x];
        sumaux1 += obj2[x] - 1;
        sumaux3 += obj2[x]*(obj2[x] - 1);
    }
    lambda2 = sumaux3/(sumaux1*sumaux4);


    if (((lambda1 + lambda2) == 0.0) || (sumaux2 == 0.0) || (sumaux4 == 0.0)){
        return 0;
    } else {
        sumaux1 = 0.0;
        for (size_t x = 0; x < obj1.size(); x++){
            sumaux1 = obj1[x]*obj2[x];
        }
        return 2*(sumaux1/((lambda1 + lambda2)*sumaux2*sumaux4));
    }
}
