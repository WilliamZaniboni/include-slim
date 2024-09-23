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
* This file implements the blocks of ParitionGlobal.
*
* @version 1.0
* @author Enzo Seraphim (seraphim@unifei.edu.br)
*/

#include <arboretum/stGnuplot3D.h>
#include <cstring>
#include <fstream>

//------------------------------------------------------------------------------
// Class stGnuplot3D
//------------------------------------------------------------------------------
stGnuplot3D::stGnuplot3D(int xvaluesCount, int barsCount){   // barCount = metodo   xvaluesCount = pagina
    xvalues = xvaluesCount;
    bars = barsCount;
    xlogscale = false;
    ylogscale = false;
    ylabel = NULL;
    yrange = NULL;
    xvaluesLabels = new char * [xvalues];
    xrealvalues = new long double * [xvalues];
    for (int i = 0; i < xvalues; i++) {
       xvaluesLabels[i] = NULL;
       xrealvalues[i] = new long double[bars];
       for(int j=0; j < bars; j++){
           xrealvalues[i][j]=0.0;
       }
    }
    barsTextures = new int[bars];
    barsLabels = new char * [bars];
    for (int i = 0; i < bars; i++) {
         barsTextures[i] = -1;
         barsLabels[i] = NULL;
    }
};

//------------------------------------------------------------------------------
stGnuplot3D::~stGnuplot3D(){
    if (ylabel)
       delete [] ylabel;
    if (yrange)
       delete [] yrange;
    if (barsTextures)
       delete [] barsTextures;
    for (int i = 0; i < xvalues; i++) {
      if (xvaluesLabels[i]){
         delete [] xvaluesLabels[i];
      }
      delete [] xrealvalues[i];
    }
    delete [] xvaluesLabels;
    delete [] xrealvalues;

    for (int i = 0; i < bars; i++) {
      if (barsLabels[i]){
         delete [] barsLabels[i];
      }
    }
    delete [] barsLabels;
};

//------------------------------------------------------------------------------
void stGnuplot3D::SetYLabel(const char * label){
   if (ylabel){
      delete [] ylabel;
   }
   ylabel = new char[strlen(label)+1];
   strcpy(ylabel, label);
}

//------------------------------------------------------------------------------
void stGnuplot3D::SetYRange(const char * label){
   if (yrange){
      delete [] yrange;
   }
   yrange = new char[strlen(label)+1];
   strcpy(yrange, label);
}

//------------------------------------------------------------------------------
void stGnuplot3D::SetBarLabel(int pos, const char * label) {
   #ifdef __stDEBUG__
   if ((pos < 0) && (pos >= bars)){
      throw range_error("pos is out of range.");
   }//end if
   #endif //__stDEBUG__
   if (barsLabels[pos]){
      delete [] barsLabels[pos];
   }
   barsLabels[pos] = new char[strlen(label)+1];
   strcpy(barsLabels[pos], label);
}

//------------------------------------------------------------------------------
void stGnuplot3D::SetXValueLabel(int pos, const char * label) {
   #ifdef __stDEBUG__
   if ((pos < 0) && (pos >= xvalues)){
      throw range_error("pos is out of range.");
   }//end if
   #endif //__stDEBUG__
   if (xvaluesLabels[pos]){
      delete [] xvaluesLabels[pos];
   }
   xvaluesLabels[pos] = new char[strlen(label)+1];
   strcpy(xvaluesLabels[pos], label);
};

//------------------------------------------------------------------------------
void stGnuplot3D::SaveTo(char * filename, tOutputPlot type){
    char * header = new char[102400]; // 100Kb
    float largura = 1.0 / float(bars + 1);
    char b[100];

    switch (type) {
       plotX11:
         strcpy(header, "set output \n");
         strcat(header, "set terminal windows \n");
         break;
       plotEps:
         sprintf(b, "set output \" %s.eps \n\0", filename);
         strcpy(header, b);
         strcat(header, "set terminal postscript eps monochrome solid lw 2 \n");
         break;
      plotPs:
         sprintf(b, "set output \" %s.ps \n\0", filename);
         strcpy(header, b);
         strcat(header, "set terminal corel monochrome linewidth 2 \n" );
         break;
       default:
         //plotWindow: 
         strcpy(header, "set output \n");
         strcat(header, "set terminal x11 \n");
    }
    strcat(header, "set encoding iso_8859_1 \n");
    strcat(header, "set data style boxes \n");
    sprintf(b, "set xrange [0:%d] \n\0", xvalues);
    strcat(header, b);

    strcat(header, "set xtic (");
    for (int i = 0; i < bars; i++) {
       sprintf(b, "\"%s\" %d\0", barsLabels[i], i);
       strcat(header, b);
       if (i < bars-1)
          strcat(header, ", ");
    }
    strcat(header, ")\n");

    if (yrange) {
       sprintf(b, "set xrange [%s] \n\0", yrange);
       strcat(header, b);
    }

    if (GetFontSize() >= 0) {
       sprintf(b, "set term postscript \"Times-Roman\" %d \n\0", GetFontSize());
       strcat(header, b);
    }

    if (GetXLabel()) {
        if (GetFontSize() >= 0){
           sprintf(b, "set xlabel \"%s\" \"Times-Roman\" %d \n\0",GetXLabel(), GetFontSize()+4);
           strcat(header, b);
        }else{
           sprintf(b, "set xlabel \"%s\" \n\0", GetXLabel());
           strcat(header, b);
        }
    }
    if (ylabel) {
        if (GetFontSize() >= 0){
           sprintf(b, "set ylabel \"%s\" \"Times-Roman\" %d \n\0", ylabel, GetFontSize()+4);
           strcat(header, b);
        }else{
           sprintf(b, "set ylabel \"%s\" \n\0", ylabel);
           strcat(header, b);
        }
    }

    if (xlogscale && ylogscale) {
        strcat(header, "set logscale xy \n");
    }
    else if (xlogscale) {
        strcat(header, "set logscale x \n");
    }
    else if (ylogscale) {
        strcat(header, "set logscale y \n");
    }

    for (int i = 0; i < xvalues; i++) {
      if (xvaluesLabels[i]){
        sprintf(b, "set label %d \"%s\" at %f , graph -0.05, 0 center norotate \n\0", i+1, xvaluesLabels[i], 0.5+i);
        strcat(header, b);
      }
    }

    if (GetLegend()){
        strcat(header, "set key outside Left reverse \n");
    }else{
        strcat(header, "set key off \n");
    }

    if (GetTitle()){
       sprintf(b, "set title \"%s\" \n\0", GetTitle());
       strcat(header, b);
    }//end if

    if (largura){
       sprintf(b, "set boxwidth %f \n\0", largura);
       strcat(header, b);
    }//end if

    strcat(header, "set style fill border 1 \n");

    strcat(header, "plot ");
    for (int i = 0; i < bars; i++) {
       sprintf(b, "\"-\" using ($1+%f):2 \0", (i+1)*largura);
       strcat(header, b);
       if (barsLabels[i]){
          sprintf(b, "title \"%s\" \0", barsLabels[i]);
          strcat(header, b);
       }
       if (barsTextures[i] == -1){
          sprintf(b, "fill pattern %d lw 2 \0",i + 1);
       }else{
          sprintf(b, "fill pattern %d lw 2 \0", barsTextures[i]);
       }
       strcat(header, b);
       if (i < bars-1)
          strcat(header, ", \\\n     ");
    }
    strcat(header, "\n");

    for (int i = 0; i < bars; i++) {
       if (barsLabels[i]){
          sprintf(b, "## %s ## \n\0", barsLabels[i]);
       }else{
          sprintf(b, "## serie %d ## \n\0", i+1);
       }
       strcat(header, b);
       for (int j = 0; j < xvalues; j++) {
           sprintf(b,"%d, %LF \n\0", j, xrealvalues[j][i]);
           strcat(header, b);
       }
       strcat(header, "end \n");
    }
    strcat(header, "\npause -1");

    // write data
    std::ofstream f (filename);
    f << header;
    f.close();

    delete [] header;

};
//------------------------------------------------------------------------------

