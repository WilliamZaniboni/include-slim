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
LEditDistance<ObjectType>::LEditDistance(){
}

/**
* Destructor.
*/
template <class ObjectType>
LEditDistance<ObjectType>::~LEditDistance(){
}

/**
* @deprecated Use getDistance(ObjectType &obj1, ObjectType &obj2) instead.
*
* @copydoc getDistance(ObjectType &obj1, ObjectType &obj2) .
*/
template <class ObjectType>
double LEditDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    return getDistance(obj1, obj2);
}
/**
* Calculates the LEdit distance between two words.
* This calculus is based in the number of the letters changed.
*
* @param obj1: The first word.
* @param obj2: The second word.
* @return The LEdit distance between feature word 1 and word 2.
*/
template <class ObjectType>
double LEditDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    char aux1, aux2;
    unsigned int cost;
    int **diff;

    diff = new int*[obj2.size() + 1];
    for (int i = 0; i < obj2.size() + 1; i++)
        diff[i] = new int[obj1.size() + 1];

    // Initialize
    diff[0][0] = 0;
    for (size_t i = 1; i <= obj1.size(); i++){
        diff[i][0] = i;
    }//end for
    for (size_t j = 1; j <= obj2.size(); j++){
        diff[0][j] = j;
    }//end for

    for (size_t i = 1; i <= obj1.size(); i++){
        for (size_t j = 1; j <= obj2.size(); j++){
            // Cost
            aux1 = obj1[i - 1];
            aux2 = obj2[j - 1];
            if ((aux1) == (aux2))
                cost = 0;
            else
                cost = 1;

            diff[i][j] = std::min(std::min(diff[i - 1][j] + 1, diff[i][j - 1] + 1),diff[i - 1][j - 1] + (int32_t)cost);
        }//end for
    }//end for


    this->updateDistanceCount();

    return diff[obj1.size()][obj2.size()];
}
