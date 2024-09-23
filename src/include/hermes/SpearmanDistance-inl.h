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
SpearmanDistance<ObjectType>::SpearmanDistance(){
}

/**
* Destructor.
*/
template <class ObjectType>
SpearmanDistance<ObjectType>::~SpearmanDistance(){
}

/**
* @deprecated Use getDistance() instead.
*
* @copydoc getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error)
*/
template <class ObjectType>
double SpearmanDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    return getDistance(obj1, obj2);
}

/**
* Calculates the Spearman's Rank correlation between two feature vectors.
*
* To make this computations both feature vectors should have the same size().
*
* @param obj1: The first feature vector.
* @param obj2: The second feature vector.
* @throw Exception If the computation is not possible.
* @return The Spearman's Rank correlation between feature vector 1 and feature vector 2.
*/
template <class ObjectType>
double SpearmanDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    if (obj1.size() != obj2.size()){
        throw std::length_error ("The feature vectors do not have the same size.");
    }

    double* xrank = new double[obj1.size()];
    double* yrank = new double[obj1.size()];

    for (size_t i = 0; i < obj1.size(); i++){
        xrank[i] = 0.0;
        yrank[i] = 0.0;
    }

    int index = 1;

    double xElement = obj1[0];
    double yElement = obj2[0];

    //Gets the minor value and set it to *Element
    for (size_t i = 0; i < obj1.size(); i++){
        if (obj1[i] < xElement)
            xElement = obj1[i];
        if (obj2[i] < yElement)
            yElement = obj2[i];
    }

    bool ranking = true;

    //It ranks first time series
    while (ranking)	{
        u_int32_t recurrence = 0;
        //Count the recurrence
        for (size_t i = 0; i < obj1.size(); i++)
            if (obj1[i] == xElement)
                recurrence++;

        double rank = (index * 2) + recurrence - 1;
        rank /= (double) 2;

        //Set rank to the proper positions
        for (size_t i = 0; i < obj1.size(); i++)
            if (obj1[i] == xElement)
                xrank[i] = rank;

        ranking = false;
        double newElement;

        for (size_t i = 0; i < obj1.size(); i++) {
            //Continue if there's a number higher than xElement
            if (obj1[i] > xElement){
                ranking = true;
                newElement = obj1[i];
            }
        }

        // it stops if there isn't more numbers to rank in the sequence
        if (ranking){
            for (size_t i = 0; i < obj1.size(); i++)
                if ((obj1[i] > xElement) && (obj1[i] < newElement))
                    newElement = obj1[i];

            // sets to xElement the minor number not ranked of the sequence
            xElement = newElement;
            // xindex gets the next number to be used in the rank
            index += recurrence;
        }
    }


    ranking = true;
    index = 1;

    //It ranks second time series
    while (ranking) {

        int recurrence = 0;

        for (size_t i = 0; i < obj1.size(); i++)
            if (obj2[i] == yElement)
                recurrence++;

        double rank = (index * 2) + recurrence - 1;
        rank /= (double) 2;

        for (size_t i = 0; i < obj1.size(); i++)
            if (obj2[i] == yElement)
                yrank[i] = rank;

        ranking = false;
        double newElement;

        for (size_t i = 0; i < obj1.size(); i++){

            if (obj2[i] > yElement){
                ranking = true;
                newElement = obj2[i];
            }
        }

        if (ranking){
            for (size_t i = 0; i < obj1.size(); i++)
                if ((obj2[i] > yElement) && (obj2[i] < newElement))
                    newElement = obj2[i];

            yElement = newElement;
            index += recurrence;
        }
    }

    double sum = 0;

    for (size_t i = 0; i < obj1.size(); i++){
        double difference = xrank[i] - yrank[i];
        sum += difference * difference;
    }

    // Spearman's Rank Correlation formula
    double result = (1 - 6 * sum / (obj1.size()* (obj1.size() * obj1.size() - 1)));

    delete [] xrank;
    delete [] yrank;

    // Statistic support
    this->updateDistanceCount();

    return result;
}

