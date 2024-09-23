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
* This file is the implementation of stSlimTree methods.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Josiel Maimone de Figueiredo (josiel@icmc.usp.br)
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
* @author Thiago Galbiatti Vespa (thiago@icmc.usp.br)
*/
#include <iostream>
//==============================================================================
// Class stSlimLogicNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSlimLogicNode<ObjectType, EvaluatorType>::stSlimLogicNode(u_int32_t maxOccupation){

   // Allocate resources
   MaxEntries = maxOccupation;
   Entries = new stSlimLogicEntry[MaxEntries];

   // Init Rep
   RepIndex[0] = 0;
   RepIndex[1] = 0;

   // Initialize
   Count = 0;

   // Minimum occupation. 25% is the default of Slim-tree
   MinOccupation = (u_int32_t) (0.25 * maxOccupation);
   // At least the nodes must store 2 objects.
   if ((MinOccupation > (maxOccupation/2)) || (MinOccupation == 0)){
      MinOccupation = 2;
   }//end if
}//end stSlimLogicNode<ObjectType, EvaluatorType>::stSlimLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSlimLogicNode<ObjectType, EvaluatorType>::~stSlimLogicNode(){
   u_int32_t i;

   if (Entries != 0){
      for (i = 0; i < Count; i++){
         if ((Entries[i].Object != 0) && (Entries[i].Mine)){
            delete Entries[i].Object;
			Entries[i].Object = 0;
         }//end if
      }//end for
   }//end if
   // Clean before exit.
   delete[] Entries;
   Entries = 0;
}//end stSlimLogicNode<ObjectType, EvaluatorType>::~stSlimLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSlimLogicNode<ObjectType, EvaluatorType>::AddEntry(u_int32_t size, const unsigned char * object){
   if (Count < MaxEntries){
      Entries[Count].Object = new ObjectType();
      Entries[Count].Object->IncludedUnserialize(object, size);
      Entries[Count].Mine = true;
      Count++;
      return Count - 1;
   }else{
      return -1;
   }//end if
}//end stSlimLogicNode<ObjectType, EvaluatorType>::AddEntry

template <class ObjectType, class EvaluatorType>
int stSlimLogicNode<ObjectType, EvaluatorType>::AddEntryForIndex(u_int32_t size, const unsigned char * object){
   if (Count < MaxEntries){
      Entries[Count].Object = new ObjectType();  
      Entries[Count].Object->Unserialize(object, size);
      Entries[Count].Mine = true;
      Count++;
      return Count - 1;
   }else{
      return -1;
   }//end if
}

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimLogicNode<ObjectType, EvaluatorType>::AddIndexNode(stSlimIndexNode * node){
   u_int32_t i;
   int idx;

   for (i = 0; i < node->GetNumberOfEntries(); i++){

      idx = AddEntryForIndex(node->GetObjectSize(i), node->GetObject(i));
   
      SetEntry(idx, node->GetIndexEntry(i).PageID,
                    node->GetIndexEntry(i).NEntries,
                    node->GetIndexEntry(i).Radius);
   }//end for

   // Node type
   NodeType = stSlimNode::INDEX;
}//end stSlimLogicNode<ObjectType, EvaluatorType>::AddIndexNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimLogicNode<ObjectType, EvaluatorType>::AddLeafNode(stSlimLeafNode * node){
   u_int32_t i;
   int idx;

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      idx = AddEntry(node->GetObjectSize(i), node->GetObject(i));
      SetEntry(idx, 0, 0, 0);
   }//end for

   // Node type
   NodeType = stSlimNode::LEAF;
}//end stSlimLogicNode<ObjectType, EvaluatorType>::AddLeafNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stSlimLogicNode<ObjectType, EvaluatorType>::TestDistribution(
      stSlimIndexNode * node0, stSlimIndexNode * node1,
      EvaluatorType * metricEvaluator){
   u_int32_t dCount;
   u_int32_t i;
   int idx;
   int l0, l1;
   int currObj;
   doubleIndex * idx0, * idx1;

   // Setup Objects
   dCount = UpdateDistances(metricEvaluator);

   // Init Map and Sorting vector
   idx0 = new doubleIndex[Count];
   idx1 = new doubleIndex[Count];
   for (i = 0; i < Count; i++){
      idx0[i].Index = i;
      idx0[i].Distance = Entries[i].Distance[0];
      idx1[i].Index = i;
      idx1[i].Distance = Entries[i].Distance[1];
      Entries[i].Mapped = false;
   }//end for

   // Sorting by distance...
   std::sort(idx0, idx0 + Count);
   std::sort(idx1, idx1 + Count);

   // Make one of then get the minimum occupation.
   l0 = l1 = 0;

   // Adds at least MinOccupation objects to each node.
   for (i = 0; i < MinOccupation; i++){
      // Find a candidate for node 0
      while (Entries[idx0[l0].Index].Mapped){
         l0++;
      }//end while
      // Add to node 0
      currObj = idx0[l0].Index;
      Entries[currObj].Mapped = true;
      idx = node0->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());

      // Test if the new object was inserted.
      if (idx >= 0){
         // Ok. Inserted into node1. Fill the others filds.
         node0->GetIndexEntry(idx).Distance = idx0[l0].Distance;
         node0->GetIndexEntry(idx).PageID = Entries[currObj].PageID;
         node0->GetIndexEntry(idx).NEntries = Entries[currObj].NEntries;
         node0->GetIndexEntry(idx).Radius = Entries[currObj].Radius;
      }else{
         // Oops. There is an error during the insertion.
         // The Node has not sufficient space to store this object.
         #ifdef __stDEBUG__
            cout << "The page size is too small. Increase it!\n";
         #endif //__stDEBUG__
         // Throw an exception.
         throw std::logic_error("The page size is too small.");
      }//end if

      // Find a candidate for node 1
      while (Entries[idx1[l1].Index].Mapped){
         l1++;
      }//end while
      // Add to node 1
      currObj = idx1[l1].Index;
      Entries[currObj].Mapped = true;
      idx = node1->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());
      // Test if the new object was inserted.
      if (idx >= 0){
         // Ok. Inserted into node1. Fill the others filds.
         node1->GetIndexEntry(idx).Distance = idx1[l1].Distance;
         node1->GetIndexEntry(idx).PageID = Entries[currObj].PageID;
         node1->GetIndexEntry(idx).NEntries = Entries[currObj].NEntries;
         node1->GetIndexEntry(idx).Radius = Entries[currObj].Radius;
      }else{
         // Oops. There is an error during the insertion.
         // The Node has not sufficient space to store this object.
         #ifdef __stDEBUG__
            cout << "The page size is too small. Increase it!\n";
         #endif //__stDEBUG__
         // Throw an exception.
         throw std::logic_error("The page size is too small.");
      }//end if
   }//end for

   // Distribute the others.
   for (i = 0; i < Count; i++){
      if (Entries[i].Mapped == false){
         Entries[i].Mapped = true;
         if (Entries[i].Distance[0] < Entries[i].Distance[1]){
            // Try to put on node 0 first
            idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node0->GetIndexEntry(idx).Distance = Entries[i].Distance[0];
               node0->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node0->GetIndexEntry(idx).NEntries = Entries[i].NEntries;
               node0->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }else{
               // Let's put it in the node 1 since it doesn't fit in the node 0
               idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node1->GetIndexEntry(idx).Distance = Entries[i].Distance[1];
               node1->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node1->GetIndexEntry(idx).NEntries = Entries[i].NEntries;
               node1->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }//end if
         }else{
            // Try to put on node 1 first
            idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node1->GetIndexEntry(idx).Distance = Entries[i].Distance[1];
               node1->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node1->GetIndexEntry(idx).NEntries = Entries[i].NEntries;
               node1->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }else{
               // Let's put it in the node 0 since it doesn't fit in the node 1
               idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node0->GetIndexEntry(idx).Distance = Entries[i].Distance[0];
               node0->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node0->GetIndexEntry(idx).NEntries = Entries[i].NEntries;
               node0->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }//end if
         }//end if
      }//end if
   }//end for

   // Clean home before go away...
   delete[] idx0;
   idx0 = 0;
   delete[] idx1;
   idx1 = 0;

   return dCount;
}//end stSlimLogicNode<ObjectType, EvaluatorType>::TestDistribution

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stSlimLogicNode<ObjectType, EvaluatorType>::TestDistribution(
      stSlimLeafNode * node0, stSlimLeafNode * node1,
      EvaluatorType * metricEvaluator){
   u_int32_t dCount;
   u_int32_t i;
   int idx;
   int l0, l1;
   int currObj;
   doubleIndex * idx0, * idx1;

   // Setup Objects
   dCount = UpdateDistances(metricEvaluator);

   // Init Map and Sorting vector
   idx0 = new doubleIndex[Count];
   idx1 = new doubleIndex[Count];
   for (i = 0; i < Count; i++){
      idx0[i].Index = i;
      idx0[i].Distance = Entries[i].Distance[0];
      idx1[i].Index = i;
      idx1[i].Distance = Entries[i].Distance[1];
      Entries[i].Mapped = false;
   }//end for

   // Sorting by distance...
   std::sort(idx0, idx0 + Count);
   std::sort(idx1, idx1 + Count);

   // Make one of then get the minimum occupation.
   l0 = l1 = 0;

   // Adds at least MinOccupation objects to each node.
   for (i = 0; i < MinOccupation; i++){
      // Find a candidate for node 0
      while (Entries[idx0[l0].Index].Mapped){
         l0++;
      }//end while
      // Add to node 0
      currObj = idx0[l0].Index;
      Entries[currObj].Mapped = true;
      idx = node0->AddEntry(Entries[currObj].Object->GetIncludedSerializedSize(),
                            Entries[currObj].Object->IncludedSerialize());
      node0->GetLeafEntry(idx).Distance = idx0[l0].Distance;

      // Find a candidate for node 1
      while (Entries[idx1[l1].Index].Mapped){
         l1++;
      }//end while
      // Add to node 1
      currObj = idx1[l1].Index;
      Entries[currObj].Mapped = true;
      idx = node1->AddEntry(Entries[currObj].Object->GetIncludedSerializedSize(),
                            Entries[currObj].Object->IncludedSerialize());
      node1->GetLeafEntry(idx).Distance = idx1[l1].Distance;
   }//end for

   // Distribute the others.
   for (i = 0; i < Count; i++){
      if (Entries[i].Mapped == false){
         Entries[i].Mapped = true;
         if (Entries[i].Distance[0] < Entries[i].Distance[1]){
            // Try to put on node 0 first
            idx = node0->AddEntry(Entries[i].Object->GetIncludedSerializedSize(),
                                  Entries[i].Object->IncludedSerialize());
            if (idx >= 0){
               node0->GetLeafEntry(idx).Distance = Entries[i].Distance[0];
            }else{
               // Let's put it in the node 1 since it doesn't fit in the node 0
               idx = node1->AddEntry(Entries[i].Object->GetIncludedSerializedSize(),
                                     Entries[i].Object->IncludedSerialize());
               node1->GetLeafEntry(idx).Distance = Entries[i].Distance[1];
            }//end if
         }else{
            // Try to put on node 1 first
            idx = node1->AddEntry(Entries[i].Object->GetIncludedSerializedSize(),
                                  Entries[i].Object->IncludedSerialize());
            if (idx >= 0){
               node1->GetLeafEntry(idx).Distance = Entries[i].Distance[1];
            }else{
               // Let's put it in the node 0 since it doesn't fit in the node 1
               idx = node0->AddEntry(Entries[i].Object->GetIncludedSerializedSize(),
                                     Entries[i].Object->IncludedSerialize());
               node0->GetLeafEntry(idx).Distance = Entries[i].Distance[0];
            }//end if
         }//end if
      }//end if
   }//end for

   // Clean home before go away...
   delete[] idx0;
   idx0 = 0;
   delete[] idx1;
   idx1 = 0;

   return dCount;
}//end stSlimLogicNode<ObjectType, EvaluatorType>::TestDistribution

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stSlimLogicNode<ObjectType, EvaluatorType>::UpdateDistances(
      EvaluatorType * metricEvaluator){
   u_int32_t i;

   for (i = 0; i < Count; i++){
      if (i == RepIndex[0]){
         Entries[i].Distance[0] = 0;
         Entries[i].Distance[1] = MAXDOUBLE;
      }else if (i == RepIndex[1]){
         Entries[i].Distance[0] = MAXDOUBLE;
         Entries[i].Distance[1] = 0;
      }else{
         Entries[i].Distance[0] = metricEvaluator->GetDistance(
               *Entries[RepIndex[0]].Object, *Entries[i].Object);
         Entries[i].Distance[1] = metricEvaluator->GetDistance(
               *Entries[RepIndex[1]].Object, *Entries[i].Object);
      }//end if
   }//end for

   return (GetNumberOfEntries() * 2) - 2;
}//end stSlimLogicNode<ObjectType, EvaluatorType>::UpdateDistances

//=============================================================================
// Class template stSlimMSTSpliter
//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSlimMSTSplitter<ObjectType, EvaluatorType>::stSlimMSTSplitter(
      tLogicNode * node){

   Node = node;
   N = Node->GetNumberOfEntries();

   // Dynamic fields
   Cluster = new tCluster[N];
   ObjectCluster = new int[N];

   // Matrix
   DMat.SetSize(N, N);
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::stSlimMSTSplitter

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSlimMSTSplitter<ObjectType, EvaluatorType>::~stSlimMSTSplitter(){

   if (Node != 0){
      delete Node;
	  Node = 0;
   }//end if
   if (Cluster != 0){
      delete[] Cluster;
	  Cluster = 0;
   }//end if
   if (ObjectCluster != 0){
      delete[] ObjectCluster;
	  ObjectCluster = 0;
   }//end if
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::stSlimMSTSplitter

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSlimMSTSplitter<ObjectType, EvaluatorType>::BuildDistanceMatrix(
      EvaluatorType * metricEvaluator){
   int i;
   int j;

   for (i = 0; i < N; i++){
      DMat[i][i] = 0;
      for (j = 0; j < i; j++){
         DMat[i][j] = metricEvaluator->GetDistance(*Node->GetObject(i),
                                                   *Node->GetObject(j));
         DMat[j][i] = DMat[i][j];
      }//end for
   }//end for
   return ((1 - N) * N) / 2;
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::BuildDistanceMatrix

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSlimMSTSplitter<ObjectType, EvaluatorType>::FindCenter(int clus){
   int i, j, center;
   double minRadius, radius;

   minRadius = MAXDOUBLE;
   for (i = 0; i < N; i++){
      if (ObjectCluster[i] == clus){
         radius = -1;
         for (j = 0; j < N; j++){
            if ((ObjectCluster[j] == clus) && (radius < DMat[i][j])){
               radius = DMat[i][j];
            }//end if
         }//end for
         if (minRadius > radius){
            minRadius = radius;
            center = i;
         }//end if
      }//end if
   }//end for

   return center;
}//end  stSlimMSTSplitter<ObjectType, EvaluatorType>::FindCenter

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimMSTSplitter<ObjectType, EvaluatorType>::PerformMST(){
   int i, j, k, cc, iBig, iBigOpposite, a, b , c;
   bool linksOk, flag;
   double big;

   // Insert each object in its own cluster.
   cc = N;
   for (i = 0; i < N; i++){
      Cluster[i].Size = 1;
      Cluster[i].State = ALIVE;
      ObjectCluster[i] = i; // Add Object
   }//end for

   // Perform it until it reaches 2 clusters.
   while (cc > 2){
      // Find the minimum distance between a cluster and its nearest
      // neighbour (connections).
      for (i = 0; i < N; i++){
         if (Cluster[i].State != DEAD){
            Cluster[i].MinDist = MAXDOUBLE;
         }//end if
      }//end for
      for (i = 0; i < N; i++){
         k = ObjectCluster[i];
         // Locate the nearest
         for (j = 0; j < N; j++){
            if (ObjectCluster[j] != k){
               if (Cluster[k].MinDist > DMat[i][j]){
                  Cluster[k].MinDist = DMat[i][j];
                  Cluster[k].Src = i;
                  Cluster[k].Dst = j;
               }//end if
            }//end if
         }//end for
      }//end for
      linksOk = true;

      // Find the largest connection. It will also locate the oposite objects
      big = -1.0;
      iBig = 0;
      for (i = 1; i < N; i++){
         if ((Cluster[i].State != DEAD) && (big < Cluster[i].MinDist)){
            big = Cluster[i].MinDist;
            iBig = i;
         }//end if
      }//end for

      // Locate the iBigOpposite.
      iBigOpposite = iBig;
      for (i = 0; i < N; i++){
         if ((Cluster[i].State != DEAD) && (Cluster[i].Src == Cluster[iBig].Dst) &&
               (Cluster[i].Dst == Cluster[iBig].Src)){
            iBigOpposite = i;
         }//end if
      }//end for

      // Join clusters
      i = 0;
      while ((i < N) && (cc > 2)) {
         if ((i != iBig) && (i != iBigOpposite) && (Cluster[i].State != DEAD)){
            // Join cluster i and its nearest cluster.
            k = ObjectCluster[Cluster[i].Dst];
            flag = true;

            // Change the cluster of all objects of the dropped one to
            // the remaining one.
            for (j = 0; j < N; j++){
               if ((ObjectCluster[j] == k) &&
                     (ObjectCluster[j] != ObjectCluster[Cluster[i].Src])){
                  if ((cc == 3) && (flag)){
                     if (!linksOk){
                        // Force update.
                        i = j = N;
                     }else{
                        a = ObjectCluster[Cluster[iBig].Src];
                        b = ObjectCluster[Cluster[iBig].Dst];
                        c = k;
                        if ((c == a) || (c == b)){
                           c = ObjectCluster[Cluster[i].Src];
                        }//end if

                        if ((ObjectCluster[Cluster[c].Src] == a) ||
                              (ObjectCluster[Cluster[c].Dst] == a)){
                           if (Cluster[b].Size > Cluster[c].Size){
                              // Join C and A
                              JoinClusters(c, a);
                           }else{
                              // Join B and A
                              JoinClusters(b , a);
                           }//end if
                        }else{
                           if (Cluster[a].Size > Cluster[c].Size){
                              // Join C and B
                              JoinClusters(c, b);
                           }else{
                              // Join A and B
                              JoinClusters(a, b);
                           }//end if
                        }//end if
                        i = j = N;
                        cc--;
                     }//end if
                  }else{
                     Cluster[k].State = DEATH_SENTENCE;
                     ObjectCluster[j] = ObjectCluster[Cluster[i].Src];
                     if (flag){
                        Cluster[ObjectCluster[Cluster[i].Src]].Size +=
                              Cluster[k].Size;
                        cc--;
                        flag = false;
                        linksOk = false;
                     }//end if
                  }//end if
               }//end if
            }//end for
         }//end if
         // Update i for the next loop
         i++;
      }//end while

      // All clusters that are destiny of an edge has gone, integrated
      // into another one.
      for (i = 0; i < N; i++){
         if (Cluster[i].State == DEATH_SENTENCE){
            Cluster[i].State = DEAD;
         }//end if
      }//end for
   }//end while

   // Locate the name of the 2 clusters.
   Cluster0 = -1;
   Cluster1 = -1;
   for (i = 0; i < N; i++){
      if (Cluster[i].State == ALIVE){
         if (Cluster0 == -1){
            Cluster0 = i;
         }else if (Cluster1 == -1){
            Cluster1 = i;
         }//end if
      }//end if
   }//end for

   #ifdef __stDEBUG__
   // Linking missing objects
   for (i = 0; i < N; i++){
      if ((ObjectCluster[i] != Cluster0) && (ObjectCluster[i] != Cluster1)){
         throw std::logic_error("At least on object has no cluster.");
      }//end if
   }//end for
   #endif //__stDEBUG__

   // Representatives
   Node->SetRepresentative(FindCenter(Cluster0), FindCenter(Cluster1));
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::PerformMST
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSlimMSTSplitter<ObjectType, EvaluatorType>::Distribute(
            stSlimIndexNode * node0, ObjectType * & rep0,
            stSlimIndexNode * node1, ObjectType * & rep1,
            EvaluatorType * metricEvaluator){
   int dCount;
   int idx;
   int i;
   int objIdx;

   // Build Distance matrix
   dCount = BuildDistanceMatrix(metricEvaluator);

   //Perform MST
   PerformMST();

   // Add representatives first
   idx = node0->AddEntry(Node->GetRepresentative(0)->GetSerializedSize(),
                         Node->GetRepresentative(0)->Serialize());
   objIdx = Node->GetRepresentativeIndex(0);
   node0->GetIndexEntry(idx).Distance = 0.0;
   node0->GetIndexEntry(idx).Radius = Node->GetRadius(objIdx);
   node0->GetIndexEntry(idx).NEntries = Node->GetNEntries(objIdx);
   node0->GetIndexEntry(idx).PageID = Node->GetPageID(objIdx);

   idx = node1->AddEntry(Node->GetRepresentative(1)->GetSerializedSize(),
                         Node->GetRepresentative(1)->Serialize());
   objIdx = Node->GetRepresentativeIndex(1);
   node1->GetIndexEntry(idx).Distance = 0.0;
   node1->GetIndexEntry(idx).Radius = Node->GetRadius(objIdx);
   node1->GetIndexEntry(idx).NEntries = Node->GetNEntries(objIdx);
   node1->GetIndexEntry(idx).PageID = Node->GetPageID(objIdx);

   // Distribute us...
   for (i = 0; i < N; i++){
      if (!Node->IsRepresentative(i)){
         if (ObjectCluster[i] == Cluster0){
            idx = node0->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                  Node->GetObject(i)->Serialize());
            if (idx >= 0){
               // Insertion Ok!
               node0->GetIndexEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(0)];
               node0->GetIndexEntry(idx).Radius = Node->GetRadius(i);
               node0->GetIndexEntry(idx).NEntries = Node->GetNEntries(i);
               node0->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }else{
               // Oops! We must put it in other node
               idx = node1->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                     Node->GetObject(i)->Serialize());
               node1->GetIndexEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
               node1->GetIndexEntry(idx).Radius = Node->GetRadius(i);
               node1->GetIndexEntry(idx).NEntries = Node->GetNEntries(i);
               node1->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }//end if
         }else{
            idx = node1->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                  Node->GetObject(i)->Serialize());
            if (idx >= 0){
               // Insertion Ok!
               node1->GetIndexEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
               node1->GetIndexEntry(idx).Radius = Node->GetRadius(i);
               node1->GetIndexEntry(idx).NEntries = Node->GetNEntries(i);
               node1->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }else{
               // Oops! We must put it in other node
               idx = node0->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                     Node->GetObject(i)->Serialize());
               node0->GetIndexEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(0)];
               node0->GetIndexEntry(idx).Radius = Node->GetRadius(i);
               node0->GetIndexEntry(idx).NEntries = Node->GetNEntries(i);
               node0->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }//end if
         }//end if
      }//end if
   }//end for

   // Representatives
   rep0 = Node->BuyObject(Node->GetRepresentativeIndex(0));
   rep1 = Node->BuyObject(Node->GetRepresentativeIndex(1));

   return dCount;
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::Distribute

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSlimMSTSplitter<ObjectType, EvaluatorType>::Distribute(
            stSlimLeafNode * node0, ObjectType * & rep0,
            stSlimLeafNode * node1, ObjectType * & rep1,
            EvaluatorType * metricEvaluator){
   int dCount;
   int idx;
   int i;

   // Build Distance matrix
   dCount = BuildDistanceMatrix(metricEvaluator);

   //Perform MST
   PerformMST();

   // Add representatives first
   idx = node0->AddEntry(Node->GetRepresentative(0)->GetIncludedSerializedSize(),
                         Node->GetRepresentative(0)->IncludedSerialize());
   node0->GetLeafEntry(idx).Distance = 0.0;
   idx = node1->AddEntry(Node->GetRepresentative(1)->GetIncludedSerializedSize(),
                         Node->GetRepresentative(1)->IncludedSerialize());
   node1->GetLeafEntry(idx).Distance = 0.0;

   // Distribute us...
   for (i = 0; i < N; i++){
      if (!Node->IsRepresentative(i)){
         if (ObjectCluster[i] == Cluster0){
            idx = node0->AddEntry(Node->GetObject(i)->GetIncludedSerializedSize(),
                                  Node->GetObject(i)->IncludedSerialize());
            if (idx >= 0){
               // Insertion Ok!
               node0->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(0)];
            }else{
               // Oops! We must put it in other node
               idx = node1->AddEntry(Node->GetObject(i)->GetIncludedSerializedSize(),
                                     Node->GetObject(i)->IncludedSerialize());
               node1->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
            }//end if
         }else{
            idx = node1->AddEntry(Node->GetObject(i)->GetIncludedSerializedSize(),
                                  Node->GetObject(i)->IncludedSerialize());
            if (idx >= 0){
               // Insertion Ok!
               node1->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
            }else{
               // Oops! We must put it in other node
               idx = node0->AddEntry(Node->GetObject(i)->GetIncludedSerializedSize(),
                                     Node->GetObject(i)->IncludedSerialize());
               node0->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(0)];
            }//end if
         }//end if
      }//end if
   }//end for

   // Representatives
   rep0 = Node->BuyObject(Node->GetRepresentativeIndex(0));
   rep1 = Node->BuyObject(Node->GetRepresentativeIndex(1));
   
   return dCount;
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::Distribute

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimMSTSplitter<ObjectType, EvaluatorType>::JoinClusters(
      int cluster1, int cluster2){
   int i;

   for (i = 0; i < N; i++){
      if (ObjectCluster[i] == cluster2){
         ObjectCluster[i] = cluster1;
      }//end if
   }//end for
   Cluster[cluster1].Size += Cluster[cluster2].Size;
   Cluster[cluster2].State = DEATH_SENTENCE;
}//end stSlimMSTSplitter<ObjectType, EvaluatorType>::JoinClusters


//==============================================================================
// Class stSlimTree
//------------------------------------------------------------------------------

// This macro will be used to replace the declaration of
//       stSlimTree<ObjectType, EvaluatorType>
#define tmpl_stSlimTree stSlimTree<ObjectType, EvaluatorType>

template <class ObjectType, class EvaluatorType>
tmpl_stSlimTree::stSlimTree(stPageManager * pageman):
   stMetricTree<ObjectType, EvaluatorType>(pageman){

   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   // Load header.
   LoadHeader();

   this->maxQueue = 0;

   // Will I create or load the tree ?
   if (tMetricTree::myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if

   this->plotSplitSequence = 0;
   
   // Visualization support
   #ifdef __stMAMVIEW__
      MAMViewer = new tViewExtractor(this->myMetricEvaluator);
   #endif //__stMAMVIEW__
}//end stSlimTree<ObjectType, EvaluatorType>::stSlimTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
tmpl_stSlimTree::stSlimTree(stPageManager * pageman, EvaluatorType * metricEval):
   stMetricTree<ObjectType, EvaluatorType>(pageman, metricEval){

   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   // Load header.
   LoadHeader();

   // Will I create or load the tree ?
   if (tMetricTree::myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if

   // Visualization support
   #ifdef __stMAMVIEW__
   MAMViewer = new tViewExtractor(this->myMetricEvaluator);
   #endif //__stMAMVIEW__
}//end stSlimTree<ObjectType, EvaluatorType>::stSlimTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
tmpl_stSlimTree::~stSlimTree(){

   // Flus header page.
   FlushHeader();

   // Visualization support
   #ifdef __stMAMVIEW__
   delete MAMViewer;
   MAMViewer = 0;
   #endif //__stMAMVIEW__
}//end stSlimTree<ObjectType, EvaluatorType>::~stSlimTree()

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::DefaultHeader(){

   // Clear header page.
   HeaderPage->Clear();

   // Default values
   Header->Magic[0] = 'S';
   Header->Magic[1] = 'L';
   Header->Magic[2] = 'I';
   Header->Magic[3] = 'M';
   Header->SplitMethod = smSPANNINGTREE;
   Header->ChooseMethod = cmMINOCCUPANCY;
   Header->CorrectMethod = crmOFF;
   Header->Root = 0;
   Header->MinOccupation = 0.25;
   Header->MaxOccupation = 0;
   Header->Height = 0;
   Header->ObjectCount = 0;
   Header->NodeCount = 0;

   // Notify modifications
   HeaderUpdate = true;
}//end stSlimTree<ObjectType, EvaluatorType>::DefaultHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::LoadHeader(){

   if (HeaderPage != NULL){
      tMetricTree::myPageManager->ReleasePage(HeaderPage);
   }//end if

   // Load and set the header.
   HeaderPage = tMetricTree::myPageManager->GetHeaderPage();
   if (HeaderPage->GetPageSize() < sizeof(stSlimHeader)){
      #ifdef __stDEBUG__
         cout << "The page size is too small. Increase it!\n";
      #endif //__stDEBUG__
      throw std::logic_error("The page size is too small.");
   }//end if

   Header = (stSlimHeader *) HeaderPage->GetData();
   HeaderUpdate = false;
}//end stSlimTree<ObjectType, EvaluatorType>::LoadHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::FlushHeader(){

   if (HeaderPage != 0){
      if (Header != 0){
         WriteHeader();
      }//end if
      tMetricTree::myPageManager->ReleasePage(HeaderPage);
	  HeaderPage = 0;
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::FlushHeader

//------------------------------------------------------------------------------
#ifdef __stFRACTALQUERY__
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::ResetFractalStatistics(){
   int i;

   GoodGuesses = 0;
   for (i = 0; i < SIZERINGCALLS; i++){
      RingCalls[i] = 0;
   }//end for
}//end ResetFractalStatistics
#endif //__stFRACTALQUERY__


//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::Add(ObjectType *newObj){
   //cout << "No add \n";
   stSubtreeInfo promo1;
   stSubtreeInfo promo2;
   int insertIdx;

   // Is there a root ?
   if (this->GetRoot() == 0){
      // No! We shall create the new node.
      stPage * auxPage  = this->NewPage();
      stSlimLeafNode * leafNode = new stSlimLeafNode(auxPage, true);
      this->SetRoot(auxPage->GetPageID());

      // Insert the new object.
      insertIdx = leafNode->AddEntry(newObj->GetIncludedSerializedSize(),
                                     newObj->IncludedSerialize());
      // Test if the page size is too big to store an object.
      if (insertIdx < 0){
         // Oops. There is an error during the insertion.
         #ifdef __stDEBUG__
            cout << "The page size is too small for the first object. Increase it!\n";
            // Throw an exception.
            throw std::logic_error("The page size is too small to store the first object.");
         #endif //__stDEBUG__
         // The new object was not inserted.
         return false;
      }else{
         // The new object was inserted.
         // It is the first object, fill the distance with zero.
         leafNode->GetLeafEntry(insertIdx).Distance = 0;
         // Update the Height
         Header->Height++;
         // Write the root node.
         tMetricTree::myPageManager->WritePage(auxPage);
      }//end if
      delete leafNode;
	  leafNode = 0;
      delete auxPage;
	  auxPage = 0;
   }else{
      // Let's continue our search for the grail!
      if (InsertRecursive(GetRoot(), newObj, NULL, promo1, promo2) == PROMOTION){
         // Split occurred! We must create a new root because it is required.
         // The tree will aacquire a new root.
        // std::cout<< "Add new root" << "\n";
         AddNewRoot(promo1.Rep, promo1.Radius, promo1.RootID, promo1.NObjects,
                    promo2.Rep, promo2.Radius, promo2.RootID, promo2.NObjects);
         delete promo1.Rep;
		 promo1.Rep = 0;
         delete promo2.Rep;
		 promo2.Rep = 0;
      }//end if
   }//end if

   // Update object count.
   UpdateObjectCounter(1);

   // Report the modification.
   HeaderUpdate = true;
   // Ok. The new object was inserted. Return success!
   return true;
}//end stSlimTree<ObjectType, EvaluatorType>::Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stSlimTree::ChooseSubTree(
      stSlimIndexNode * slimIndexNode, ObjectType * obj) {
   u_int32_t idx;
   //int j;
   int * candidates;
   int candidateCount;
   bool stop;
   u_int32_t tmpNumberOfEntries;
   int numberOfEntries, minIndex = 0;

   ObjectType * objectType = new ObjectType;
   double distance;
   double minDistance = MAXDOUBLE; // Largest magnitude double value
   // Get the total number of entries.
   numberOfEntries = slimIndexNode->GetNumberOfEntries();
   idx = 0;

   switch (this->GetChooseMethod()){
      case stSlimTree::cmBIASED :
         // Find the first subtree that covers the new object.
         stop = (idx >= numberOfEntries);
         while (!stop){
            // Get the object from idx position from IndexNode
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);
            // is this a subtree that covers the new object?
            if (distance < slimIndexNode->GetIndexEntry(idx).Radius) {
               minDistance = 0;     // the gain will be 0
               stop = true;         // stop the search.
               minIndex = idx;
            }else if (distance - slimIndexNode->GetIndexEntry(idx).Radius < minDistance) {
               minDistance = distance - slimIndexNode->GetIndexEntry(idx).Radius;
               minIndex = idx;
            }//end if
            idx++;
            // if one of the these condicions are true, stop this while.
            stop = stop || (idx >= numberOfEntries);
         }//end while
         break; //end stSlimTree::cmBIASED

      case stSlimTree::cmRANDOM :
         // Note: I replaced the previous algorithm with a better one by adding a new
		 // random selection procedure. This procedure will create a list of all possible
	     // candidates, adding them to the candidates vector. After that, it will select
         // a random position from the list.
		  
		 // allocate resources to cover.
         candidates = new int[numberOfEntries];
         candidateCount = 0;

		 /* Find if there is some circle that contains obj */
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Recover the object from the IndexNode.
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance between the object and the candidate nodes.
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);
            
			// Do this node cover obj?
            if (distance < (slimIndexNode->GetIndexEntry(idx).Radius)){
               // Yes, add it.
               candidates[candidateCount] = idx;
			   candidateCount++;
            }//end if
         }//end for

		 // Did I find candidates?
		 if (candidateCount) {
			// Yes. I'll choose one at will.
			minIndex = candidates[rand() % candidateCount];
		 } else {
			// No options! Select one at will.
			// TODO 
			minIndex = rand() % numberOfEntries;
		 }

		 // clean cover.
         delete[] candidates;
		 candidates = NULL;
         break; // end stSlimTree::cmRANDOM

      case stSlimTree::cmMINDIST :
         /* Find if there is some circle that contains obj */
         stop = (idx >= numberOfEntries);
         while (!stop){
            //get out the object from IndexNode
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);
            // find the first subtree that cover the new object.
            if (distance < slimIndexNode->GetIndexEntry(idx).Radius) {
               minDistance = distance;     // the gain will be 0
               stop = true;                // stop the search.
               minIndex = idx;
            }else if (distance - slimIndexNode->GetIndexEntry(idx).Radius < minDistance) {
               minDistance = distance - slimIndexNode->GetIndexEntry(idx).Radius;
               minIndex = idx;
            }//end if
            idx++;
            // if one of the these condicions are true, stop this while.
            stop = stop || (idx >= numberOfEntries);
         }//end while
         // Try to find a better entry.
         while (idx < numberOfEntries) {
            // Get out the object from IndexNode.
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance.                                    
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);
            if ((distance < slimIndexNode->GetIndexEntry(idx).Radius) && (distance < minDistance)) {
               minDistance = distance;
               minIndex = idx;
            }//end if
            idx++;
         }//end while
         break; // end stSlimTree::cmMINDIST

      case stSlimTree::cmMINGDIST :
         // Find if there is some circle that contains obj
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Get out the object from IndexNode.
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);
            if (distance < minDistance) {
               minDistance = distance;
               minIndex = idx;
            }//end if
         }//end for
         break; //end stSlimTree::cmMINGDIST

      case stSlimTree::cmMINOCCUPANCY :
         /* Find if there is some circle that contains obj */
         tmpNumberOfEntries = MAXINT;
         // First try to find a subtree that covers the new object.
         stop = (idx >= numberOfEntries);
         while (!stop){
            //get out the object from IndexNode
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);
            // find the first subtree that covers the new object.
            if (distance < slimIndexNode->GetIndexEntry(idx).Radius) {
               minDistance = distance;     // the gain will be 0
               stop = true;                // stop the search.
               minIndex = idx;
               tmpNumberOfEntries = slimIndexNode->GetIndexEntry(idx).NEntries;
            }else if (distance - slimIndexNode->GetIndexEntry(idx).Radius < minDistance) {
               minDistance = distance - slimIndexNode->GetIndexEntry(idx).Radius;
               minIndex = idx;
            }//end if
            idx++;
            // if one of the these condicions are true, stop this while.
            stop = stop || (idx >= numberOfEntries);
         }//end while

         while (idx < numberOfEntries) {
            // Get out the object from IndexNode
            objectType->Unserialize(slimIndexNode->GetObject(idx),
                                    slimIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(*objectType, *obj);

            if ((distance < slimIndexNode->GetIndexEntry(idx).Radius) &&
                (slimIndexNode->GetIndexEntry(idx).NEntries < tmpNumberOfEntries)) {
               tmpNumberOfEntries = slimIndexNode->GetIndexEntry(idx).NEntries;
               minIndex = idx;
            }//end if
            idx++;
         }//end for
         break; //end stSlimTree::cmMINOCCUPANCY

   }//end switch

   delete objectType;
   objectType = 0;

   return minIndex;
}//end stSlimTree<ObjectType, EvaluatorType>::ChooseSubTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::AddNewRoot(
      ObjectType * obj1, double radius1, u_int32_t nodeID1, u_int32_t nEntries1,
      ObjectType * obj2, double radius2, u_int32_t nodeID2, u_int32_t nEntries2){
   stPage * newPage;
   stSlimIndexNode * newRoot;
   int idx;

   // Debug mode!
   #ifdef __stDEBUG__
      if ((obj1 == NULL) || (obj2 == NULL)){
         throw std::logic_error("Invalid object.");
      }//end if
   #endif //__stDEBUG__

   // Create a new node
   newPage = this->NewPage();
   newRoot = new stSlimIndexNode(newPage, true);

   // Add obj1
   idx = newRoot->AddEntry(obj1->GetSerializedSize(), obj1->Serialize());
   newRoot->GetIndexEntry(idx).Distance = 0.0;
   newRoot->GetIndexEntry(idx).PageID = nodeID1;
   newRoot->GetIndexEntry(idx).Radius = radius1;
   newRoot->GetIndexEntry(idx).NEntries = nEntries1;

   // Add obj2
   idx = newRoot->AddEntry(obj2->GetSerializedSize(), obj2->Serialize());
   newRoot->GetIndexEntry(idx).Distance = 0.0;
   newRoot->GetIndexEntry(idx).PageID = nodeID2;
   newRoot->GetIndexEntry(idx).Radius = radius2;
   newRoot->GetIndexEntry(idx).NEntries = nEntries2;

   // Update tree
   Header->Height++;
   SetRoot(newRoot->GetPage()->GetPageID());
   tMetricTree::myPageManager->WritePage(newPage);

   // Dispose page
   delete newRoot;
   newRoot = 0;
   tMetricTree::myPageManager->ReleasePage(newPage);
}//end SlimTree::AddNewRoot

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stSlimTree::InsertRecursive(
      u_int32_t currNodeID, ObjectType * newObj, ObjectType * repObj,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2){
   stPage * currPage;      // Current page
   stPage * newPage;       // New page
   stSlimNode * currNode;  // Current node
   stSlimIndexNode * indexNode; // Current index node.
   stSlimIndexNode * newIndexNode; // New index node for splits
   stSlimLeafNode * leafNode; // Current leaf node.
   stSlimLeafNode * newLeafNode; // New leaf node.
   int insertIdx;          // Insert index.
   int result;             // Returning value.
   double dist;        // Temporary distance.
   int subtree;            // Subtree
   ObjectType * subRep;    // Subtree representative.
   // Read node...
   currPage = tMetricTree::myPageManager->GetPage(currNodeID);
   currNode = stSlimNode::CreateNode(currPage);

   // What shall I do ?
   if (currNode->GetNodeType() == stSlimNode::INDEX){
      // Index Node cast.
      indexNode = (stSlimIndexNode *)currNode;

      // Where do I add it ?
      subtree = ChooseSubTree(indexNode, newObj);

      // Lets get the information about this tree.
      subRep = new ObjectType();
      subRep->Unserialize(indexNode->GetObject(subtree),
                          indexNode->GetObjectSize(subtree));

      // Try to insert...
      switch (InsertRecursive(indexNode->GetIndexEntry(subtree).PageID,
            newObj, subRep, promo1, promo2)){
         case NO_ACT: // Update Radius and count.
            indexNode->GetIndexEntry(subtree).NEntries++;
            indexNode->GetIndexEntry(subtree).Radius = promo1.Radius;

            // Returning status.
            promo1.NObjects = indexNode->GetTotalObjectCount();
            promo1.Radius = indexNode->GetMinimumRadius();
            result = NO_ACT;
            break;
         case CHANGE_REP: // Replace representative
            // Remove previous entry.
            indexNode->RemoveEntry(subtree);

            // Try to add the new entry...
            insertIdx = indexNode->AddEntry(promo1.Rep->GetSerializedSize(),
                                            promo1.Rep->Serialize());
            if (insertIdx >= 0){
               // Swap OK. Fill data.
               indexNode->GetIndexEntry(insertIdx).Radius = promo1.Radius;
               indexNode->GetIndexEntry(insertIdx).NEntries = promo1.NObjects;
               indexNode->GetIndexEntry(insertIdx).PageID = promo1.RootID;

               // Will it replace the representative ?
               // WARNING: Do not change the order of this checking.
               if ((repObj != NULL) && (repObj->IsEqual(subRep))){
                  // promo1.Rep is the new representative.
                  indexNode->GetIndexEntry(insertIdx).Distance = 0;

                  // Oops! We must propagate the representative change
                  // promo1.Rep will remain the same.
                  promo1.RootID = currNodeID;

                  // update distances between the new representative
                  // and the others
                  UpdateDistances(indexNode, promo1.Rep, insertIdx);

                  result = CHANGE_REP;
               }else{
                  // promo1.Rep is not the new representative.
                  if (repObj != NULL){
                     // Distance from representative is...
                     indexNode->GetIndexEntry(insertIdx).Distance =
                           this->myMetricEvaluator->GetDistance(*repObj, 
                                                                *promo1.Rep);
                  }else{
                     // It is the root!
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;
                  }//end if

                  // Cut it here
                  delete promo1.Rep; // promo1.rep will never be used again.
                  promo1.Rep = 0;
                  result = NO_ACT;
               }//end if
               promo1.Radius = indexNode->GetMinimumRadius();
               promo1.NObjects = indexNode->GetTotalObjectCount();
            }else{
               // Split it!
               // New node.
               newPage = this->NewPage();
               newIndexNode = new stSlimIndexNode(newPage, true);

               // Split!
               SplitIndex(indexNode, newIndexNode,
                     promo1.Rep, promo1.Radius, promo1.RootID, promo1.NObjects,
                     NULL, 0, 0, 0,
                     repObj, promo1, promo2);

               // Write nodes
               tMetricTree::myPageManager->WritePage(newPage);
               // Clean home.
               delete newIndexNode;
			   newIndexNode = 0;
               tMetricTree::myPageManager->ReleasePage(newPage);
               result = PROMOTION; //Report split.
            }//end if
            break;
         case PROMOTION: // Promotion!!!
            if (promo1.Rep == NULL){
               // Update subtree
               indexNode->GetIndexEntry(subtree).NEntries = promo1.NObjects;
               indexNode->GetIndexEntry(subtree).Radius = promo1.Radius;
               indexNode->GetIndexEntry(subtree).PageID = promo1.RootID;

               // Try to insert the promo2.Rep
               insertIdx = indexNode->AddEntry(promo2.Rep->GetSerializedSize(),
                                               promo2.Rep->Serialize());
               if (insertIdx >= 0){
                  // Swap OK. Fill data.
                  indexNode->GetIndexEntry(insertIdx).NEntries = promo2.NObjects;
                  indexNode->GetIndexEntry(insertIdx).Radius = promo2.Radius;
                  indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;
                  // Update promo2 distance
                  if (repObj != NULL){
                     // Distance from representative is...
                     indexNode->GetIndexEntry(insertIdx).Distance =
                           this->myMetricEvaluator->GetDistance(*repObj, 
                                                                *promo2.Rep);
                  }else{
                     // It is the root!
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;
                  }//end if

                  //Update radius...
                  promo1.Radius = indexNode->GetMinimumRadius();
                  promo1.NObjects = indexNode->GetTotalObjectCount();
                  delete promo2.Rep;
				  promo2.Rep = 0;
                  result = NO_ACT;
               }else{
                  // Split it!
                  // New node.
                  newPage = this->NewPage();
                  newIndexNode = new stSlimIndexNode(newPage, true);

                  // Split!
                  SplitIndex(indexNode, newIndexNode,
                        promo2.Rep, promo2.Radius, promo2.RootID, promo2.NObjects,
                        NULL, 0, 0, 0,
                        repObj, promo1, promo2);

                  // Write nodes
                  tMetricTree::myPageManager->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
				  newIndexNode = 0;
                  tMetricTree::myPageManager->ReleasePage(newPage);
                  result = PROMOTION; //Report split.
               }//end if
            }else{
               // Remove the previous entry.
               indexNode->RemoveEntry(subtree);

               // Try to add the new entry...
               insertIdx = indexNode->AddEntry(promo1.Rep->GetSerializedSize(),
                                               promo1.Rep->Serialize());
               if (insertIdx >= 0){
                  // Swap OK. Fill data.
                  indexNode->GetIndexEntry(insertIdx).Radius = promo1.Radius;
                  indexNode->GetIndexEntry(insertIdx).NEntries = promo1.NObjects;
                  indexNode->GetIndexEntry(insertIdx).PageID = promo1.RootID;

                  // Will it replace the representative ?
                  // WARNING: Do not change the order of this checking.
                  if ((repObj != NULL) && (repObj->IsEqual(subRep))){
                     // promo1.Rep is the new representative.
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;

                     // Oops! We must propagate the representative change
                     // promo1.Rep will remains the same,
                     promo1.RootID = currNodeID;

                     // update distances between the new representative
                     // and the others
                     UpdateDistances(indexNode, promo1.Rep, insertIdx);

                     result = CHANGE_REP;
                  }else{
                     // promo1.Rep is not the new representative.
                     if (repObj != NULL){
                        // Distance from representative is...
                        indexNode->GetIndexEntry(insertIdx).Distance =
                              this->myMetricEvaluator->GetDistance(*repObj,
                                                                   *promo1.Rep);
                     }else{
                        // It is the root!
                        indexNode->GetIndexEntry(insertIdx).Distance = 0;
                     }//end if

                     // Cut it here
                     delete promo1.Rep; // promo1.rep will never be used again.
                     promo1.Rep = 0;
                     result = NO_ACT;
                  }//end if

                  // Try to add promo2
                  insertIdx = indexNode->AddEntry(promo2.Rep->GetSerializedSize(),
                                                  promo2.Rep->Serialize());
                  if (insertIdx >= 0){
                     // Swap OK. Fill data.
                     indexNode->GetIndexEntry(insertIdx).Radius = promo2.Radius;
                     indexNode->GetIndexEntry(insertIdx).NEntries = promo2.NObjects;
                     indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;

                     // The new distance is...
                     if (promo1.Rep != NULL){
                        // Rep. changed...
                        // Distance from representative is...
                        indexNode->GetIndexEntry(insertIdx).Distance =
                              this->myMetricEvaluator->GetDistance(*promo1.Rep,
                                                                   *promo2.Rep);
                     }else{
                        // No change!
                        if (repObj != NULL){
                           // Distance from representative is...
                           indexNode->GetIndexEntry(insertIdx).Distance =
                                 this->myMetricEvaluator->GetDistance(*repObj,
                                                                      *promo2.Rep);
                        }else{
                           // It is the root!
                           indexNode->GetIndexEntry(insertIdx).Distance = 0;
                        }//end if
                     }//end if

                     delete promo2.Rep;
					 promo2.Rep = 0;
                     // set the number of objects to high levels.
                     promo1.NObjects = indexNode->GetTotalObjectCount();
                     // set the radius to high levels.
                     promo1.Radius = indexNode->GetMinimumRadius();
                  }else{
                     // Split it promo2.rep does not fit.
                     // New node.
                     newPage = this->NewPage();
                     newIndexNode = new stSlimIndexNode(newPage, true);

                     // Dispose promo1.rep it if exists because it will not be
                     // used again. It happens when result is CHANGE_REP.
                     if (promo1.Rep != 0){
                        delete promo1.Rep;
						promo1.Rep = 0;
                     }//end if

                     // Add promo2 and split!
                     SplitIndex(indexNode, newIndexNode,
                           promo2.Rep, promo2.Radius, promo2.RootID, promo2.NObjects,
                           NULL, 0, 0, 0, // Ignore this object
                           repObj, promo1, promo2);

                     // Write nodes
                     tMetricTree::myPageManager->WritePage(newPage);
                     // Clean home.
                     delete newIndexNode;
					 newIndexNode = 0;
                     tMetricTree::myPageManager->ReleasePage(newPage);
                     result = PROMOTION; //Report split.
                  }//end if
               }else{
                  // Split it because both objects don't fit.
                  // New node.
                  newPage = this->NewPage();
                  newIndexNode = new stSlimIndexNode(newPage, true);

                  // Split!
                  SplitIndex(indexNode, newIndexNode,
                        promo1.Rep, promo1.Radius, promo1.RootID, promo1.NObjects,
                        promo2.Rep, promo2.Radius, promo2.RootID, promo2.NObjects,
                        repObj, promo1, promo2);

                  // Write nodes
                  tMetricTree::myPageManager->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
				  newIndexNode = 0;
                  tMetricTree::myPageManager->ReleasePage(newPage);
                  result = PROMOTION; //Report split.
               }//end if
            }//end if
      };//end switch

      // Clear the mess.
      delete subRep;
	  subRep = 0;
   }else{
      // Leaf node cast.
      leafNode = (stSlimLeafNode *) currNode;

      // Try to insert...
      insertIdx = leafNode->AddEntry(newObj->GetIncludedSerializedSize(),
                                     newObj->IncludedSerialize());
      if (insertIdx >= 0){
         // Don't split!
         // Calculate distance and verify if it is a new radius!
         if (repObj == NULL){
            dist = 0;
         }else{
            dist = this->myMetricEvaluator->GetDistance(*newObj, *repObj);
         }//end if

         // Fill entry's fields
         leafNode->GetLeafEntry(insertIdx).Distance = dist;

         // Write node.
         tMetricTree::myPageManager->WritePage(currPage);

         // Returning values
         promo1.Rep = NULL;
         promo1.Radius = leafNode->GetMinimumRadius();
         promo1.RootID = currNodeID;
         promo1.NObjects = leafNode->GetNumberOfEntries();
         result = NO_ACT;
      }else{
         // Split it!
         // New node.
         newPage = this->NewPage();
         newLeafNode = new stSlimLeafNode(newPage, true);

         // Split!
         SplitLeaf(leafNode, newLeafNode, (ObjectType *)newObj->Clone(),
                   repObj, promo1, promo2);

         // Write node.
         tMetricTree::myPageManager->WritePage(newPage);
         // Clean home.
         delete newLeafNode;
		 newLeafNode = 0;
         tMetricTree::myPageManager->ReleasePage(newPage);
		 newPage = 0;
         result = PROMOTION; //Report split.
      }//end if
   }//end if

   // Write node.
   tMetricTree::myPageManager->WritePage(currPage);
   // Clean home
   delete currNode;
   currNode = 0;
   tMetricTree::myPageManager->ReleasePage(currPage);
   currPage = 0;
   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::InsertRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::RandomPromote(tLogicNode * node) {

   u_int32_t idx1, idx2;
   u_int32_t numberOfEntries = node->GetNumberOfEntries();
   
   // generate a number between 0 to numberOfEntries-1 fo idx1
   idx1 = (rand() % numberOfEntries);

   // Choose another random index such that idx2 != idx1
   do {
	   idx2 = (rand() % numberOfEntries);
   } while (idx2 == idx1);

   // Choose representatives
   node->SetRepresentative(idx1, idx2);
}//end stSlimTree<ObjectType, EvaluatorType>::RandomPromote

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MinMaxPromote(tLogicNode * node) {

   double iRadius, jRadius, min;
   u_int32_t numberOfEntries, idx1, idx2, i, j;
   stPage * newPage1 = new stPage(tMetricTree::myPageManager->GetMinimumPageSize());
   stPage * newPage2 = new stPage(tMetricTree::myPageManager->GetMinimumPageSize());

   numberOfEntries = node->GetNumberOfEntries();
   min = MAXDOUBLE;   // Largest magnitude double value

   // Is it an Index node?
   if (node->GetNodeType() == stSlimNode::INDEX) {
      stSlimIndexNode * indexNode1 = new stSlimIndexNode(newPage1, true);
      stSlimIndexNode * indexNode2 = new stSlimIndexNode(newPage2, true);

      for (i = 0; i < numberOfEntries; i++) {
         for (j = i + 1; j < numberOfEntries; j++) {
            node->SetRepresentative(i, j);
            indexNode1->RemoveAll();
            indexNode2->RemoveAll();
            node->TestDistribution(indexNode1, indexNode2, this->myMetricEvaluator);
            iRadius = indexNode1->GetMinimumRadius();
            jRadius = indexNode2->GetMinimumRadius();
            if (iRadius < jRadius){
               iRadius = jRadius;      // take the maximum
            }//end if
            if (iRadius < min) {
               min = iRadius;
               idx1 = i;
               idx2 = j;
            }//end if
         }//end for
      }//end for
      delete indexNode1;
	  indexNode1 = 0;
      delete indexNode2;
	  indexNode2 = 0;
   }else{//it is a Leaf node
      stSlimLeafNode * leafNode1 = new stSlimLeafNode(newPage1, true);
      stSlimLeafNode * leafNode2 = new stSlimLeafNode(newPage2, true);

      for (i = 0; i < numberOfEntries; i++) {
         for (j = i + 1; j < numberOfEntries; j++) {
            node->SetRepresentative(i, j);
            leafNode1->RemoveAll();
            leafNode2->RemoveAll();
            node->TestDistribution(leafNode1, leafNode2, this->myMetricEvaluator);
            iRadius = leafNode1->GetMinimumRadius();
            jRadius = leafNode2->GetMinimumRadius();
            if (iRadius < jRadius){
               iRadius = jRadius;      // take the maximum
            }//end if
            if (iRadius < min) {
               min = iRadius;
               idx1 = i;
               idx2 = j;
            }//end if
         }//end for
      }//end for
      delete leafNode1;
	  leafNode1 = 0;
      delete leafNode2;
	  leafNode2 = 0;
   }//end else

   // Choose representatives
   node->SetRepresentative(idx1, idx2);

   delete newPage1;
   newPage1 = 0;
   delete newPage2;
   newPage2 = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::MinMaxPromote

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MapSplit(stSlimLeafNode * oldNode, ObjectType *newObj) {

    typedef stFastMapper < ObjectType, EvaluatorType > myFastMapper;
    myFastMapper *fm = new myFastMapper(this->GetMetricEvaluator(),2);
    
    u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();

    ObjectType **objects = new ObjectType*[numberOfEntries+1];
    for (u_int32_t i = 0; i < numberOfEntries; i++) {
        objects[i] = new ObjectType();
        objects[i]->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
    }
    objects[numberOfEntries] = newObj;
    
    fm->ChoosePivots(objects,numberOfEntries+1);
    
    char filename[50];
    sprintf(filename,"%d-before-split-leaf.plt",plotSplitSequence);
    FILE *f = fopen(filename,"wt");
    fprintf(f,"set title \"Before split %d\"\n",plotSplitSequence);
    fprintf(f,"plot '-' title 'Old node' with points, '-' title 'New element' with points\n");
    fprintf(f,"# Old node\n");
    
    double *mapped = new double[3];
    for (u_int32_t i = 0; i < numberOfEntries; i++) {
        fm->Map(objects[i], mapped);
        fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
    }

    fprintf(f,"end\n");
    
    fprintf(f,"# New element\n");
    fm->Map(newObj, mapped);
    fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
    fprintf(f,"end\n");
    fprintf(f,"pause -1\n");
    
    fclose(f);
    
    // clean up memory
    delete fm;
    delete[]mapped;

    for (u_int32_t i = 0; i < numberOfEntries; i++) {
        delete objects[i];
    }
    delete[]objects;
}

template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MapSplit(stSlimLeafNode *oldNode, ObjectType *lRep, stSlimLeafNode *newNode, ObjectType *rRep) {

    typedef stFastMapper < ObjectType, EvaluatorType > myFastMapper;
    myFastMapper *fm = new myFastMapper(this->GetMetricEvaluator(),2);
    
    u_int32_t oldNodeNumberOfEntries = oldNode->GetNumberOfEntries();
    u_int32_t newNodeNumberOfEntries = newNode->GetNumberOfEntries();

    ObjectType **allobjects = new ObjectType*[oldNodeNumberOfEntries+newNodeNumberOfEntries];
    for (u_int32_t i = 0; i < oldNodeNumberOfEntries; i++) {
        allobjects[i] = new ObjectType();
        allobjects[i]->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
    }
    for (u_int32_t i = oldNodeNumberOfEntries; i < oldNodeNumberOfEntries+newNodeNumberOfEntries; i++) {
        allobjects[i] = new ObjectType();
        allobjects[i]->Unserialize(newNode->GetObject(i-oldNodeNumberOfEntries),newNode->GetObjectSize(i-oldNodeNumberOfEntries));
    }

    fm->ChoosePivots(allobjects,oldNodeNumberOfEntries+newNodeNumberOfEntries);
    
    char filename[50];
    sprintf(filename,"%d-after-split-leaf.plt",plotSplitSequence);
    FILE *f = fopen(filename,"wt");
    fprintf(f,"set title \"After split %d (left=%d, right=%d elements)\"\n",plotSplitSequence,oldNodeNumberOfEntries,newNodeNumberOfEntries);
    fprintf(f,"plot ");
    fprintf(f,"'-' title 'left node' with points, '-' title 'left representative' with points, ");
    fprintf(f,"'-' title 'right node' with points, '-' title 'right representative' with points\n");
    
    double *mapped = new double[3];

    fprintf(f,"# left node\n");
    for (u_int32_t i = 0; i < oldNodeNumberOfEntries; i++) {
        ObjectType *obj = new ObjectType();
        obj->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
        fm->Map(obj, mapped);
        fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
        delete obj;
    }
    
    fprintf(f,"end\n");
    
    fprintf(f,"# left representative\n");
    fm->Map(lRep, mapped);
    fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
    fprintf(f,"end\n");

    fprintf(f,"# right node\n");
    for (u_int32_t i = 0; i < newNodeNumberOfEntries; i++) {
        ObjectType *obj = new ObjectType();
        obj->Unserialize(newNode->GetObject(i),newNode->GetObjectSize(i));
        fm->Map(obj, mapped);
        fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
        delete obj;
    }
    
    fprintf(f,"end\n");
    
    fprintf(f,"# right representative\n");
    fm->Map(rRep, mapped);
    fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
    fprintf(f,"end\n");
    
    fprintf(f,"pause -1\n");
    
    fclose(f);
    
    // clean up memory
    delete fm;
    delete[]mapped;

    for (u_int32_t i = 0; i < oldNodeNumberOfEntries+newNodeNumberOfEntries; i++) {
        delete allobjects[i];
    }
    delete[]allobjects;

    this->plotSplitSequence++;
}

template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MapSplit(stSlimIndexNode *oldNode, ObjectType *lRep, stSlimIndexNode *newNode, ObjectType *rRep) {

    typedef stFastMapper < ObjectType, EvaluatorType > myFastMapper;
    myFastMapper *fm = new myFastMapper(this->GetMetricEvaluator(),2);
    
    u_int32_t oldNodeNumberOfEntries = oldNode->GetNumberOfEntries();
    u_int32_t newNodeNumberOfEntries = newNode->GetNumberOfEntries();

    ObjectType **allobjects = new ObjectType*[oldNodeNumberOfEntries+newNodeNumberOfEntries];
    for (u_int32_t i = 0; i < oldNodeNumberOfEntries; i++) {
        allobjects[i] = new ObjectType();
        allobjects[i]->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
    }
    for (u_int32_t i = oldNodeNumberOfEntries; i < oldNodeNumberOfEntries+newNodeNumberOfEntries; i++) {
        allobjects[i] = new ObjectType();
        allobjects[i]->Unserialize(newNode->GetObject(i-oldNodeNumberOfEntries),newNode->GetObjectSize(i-oldNodeNumberOfEntries));
    }

    fm->ChoosePivots(allobjects,oldNodeNumberOfEntries+newNodeNumberOfEntries);
    
    char filename[50];
    sprintf(filename,"%d-after-split-index.plt",plotSplitSequence);
    FILE *f = fopen(filename,"wt");
    fprintf(f,"set title \"After split %d (left=%d, right=%d elements)\"\n",plotSplitSequence,oldNodeNumberOfEntries,newNodeNumberOfEntries);
    fprintf(f,"plot ");
    fprintf(f,"'-' title 'left node' with points, '-' title 'left representative' with points, ");
    fprintf(f,"'-' title 'right node' with points, '-' title 'right representative' with points\n");
    
    double *mapped = new double[3];

    fprintf(f,"# left node\n");
    for (u_int32_t i = 0; i < oldNodeNumberOfEntries; i++) {
        ObjectType *obj = new ObjectType();
        obj->Unserialize(oldNode->GetObject(i),oldNode->GetObjectSize(i));
        fm->Map(obj, mapped);
        fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
        delete obj;
    }
    
    fprintf(f,"end\n");
    
    fprintf(f,"# left representative\n");
    fm->Map(lRep, mapped);
    fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
    fprintf(f,"end\n");

    fprintf(f,"# right node\n");
    for (u_int32_t i = 0; i < newNodeNumberOfEntries; i++) {
        ObjectType *obj = new ObjectType();
        obj->Unserialize(newNode->GetObject(i),newNode->GetObjectSize(i));
        fm->Map(obj, mapped);
        fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
        delete obj;
    }
    
    fprintf(f,"end\n");
    
    fprintf(f,"# right representative\n");
    fm->Map(rRep, mapped);
    fprintf(f,"%f %f %f\n",mapped[0],mapped[1],mapped[2]);
    fprintf(f,"end\n");
    
    fprintf(f,"pause -1\n");
    
    fclose(f);
    
    // clean up memory
    delete fm;
    delete[]mapped;

    for (u_int32_t i = 0; i < oldNodeNumberOfEntries+newNodeNumberOfEntries; i++) {
        delete allobjects[i];
    }
    delete[]allobjects;

    this->plotSplitSequence++;
}

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::SplitLeaf(
      stSlimLeafNode * oldNode, stSlimLeafNode * newNode,
      ObjectType * newObj, ObjectType * prevRep,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2) {
   tLogicNode * logicNode;
   tMSTSplitter * mstSplitter;
   ObjectType * lRep;
   ObjectType * rRep;
   u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();

   //std::cout << "Split leaf \n";


   // Create the new tLogicNode
   logicNode = new tLogicNode(numberOfEntries + 1);
   logicNode->SetMinOccupation((u_int32_t )(GetMinOccupation() * (numberOfEntries + 1)));
   logicNode->SetNodeType(stSlimNode::LEAF);

   // update the maximum number of entries.
   this->SetMaxOccupation(numberOfEntries);

   #ifdef SPLITREEMAPSPLIT
   MapSplit(oldNode,newObj);
   #endif

   // Add objects
   logicNode->AddLeafNode(oldNode);
   logicNode->AddEntry(newObj);

   // Split it.
   switch (GetSplitMethod()) {
      case stSlimTree::smRANDOM:
         this->RandomPromote(logicNode);
         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
		 logicNode = 0;
         break; //end stSlimTree::smRANDOM
      case stSlimTree::smMINMAX:
         this->MinMaxPromote(logicNode);
         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
		 logicNode = 0;
         break;  //end stSlimTree::smMINMAX
      case stSlimTree::smSPANNINGTREE:
         // MST Split
         mstSplitter = new tMSTSplitter(logicNode);
         // Perform MST
         oldNode->RemoveAll();
         mstSplitter->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         // Clean home
         delete mstSplitter;
		 mstSplitter = 0;
         break; //end stSlimTree::smSPANNINGTREE
      #ifdef __stDEBUG__
      default:
         throw std::logic_error("There is no Split method selected.");
      #endif //__stDEBUG__
   };//end switch

   #ifdef SPLITREEMAPSPLIT
   MapSplit(oldNode,lRep,newNode,rRep);
   #endif
   
   // Update fields. We may need to change lRep and rRep.
   if (prevRep == NULL){
      // This is a root. The order of lRep and rRep is not important.
      promo1.Rep = lRep;
      promo1.Radius = oldNode->GetMinimumRadius();
      promo1.RootID = oldNode->GetPageID();
      promo1.NObjects = oldNode->GetTotalObjectCount();
      promo2.Rep = rRep;
      promo2.Radius = newNode->GetMinimumRadius();
      promo2.RootID = newNode->GetPageID();
      promo2.NObjects = newNode->GetTotalObjectCount();
   }else{
      // Let's see if it is necessary to change things.
      if (prevRep->IsEqual(lRep)){
         // lRep is the prevRep. Delete it.
         delete lRep;
		 lRep = 0;
         promo1.Rep = NULL;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo1.NObjects = oldNode->GetTotalObjectCount();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
         promo2.NObjects = newNode->GetTotalObjectCount();
      }else if (prevRep->IsEqual(rRep)){
         // rRep is the prevRep. Delete it.
         delete rRep;
		 rRep = 0;
         promo2.Rep = lRep;
         promo2.Radius = oldNode->GetMinimumRadius();
         promo2.RootID = oldNode->GetPageID();
         promo2.NObjects = oldNode->GetTotalObjectCount();
         promo1.Rep = NULL;
         promo1.Radius = newNode->GetMinimumRadius();
         promo1.RootID = newNode->GetPageID();
         promo1.NObjects = newNode->GetTotalObjectCount();
      }else{
         // This is a root. The order of lRep and rRep is not important.
         promo1.Rep = lRep;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo1.NObjects = oldNode->GetTotalObjectCount();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
         promo2.NObjects = newNode->GetTotalObjectCount();
      }//end if
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::SplitLeaf

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::SplitIndex(
      stSlimIndexNode * oldNode, stSlimIndexNode * newNode,
      ObjectType * newObj1, double newRadius1,
      u_int32_t newNodeID1, u_int32_t newNEntries1,
      ObjectType * newObj2, double newRadius2,
      u_int32_t newNodeID2, u_int32_t newNEntries2,
      ObjectType * prevRep,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2){
   tLogicNode * logicNode;
   tMSTSplitter * mstSplitter;
   ObjectType * lRep;
   ObjectType * rRep;
   u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();

   std::cout << "Split index \n";

   // Create the new tLogicNode
   logicNode = new tLogicNode(numberOfEntries + 2);
   logicNode->SetMinOccupation((u_int32_t )(GetMinOccupation() * (numberOfEntries + 2)));
   logicNode->SetNodeType(stSlimNode::INDEX);

   // update the maximum number of entries.
   this->SetMaxOccupation(numberOfEntries);

   // Add objects
   logicNode->AddIndexNode(oldNode);

   // Add newObj1
   logicNode->AddEntry(newObj1);
   logicNode->SetEntry(logicNode->GetNumberOfEntries() - 1,
         newNodeID1, newNEntries1, newRadius1);

   // Will I add newObj2 ?
   if (newObj2 != NULL){
      logicNode->AddEntry(newObj2);
      logicNode->SetEntry(logicNode->GetNumberOfEntries() - 1,
            newNodeID2, newNEntries2, newRadius2);
   }//end if

   // Split it.
   switch (GetSplitMethod()) {
      case stSlimTree::smRANDOM:
         this->RandomPromote(logicNode);
         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
		 logicNode = 0;
         break; //end stSlimTree::smRANDOM
      case stSlimTree::smMINMAX:
         this->MinMaxPromote(logicNode);
         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
		 logicNode = 0;
         break;  //end stSlimTree::smMINMAX
      case stSlimTree::smSPANNINGTREE:
         // MST Split
         mstSplitter = new tMSTSplitter(logicNode);
         // Perform MST
         oldNode->RemoveAll();
         mstSplitter->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         // Clean home
         delete mstSplitter;
		 mstSplitter = 0;
         break; //end stSlimTree::smSPANNINGTREE
      #ifdef __stDEBUG__
      default:
         throw std::logic_error("There is no Split method selected.");
      #endif //__stDEBUG__
   };//end switch

   #ifdef SPLITREEMAPSPLIT
   MapSplit(oldNode,lRep,newNode,rRep);
   #endif
   
   // Update fields. We may need to change lRep and rRep.
   if (prevRep == NULL){
      // This is a root. The order of lRep and rRep is not important.
      promo1.Rep = lRep;
      promo1.Radius = oldNode->GetMinimumRadius();
      promo1.RootID = oldNode->GetPageID();
      promo1.NObjects = oldNode->GetTotalObjectCount();
      promo2.Rep = rRep;
      promo2.Radius = newNode->GetMinimumRadius();
      promo2.RootID = newNode->GetPageID();
      promo2.NObjects = newNode->GetTotalObjectCount();
   }else{
      // Let's see if it is necessary to change things.
      if (prevRep->IsEqual(lRep)){
         // lRep is the prevRep. Delete it.
         delete lRep;
		 lRep = 0;
         promo1.Rep = NULL;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo1.NObjects = oldNode->GetTotalObjectCount();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
         promo2.NObjects = newNode->GetTotalObjectCount();
      }else if (prevRep->IsEqual(rRep)){
         // rRep is the prevRep. Delete it.
         delete rRep;
		 rRep = 0;
         promo2.Rep = lRep;
         promo2.Radius = oldNode->GetMinimumRadius();
         promo2.RootID = oldNode->GetPageID();
         promo2.NObjects = oldNode->GetTotalObjectCount();
         promo1.Rep = NULL;
         promo1.Radius = newNode->GetMinimumRadius();
         promo1.RootID = newNode->GetPageID();
         promo1.NObjects = newNode->GetTotalObjectCount();
      }else{
         // This is a root. The order of lRep and rRep is not important.
         promo1.Rep = lRep;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo1.NObjects = oldNode->GetTotalObjectCount();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
         promo2.NObjects = newNode->GetTotalObjectCount();
      }//end if
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::SplitIndex

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::UpdateDistances(stSlimIndexNode * node,
            ObjectType * repObj, u_int32_t repObjIdx){
   u_int32_t i;
   ObjectType * tempObj = new ObjectType();

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      if (i != repObjIdx){
         tempObj->Unserialize(node->GetObject(i), node->GetObjectSize(i));
         node->GetIndexEntry(i).Distance =
            this->myMetricEvaluator->GetDistance(*repObj, *tempObj);
      }else{
         //it's the representative object
         node->GetIndexEntry(i).Distance = 0.0;
      }//end if
   }//end for

   //clean the house before exit.
   delete tempObj;
   tempObj = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::UpdateDistances

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetDistanceLimit(){
   double distance = 0;
   double distanceTemp = 0;
   u_int32_t i, j;
   ObjectType * object1 = new ObjectType();
   ObjectType * object2 = new ObjectType();
   stPage * currPage;
   stSlimNode * currNode;
   stSlimIndexNode * indexNode;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes.
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Index Node cast.
      indexNode = (stSlimIndexNode *)currNode;

      //search every entry in the root node
      for (i = 0; i < indexNode->GetNumberOfEntries() - 1; i++){
         //get a object
         object1->Unserialize(indexNode->GetObject(i), indexNode->GetObjectSize(i));
         //combine with the others objects
         for (j = i + 1; j < indexNode->GetNumberOfEntries(); j++){
            //get the other object
            object2->Unserialize(indexNode->GetObject(j),
                                 indexNode->GetObjectSize(j));
            //calculate the distance of the two objects
            distanceTemp = this->myMetricEvaluator->GetDistance(*object1, *object2);
            //sum the distance with the distance of the two
            distanceTemp = distanceTemp + indexNode->GetIndexEntry(i).Radius +
                                      indexNode->GetIndexEntry(j).Radius;
            //if this sum is greater than the previous...
            if (distanceTemp > distance)
               //store it to return this value
               distance = distanceTemp;
         }//end for
      }//end for
   }//end if

   //cleaning...
   delete object1;
   object1 = 0;
   delete object2;
   object2 = 0;

   //return the maximum distance between 2 objects of this tree
   return distance;
}//end stSlimTree<ObjectType, EvaluatorType>::GetDistanceLimit

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetGreaterEstimatedDistance(){
   double distance = 0;
   double distanceTemp = 0;
   u_int32_t idx, idx2;
   ObjectType ** objects;
   u_int32_t size = 0;
   stPage * currPage, * currPage2;
   stSlimNode * currNode, * currNode2;
   stSlimIndexNode * indexNode, * indexNode2;
   stSlimLeafNode * leafNode;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes. Read node...
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         indexNode = (stSlimIndexNode *)currNode;
         // Estimate the maximum number of entries.
         objects = new ObjectType * [this->GetMaxOccupation() * indexNode->GetNumberOfEntries()];
         // For each entry...
         for (idx = 0; idx < indexNode->GetNumberOfEntries(); idx++) {
            // Get the pages.
            currPage2 = tMetricTree::myPageManager->GetPage(indexNode->GetIndexEntry(idx).PageID);
            currNode2 = stSlimNode::CreateNode(currPage2);
            // Is it am index node?
            if (currNode2->GetNodeType() == stSlimNode::INDEX) {
               // Get Index node
               indexNode2 = (stSlimIndexNode *)currNode2;

               // For each entry...
               for (idx2 = 0; idx2 < indexNode2->GetNumberOfEntries(); idx2++) {
                  // Rebuild the object
                  objects[size] = new ObjectType();
                  objects[size++]->Unserialize(indexNode2->GetObject(idx2),
                                               indexNode2->GetObjectSize(idx2));
               }//end for
            }else{
               // No, it is a leaf node. Get it.
               leafNode = (stSlimLeafNode *)currNode2;
               // For each entry...
               for (idx2 = 0; idx2 < leafNode->GetNumberOfEntries(); idx2++) {
                  // Rebuild the object
                  objects[size] = new ObjectType();
                  objects[size++]->Unserialize(leafNode->GetObject(idx2),
                                               leafNode->GetObjectSize(idx2));
               }//end for
            }//end if
            // Free it all
            delete currNode2;
			currNode2 = 0;
            tMetricTree::myPageManager->ReleasePage(currPage2);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         leafNode = (stSlimLeafNode *)currNode;
         // Estimate the maximum number of entries.
         objects = new ObjectType * [this->GetMaxOccupation() * leafNode->GetNumberOfEntries()];
         // For each entry...
         for (idx = 0; idx < leafNode->GetNumberOfEntries(); idx++) {
            // Rebuild the object
            objects[size] = new ObjectType();
            objects[size++]->Unserialize(leafNode->GetObject(idx),
                                         leafNode->GetObjectSize(idx));
         }//end for
      }//end else

      // Search in all entries in the second level.
      for (idx = 0; idx < size-1; idx++){
         //combine with the others objects
         for (idx2 = idx+1; idx2 < size; idx2++){
            // Calculate the distance of the two objects
            distanceTemp = this->myMetricEvaluator->GetDistance(objects[idx], objects[idx2]);
            // If this sum is greater than the previous...
            if (distanceTemp > distance)
               //store it to return this value
               distance = distanceTemp;
         }//end for
      }//end for

      // Cleaning...
      for (idx = 0; idx < size; idx++){
         delete objects[idx];
		 objects[idx] = 0;
      }//end for
      delete[] objects;
	  objects = 0;
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   //return the maximum distance between 2 objects of this tree
   return distance;
}//end stSlimTree<ObjectType, EvaluatorType>::GetGreaterEstimatedDistance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetGreaterDistance(){
   double greaterDistance;
   double distanceTemp;
   u_int32_t idx, idx2;
   ObjectType ** objects;
   u_int32_t size;

   greaterDistance = 0;
   // Is there a root ?
   if (this->GetRoot()){
      distanceTemp = 0;
      size = 0;
      // Allocate the maximum number of entries.
      objects = new ObjectType * [this->GetNumberOfObjects()];
      // Call the GetGreaterDistance recursively.
      this->GetGreaterDistance(this->GetRoot(), objects, size);

      // Search in all entries.
      for (idx = 0; idx < size - 1; idx++){
         //combine with the others objects
         for (idx2 = idx + 1; idx2 < size; idx2++){
            // Calculate the distance of the two objects
            distanceTemp = this->myMetricEvaluator->GetDistance(objects[idx], objects[idx2]);
            // If this sum is greater than the previous...
            if (distanceTemp > greaterDistance)
               //store it to return this value
               greaterDistance = distanceTemp;
         }//end for
      }//end for

      // Cleaning...
      for (idx = 0; idx < size; idx++){
         delete objects[idx];
		 objects[idx] = 0;
      }//end for
      delete[] objects;
	  objects = 0;
   }//end if

   //return the maximum distance between 2 objects of this tree
   return greaterDistance;
}//end stSlimTree<ObjectType, EvaluatorType>::GetGreaterDistance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GetGreaterDistance(u_int32_t pageID, ObjectType ** objects,
                                         u_int32_t & size){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze it!
            this->GetGreaterDistance(indexNode->GetIndexEntry(idx).PageID, objects, size);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Put it in the objects set.
            objects[size++] = tmpObj.Clone();
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::GetGreaterDistance

#ifdef __stDISKACCESSSTATS__
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetEstimateDiskAccesses(double range,
               double fractalDimension, double maxDistance){
               
   stPage * rootPage;
   stSlimNode * rootNode;
   u_int32_t idx, numberOfEntries;
   double diskAccesses;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes, read the root node...
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);

      // Is it an Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX){
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze the subtree.
            diskAccesses += pow((indexNode->GetIndexEntry(idx).Radius + range), fractalDimension) +
                            this->GetEstimateDiskAccesses(
                              indexNode->GetIndexEntry(idx).PageID, range, fractalDimension);
         }//end for
      }//end if
      // Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
      // divide with the maxDistance.
      diskAccesses = diskAccesses / pow(maxDistance/2.0, fractalDimension);
   }//end if

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetEstimateDiskAccesses(u_int32_t pageID, double range,
                                                double fractalDimension){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   double diskAccesses;

   // Default value.
   diskAccesses = 0.0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            diskAccesses += pow((indexNode->GetIndexEntry(idx).Radius + range), fractalDimension) +
                            this->GetEstimateDiskAccesses(indexNode->GetIndexEntry(idx).PageID,
                                                          range, fractalDimension);
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetFastEstimateDiskAccesses(double range,
               double fractalDimension, double maxDistance){

   u_int32_t idx;
   long numberOfObjects;
   double diskAccesses;
   int h;
   u_int32_t height;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Get the total number of entries in the tree.
      numberOfObjects = this->GetNumberOfObjects();
      // Get the height.
      height = this->GetHeight();

      // For each level of the tree.
      for (h = 0; h < height; h++){
         diskAccesses += pow(numberOfObjects,((double) h)/height) *
                         pow((pow(numberOfObjects,
                             ((double) -h) / (height * fractalDimension)) + range),
                              fractalDimension);
      }//end for
      // Get the average.
      diskAccesses = diskAccesses / pow(maxDistance/2.0, fractalDimension);
   }//end if

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetFastEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetFatFactorFastEstimateDiskAccesses(double fatFactor,
         double range, double fractalDimension, double maxDistance){

   u_int32_t idx;
   long numberOfObjects;
   double diskAccesses;
   int h;
   u_int32_t height;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Get the total number of entries in the tree.
      numberOfObjects = this->GetNumberOfObjects();
      // Get the height.
      height = this->GetHeight();

      // For each level of the tree.
      for (h = 0; h < height; h++){
         diskAccesses += (1.0 + fatFactor * (pow(numberOfObjects,(((double) h)/(height-1.0)))-1.0)) *
                          pow(numberOfObjects,((double) h)/height) *
                          pow((pow(numberOfObjects,
                                   ((double) -h) / (height * fractalDimension)) + range),
                              fractalDimension);
      }//end for
      // Get the average.
      diskAccesses = diskAccesses / pow(maxDistance/2.0, fractalDimension);
   }//end if

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetFatFactorFastEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetCiacciaEstimateDiskAccesses(double range,
               tHistogram * histogram){
               
   stPage * rootPage;
   stSlimNode * rootNode;
   u_int32_t idx, numberOfEntries;
   double diskAccesses;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes, read the root node...
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);

      // Is it an Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX){
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze the subtree.
            diskAccesses += histogram->GetValueOfBin(indexNode->GetIndexEntry(idx).Radius + range) +
                            this->GetCiacciaEstimateDiskAccesses(
                                  indexNode->GetIndexEntry(idx).PageID, range, histogram);
         }//end for
      }//end if
      // Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
   }//end if

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetCiacciaEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetCiacciaEstimateDiskAccesses(u_int32_t pageID, double range,
                                                tHistogram * histogram){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   double diskAccesses;

   // Default value.
   diskAccesses = 0.0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            diskAccesses += histogram->GetValueOfBin(indexNode->GetIndexEntry(idx).Radius + range) +
                            this->GetCiacciaEstimateDiskAccesses(indexNode->GetIndexEntry(idx).PageID,
                                                                 range, histogram);
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetCiacciaEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetCiacciaEstimateDistCalculation(double range,
               tHistogram * histogram){
               
   stPage * rootPage;
   stSlimNode * rootNode;
   u_int32_t idx, numberOfEntries;
   double distCalc;

   // Default value. The root node is always accessed.
   distCalc = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes, read the root node...
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);

      // Is it an Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX){
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze the subtree.
            distCalc += histogram->GetValueOfBin(indexNode->GetIndexEntry(idx).Radius + range) +
                         this->GetCiacciaEstimateDistCalculation(
                               indexNode->GetIndexEntry(idx).PageID, range, histogram);
         }//end for
      }//end if
      // Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
   }//end if

   // return the estimation of disk accesses.
   return distCalc;
}//end GetCiacciaEstimateDistCalculation

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetCiacciaEstimateDistCalculation(u_int32_t pageID, double range,
                                                tHistogram * histogram){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   double distCalc;

   // Default value.
   distCalc = 0.0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            distCalc += histogram->GetValueOfBin(indexNode->GetIndexEntry(idx).Radius + range) +
                        this->GetCiacciaEstimateDistCalculation(indexNode->GetIndexEntry(idx).PageID,
                                                                range, histogram);
         }//end for
      }else{
         // Is it a leaf node.
         // Get the Index node
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            distCalc += histogram->GetValueOfBin(range);
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   // return the estimation of disk accesses.
   return distCalc;
}//end GetCiacciaEstimateDistCalculation

//---------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::CalculateLevelStatistics(stLevelDiskAccess * levelDiskAccess){
   stPage * rootPage;
   stSlimNode * rootNode;
   u_int32_t idx, numberOfEntries;
   u_int32_t height;

   // Is there a root ?
   if (this->GetRoot()){
      height = 0;
      // Yes, read the root node...
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);

      // Is it an Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX){
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Add the radius.
            levelDiskAccess->AddEntry(indexNode->GetIndexEntry(idx).Radius, height);
            // Analyze the subtree.
            this->CalculateLevelStatistics(levelDiskAccess,
                     indexNode->GetIndexEntry(idx).PageID, height + 1);
         }//end for
      }//end if
      // Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
   }//end if

   levelDiskAccess->Sumarize();
}//end CalculateLevelStatistics

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::CalculateLevelStatistics(stLevelDiskAccess * levelDiskAccess,
         u_int32_t pageID, u_int32_t height){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();
         // Add the number of nodes.
         levelDiskAccess->AddNumberOfNodes(height-1);
         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Add the radius.
            levelDiskAccess->AddEntry(indexNode->GetIndexEntry(idx).Radius, height);
            // Analyze the subtree.
            this->CalculateLevelStatistics(levelDiskAccess,
                     indexNode->GetIndexEntry(idx).PageID, height + 1);
         }//end for
      }else{
         // Add the number of nodes.
         levelDiskAccess->AddNumberOfNodes(height-1);
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end CalculateLevelStatistics

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetCiacciaLevelEstimateDiskAccesses(
               stLevelDiskAccess * levelDiskAccess, double range,
               tHistogram * histogram){
               
   u_int32_t idx, numberOfEntries;
   double diskAccesses;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Analyze the disk access for each level.
   for (idx = 0; idx < levelDiskAccess->GetHeight(); idx++) {
      // Analyze the subtree.
      diskAccesses += levelDiskAccess->GetNumberOfNodes(idx) *
                      histogram->GetValueOfBin(levelDiskAccess->GetAvgRadius(idx) + range);
   }//end for

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetCiacciaLevelEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GenerateLevelHistograms(tHistogram ** histogram){
   // Let's search
   if (this->GetRoot()){
      this->GenerateLevelHistograms(histogram, this->GetRoot(), 0);
   }//end if
}//end GenerateLevelHistograms

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GenerateLevelHistograms(tHistogram ** histogram,
               u_int32_t pageID, u_int32_t height){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   ObjectType obj;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            obj.Unserialize(indexNode->GetObject(idx),
                            indexNode->GetObjectSize(idx));
            // Add the object.
            histogram[height]->Add(&obj);
            // Analyze the subtree.
            this->GenerateLevelHistograms(histogram, indexNode->GetIndexEntry(idx).PageID, height + 1);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
//            if (drand() <= 0.1){
               // Rebuild the object
               obj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
               // Add the object.
               histogram[height]->Add(&obj);
//            }//end if
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end GenerateLevelHistograms

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetTestLevelEstimateDiskAccesses(
               stLevelDiskAccess * levelDiskAccess, tHistogram ** histogram,
               double range){

   u_int32_t idx, numberOfEntries;
   double diskAccesses;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Analyze the disk access for each level.
   for (idx = 0; idx < levelDiskAccess->GetHeight(); idx++) {
      // Estimate the disk access.
      diskAccesses += levelDiskAccess->GetNumberOfNodes(idx) *
                      histogram[idx]->GetValueOfBin(levelDiskAccess->GetAvgRadius(idx) + range);
   }//end for

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetTestLevelEstimateDiskAccesses

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GenerateSampledLevelHistograms(tHistogram ** histogram){
   // Let's search
   if (this->GetRoot()){
      this->GenerateSampledLevelHistograms(histogram, this->GetRoot(), 0);
   }//end if
}//end GenerateSampledLevelHistograms

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GenerateSampledLevelHistograms(tHistogram ** histogram,
               u_int32_t pageID, u_int32_t height){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   ObjectType obj;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get the Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         // Get the total number of entries in the root node.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            obj.Unserialize(indexNode->GetObject(idx),
                            indexNode->GetObjectSize(idx));
            // Add the object.
            histogram[height]->Add(&obj);
            // Analyze the subtree.
            this->GenerateSampledLevelHistograms(histogram, indexNode->GetIndexEntry(idx).PageID, height + 1);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Test if there is space to store this new object.
            if (histogram[height]->GetNumberOfObjects() < 0.1 * this->GetNumberOfObjects()){
               // Test if I will add this new object.
               if (drand() <= 0.1){
                  // Rebuild the object
                  obj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
                  // Add the object.
                  histogram[height]->Add(&obj);
               }//end if
            }//end if
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end GenerateSampledLevelHistograms

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetSampledLevelEstimateDiskAccesses(
               stLevelDiskAccess * levelDiskAccess, tHistogram ** histogram,
               double range){

   u_int32_t idx, numberOfEntries;
   double diskAccesses;

   // Default value. The root node is always accessed.
   diskAccesses = 1;

   // Analyze the disk access for each level.
   for (idx = 0; idx < levelDiskAccess->GetHeight(); idx++) {
      // Estimate the disk access.
      diskAccesses += levelDiskAccess->GetNumberOfNodes(idx) *
                      histogram[idx]->GetValueOfBin(levelDiskAccess->GetAvgRadius(idx) + range);
   }//end for

   // return the estimation of disk accesses.
   return diskAccesses;
}//end GetSampledLevelEstimateDiskAccesses
#endif //__stDISKACCESSSTATS__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stSlimTree::GetIndexNodeCount(){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx, numberOfEntries;
   long nodeCount = 1;

   // Is there a root ?
   if (this->GetRoot()){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetIndexNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for

      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return
   return nodeCount;
}//end stSlimTree<ObjectType, EvaluatorType>::GetIndexNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stSlimTree::GetIndexNodeCount(u_int32_t pageID){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   long nodeCount = 0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         nodeCount += 1;
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetIndexNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return nodeCount;
}//end stSlimTree<ObjectType, EvaluatorType>::GetIndexNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stSlimTree::GetLeafNodeCount(){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx, numberOfEntries;
   long nodeCount = 0;

   // Is there a root ?
   if (this->GetRoot()){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetLeafNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for

      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return
   return nodeCount;
}//end stSlimTree<ObjectType, EvaluatorType>::GetLeafNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
long tmpl_stSlimTree::GetLeafNodeCount(u_int32_t pageID){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   long nodeCount = 0;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze this subtree.
            nodeCount += this->GetLeafNodeCount(indexNode->GetIndexEntry(idx).PageID);
         }//end for
      }else{
         nodeCount = 1;
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return nodeCount;
}//end stSlimTree<ObjectType, EvaluatorType>::GetLeafNodeCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetAvgRadiusLeaf(){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx, numberOfEntries;
   double avg = 0;
   u_int32_t height = this->GetHeight();

   // Is there a root ?
   if (this->GetRoot()){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            if (height == 2){ //stSlimNode::INDEX
               // Analyze this subtree.
               avg += this->GetAvgRadiusLeaf(indexNode->GetIndexEntry(idx).PageID, height-1);
            }else{
               // Analyze this subtree.
               avg += indexNode->GetIndexEntry(idx).Radius;
            }//endif
         }//end for
         //avg
         avg /= numberOfEntries;

      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return
   return avg;
}//end stSlimTree<ObjectType, EvaluatorType>::GetAvgRadiusLeaf

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GetAvgRadiusLeaf(u_int32_t pageID, u_int32_t height){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   double avg = 0;

   // Let's search
   if (pageID != 0){
      // Is it an Index node?
      if (height > 1){ //stSlimNode::INDEX
         // Get Index node
         currPage = this->myPageManager->GetPage(pageID);
         currNode = stSlimNode::CreateNode(currPage);
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            if (height == 2){ //stSlimNode::INDEX
               // Analyze this subtree.
               avg += this->GetAvgRadiusLeaf(indexNode->GetIndexEntry(idx).PageID, height-1);
            }else{
               // Analyze this subtree.
               avg += indexNode->GetIndexEntry(idx).Radius;
            }//endif
         }//end for
         //avg
         avg /= numberOfEntries;
         // Free it all
         delete currNode;
		 currNode = 0;
         this->myPageManager->ReleasePage(currPage);
      }//end if
   }//end if

   return avg;
}//end stSlimTree<ObjectType, EvaluatorType>::GetAvgRadiusLeaf

//==============================================================================
// Begin of Queries
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::AggregateDistanceToSlimNode(double numerator, double denominator,
    ObjectType ** sampleList, u_int32_t sampleSize, ObjectType *representativeObject, double nodeRadius, double *weights){

    if (denominator == 0)
        return 0;

    int idy;
    double distance = 0, a, b;

    bool delweights = false;
    if (weights == NULL) {
       delweights = true;
       weights = new double[sampleSize];
       for (idy = 0; idy < sampleSize; idy++)
           weights[idy] = 1.0;
    }

    if ((numerator == MAXDOUBLE) && (denominator == 1)) { // INFINITY
        b = nodeRadius;
        for (idy = 0; idy < sampleSize; idy++) {
            a = this->myMetricEvaluator->GetDistance(representativeObject, sampleList[idy]);
            if (weights[idy] * (a - b) > distance)
                distance = weights[idy] * (a - b);
        }
    }
    else if ((numerator == -MAXDOUBLE) && (denominator == 1)) { // minus INFINITY
        distance = MAXDOUBLE;
        b = nodeRadius;
        for (idy = 0; idy < sampleSize; idy++) {
            a = this->myMetricEvaluator->GetDistance(representativeObject, sampleList[idy]);
            if (weights[idy] * (a - b) < distance)
                distance = weights[idy] * (a - b);
        }
    }
    else if (numerator/denominator > 0) {
        b = nodeRadius;
        for (idy = 0; idy < sampleSize; idy++) {
            a = this->myMetricEvaluator->GetDistance(representativeObject, sampleList[idy]);
            if (a > b)
                distance += weights[idy] * pow(fabs(a - b), numerator/denominator);
        }
        if (distance != 0)
            distance = pow(fabs(distance), denominator/numerator);
    }
    else if (numerator/denominator < 0) {
        b = nodeRadius;
        double num = 1;
        double den = 0;
        for (idy = 0; idy < sampleSize; idy++) {
            a = this->myMetricEvaluator->GetDistance(representativeObject, sampleList[idy]);
            if (a > b) {
                num = num * pow(fabs(a - b), fabs(numerator));
                den = den + pow(fabs(a - b), fabs(numerator));
            }
            else {
                num = 0;
                idy = sampleSize;
            }
        }
        if (den != 0)
           distance = num / den;
        if (distance != 0)
            distance = pow(fabs(distance), 1.0/fabs(numerator));
    }

    if (delweights) {
        delete[]weights;
        weights = NULL;
    }

    return distance;
}
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::GroupDistance(
          double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, ObjectType *obj, double *weights) {

    if (denominator == 0)
        return 0;

    u_int32_t idx;
    double distance = 0.0, tmpdistance;

    bool delweights = false;
    if (weights == NULL) {
       delweights = true;
       weights = new double[sampleSize];
       for (idx = 0; idx < sampleSize; idx++)
           weights[idx] = 1.0;
    }

    if ((numerator == MAXDOUBLE) && (denominator == 1)) { // INFINITY
        for (idx = 0; idx < sampleSize; idx++) {
            // Evaluate distance
            tmpdistance = this->myMetricEvaluator->GetDistance(obj, sampleList[idx]);
            if (weights[idx] * tmpdistance > distance) {
                distance = weights[idx] * tmpdistance;
            }
        }//end for
    }
    else if ((numerator == -MAXDOUBLE) && (denominator == 1)) { // minus INFINITY
        distance = MAXDOUBLE;
        // For each sample object
        for (idx = 0; idx < sampleSize; idx++) {
            // Evaluate distance
            tmpdistance = this->myMetricEvaluator->GetDistance(obj, sampleList[idx]);
            if (weights[idx] * tmpdistance < distance) {
                distance = weights[idx] * tmpdistance;
            }
        }//end for
    }
    else { 
       // For each sample object
       for (idx = 0; idx < sampleSize; idx++) {
          // Evaluate distance
          tmpdistance = this->myMetricEvaluator->GetDistance(obj, sampleList[idx]);
          if (tmpdistance != 0)
              distance += weights[idx] * pow(fabs(tmpdistance),numerator/denominator);
       }//end for
       if (distance != 0)
           distance = pow(fabs(distance),denominator/numerator);
    }

    if (delweights) {
        delete[]weights;
        weights = NULL;
    }

    return distance;
}//end GroupDistance
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::AggregateRangeQuery(
          double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, double range, double *weights) {

   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, idy;
   double distance, a, b;

   // Set the information.
   //result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   if (weights == NULL) {
       weights = new double[sampleSize];
       for (idx = 0; idx < sampleSize; idx++)
           weights[idx] = 1.0;
   }

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         for (idx = 0; idx < indexNode->GetNumberOfEntries(); idx++) {
            tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
            distance = AggregateDistanceToSlimNode(numerator, denominator, sampleList, sampleSize, &tmpObj, indexNode->GetIndexEntry(idx).Radius, weights);

            // test if this subtree qualifies.
            if (distance <= range) {
                this->AggregateRangeQuery(indexNode->GetIndexEntry(idx).PageID, result, numerator, denominator, sampleList, sampleSize, range, weights);
            }
         }//end for
      }
      else {
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         for (idx = 0; idx < leafNode->GetNumberOfEntries(); idx++) {
            tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
            distance = GroupDistance(numerator, denominator, sampleList, sampleSize, &tmpObj, weights);
            // is it a object that qualified?
            if (distance <= range){
               result->AddPair(tmpObj.Clone(), distance);
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
   return result;
}//end AggregateRangeQuery
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::AggregateRangeQuery(u_int32_t pageID,
         tResult * result, double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, double range, double *weights){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance, a, b;
   u_int32_t idx, idy;
   u_int32_t numberOfEntries;

   if (pageID != 0){
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
               tmpObj.Unserialize(indexNode->GetObject(idx), indexNode->GetObjectSize(idx));
               distance = AggregateDistanceToSlimNode(numerator, denominator, sampleList, sampleSize, &tmpObj, indexNode->GetIndexEntry(idx).Radius, weights);

               // is this a qualified subtree?
               if (distance <= range) {
                  this->AggregateRangeQuery(indexNode->GetIndexEntry(idx).PageID, result, numerator, denominator, sampleList, sampleSize, range, weights);
               }
         }//end for
      }
      else {
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
             tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
             distance = GroupDistance(numerator, denominator, sampleList, sampleSize, &tmpObj, weights);
             if (distance <= range) {
                // Yes! Put it in the result set.
                result->AddPair(tmpObj.Clone(), distance);
             }
         }
      }

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::AggregateNearestQuery(
          double numerator, double denominator, ObjectType ** sampleList, u_int32_t sampleSize, u_int32_t k, bool tie, double *weights) {

   tResult * result = new tResult();  // Create result
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;
   double rangeK = MAXDOUBLE;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   
   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            //if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <= rangeK + indexNode->GetIndexEntry(idx).Radius){
               
                // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),indexNode->GetObjectSize(idx));

               // Evaluate distance
               distance = AggregateDistanceToSlimNode(numerator, denominator, sampleList, sampleSize, &tmpObj, indexNode->GetIndexEntry(idx).Radius, weights);

               if (distance <= rangeK) {
               //if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTmpValue);
               }//end if
            //}//end if
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            //if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= rangeK){

                // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),leafNode->GetObjectSize(idx));

               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance
               distance = GroupDistance(numerator, denominator, sampleList, sampleSize, &tmpObj, weights);
               
               //test if the object qualify
               if (distance <= rangeK){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->Cut(k);
                     //may I use this for performance?
                     rangeK = result->GetMaximumDistance();
                  }//end if
               }//end if
            //}//end if
         }//end for

      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);

      // Go to next node
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            // Qualified if distance <= rangeK + radius
            if (distance <= rangeK + pqCurrValue.Radius){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
   
   return result;
}//end AggregateNearestQuery
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stSlimTree::ForwardRangeQueryWithoutPriority(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){
   tResultPaged * result = new tResultPaged();  // Create result
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;
   double external;

   external = externalRadius;

   // Set the information.
   result->SetQueryInfo(sample->Clone(), NEXTRANGEQUERY, nObj, externalRadius, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // test external qualifies.
            if ( ((distance - indexNode->GetIndexEntry(idx).Radius) <= external ) &&
                  // test internal qualifies.
                  (( distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
               // Yes! Analyze this subtree.
               this->ForwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, nObj, internalRadius, external, distance, oid);
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

            // is it a object that qualified?
            if ( ( distance <= external) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
               // there is more than k elements?
               if (result->GetNumOfEntries() >= nObj){
                  //cut if there is more than k elements
                  result->Cut(nObj);
               }//end if
            }//end if

         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end tmpl_stSlimTree<ObjectType, EvaluatorType>::ForwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::ForwardRangeQueryWithoutPriority(
         u_int32_t pageID, tResultPaged * result, ObjectType * sample,
         u_int32_t nObj, double internalRadius, double & externalRadius,
         double distanceRepres, long oid){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
            if ( ( (distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                    indexNode->GetIndexEntry(idx).Radius ) <= externalRadius ) &&
                  // test internal qualifies.
                  ( (distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                    indexNode->GetIndexEntry(idx).Radius ) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // test external qualifies.
               if ( (( distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                     // test internal qualifies.
                     ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
                  // Yes! Analyze it!
                  this->ForwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, nObj, internalRadius, externalRadius, distance, oid);
               }//end if
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
             if ( ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius) &&
                  // test internal qualifies.
                  ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // Is this a qualified object?
               if ( ( distance <= externalRadius) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= nObj){
                     //cut if there is more than k elements
                     result->Cut(nObj);
                     //Minimizing Range
                     externalRadius = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for

      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end tmpl_stSlimTree<ObjectType, EvaluatorType>::ForwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stSlimTree::ForwardRangeQuery(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){

   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   tResultPaged * result;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;

   result = new tResultPaged();  // Create result

   // Set the information.
   result->SetQueryInfo(sample->Clone(), NEXTRANGEQUERY, nObj, externalRadius, false);

   // Let's search
   if (this->GetRoot() != 0){

      // Root node
      pqCurrValue.PageID = this->GetRoot();
      pqCurrValue.Radius = 0;

      // Create the Global Priority Queue
      queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

      // Let's search
      while (pqCurrValue.PageID != 0){
         // Read node...
         currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
         currNode = stSlimNode::CreateNode(currPage);
         // Is it a Index node?
         if (currNode->GetNodeType() == stSlimNode::INDEX) {
            // Get Index node
            stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
               if (  ((distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                       indexNode->GetIndexEntry(idx).Radius) <= externalRadius ) &&
                     // test internal qualifies.
                     ((distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                       indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ){
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

                  // test external qualifies.
                  if (  ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                        // test internal qualifies.
                        ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {

                     // Yes! I'm qualified! Put it in the queue.
                     pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                     pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                     queue->Add(distance, pqTmpValue);
                  }//end if
               }//end if
            }//end for
         }else{
            // No, it is a leaf node. Get it.
            stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
                if ( ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius ) &&
                     // test internal qualifies.
                     ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius ) ){
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  // When this entry is a representative, it does not need to evaluate
                  // a distance, because distanceRepres is iqual to distance.
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
                  // Is this a qualified object?
                  if ( ( distance <= externalRadius) && (
                       ( distance > internalRadius) ||
                       ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                     // Yes! Put it in the result set.
                     result->AddPair(tmpObj.Clone(), distance);
                     // there is more than k elements?
                     if (result->GetNumOfEntries() >= nObj){
                        //cut if there is more than k elements
                        result->Cut(nObj);
                        //Minimizing Range
                        externalRadius = result->GetMaximumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end for

         }//end else

         // Free it all
         delete currNode;
		 currNode = 0;
         this->myPageManager->ReleasePage(currPage);

         // Go to next node
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){

               // test external qualifies.
               if ( (distance <= (externalRadius + pqCurrValue.Radius)) &&
                     // test internal qualifies.
                     (( distance + pqCurrValue.Radius) >= internalRadius) ) {
                  // Yes, get the pageID and the distance from the representative
                  // and the query object.
                  distanceRepres = distance;
                  // Break the while.
                  stop = true;
               }//end if
            }else{
               // the queue is empty!
               pqCurrValue.PageID = 0;
               // Break the while.
               stop = true;
            }//end if
         }while (!stop);
      }// end while
      // Release the Global Priority Queue
      delete queue;
	  queue = 0;

   }// end if

   return result;
}//end tmpl_stSlimTree<ObjectType, EvaluatorType>::ForwardRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stSlimTree::BackwardRangeQueryWithoutPriority(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){
   tResultPaged * result = new tResultPaged();  // Create result
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;
   double internal;

   internal = internalRadius;

   // Set the information.
   result->SetQueryInfo(sample->Clone(), PREVIOUSRANGEQUERY, nObj, externalRadius, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // test external qualifies.
            if ( ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius ) &&
                 // test internal qualifies.
                 ((distance + indexNode->GetIndexEntry(idx).Radius) >= internal) ) {
               // Yes! Analyze this subtree.
               this->BackwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, nObj, internal, externalRadius, distance, oid);
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

            // is it a object that qualified?
            if ( ( distance <= externalRadius) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
               // there is more than nObj elements?
               if (result->GetNumOfEntries() >= nObj){
                  //cut if there is more than nObj elements
                  result->CutFirst(nObj);
               }//end if
            }//end if

         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end tmpl_stSlimTree<ObjectType, EvaluatorType>::BackwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::BackwardRangeQueryWithoutPriority(
         u_int32_t pageID, tResultPaged * result, ObjectType * sample,
         u_int32_t nObj, double & internalRadius, double externalRadius,
         double distanceRepres, long oid){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
            if ( (( distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                    indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                  // test internal qualifies.
                  ((distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                    indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // test external qualifies.
               if (  ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                     // test internal qualifies.
                     ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
                  // Yes! Analyze it!
                  this->BackwardRangeQueryWithoutPriority(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, nObj, internalRadius, externalRadius, distance, oid);
               }//end if
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {

            // use of the triangle inequality to cut a subtree
            // test external qualifies.
             if ( ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius) &&
                  // test internal qualifies.
                  ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius) ){

               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // Is this a qualified object?
               if ( ( distance <= externalRadius) && (
                 ( distance > internalRadius) ||
                 ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than nObj elements?
                  if (result->GetNumOfEntries() >= nObj){
                     //cut if there is more than nObj elements
                     result->CutFirst(nObj);
                     //Minimizing Range
                     internalRadius = result->GetMinimumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for

      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end tmpl_stSlimTree<ObjectType, EvaluatorType>::BackwardRangeQueryWithoutPriority

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResultPaged<ObjectType> * tmpl_stSlimTree::BackwardRangeQuery(
        ObjectType * sample, u_int32_t nObj,
        double internalRadius, double externalRadius, long oid){

   tDynamicReversedPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   tResultPaged * result;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;

   // Set the information.
   result = new tResultPaged();  // Create result
   result->SetQueryInfo(sample->Clone(), PREVIOUSRANGEQUERY, nObj, externalRadius, false);

   // Let's search
   if (this->GetRoot() != 0){

      // Root node
      pqCurrValue.PageID = this->GetRoot();
      pqCurrValue.Radius = 0;

      // Create the Global Priority Queue
      queue = new tDynamicReversedPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

      // Let's search
      while (pqCurrValue.PageID != 0){
         // Read node...
         currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
         currNode = stSlimNode::CreateNode(currPage);
         // Is it a Index node?
         if (currNode->GetNodeType() == stSlimNode::INDEX) {
            // Get Index node
            stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
               if (  ((distanceRepres - indexNode->GetIndexEntry(idx).Distance -
                     indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                     // test internal qualifies.
                     ((distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                     indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ){
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

                  // test external qualifies.
                  if (  ((distance - indexNode->GetIndexEntry(idx).Radius) <= externalRadius) &&
                        // test internal qualifies.
                        ((distance + indexNode->GetIndexEntry(idx).Radius) >= internalRadius) ) {
                     // Yes! I'm qualified! Put it in the queue.
                     pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                     pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                     queue->Add(distance, pqTmpValue);
                  }//end if
               }//end if
            }//end for
         }else{
            // No, it is a leaf node. Get it.
            stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();

            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // use of the triangle inequality to cut a subtree
               // test external qualifies.
               if (  ((distanceRepres - leafNode->GetLeafEntry(idx).Distance) <= externalRadius) &&
                     // test internal qualifies.
                     ((distanceRepres + leafNode->GetLeafEntry(idx).Distance) >= internalRadius) ){
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  // When this entry is a representative, it does not need to evaluate
                  // a distance, because distanceRepres is iqual to distance.
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
                  // Is this a qualified object?
                  if ( ( distance <= externalRadius) && (
                    ( distance > internalRadius) ||
                    ((distance == internalRadius) && (tmpObj.GetOID() > oid)))) {
                     // Yes! Put it in the result set.
                     result->AddPair(tmpObj.Clone(), distance);
                     // there is more than nObj elements?
                     if (result->GetNumOfEntries() >= nObj){
                        //cut if there is more than nObj elements
                        result->CutFirst(nObj);
                        //Minimizing Range
                        internalRadius = result->GetMinimumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end for

         }//end else

         // Free it all
         delete currNode;
		 currNode = 0;
         this->myPageManager->ReleasePage(currPage);

         // Go to next node
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){

               // test external qualifies.
               if ( (distance <= (externalRadius + pqCurrValue.Radius)) &&
                     // test internal qualifies.
                     (( distance + pqCurrValue.Radius) >= internalRadius) ) {

                  // Yes, get the pageID and the distance from the representative
                  // and the query object.
                  distanceRepres = distance;
                  // Break the while.
                  stop = true;
               }//end if
            }else{
               // the queue is empty!
               pqCurrValue.PageID = 0;
               // Break the while.
               stop = true;
            }//end if
         }while (!stop);
      }// end while

      // Release the Global Priority Queue
      delete queue;
	  queue = 0;
   }//end if
   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::BackwardRangeQuery


template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::GetEmptyResult(
            ){
               
   tResult * result = new tResult(); 

   return result;

}
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::RangeQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;
   #ifdef __stMAMVIEW__
      stMessageString title;
      stMessageString comment;
   #endif //__stMAMVIEW__

   // Set the information.
   result->SetQueryInfo((ObjectType*) sample->Clone(), RANGEQUERY, -1, range, false);

   // Visualization support
   #ifdef __stMAMVIEW__
      MAMViewer->SetQueryInfo(0, range);
      MAMViewer->SetLevel(0);
      title.Append("Slim-Tree: Range Query with page size ");
      title.Append((int) tMetricTree::myPageManager->GetMinimumPageSize());
      comment.Append("The radius of this range query is ");
      comment.Append((double)range);
      MAMViewer->BeginAnimation(title.GetStr(), comment.GetStr());
   #endif //__stMAMVIEW__

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);



      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // Visualization support
         #ifdef __stMAMVIEW__
            MAMViewer->LevelUp();
            comment.Clear();
            comment.Append("Root is the index node ");
            comment.Append((int) this->GetRoot());
            MAMViewer->BeginFrame(comment.GetStr());
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add all child nodes all active
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               MAMViewer->SetNode(indexNode->GetIndexEntry(idx).PageID, &tmpObj,
                     indexNode->GetIndexEntry(idx).Radius, this->GetRoot(), 0, true);
            }//end for
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // test if this subtree qualifies.
            if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
               // Yes! Analyze this subtree.
               this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, range, distance);
            }//end if
         }//end for
         
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         #ifdef __stMAMVIEW__
            comment.Append("Root is the leaf node ");
            comment.Append((int) this->GetRoot());
            MAMViewer->BeginFrame(comment.GetStr());
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add objects to the node
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               MAMViewer->SetObject(&tmpObj, this->GetRoot(), true);
            }//end for
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
         
         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.IncludedUnserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));

            if(this->myMetricEvaluator->GetFilter(tmpObj, *sample) == true){

            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // is it a object that qualified?
            if (distance <= range){
               // Yes! Put it in the result set.

                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);
               }
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   // Visualization support
   #ifdef __stMAMVIEW__
      // Add the last frame with the final result
      comment.Clear();
      comment.Append("The final result has ");
      comment.Append((int)result->GetNumOfEntries());
      comment.Append(" object(s) and radius ");
      comment.Append((double)result->GetMaximumDistance());
      MAMViewer->BeginFrame(comment.GetStr());
      MAMViewer->SetResult(sample, result);
      MAMViewer->EndFrame();
      MAMViewer->EndAnimation();
   #endif //__stMAMVIEW__
   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::RangeQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double range, double distanceRepres){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   #ifdef __stMAMVIEW__
      stMessageString comment;
   #endif //__stMAMVIEW__

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // Visualization support
         #ifdef __stMAMVIEW__
            MAMViewer->LevelUp();
            comment.Clear();
            comment.Append("Entering in the index node ");
            comment.Append((int) pageID);
            comment.Append(" at level ");
            comment.Append((int)  MAMViewer->GetLevel());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->EnableNode(pageID);
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add all child nodes all active
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               MAMViewer->SetNode(indexNode->GetIndexEntry(idx).PageID, &tmpObj,
                     indexNode->GetIndexEntry(idx).Radius, pageID, 0, true);
            }//end for
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality to cut a subtree
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      range + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // is this a qualified subtree?
               if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! Analyze it!
                  this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, range, distance);
                  #ifdef __stMAMVIEW__
                     comment.Clear();
                     comment.Append("Returning to the index node ");
                     comment.Append((int) pageID);
                     comment.Append(" at level ");
                     comment.Append((int)  MAMViewer->GetLevel());
                     MAMViewer->BeginFrame(comment.GetStr());
                     MAMViewer->EnableNode(pageID);
                     MAMViewer->EndFrame();
                  #endif //__stMAMVIEW__
               }//end if
            }//end if
         }//end for

         // Visualization support
         #ifdef __stMAMVIEW__
            MAMViewer->LevelDown();
         #endif //__stMAMVIEW__
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("Entering in the leaf node ");
            comment.Append((int) pageID);
            comment.Append(" at level ");
            comment.Append((int)  MAMViewer->GetLevel());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->EnableNode(pageID);
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add objects to the node
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               MAMViewer->SetObject(&tmpObj, pageID, true);
            }//end for
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
         
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      range){
               // Rebuild the object
               tmpObj.IncludedUnserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));

               if(this->myMetricEvaluator->GetFilter(tmpObj, *sample) == true){

               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // Is this a qualified object?
               if (distance <= range){
                  // Yes! Put it in the result set.


                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);

                }
               }//end if
            }//end if
         }//end for

         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("The result after the leaf node ");
            comment.Append((int) pageID);
            comment.Append(" at level ");
            comment.Append((int)  MAMViewer->GetLevel());
            comment.Append(" has ");
            comment.Append((int)result->GetNumOfEntries());
            comment.Append(" object(s) and radius ");
            comment.Append((double)result->GetMaximumDistance());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::RangeQuery



//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::ExistsQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;
   #ifdef __stMAMVIEW__
      stMessageString title;
      stMessageString comment;
   #endif //__stMAMVIEW__

   // Set the information.
   result->SetQueryInfo((ObjectType*) sample->Clone(), RANGEQUERY, -1, range, false);

   // Visualization support
   #ifdef __stMAMVIEW__
      MAMViewer->SetQueryInfo(0, range);
      MAMViewer->SetLevel(0);
      title.Append("Slim-Tree: Range Query with page size ");
      title.Append((int) tMetricTree::myPageManager->GetMinimumPageSize());
      comment.Append("The radius of this range query is ");
      comment.Append((double)range);
      MAMViewer->BeginAnimation(title.GetStr(), comment.GetStr());
   #endif //__stMAMVIEW__

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // Visualization support
         #ifdef __stMAMVIEW__
            MAMViewer->LevelUp();
            comment.Clear();
            comment.Append("Root is the index node ");
            comment.Append((int) this->GetRoot());
            MAMViewer->BeginFrame(comment.GetStr());
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add all child nodes all active
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               MAMViewer->SetNode(indexNode->GetIndexEntry(idx).PageID, &tmpObj,
                     indexNode->GetIndexEntry(idx).Radius, this->GetRoot(), 0, true);
            }//end for
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // test if this subtree qualifies.
            if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
               // Yes! Analyze this subtree.
               if(result->GetNumOfEntries() == 0){
                  this->ExistsQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                 sample, range, distance);
               }else{
                  return result;
               }
            }//end if
         }//end for
         
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         #ifdef __stMAMVIEW__
            comment.Append("Root is the leaf node ");
            comment.Append((int) this->GetRoot());
            MAMViewer->BeginFrame(comment.GetStr());
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add objects to the node
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               MAMViewer->SetObject(&tmpObj, this->GetRoot(), true);
            }//end for
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
         
         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.IncludedUnserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));

            if(this->myMetricEvaluator->GetFilter(tmpObj, *sample) == true){
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // is it a object that qualified?
            if (distance <= range){
               // Yes! Put it in the result set.
            
               result->AddPair((ObjectType*) tmpObj.Clone(), distance);
               return result;
              }
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   // Visualization support
   #ifdef __stMAMVIEW__
      // Add the last frame with the final result
      comment.Clear();
      comment.Append("The final result has ");
      comment.Append((int)result->GetNumOfEntries());
      comment.Append(" object(s) and radius ");
      comment.Append((double)result->GetMaximumDistance());
      MAMViewer->BeginFrame(comment.GetStr());
      MAMViewer->SetResult(sample, result);
      MAMViewer->EndFrame();
      MAMViewer->EndAnimation();
   #endif //__stMAMVIEW__
   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::ExistsQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double range, double distanceRepres){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   #ifdef __stMAMVIEW__
      stMessageString comment;
   #endif //__stMAMVIEW__

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // Visualization support
         #ifdef __stMAMVIEW__
            MAMViewer->LevelUp();
            comment.Clear();
            comment.Append("Entering in the index node ");
            comment.Append((int) pageID);
            comment.Append(" at level ");
            comment.Append((int)  MAMViewer->GetLevel());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->EnableNode(pageID);
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add all child nodes all active
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               MAMViewer->SetNode(indexNode->GetIndexEntry(idx).PageID, &tmpObj,
                     indexNode->GetIndexEntry(idx).Radius, pageID, 0, true);
            }//end for
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality to cut a subtree
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      range + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // is this a qualified subtree?
               if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! Analyze it!


                     if(result->GetNumOfEntries() == 0){
                     
                        this->ExistsQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, range, distance);
                     }
                  
                  #ifdef __stMAMVIEW__
                     comment.Clear();
                     comment.Append("Returning to the index node ");
                     comment.Append((int) pageID);
                     comment.Append(" at level ");
                     comment.Append((int)  MAMViewer->GetLevel());
                     MAMViewer->BeginFrame(comment.GetStr());
                     MAMViewer->EnableNode(pageID);
                     MAMViewer->EndFrame();
                  #endif //__stMAMVIEW__
               }//end if
            }//end if
         }//end for

         // Visualization support
         #ifdef __stMAMVIEW__
            MAMViewer->LevelDown();
         #endif //__stMAMVIEW__
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("Entering in the leaf node ");
            comment.Append((int) pageID);
            comment.Append(" at level ");
            comment.Append((int)  MAMViewer->GetLevel());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->EnableNode(pageID);
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add objects to the node
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               MAMViewer->SetObject(&tmpObj, pageID, true);
            }//end for
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
         
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      range){
               // Rebuild the object
               tmpObj.IncludedUnserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               
               if(this->myMetricEvaluator->GetFilter(tmpObj, *sample) == true){

               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // Is this a qualified object?
               if (distance <= range){
                  // Yes! Put it in the result set.

                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);

                  break;

                 }
               }//end if
            }//end if
         }//end for

         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("The result after the leaf node ");
            comment.Append((int) pageID);
            comment.Append(" at level ");
            comment.Append((int)  MAMViewer->GetLevel());
            comment.Append(" has ");
            comment.Append((int)result->GetNumOfEntries());
            comment.Append(" object(s) and radius ");
            comment.Append((double)result->GetMaximumDistance());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::ReversedRangeQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;

   result->SetQueryInfo(sample->Clone(), REVERSEDRANGEQUERY, -1, range, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);

      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // test if this subtree qualifies.
            if (distance + indexNode->GetIndexEntry(idx).Radius >= range){
               // Yes! Analyze this subtree.
               this->ReversedRangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                        sample, range, distance);
            }//end if
         }//end for

      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // is it a object that qualified?
            if (distance >= range){
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::ReversedRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::ReversedRangeQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double range, double distanceRepres){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if (distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                indexNode->GetIndexEntry(idx).Radius >= range){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // is this a qualified subtree?
               if (distance + indexNode->GetIndexEntry(idx).Radius >= range){
                  // Yes! Analyze it!
                  this->ReversedRangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                           sample, range, distance);
               }//end if
            }//end if
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if (distanceRepres + leafNode->GetLeafEntry(idx).Distance >= range){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // Is this a qualified object?
               if (distance >= range){
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::ReversedRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::LocalNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   double rangeK = MAXDOUBLE;
   stPage * rootPage;
   stSlimNode * rootNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   tPriorityQueue * queue;
   u_int32_t pid;

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, rangeK, tie);

   // Let's search
   if (this->GetRoot() != 0){
      // Read node...
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);

      // Is it a Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // Priority queue
         queue = new tPriorityQueue(numberOfEntries);

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // Is this a qualified subtree?
            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
               // Yes! Put it in the queue.
               queue->Add(distance, idx);
               this->sumOperationsQueue++;  // Update the statistics for the queue
            }//end if
         }//end for

         if (queue->GetSize() > this->maxQueue)
            this->maxQueue = queue->GetSize();
            
         // Search...
         while (queue->Get(distance, pid)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Will qualify ?
            if (distance <= rangeK + indexNode->GetIndexEntry(pid).Radius){
               // Yes! Analyze it recursively.
               this->LocalNearestQuery(indexNode->GetIndexEntry(pid).PageID, result,
                                       sample, rangeK, k, distance);
            }//end if
         }//end while

         // Release queue.
         delete queue;
		 queue = 0;

      }else{  
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)rootNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate the distance.
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            //test if the object qualify
            if (distance <= rangeK){
               // Add the object.
               result->AddPair(tmpObj.Clone(), distance);
               // there is more than k elements?
               if (result->GetNumOfEntries() >= k){
                  //cut if there is more than k elements
                  result->Cut(k);
                  //may I use this for performance?
                  rangeK = result->GetMaximumDistance();
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::LocalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::LocalNearestQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double & rangeK, u_int32_t k, double distanceRepres){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   tPriorityQueue * queue;
   u_int32_t pid;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // Priority queue
         queue = new tPriorityQueue(numberOfEntries);
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      rangeK + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // Is it a qualified subtree?
               if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! Put it in the queue.
                  queue->Add(distance, idx);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for

         if (queue->GetSize() > this->maxQueue)
            this->maxQueue = queue->GetSize();
         // Search...
         while (queue->Get(distance, pid)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Will qualify ?
            if (distance <= rangeK + indexNode->GetIndexEntry(pid).Radius){
               // Yes! Analyze it.
               this->LocalNearestQuery(indexNode->GetIndexEntry(pid).PageID, result,
                                  sample, rangeK, k, distance);
            }//end if
         }//end while

         // Release queue.
         delete queue;
		 queue = 0;
         
      }else{  
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      rangeK){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // is it a Representative?
               if (leafNode->GetLeafEntry(idx).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if
               // is this a qualified object?
               if (distance <= rangeK){
                  // Yes, add it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->Cut(k);
                     //may I use this for performance?
                     rangeK = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

}//end stSlimTree<ObjectType, EvaluatorType>::LocalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::ListNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      this->ListNearestQuery(result, sample, MAXDOUBLE, k);
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::ListNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::ListNearestQuery(tResult * result,
         ObjectType * sample, double rangeK, u_int32_t k){
   tGenericPriorityQueue * globalQueue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   tGenericEntry * entryNode = NULL;
   u_int32_t numberOfEntries;
   bool stop;

   if (this->GetRoot() != 0){
      // allocate the priority list.
      globalQueue = new tGenericPriorityQueue();
      // Get the root node.
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // Put the Node in the Queue.
            globalQueue->Add(tmpObj.Clone(), indexNode->GetIndexEntry(idx).PageID, distance,
                             indexNode->GetIndexEntry(idx).Radius, tGenericEntry::NODE);
            this->sumOperationsQueue++;  // Update the statistics for the queue
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate the distance.
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            //test if the object qualify
            if (distance <= rangeK){
               // Add the object.
               result->AddPair(tmpObj.Clone(), distance);
               // there is more than k elements?
               if (result->GetNumOfEntries() >= k){
                  //cut if there is more than k elements
                  result->Cut(k);
                  //may I use this for performance?
                  rangeK = result->GetMaximumDistance();
               }//end if
            }//end if
         }//end for
      }//end if

      //Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      do{
         entryNode = globalQueue->Get();
         this->sumOperationsQueue++;  // Update the statistics for the queue
         // Read node...
         currPage = tMetricTree::myPageManager->GetPage(entryNode->GetPageID());
         currNode = stSlimNode::CreateNode(currPage);
         // Is it a Index node?
         if (currNode->GetNodeType() == stSlimNode::INDEX) {
            // Get Index node
            stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();

            // for each entry...
            // Put the Children in the global priority queue.
            for (idx = 0; idx < numberOfEntries; idx++) {
               // try to cut this subtree with the triangle inequality.
               if ( fabs(entryNode->GetDistanceRepQuery() - indexNode->GetIndexEntry(idx).Distance) <=
                         rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  // calculate its distance.
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
                  // add in the priority queue.
                  globalQueue->Add(tmpObj.Clone(), indexNode->GetIndexEntry(idx).PageID,
                                   distance, indexNode->GetIndexEntry(idx).Radius,
                                   tGenericEntry::NODE);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end for
         }else{
            // No, it is a leaf node. Get it.
            stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // try to cut this object with the triangle inequality.
               if ( fabs(entryNode->GetDistanceRepQuery() - leafNode->GetLeafEntry(idx).Distance) <=
                         rangeK){
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  // When this entry is a representative, it does not need to evaluate
                  // a distance, because distanceRepres is iqual to distance.
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
                  //test if the object qualify
                  if (distance <= rangeK){
                     // Add the object.
                     result->AddPair(tmpObj.Clone(), distance);
                     // there is more than k elements?
                     if (result->GetNumOfEntries() >= k){
                        //cut if there is more than k elements
                        result->Cut(k);
                        //may I use this for performance?
                        rangeK = result->GetMaximumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end for
         }//end if
         //Free it all
         delete currNode;
		 currNode = 0;
         tMetricTree::myPageManager->ReleasePage(currPage);

         // Release this entry.
         delete entryNode;
		 entryNode = 0;

      }while (!stop && !globalQueue->IsEmpty());
      // delete the priority list.
      delete globalQueue;
	  globalQueue = 0;
   }//end if

}//end stSlimTree<ObjectType, EvaluatorType>::ListNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::NearestQuery(
      ObjectType * sample, u_int32_t k, bool tie, bool tiebreaker){
   tResult * result = new tResult();  // Create result
   #ifdef __stMAMVIEW__
      stMessageString title;
      stMessageString comment;
   #endif //__stMAMVIEW__

   // Set information for this query
   result->SetQueryInfo((ObjectType*) sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   #ifdef __stMAMVIEW__
      MAMViewer->SetQueryInfo(k, 0);
      MAMViewer->SetLevel(0);
      title.Append("Slim-Tree: k-Nearest Neighbor Query with page size ");
      title.Append((int) tMetricTree::myPageManager->GetMinimumPageSize());
      comment.Append("The k of this nearest neighbor query is ");
      comment.Append((int)k);
      MAMViewer->BeginAnimation(title.GetStr(), comment.GetStr());
   #endif //__stMAMVIEW__

   // Let's search
   if (this->GetRoot() != 0){
      this->NearestQuery(result, sample, MAXDOUBLE, k, tiebreaker);
   }//end if

   // Visualization support
   #ifdef __stMAMVIEW__
      // Add the last frame with the final result
      comment.Clear();
      comment.Append("The final result has ");
      comment.Append((int)result->GetNumOfEntries());
      comment.Append(" object(s) and radius ");
      comment.Append((double)result->GetMaximumDistance());
      MAMViewer->BeginFrame(comment.GetStr());
      MAMViewer->SetResult(sample, result);
      MAMViewer->EndFrame();
      MAMViewer->EndAnimation();
   #endif //__stMAMVIEW__

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::NearestQuery


#include <list>
#include <iterator>

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::NearestQuery(tResult * result,
         ObjectType * sample, double rangeK, u_int32_t k, bool tiebreaker){
   
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;
   #ifdef __stMAMVIEW__
      stMessageString comment;
   #endif //__stMAMVIEW__   

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   #ifdef __stMAMVIEW__
      pqCurrValue.Parent = -1;
      pqCurrValue.Level = 0;
   #endif //__stMAMVIEW__
   
   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // Visualization support
         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("Entering in the index node ");
            comment.Append((int) pqCurrValue.PageID);
            comment.Append(" at level ");
            comment.Append((int) pqCurrValue.Level);            
            MAMViewer->SetLevel( pqCurrValue.Level + 1);
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->EnableNode(pqCurrValue.PageID);
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add all child nodes all active
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               MAMViewer->SetNode(indexNode->GetIndexEntry(idx).PageID, &tmpObj,
                                  indexNode->GetIndexEntry(idx).Radius,
                                  pqCurrValue.PageID, 0, true);
            }//end for
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
         
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      rangeK + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  #ifdef __stMAMVIEW__
                     pqTmpValue.Parent = pqCurrValue.Parent;
                     pqTmpValue.Level = pqCurrValue.Level + 1;
                  #endif //__stMAMVIEW__                     
                  queue->Add(distance, pqTmpValue);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("Entering in the leaf node ");
            comment.Append((int) pqCurrValue.PageID);
            comment.Append(" at level ");
            comment.Append((int) pqCurrValue.Level);
            
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->EnableNode(pqCurrValue.PageID);
            // for each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Add objects to the node
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               MAMViewer->SetObject(&tmpObj, pqCurrValue.PageID, true);
            }//end for
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      rangeK){
               // Rebuild the object
               tmpObj.IncludedUnserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));

            if(this->myMetricEvaluator->GetFilter(tmpObj, *sample) == true){

               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               //test if the object qualify
               if (distance <= rangeK){
                
                  if(tiebreaker){

                     if(result->GetNumOfEntries() < (k-1)){
                        //I don't have k-2 elements yet, I can just add

                        result->AddPair((ObjectType*) tmpObj.Clone(), distance);

                     }else if(result->GetNumOfEntries() == (k-1)){

                        //I'm going to have k elements. Add the element and calculate the radius

                        result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                        rangeK = result->GetMaximumDistance();
                        //std::cout << "Maximum distance: " <<  rangeK << "\n";


                     }else if(result->GetNumOfEntries() == k && distance < rangeK){

                        //I already have k elements. The new element is not a tie. I add it, cut the last element and recalculate the distance.

                        result->AddPair((ObjectType*) tmpObj.Clone(), distance);

                        int index = k;

                        std::list<ObjectType> sortList;

                        while(result->GetPair(index)->GetDistance() == rangeK){
                        
                           ObjectType* objAux = (ObjectType*) result->GetPair(index)->GetObject()->Clone();  

                           sortList.push_front(*objAux);

                           result->RemoveLast();

                           index--;

                           if(index == -1){
                              break;
                           }

                        }
                        
                        sortList.sort();

                        for(typename std::list<ObjectType>::iterator iter= sortList.begin(); iter != sortList.end(); iter++){

                           result->AddPair((ObjectType*) iter->Clone(), rangeK);

                        }

                        result->Cut(k);
                        rangeK = result->GetMaximumDistance();


                     }else if(result->GetNumOfEntries() == k && distance == rangeK){
                        //I already have k elements. The new element is a tie.

                        result->AddPair((ObjectType*) tmpObj.Clone(), distance);

                        int index = k;

                        std::list<ObjectType> sortList;

                        while(result->GetPair(index)->GetDistance() == distance){
                        
                           ObjectType* objAux = (ObjectType*) result->GetPair(index)->GetObject()->Clone();  

                           sortList.push_front(*objAux);

                           result->RemoveLast();

                           index--;

                           if(index == -1){
                              break;
                           }

                        }

                        sortList.sort();

                        for(typename std::list<ObjectType>::iterator iter= sortList.begin(); iter != sortList.end(); iter++){

                           result->AddPair((ObjectType*) iter->Clone(), distance);

                        }
                           
                        result->Cut(k);

                     }
                  }else {
                 
                         // Add the object.
                     result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                     // there is more than k elements?
                     if (result->GetNumOfEntries() >= k){
                        //cut if there is more than k elements
                        result->Cut(k);
                        //may I use this for performance?
                        rangeK = result->GetMaximumDistance();
                     }//end if

                  }
            

                    }
               }//end if
            }//end if
         }//end for

         #ifdef __stMAMVIEW__
            comment.Clear();
            comment.Append("The result after the leaf node ");
            comment.Append((int) pqCurrValue.PageID);
            comment.Append(" at level ");
            comment.Append((int) pqCurrValue.Level);
            comment.Append(" has ");
            comment.Append((int)result->GetNumOfEntries());
            comment.Append(" object(s) and radius ");
            comment.Append((double)result->GetMaximumDistance());
            MAMViewer->BeginFrame(comment.GetStr());
            MAMViewer->SetResult(sample, result);
            MAMViewer->EndFrame();
         #endif //__stMAMVIEW__
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
         this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Qualified if distance <= rangeK + radius
            if (distance <= rangeK + pqCurrValue.Radius){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
   queue = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::NearestQuery

//------------------------------------------------------------------------------
//
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::FarthestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KFARTHESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      this->FarthestQuery(result, sample, 0.0, k);
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::FarthestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::FarthestQuery(tResult * result,
         ObjectType * sample, double rangeK, u_int32_t k){
   tDynamicReversedPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTmpValue;
   bool stop;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   
   // Create the Global Priority Queue
   queue = new tDynamicReversedPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if (distanceRepres + indexNode->GetIndexEntry(idx).Distance +
                indexNode->GetIndexEntry(idx).Radius >= rangeK){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if (distance + indexNode->GetIndexEntry(idx).Radius >= rangeK){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTmpValue);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if (distanceRepres + leafNode->GetLeafEntry(idx).Distance >= rangeK){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               //test if the object qualify
               if (distance >= rangeK){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->CutFirst(k);
                     //may I use this for performance?
                     rangeK = result->GetMinimumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for

      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      // Go to next node
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Qualified if distance <= rangeK + radius
            if (distance + pqCurrValue.Radius >= rangeK){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
   queue = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::FarthestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::PointQuery(
      ObjectType * sample){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo((ObjectType*) sample->Clone(), POINTQUERY);
   // Let's search
   if (this->GetRoot() != 0){
      this->PointQuery(result, sample);
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::PointQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::PointQuery(
         tResult * result, ObjectType * sample){
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTMPValue;
   u_int32_t numberOfEntries;
   bool stop;
   bool find = false;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   
   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while ((pqCurrValue.PageID != 0) && (!find)){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?        
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if (distance <= indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTMPValue.PageID =  indexNode->GetIndexEntry(idx).PageID;
                  pqTMPValue.Radius =  indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTMPValue);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality
            if ( distanceRepres == leafNode->GetLeafEntry(idx).Distance){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance.
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               //test if the object qualify
               if (distance == 0){
                  // Add the object.
                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                  // Stop the query because the object was found!
                  find = true;
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      // Go to next node.
      if (!find){
         // Search... and feed query
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){
               this->sumOperationsQueue++;  // Update the statistics for the queue
               // Qualified if distance <= rangeK + radius
               if (distance <= pqCurrValue.Radius){
                  // Yes, get the pageID and the distance from the representative
                  // and the query object.
                  distanceRepres = distance;
                  // Break the while.
                  stop = true;
               }//end if
            }else{
               // the queue is empty!
               pqCurrValue.PageID = 0;
               // Break the while.
               stop = true;
            }//end if
         }while (!stop);
      }//end if
   }//end while

   // Release the Global Priority Queue
   delete queue;
   queue = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::PointQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::EstimateNearestQuery(
         ObjectType * sample, double fractalDimension, long totalNroObjects,
         double maxDistance, u_int32_t k, bool tie){

   tResult * result = new tResult();  // Create result
   double estimatedRadius, firstRadius;
   double distanceRepres = 0;
   long returnedNroObjects;
   double logK = 0;
   int idx;

   // Check if it is possible to calculate the log funcion.
   if (k > 2){
      logK = log((k - 1.0) * (double )totalNroObjects / 2.0);
   }else{
      // The following code is equal to logK = log(3 - 1);
      logK = 0.7 * (double )totalNroObjects;
   }//end if

   // Estimate the first radius. Add a secure factor!
   estimatedRadius = SECUREVALUE * maxDistance *
                     exp((logK - log(totalNroObjects * (totalNroObjects - 1.0) / 2.0)) /
                     fractalDimension);

   // The tie list is set for the possibility of KRingQuery call
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      // Call the nearest query with the estimated radius.
      this->NearestQuery(result, sample, estimatedRadius, k);
      
      // Get the number of objects returned by the NearestQuery.
      returnedNroObjects = result->GetNumOfEntries();

      #ifdef __stFRACTALQUERY__
         // Set the idx.
         idx = 0;
         // Set the GoodGuesses.
         if (returnedNroObjects >= k){
            this->GoodGuesses++;
         }//end if
      #endif //__stFRACTALQUERY__

      while (returnedNroObjects < k) {
         // to store the previous result range
         firstRadius = estimatedRadius;
         //check if it is possible to calculate the log function
         if (returnedNroObjects > 2){
            estimatedRadius = SECUREVALUE * firstRadius *
                              exp((logK - log(totalNroObjects * (returnedNroObjects - 1.0) / 2.0)) /
                              fractalDimension);
         }else{
            estimatedRadius = SECUREVALUE * firstRadius *
                              exp((logK - log(0.7 * (double )totalNroObjects) / 2.0) /
                              fractalDimension);
         }//end if

         // Do the ring query to reach k's element.
         this->KRingQuery(result, sample, firstRadius, estimatedRadius, k);

         // Get the number of objects returned by the call.
         returnedNroObjects = result->GetNumOfEntries();

         #ifdef __stFRACTALQUERY__
            if (idx < SIZERINGCALLS){
               RingCalls[idx]++;
               idx++;
            }//end if   
         #endif //__stFRACTALQUERY__
      }//end while
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::EstimateNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::KAndRangeQuery(
      ObjectType * sample, double range, u_int32_t k, bool tie){

   tResult * result = new tResult();  // Create result

   result->SetQueryInfo((ObjectType*) sample->Clone(), KANDRANGEQUERY, k, range, tie);
   // Let's search
   if (this->GetRoot() != 0){
      this->KAndRangeQuery(result, sample, range, k);
   }//end if
   // return the result
   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::KAndRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::KAndRangeQuery(
         tResult * result, ObjectType * sample, double range, u_int32_t k){
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTMPValue;
   u_int32_t numberOfEntries;
   bool stop;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;

   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      range + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // test if this subtree qualifies.
               if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTMPValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTMPValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTMPValue);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for

      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      range){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // is it a Representative?
               if (leafNode->GetLeafEntry(idx).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if
               if (distance <= range){
                  // Yes! I'm qualified !
                  if (result->GetNumOfEntries() < k){
                     // Has less than k.
                     result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                  }else{
                     // May I add ?
                     if (distance <= result->GetMaximumDistance()){
                        // Yes! I'll add it and cut the results if necessary
                        result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                        //cut if there is more than k elements
                        result->Cut(k);
                        //may I use this for performance?
                        range = result->GetMaximumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      // Next node
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Qualified if distance <= rangeK + radius
            if (distance <= range + pqCurrValue.Radius){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
   queue = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::KAndRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::KOrRangeQuery(
            ObjectType * sample, double range, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result

   result->SetQueryInfo((ObjectType*) sample->Clone(), KORRANGEQUERY, k, range, tie);
   // Let's search
   if (this->GetRoot() != 0){
      this->KOrRangeQuery(result, sample, range, k);
   }//end if
   // Return the result.
   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::KOrRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::KOrRangeQuery(
      tResult * result, ObjectType * sample, double range, u_int32_t k){
      
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTMPValue;
   double distanceK = MAXDOUBLE;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   bool stop;

   // Root node
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   
   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      distanceK + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               // test if this subtree qualifies.
               if (distance <= distanceK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTMPValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTMPValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTMPValue);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for

      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      distanceK){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // is it a Representative?
               if (leafNode->GetLeafEntry(idx).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if
               // KorRange part
               if (distance <= distanceK){
                  //Add in the result.
                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                  // distanceK will never be smaller than range
                  if (distanceK > range){//Nearest > Range then to cut the result is possible

                     if (result->GetNumOfEntries() >= k){
                        //cut if there is more than k elements
                        result->Cut(k); // This depends on tie list too
                        if (result->GetMaximumDistance() <= range)  //Range > Nearest
                           distanceK = range; //Query radius is range
                        else //Query radius is the farthest object (last nearest)
                           distanceK = result->GetMaximumDistance();
                     }//end if
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      // Next node...
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Qualified if distance <= rangeK + radius
            if (distance <= distanceK + pqCurrValue.Radius){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
   queue = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::KOrRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::RingQuery(
      ObjectType * sample, double inRange, double outRange){
   tResult * result = new tResult();  // Create result
   double distanceRepres = 0;

   result->SetQueryInfo((ObjectType*) sample->Clone(), RINGQUERY, -1, outRange, inRange);

   // check if inRange is smaller than outRange
   if (inRange < outRange){
      // Let's search
      if (this->GetRoot() != 0){
         this->RingQuery(this->GetRoot(), result, sample, inRange, outRange,
                         distanceRepres);
      }//end if
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::RingQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::RingQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double inRange, double outRange, double distanceRepres){

   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   tPriorityQueue * queue;
   u_int32_t pid;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // Priority queue
         queue = new tPriorityQueue(numberOfEntries);
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      outRange + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if ((distance <= outRange + indexNode->GetIndexEntry(idx).Radius) &&
                   (distance + indexNode->GetIndexEntry(idx).Radius > inRange)){
                  // Yes! I'm qualified !
                  queue->Add(distance, idx);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for

         while (queue->Get(distance, pid)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Will qualify ?
            if ((distance <= outRange + indexNode->GetIndexEntry(pid).Radius) &&
                (distance + indexNode->GetIndexEntry(pid).Radius > inRange)){

               // Yes! I'm qualified !
               this->RingQuery(indexNode->GetIndexEntry(pid).PageID, result,
                     sample, inRange, outRange, distance);
            }//end if
         }//end while

         // Release queue.
         delete queue;
		 queue = 0;

      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      outRange){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // is it a Representative?
               if (leafNode->GetLeafEntry(idx).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if
               //test if the object qualify
               if ((distance <= outRange) && (distance > inRange)){
                  // Add the object.
                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

}//end stSlimTree<ObjectType, EvaluatorType>::RingQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::LocalKRingQuery(
            ObjectType * sample, double inRange, double outRange,
            u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   double distanceRepres = 0;

   //fix this, it is wrong
   result->SetQueryInfo(sample->Clone(), KRINGQUERY, -1,
                        outRange, inRange, tie);

   // check if inRange is smaller than outRange
   if (inRange < outRange){
      // Let's search
      if (this->GetRoot() != 0){
         this->LocalKRingQuery(this->GetRoot(), result, sample, inRange,
                               outRange, k, distanceRepres);
      }//end if
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::LocalKRingQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::LocalKRingQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double inRange, double & outRange, u_int32_t k,
         double distanceRepres){

   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   tPriorityQueue * queue;
   u_int32_t pid;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // Priority queue
         queue = new tPriorityQueue(numberOfEntries);
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality to cut a subtree
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      outRange + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if ((distance <= outRange + indexNode->GetIndexEntry(idx).Radius) &&
                   (distance + indexNode->GetIndexEntry(idx).Radius > inRange)){
                  // Yes! I'm qualified !
                  queue->Add(distance, idx);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for

         while (queue->Get(distance, pid)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Will qualify ?
            if ((distance <= outRange + indexNode->GetIndexEntry(pid).Radius) &&
                (distance + indexNode->GetIndexEntry(pid).Radius > inRange)){

               // Yes! I'm qualified !
               this->LocalKRingQuery(indexNode->GetIndexEntry(pid).PageID, result,
                                     sample, inRange, outRange, k, distance);
            }//end if
         }//end while

         // Release queue.
         delete queue;
		 queue = 0;

      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      outRange){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // is it a Representative?
               if (leafNode->GetLeafEntry(idx).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if
               //test if the object qualify
               if ((distance <= outRange) && (distance > inRange)){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->Cut(k);
                     //may I use this for performance?
                     outRange = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

}//end stSlimTree<ObjectType, EvaluatorType>::LocalKRingQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::LocalEstimateNearestQuery(
         ObjectType * sample, double fractalDimension, long
         nroObjects, double radiusTree, u_int32_t k, bool tie){

   tResult * result = new tResult();  // Create result
   double estimatedRange, firstRange;
   double distanceRepres = 0;
   double logK = 0;
   #ifdef __stFRACTALQUERY__
      int idx = 0;
   #endif //__stFRACTALQUERY__

   // check if it is possible to calculate the log funcion
   if (k > 2){
      logK = log(k * (k - 1) / 2);
   }//end if

   estimatedRange = exp( (logK - nroObjects)/fractalDimension + radiusTree) * 1.1;

   //the tie list is set for the possibility of KRingQuery call
   result->SetQueryInfo(sample->Clone(), ESTIMATEKNEARESTQUERY, k,
                        MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      //do a nearest query with a estimated range
      this->LocalNearestQuery(this->GetRoot(), result, sample, estimatedRange,
                              k, distanceRepres);

      nroObjects = result->GetNumOfEntries();
      #ifdef __stFRACTALQUERY__
      if (nroObjects >= k){
        GoodKicks++;
      }//end if
      #endif //__stFRACTALQUERY__

      while (nroObjects < k) {
         // to store the previous result range
         firstRange = estimatedRange;
         //check if it is possible to calculate the log function
         if (nroObjects > 2){
            estimatedRange = 1.1 * exp(::log(firstRange) +
                             (logK - log(nroObjects*(nroObjects-1)/2))/fractalDimension);
         }else{
            estimatedRange = 1.1 * exp(log(firstRange) + logK/fractalDimension);
         }//end if
         //do the ring query to reach k elements or range radius
         this->LocalKRingQuery(this->GetRoot(), result, sample, firstRange,
                               estimatedRange, k, distanceRepres);
         nroObjects = result->GetNumOfEntries();

         #ifdef __stFRACTALQUERY__
         if (w < SIZERINGCALLS){
            RingCalls[idx]++;
            w++;
         }
         #endif //__stFRACTALQUERY__
      }//end while

   }//end if

   return result;
}//end LocalEstimateNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::KRingQuery(
            ObjectType * sample, double inRange, double outRange,
            u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result

   //fix this, it is wrong
   result->SetQueryInfo((ObjectType*) sample->Clone(), KRINGQUERY, -1, outRange, inRange, tie);

   // check if inRange is smaller than outRange
   if (inRange < outRange){
      // Let's search
      if (this->GetRoot() != 0){
         this->KRingQuery(result, sample, inRange, outRange, k);
      }//end if
   }//end if

   return result;
}//end KRingQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::KRingQuery(
         tResult * result, ObjectType * sample,
         double inRange, double & outRange, u_int32_t k){

   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   stQueryPriorityQueueValue pqCurrValue;
   stQueryPriorityQueueValue pqTMPValue;
   double distance;
   double distanceRepres = 0;
   u_int32_t numberOfEntries;
   bool stop;

   // Root node.
   pqCurrValue.PageID = this->GetRoot();
   pqCurrValue.Radius = 0;
   
   // Create the Global Priority Queue
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      outRange + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if ((distance <= outRange + indexNode->GetIndexEntry(idx).Radius) &&
                   (distance + indexNode->GetIndexEntry(idx).Radius > inRange)){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTMPValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTMPValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTMPValue);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end if
            }//end if
         }//end for

      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      outRange){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // is it a Representative?
               if (leafNode->GetLeafEntry(idx).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if
               //test if the object qualify
               if ((distance <= outRange) && (distance > inRange)){
                  // Add the object.
                  result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->Cut(k);
                     //may I use this for performance?
                     outRange = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
         this->maxQueue = queue->GetSize();

      // Next node...
      stop = false;
      do{
         if (queue->Get(distance, pqCurrValue)){
            this->sumOperationsQueue++;  // Update the statistics for the queue
            // Qualified if distance <= outRange + radius && distance + radius > inRange
            if ((distance <= outRange + pqCurrValue.Radius) &&
                  (distance + pqCurrValue.Radius > inRange)){
               // Yes, get the pageID and the distance from the representative
               // and the query object.
               distanceRepres = distance;
               // Break the while.
               stop = true;
            }//end if
         }else{
            // the queue is empty!
            pqCurrValue.PageID = 0;
            // Break the while.
            stop = true;
         }//end if
      }while (!stop);
   }// end while

   // Release the Global Priority Queue
   delete queue;
   queue = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::KRingQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::IncrementalListNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   tGenericPriorityQueue * globalQueue;

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      globalQueue = new tGenericPriorityQueue();
      // Call the initialize to the incremental nearest.
      this->InitializeIncrementalNearestQuery(sample, k, result, globalQueue);
      // Call the query itself.
      this->IncrementalNearestQuery(sample, k, result, globalQueue);
      delete globalQueue;
	  globalQueue = 0;
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::InitializeIncrementalNearestQuery(
      ObjectType * sample, u_int32_t k, tResult * result, tGenericPriorityQueue * globalQueue){

   stPage * rootPage;
   stSlimNode * rootNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE);

   if (this->GetRoot() != 0){
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);
      // Is it a Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // Put the Node in the Queue.
            globalQueue->Add(tmpObj.Clone(), indexNode->GetIndexEntry(idx).PageID, distance,
                             indexNode->GetIndexEntry(idx).Radius, NODE);
            this->sumOperationsQueue++;  // Update the statistics for the queue
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)rootNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // Put the Objects in the Queue.
            globalQueue->Add(tmpObj.Clone(), distance, OBJECT);
            this->sumOperationsQueue++;  // Update the statistics for the queue
         }//end for
      }//end if

      if (globalQueue->GetSize() > this->maxQueue)
         this->maxQueue = globalQueue->GetSize();
      //Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
   }//end if
   
}//end stSlimTree<ObjectType, EvaluatorType>::InitializeIncrementalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery(
         ObjectType * sample, u_int32_t k, tResult * result, tGenericPriorityQueue * globalQueue){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   tGenericEntry * entryNode = NULL;
   bool stop = false;

   do{
      entryNode = globalQueue->Get();
      this->sumOperationsQueue++;  // Update the statistics for the queue
      
      switch (entryNode->GetType()){
         case NODE:
            // Read node...
            currPage = tMetricTree::myPageManager->GetPage(entryNode->GetPageID());
            currNode = stSlimNode::CreateNode(currPage);
            // Is it a Index node?
            if (currNode->GetNodeType() == stSlimNode::INDEX) {
               // Get Index node
               stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
               numberOfEntries = indexNode->GetNumberOfEntries();

               // for each entry...
               // Put the Children in the global priority queue.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  globalQueue->Add(tmpObj.Clone(), indexNode->GetIndexEntry(idx).PageID,
                                   entryNode->GetDistanceRepQuery(), indexNode->GetIndexEntry(idx).Distance,
                                   indexNode->GetIndexEntry(idx).Radius, APPROXIMATENODE);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end for
            }else{
               // No, it is a leaf node. Get it.
               stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
               numberOfEntries = leafNode->GetNumberOfEntries();
               // for each entry...
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  globalQueue->Add(tmpObj.Clone(), leafNode->GetLeafEntry(idx).Distance,
                                   entryNode->GetDistanceRepQuery(), APPROXIMATEOBJECT);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end for
            }//end if
            //Free it all
            delete currNode;
			currNode = 0;
            tMetricTree::myPageManager->ReleasePage(currPage);
            break;
         case APPROXIMATENODE :
            distance = this->myMetricEvaluator->GetDistance(entryNode->GetObject(), sample);
            globalQueue->Add(entryNode->GetObject(), entryNode->GetPageID(), distance,
                             entryNode->GetRadius(), NODE);
            this->sumOperationsQueue++;  // Update the statistics for the queue
            //this entry does not has the object! 
            entryNode->SetMine(false);
            break;
         case APPROXIMATEOBJECT :
            distance = this->myMetricEvaluator->GetDistance(entryNode->GetObject(), sample);
            globalQueue->Add(entryNode->GetObject(), distance, OBJECT);
            this->sumOperationsQueue++;  // Update the statistics for the queue
            //this entry does not has the object!
            entryNode->SetMine(false);
            break;
         case OBJECT :
            // Add the object.
            result->AddPair(entryNode->GetObject(), entryNode->GetDistanceQuery());
            //this entry does not has the object!
            entryNode->SetMine(false);
            // is it reach k elements?
            if (result->GetNumOfEntries() == k){
               stop = true;
            }//end if
            break;
      }//end switch

      if (globalQueue->GetSize() > this->maxQueue)
         this->maxQueue = globalQueue->GetSize();
      // Release this entry.
      delete entryNode;
	  entryNode = 0;

   }while (!stop && !globalQueue->IsEmpty());

}//end stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   tPGenericHeap * genericHeap = NULL;

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      genericHeap = new tPGenericHeap(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);
      // Call the initialize to the incremental nearest.
      this->InitializeIncrementalNearestQuery(sample, k, result, genericHeap);
      // Call the query itself.
      this->IncrementalNearestQuery(sample, k, result, genericHeap);
      delete genericHeap;
	  genericHeap = 0;
   }//end if

   return result;
}//end stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::InitializeIncrementalNearestQuery(
      ObjectType * sample, u_int32_t k, tResult * result, tPGenericHeap * globalQueue){

   stPage * rootPage;
   stSlimNode * rootNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE);

   if (this->GetRoot() != 0){
      rootPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      rootNode = stSlimNode::CreateNode(rootPage);
      // Is it a Index node?
      if (rootNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)rootNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // Put the Node in the Queue.
            globalQueue->Add(tmpObj.Clone(), indexNode->GetIndexEntry(idx).PageID,
                             distance, 0, 0,
                             indexNode->GetIndexEntry(idx).Radius, 0, NODE);
            this->sumOperationsQueue++;  // Update the statistics for the queue
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)rootNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
            // Put the Objects in the Queue.
            globalQueue->Add(tmpObj.Clone(), -1,
                             distance, 0, 0,
                             0, 0, OBJECT);
            this->sumOperationsQueue++;  // Update the statistics for the queue
         }//end for
      }//end if
      if (globalQueue->GetSize() > this->maxQueue)
         this->maxQueue = globalQueue->GetSize();
      //Free it all
      delete rootNode;
	  rootNode = 0;
      tMetricTree::myPageManager->ReleasePage(rootPage);
   }//end if
   
}//end stSlimTree<ObjectType, EvaluatorType>::InitializeIncrementalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery(
         ObjectType * sample, u_int32_t k, tResult * result, tPGenericHeap * globalQueue){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   ObjectType * object;
   u_int32_t pageID;
   double distance;
   double distanceRep;
   double distanceQuery;
   double distanceRepQuery;
   double radius;
   u_int32_t height;
   enum tType type;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   bool stop = false;

   while (!stop && globalQueue->Get(object, pageID,
                                    distanceQuery, distanceRep, distanceRepQuery,
                                    radius, height, type)){
      this->sumOperationsQueue++;  // Update the statistics for the queue
      
      switch (type){
         case NODE:
            // Read node...
            currPage = tMetricTree::myPageManager->GetPage(pageID);
            currNode = stSlimNode::CreateNode(currPage);
            // Is it a Index node?
            if (currNode->GetNodeType() == stSlimNode::INDEX) {
               // Get Index node
               stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
               numberOfEntries = indexNode->GetNumberOfEntries();

               // for each entry...
               // Put the Children in the global priority queue.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Rebuild the object
                  tmpObj.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));

                  globalQueue->Add(tmpObj.Clone(), indexNode->GetIndexEntry(idx).PageID,
                                   0, indexNode->GetIndexEntry(idx).Distance, distanceQuery,
                                   indexNode->GetIndexEntry(idx).Radius, height+1,
                                   APPROXIMATENODE);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end for
            }else{
               // No, it is a leaf node. Get it.
               stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
               numberOfEntries = leafNode->GetNumberOfEntries();
               // for each entry...
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));

                  globalQueue->Add(tmpObj.Clone(), -1,
                                   0, leafNode->GetLeafEntry(idx).Distance, distanceQuery,
                                   0, height + 1, APPROXIMATEOBJECT);
                  this->sumOperationsQueue++;  // Update the statistics for the queue
               }//end for
            }//end if
            //Free it all
            delete currNode;
			currNode = 0;
            tMetricTree::myPageManager->ReleasePage(currPage);
            break;//end NODE
         case APPROXIMATENODE :
            distance = this->myMetricEvaluator->GetDistance(object, sample);
            globalQueue->Add(object, pageID,
                             distance, distanceRep, distanceRepQuery,
                             radius, height, NODE);
            this->sumOperationsQueue++;  // Update the statistics for the queue
            break;//end APPROXIMATENODE
         case APPROXIMATEOBJECT :
            distance = this->myMetricEvaluator->GetDistance(object, sample);
            globalQueue->Add(object, -1,
                             distance, 0, 0,
                             0, height, OBJECT);
            this->sumOperationsQueue++;  // Update the statistics for the queue
            break;//end APPROXIMATEOBJECT
         case OBJECT :
            // Add the object.
            result->AddPair(object, distanceQuery);
            // is it reach k elements?
            if (result->GetNumOfEntries() == k){
               stop = true;
            }//end if OBJECT
            break;//end
      }//end switch
      if (globalQueue->GetSize() > this->maxQueue)
         this->maxQueue = globalQueue->GetSize();
   }//end do
   
}//end stSlimTree<ObjectType, EvaluatorType>::IncrementalNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSlimTree<ObjectType, EvaluatorType>::LazyRangeQuery(
      ObjectType * sample, double range, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   double distanceRepres = 0;
   bool stop = false;   // for what?

   result->SetQueryInfo(sample->Clone(), LAZYRANGEQUERY, k, range, tie);

   // Let's search
   if (this->GetRoot() != 0){
      this->LazyRangeQuery(this->GetRoot(), result, sample, range, k,
                           distanceRepres, stop);
   }//end if

   return result;
}//end LazyRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSlimTree<ObjectType, EvaluatorType>::LazyRangeQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double & range, u_int32_t k, double distanceRepres,
         bool & stop){

   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         for (idx = 0; (idx < numberOfEntries) && !stop; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      range + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);

               if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified !
                  this->LazyRangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                       sample, range, k, distance, stop);
               }//end if
            }//end if
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         for (idx = 0; (idx < numberOfEntries) && !stop; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      range){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));

               if (leafNode->GetLeafEntry(idx).Distance != 0) {// is it a Representative?
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  distance = distanceRepres;
               }//end if

               if (distance <= range){
                  // Yes! I'm qualified !
                  result->AddPair(tmpObj.Clone(), distance);
                  // if has k elements, stop!
                  if (result->GetNumOfEntries() == k)
                     // cut the rest!
                     stop = true;
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

}//end LazyRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSlimTree::AproximateNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   double rangeK = MAXDOUBLE;

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, rangeK, tie);

   // Let's search
   if (this->GetRoot() != 0){
      this->AproximateNearestQuery(this->GetRoot(), result, sample, rangeK, k);
   }//end if

   return result;
}//end AproximateNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::AproximateNearestQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double & rangeK, u_int32_t k){
   stPage * currPage;
   stSlimNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;
   u_int32_t pid;
   double distanceRepres = 0;
   u_int32_t TreeHeight = this->GetHeight();

   //path
   int tpath[4];
   tpath[0] = 2;
   tpath[1] = 4;
   tpath[2] = 6;

   // Let's search
   if (pageID != 0){
      stSlimIndexNode * indexNode;

      //Get the PageID of a Leaf Node
      for (idx = 0; (idx < TreeHeight-1) && (tpath[idx] < numberOfEntries); idx++){
         currPage = tMetricTree::myPageManager->GetPage(pageID);
         currNode = stSlimNode::CreateNode(currPage);
         indexNode = (stSlimIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         //Exist the node?
         if (tpath[idx] < numberOfEntries){
            if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance)<=
                      rangeK + indexNode->GetIndexEntry(tpath[idx]).Radius){
               tmpObj.Unserialize(indexNode->GetObject(tpath[idx]),
                                  indexNode->GetObjectSize(tpath[idx]));
               distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               if (distance <= rangeK + indexNode->GetIndexEntry(tpath[idx]).Radius){
                   //Yes! I'm qualified
                   distanceRepres = distance;
                   pageID = indexNode->GetIndexEntry(tpath[idx]).PageID;
               }//end if
            }//end if
         }//end if
      }//end for

      //leaf node
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);

      // Get Leaf node
      stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
      numberOfEntries = leafNode->GetNumberOfEntries();

      if (currNode->GetNodeType() == stSlimNode::LEAF){
         //search in the leaf node
         for (int i = 0; i < numberOfEntries; i++) {
            // use of the triangle inequality
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(i).Distance) <=
                      rangeK){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(i),
                                  leafNode->GetObjectSize(i));
               // is it a Representative?
               if (leafNode->GetLeafEntry(i).Distance != 0) {
                  // No, it is not a representative. Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
               }else{
                  //distance = this->myMetricEvaluator->GetDistance(tmpObj, *sample);
                  distance = distanceRepres;
               }//end if
               //test if the object qualify
               if (distance <= rangeK){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // there is more than k elements?
                  if (result->GetNumOfEntries() >= k){
                     //cut if there is more than k elements
                     result->Cut(k);
                     //may I use this for performance?
                     rangeK = result->GetMaximumDistance();
                  }//end if
               }//end if
            }//end if
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end AproximateNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stJoinedResult<ObjectType> * tmpl_stSlimTree::NearestJoinQuery(
      stSlimTree * slimTree, u_int32_t k, bool tie){
   // Create result
   tJoinedResult * result = new tJoinedResult();
   // Set the result.
   result->SetQueryInfo(KNEARESTJOINQUERY, k, MAXDOUBLE, tie);

   // Soon...

   // Return the result.
   return result;
}//end NearestJoinQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stJoinedResult<ObjectType> * tmpl_stSlimTree::RangeJoinQuery(
      stSlimTree * slimTree, double range, bool buffer){
   // Create result
   tJoinedResult * result = new tJoinedResult();
   result->SetQueryInfo(RANGEJOINQUERY, -1, range, false);
   stPage * currPage;
   stSlimNode * currIndexNode;

   // Is there entries in the first tree?
   if (this->GetRoot() != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(GetRoot());
      currIndexNode = stSlimNode::CreateNode(currPage);
      //verifing navegation
      RangeJoinRecursive(this->GetHeight(), currIndexNode, slimTree, range,
                         buffer, result);
      // Free it all
      delete currIndexNode;
	  currIndexNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
   return result;
}//end RangeJoinQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree:: RangeJoinRecursive(u_int32_t heightIndex,
      stSlimNode * currNode, stSlimTree * slimTree,
      double range, bool buffer, tJoinedResult * result){

   stPage * subPageIndex;
   stPage * joinedPage;
   stPage ** subPageJoin;
   stSlimNode * subNodeIndex;
   stSlimNode * joinedNode;
   stSlimNode ** subNodeJoin;
   stSlimIndexNode * indexNode;
   stSlimIndexNode * joinedIndexNode;
   stSlimIndexNode * indexNodeIndex;
   u_int32_t numberOfEntries, joinedNumberOfEntries, leafNumberOfEntries;
   stSlimLeafNode * leafNode;
   ObjectType * tmpObj;
   ObjectType ** bufferJoinedObj;
   double distance;
   u_int32_t i, j;

   // Get the number of entries in currNode.
   numberOfEntries = currNode->GetNumberOfEntries();

   // When the height of index is more than the height of the joined tree.
   if (heightIndex > slimTree->GetHeight()){
      //node of index is index
      indexNode = (stSlimIndexNode *) currNode;
      for (i = 0; i < numberOfEntries; i++){
         // Read node...
         subPageIndex = tMetricTree::myPageManager->GetPage(indexNode->GetIndexEntry(i).PageID);
         subNodeIndex = stSlimNode::CreateNode(subPageIndex);
         // Navegate thought it.
         RangeJoinRecursive(heightIndex - 1, subNodeIndex, slimTree, range, 
                            buffer, result);
         // Free it all.
         delete subNodeIndex;
		 subNodeIndex = 0;
         tMetricTree::myPageManager->ReleasePage(subPageIndex);
      }//end for
   }else{
      // Read node join..
      joinedPage = slimTree->GetPageManager()->GetPage(slimTree->GetRoot());
      joinedNode = stSlimNode::CreateNode(joinedPage);
      //number of entries
      joinedNumberOfEntries = joinedNode->GetNumberOfEntries();
      //obj index
      tmpObj = new ObjectType;
      //create cache objJoin
      bufferJoinedObj = new ObjectType * [joinedNumberOfEntries];
      
      // For each entry in node join
      for (i = 0; i < joinedNumberOfEntries; i++) {
         // Rebuild the object
         bufferJoinedObj[i] = new ObjectType;
         bufferJoinedObj[i]->Unserialize(joinedNode->GetObject(i),
                                    joinedNode->GetObjectSize(i));
      }//end for

      // Is it an Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         //node of index is index
         indexNodeIndex = (stSlimIndexNode *) currNode;
         // if node of index is index, so node of join is index
         joinedIndexNode = (stSlimIndexNode *)joinedNode;
         // buffer is active
         if (buffer){
            //create cache distance
            subPageJoin = new stPage * [joinedNumberOfEntries];
            subNodeJoin = new stSlimNode * [joinedNumberOfEntries];
            for (j = 0; j < joinedNumberOfEntries; j++){
               //page and node is null
               subPageJoin[j] = NULL;
               subNodeJoin[j] = NULL;
            }//end for
         }else{
            //create cache distance
            subPageJoin = new stPage * [1];
            subNodeJoin = new stSlimNode * [1];
         }//end if
         for (i = 0; i < numberOfEntries; i++){
            // Rebuild the object
            tmpObj->Unserialize(indexNodeIndex->GetObject(i),
                                indexNodeIndex->GetObjectSize(i));
            // read sub node
            subPageIndex = tMetricTree::myPageManager->GetPage(
                           indexNodeIndex->GetIndexEntry(i).PageID);
            subNodeIndex = stSlimNode::CreateNode(subPageIndex);
            // For each entry in node join
            for (j = 0; j < joinedNumberOfEntries; j++) {
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(tmpObj,
                                                         bufferJoinedObj[j]);
               // is this a qualified subtree?
               if (distance <= indexNodeIndex->GetIndexEntry(i).Radius +
                   joinedIndexNode->GetIndexEntry(j).Radius + range){
                  // buffer is active
                  if (buffer){
                     // if not read
                     if (subPageJoin[j] == NULL){
                        //read node
                        subPageJoin[j] = slimTree->GetPageManager()->GetPage(
                           joinedIndexNode->GetIndexEntry(j).PageID);
                        subNodeJoin[j] = stSlimNode::CreateNode(subPageJoin[j]);
                     }//end if
                     // Yes! Analyze it!
                     RangeJoinQueryRecursive(subNodeIndex,
                        indexNodeIndex->GetIndexEntry(i).Radius,
                        subNodeJoin[j], joinedIndexNode->GetIndexEntry(j).Radius,
                        slimTree->GetPageManager(), distance, range,
                        result, buffer);
                  }else{
                     //read node
                     subPageJoin[0] = slimTree->GetPageManager()->GetPage(
                        joinedIndexNode->GetIndexEntry(j).PageID);
                     subNodeJoin[0] = stSlimNode::CreateNode(subPageJoin[0]);
                     // Yes! Analyze it!
                     RangeJoinQueryRecursive(subNodeIndex,
                        indexNodeIndex->GetIndexEntry(i).Radius,
                        subNodeJoin[0], joinedIndexNode->GetIndexEntry(j).Radius,
                        slimTree->GetPageManager(), distance, range,
                        result, buffer);
                     //free it all
                     delete subNodeJoin[0];
					 subNodeJoin[0] = 0;
                     slimTree->GetPageManager()->ReleasePage(subPageJoin[0]);
                  }//end if
               }//end if
            }//end for
            // Free it all
            delete subNodeIndex;
			subNodeIndex = 0;
            tMetricTree::myPageManager->ReleasePage(subPageIndex);
         }//end for
         // Free it all
         if (buffer){
            for (j = 0; j < joinedNumberOfEntries; j++) {
               if (subNodeJoin[j] != NULL){
                  delete subNodeJoin[j];
				  subNodeJoin[j] = 0;
                  slimTree->GetPageManager()->ReleasePage(subPageJoin[j]);
               }//end if
            }//end for
         }//end if
         delete[] subNodeJoin;
		 subNodeJoin = 0;
         delete[] subPageJoin;
		 subPageJoin = 0;
      }else{
         //node of index is leaf
         leafNode = (stSlimLeafNode *) currNode;
         // Is it an Index node?
         if (joinedNode->GetNodeType() == stSlimNode::INDEX) {
            //node of join is index
            joinedIndexNode = (stSlimIndexNode *)joinedNode;
            // buffer is active
            if (buffer){
               //create cache distance
               subPageJoin = new stPage * [joinedNumberOfEntries];
               subNodeJoin = new stSlimNode * [joinedNumberOfEntries];
               for (j = 0; j < joinedNumberOfEntries; j++){
                  //page and node is null
                  subPageJoin[j] = NULL;
                  subNodeJoin[j] = NULL;
               }//end for
            }else{
               //create cache distance
               subPageJoin = new stPage * [1];
               subNodeJoin = new stSlimNode * [1];
            }//end if
            for (i = 0; i < numberOfEntries; i++){
               // Rebuild the object
               tmpObj->Unserialize(leafNode->GetObject(i),
                                   leafNode->GetObjectSize(i));
               for (j = 0; j < joinedNumberOfEntries; j++){
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj,
                                                            bufferJoinedObj[j]);
                  // is this a qualified subtree?
                  if (distance <= joinedIndexNode->GetIndexEntry(j).Radius + range){
                     // buffer is active
                     if (buffer){
                        // if not read
                        if (subPageJoin[j] == NULL){
                           //read node
                           subPageJoin[j] = slimTree->GetPageManager()->GetPage(
                              joinedIndexNode->GetIndexEntry(j).PageID);
                           subNodeJoin[j] = stSlimNode::CreateNode(subPageJoin[j]);
                        }//end if
                        // Yes! Analyze it!
                        JoinedTreeRangeJoinRecursive(slimTree->GetPageManager(),
                                    subNodeJoin[j], tmpObj,
                                    distance, range, result);
                     }else{
                        //read node
                        subPageJoin[0] = slimTree->GetPageManager()->GetPage(
                                         joinedIndexNode->GetIndexEntry(j).PageID);
                        subNodeJoin[0] = stSlimNode::CreateNode(subPageJoin[0]);
                        // Yes! Analyze it!
                        JoinedTreeRangeJoinRecursive(slimTree->GetPageManager(),
                                                     subNodeJoin[0], tmpObj,
                                                     distance, range, result);
                        // Free it all
                        delete subNodeJoin[0];
						subNodeJoin[0] = 0;
                        slimTree->GetPageManager()->ReleasePage(subPageJoin[0]);
                     }//end if
                  }//end if
               }//end for
            }//end for
            // Free it all
            if (buffer){
               for (j = 0; j < joinedNumberOfEntries; j++) {
                  if (subNodeJoin[j] != NULL){
                     delete subNodeJoin[j];
					 subNodeJoin[j] = 0;
                     slimTree->GetPageManager()->ReleasePage(subPageJoin[j]);
                  }//end if
               }//end for
            }//end if
            delete[] subNodeJoin;
			subNodeJoin = 0;
            delete[] subPageJoin;
			subPageJoin = 0;
         }else{
            // joined node is leaf
            // Lets check all objects in this node
            for (i = 0; i < numberOfEntries; i++){
               // Rebuild the object
               tmpObj->Unserialize(leafNode->GetObject(i),
                                   leafNode->GetObjectSize(i));
               // For each entry in node join
               for (j = 0; j < joinedNumberOfEntries; j++) {
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj,
                                                            bufferJoinedObj[j]);
                  // is this a qualified subtree?
                  if (distance <= range){
                     // Yes! Put it in the result set.
                     result->AddJoinedTriple(tmpObj->Clone(),
                                             bufferJoinedObj[j]->Clone(),
                                             distance);
                  }//end if
               }//end for
            }//end for
         }//end if
      }//end if

      // Free it all
      for (j = 0; j < joinedNumberOfEntries; j++) {
         delete bufferJoinedObj[j];
		 bufferJoinedObj[j] = 0;
      }//end for
      delete[] bufferJoinedObj;
	  bufferJoinedObj = 0;
      delete joinedNode;
	  joinedNode = 0;      
      slimTree->GetPageManager()->ReleasePage(joinedPage);
      delete tmpObj;
	  tmpObj = 0;
   }//end if
}//end RangeJoinRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::RangeJoinQueryRecursive(
      stSlimNode * currIndexNode, double radiusObjIndex,
      stSlimNode * joinedNode, double radiusObjJoin,
      stPageManager * PageManagerJoin, double distRepres,
      const double range, tJoinedResult * result,
      bool buffer){
      
   u_int32_t numberOfEntries, joinedNumberOfEntries;
   ObjectType * tmpObj = new ObjectType();
   ObjectType ** bufferJoinedObj;
   double distance;
   u_int32_t i, j;
   stPage ** subPageJoin;
   stSlimNode ** subNodeJoin;
   stSlimIndexNode * indexNodeIndex;
   stSlimIndexNode * joinedIndexNode;
   stSlimLeafNode * leafNode;
   stSlimLeafNode * leafNodeJoin;

   // Get the number of entries.
   joinedNumberOfEntries = joinedNode->GetNumberOfEntries();
   numberOfEntries = currIndexNode->GetNumberOfEntries();
   // Create cache objJoin
   bufferJoinedObj = new ObjectType * [joinedNumberOfEntries];

   // For each entry in node join
   for (j = 0; j < joinedNumberOfEntries; j++) {
      // Rebuild the object
      bufferJoinedObj[j] = new ObjectType();
      bufferJoinedObj[j]->Unserialize(joinedNode->GetObject(j),
                                 joinedNode->GetObjectSize(j));
   }//end for


   // Is it an Index node?
   if (currIndexNode->GetNodeType() == stSlimNode::INDEX){
      //node of index is index
      indexNodeIndex = (stSlimIndexNode *) currIndexNode;
      // joined node always will be index
      joinedIndexNode = (stSlimIndexNode *)joinedNode;
      // buffer is active
      if (buffer){
         //create cache distance
         subPageJoin = new stPage * [joinedNumberOfEntries];
         subNodeJoin = new stSlimNode * [joinedNumberOfEntries];
         for (j = 0; j < joinedNumberOfEntries; j++){
            //page and node is null
            subPageJoin[j] = NULL;
            subNodeJoin[j] = NULL;
         }//end for
      }else{
         //create cache distance
         subPageJoin = new stPage * [1];
         subNodeJoin = new stSlimNode * [1];
      }//end if
      for (i = 0; i < numberOfEntries; i++){
         // use of the triangle inequality to cut a subtree
         if (distRepres <= indexNodeIndex->GetIndexEntry(i).Distance +
             indexNodeIndex->GetIndexEntry(i).Radius + radiusObjJoin + range){
            // Rebuild the object
            tmpObj->Unserialize(indexNodeIndex->GetObject(i),
                                indexNodeIndex->GetObjectSize(i));
            // read sub node
            stPage * subPageIndex = tMetricTree::myPageManager->GetPage(
                  indexNodeIndex->GetIndexEntry(i).PageID);
            stSlimNode * subNodeIndex = stSlimNode::CreateNode(subPageIndex);
            // For each entry in node join
            for (j = 0; j < joinedNumberOfEntries; j++) {
               // use of the triangle inequality to cut a subtree
               if (distRepres <= joinedIndexNode->GetIndexEntry(j).Distance +
                   joinedIndexNode->GetIndexEntry(j).Radius +
                   radiusObjIndex + range){
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpObj,
                                                            bufferJoinedObj[j]);
                  // is this a qualified subtree?
                  if (distance <= indexNodeIndex->GetIndexEntry(i).Radius +
                      joinedIndexNode->GetIndexEntry(j).Radius + range){
                     // buffer is active
                     if (buffer){
                        // if not read
                        if (subPageJoin[j] == NULL){
                           //read node
                           subPageJoin[j] = PageManagerJoin->GetPage(
                              joinedIndexNode->GetIndexEntry(j).PageID);
                           subNodeJoin[j] = stSlimNode::CreateNode(subPageJoin[j]);
                        }//end if
                        // Yes! Analyze it!
                        RangeJoinQueryRecursive(subNodeIndex,
                           indexNodeIndex->GetIndexEntry(i).Radius,
                           subNodeJoin[j], joinedIndexNode->GetIndexEntry(j).Radius,
                           PageManagerJoin, distance, range, result,
                           buffer);
                     }else{
                        //read node
                        subPageJoin[0] = PageManagerJoin->GetPage(
                           joinedIndexNode->GetIndexEntry(j).PageID);
                        subNodeJoin[0] = stSlimNode::CreateNode(subPageJoin[0]);
                        // Yes! Analyze it!
                        RangeJoinQueryRecursive(subNodeIndex,
                           indexNodeIndex->GetIndexEntry(i).Radius,
                           subNodeJoin[0], joinedIndexNode->GetIndexEntry(j).Radius,
                           PageManagerJoin, distance, range, result,
                           buffer);
                        //free it all
                        delete subNodeJoin[0];
						subNodeJoin[0] = 0;
                        PageManagerJoin->ReleasePage(subPageJoin[0]);
                     }//end if
                  }//end if
               }//end if
            }//end for
            // Free it all
            delete subNodeIndex;
            subNodeIndex = 0;
            tMetricTree::myPageManager->ReleasePage(subPageIndex);
         }//end if
      }//end for
      // Free it all
      if (buffer){
         for (j = 0; j < joinedNumberOfEntries; j++) {
            if (subNodeJoin[j] != NULL){
               delete subNodeJoin[j];
			   subNodeJoin[j] = 0;
               PageManagerJoin->ReleasePage(subPageJoin[j]);
            }//end if
         }//end for
      }//end if
      delete[] subNodeJoin;
	  subNodeJoin = 0;
      delete[] subPageJoin;
	  subPageJoin = 0;
   }else{
      // The node is a leaf node.
      leafNode = (stSlimLeafNode *) currIndexNode;
      // Is it an Index node?
      if (joinedNode->GetNodeType() == stSlimNode::INDEX) {
         //node of join is index
         joinedIndexNode = (stSlimIndexNode *)joinedNode;
         //buffer is active
         if (buffer){
            //create cache distance
            subPageJoin = new stPage * [joinedNumberOfEntries];
            subNodeJoin = new stSlimNode * [joinedNumberOfEntries];
            for (j = 0; j < joinedNumberOfEntries; j++){
               //page and node is null
               subPageJoin[j] = NULL;
               subNodeJoin[j] = NULL;
            }//end for
         }else{
            //create cache distance
            subPageJoin = new stPage * [1];
            subNodeJoin = new stSlimNode * [1];
         }//end if
         for (i = 0; i < numberOfEntries; i++){
            // use of the triangle inequality to cut a subtree
            if (distRepres <= leafNode->GetLeafEntry(i).Distance +
                radiusObjJoin + range){
               // Rebuild the object
               tmpObj->Unserialize(leafNode->GetObject(i),
                                   leafNode->GetObjectSize(i));
               for (j = 0; j < joinedNumberOfEntries; j++){
                  // use of the triangle inequality to cut a subtree
                  if (distRepres <= joinedIndexNode->GetIndexEntry(j).Distance +
                      joinedIndexNode->GetIndexEntry(j).Radius +
                      radiusObjIndex + range){
                     // Evaluate distance
                     distance = this->myMetricEvaluator->GetDistance(tmpObj,
                                                               bufferJoinedObj[j]);
                     // is this a qualified subtree?
                     if (distance <= joinedIndexNode->GetIndexEntry(j).Radius + range){
                        //buffer is active
                        if (buffer){
                           // if not read
                           if (subPageJoin[j] == NULL){
                              //read node
                              subPageJoin[j] = PageManagerJoin->GetPage(
                                 joinedIndexNode->GetIndexEntry(j).PageID);
                              subNodeJoin[j] = stSlimNode::CreateNode(subPageJoin[j]);
                           }//end if
                           // Yes! Analyze it!
                           JoinedTreeRangeJoinRecursive(PageManagerJoin,
                                             subNodeJoin[j], tmpObj,
                                             distance, range, result);
                        }else{
                           //read node
                           subPageJoin[0] = PageManagerJoin->GetPage(
                              joinedIndexNode->GetIndexEntry(j).PageID);
                           subNodeJoin[0] = stSlimNode::CreateNode(subPageJoin[0]);
                           // Yes! Analyze it!
                           JoinedTreeRangeJoinRecursive(PageManagerJoin, subNodeJoin[0],
                                             tmpObj, distance,
                                             range, result);
                           // Free it all
                           delete subNodeJoin[0];
						   subNodeJoin[0] = 0;
                           PageManagerJoin->ReleasePage(subPageJoin[0]);
                        }//end if
                     }//end if
                  }//end if
               }//end for
            }//end if
         }//end for
         // Free it all
         if (buffer){
            for (j = 0; j < joinedNumberOfEntries; j++) {
               if (subNodeJoin[j] != NULL){
                  delete subNodeJoin[j];
				  subNodeJoin[j] = 0;
                  PageManagerJoin->ReleasePage(subPageJoin[j]);
               }//end if
            }//end for
         }//end if
         delete[] subNodeJoin;
		 subNodeJoin = 0;
         delete[] subPageJoin;
		 subPageJoin = 0;
      }else{
         // joined node is leaf
         leafNodeJoin = (stSlimLeafNode *)joinedNode;
         // Lets check all objects in this node
         for (i = 0; i < numberOfEntries; i++){
            // use of the triangle inequality to cut a subtree
            if (distRepres <= leafNode->GetLeafEntry(i).Distance +
                radiusObjJoin + range){
               // Rebuild the object
               tmpObj->Unserialize(leafNode->GetObject(i),
                                   leafNode->GetObjectSize(i));
               // For each entry in node join
               for (j = 0; j < joinedNumberOfEntries; j++) {
                  // use of the triangle inequality to cut a subtree
                  if (distRepres <= leafNodeJoin->GetLeafEntry(j).Distance +
                      radiusObjIndex + range){
                     // Evaluate distance
                     distance = this->myMetricEvaluator->GetDistance(tmpObj,
                                                               bufferJoinedObj[j]);
                     // is this a qualified subtree?
                     if (distance <= range){
                        // Yes! Put it in the result set.
                        result->AddJoinedTriple(tmpObj->Clone(),
                                                bufferJoinedObj[j]->Clone(),
                                                distance);
                     }//end if
                  }//end if
               }//end for
            }//end if
         }//end for
      }//end if
   }//end if
   
   // Free it all
   for (j = 0; j < joinedNumberOfEntries; j++) {
      delete bufferJoinedObj[j];
	  bufferJoinedObj[j] = 0;
   }//end for
   delete[] bufferJoinedObj;
   bufferJoinedObj = 0;
   delete tmpObj;
   tmpObj = 0;
}//end RangeJoinQueryRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::JoinedTreeRangeJoinRecursive(
      stPageManager * PageManagerJoin, stSlimNode * joinedNode,
      ObjectType * objIndex, double distRepres, double range,
      tJoinedResult * result){

   double distance;
   ObjectType * tmpObj = new ObjectType();
   u_int32_t joinedNumberOfEntries;
   stSlimIndexNode * joinedIndexNode;
   stSlimLeafNode * leafNodeJoin;
   u_int32_t i, j;

   // Get the number of entries.
   joinedNumberOfEntries = joinedNode->GetNumberOfEntries();
   
   // Is it an Index node?
   if (joinedNode->GetNodeType() == stSlimNode::INDEX) {
      // Get Index node
      joinedIndexNode = (stSlimIndexNode *)joinedNode;
      // For each entry...
      for (j = 0; j < joinedNumberOfEntries; j++) {
         // use of the triangle inequality to cut a subtree
         if (fabs(distRepres - joinedIndexNode->GetIndexEntry(j).Distance) <=
             range + joinedIndexNode->GetIndexEntry(j).Radius){
            // Rebuild the object
            tmpObj->Unserialize(joinedIndexNode->GetObject(j),
                               joinedIndexNode->GetObjectSize(j));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, objIndex);
            // is this a qualified subtree?
            if (distance <= range + joinedIndexNode->GetIndexEntry(j).Radius){
               //read sub node
               stPage * subPageJoin = PageManagerJoin->GetPage(
                  joinedIndexNode->GetIndexEntry(j).PageID);
               stSlimNode * subNodeJoin = stSlimNode::CreateNode(subPageJoin);
               // Yes! Analyze it!
               JoinedTreeRangeJoinRecursive(PageManagerJoin, subNodeJoin,
                  objIndex, distance, range, result);
               // Free it all
               delete subNodeJoin;
               subNodeJoin = 0;
               PageManagerJoin->ReleasePage(subPageJoin);
            }//end if
         }//end if
      }//end for
   }else{
      // No, it is a leaf node. Get it.
      leafNodeJoin = (stSlimLeafNode *)joinedNode;
      // for each entry...
      for (j = 0; j < joinedNumberOfEntries; j++) {
         // use of the triangle inequality.
         if ( fabs(distRepres - leafNodeJoin->GetLeafEntry(j).Distance) <= range){
            // Rebuild the object
            tmpObj->Unserialize(leafNodeJoin->GetObject(j),
                               leafNodeJoin->GetObjectSize(j));
            // No, it is not a representative. Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(tmpObj, objIndex);
            // Is this a qualified object?
            if (distance <= range){
               // Yes! Put it in the result set.
               result->AddJoinedTriple(objIndex->Clone(),
                                       tmpObj->Clone(),
                                       distance);
            }//end if
         }//end if
      }//end for
   }//end else
   //free it all
   delete tmpObj;
   tmpObj = 0;
}//end JoinedTreeRangeJoinRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stJoinedResult<ObjectType> * tmpl_stSlimTree::DummyRangeJoinQuery(
      stMetricTree<ObjectType, EvaluatorType> * joinedTree,
      double range){
   // Create result
   tJoinedResult * result = new tJoinedResult();
   result->SetQueryInfo(RANGEJOINQUERY, -1, range, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      //call recursive
      DummyRangeJoinQueryRecursive(result, joinedTree, Header->Root, range);
   }//end if

   // Return the result.
   return result;
}//end DummyRangeJoinQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::DummyRangeJoinQueryRecursive(
      stJoinedResult<ObjectType> * result,
      stMetricTree<ObjectType, EvaluatorType> * joinedTree,
      u_int32_t pageID, double range){

   stSlimIndexNode * indexNode;
   stSlimLeafNode * leafNode;
   stPage * curPage;
   stSlimNode * currNode;
   tResult * localResult;
   ObjectType tmp;
   u_int32_t i, j;

   // Let's search
   if (pageID != 0){
      // Read node...
      curPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(curPage);  
      // if node is index
      if (currNode->GetNodeType() == stSlimNode::INDEX){
         // It is a index node.
         indexNode = (stSlimIndexNode *) currNode;
         // For each entry, call it recursively.
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            //call recursive
            DummyRangeJoinQueryRecursive(result, joinedTree,
                                         indexNode->GetIndexEntry(i).PageID,
                                         range);
         }//end for
      }else{
         // It is a leaf node.
         leafNode = (stSlimLeafNode *) currNode;
         // Check all entries in this leaf node.
         for (i = 0; i < leafNode->GetNumberOfEntries(); i++){
            // Rebuild the object
            tmp.Unserialize(leafNode->GetObject(i),
                            leafNode->GetObjectSize(i));
            // Allocate the resorces.
            localResult = new tResult();
            // Call the range query for tmp object.
            localResult = joinedTree->RangeQuery(&tmp, range);
            // For all elements in the result, copy then in result.
            for (j = 0; j < localResult->GetNumOfEntries(); j++) {
               result->AddJoinedTriple(tmp.Clone(),
                                      (* localResult)[j].GetObject()->Clone(),
                                      (* localResult)[j].GetDistance());
            }//end for
            // Cleanning.
            delete localResult;
			localResult = 0;
         }//end for
      }//end if
      
      // Clean the mess.
      tMetricTree::myPageManager->ReleasePage(curPage);
      delete currNode;
      currNode = 0;
   }//end if
   
}//end DummyRangeJoinQueryRecursive

//==============================================================================
// End of Queries
//------------------------------------------------------------------------------

// Visualization support
#ifdef __stMAMVIEW__

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MAMViewInit(){
   tObjectSample * sample;
   int size;

   size = int(float(this->GetNumberOfObjects()) * 0.1);

   if (size < 200){
      size = 200;
   }//end if

   sample = GetSample(size);
   MAMViewer->Init(sample);
   delete sample;
   sample = 0;
}//end stSlimTree<ObjectType, EvaluatorType>::MAMViewInit

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMAMViewObjectSample <ObjectType> * stSlimTree<ObjectType, EvaluatorType>::
GetSample(int sampleSize){
   tObjectSample * sample;

   // Create the sample object.
   sample = new tObjectSample(sampleSize, GetNumberOfObjects());

   // Get them!
   GetSampleRecursive(this->GetRoot(), sample);

   return sample;
}//end stSlimTree<ObjectType, EvaluatorType>::GetSample

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GetSampleRecursive(
      u_int32_t pageID, tmpl_stSlimTree::tObjectSample * sample){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t i;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;

         // for each entry call GetSampleRecursive
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++) {
            GetSampleRecursive(indexNode->GetIndexEntry(i).PageID, sample);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
         ObjectType tmp;

         for (i = 0; i < leafNode->GetNumberOfEntries(); i++) {
            if (sample->MayAdd()){
               // YES! I'll add it now.
               tmp.Unserialize(leafNode->GetObject(i),
                               leafNode->GetObjectSize(i));
               sample->Add(tmp.Clone());
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::GetSampleRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MAMViewDumpTree(){

   MAMViewer->BeginAnimation("Full tree dump.");
   MAMViewer->BeginFrame(NULL);
   MAMViewDumpTreeRecursive(this->GetRoot(), NULL, 0, 0);
   MAMViewer->EndFrame();
   MAMViewer->EndAnimation();
}//end stSlimTree<ObjectType, EvaluatorType>::MAMViewDumpTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::MAMViewDumpTreeRecursive(u_int32_t pageID, ObjectType * rep,
            double radius, u_int32_t parent){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t i;
   ObjectType tmp;

   // Level Up
   MAMViewer->LevelUp();

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;

         // Add this node
         if (rep != NULL){
            MAMViewer->SetNode(pageID, rep, radius, parent, 0, false);
         }//end if

         // Adding all entries of this node.
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++) {
            tmp.Unserialize(indexNode->GetObject(i),
                  indexNode->GetObjectSize(i));
            MAMViewDumpTreeRecursive(indexNode->GetIndexEntry(i).PageID, &tmp,
                  indexNode->GetIndexEntry(i).Radius, pageID);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;

         // Add this node
         if (rep != NULL){
            MAMViewer->SetNode(pageID, rep, radius, parent, 1, false);
         }//end if

         for (i = 0; i < leafNode->GetNumberOfEntries(); i++) {
            tmp.Unserialize(leafNode->GetObject(i),
                  leafNode->GetObjectSize(i));
            MAMViewer->SetObject(&tmp, pageID, false);
         }//end for
      }//end else

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if

   // Level Down
   MAMViewer->LevelDown();
}//end stSlimTree<ObjectType, EvaluatorType>::MAMViewDumpTree

#endif //__stMAMVIEW__

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stTreeInfoResult * tmpl_stSlimTree::GetTreeInfo(){
   stTreeInformation * info;

   // No cache of information. I think a cahe would be a good idea.
   info = new stTreeInformation(GetHeight(), GetNumberOfObjects());

   // Let's get the information!
   GetTreeInfoRecursive(this->GetRoot(), 0, info);

   // Optimal tree
   if (info->GetMeanObjectSize() != 0){
      info->CalculateOptimalTreeInfo(int(tMetricTree::myPageManager->GetMinimumPageSize() /
            info->GetMeanObjectSize()));
   }//end if

   return info;
}//end stSlimTree<ObjectType, EvaluatorType>::GetTreeInfo

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::GetTreeInfoRecursive(u_int32_t pageID, int level,
      stTreeInformation * info){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t i;
   ObjectType tmp;

   // Let's search
   if (pageID != 0){
      // Update node count
      info->UpdateNodeCount(level);

      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;

         // Object count
         info->UpdateObjectCount(level, indexNode->GetNumberOfEntries());

         // Scan all entries
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            GetTreeInfoRecursive(indexNode->GetIndexEntry(i).PageID, level + 1,
                                 info);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;

         // Object count
         info->UpdateObjectCount(level, leafNode->GetNumberOfEntries());

         // Update object count
         for (i = 0; i < leafNode->GetNumberOfEntries(); i++){
            // Update other statistics
            info->UpdateMeanObjectSize(leafNode->GetObjectSize(i));

            // Compute intersections
            tmp.Unserialize(leafNode->GetObject(i),
                            leafNode->GetObjectSize(i));

            // Compute intersections
            ObjectIntersectionsRecursive(this->GetRoot(), &tmp, 0, info);
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::GetTreeInfoRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::ObjectIntersectionsRecursive(u_int32_t pageID,
      ObjectType * obj, int level, stTreeInformation * info){
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t i;
   ObjectType tmp;
   double d;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
         // Get Index node
         stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;

         // Scan all entries
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            tmp.Unserialize(indexNode->GetObject(i),
                            indexNode->GetObjectSize(i));
            d = this->myMetricEvaluator->GetDistance(tmp, *obj);
            if (d <= indexNode->GetIndexEntry(i).Radius){
               // Intersection !!!!
               info->UpdateIntersections(level);
               ObjectIntersectionsRecursive(indexNode->GetIndexEntry(i).PageID,
                                            obj, level + 1, info);
            }//end if
         }//end for
      }//end if

      // Free it all
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::ObjectIntersectionsRecursive

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::Optimize(){

   if (this->GetHeight() >= 3){
      SlimDownRecursive(this->GetRoot(), 0);
      // Notify modifications.
      HeaderUpdate = true;
      // Don't worry. This is a debug block!!!
   #ifdef __stPRINTMSG__
   }else{
      cout << "Unable to perform the Slim-Down. This tree has only " <<
         this->GetHeight() << " level(s).\n";
   #endif //__stPRINTMSG__
   }//end if
}//end tmpl_stSlimTree::Optimize

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::SlimDownRecursive(u_int32_t pageID, int level){
   stPage * currPage;
   stSlimNode * currNode;
   stSlimIndexNode * indexNode;
   double radius;
   u_int32_t i;

   // Let's search
   if (pageID != 0){
      
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);

      #ifdef __stPRINTMSG__
         if (currNode->GetNodeType() != stSlimNode::INDEX){
            // This tree has less than 3 levels. This method will not work.
            throw std::logic_error("Slimdown reached the bottom of the tree.");
         }//end if
      #endif //__stPRINTMSG__

      indexNode = (stSlimIndexNode *)currNode;

      // Where am I ?
      if (level == GetHeight() - 3){
         // Slim-Down next level!
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            #ifdef __stPRINTMSG__
               cout << "Level:" << level << ". Begin of the local slim down of " <<
                     indexNode->GetIndexEntry(i).PageID <<
                     " which current radius is " <<
                     indexNode->GetIndexEntry(i).Radius << ".\n";
            #endif //__stPRINTMSG__
            indexNode->GetIndexEntry(i).Radius = SlimDown(
                  indexNode->GetIndexEntry(i).PageID);

            #ifdef __stPRINTMSG__
               cout << "Level:" << level << ". End of the local slim down of " <<
                     indexNode->GetIndexEntry(i).PageID <<
                     " which current radius is " <<
                     indexNode->GetIndexEntry(i).Radius << ".\n";
            #endif //__stPRINTMSG__

         }//end for
      }else{
         // Move on...
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            indexNode->GetIndexEntry(i).Radius = SlimDownRecursive(
                  indexNode->GetIndexEntry(i).PageID, level + 1);
         }//end for
      }//end if
      
      // Update my radius.
      radius = indexNode->GetMinimumRadius();

      // Write me and get the garbage.
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->WritePage(currPage);
      tMetricTree::myPageManager->ReleasePage(currPage);
      return radius;
   }else{
      // This tree is corrupted or is empty.
      throw std::logic_error("The given tree is corrupted or empty.");
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::SlimDownRecursive

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSlimTree::SlimDown(u_int32_t pageID){
   stPage * currPage;
   stSlimNode * currNode;
   stSlimIndexNode * indexNode;
   tMemLeafNode ** memLeafNodes;
   stSlimNode * tmpNode;
   stPage * tmpPage;
   stSlimLeafNode * leafNode;
   double radius;
   int maxSwaps;
   u_int32_t nodeCount;
   u_int32_t idx;
   u_int32_t i;

   // Let's search
   if (pageID != 0){

      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      currNode = stSlimNode::CreateNode(currPage);

      #ifdef __stPRINTMSG__
         if (currNode->GetNodeType() != stSlimNode::INDEX){
            // This tree has less than 3 levels. This method will not work.
            throw std::logic_error("Slimdown reached the bottom of the tree.");
         }//end if
      #endif //__stPRINTMSG__

      // Cast currNode to stSlimIndexNode as it must be...
      indexNode = (stSlimIndexNode *)currNode;
      nodeCount = indexNode->GetNumberOfEntries();

      #ifdef __stPRINTMSG__
         cout << "Local Slimdown in " <<
               indexNode->GetPageID() <<
               " which current radius is " <<
               indexNode->GetMinimumRadius() << ".\n";
      #endif //__stPRINTMSG__


      // Create  all stSlimMemLeafNodes
      memLeafNodes = new tMemLeafNode * [nodeCount];
      maxSwaps = 0;
      for (i = 0; i < nodeCount; i++){
         // Read leaf
         tmpPage = tMetricTree::myPageManager->GetPage(indexNode->GetIndexEntry(i).PageID);
         tmpNode = stSlimNode::CreateNode(tmpPage);

         #ifdef __stPRINTMSG__
            if (tmpNode->GetNodeType() != stSlimNode::LEAF){
               // This tree has less than 3 levels. This method will not work.
               throw std::logic_error("Oops. This tree is corrupted.");
            }//end if
         #endif //__stPRINTMSG__
         leafNode = (stSlimLeafNode *) tmpNode;

         // Update maxSwaps
         maxSwaps += leafNode->GetNumberOfEntries();

         // Assemble memory version
         memLeafNodes[i] = new tMemLeafNode(leafNode);
      }//end for
      maxSwaps *= 3;

      // Execute the local SlimDown
      LocalSlimDown(memLeafNodes, nodeCount, maxSwaps);

      // Rebuild nodes and write them. Of course, the empty ones will be disposed.
      idx = 0;
      for (i = 0; i < nodeCount; i++){
         // Dispose memory version
         if (memLeafNodes[i]->GetNumberOfEntries() != 0){
            leafNode = memLeafNodes[i]->ReleaseNode();
            delete memLeafNodes[i];
			memLeafNodes[i] = 0;

            // Update entry
            indexNode->GetIndexEntry(idx).NEntries = leafNode->GetNumberOfEntries();
            indexNode->GetIndexEntry(idx).Radius = leafNode->GetMinimumRadius();
            idx++;

            // Write back
            tmpPage = leafNode->GetPage();
            delete leafNode;
			leafNode = 0;
            tMetricTree::myPageManager->WritePage(tmpPage);
            tMetricTree::myPageManager->ReleasePage(tmpPage);
         }else{
            // Empty node
            leafNode = memLeafNodes[i]->ReleaseNode();
            delete memLeafNodes[i];
			memLeafNodes[i] = 0;

            // Remove entry
            indexNode->RemoveEntry(idx);
            #ifdef __stPRINTMSG__
               cout << "Node " << i << " is no more!\n";
            #endif //__stPRINTMSG__

            // Dispose empty node
            tmpPage = leafNode->GetPage();
            delete leafNode;
			leafNode = 0;
            DisposePage(tmpPage);
         }//end if
      }//end for
      delete[] memLeafNodes;
	  memLeafNodes = 0;

      // Update my radius.
      radius = indexNode->GetMinimumRadius();

      // Write me and get the garbage.
      delete currNode;
	  currNode = 0;
      tMetricTree::myPageManager->WritePage(currPage);
      tMetricTree::myPageManager->ReleasePage(currPage);
      return radius;
   }else{
      // This tree is corrupted or is empty.
      throw std::logic_error("The given tree is corrupted or empty.");
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::SlimDown

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSlimTree::LocalSlimDown(
      tMemLeafNode ** memLeafNodes, int nodeCount,
      int maxSwaps){
   bool stop;
   int src;
   int dst;
   int i;
   int localSwapCount;
   int swapCount = 0;
   double minDist;
   double tmpDist;

   // main loop
   stop = false;
   while (!stop){
      // Try to swap them
      localSwapCount = 0;
      for (src = 0; src < nodeCount; src++){
         if (memLeafNodes[src]->GetNumberOfEntries() > 0){
            // Look for the target...
            dst = -1;
            minDist = MAXDOUBLE;
            for (i = 0; i < nodeCount; i++){
               if (i != src){
                  if (SlimDownCanSwap(memLeafNodes[src], memLeafNodes[i],
                        tmpDist)){
                     if (tmpDist < minDist){
                        dst = i;
                        minDist = tmpDist;
                     }//end if
                  }//end if
               }//end if
            }//end for
   
            // Swap!
            if (dst != -1){
               // Update swap count
               localSwapCount++;
   
               // Swap!
               memLeafNodes[dst]->Add(memLeafNodes[src]->PopObject(), minDist);
            }//end if
         //}else{
            // This node is empty.
         }//end if
      }//end for

      // Stop condition
      swapCount += localSwapCount;
      stop = (swapCount > maxSwaps) || (localSwapCount == 0);
   }//end while
}//end stSlimTree<ObjectType, EvaluatorType>::LocalSlimDown

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::SlimDownCanSwap(
      tMemLeafNode * src, tMemLeafNode * dst,
      double & distance){

   // Check to see if destination is empty
   if (dst->GetNumberOfEntries() == 0){
      return false;
   }//end if

   // Calculate the distance between src's last object and dst's representative
   distance = this->myMetricEvaluator->GetDistance(*src->LastObject(), *dst->RepObject());

   // Test distances and occupation
   if (distance <= dst->GetMinimumRadius()){
      if (dst->CanAdd(src->LastObject())){
         return true;
      }else{
         // Will not fit.
         return false;
      }//end if
   }else{
      // dst does not cover
      return false;
   }//end if
}//end stSlimTree<ObjectType, EvaluatorType>::SlimDownIntersects

#ifdef __BULKLOAD__

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadOrdered(ObjectType **objects, u_int32_t numObj){
   return BulkLoadOrdered(objects,numObj, 1, bulkRANDOM); // Default
}
//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadOrdered(ObjectType **objects, u_int32_t numObj, enum tBulkMethod method){
   return BulkLoadOrdered(objects,numObj, 0.8, method); // Default
}
//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadOrdered(ObjectType **objects, u_int32_t numObj, double nodeOccupancy, enum tBulkMethod method){
   return BulkLoadOrdered(objects,numObj,nodeOccupancy,nodeOccupancy, method);
}
//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
int tmpl_stSlimTree::BulkLoadSimple(ObjectType **objects, u_int32_t numObj, double leafNodeOccupancy, stPage *& auxPage, int currObj) {

   auxPage  = this->NewPage();
   stSlimLeafNode * leafNode = new stSlimLeafNode(auxPage, true);

   ObjectType *newObj; // Object
   u_int32_t insertIdx; // Insertion index

   // Current insertion object
   newObj = objects[currObj];
   u_int32_t newObjSize = newObj->GetSerializedSize();

   // Number of objects in the current node
   u_int32_t numObjNode = 0;

   // While has space
   while((leafNode->GetFree()*leafNodeOccupancy>newObjSize+sizeof(stSlimLeafNode::stSlimLeafEntry))&&(currObj<numObj)) {

      #ifdef __stPRINTMSG__
         cout << endl << "Inserting the following object #: " << currObj << endl;
      #endif //__stPRINTMSG__

      // Insert the new object.
      insertIdx = leafNode->AddEntry(newObjSize,
                                       newObj->Serialize());

      #ifdef __stPRINTMSG__
         cout << endl << "Insertion OK! Object #: " << currObj << " " << *newObj << ". " << endl;
      #endif //__stPRINTMSG__

      currObj++; // Next object.
      numObjNode++;

      // Get the right object
      if(currObj<numObj) {
         newObj = objects[currObj];
         newObjSize = newObj->GetSerializedSize();
      } //end if


      #ifdef __stDEBUG__
      // Test if the page size is too big to store an object.
      if (insertIdx < 0){
         // Oops. There is an error during the insertion.
            cout << "The page size is too small for the first object. Increase it!\n";
            // Throw an exception.
            throw std::logic_error("The page size is too small to store the first object.");
         // The new object was not inserted.
         //return false;
      } //end if
      #endif //__stDEBUG__

   } //end while

   delete leafNode;
   leafNode = 0;

   return currObj;

}

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
int tmpl_stSlimTree::BulkLoadSampled(ObjectType **objects, u_int32_t numObj, double leafNodeOccupancy, stPage *& auxPage, int currObj){

   u_int32_t numOfSamples = 5;

   stSlimLeafNode ** sampleNode = new stSlimLeafNode * [numOfSamples]; // @todo: delete this
   stPage ** samplePage  = new stPage * [numOfSamples]; // @todo: delete this

   u_int32_t insertIdx; // Insertion index

   #ifdef __stDEBUG__
      if(numObj<=numOfSamples) {
          cout << endl << "Error: too few objects: " << numObj << endl;
          exit(1);
      }
   #endif //__stDEBUG__


   // get the first ones - @todo: Change this to get random


   u_int32_t newObjSize = 0;
   for(int i=0;i<numOfSamples;i++) {

      newObj = objects[currObj];
      newObjSize = newObj->GetSerializedSize();
      // Insert the new object.
      samplePage[i] = this->NewPage();
      sampleNode[i] = new SlimLeafNode(samplePage[i],true); // @todo: Change this to work
      insertIdx = sampleNode[i]->AddEntry(newObjSize,
                                          newObj->Serialize());

      currObj++; // @todo: Change this to get random
   }


   // Current insertion object
   newObj = objects[currObj];

   // Number of objects in the current node
   u_int32_t numObjNode = 0;


   // While has space
   //while((leafNode->GetFree()*leafNodeOccupancy>newObjSize+sizeof(stSlimLeafNode::stSlimLeafEntry))&&(currObj<numObj)) {

   // While has space in all sample nodes
   while(true) {

      ObjectType rep; // Object

      double minDist = MAXDOUBLE;
      u_int32_t sampleIdx;

      //Choose the corret node to insert - @todo: Change this to get random
      for(int i=0;i<numOfSamples;i++) {
         rep.Unserialize(sampleNode[i]->GetObject(0), // Rep is always the first element in this case
                            sampleNode[i]->GetObjectSize(0));
         double distance = this->myMetricEvaluator->GetDistance(newObj, &rep);
         if(distance < minDist) {
            distance = minDist;
            sampleIdx = i;
         }
      }

      newObjSize = newObj->GetSerializedSize();
      insertIdx = sampleNode[i]->AddEntry(newObjSize,
                                          newObj->Serialize());
      currObj++;

      if(sampleNode[i]->GetFree()*leafNodeOccupancy<=newObjSize+sizeof(stSlimLeafNode::stSlimLeafEntry)) { // no space for other object with the same size
         auxPage = samplePage[i];
         delete sampleNode[i];
		 sampleNode[i] = 0;
         //Choose another - @todo: Change this to get random
         //@TODO!!!!!!!
         return currObj;

      }



         #ifdef __stPRINTMSG__
            cout << endl << "Inserting the following object #: " << currObj << endl;
         #endif //__stPRINTMSG__

         // Insert the new object.
         insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),
                                        newObj->Serialize());

         #ifdef __stPRINTMSG__
            cout << endl << "Insertion OK! Object #: " << currObj << " " << *newObj << ". " << endl;
         #endif //__stPRINTMSG__

         currObj++; // Next object.
         numObjNode++;

         // Get the right object
         if(currObj<numObj) {
            newObj = objects[currObj];
            newObjSize = newObj->GetSerializedSize();
         } //end if


         #ifdef __stDEBUG__
         // Test if the page size is too big to store an object.
         if (insertIdx < 0){
            // Oops. There is an error during the insertion.
               cout << "The page size is too small for the first object. Increase it!\n";
               // Throw an exception.
               throw std::logic_error("The page size is too small to store the first object.");
            // The new object was not inserted.
            //return false;
         } //end if
         #endif //__stDEBUG__

      } //end while

      return currObj;

}

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadOrdered(ObjectType **objects, u_int32_t numObj, double leafNodeOccupancy, double indexNodeOccupancy, enum tBulkMethod method){
   if(method == bulkRANDOM) {
      randomize();
   }

   int currObj = 0;  // Number of current object


   #ifdef __stPRINTMSG__
      cout << endl << "Starting bulk loading..." <<endl;
   #endif //__stPRINTMSG__

   Header->Height++;

   bool finished = false;

   // Ladies and getlemen, start your engines! :D
   while(currObj<numObj) {

      // New leaf node
      stPage * auxPage;


      //SIMPLE
      currObj = BulkLoadSimple(objects, numObj, leafNodeOccupancy, auxPage, currObj);


      stSlimLeafNode * leafNode = (stSlimLeafNode *)stSlimNode::CreateNode(auxPage);



      // Representative
      ObjectType rep, obj; // @todo: Choose the best representative

      double distance = 0;

      u_int32_t repIdx = 0;

      u_int32_t numObjNode = leafNode->GetNumberOfEntries();

      switch(method) {
         case bulkBIASED: // first object
            rep.Unserialize(leafNode->GetObject(repIdx),
                            leafNode->GetObjectSize(repIdx));
            //Update the distance
            for(int idx=0;idx<numObjNode;idx++) {
               if(idx!=repIdx) {
                  obj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
                  distance = this->myMetricEvaluator->GetDistance(&obj, &rep);
               } //end if
               leafNode->GetLeafEntry(idx).Distance = distance;
            }
            break;
         case bulkRANDOM: // random object
            repIdx = numObjNode * random(RAND_MAX) / (RAND_MAX + 1);
            rep.Unserialize(leafNode->GetObject(repIdx),
                            leafNode->GetObjectSize(repIdx));
            //Update the distance
            for(int idx=0;idx<numObjNode;idx++) {
               if(idx!=repIdx) {
                  obj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
                  distance = this->myMetricEvaluator->GetDistance(&obj, &rep);
               } else {
                  distance = 0;
               } //end if
               leafNode->GetLeafEntry(idx).Distance = distance;
            }
            break;
         case bulkMINDIST: // best object
            double ** distMatrix = new double * [numObjNode]; // to reduce distance calculations
            double minDist = MAXDOUBLE;
            double maxDist;
            ObjectType obj1,obj2;

            for(int idx=0;idx<numObjNode;idx++) {
               distMatrix[idx] = new double[numObjNode];
            }

            int i;

            for(i=0;i<numObjNode-1;i++) {
               obj1.Unserialize(leafNode->GetObject(i),
                                leafNode->GetObjectSize(i));
               distMatrix[i][i]=0;
               maxDist=0;
               for(int j=i+1;j<numObjNode;j++) {
                  obj2.Unserialize(leafNode->GetObject(j),
                                   leafNode->GetObjectSize(j));
                  distMatrix[i][j] = this->myMetricEvaluator->GetDistance(&obj1, &obj2);
                  distMatrix[j][i] = distMatrix[i][j];
                  if(distMatrix[j][i] > maxDist) {
                     maxDist = distMatrix[j][i];
                  } // end if
               } // end for
               if(maxDist<minDist) {
                  minDist = maxDist;
                  repIdx = i;
               } // end if
            } // end for
            distMatrix[i][i]=0;

            //Update the distance
            for(int idx=0;idx<numObjNode;idx++) {
               if(idx!=repIdx) {
                  distance = distMatrix[repIdx][idx];
               } else {
                  distance = 0;
               } //end if
               leafNode->GetLeafEntry(idx).Distance = distance;
            }

            rep.Unserialize(leafNode->GetObject(repIdx),
                            leafNode->GetObjectSize(repIdx));

            for(int idx=0;idx<numObjNode;idx++) {
               delete []distMatrix[idx];
			   distMatrix[idx] = 0;
            }

            delete []distMatrix;
			distMatrix = 0;

            break;
      }



      // Current SubTree
      stSubtreeInfo sub;
      sub.Rep = &rep;
      sub.Radius = leafNode->GetMinimumRadius();
      sub.RootID = auxPage->GetPageID();
      sub.NObjects = numObjNode;

      #ifdef __stPRINTMSG__
         cout << endl << "SubTreeInfo - Radius: " << sub.Radius << ". NObjects: " << numObjNode << " . RootID: " << sub.RootID << endl;
      #endif //__stPRINTMSG__

      // Auxiliary SubTree
      stSubtreeInfo sub1;
      sub1.Rep = NULL;
      sub1.Radius = 0;
      sub1.RootID = 0;
      sub1.NObjects = 0;

      // Write the node.
      tMetricTree::myPageManager->WritePage(auxPage);
      delete leafNode;
	  leafNode = 0;
      delete auxPage;
	  auxPage = 0;

      BulkInsert(sub1, sub, indexNodeOccupancy, method);
   } //end while


   // Update object count.
   UpdateObjectCounter(numObj);

   // Report the modification.
   HeaderUpdate = true;

   // Clean the stacks
   int pathSize = this->rightPathEntries.size();

   ObjectType tmpObj, repObj;

   // @todo: Optimize this stuff
   while(this->rightPathEntries.size()>1) {

      stPage * stackPage = this->rightPathEntries.top();
      this->rightPathEntries.pop();
      stPage * fatherPage = this->rightPathEntries.top();

      stSlimIndexNode * currNode = (stSlimIndexNode *)stSlimNode::CreateNode(stackPage);
      stSlimIndexNode * fatherNode = (stSlimIndexNode *)stSlimNode::CreateNode(fatherPage);

      //cout << "\nNode " << currNode->GetPageID() << endl;

      u_int32_t numberOfEntries = currNode->GetNumberOfEntries();
      u_int32_t repIdx = fatherNode->GetNumberOfEntries()-1;//currNode->GetRepresentativeEntry();

      repObj.Unserialize(fatherNode->GetObject(repIdx),
                         fatherNode->GetObjectSize(repIdx));


      //Update the distance
      for(int idx=0;idx<numberOfEntries;idx++) {
         tmpObj.Unserialize(currNode->GetObject(idx),
                            currNode->GetObjectSize(idx));
         currNode->GetIndexEntry(idx).Distance = this->myMetricEvaluator->GetDistance(&tmpObj, &repObj);

      }


      fatherNode->GetIndexEntry(repIdx).Radius = currNode->GetMinimumRadius();
      fatherNode->GetIndexEntry(repIdx).NEntries = currNode->GetTotalObjectCount(); // Update the number of objects

      // Write the current page (node).
      tMetricTree::myPageManager->WritePage(stackPage);
      // Write the current page (node). // @TODO: Optimize this... disk access
      //tMetricTree::myPageManager->WritePage(fatherPage);

      delete currNode;
	  currNode = 0;
      delete fatherNode;
	  fatherNode = 0;
      delete stackPage;
	  stackPage = 0;



   } //end while

   if(!this->rightPathEntries.empty()) {
        stPage * stackPage = this->rightPathEntries.top();
         // Write the current page (node).
        tMetricTree::myPageManager->WritePage(stackPage);
        //cout << "\nNode " << stackPage->GetPageID() << endl;
        delete stackPage;
		stackPage = 0;
        this->rightPathEntries.pop();
   } // end if


   // Ok. Objects inserted. Return success!
   return true;
   
}//end stSlimTree<ObjectType, EvaluatorType>::BulkLoad

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkInsert(stSubtreeInfo & sub1, stSubtreeInfo & sub, double nodeOccupancy, enum tBulkMethod method) {

   ObjectType tmpObj;

   bool hasNewRoot = false;

   // No node in the stack! Create a new root.
   if (this->rightPathEntries.empty()) {

      #ifdef __stPRINTMSG__
         cout << endl << "Creating a new root." << endl;
      #endif //__stPRINTMSG__

      // New node.
      stPage * newPage  = this->NewPage();
      stSlimIndexNode * newRoot = new stSlimIndexNode(newPage, true);

      // New root.
      this->SetRoot(newPage->GetPageID());

      // Put it on the stack
      //this->rightPath.push(newRoot);
      this->rightPathEntries.push(newPage);

      // Update the Height
      Header->Height++;

      hasNewRoot = true;

      #ifdef __stPRINTMSG__
         cout << endl << "Height " << Header->Height << endl;
      #endif //__stPRINTMSG__

      delete newRoot;
	  newRoot = 0;


   } //end if


   stPage * currPage = this->rightPathEntries.top();
   stSlimIndexNode * currNode = (stSlimIndexNode *)stSlimNode::CreateNode(currPage);

   if(sub.Rep==NULL) return false;

   // Get the object size
   u_int32_t newObjSize = sub.Rep->GetSerializedSize();

   // Has this node space left?
   if(currNode->GetFree()*nodeOccupancy<newObjSize+sizeof(stSlimIndexNode::stSlimIndexEntry)) { // No space left.
      #ifdef __stPRINTMSG__
         cout << endl << "No space left!" << endl;
      #endif //__stPRINTMSG__

      stSubtreeInfo firstSub;


      u_int32_t numberOfEntries = currNode->GetNumberOfEntries();

      double distance = 0;

      u_int32_t bestIdx = 0;

      ObjectType *repObj = NULL;

      switch(method) {
         case bulkBIASED: // first object
            tmpObj.Unserialize(currNode->GetObject(bestIdx),
                               currNode->GetObjectSize(bestIdx));

            repObj = tmpObj.Clone();

            //Update the distance
            for(int idx=0;idx<numberOfEntries;idx++) {
               if(idx!=bestIdx) {
                  tmpObj.Unserialize(currNode->GetObject(idx),
                                     currNode->GetObjectSize(idx));
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, repObj);
               } else {
                  distance = 0;
               }
               currNode->GetIndexEntry(idx).Distance = distance;
            }
            break;
         case bulkRANDOM:
            bestIdx = numberOfEntries * random(RAND_MAX) / (RAND_MAX + 1);

            tmpObj.Unserialize(currNode->GetObject(bestIdx),
                               currNode->GetObjectSize(bestIdx));
                               
            repObj = tmpObj.Clone();

            //Update the distance
            for(int idx=0;idx<numberOfEntries;idx++) {
               if(idx!=bestIdx) {
                  tmpObj.Unserialize(currNode->GetObject(idx),
                                     currNode->GetObjectSize(idx));
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, repObj);
               } else {
                  distance = 0;
               }
               currNode->GetIndexEntry(idx).Distance = distance;
            }

            break;

         case bulkMINDIST: // best object
            double ** distMatrix = new double * [numberOfEntries]; // to reduce distance calculations
            double minDist = MAXDOUBLE;
            double maxDist;
            ObjectType obj1,obj2;

            for(int idx=0;idx<numberOfEntries;idx++) {
               distMatrix[idx] = new double[numberOfEntries];
            }

            int i;

            for(i=0;i<numberOfEntries-1;i++) {
               obj1.Unserialize(currNode->GetObject(i),
                                currNode->GetObjectSize(i));
               distMatrix[i][i]=0;
               maxDist=0;
               for(int j=i+1;j<numberOfEntries;j++) {
                  obj2.Unserialize(currNode->GetObject(j),
                                   currNode->GetObjectSize(j));
                  distMatrix[i][j] = this->myMetricEvaluator->GetDistance(&obj1, &obj2);
                  distMatrix[j][i] = distMatrix[i][j];
                  if(distMatrix[j][i] > maxDist) {
                     maxDist = distMatrix[j][i];
                  } // end if
               } // end for
               if(maxDist<minDist) {
                  minDist = maxDist;
                  bestIdx = i;
               } // end if
            } // end for
            distMatrix[i][i]=0;


            //Update the distance
            for(int idx=0;idx<numberOfEntries;idx++) {
               if(idx!=bestIdx) {
                  distance = distMatrix[bestIdx][idx];
               } else {
                  distance = 0;
               } //end if
               currNode->GetIndexEntry(idx).Distance = distance;
            }

            tmpObj.Unserialize(currNode->GetObject(bestIdx),
                               currNode->GetObjectSize(bestIdx));

            repObj = tmpObj.Clone();

            for(int idx=0;idx<numberOfEntries;idx++) {
               delete []distMatrix[idx];
			   distMatrix[idx] = 0;
            }

            delete []distMatrix;
			distMatrix = 0;

            break;


      }


      firstSub.Rep = repObj; // @todo: choose the best representative!!!


      firstSub.Radius = currNode->GetMinimumRadius();
      firstSub.RootID = currPage->GetPageID(); // Child node
      firstSub.NObjects = currNode->GetTotalObjectCount(); // Update the number of objects

      // Pop from the stack.
      this->rightPathEntries.pop();

      //Update the father
      if(!this->rightPathEntries.empty()) {
         stPage * fatherPage = this->rightPathEntries.top();
         stSlimIndexNode * fatherNode = (stSlimIndexNode *)stSlimNode::CreateNode(fatherPage);
         u_int32_t repIdx = fatherNode->GetNumberOfEntries() - 1; // @TODO Get Father
         fatherNode->RemoveEntry(repIdx);
         fatherNode->AddEntry(firstSub.Rep->GetSerializedSize(),
                              firstSub.Rep->Serialize());
         fatherNode->GetIndexEntry(repIdx).Radius = firstSub.Radius;
         fatherNode->GetIndexEntry(repIdx).NEntries = firstSub.NObjects;
         delete fatherNode;
		 fatherNode = 0;
      } // end if


       // Write the current page (node).
      tMetricTree::myPageManager->WritePage(currPage);



      #ifdef __stPRINTMSG__
         cout << endl << "SubTreeInfo - Radius: " << firstSub.Radius << ". NObjects: " << firstSub.NObjects << " . RootID: " << firstSub.RootID << endl;
      #endif //__stPRINTMSG__


      // Clean the mess.
      delete currPage;
	  currPage = 0;

      // New node
      stPage * newNodePage  = this->NewPage();
      stSlimIndexNode * newNode = new stSlimIndexNode(newNodePage, true);


      // Add the representative
      int insertIdx = newNode->AddEntry(sub.Rep->GetSerializedSize(),
                         sub.Rep->Serialize());

      newNode->GetIndexEntry(insertIdx).Radius = sub.Radius;
      newNode->GetIndexEntry(insertIdx).NEntries = sub.NObjects;
      newNode->GetIndexEntry(insertIdx).PageID = sub.RootID;
      newNode->GetIndexEntry(insertIdx).Distance = 0;


      // Clone the representative
      stSubtreeInfo representativeSub;
      representativeSub.Rep = sub.Rep;  // @todo: clone???
      representativeSub.Radius = 0;
      representativeSub.RootID = newNodePage->GetPageID(); // Child node
      representativeSub.NObjects = 0;


      #ifdef __stPRINTMSG__
         cout << endl << "SubTreeInfo - Radius: " << representativeSub.Radius << ". NObjects: " << representativeSub.NObjects <<  " . RootID: " << representativeSub.RootID << endl;
      #endif //__stPRINTMSG__

      // Do it recursively until has space
      BulkInsert(firstSub,representativeSub,nodeOccupancy, method);

      // Put the new node on the stack
      this->rightPathEntries.push(newNodePage);



      delete newNode;
	  newNode = 0;
      delete repObj;
	  repObj = 0;

   } else { // Got space!

      #ifdef __stPRINTMSG__
         cout << endl << "Simple insertion. " << endl;
      #endif //__stPRINTMSG__

      // Do we have two representatives to insert?
      if((sub1.Rep!=NULL)&&(hasNewRoot)) { // Yes!

         // So, do it!
         int insertIdx = currNode->AddEntry(sub1.Rep->GetSerializedSize(),
                                            sub1.Rep->Serialize());

         currNode->GetIndexEntry(insertIdx).Radius = sub1.Radius;
         currNode->GetIndexEntry(insertIdx).NEntries = sub1.NObjects;
         currNode->GetIndexEntry(insertIdx).PageID = sub1.RootID;


      #ifdef __stPRINTMSG__
         cout << endl << "SubTreeInfo - Radius: " << sub1.Radius << ". NObjects: " << sub1.NObjects << " . RootID: " << sub1.RootID << endl;
      #endif //__stPRINTMSG__

      } // end if

      // Add the representative
      int insertIdx = currNode->AddEntry(sub.Rep->GetSerializedSize(),
                                         sub.Rep->Serialize());

      currNode->GetIndexEntry(insertIdx).Radius = sub.Radius;
      currNode->GetIndexEntry(insertIdx).NEntries = sub.NObjects;
      currNode->GetIndexEntry(insertIdx).PageID = sub.RootID;
      currNode->GetIndexEntry(insertIdx).Distance = 0;


      #ifdef __stPRINTMSG__
         cout << endl << "SubTreeInfo - Radius: " << sub.Radius << ". NObjects: " << sub.NObjects << " . RootID: " << sub.RootID << endl;
      #endif //__stPRINTMSG__

   } // end if

   if(currNode != 0){
	   delete currNode;
	   currNode = 0;
   }

   return true;

} //end stSlimTree<ObjectType, EvaluatorType>::BulkInsert

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(ObjectType **objects, u_int32_t numObj, enum tBulkType type){
   return BulkLoadMemory(objects, numObj, 0.7, type);  // Default node occupancy
} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(ObjectType **objects, u_int32_t numObj, double nodeOccupancy, enum tBulkType type){

   u_int32_t objSize = objects[4]->GetSerializedSize();   //@TODO: get the medium or max obj size

   return BulkLoadMemory(objects, numObj, nodeOccupancy, objSize, type);
} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(ObjectType **objects, u_int32_t numObj, double nodeOccupancy, u_int32_t objSize, enum tBulkType type) {

   vector< SampleSon<ObjectType> > objs; // objects vector

   for(u_int32_t i=0;i<numObj;i++) {
      SampleSon<ObjectType> son(objects[i],0.0);
      objs.push_back(son);
   } //end for

   return BulkLoadMemory(objs, nodeOccupancy, objSize, type);

} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory
//-----------------------------------------------------------------------------
// Utils


template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSlimTree::getBulkHeight(u_int32_t numObj,u_int32_t objSize,double nodeOccupancy) {
   u_int32_t numIndexNodeObj = getNumIndexNodeObj(objSize)*nodeOccupancy;
   u_int32_t numLeafNodeObj = getNumLeafNodeObj(objSize)*nodeOccupancy;

   if(numObj<=numLeafNodeObj) return 1;


   // Bottom-up
   /*
   u_int32_t aux = numObj/numLeafNodeObj;
   u_int32_t height = 1;
   while(aux>numIndexNodeObj) {
        aux = aux/numIndexNodeObj;
        height++;
   }*/

   // Top-down
   u_int32_t aux = numObj/numIndexNodeObj;
   u_int32_t height = 1;
   while(aux>numLeafNodeObj) {
        aux = aux/numIndexNodeObj;
        height++;
   }

   return height+1;
}

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSlimTree::setBulkHeight(u_int32_t numObj,u_int32_t objSize,double nodeOccupancy) {

   Header->Height = getBulkHeight(numObj,objSize,nodeOccupancy);
   return Header->Height;
}

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSlimTree::getNodeFreeSize() {
   return GetPageManager()->GetMinimumPageSize() - stSlimNode::GetGlobalOverhead();
} //end stSlimTree<ObjectType, EvaluatorType>::getNodeFreeSize

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSlimTree::getNumIndexNodeObj(u_int32_t objSize) {
   u_int32_t nodeFreeSize = getNodeFreeSize();
   return ((nodeFreeSize)/(objSize + sizeof(stSlimIndexNode::stSlimIndexEntry)));
} //end stSlimTree<ObjectType, EvaluatorType>::getNumIndexNodeObj

template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSlimTree::getNumLeafNodeObj(u_int32_t objSize) {
   u_int32_t nodeFreeSize = getNodeFreeSize();
   return ((nodeFreeSize)/(objSize + sizeof(stSlimLeafNode::stSlimLeafEntry)));
} //end stSlimTree<ObjectType, EvaluatorType>::getNumLeafNodeObj


bool searchIdx(int array[], u_int32_t size, u_int32_t value) {
   u_int32_t tmpIdx = 0;
   for(;tmpIdx<size; tmpIdx++) {
      if(array[tmpIdx] == value) return true;
   } //end for
   return false;
} //end searchIdx

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(vector< SampleSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, enum tBulkType type){

   stSubtreeInfo firstSub;

   bool ret = BulkLoadMemory(objects, nodeOccupancy, objSize, firstSub, type);

   this->SetRoot(firstSub.RootID);

   // Update the Height
   setBulkHeight(objects.size(),objSize,nodeOccupancy);

   #ifdef __stPRINTMSG__
      cout << endl << "Height " << Header->Height << endl;
   #endif //__stPRINTMSG__

   // Update object count.
   UpdateObjectCounter(objects.size());

   // Report the modification.
   HeaderUpdate = true;

   return ret;
} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(vector< SampleSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, enum tBulkType type){

   u_int32_t numLeafNodeObj = getNumLeafNodeObj(objSize)*nodeOccupancy;

   bool insertLeaf = false;

   if(objects.size()<=numLeafNodeObj) insertLeaf = true;

   return BulkLoadMemory(objects, nodeOccupancy, objSize, sub, insertLeaf, type);
} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(vector< SampleSon<ObjectType> > objects, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, bool insertLeaf, enum tBulkType type){

   return BulkLoadMemory(objects, -1, nodeOccupancy, objSize, sub, insertLeaf, type);
} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory

//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::BulkLoadMemory(vector< SampleSon<ObjectType> > objects, int father, double nodeOccupancy, u_int32_t objSize, stSubtreeInfo & sub, bool insertLeaf, enum tBulkType type){

   u_int32_t numIndexNodeObj = getNumIndexNodeObj(objSize)*nodeOccupancy;
   u_int32_t numLeafNodeObj = getNumLeafNodeObj(objSize)*nodeOccupancy;



   u_int32_t numObj = objects.size();

   double numObjects = 0;

   double numMinObjects = 0;

   double minNodeOccupancy = 0.4; //@todo pass as parameter

   switch (type) {

      case bulkRANGE:
         numMinObjects = ceil((double)numIndexNodeObj*minNodeOccupancy);
      case bulkFIXED:
         numObjects = numIndexNodeObj;
         break;

      case bulkFUNCTION:
         numObjects = (double)numObj/(double)numLeafNodeObj;
         while(numObjects>numIndexNodeObj) {
            numObjects /= (double)numIndexNodeObj;
         }
         numObjects = ceil(numObjects);
         break;
   }

   if(numObj < numObjects) {
      numObjects = numObj;
   }


   #ifdef __stPRINTMSG__
      cout << "# Index Obj: " << numIndexNodeObj <<
      "# Leaf Obj: " << numLeafNodeObj <<
      "# Obj: " << numObj << endl;
   #endif //__stPRINTMSG__

   if(insertLeaf) { // insert on leaf?

      #ifdef __stPRINTMSG__
         cout << endl << "Insert in leaf node" <<endl;
      #endif //__stPRINTMSG__

      // new leaf node
      stPage * newPage  = this->NewPage();
      stSlimLeafNode * leafNode = new stSlimLeafNode(newPage, true);

      u_int32_t repIdx = father;

      // insert all
      for(u_int32_t i=0;i<numObj;i++) {

         ObjectType *newObj = objects[i].getObject();

         u_int32_t insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),
                                                newObj->Serialize());

         // distance calculation
         leafNode->GetLeafEntry(insertIdx).Distance = this->myMetricEvaluator->GetDistance(newObj, objects[repIdx].getObject());

      } //end for

      sub.Radius = leafNode->GetMinimumRadius();
      sub.NObjects = numObj;
      sub.RootID = newPage->GetPageID();
      sub.Rep =  objects[repIdx].getObject();

      // clean the mess
      delete leafNode;
	  leafNode = 0;

      // write to disk
      tMetricTree::myPageManager->WritePage(newPage);
      delete newPage;
	  newPage = 0;


   } else {

      // choose samples
      SampleSon<ObjectType> *sample = new SampleSon<ObjectType>[numObjects]; // holds sample data
      int *sampleIdx = new int[numObjects]; // holds sample indexes
      u_int32_t tmpIdx = 0;
      if(father>=0) { // put the father as a sample
         sampleIdx[0] = father;
         sample[0] = objects[father];
         tmpIdx = 1;
      }

      for(;tmpIdx<numObjects; tmpIdx++) {
         sampleIdx[tmpIdx] = -1;
      }

      for(tmpIdx = 0;tmpIdx<numObjects; tmpIdx++) {
         u_int32_t aux = rand()%numObj;
         while(searchIdx(sampleIdx,numObjects,aux)) { // avoid duplicated samples
            aux = rand()%numObj;
         }

         sampleIdx[tmpIdx] = aux;
         sample[tmpIdx] = objects[aux];

         #ifdef __stPRINTMSG__
            cout << "# " << aux << " - " << numObjects << endl;
         #endif //__stPRINTMSG__
      } //end for
      delete[] sampleIdx;
	  sampleIdx = 0;

      vector< vector< SampleSon<ObjectType> > > samplesVector(numObjects);
      //vector< vector< SampleSon<ObjectType> > > samplesVector();


      // distribute objects
      for(u_int32_t i=0;i<numObj;i++) {
         int choice = -1;
         double dist = 0.0;
         double distOld = 0.0;
	 for(u_int32_t j=0;j<numObjects;j++) {
	    dist = this->myMetricEvaluator->GetDistance(sample[j].getObject(), objects[i].getObject());
	    if((choice == -1) || (dist < distOld)) {
	       distOld = dist;
               choice = j;
	    } //end if
         } //end for



         SampleSon<ObjectType> son(objects[i].getObject(),distOld);
         #ifdef __stPRINTMSG__
            cout << "Sample #: " << choice << " Object #: " << i << " Distance: " << distOld << endl;
         #endif //__stPRINTMSG__

	 samplesVector[choice].push_back(son); // opt
      } //end for

      u_int32_t numberObjBucket = numObj/numObjects;
      u_int32_t numberObjRem = numObj - numberObjBucket*numObjects;

      #ifdef __stPRINTMSG__
         cout << "Estimate number per bucket: " << numberObjBucket << " # rem: " << numberObjRem << endl;
         u_int32_t total = 0;
         for(u_int32_t i=0;i<samplesVector.size();i++) {
            cout << "Sample #: " << i << " Objects #: " << samplesVector[i].size() << endl;
            total +=  samplesVector[i].size();
         }
         cout << "Total: " << total << endl;
      #endif //__stPRINTMSG__

      vector<int> candidates;
      vector< SampleSon<ObjectType> > newObjs;



      // redistribution phase
      for(u_int32_t i=0;i<samplesVector.size();i++) {

         if(type==bulkRANGE) {

            if(samplesVector[i].size()<ceil((double)numberObjBucket*minNodeOccupancy)) {
               for(u_int32_t idx=0;idx<samplesVector[i].size();idx++) {
                  newObjs.push_back(samplesVector[i][idx]);
               } //end for
               samplesVector.erase(& samplesVector[i]);
               i--;

      #ifdef __stPRINTMSG__
         cout << "Estimate number per bucket: " << numberObjBucket << " # rem: " << numberObjRem << endl;
         u_int32_t total = 0;
         for(u_int32_t i=0;i<samplesVector.size();i++) {
            cout << "Sample #: " << i << " Objects #: " << samplesVector[i].size() << endl;
            total +=  samplesVector[i].size();
         }
         cout << "Total: " << total << endl;
      #endif //__stPRINTMSG__
                     
            } else {
               candidates.push_back(i);
            } //end if

         } else {


            if(samplesVector[i].size()>numberObjBucket) { // need to balance
               while(samplesVector[i].size()>numberObjBucket) {
                  // choose the fartest
                  u_int32_t fartest = 0;
                  double dist = samplesVector[i][0].getDistance();
                  for(u_int32_t idx=1;idx<samplesVector[i].size();idx++) {
                     if(samplesVector[i][idx].getDistance()>dist) {
                        dist = samplesVector[i][idx].getDistance();
                        fartest = idx;
                     } //end if
                  } //end for

                  newObjs.push_back(samplesVector[i][fartest]);
                  samplesVector[i].erase(& samplesVector[i][fartest]);
               } //end while
            } else { // candidates
               if(samplesVector[i].size()!=numberObjBucket) {
                  candidates.push_back(i);
               } //end if
            } //end if
            
         } //end if

      } //end for


      ObjectType *uniRep = NULL;

      // redistribute objects
      for(u_int32_t i=0;i<newObjs.size();i++) {
         int choice = -1;
         int candidateIdx;
         double dist = 0.0;
         double distOld = 0.0;
	 for(u_int32_t j=0;j<candidates.size();j++) {
	    dist = this->myMetricEvaluator->GetDistance(sample[candidates[j]].getObject(), newObjs[i].getObject());
	    if((choice == -1) || (dist < distOld)) {
	       distOld = dist;
               choice = candidates[j];
               candidateIdx = j;
	    } //end if
         } //end for


         if(choice == -1) { // remaining

            for(u_int32_t j=0;j<samplesVector.size();j++) {
               if(samplesVector[j].size()>numberObjBucket) {
                  continue;
               } //end if
	       dist = this->myMetricEvaluator->GetDistance(sample[j].getObject(), newObjs[i].getObject());
	       if((choice == -1) || (dist < distOld)) {
	          distOld = dist;
                  choice = j;
	       } //end if
            } //end for
         } else {
            if(samplesVector[choice].size()+1>=numberObjBucket) {
               candidates.erase(& candidates[candidateIdx]);
            } //end if
         } //end if

         SampleSon<ObjectType> son(newObjs[i].getObject(),distOld);
         #ifdef __stPRINTMSG__
            cout << "Sample #: " << choice << " Object #: " << i << " Distance: " << distOld << endl;
         #endif //__stPRINTMSG__

         samplesVector[choice].push_back(son); // opt

      } //end for

      #ifdef __stPRINTMSG__
         cout << "Estimate number per bucket: " << numberObjBucket << endl;
         u_int32_t total2 = 0;
         for(u_int32_t i=0;i<samplesVector.size();i++) {
            cout << "Sample #: " << i << " Objects #: " << samplesVector[i].size() << endl;
            total2 +=  samplesVector[i].size();
         } //end for
         cout << "Total: " << total2 << endl;
      #endif //__stPRINTMSG__

      // do while?

      if(type==bulkRANGE) {
         int minHeight = -1;

         // minimum height - @todo: create a function
         for(u_int32_t i=0;i<samplesVector.size();i++) {
            u_int32_t actHeight = getBulkHeight(samplesVector[i].size(),objSize,minNodeOccupancy);
            if((minHeight<0)||(minHeight > actHeight)) {
               minHeight = actHeight;
            }// end if
         } //end for



         for(u_int32_t i=0;i<samplesVector.size();i++) {
            u_int32_t maxHeight = getBulkHeight(samplesVector[i].size(),objSize,nodeOccupancy);
            if(maxHeight>minHeight) { // taller subtrees
            #ifdef __stPRINTMSG__
                cout << endl << "Break it " << i << endl;
            #endif //__stPRINTMSG__
                u_int32_t numDiv = 2;


                while(getBulkHeight(ceil((double)samplesVector[i].size()/(double)numDiv),objSize,nodeOccupancy)>minHeight) {
                   numDiv++;
                } //end while
                u_int32_t possNumDiv = ceil((double)samplesVector[i].size()/(double)numberObjBucket);
                if(possNumDiv>numDiv)
                   numDiv = possNumDiv;

                vector< vector< SampleSon<ObjectType> > > newSamplesVector(numDiv);

                // redistribute objects
                for(u_int32_t idx=0;idx<samplesVector[i].size();idx++) {
                   int choice = -1;
                   int candidateIdx;
                   double dist = 0.0;
                   double distOld = 0.0;
                   for(u_int32_t j=0;j<numDiv;j++) {

                      dist = this->myMetricEvaluator->GetDistance(samplesVector[i][j].getObject(), samplesVector[i][idx].getObject());
                      if((choice == -1) || (dist < distOld)) {
                         distOld = dist;
                         if(newSamplesVector[j].size() < numberObjBucket)
                            choice = j;
                      } //end if
                   } //end for

                   SampleSon<ObjectType> son(samplesVector[i][idx].getObject(),distOld);
                   newSamplesVector[choice].push_back(son); // opt

                } //end for

                samplesVector.erase(& samplesVector[i]);

                for(u_int32_t idx=0;idx<numDiv;idx++) {
                   samplesVector.push_back(newSamplesVector[idx]);
                } //end for

            }// end if
         } //end for



      } //end if





      #ifdef __stPRINTMSG__
         cout << endl << "Insert" <<endl;
      #endif //__stPRINTMSG__

      stPage * newIndexPage  = this->NewPage();
      stSlimIndexNode * indexNode = new stSlimIndexNode(newIndexPage, true);

      bool insertLeaf = true; //samplesVector[0].size() <= numLeafNodeObj;

      for(int i=0;i<samplesVector.size();i++) {
         if(samplesVector[i].size() > numLeafNodeObj) {
            insertLeaf = false;
            break;
         } //end if
      } //end for

      #ifdef __stPRINTMSG__
         cout << "InsertLeaf: " << insertLeaf << endl;
         cout << "numLeafNodeObj: " << numLeafNodeObj << endl;
         u_int32_t total3 = 0;
         for(u_int32_t i=0;i<samplesVector.size();i++) {
            cout << "Sample #: " << i << " Objects #: " << samplesVector[i].size() << endl;
            total3 +=  samplesVector[i].size();
         } //end for
         cout << "Total: " << total3 << endl;
      #endif //__stPRINTMSG__

      for(u_int32_t i=0;i<samplesVector.size();i++) {

         BulkLoadMemory(samplesVector[i], 0, nodeOccupancy, objSize, sub, insertLeaf, type);

         //@TODO: To reduce memory usage and increase disk access, do the bulk and after, load the node and update the information
         ObjectType *newIndexObj = sub.Rep;

         u_int32_t insertIdx = indexNode->AddEntry(newIndexObj->GetSerializedSize(),
                                        newIndexObj->Serialize());

        indexNode->GetIndexEntry(insertIdx).Radius = sub.Radius;
        indexNode->GetIndexEntry(insertIdx).NEntries = sub.NObjects;
        indexNode->GetIndexEntry(insertIdx).PageID = sub.RootID;
        if((i==father)||(father<0)) {
           uniRep = newIndexObj;
           indexNode->GetIndexEntry(insertIdx).Distance = 0;
        } else {
           indexNode->GetIndexEntry(insertIdx).Distance = this->myMetricEvaluator->GetDistance(newIndexObj, uniRep);
        } //end if

      } //end for

      sub.Radius = indexNode->GetMinimumRadius();
      sub.NObjects = indexNode->GetTotalObjectCount();
      sub.RootID = newIndexPage->GetPageID();
      sub.Rep =  uniRep;

      // clean the mess
      delete indexNode;
	  indexNode = 0;

      tMetricTree::myPageManager->WritePage(newIndexPage);
      delete newIndexPage;
	  newIndexPage = 0;

      delete[] sample;
	  sample = 0;

   } //end if

   return true;
} //end stSlimTree<ObjectType, EvaluatorType>::BulkLoadMemory

#endif //__BULKLOAD__

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::Consistency(){

   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t numberOfEntries;
   ObjectType subRep;
   double radius;
   u_int32_t maxHeight, minHeight;
   u_int32_t * heights;
   u_int32_t objectCount;
   u_int32_t subtreeObjects;
   bool result = true;

   // Let's search
   if (this->GetRoot() == 0){
      // Problem! The header has wrong value for the root node.
      result = false;
   }else{
      // Read the root node
      currPage = tMetricTree::myPageManager->GetPage(this->GetRoot());
      // Test the root pageID.
      if (currPage == NULL){
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in the root!";
         #endif //__stPRINTMSG__
         // Problem!
         result = false;
      }else{
         // Get the node.
         currNode =  stSlimNode::CreateNode(currPage);
         result = true;
         objectCount = 0;

         // Is it an Index node?
         if (currNode->GetNodeType() == stSlimNode::INDEX){
            // Get Index node
            stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();
            heights = new u_int32_t[numberOfEntries];
            // Set the height variables.
            maxHeight = 0;
            minHeight = 0;
            // For each entry...
            for (idx = 0; idx < numberOfEntries; idx++) {
               // set the subtree objects.
               subtreeObjects = 0;
               // Rebuild the object
               subRep.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Call it recursively.
               result = result && this->Consistency(indexNode->GetIndexEntry(idx).PageID,
                                                    &subRep, radius, heights[idx], 
                                                    subtreeObjects);
               // Test the subtree radius.
               if (radius != indexNode->GetIndexEntry(idx).Radius){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a radius problem with entry " << idx
                          << " in pageID: " << indexNode->GetIndexEntry(idx).PageID
                          << " radius " << indexNode->GetIndexEntry(idx).Radius
                          << " measured " << radius << ".";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
               // Test the max height
               if (heights[idx] > heights[maxHeight]){
                  maxHeight = idx;
               }//end if
               // Test the min height
               if (heights[idx] < heights[minHeight]){
                  minHeight = idx;
               }//end if
               // Test the subtree number of objects.
               if (indexNode->GetIndexEntry(idx).NEntries != subtreeObjects){
                  //Ops, problem
                  #ifdef __stPRINTMSG__
                     cout << "\nThe subtree pageID " << indexNode->GetIndexEntry(idx).PageID
                          << " is set with " << indexNode->GetIndexEntry(idx).NEntries
                          << " objects but has " << subtreeObjects
                          << " objects.";
                  #endif //__stPRINTMSG__
                  result = false;
               }//end if
               objectCount += subtreeObjects;
            }//end for
            // Verify the subtrees regarding the heights.
            if (heights[minHeight] != heights[maxHeight]){
               #ifdef __stPRINTMSG__
                  cout << "\nThe subtree pageID " << indexNode->GetIndexEntry(minHeight).PageID
                       << " with height " << heights[minHeight]
                       << " in idx " << minHeight
                       << " and the subtree pageID " << indexNode->GetIndexEntry(maxHeight).PageID
                       << " with height " << heights[maxHeight]
                       << " in idx " << maxHeight
                       << " both in pageID " << this->GetRoot()
                       << " have different heights from the others";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }//end if
            // release memory.
            delete[] heights;
			heights = 0;
         }else{
            // No, it is a leaf node. There is only the header to test it.
            stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
            if (leafNode->GetNumberOfEntries() != Header->ObjectCount){
               // Ops, problem with the header.
               result = false;
               #ifdef __stPRINTMSG__
                  cout << "\nThe header is set with: " << Header->ObjectCount
                       << " objects and the unique leaf node with: " << leafNode->GetNumberOfEntries()
                       << ". Both should have the same number.";
               #endif //__stPRINTMSG__
            }//end if
         }//end else

         // Free it all
         delete currNode;
		 currNode = 0;
         tMetricTree::myPageManager->ReleasePage(currPage);
      }//end if
   }//end if

   return result;
}//end stSlimTree::Consistency

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSlimTree::Consistency(u_int32_t pageID, ObjectType * repObj, 
                                  double & radius, u_int32_t & height, 
                                  u_int32_t & objectCount){

   u_int32_t idx;
   stPage * currPage;
   stSlimNode * currNode;
   u_int32_t numberOfEntries;
   double distance;
   ObjectType subRep, localRep, tmpObj;
   u_int32_t maxHeight, minHeight;
   u_int32_t * heights;
   u_int32_t subtreeObjects;
   int idxRep;
   radius = MAXDOUBLE;
   bool result = true;

   // Let's search
   if (pageID == 0){
      // Problem, invalid pageID!
      result = false;
   }else{
      // Read the node.
      currPage = tMetricTree::myPageManager->GetPage(pageID);
      // Test the pageID consistency.
      if (currPage == NULL){
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in " << pageID << ".";
         #endif //__stPRINTMSG__
         // Problem!
         result = false;
      }else{
         // Get the node.
         currNode = stSlimNode::CreateNode(currPage);
         // Is it an Index node?
         if (currNode->GetNodeType() == stSlimNode::INDEX) {
            // Get Index node
            stSlimIndexNode * indexNode = (stSlimIndexNode *)currNode;
            numberOfEntries = indexNode->GetNumberOfEntries();
            heights = new u_int32_t[numberOfEntries];
            // Set the height variables.
            maxHeight = 0;
            minHeight = 0;   
            // Get the representative entry of this node.
            idxRep = indexNode->GetRepresentativeEntry();
            // Test if there is a representative.
            if ((idxRep < 0) || (idxRep >= numberOfEntries)){
               #ifdef __stPRINTMSG__
                  cout << "\nThere is no representative entry in pageID "
                       << pageID << ".";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }else{
               // Get the representative.
               localRep.Unserialize(indexNode->GetObject(idxRep),
                                    indexNode->GetObjectSize(idxRep));
               // Check the field entries.
               result = true;
               // Test all entries in this node.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Set the subtree number of objects.
                  subtreeObjects = 0;
                  // Get the subtree representative.
                  subRep.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
            
                  result = result && this->Consistency(indexNode->GetIndexEntry(idx).PageID,
                                                       &subRep, radius, heights[idx],
                                                       subtreeObjects);
                  // Test the subtree radius with the local field.
                  if (radius != indexNode->GetIndexEntry(idx).Radius){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a radius problem with entry " << idx
                             << " in Index pageID: " << indexNode->GetIndexEntry(idx).PageID
                             << " radius " << indexNode->GetIndexEntry(idx).Radius
                             << " measured " << radius << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if
            
                  // Calculate the distance.
                  if (idx != (u_int32_t )idxRep){
                     tmpObj.Unserialize(indexNode->GetObject(idx),
                                        indexNode->GetObjectSize(idx));
                     // Evaluate it!
                     distance = this->myMetricEvaluator->GetDistance(*repObj, tmpObj);
                  }else{
                     // The distance is already calculated.
                     distance = 0;
                  }//end if
                  // Test the distance for every entry.
                  if (distance != indexNode->GetIndexEntry(idx).Distance){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a distance problem in entry " << idx
                             << " in Index pageID " << currPage->GetPageID()
                             << " distance: " << indexNode->GetIndexEntry(idx).Distance
                             << " measured " << distance << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if
                  // Test the max height.
                  if (heights[idx] > heights[maxHeight]){
                     maxHeight = idx;
                  }//end if
                  // Test the min height
                  if (heights[idx] < heights[minHeight]){
                     minHeight = idx;
                  }//end if
                  // Test the subtree number of objects.
                  if (indexNode->GetIndexEntry(idx).NEntries != subtreeObjects){
                     //Ops, problem
                     #ifdef __stPRINTMSG__
                        cout << "\nThe subtree pageID " << indexNode->GetIndexEntry(idx).PageID
                             << " is set with  " << indexNode->GetIndexEntry(idx).NEntries
                             << " objects but has " << subtreeObjects
                             << " objects.";
                     #endif //__stPRINTMSG__
                     result = false;
                  }//end if
                  objectCount += subtreeObjects;
               }//end for
               
               // Set the radius for the upper levels.
               radius = indexNode->GetMinimumRadius();
               // Test if the representative is the same to the upper level.
               if (!localRep.IsEqual(repObj)){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a problem in the representative entry " << idxRep
                          << " in Index pageID " << currPage->GetPageID()
                          << ". The representative is not the same of the upper level.";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
            }//end if
            // Verify the subtrees regarding the heights.
            if (heights[minHeight] != heights[maxHeight]){
               #ifdef __stPRINTMSG__
                  cout << "\nThe subtree pageID " << indexNode->GetIndexEntry(minHeight).PageID
                       << " with height " << heights[minHeight]
                       << " in idx " << minHeight
                       << " and the subtree pageID " << indexNode->GetIndexEntry(maxHeight).PageID
                       << " with height " << heights[maxHeight]
                       << " in idx " << maxHeight
                       << " both in pageID " << currPage->GetPageID()
                       << " have different heights from the others";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }//end if
            // return the minimum height.
            height = heights[maxHeight] + 1;
            // release memory.
            delete[] heights;
			heights = 0;
         }else{
            // It is a leaf node. Get it.
            stSlimLeafNode * leafNode = (stSlimLeafNode *)currNode;
            numberOfEntries = leafNode->GetNumberOfEntries();
            objectCount += numberOfEntries;
   
            // Get the representative entry of this node.
            idxRep = leafNode->GetRepresentativeEntry();
            // Test if there is a representative.
            if ((idxRep < 0) || (idxRep >= numberOfEntries)){
               #ifdef __stPRINTMSG__
                  cout << "\nThere is no representative entry in pageID "
                       << pageID << ".";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }else{
               // Get the representative.
               localRep.Unserialize(leafNode->GetObject(idxRep),
                                    leafNode->GetObjectSize(idxRep));
               // Check the field entries.
               result = true;
               // Test all entries in this node.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Get the subtree representative.
                  subRep.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
            
                  // Calculate the distance.
                  if (idx != (u_int32_t )idxRep){
                     tmpObj.Unserialize(leafNode->GetObject(idx),
                                        leafNode->GetObjectSize(idx));
                     // Evaluate it!
                     distance = this->myMetricEvaluator->GetDistance(*repObj, tmpObj);
                  }else{
                     // The distance is already calculated.
                     distance = 0;
                  }//end if
                  // Test the distance for every entry.
                  if (distance != leafNode->GetLeafEntry(idx).Distance){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a distance problem in entry " << idx
                             << " in Leaf pageID " << currPage->GetPageID()
                             << " distance: " << leafNode->GetLeafEntry(idx).Distance
                             << " measured " << distance << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if
               }//end for
               // Set the radius for the upper levels.
               radius = leafNode->GetMinimumRadius();
               // Test if the representative is the same to the upper level.
               if (!localRep.IsEqual(repObj)){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a problem in the representative entry " << idxRep
                          << " in Leaf pageID " << currPage->GetPageID()
                          << ". The representative is not the same of the upper level.";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
            }//end if
            // return height.
            height = 1;
         }//end else
   
         // Free it all
         delete currNode;
		 currNode = 0;
         tMetricTree::myPageManager->ReleasePage(currPage);
      }//end if
   }//end if

   return result;
}//end stSlimTree::Consistency
//-----------------------------------------------------------------------------

#ifdef __stCKNNQ__

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::preConstrainedNearestQuery(
                                                                   tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value, DataBlockManagerType& dataBlockManager) {

  //tResult * result;
  tConstrainedResult * result;
  u_int32_t i;

  // Create result
  //result = new tResult(k);
  result = new tConstrainedResult(k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the "row id" (pageId and offset) pointed by the tmpObj index entry, which is stored serialized in the OID of tmpObj
              TupleTypeIndex tupleIndex;
              tupleIndex.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              unsigned char* buffer;
              u_int32_t size;
              TupleTypeData tuple;
              tObject dummyObj;

              // Get a copy of the serialized object, accessing the pointed data page
              dataBlockManager.getSerializedObject(*((long*) tupleIndex.get(0)), *((long*) tupleIndex.get(1)), buffer, size);

              // Rebuild the object that stores: the feature vector and the remainder attributes (in the OID field)
              dummyObj.Unserialize(buffer, size);
              // Free the serialized copy
              delete[] buffer;

              // Get the data tuple storing the remainder attributes
              tuple.Unserialize(dummyObj.GetStrOID(), dummyObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              if (compare(tuple.get(idxConstraint), value)) {
                // Add the object.
//                result->AddPair((ObjectType*) tmpObj.Clone(), distance);//errado!
                //  result->AddPair((ObjectType*) dummyObj.Clone(), distance);
                result->AddSatisfyPair((ObjectType*) dummyObj.Clone(), distance);
                //cut if there is more than k elements
                result->Cut(k);
                //may I use this for performance?
                if (result->GetNumOfEntries() >= k)
                  rangeK = result->GetMaximumDistance();
              }//end if
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;

}//end preConstrainedNearestQuery


//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountGreaterThanOrEqual(//Obs: CountGreaterThan(5) can be CountGreaterThanOrEqual(6)
                                                                                            tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                            u_int32_t aggValue, DataBlockManagerType& dataBlockManager) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the "row id" (pageId and offset) pointed by the tmpObj index entry, which is stored serialized in the OID of tmpObj
              TupleTypeIndex tupleIndex;
              tupleIndex.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              unsigned char* buffer;
              u_int32_t size;
              TupleTypeData tuple;
              tObject dummyObj;

              // Get a copy of the serialized object, accessing the pointed data page
              dataBlockManager.getSerializedObject(*((long*) tupleIndex.get(0)), *((long*) tupleIndex.get(1)), buffer, size);

              // Rebuild the object that stores: the feature vector and the remainder attributes (in the OID field)
              dummyObj.Unserialize(buffer, size);
              // Free the serialized copy
              delete[] buffer;

              // Get the data tuple storing the remainder attributes
              tuple.Unserialize(dummyObj.GetStrOID(), dummyObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              if (compare(tuple.get(idxConstraint), value)) {

                // Unnecessary to check. Just add.
                result->AddSatisfyPair(dummyObj.Clone(), distance);
//                result->AddSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end if
              else {
                // Is there room for elements not satisfying the constraint?
                if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                  result->AddNotSatisfyPair(dummyObj.Clone(), distance);
//                  result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that does not satisfy the constraint?
                  if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddNotSatisfyPair(dummyObj.Clone(), distance);
//                    result->AddNotSatisfyPair(tmpObj.Clone(), distance);
                    result->CutNotSatisfyPairs(k - aggValue);
              
                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;

}//end intraConstrainedNearestQueryCountGreaterThanOrEqual

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountLessThanOrEqual(//Obs: CountLessThan(5) can be CountLessThanOrEqual(4)
                                                                                         tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                         u_int32_t aggValue, DataBlockManagerType& dataBlockManager) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the "row id" (pageId and offset) pointed by the tmpObj index entry, which is stored serialized in the OID of tmpObj
              TupleTypeIndex tupleIndex;
              tupleIndex.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              unsigned char* buffer;
              u_int32_t size;
              TupleTypeData tuple;
              tObject dummyObj;

              // Get a copy of the serialized object, accessing the pointed data page
              dataBlockManager.getSerializedObject(*((long*) tupleIndex.get(0)), *((long*) tupleIndex.get(1)), buffer, size);

              // Rebuild the object that stores: the feature vector and the remainder attributes (in the OID field)
              dummyObj.Unserialize(buffer, size);
              // Free the serialized copy
              delete[] buffer;

              // Get the data tuple storing the remainder attributes
              tuple.Unserialize(dummyObj.GetStrOID(), dummyObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              if (compare(tuple.get(idxConstraint), value)) {

                // Is there room for elements satisfying the constraint?
                if (result->GetNumOfEntriesSatisfyPairs() < (aggValue)) {
                  result->AddSatisfyPair(dummyObj.Clone(), distance);
//                  result->AddSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that satisfies the constraint?
                  if ((result->GetNumOfEntriesSatisfyPairs() > 0) && (distance < result->GetLastSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddSatisfyPair(dummyObj.Clone(), distance);
//                    result->AddSatisfyPair(tmpObj.Clone(), distance);
                    result->CutSatisfyPairs(aggValue);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end if
              else {
                // Unnecessary to check. Just add.
                result->AddNotSatisfyPair(dummyObj.Clone(), distance);
//                result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;
}//end intraConstrainedNearestQueryCountLessThanOrEqual

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual(
                                                                                                    tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                                    u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue, DataBlockManagerType& dataBlockManager) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the "row id" (pageId and offset) pointed by the tmpObj index entry, which is stored serialized in the OID of tmpObj
              TupleTypeIndex tupleIndex;
              tupleIndex.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              unsigned char* buffer;
              u_int32_t size;
              TupleTypeData tuple;
              tObject dummyObj;

              // Get a copy of the serialized object, accessing the pointed data page
              dataBlockManager.getSerializedObject(*((long*) tupleIndex.get(0)), *((long*) tupleIndex.get(1)), buffer, size);

              // Rebuild the object that stores: the feature vector and the remainder attributes (in the OID field)
              dummyObj.Unserialize(buffer, size);
              // Free the serialized copy
              delete[] buffer;

              // Get the data tuple storing the remainder attributes
              tuple.Unserialize(dummyObj.GetStrOID(), dummyObj.GetStrOIDSize());

              bool duplicateFound = false;

              // Is it qualified regarding the constraint?
              if (compare(tuple.get(idxConstraint), value)) {

                // Look for a duplicate
                for (typename tConstrainedResult::tIteSPairs it = result->beginSatisfyPairs(); it != result->endSatisfyPairs(); it++) {
                  // Get the object
                  tObject * tmpSatisfy;
                  tmpSatisfy = (tObject *) (*(*it))->GetObject();

                  // Get the tuple
                  TupleTypeData tupleSatisfy;
                  tupleSatisfy.Unserialize(tmpSatisfy->GetStrOID(), tmpSatisfy->GetStrOIDSize());

                  // Is it a duplicate?
                  if (aggCompare(tuple.get(aggIdx), tupleSatisfy.get(aggIdx))) {

                    duplicateFound = true;
                    // Is it closer than the duplicate?
                    if (distance < (*(*it))->GetDistance()) {

                      // Add the closer duplicate to the satisfying list.
                      result->AddSatisfyPair(dummyObj.Clone(), distance);

                      // The farther duplicate must now be considered as a non-satisfying element.
                      // So, is there room for elements not satisfying the constraint?
                      if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                        // Yes, move it to the not satisfying list.
                        result->ToNotSatisfyPairs(it);

                        // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                        result->Cut(k);
                      }//end if
                      else {
                        // No. Is it closer than the farthest element that does not satisfy the constraint?
                        if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && ((*(*it))->GetDistance() < result->GetLastNotSatisfyPair()->GetDistance())) {

                          // Yes, exchange them.
                          result->ToNotSatisfyPairs(it);
                          result->CutNotSatisfyPairs(k - aggValue);
                        }//end if
                        else {
                          // No, remove it.
                          result->RemoveSatisfyPairs(it);
                        }
                      }//end else

                      // May I use this for performance?
                      if (result->GetNumOfEntries() >= k) {
                        rangeK = result->GetMaximumDistance();
                      }//end if
                    }//end if
                    else {
                      // As tmp will not be considered in the counting constraint, it can be treated as a non-satisfying element.
                      // So, is there room for elements not satisfying the constraint?
                      if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                        // Yes, add it to the not satisfying list.
                        result->AddNotSatisfyPair(dummyObj.Clone(), distance);

                        // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                        result->Cut(k);

                        // May I use this for performance?
                        if (result->GetNumOfEntries() >= k) {
                          rangeK = result->GetMaximumDistance();
                        }//end if
                      }//end if
                      else {
                        // Is it closer than the farthest element that does not satisfy the constraint?
                        if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                          // Exchange them
                          result->AddNotSatisfyPair(dummyObj.Clone(), distance);
                          result->CutNotSatisfyPairs(k - aggValue);

                          // May I use this for performance?
                          if (result->GetNumOfEntries() >= k) {
                            rangeK = result->GetMaximumDistance();
                          }//end if
                        }//end if
                      }//end else
                    }//end else
                    break; // OBS: it is necessary to break the loop to avoid testing the loop's end condition, as the next iterator position could be deleted, yielding a segmentation fault.
                  }//end if
                }//end for

                if (!duplicateFound) {
                  result->AddSatisfyPair(dummyObj.Clone(), distance);

                  // Is it necessary to cut?
                  if (result->GetNumOfEntries() > k) {
                    // Is there more than aggValue satisfying elements?
                    if (result->GetNumOfEntriesSatisfyPairs() > aggValue) {
                      // Yes, so the last can be safely cut.
                      result->Cut(k);
                    }
                    else {
                      // No, cut the last non-satisfying element.
                      result->CutNotSatisfyPairs(k - aggValue);
                    }
                  }//end if

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
              }//end if
              else {
                // Is there room for elements not satisfying the constraint?
                if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                  result->AddNotSatisfyPair(dummyObj.Clone(), distance);

                  // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that does not satisfy the constraint?
                  if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddNotSatisfyPair(dummyObj.Clone(), distance);
                    result->CutNotSatisfyPairs(k - aggValue);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if

  return result;
}//end intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountDistinctLessThanOrEqual(
                                                                                                 tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                                 u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue, DataBlockManagerType& dataBlockManager) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the "row id" (pageId and offset) pointed by the tmpObj index entry, which is stored serialized in the OID of tmpObj
              TupleTypeIndex tupleIndex;
              tupleIndex.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              unsigned char* buffer;
              u_int32_t size;
              TupleTypeData tuple;
              tObject dummyObj;

              // Get a copy of the serialized object, accessing the pointed data page
              dataBlockManager.getSerializedObject(*((long*) tupleIndex.get(0)), *((long*) tupleIndex.get(1)), buffer, size);

              // Rebuild the object that stores: the feature vector and the remainder attributes (in the OID field)
              dummyObj.Unserialize(buffer, size);
              // Free the serialized copy
              delete[] buffer;

              // Get the data tuple storing the remainder attributes
              tuple.Unserialize(dummyObj.GetStrOID(), dummyObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              if (compare(tuple.get(idxConstraint), value)) {

                bool duplicateFound = false;

                for (typename tConstrainedResult::tIteSPairs it = result->beginSatisfyPairs(); it != result->endSatisfyPairs(); it++) {
                  // Get the object
                  tObject * tmpSatisfy;
                  tmpSatisfy = (tObject *) (*(*it))->GetObject();

                  // Get the tuple
                  TupleTypeData tupleSatisfy;
                  tupleSatisfy.Unserialize(tmpSatisfy->GetStrOID(),tmpSatisfy->GetStrOIDSize());

                  // Is it a duplicate?
                  if (aggCompare(tuple.get(aggIdx), tupleSatisfy.get(aggIdx))) {

                    duplicateFound = true;

                    // Is it closer than the duplicate?
                    if (distance < (*(*it))->GetDistance()) {
                      // Exchange them (the current element with its duplicate)
                      result->AddSatisfyPair(dummyObj.Clone(), distance);
                      result->RemoveSatisfyPairs(it);// the duplicate

                      // May I use this for performance?
                      if (result->GetNumOfEntries() >= k) {
                        rangeK = result->GetMaximumDistance();
                      }//end if

                      break;
                    }//end if
                  }//end if
                }//end for

                if (!duplicateFound) {
                  // Is there room for elements satisfying the constraint?
                  if (result->GetNumOfEntriesSatisfyPairs() < (aggValue)) {
                    result->AddSatisfyPair(dummyObj.Clone(), distance);

                    // If it is necessary to cut, the number of satisfying elements will not become greater than aggValue, so the last can be safely cut.
                    result->Cut(k);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                  else {
                    // Is it closer than the farthest element that satisfies the constraint?
                    if ((result->GetNumOfEntriesSatisfyPairs() > 0) && (distance < result->GetLastSatisfyPair()->GetDistance())) {

                      // Exchange them (the current element with the last satisfying one)
                      result->AddSatisfyPair(dummyObj.Clone(), distance);
//                      result->AddSatisfyPair(tmpObj.Clone(), distance);
                      result->CutSatisfyPairs(aggValue);

                      // May I use this for performance?
                      if (result->GetNumOfEntries() >= k) {
                        rangeK = result->GetMaximumDistance();
                      }//end if
                    }//end if
                  }//end else
                }//end if
              }//end if
              else {
                // Unnecessary to check. Just add.
                result->AddNotSatisfyPair(dummyObj.Clone(), distance);

                // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;
}//end intraConstrainedNearestQueryCountDistinctLessThanOrEqual




///WITH ATTR
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::preConstrainedNearestQuery(
    tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value) {

  //tResult * result;
  tConstrainedResult * result;
  u_int32_t i;

  // Create result
  //result = new tResult(k);
  result = new tConstrainedResult(k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);

  
  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              if (compare(tuple.get(idxConstraint), value)) {
                // Add the object.
                //result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                result->AddSatisfyPair((ObjectType*) tmpObj.Clone(), distance);

                // Cut if there is more than k elements
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end if
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;

}//end preConstrainedNearestQuery (Covering Index)


//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountGreaterThanOrEqual(//Obs: CountGreaterThan(5) can be CountGreaterThanOrEqual(6)
                                                                                            tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                            u_int32_t aggValue) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              if (compare(tuple.get(idxConstraint), value)) {

                // Unnecessary to check. Just add.
                result->AddSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end if
              else {
                // Is there room for elements not satisfying the constraint?
                if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                  result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that does not satisfy the constraint?
                  if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddNotSatisfyPair(tmpObj.Clone(), distance);
                    result->CutNotSatisfyPairs(k - aggValue);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;

}//end intraConstrainedNearestQueryCountGreaterThanOrEqual (Covering Index)

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountLessThanOrEqual(//Obs: CountLessThan(5) can be CountLessThanOrEqual(4)
                                                                                         tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                         u_int32_t aggValue) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              if (compare(tuple.get(idxConstraint), value)) {

                // Is there room for elements satisfying the constraint?
                if (result->GetNumOfEntriesSatisfyPairs() < (aggValue)) {
                  result->AddSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that satisfies the constraint?
                  if ((result->GetNumOfEntriesSatisfyPairs() > 0) && (distance < result->GetLastSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddSatisfyPair(tmpObj.Clone(), distance);
                    result->CutSatisfyPairs(aggValue);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end if
              else {
                // Unnecessary to check. Just add.
                result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;
}//end intraConstrainedNearestQueryCountLessThanOrEqual (Covering Index)

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual(
                                                                                                    tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                                    u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              bool duplicateFound = false;

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              if (compare(tuple.get(idxConstraint), value)) {

                // Look for a duplicate
                for (typename tConstrainedResult::tIteSPairs it = result->beginSatisfyPairs(); it != result->endSatisfyPairs(); it++) {
                  // Get the object
                  tObject * tmpSatisfy;
                  tmpSatisfy = (tObject *) (*(*it))->GetObject();

                  TupleTypeIndex tupleSatisfy;
                  tupleSatisfy.Unserialize((unsigned char *) tmpSatisfy->GetStrOID(), tmpSatisfy->GetStrOIDSize());

                  // Is it a duplicate?
                  if (aggCompare(tuple.get(aggIdx), tupleSatisfy.get(aggIdx))) {

                    duplicateFound = true;

                    // Is it closer than the duplicate?
                    if (distance < (*(*it))->GetDistance()) {

                      // Add the closer duplicate to the satisfying list.
                      result->AddSatisfyPair(tmpObj.Clone(), distance);

                      // The farther duplicate must now be considered as a non-satisfying element.
                      // So, is there room for elements not satisfying the constraint?
                      if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                        // Yes, move it to the not satisfying list.
                        result->ToNotSatisfyPairs(it);

                        // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                        result->Cut(k);
                      }//end if
                      else {
                        // No. Is it closer than the farthest element that does not satisfy the constraint?
                        if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && ((*(*it))->GetDistance() < result->GetLastNotSatisfyPair()->GetDistance())) {

                          // Yes, exchange them.
                          result->ToNotSatisfyPairs(it);
                          result->CutNotSatisfyPairs(k - aggValue);
                        }//end if
                        else {
                          // No, remove it.
                          result->RemoveSatisfyPairs(it);
                        }
                      }//end else

                      // May I use this for performance?
                      if (result->GetNumOfEntries() >= k) {
                        rangeK = result->GetMaximumDistance();
                      }//end if
                    }//end if
                    else {
                      // As tmp will not be considered in the counting constraint, it can be treated as a non-satisfying element.
                      // So, is there room for elements not satisfying the constraint?
                      if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                        // Yes, add it to the not satisfying list.
                        result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                        // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                        result->Cut(k);

                        // May I use this for performance?
                        if (result->GetNumOfEntries() >= k) {
                          rangeK = result->GetMaximumDistance();
                        }//end if
                      }//end if
                      else {
                        // Is it closer than the farthest element that does not satisfy the constraint?
                        if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                          // Exchange them
                          result->AddNotSatisfyPair(tmpObj.Clone(), distance);
                          result->CutNotSatisfyPairs(k - aggValue);

                          // May I use this for performance?
                          if (result->GetNumOfEntries() >= k) {
                            rangeK = result->GetMaximumDistance();
                          }//end if
                        }//end if
                      }//end else
                    }//end else
                    break; // OBS: it is necessary to break the loop to avoid testing the loop's end condition, as the next iterator position could be deleted, yielding a segmentation fault.
                  }//end if
                }//end for

                if (!duplicateFound) {
                  result->AddSatisfyPair(tmpObj.Clone(), distance);

                  // Is it necessary to cut?
                  if (result->GetNumOfEntries() > k) {
                    // Is there more than aggValue satisfying elements?
                    if (result->GetNumOfEntriesSatisfyPairs() > aggValue) {
                      // Yes, so the last can be safely cut.
                      result->Cut(k);
                    }
                    else {
                      // No, cut the last non-satisfying element.
                      result->CutNotSatisfyPairs(k - aggValue);
                    }
                  }//end if

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
              }//end if
              else {
                // Is there room for elements not satisfying the constraint?
                if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                  result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that does not satisfy the constraint?
                  if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddNotSatisfyPair(tmpObj.Clone(), distance);
                    result->CutNotSatisfyPairs(k - aggValue);
                    
                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if

  return result;
}//end intraConstrainedNearestQueryCountDistinctGreaterThanOrEqual

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::intraConstrainedNearestQueryCountDistinctLessThanOrEqual(
                                                                                                 tObject * sample, u_int32_t k, u_int32_t idxConstraint, bool (*compare)(const void *, const void *), const void * value,
                                                                                                 u_int32_t aggIdx, bool (*aggCompare)(const void *, const void *), u_int32_t aggValue) {

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              bool duplicateFound = false;

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              if (compare(tuple.get(idxConstraint), value)) {

                // Look for a duplicate
                for (typename tConstrainedResult::tIteSPairs it = result->beginSatisfyPairs(); it != result->endSatisfyPairs(); it++) {
                  // Get the object
                  tObject * tmpSatisfy;
                  tmpSatisfy = (tObject *) (*(*it))->GetObject();

                  // Get the tuple
                  TupleTypeIndex tupleSatisfy;
                  tupleSatisfy.Unserialize((unsigned char *) tmpSatisfy->GetStrOID(), tmpSatisfy->GetStrOIDSize());

                  // Is it a duplicate?
                  if (aggCompare(tuple.get(aggIdx), tupleSatisfy.get(aggIdx))) {

                    duplicateFound = true;

                    // Is it closer than the duplicate?
                    if (distance < (*(*it))->GetDistance()) {
                      // Exchange them (the current element with its duplicate)
                      result->AddSatisfyPair(tmpObj.Clone(), distance);
                      result->RemoveSatisfyPairs(it);// the duplicate

                      // May I use this for performance?
                      if (result->GetNumOfEntries() >= k) {
                        rangeK = result->GetMaximumDistance();
                      }//end if

                      break;
                    }//end if
                  }//end if
                }//end for

                if (!duplicateFound) {
                  // Is there room for elements satisfying the constraint?
                  if (result->GetNumOfEntriesSatisfyPairs() < (aggValue)) {
                    result->AddSatisfyPair(tmpObj.Clone(), distance);

                    // If it is necessary to cut, the number of satisfying elements will not become greater than aggValue, so the last can be safely cut.
                    result->Cut(k);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                  else {
                    // Is it closer than the farthest element that satisfies the constraint?
                    if ((result->GetNumOfEntriesSatisfyPairs() > 0) && (distance < result->GetLastSatisfyPair()->GetDistance())) {

                      // Exchange them (the current element with the last satisfying one)
                      result->AddSatisfyPair(tmpObj.Clone(), distance);
                      result->CutSatisfyPairs(aggValue);

                      // May I use this for performance?
                      if (result->GetNumOfEntries() >= k) {
                        rangeK = result->GetMaximumDistance();
                      }//end if
                    }//end if
                  }//end else
                }//end if
              }//end if
              else {
                // Unnecessary to check. Just add.
                result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;
}//end intraConstrainedNearestQueryCountDistinctLessThanOrEqual



template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class RowId>
stResult<ObjectType> * tmpl_stSlimTree::BConstrainedNearestQuery(std::multiset<RowId, rowidComparator> m, tObject * sample, u_int32_t k) {

  //tResult * result;
  tConstrainedResult * result;
  u_int32_t i;

  // Create result
  //result = new tResult(k);
  result = new tConstrainedResult(k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);

  
  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;
    
    RowId rowid;  //?????????????????????????????????????????????

    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              rowid.PageId = *((u_int32_t*)tuple.get(0));
              rowid.OffSet = *((u_int32_t*)tuple.get(1));
              if(m.find(rowid) != m.end())
              {
                 // Add the object.
                //result->AddPair((ObjectType*) tmpObj.Clone(), distance);
                result->AddSatisfyPair((ObjectType*) tmpObj.Clone(), distance);

                // Cut if there is more than k elements
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end if
              
//              if (compare(tuple.get(idxConstraint), value)) {
//                // Add the object.
//                //result->AddPair((ObjectType*) tmpObj.Clone(), distance);
//                result->AddSatisfyPair((ObjectType*) tmpObj.Clone(), distance);
//
//                // Cut if there is more than k elements
//                result->Cut(k);
//
//                // May I use this for performance?
//                if (result->GetNumOfEntries() >= k) {
//                  rangeK = result->GetMaximumDistance();
//                }//end if
//              }//end if
              
              
              
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if


  return result;

}//end BConstrainedNearestQuery 
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::BSlimIntraConstrainedNearestQueryCountGreaterThanOrEqual(
    std::multiset<RowId, rowidComparator> m,tObject * sample, u_int32_t k,
    u_int32_t aggValue, DataBlockManagerType& dataBlockManager){
    
  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;
    
    RowId rowid;
    
    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
      
            if (distance <= rangeK) {
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.
              rowid.PageId = *((u_int32_t*)tuple.get(0));
              rowid.OffSet = *((u_int32_t*)tuple.get(1));
              
              if(m.find(rowid) != m.end()) {
                // Unnecessary to check. Just add.
                result->AddSatisfyPair(tmpObj.Clone(), distance);
//                result->AddSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end if
              else {
                // Is there room for elements not satisfying the constraint?
                if (result->GetNumOfEntriesNotSatisfyPairs() < (k - aggValue)) {
                  result->AddNotSatisfyPair(tmpObj.Clone(), distance);
//                  result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, there are more than aggValue satisfying elements, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that does not satisfy the constraint?
                  if ((result->GetNumOfEntriesNotSatisfyPairs() > 0) && (distance < result->GetLastNotSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddNotSatisfyPair(tmpObj.Clone(), distance);
//                    result->AddNotSatisfyPair(tmpObj.Clone(), distance);
                    result->CutNotSatisfyPairs(k - aggValue);
              
                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if
  return result;
}
// end BSlimIntraConstrainedNearestQueryCountGreaterThanOrEqual
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
template <class TupleTypeIndex, class TupleTypeData, class DataBlockManagerType>
stResult<ObjectType> * tmpl_stSlimTree::BSlimIntraConstrainedNearestQueryCountLessThanOrEqual(
    std::multiset<RowId, rowidComparator> m,tObject * sample, u_int32_t k,
    u_int32_t aggValue, DataBlockManagerType& dataBlockManager){

  stConstrainedResult<ObjectType > * result;
  u_int32_t i;

  // Create result
  result = new stConstrainedResult<ObjectType > (k);
  result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, false);


  // Let's search
  if (this->GetRoot() != 0) {
    tDynamicPriorityQueue * queue;
    u_int32_t idx;
    stPage * currPage;
    stSlimNode * currNode;
    ObjectType tmpObj;
    double distance;
    double distanceRepres = 0;
    u_int32_t numberOfEntries;
    stQueryPriorityQueueValue pqCurrValue;
    stQueryPriorityQueueValue pqTmpValue;
    bool stop;
    double rangeK = MAXDOUBLE;
    
    RowId rowid;
    
    // Root node
    pqCurrValue.PageID = this->GetRoot();
    pqCurrValue.Radius = 0;

    // Create the Global Priority Queue
    queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

    // Let's search
    while (pqCurrValue.PageID != 0) {
      // Read node...
      currPage = tMetricTree::myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSlimNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSlimNode::INDEX) {
        // Get Index node
        stSlimIndexNode * indexNode = (stSlimIndexNode *) currNode;
        numberOfEntries = indexNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this subtree with the triangle inequality.
          if (fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
              rangeK + indexNode->GetIndexEntry(idx).Radius) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius) {
              // Yes! I'm qualified! Put it in the queue.
              pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
              pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
              queue->Add(distance, pqTmpValue);
              this->sumOperationsQueue++; // Update the statistics for the queue
            }//end if
          }//end if
        }//end for
      }
      else {
        // No, it is a leaf node. Get it.
        stSlimLeafNode * leafNode = (stSlimLeafNode *) currNode;
        numberOfEntries = leafNode->GetNumberOfEntries();

        // for each entry...
        for (idx = 0; idx < numberOfEntries; idx++) {
          // try to cut this object with the triangle inequality.
          if (fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
              rangeK) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            //test if the object qualify
            if (distance <= rangeK) {   
              // Get the tuple
              TupleTypeIndex tuple;
              tuple.Unserialize((unsigned char *) tmpObj.GetStrOID(), tmpObj.GetStrOIDSize());

              // Is it qualified regarding the constraint?
              // NOTE: idxConstraint > 1 must point to an attribute defined during the covering index creation.


              rowid.PageId = *((u_int32_t*)tuple.get(0));
              rowid.OffSet = *((u_int32_t*)tuple.get(1));

              // Is it qualified regarding the constraint?
              if(m.find(rowid) != m.end()) {
                // Is there room for elements satisfying the constraint?
                if (result->GetNumOfEntriesSatisfyPairs() < (aggValue)) {
                  result->AddSatisfyPair(tmpObj.Clone(), distance);
//                  result->AddSatisfyPair(tmpObj.Clone(), distance);

                  // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                  result->Cut(k);

                  // May I use this for performance?
                  if (result->GetNumOfEntries() >= k) {
                    rangeK = result->GetMaximumDistance();
                  }//end if
                }//end if
                else {
                  // Is it closer than the farthest element that satisfies the constraint?
                  if ((result->GetNumOfEntriesSatisfyPairs() > 0) && (distance < result->GetLastSatisfyPair()->GetDistance())) {

                    // Exchange them
                    result->AddSatisfyPair(tmpObj.Clone(), distance);
//                    result->AddSatisfyPair(tmpObj.Clone(), distance);
                    result->CutSatisfyPairs(aggValue);

                    // May I use this for performance?
                    if (result->GetNumOfEntries() >= k) {
                      rangeK = result->GetMaximumDistance();
                    }//end if
                  }//end if
                }//end else
              }//end if
              else {
                // Unnecessary to check. Just add.
                result->AddNotSatisfyPair(tmpObj.Clone(), distance);
//                result->AddNotSatisfyPair(tmpObj.Clone(), distance);

                // If it is necessary to cut, the number of satisfying elements will not increase, so the last can be safely cut.
                result->Cut(k);

                // May I use this for performance?
                if (result->GetNumOfEntries() >= k) {
                  rangeK = result->GetMaximumDistance();
                }//end if
              }//end else
            }//end if
          }//end if
        }//end for

      }//end else

      // Free it all
      delete currNode;
      currNode = 0;
      tMetricTree::myPageManager->ReleasePage(currPage);

      if (queue->GetSize() > this->maxQueue)
        this->maxQueue = queue->GetSize();
      // Go to next node
      stop = false;
      do {
        if (queue->Get(distance, pqCurrValue)) {
          this->sumOperationsQueue++; // Update the statistics for the queue
          // Qualified if distance <= rangeK + radius
          if (distance <= rangeK + pqCurrValue.Radius) {
            // Yes, get the pageID and the distance from the representative
            // and the query object.
            distanceRepres = distance;
            // Break the while.
            stop = true;
          }//end if
        }
        else {
          // the queue is empty!
          pqCurrValue.PageID = 0;
          // Break the while.
          stop = true;
        }//end if
      }
      while (!stop);
    }// end while

    // Release the Global Priority Queue
    delete queue;
    queue = NULL;

  }//end if
  return result;  
}
// end BSlimIntraConstrainedNearestQueryCountLessThanOrEqual
#endif // __stCKNNQ__
