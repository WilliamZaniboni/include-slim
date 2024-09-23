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
* This file implements the class stFDASE.
*
* @version 1.0
* @author Caetano Traina Jr (caetano@icmc.usp.br)
* @author Agma Juci Machado Traina (agma@icmc.usp.br)
* @author Christos Faloutsos (christos@cs.cmu.edu)
* @author Elaine Parros Machado de Sousa (parros@icmc.usp.br)
* @author Ana Carolina Riekstin (anacarol@grad.icmc.usp.br)
* @ingroup fastmap
*/

//------------------------------------------------------------------------------
// template class stFDASE
//------------------------------------------------------------------------------
template <class DataType, class ObjectType> stFDASE<DataType,ObjectType>::stFDASE
        (int NumberOfObjects, ObjectType **ObjectsArray, TRange q1,
        int generalized, int Dimension, int NumberOfPointsInInterval, int PointSelect,
        int NormalizeFactor, int FittingAlgorithmFactor, double XInterval[30],
        double YInterval[30], double Threshold){

   /* Starts with zero - nothing done*/
   itFormed = 0;

   level = 0;
   stop=1;
   groupCount=0;
   last=0;
   A=B=C=D=0;

   Attributes = (struct stObjects *) calloc (Dimension,sizeof(struct stObjects));
   MaskFDASE = (unsigned int *) calloc (Dimension,sizeof(unsigned int));
   MaskUSE = (unsigned int *) calloc (Dimension,sizeof(unsigned int));
   MaskANSW = (unsigned int *) calloc (Dimension,sizeof(unsigned int));
   MaskCB = (unsigned int *) calloc (Dimension,sizeof(unsigned int));
   MaskBASE = (unsigned int *) calloc (Dimension,sizeof(unsigned int));


   /*In the begginig, this vector have all indexes with value 1, taht is,
   all attributes could be used.*/
   for (int i = 0, j = 1; i < Dimension; i++){
      MaskUSE[i] = j;
   }//end for

   /*To calculate individual contributions*/
   IndividualContribution(NumberOfObjects,ObjectsArray,q1,false,Dimension,NormalizeFactor,
             XInterval, YInterval, FittingAlgorithmFactor, NumberOfPointsInInterval,
             PointSelect, Attributes, MaskFDASE);

   /*Ordinance*/
   double min = 1000.0;
   double max = -1000.0;
   for (int i=0; i<Dimension; i++) {
      if (Attributes[i].IndividualContribution < min){
         min = Attributes[i].IndividualContribution;
      }else{
         if (Attributes[i].IndividualContribution > max){
            max = Attributes[i].IndividualContribution;
         }//end if
      }//end if
   }//end for
   if ((max - min) > 0.05){
      SortAttributes(Attributes,0,Dimension-1);
   }//end if

   /*The attributes that have iC less than 0,1 will be eliminated*/
   for (int i=0;i<Dimension;i++){
      if(Attributes[i].IndividualContribution<0.1){
         MaskUSE[i]=0;
      }//end if
   }//end for

   /*To store the groups*/
   FILE *FDASE=fopen("fdase.txt","w");
   if (FDASE == NULL) {
      printf ("Error opening File \"fdase.txt\"\n");
      return;
   }

   /*If it´s necessary to view the individual contribution, remove the commentaries symbols */
   /*fprintf (FDASE, "*** FDASE ***\n");
   fprintf(FDASE, "Individual Contributions:\n");
   for (int i=0;i<Dimension;i++)
     fprintf(FDASE,"Attribute: %d - iC: %f\n",Attributes[i].attribute,Attributes[i].IndividualContribution);
   fprintf(FDASE,"\n");*/
   fprintf (FDASE, "Final result\n");

   /***************************************************************************/
   //Finding groups...

   while(stop==1){
      for (int i=0;i<Dimension;i++){
         MaskFDASE[i]=0;
         MaskANSW[i]=0;
      }

      PartialDimension(NumberOfObjects,ObjectsArray,q1,false,Dimension,NormalizeFactor,
         XInterval, YInterval, FittingAlgorithmFactor, NumberOfPointsInInterval,
         PointSelect, Attributes, MaskFDASE, Threshold, MaskUSE);

      double A1, B1;
      int j;
      if (itFormed == 2){
         for(j=0;j<Dimension;j++){
            if(MaskFDASE[j]!=1 && MaskUSE[j]!=0){
               for(int i=j;i<=last;i++){
                  if(MaskUSE[i]!=0){
                     MaskFDASE[i]=1;
                  }//end if
               }//end for
               if(j>last){
                  sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
                  A1=sFD->GetAlpha();
                  MaskFDASE[j]=1;
                  /*Calculates the difference in partial dimensions.
                  When the attribute does not increase significantly the pD, break!*/
                  sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
                  B1=sFD->GetAlpha();
                  MaskUSE[j]=2;
                  if(fabs(A1-B1)<Attributes[j].IndividualContribution*Threshold){
                     last=j;
                     break;
                  }else{
                     if (j==Dimension-1){
                        /*If it happens, it means that a group won´t be formed, or else
                        it would have "entered" the if above*/
                        for(int b=0;b<Dimension;b++){
                           MaskFDASE[b]=0;
                        }//end for
                     }else {
                        continue;
                     }//end if
                  }//end if
               }else{
                  continue;
               }//end if
            }//end if
         }//end for
      }//end if

      int k=0;
      for (j=0;j<Dimension;j++){
         if (MaskFDASE[j]==1){
            MaskUSE[j]=2; //it means that the attribute was already been visited
            k++;
         }//end if
      }//end for

      if (k==Dimension){
         sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
         A=sFD->GetAlpha();
         MaskFDASE[k-1]=0;
         sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
         B=sFD->GetAlpha();
         MaskFDASE[k-1]=1;
         if (fabs(A-B)>=Threshold*Attributes[k-1].IndividualContribution){
            ShowMessage("There aren't correlated attributes in this group.");
            break;
         }//end if
      }//end if

      /*MaskANSW = */ FindGroup (NumberOfObjects,ObjectsArray,q1,false,Dimension,NormalizeFactor,
               XInterval, YInterval, FittingAlgorithmFactor, NumberOfPointsInInterval,
               PointSelect, Attributes, MaskFDASE, Threshold, MaskUSE, MaskANSW, level);

      if (itFormed == 3 || itFormed == 1){
         int count=0;
         for (int n=0;n<Dimension;n++){
            if(MaskBASE[n]==1)
               count=n;
         }//end for

         for (j=count;j<Dimension;j++){
            if (MaskANSW[j]==0 && MaskUSE[j]!=0){
               /*It´s necessary to clear MaskFDASE, because the comparations are
               made to each aj*/
               for (int i=0;i<Dimension;i++){
                  MaskFDASE[i]=MaskBASE[i];
               }//end for
               sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
               A=sFD->GetAlpha();
               MaskFDASE[j]=1;

               sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
               double auxPD=sFD->GetAlpha();
               if(fabs(A-auxPD)<Threshold*Attributes[j].IndividualContribution){
                  MaskANSW[j]=1;
                  MaskUSE[j]=2;
                  for (int i=0;i<Dimension;i++){
                     if (MaskBASE[i]==1 && MaskUSE[i]!=0){
                        MaskFDASE[i]=0;
                        sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
                        A=sFD->GetAlpha();//pd (base - base[j] + a[j])
                        C=fabs(A-auxPD);
                        MaskFDASE[j]=0; //0 because aj is not considered anymore
                        sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q1,false,Dimension,
                                   NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                   FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                   Attributes);
                        B=sFD->GetAlpha();
                        D=fabs(A-B);
                        MaskFDASE[j]=1;
                        MaskFDASE[i]=1;
                        if (C>=Threshold*Attributes[i].IndividualContribution){
                           MaskANSW[j]=0;
                           MaskUSE[j]=1;
                        }else{
                           if(D<Threshold*Attributes[j].IndividualContribution){
                              MaskANSW[j]=0;
                              MaskUSE[j]=1;
                           }//end if D
                        }//end if C
                     }//end if
                  }//end for
               }//end if
            }//end if
            MaskFDASE[j]=0;
         }//end for

         fprintf(FDASE,"      Correlation Group %d:    ",groupCount+1);
         for (int i=0;i<Dimension;i++){
            if (MaskANSW[i]==1){
               fprintf(FDASE,"%d$ ",Attributes[i].attribute);
            }//end if
         }//end for
         fprintf(FDASE,"\n        Correlation Base %d:    ",groupCount+1);
         for (int i=0;i<Dimension;i++){
            if (MaskBASE[i]==1){
               fprintf(FDASE,"%d$ ",Attributes[i].attribute);
            }//end if
         }//end for
         fprintf(FDASE,"\n\n");
         itFormed=0;
         last=0;

         //remove G-B from A
         for (int i=0;i<Dimension;i++){
            if(MaskANSW[i]==1 && MaskBASE[i]==0){
               MaskUSE[i]=0;
            }//end if
         }//end for

         for(int i=0;i<Dimension;i++){
            if(MaskBASE[i]==1){
               MaskCB[i]=1;
            }//end if
         }//end for

         last=0;
         groupCount+=1;
      }//end if

      int i;
      for (i=0;i<Dimension;i++){
         if (MaskUSE[i]==1 && MaskCB[i]==0){
            break;
         }//end if
      }//end for
      if (i==Dimension){
         stop=0;
      }//end if
   }//end while
   /***************************************************************************/
   fprintf(FDASE,"Attribute Set Core:  ");
   for (int i=0;i<Dimension;i++){
      if (MaskCB[i]==1 || MaskUSE[i]==2){
         fprintf(FDASE,"%d$ ",Attributes[i].attribute);
         MaskCB[i]=1;
      }
   }
   fprintf(FDASE,"&\n\n");
   fclose(FDASE);
  
   delete MaskFDASE;
   delete MaskUSE;
   delete MaskANSW;
   delete MaskCB;
   delete Attributes;
   delete MaskBASE;
};//end stFractalDimension::stFractalDimension
//---------------------------------------------------------------------------
template <class DataType, class ObjectType> void stFDASE<DataType,ObjectType>::IndividualContribution
        (int NumberOfObjects, ObjectType **ObjectsArray, TRange q,
        int generalized, int Dimension, int NormalizeFactor, double XInterval[30],
        double YInterval[30], int FittingAlgorithmFactor, int NumberOfPointsInInterval,
        int PointSelect, struct stObjects *Attributes, unsigned int *MaskFDASE){

   for (int i=0;i<Dimension;i++){
     for (int k = 0, j = 0; k < Dimension; k++){
        MaskFDASE[k] = j; /*j <<= 0*/;
     }
     MaskFDASE[i]=1;
     Attributes[i].attribute=i;
     sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                          Attributes);
     Attributes[i].IndividualContribution = sFD->GetAlpha();
   }
};//end stFDASE::IndividualContribution

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> void stFDASE<DataType,ObjectType>::SortAttributes
        (struct stObjects *Vector, int start, int Dimension){
   /*QuickSort*/
   int i,j;
   struct stObjects x,y;

   i=start;
   j=Dimension;
   x.IndividualContribution=Vector[(start+Dimension)/2].IndividualContribution;

   do{
      while (Vector[i].IndividualContribution>x.IndividualContribution && i<Dimension) i++;
      while(x.IndividualContribution>Vector[j].IndividualContribution && j>start) j--;

      if(i<=j){
         y=Vector[i];
         Vector[i]=Vector[j];
         Vector[j]=y;
         i++;
         j--;
      }
  }while (i<=j);

  if(start<j)SortAttributes(Vector,start,j);
  if(i<Dimension)SortAttributes(Vector,i,Dimension);

   /*InsertionSort*/
   /*int a,b;
   struct stObjects t;
   for(a=1;a<=Dimension;++a){
      t.a=Vector[a].a;
      t.IndividualContribution=Vector[a].IndividualContribution;
      for(b=a-1; b>=0 && t.IndividualContribution>Vector[b].IndividualContribution; b--)
         Vector[b+1]=Vector[b];
      Vector[b+1]=t;
   }*/
};//end stFDASE::SortAttributes

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> void stFDASE<DataType,ObjectType>::PartialDimension
        (int NumberOfObjects, ObjectType **ObjectsArray, TRange q,
        int generalized, int Dimension, int NormalizeFactor, double XInterval[30],
        double YInterval[30], int FittingAlgorithmFactor, int NumberOfPointsInInterval,
        int PointSelect, struct stObjects *Attributes, unsigned int *MaskFDASE,
        double Threshold, unsigned int *MaskUSE){

   int i = 0;
   double m,n,prim;
   DataType Alpha, Beta;

   for (int i = 0, j = 0; i < Dimension; i++)
      MaskFDASE[i] = j;
   MaskFDASE[i]=1;
   prim=0;
   sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                          Attributes);
   Alpha = sFD->GetAlpha();
   for(i=1;i<Dimension;i++){
      if(MaskUSE[i]!=0){
         prim=Alpha;
         MaskFDASE[i]=1;
         sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                          Attributes);
         Alpha = sFD->GetAlpha();
         if(fabs(Alpha-prim)<Attributes[i].IndividualContribution*Threshold)
            break;
      }//end if
   }//end for
};

//---------------------------------------------------------------------------
template <class DataType, class ObjectType> unsigned int * stFDASE<DataType,ObjectType>::FindGroup
        (int NumberOfObjects, ObjectType **ObjectsArray, TRange q,
        int generalized, int Dimension, int NormalizeFactor, double XInterval[30],
        double YInterval[30], int FittingAlgorithmFactor, int NumberOfPointsInInterval,
        int PointSelect, struct stObjects *Attributes, unsigned int *MaskFDASE,
        double Threshold, unsigned int *MaskUSE, unsigned int *MaskANSW, int level){
   DataType A;
   DataType B;
   int i,j,k;

   /*itFormed will be verified constantly because of the recursion*/

   /*MaskAUX is needed when the algorithm returns from recursion, because the
   indexes change constantly.*/
   unsigned int * MaskAUX;
   MaskAUX = (unsigned int *) calloc (Dimension,sizeof(unsigned int));

   for (j=0;j<Dimension && itFormed!=3;j++){
      for (i=0;i<Dimension;i++){
         MaskAUX[i]=MaskFDASE[i];
      }//end for
      if(MaskFDASE[j]==1 && MaskUSE[j]!=0){
         sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                          Attributes);
         A = sFD->GetAlpha();

         MaskFDASE[j]=0;
         sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                          Attributes);
         B = sFD->GetAlpha();

         if (fabs(A-B)>=(Threshold*Attributes[j].IndividualContribution)){
            MaskANSW[j]=0; //out of the group
         }else{
            A = B;
            //Removes ak - step 4
            for (i=Dimension-1;i>=0;i--){
               if (MaskFDASE[i]==1){
                  MaskFDASE[i]=0;
                  break;
               }//end if
            }//end for

            sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskFDASE,
                                          Attributes);
            B = sFD->GetAlpha();

            MaskFDASE[i]=1;

            /*This is needed in situations which ak from the algorithm is minor
            than the current index j*/
            k = i;
            if (i<j){
               k=j;
            }//end if

            if (fabs(A-B)<(Threshold*Attributes[k].IndividualContribution)){
               level++;
               for(k=0;k<j;k++){
                  MaskANSW[k]=0; //what is before must be ignored in the next step
               }//end for
               FindGroup(NumberOfObjects,ObjectsArray,q,false,Dimension,NormalizeFactor,
                      XInterval, YInterval, FittingAlgorithmFactor, NumberOfPointsInInterval,
                      PointSelect, Attributes, MaskFDASE, Threshold, MaskUSE, MaskANSW, level);
               level--;

               if (itFormed==1){
                  itFormed = 3; //to control
                  return MaskANSW;
               }else{
                  if (itFormed==2){
                     for(k=0;k<Dimension;k++){ //to return to the original mask before recursion
                        MaskFDASE[k]=MaskAUX[k];
                        MaskANSW[k]=0;
                     }
                  }//end if
               }//end if
            }else{
               if (itFormed!=3){
                  MaskANSW[j]=1;
               }//end if
            }//end if
            if (itFormed!=3){
               MaskFDASE[j]=1;
            }//end if
         }//end if
      }//end if
   }//end for

   if (itFormed!=3){
      for (i=0;i<Dimension;i++){
         MaskBASE[i]=0;
      }//end for

      for (i=0;i<Dimension;i++){
         if (MaskANSW[i]==1 && MaskUSE[i]!=0){
            sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskBASE,
                                          Attributes);
            A = sFD->GetAlpha();
            MaskBASE[i]=1;
            sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskBASE,
                                          Attributes);
            B = sFD->GetAlpha();

            if (fabs(B-A)<=(Threshold*Attributes[i].IndividualContribution)){
               /*In a first step, the ith index of MaskBase is inserted.
               In case the test above is true (step 7 of FindGroup), then the
               ith index is removed*/
               MaskBASE[i]=0;
            }//end if
         }//end if
      }//end for

      //8th step of FindGroup
      for (i=0;i<Dimension;i++){
         if (MaskBASE[i]==MaskANSW[i]){
            continue;
         }else{
            break;
         }//end if
      }//end for

      if (i==Dimension){ //group and base are equal
         itFormed=2;
      }else{
         for (j=0;j<Dimension;j++){
            if(MaskANSW[j]==1 && MaskBASE[j]==0){
               for (k=0;k<Dimension;k++){
                  if (MaskBASE[k]==1 && MaskUSE[k]!=0){
                     MaskBASE[j]=1;
                     sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskBASE,
                                          Attributes);
                     A=sFD->GetAlpha();
                     MaskBASE[k]=0;
                     sFD = new stFractalDimension<DataType,ObjectType>(NumberOfObjects,ObjectsArray,q,false,Dimension,
                                          NumberOfPointsInInterval,PointSelect,NormalizeFactor,
                                          FittingAlgorithmFactor,XInterval,YInterval,MaskBASE,
                                          Attributes);
                     B=sFD->GetAlpha();
                     MaskBASE[k]=1;

                     if (fabs(A-B)>=Threshold*Attributes[k].IndividualContribution){
                        MaskANSW[j]=0;
                     }//end if
                     MaskBASE[j]=0;
                  }//end if
               }//end for
            }//end if
         }//end for

         //We need to verify again if group and base are equal
         for (i=0;i<Dimension;i++){
            if (MaskBASE[i]==MaskANSW[i]){
              continue;
            }else{
               break;
            }//end if
         }//end for

         if (i==Dimension){ //group and base are equal
            itFormed=2;
         }else{
            itFormed=1;
         }//end if
      }//end if
   }//end if
   delete MaskAUX;
   return MaskANSW;
};
