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
WarpDistance<ObjectType>::WarpDistance(){
}

/**
* Destructor.
*/
template <class ObjectType>
WarpDistance<ObjectType>::~WarpDistance(){
}

/**
* @deprecated Use getDistance(ObjectType &obj1, ObjectType &obj2) instead.
*
* @copydoc getDistance(ObjectType &obj1, ObjectType &obj2) .
*/
template <class ObjectType>
double WarpDistance<ObjectType>::GetDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    return getDistance(obj1, obj2);
}

/**
* Calculates similariry between the two metric histograms.
* This calculus is based on the histograms.
*
* @param obj1: The first feature vector.
* @param obj2: The second feature vector.
* @return The warp distance between feature vector 1 and feature vector 2.
*/
template <class ObjectType>
double WarpDistance<ObjectType>::getDistance(ObjectType &obj1, ObjectType &obj2) throw (std::length_error){

    double *new_obj1_value = new double[(obj1.size()/2)];
    double *new_obj2_value = new double[(obj2.size()/2)];

    for (size_t x = 0; x < (obj1.size()/2); x++){
        new_obj1_value[x] = obj1[(obj1.size()/2) + x];
    }

    for (size_t x = 0; x < (obj2.size()/2); x++){
        new_obj2_value[x] = obj2[(obj2.size()/2) + x];
    }

    size_t  i, j, lin, col, w;
    float distfinal;
    float dist[256][256], distacum[256][256];

    lin = (obj1.size()/2);
    col = (obj2.size()/2);

    float vetq[256], vetc[256];

    for (i = 0; i < lin; i++)
        vetq[i] = new_obj1_value[i];
    for (j = 0; j < col; j++)
        vetc[j] = new_obj2_value[j];

    for(i = 0; i < lin; i++){
        for(j = 0; j < col; j++){
            dist[i][j] = (fabs((vetq[i] - vetc[j])) * fabs(vetq[i] - vetc[j]));
        }
    }

    float d1, d2, d3;
    for(i = 0; i < lin; i++){
        for(j = 0; j < col; j++){
            if(i == 0){
                if(j == 0)
                    distacum[i][j] = dist[i][j];
                else
                    distacum[i][j] = dist[i][j] + distacum[i][j-1];
            }else{
                if(j == 0)
                    distacum[i][j] = dist[i][j] + distacum[i-1][j];
                else{
                    d1 = distacum[i-1][j-1];
                    d2 = distacum[i-1][j];
                    d3 = distacum[i][j-1];
                    if (d1<=d2 && d1<=d3) distacum[i][j] = dist[i][j] + d1;
                    else if (d2<=d3) distacum[i][j] = dist[i][j] + d2;
                    else distacum[i][j] = dist[i][j] + d3;
                }
            }
        }
    }

    w = 1;
    i = lin-1;
    j = col-1;
    do
    {   if(i == 0)
        {   if(j != 0)  j--;
        }
        else
        {   if(j == 0)  i--;
            else
            {   d1 = distacum[i-1][j-1];
                d2 = distacum[i-1][j];
                d3 = distacum[i][j-1];
                if (d1<=d2 && d1<=d3) { i--; j--; }
                else if (d2<=d3) i--;
                else j--;
            }
        }
        w++;

    }while(i > 0 || j > 0);

    distfinal = distacum[lin-1][col-1]/w;

    this->updateDistanceCount();

    return distfinal;
}
