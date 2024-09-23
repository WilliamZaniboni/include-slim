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
#include <arboretum/stLevelDiskAccess.h>

//---------------------------------------------------------------------------
// class stLevelDiskAccess
//---------------------------------------------------------------------------
stLevelDiskAccess::stLevelDiskAccess(unsigned int height){
   Height = height;
   Radius = new double [Height];
   Nodes = new unsigned int [Height];
   Entries = new unsigned int [Height];
   for (unsigned int i = 0; i < Height; i++){
      Radius[i] = 0;
      Nodes[i] = 0;
      Entries[i] = 0;
   }//end if
}//end stLevelDiskAccess

//---------------------------------------------------------------------------
stLevelDiskAccess::~stLevelDiskAccess(){
   // Delete the resources.
   delete[] Radius;
   delete[] Nodes;
   delete[] Entries;
}//end ~stLevelDiskAccess

//---------------------------------------------------------------------------
void stLevelDiskAccess::AddEntry(double radius, unsigned int height){
   if (height < Height){
      Radius[height] += radius;
      Entries[height]++;
   }//end if
}//end AddEntry

//---------------------------------------------------------------------------
double stLevelDiskAccess::GetAvgRadius(unsigned int idx){
   if (idx < Height){
      return Radius[idx];
   }//end if
   return 0;
}//end GetAvgRadius

//---------------------------------------------------------------------------
long stLevelDiskAccess::GetNumberOfNodes(unsigned int idx){
   if (idx < Height){
      return Nodes[idx];
   }//end if
   return 0;
}//end GetNumberOfNodes

//---------------------------------------------------------------------------
void stLevelDiskAccess::AddNumberOfNodes(unsigned int height){
   if (height < Height){
      Nodes[height]++;
   }//end if
}//end GetNumberOfNodes

//---------------------------------------------------------------------------
void stLevelDiskAccess::Sumarize(){
   unsigned int i;
   
   for (i = 0; i < Height; i++){
      if (Entries[i] != 0){
         Radius[i] = Radius[i] / Entries[i];
      }else{
         Radius[i] = 0; 
      }//end if
   }//end if
}//end Sumarize
