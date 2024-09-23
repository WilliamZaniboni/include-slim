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

//----------------------------------------------------------------------------
// class stListPriorityQueue
//----------------------------------------------------------------------------
/**
* This class defines node type of stListPriorityQueue.
*
*
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @version 1.0
* @ingroup util
* @todo Documentation update.
*/
#include <arboretum/stListPriorityQueue.h>

//------------------------------------------------------------------------------
stListPriorityQueue::~stListPriorityQueue(){
   stEntry * temp = head;
   while(head){
      head = head->GetNext();
      delete temp;
      temp = head;
   }//end while
}//end ~stListPriorityQueue

//------------------------------------------------------------------------------
void stListPriorityQueue::Add(u_int32_t pageID, double distance, double radius){
    stEntry * newEntry;
    stEntry * priorEntry = NULL;
    stEntry * currEntry = head;

    while ((currEntry!=NULL) && (distance > currEntry->GetDistance())){
       priorEntry = currEntry;
       currEntry = currEntry->GetNext();
    }//end while

    newEntry = new stEntry();
    newEntry->SetPageID(pageID);
    newEntry->SetDistance(distance);
    newEntry->SetRadius(radius);
    newEntry->SetNext(currEntry);
    capacity++;

    if (priorEntry==NULL){
       head = newEntry;
    }else{
       priorEntry->SetNext(newEntry);
    }//end if

}//end stListPriorityQueue::Add

//------------------------------------------------------------------------------
stEntry * stListPriorityQueue::Get(){
   stEntry * returnEntry = head;
   if(head){
      head = head->GetNext();
      capacity--;
   }//end if
   return returnEntry;
}//end stListPriorityQueue::Get
