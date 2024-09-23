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
* This file defines the abstract class DistanceFunction.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
// Copyright (c) 2002 GBDI-ICMC-USP

#ifndef __DistanceFunction_H
#define __DistanceFunction_H


/**
* This abstract class implements the DistanceFunction interface. This interface is
* required by the metric distance functions that will be used by the metric
* trees implemented by this library. Since these metric trees are implemented as
* class templates, any class that implements all methods defined in this
* interface may be used by a metric tree.
*
*
* <p>As an optional function, a metric evaluator is the responsible to compute
* the number of distances calculated WHEN IT IS REQUIRED (it is not used by the
* Structure Layer).
*
* <p>The implementation of this feature has no standard interface and may be
* implemented freely but to make the implementation easier, there
* is a class called DistanceFunctionStatistics that implements the basic
* functions necessary to accomplish this task.
*
* <P>This class may be used as the base class for classes that implements the
* metric evaluators which will be used by a metric tree to compute distances
* but it is not recommended.
*
* @version 1.0
* @ingroup userinterface
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @date 29-10-2014
* @see DistanceFunctionStatistics
*/

#include <cmath>
#include <cstdlib>

template <class ObjectType>
class DistanceFunction{

    protected:
        /**
        * The distance counter itself.
        */
        u_int32_t distCount;

    public:

        /**
        * Constructor.
        * @param d Internal statistics of distance function.
        */
        DistanceFunction(u_int32_t d = 0){
            distCount = d;
        }

        /**
        * Destroy instance.
        */
        virtual ~DistanceFunction(){
        }

        /**
        * This method calculates the metric distance between 2 objects.
        *
        * @deprecated Use getDistance() instead.
        *
        * @param obj1 Object 1.
        * @param obj2 Object 2.
        * @return The distance between to objects.
        * @note This method is required to fulfill the DistanceFunction interface.
        */
        virtual double GetDistance(ObjectType & obj1, ObjectType & obj2) = 0;

        /**
        * @copydoc GetDistance(ObjectType & obj1, ObjectType & obj2) .
        */
        virtual double getDistance(ObjectType & obj1, ObjectType & obj2) = 0;

        /**
        * Overload on operator to set statistics on a new operator.
        *
        * @param evaluator The evaluator statistics to be set.
        * @return The DF with new statistics.
        */
        DistanceFunction& operator=(const DistanceFunction& evaluator){

            distCount = evaluator.getDistanceCount();
            return *this;
        }
      

        /**
        * @deprecated Use resetStatistics() instead.
        * Resets statistics.
        */
        void ResetStatistics(){

            resetStatistics();
        }

        /**
        * @copydoc ResetStatistics() .
        */
        void resetStatistics(){

            distCount = 0;
        }
        /**
        * @copydoc getDistanceCount() .
        */
        u_int32_t GetDistanceCount() {

            return getDistanceCount();
        }

        /**
        * Returns the number of distances performed.
        * @deprecated use getDistanceCount() instead.
        * @return Returns the number of distances performed.
        */
        u_int32_t getDistanceCount() {
            return distCount;
        }

        /**
        * @copydoc UpdateDistanceCount() .
        */
        void UpdateDistanceCount(){

            updateDistanceCount();
        }

        /**
        * Updates the distance counter by adding 1.
        * @deprecated use updateDistanceCount() instead.
        */
        void updateDistanceCount(){

            distCount++;
        }

   
};//end DistanceFunction
#endif //__DistanceFunction_H
