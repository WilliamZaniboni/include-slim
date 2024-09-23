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
* This file defines the class stQueryHint.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STQUERYHINT_H
#define __STQUERYHINT_H

/**
* This class is the base class for all implementations of the query hint
* classes. A query hint is a special class that is used by some metric tree
* operations to store information about previous executions. This information
* can be used to enhance the performance of sucessive executions of the same
* operation.
*
* <p>Since each combination of operation/metric tree requires a specific type
* of information, each metric tree must implement its own set of query hint
* classes.
*
* <p>Users should not create instances of this class. Each tree has a method
* called stMetricTree::CreateQueryHint() which must be used to create the
* proper hint for each combination of mteric tree/query hint type.
*
* <p>For further information about this concept, see the library architecture
* documentation.
*
* @ingroup struct
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @note The implementation details of subclasses of this class are not
* relevant to users.
*/
class stQueryHint{

   public:

      /**
      * This destructor disposes this hint and releases all allocated resources.
      */
      virtual ~stQueryHint(){};
   
      /**
      * Returns the hint type. The mean of this value must be specified by
      * each metric access method.
      */
      int GetHintType() const { return type; }     

   private:

      /**
      * Hint type.
      */
      int type;
      
};//end stQueryHint


#endif //__STQUERYHINT_H
