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
* This file is the implementation of stSeqTree methods.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Josiel Maimone de Figueiredo (josiel@icmc.usp.br)
* @author Adriano Siqueira Arantes (arantes@icmc.usp.br)
*/

//==============================================================================
// Class stSeqLogicNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSeqLogicNode<ObjectType, EvaluatorType>::stSeqLogicNode(u_int32_t maxOccupation){

   // Allocate resources
   MaxEntries = maxOccupation;
   Entries = new stSeqLogicEntry[MaxEntries];

   // Init Rep
   RepIndex[0] = 0;
   RepIndex[1] = 0;

   // Initialize
   Count = 0;

   // Minimum occupation. 25% is the default of Seq-tree
   MinOccupation = (u_int32_t) (0.25 * maxOccupation);
   // At least the nodes must store 2 objects.
   if ((MinOccupation > (maxOccupation/2)) || (MinOccupation == 0)){
      MinOccupation = 2;
   }//end if

}//end stSeqLogicNode<ObjectType, EvaluatorType>::stSeqLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSeqLogicNode<ObjectType, EvaluatorType>::~stSeqLogicNode(){
   u_int32_t i;

   if (Entries != NULL){
      for (i = 0; i < Count; i++){
         if ((Entries[i].Object != NULL) && (Entries[i].Mine)){
            delete Entries[i].Object;
         }//end if
      }//end for
   }//end if
   // Clean before exit.
   delete[] Entries;
}//end stSeqLogicNode<ObjectType, EvaluatorType>::~stSeqLogicNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSeqLogicNode<ObjectType, EvaluatorType>::AddEntry(u_int32_t size, const unsigned char * object){
   if (Count < MaxEntries){
      Entries[Count].Object = new ObjectType();
      Entries[Count].Object->Unserialize(object, size);
      Entries[Count].Mine = true;
      Count++;
      return Count - 1;
   }else{
      return -1;
   }//end if
}//end stSeqLogicNode<ObjectType, EvaluatorType>::AddEntry

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqLogicNode<ObjectType, EvaluatorType>::AddIndexNode(stSeqIndexNode * node){
   u_int32_t i;
   int idx;

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      idx = AddEntry(node->GetObjectSize(i), node->GetObject(i));
      SetEntry(idx, node->GetIndexEntry(i).PageID,
                    node->GetIndexEntry(i).Radius);
   }//end for

   // Node type
   NodeType = stSeqNode::INDEX;
}//end stSeqLogicNode<ObjectType, EvaluatorType>::AddIndexNode
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqLogicNode<ObjectType, EvaluatorType>::AddLeafNode(stSeqLeafNode * node){
   u_int32_t i;
   int idx;

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      idx = AddEntry(node->GetObjectSize(i), node->GetObject(i));
      SetEntry(idx, 0, 0);
   }//end for

   // Node type
   NodeType = stSeqNode::LEAF;
}//end stSeqLogicNode<ObjectType, EvaluatorType>::AddLeafNode

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stSeqLogicNode<ObjectType, EvaluatorType>::TestDistribution(
      stSeqIndexNode * node0, stSeqIndexNode * node1,
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
               node0->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }else{
               // Let's put it in the node 1 since it doesn't fit in the node 0
               idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node1->GetIndexEntry(idx).Distance = Entries[i].Distance[1];
               node1->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node1->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }//end if
         }else{
            // Try to put on node 1 first
            idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node1->GetIndexEntry(idx).Distance = Entries[i].Distance[1];
               node1->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node1->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }else{
               // Let's put it in the node 0 since it doesn't fit in the node 1
               idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node0->GetIndexEntry(idx).Distance = Entries[i].Distance[0];
               node0->GetIndexEntry(idx).PageID = Entries[i].PageID;
               node0->GetIndexEntry(idx).Radius = Entries[i].Radius;
            }//end if
         }//end if
      }//end if
   }//end for

   // Clean home before go away...
   delete[] idx0;
   delete[] idx1;

   return dCount;
}//end stSeqLogicNode<ObjectType, EvaluatorType>::TestDistribution

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stSeqLogicNode<ObjectType, EvaluatorType>::TestDistribution(
      stSeqLeafNode * node0, stSeqLeafNode * node1,
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
      node0->GetLeafEntry(idx).Distance = idx0[l0].Distance;

      // Find a candidate for node 1
      while (Entries[idx1[l1].Index].Mapped){
         l1++;
      }//end while
      // Add to node 1
      currObj = idx1[l1].Index;
      Entries[currObj].Mapped = true;
      idx = node1->AddEntry(Entries[currObj].Object->GetSerializedSize(),
                            Entries[currObj].Object->Serialize());
      node1->GetLeafEntry(idx).Distance = idx1[l1].Distance;
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
               node0->GetLeafEntry(idx).Distance = Entries[i].Distance[0];
            }else{
               // Let's put it in the node 1 since it doesn't fit in the node 0
               idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node1->GetLeafEntry(idx).Distance = Entries[i].Distance[1];
            }//end if
         }else{
            // Try to put on node 1 first
            idx = node1->AddEntry(Entries[i].Object->GetSerializedSize(),
                                  Entries[i].Object->Serialize());
            if (idx >= 0){
               node1->GetLeafEntry(idx).Distance = Entries[i].Distance[1];
            }else{
               // Let's put it in the node 0 since it doesn't fit in the node 1
               idx = node0->AddEntry(Entries[i].Object->GetSerializedSize(),
                                     Entries[i].Object->Serialize());
               node0->GetLeafEntry(idx).Distance = Entries[i].Distance[0];
            }//end if
         }//end if
      }//end if
   }//end for

   // Clean home before go away...
   delete idx0;
   delete idx1;

   return dCount;
}//end stSeqLogicNode<ObjectType, EvaluatorType>::TestDistribution

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t stSeqLogicNode<ObjectType, EvaluatorType>::UpdateDistances(
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
               Entries[RepIndex[0]].Object, Entries[i].Object);
         Entries[i].Distance[1] = metricEvaluator->GetDistance(
               Entries[RepIndex[1]].Object, Entries[i].Object);
      }//end if
   }//end for

   return (GetNumberOfEntries() * 2) - 2;
}//end stSeqLogicNode<ObjectType, EvaluatorType>::UpdateDistances

//=============================================================================
// Class template stSeqMSTSpliter
//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSeqMSTSplitter<ObjectType, EvaluatorType>::stSeqMSTSplitter(
      tLogicNode * node){

   Node = node;
   N = Node->GetNumberOfEntries();

   // Dynamic fields
   Cluster = new tCluster[N];
   ObjectCluster = new int[N];

   // Matrix
   DMat.SetSize(N, N);
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::stSeqMSTSplitter

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stSeqMSTSplitter<ObjectType, EvaluatorType>::~stSeqMSTSplitter(){

   if (Node != NULL){
      delete Node;
   }//end if
   if (Cluster != NULL){
      delete[] Cluster;
   }//end if
   if (ObjectCluster != NULL){
      delete[] ObjectCluster;
   }//end if
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::stSeqMSTSplitter

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSeqMSTSplitter<ObjectType, EvaluatorType>::BuildDistanceMatrix(
      EvaluatorType * metricEvaluator){
   int i;
   int j;

   for (i = 0; i < N; i++){
      DMat[i][i] = 0;
      for (j = 0; j < i; j++){
         DMat[i][j] = metricEvaluator->GetDistance(Node->GetObject(i),
                                                   Node->GetObject(j));
         DMat[j][i] = DMat[i][j];
      }//end for
   }//end for
   return ((1 - N) * N) / 2;
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::BuildDistanceMatrix

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSeqMSTSplitter<ObjectType, EvaluatorType>::FindCenter(int clus){
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
}//end  stSeqMSTSplitter<ObjectType, EvaluatorType>::FindCenter

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqMSTSplitter<ObjectType, EvaluatorType>::PerformMST(){
   int i, j, k, l, cc, iBig, iBigOpposite, a, b , c;
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
         throw logic_error("At least on object has no cluster.");
      }//end if
   }//end for
   #endif //__stDEBUG__

   // Representatives
   Node->SetRepresentative(FindCenter(Cluster0), FindCenter(Cluster1));
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::PerformMST
//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSeqMSTSplitter<ObjectType, EvaluatorType>::Distribute(
            stSeqIndexNode * node0, ObjectType * & rep0,
            stSeqIndexNode * node1, ObjectType * & rep1,
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
   node0->GetIndexEntry(idx).PageID = Node->GetPageID(objIdx);

   idx = node1->AddEntry(Node->GetRepresentative(1)->GetSerializedSize(),
                         Node->GetRepresentative(1)->Serialize());
   objIdx = Node->GetRepresentativeIndex(1);
   node1->GetIndexEntry(idx).Distance = 0.0;
   node1->GetIndexEntry(idx).Radius = Node->GetRadius(objIdx);
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
               node0->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }else{
               // Oops! We must put it in other node
               idx = node1->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                     Node->GetObject(i)->Serialize());
               node1->GetIndexEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
               node1->GetIndexEntry(idx).Radius = Node->GetRadius(i);
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
               node1->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }else{
               // Oops! We must put it in other node
               idx = node0->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                     Node->GetObject(i)->Serialize());
               node0->GetIndexEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(0)];
               node0->GetIndexEntry(idx).Radius = Node->GetRadius(i);
               node0->GetIndexEntry(idx).PageID = Node->GetPageID(i);
            }//end if
         }//end if
      }//end if
   }//end for

   // Representatives
   rep0 = Node->BuyObject(Node->GetRepresentativeIndex(0));
   rep1 = Node->BuyObject(Node->GetRepresentativeIndex(1));

   return dCount;
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::Distribute

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stSeqMSTSplitter<ObjectType, EvaluatorType>::Distribute(
            stSeqLeafNode * node0, ObjectType * & rep0,
            stSeqLeafNode * node1, ObjectType * & rep1,
            EvaluatorType * metricEvaluator){
   int dCount;
   int idx;
   int i;

   // Build Distance matrix
   dCount = BuildDistanceMatrix(metricEvaluator);

   //Perform MST
   PerformMST();

   // Add representatives first
   idx = node0->AddEntry(Node->GetRepresentative(0)->GetSerializedSize(),
                         Node->GetRepresentative(0)->Serialize());
   node0->GetLeafEntry(idx).Distance = 0.0;
   idx = node1->AddEntry(Node->GetRepresentative(1)->GetSerializedSize(),
                         Node->GetRepresentative(1)->Serialize());
   node1->GetLeafEntry(idx).Distance = 0.0;

   // Distribute us...
   for (i = 0; i < N; i++){
      if (!Node->IsRepresentative(i)){
         if (ObjectCluster[i] == Cluster0){
            idx = node0->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                  Node->GetObject(i)->Serialize());
            if (idx >= 0){
               // Insertion Ok!
               node0->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(0)];
            }else{
               // Oops! We must put it in other node
               idx = node1->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                     Node->GetObject(i)->Serialize());
               node1->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
            }//end if
         }else{
            idx = node1->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                  Node->GetObject(i)->Serialize());
            if (idx >= 0){
               // Insertion Ok!
               node1->GetLeafEntry(idx).Distance =
                     DMat[i][Node->GetRepresentativeIndex(1)];
            }else{
               // Oops! We must put it in other node
               idx = node0->AddEntry(Node->GetObject(i)->GetSerializedSize(),
                                     Node->GetObject(i)->Serialize());
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
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::Distribute

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqMSTSplitter<ObjectType, EvaluatorType>::JoinClusters(
      int cluster1, int cluster2){
   int i;

   for (i = 0; i < N; i++){
      if (ObjectCluster[i] == cluster2){
         ObjectCluster[i] = cluster1;
      }//end if
   }//end for
   Cluster[cluster1].Size += Cluster[cluster2].Size;
   Cluster[cluster2].State = DEATH_SENTENCE;
}//end stSeqMSTSplitter<ObjectType, EvaluatorType>::JoinClusters


//==============================================================================
// Class stSeqTree
//------------------------------------------------------------------------------

// This macro will be used to replace the declaration of
//       stSeqTree<ObjectType, EvaluatorType>
#define tmpl_stSeqTree stSeqTree<ObjectType, EvaluatorType>

template <class ObjectType, class EvaluatorType>
tmpl_stSeqTree::stSeqTree(stPageManager * pageman):
   stMetricTree<ObjectType, EvaluatorType>(pageman){

   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   // Load header.
   LoadHeader();

   // Will I create or load the tree ?
   if (this->myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if
}//end stSeqTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
tmpl_stSeqTree::stSeqTree(stPageManager * pageman, EvaluatorType * metricEval):
   stMetricTree<ObjectType, EvaluatorType>(pageman, metricEval){

   // Initialize fields
   Header = NULL;
   HeaderPage = NULL;

   // Load header.
   LoadHeader();

   // Will I create or load the tree ?
   if (this->myPageManager->IsEmpty()){
      DefaultHeader();
   }//end if
}//end stSeqTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
tmpl_stSeqTree::~stSeqTree(){

   // Flus header page.
   FlushHeader();
}//end ~stSeqTree()

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::DefaultHeader(){

   // Clear header page.
   HeaderPage->Clear();

   // Default values
   Header->Magic[0] = 'S';
   Header->Magic[1] = 'E';
   Header->Magic[2] = 'Q';
   Header->Magic[3] = '-';
   Header->SplitMethod = smSPANNINGTREE;
   Header->ChooseMethod = cmMINDIST;
   Header->CorrectMethod = crmOFF;
   Header->Root = 0;
   Header->FirstIndexNode = 0;
   Header->FirstLeafNode = 0;
   Header->MinOccupation = 0.25;
   Header->MaxOccupation = 0;
   Header->Height = 0;
   Header->ObjectCount = 0;
   Header->NodeCount = 0;

   // Notify modifications
   HeaderUpdate = true;
}//end DefaultHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::LoadHeader(){

   if (HeaderPage != NULL){
      this->myPageManager->ReleasePage(HeaderPage);
   }//end if

   // Load and set the header.
   HeaderPage = this->myPageManager->GetHeaderPage();
   if (HeaderPage->GetPageSize() < sizeof(stSeqHeader)){
      #ifdef __stDEBUG__
         cout << "The page size is too small. Increase it!\n";
      #endif //__stDEBUG__
      throw std::logic_error("The page size is too small.");
   }//end if

   Header = (stSeqHeader *) HeaderPage->GetData();
   HeaderUpdate = false;
}//end LoadHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::FlushHeader(){

   if (HeaderPage != NULL){
      if (Header != NULL){
         WriteHeader();
      }//end if
      this->myPageManager->ReleasePage(HeaderPage);
   }//end if
}//end FlushHeader

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSeqTree::Add(ObjectType *newObj){
   stSubtreeInfo promo1;
   stSubtreeInfo promo2;
   int insertIdx;

   // Is there a root ?
   if (this->GetRoot() == 0){
      // No! We shall create the new node.
      stPage * auxPage  = NewPage();
      stSeqLeafNode * leafNode = new stSeqLeafNode(auxPage, true);
      this->SetRoot(auxPage->GetPageID());
      this->SetFirstLeafNode(auxPage->GetPageID());

      // Insert the new object.
      insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),
                                     newObj->Serialize());
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
         // Link this node.
         leafNode->SetNextNode(0);
         leafNode->SetPrevNode(0);
         // Update the Height
         Header->Height++;
         // Write the root node.
         this->myPageManager->WritePage(auxPage);
         delete leafNode;
      }//end if
   }else{
      // Let's continue our search for the grail!
      if (InsertRecursive(GetRoot(), newObj, NULL, promo1, promo2) == PROMOTION){
         // Split occurred! We must create a new root because it is required.
         // The tree will aacquire a new root.
         AddNewRoot(promo1.Rep, promo1.Radius, promo1.RootID,
                    promo2.Rep, promo2.Radius, promo2.RootID);
         delete promo1.Rep;
         delete promo2.Rep;
      }//end if
   }//end if

   // Update object count.
   UpdateObjectCounter(1);

   // Write Header!
   WriteHeader();
   // Ok. The new object was inserted. Return success!
   return true;
}//end Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stSeqTree::ChooseSubTree(
      stSeqIndexNode * seqIndexNode, ObjectType * obj) {
   int idx;
   int j;
   int * cover;
   bool stop;
   u_int32_t tmpNumberOfEntries;
   int numberOfEntries, minIndex = 0;

   ObjectType * objectType = new ObjectType;
   double distance;
   double minDistance = MAXDOUBLE; // Largest magnitude double value
   // Get the total number of entries.
   numberOfEntries = seqIndexNode->GetNumberOfEntries();
   idx = 0;

   switch (this->GetChooseMethod()){
      case stSeqTree::cmBIASED :
         // Find the first subtree that covers the new object.
         stop = (idx >= numberOfEntries);
         while (!stop){
            // Get the object from idx position from IndexNode
            objectType->Unserialize(seqIndexNode->GetObject(idx),
                                    seqIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            // is this a subtree that covers the new object?
            if (distance < seqIndexNode->GetIndexEntry(idx).Radius) {
               minDistance = 0;     // the gain will be 0
               stop = true;         // stop the search.
               minIndex = idx;
            }else if (distance - seqIndexNode->GetIndexEntry(idx).Radius < minDistance) {
               minDistance = distance - seqIndexNode->GetIndexEntry(idx).Radius;
               minIndex = idx;
            }//end if
            idx++;
            // if one of the these condicions are true, stop this while.
            stop = stop || (idx >= numberOfEntries);
         }//end while
         break; //end stSeqTree::cmBIASED

      case stSeqTree::cmMINDIST :
         /* Find if there is some circle that contains obj */
         stop = (idx >= numberOfEntries);
         while (!stop){
            //get out the object from IndexNode
            objectType->Unserialize(seqIndexNode->GetObject(idx),
                                    seqIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            // find the first subtree that cover the new object.
            if (distance < seqIndexNode->GetIndexEntry(idx).Radius) {
               minDistance = distance;     // the gain will be 0
               stop = true;                // stop the search.
               minIndex = idx;
            }else if (distance - seqIndexNode->GetIndexEntry(idx).Radius < minDistance) {
               minDistance = distance - seqIndexNode->GetIndexEntry(idx).Radius;
               minIndex = idx;
            }//end if
            idx++;
            // if one of the these condicions are true, stop this while.
            stop = stop || (idx >= numberOfEntries);
         }//end while
         // Try to find a better entry.
         while (idx < numberOfEntries) {
            // Get out the object from IndexNode.
            objectType->Unserialize(seqIndexNode->GetObject(idx),
                                    seqIndexNode->GetObjectSize(idx));
            // Calculate the distance.                                    
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            if ((distance < seqIndexNode->GetIndexEntry(idx).Radius) && (distance < minDistance)) {
               minDistance = distance;
               minIndex = idx;
            }//end if
            idx++;
         }//end while
         break; // end stSeqTree::cmMINDIST

      case stSeqTree::cmMINGDIST :
         // Find if there is some circle that contains obj
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Get out the object from IndexNode.
            objectType->Unserialize(seqIndexNode->GetObject(idx),
                                    seqIndexNode->GetObjectSize(idx));
            // Calculate the distance.
            distance = this->myMetricEvaluator->GetDistance(objectType, obj);
            if (distance < minDistance) {
               minDistance = distance;
               minIndex = idx;
            }//end if
         }//end for
         break; //end stSeqTree::cmMINGDIST

   }//end switch

   delete objectType;

   return minIndex;
}//end ChooseSubTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::AddNewRoot(
      ObjectType * obj1, double radius1, u_int32_t nodeID1,
      ObjectType * obj2, double radius2, u_int32_t nodeID2){
   stPage * newPage;
   stSeqIndexNode * newRoot;
   int idx;

   // Debug mode!
   #ifdef __stDEBUG__
      if ((obj1 == NULL) || (obj2 == NULL)){
         throw std::logic_error("Invalid object.");
      }//end if
   #endif //__stDEBUG__

   // Create a new node
   newPage = NewPage();
   newRoot = new stSeqIndexNode(newPage, true);

   // Add obj1
   idx = newRoot->AddEntry(obj1->GetSerializedSize(), obj1->Serialize());
   newRoot->GetIndexEntry(idx).Distance = 0.0;
   newRoot->GetIndexEntry(idx).PageID = nodeID1;
   newRoot->GetIndexEntry(idx).Radius = radius1;

   // Add obj2
   idx = newRoot->AddEntry(obj2->GetSerializedSize(), obj2->Serialize());
   newRoot->GetIndexEntry(idx).Distance = 0.0;
   newRoot->GetIndexEntry(idx).PageID = nodeID2;
   newRoot->GetIndexEntry(idx).Radius = radius2;

   // Set the pointers.
   if (!this->GetFirstIndexNode()){
      this->SetFirstIndexNode(newRoot->GetPageID());
   }//end if
   // Set the links.
   newRoot->SetNextNode(0);
   newRoot->SetPrevNode(0);

   // Update tree
   Header->Height++;
   SetRoot(newRoot->GetPage()->GetPageID());
   this->myPageManager->WritePage(newPage);

   // Dispose page
   delete newRoot;
   this->myPageManager->ReleasePage(newPage);
}//end SeqTree::AddNewRoot

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int tmpl_stSeqTree::InsertRecursive(
      u_int32_t currNodeID, ObjectType * newObj, ObjectType * repObj,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2){
   stPage * currPage;      // Current page
   stPage * newPage;       // New page
   stSeqNode * currNode;  // Current node
   stSeqIndexNode * indexNode; // Current index node.
   stSeqIndexNode * newIndexNode; // New index node for splits
   stSeqLeafNode * leafNode; // Current leaf node.
   stSeqLeafNode * newLeafNode; // New leaf node.
   int insertIdx;          // Insert index.
   int result;             // Returning value.
   double dist;        // Temporary distance.
   int subtree;            // Subtree
   ObjectType * subRep;    // Subtree representative.

   // Read node...
   currPage = this->myPageManager->GetPage(currNodeID);
   currNode = stSeqNode::CreateNode(currPage);

   // What shall I do ?
   if (currNode->GetNodeType() == stSeqNode::INDEX){
      // Index Node cast.
      indexNode = (stSeqIndexNode *)currNode;

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
            indexNode->GetIndexEntry(subtree).Radius = promo1.Radius;

            // Returning status.
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
                           this->myMetricEvaluator->GetDistance(repObj, 
                                                                promo1.Rep);
                  }else{
                     // It is the root!
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;
                  }//end if

                  // Cut it here
                  delete promo1.Rep; // promo1.rep will never be used again.
                  promo1.Rep = NULL;
                  result = NO_ACT;
               }//end if
               promo1.Radius = indexNode->GetMinimumRadius();
            }else{
               // Split it!
               // New node.
               newPage = NewPage();
               newIndexNode = new stSeqIndexNode(newPage, true);

               // Split!
               SplitIndex(indexNode, newIndexNode,
                     promo1.Rep, promo1.Radius, promo1.RootID,
                     NULL, 0, 0, 
                     repObj, promo1, promo2);

               // Write nodes
               this->myPageManager->WritePage(newPage);
               // Clean home.
               delete newIndexNode;
               this->myPageManager->ReleasePage(newPage);
               result = PROMOTION; //Report split.
            }//end if
            break;
         case PROMOTION: // Promotion!!!
            if (promo1.Rep == NULL){
               // Update subtree
               indexNode->GetIndexEntry(subtree).Radius = promo1.Radius;
               indexNode->GetIndexEntry(subtree).PageID = promo1.RootID;

               // Try to insert the promo2.Rep
               insertIdx = indexNode->AddEntry(promo2.Rep->GetSerializedSize(),
                                               promo2.Rep->Serialize());
               if (insertIdx >= 0){
                  // Swap OK. Fill data.
                  indexNode->GetIndexEntry(insertIdx).Radius = promo2.Radius;
                  indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;
                  // Update promo2 distance
                  if (repObj != NULL){
                     // Distance from representative is...
                     indexNode->GetIndexEntry(insertIdx).Distance =
                           this->myMetricEvaluator->GetDistance(repObj, 
                                                                promo2.Rep);
                  }else{
                     // It is the root!
                     indexNode->GetIndexEntry(insertIdx).Distance = 0;
                  }//end if

                  //Update radius...
                  promo1.Radius = indexNode->GetMinimumRadius();
                  delete promo2.Rep;
                  result = NO_ACT;
               }else{
                  // Split it!
                  // New node.
                  newPage = NewPage();
                  newIndexNode = new stSeqIndexNode(newPage, true);

                  // Split!
                  SplitIndex(indexNode, newIndexNode,
                        promo2.Rep, promo2.Radius, promo2.RootID, 
                        NULL, 0, 0,
                        repObj, promo1, promo2);

                  // Write nodes
                  this->myPageManager->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
                  this->myPageManager->ReleasePage(newPage);
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
                              this->myMetricEvaluator->GetDistance(repObj,
                                                                   promo1.Rep);
                     }else{
                        // It is the root!
                        indexNode->GetIndexEntry(insertIdx).Distance = 0;
                     }//end if

                     // Cut it here
                     delete promo1.Rep; // promo1.rep will never be used again.
                     promo1.Rep = NULL;
                     result = NO_ACT;
                  }//end if

                  // Try to add promo2
                  insertIdx = indexNode->AddEntry(promo2.Rep->GetSerializedSize(),
                                                  promo2.Rep->Serialize());
                  if (insertIdx >= 0){
                     // Swap OK. Fill data.
                     indexNode->GetIndexEntry(insertIdx).Radius = promo2.Radius;
                     indexNode->GetIndexEntry(insertIdx).PageID = promo2.RootID;

                     // The new distance is...
                     if (promo1.Rep != NULL){
                        // Rep. changed...
                        // Distance from representative is...
                        indexNode->GetIndexEntry(insertIdx).Distance =
                              this->myMetricEvaluator->GetDistance(promo1.Rep,
                                                                   promo2.Rep);
                     }else{
                        // No change!
                        if (repObj != NULL){
                           // Distance from representative is...
                           indexNode->GetIndexEntry(insertIdx).Distance =
                                 this->myMetricEvaluator->GetDistance(repObj,
                                                                      promo2.Rep);
                        }else{
                           // It is the root!
                           indexNode->GetIndexEntry(insertIdx).Distance = 0;
                        }//end if
                     }//end if

                     delete promo2.Rep;
                     // set the radius to high levels.
                     promo1.Radius = indexNode->GetMinimumRadius();
                  }else{
                     // Split it promo2.rep does not fit.
                     // New node.
                     newPage = NewPage();
                     newIndexNode = new stSeqIndexNode(newPage, true);

                     // Dispose promo1.rep it if exists because it will not be
                     // used again. It happens when result is CHANGE_REP.
                     if (promo1.Rep != NULL){
                        delete promo1.Rep;
                     }//end if

                     // Add promo2 and split!
                     SplitIndex(indexNode, newIndexNode,
                           promo2.Rep, promo2.Radius, promo2.RootID,
                           NULL, 0, 0, // Ignore this object
                           repObj, promo1, promo2);

                     // Write nodes
                     this->myPageManager->WritePage(newPage);
                     // Clean home.
                     delete newIndexNode;
                     this->myPageManager->ReleasePage(newPage);
                     result = PROMOTION; //Report split.
                  }//end if
               }else{
                  // Split it because both objects don't fit.
                  // New node.
                  newPage = NewPage();
                  newIndexNode = new stSeqIndexNode(newPage, true);

                  // Split!
                  SplitIndex(indexNode, newIndexNode,
                        promo1.Rep, promo1.Radius, promo1.RootID,
                        promo2.Rep, promo2.Radius, promo2.RootID, 
                        repObj, promo1, promo2);

                  // Write nodes
                  this->myPageManager->WritePage(newPage);
                  // Clean home.
                  delete newIndexNode;
                  this->myPageManager->ReleasePage(newPage);
                  result = PROMOTION; //Report split.
               }//end if
            }//end if
      };//end switch

      // Clear the mess.
      delete subRep;
   }else{
      // Leaf node cast.
      leafNode = (stSeqLeafNode *) currNode;

      // Try to insert...
      insertIdx = leafNode->AddEntry(newObj->GetSerializedSize(),
                                     newObj->Serialize());
      if (insertIdx >= 0){
         // Don't split!
         // Calculate distance and verify if it is a new radius!
         if (repObj == NULL){
            dist = 0;
         }else{
            dist = this->myMetricEvaluator->GetDistance(newObj, repObj);
         }//end if

         // Fill entry's fields
         leafNode->GetLeafEntry(insertIdx).Distance = dist;

         // Write node.
         this->myPageManager->WritePage(currPage);

         // Returning values
         promo1.Rep = NULL;
         promo1.Radius = leafNode->GetMinimumRadius();
         promo1.RootID = currNodeID;
         result = NO_ACT;
      }else{
         // Split it!
         // New node.
         newPage = NewPage();
         newLeafNode = new stSeqLeafNode(newPage, true);

         // Split!
         SplitLeaf(leafNode, newLeafNode, (ObjectType *)newObj->Clone(),
                   repObj, promo1, promo2);

         // Write node.
         this->myPageManager->WritePage(newPage);
         // Clean home.
         delete newLeafNode;
         this->myPageManager->ReleasePage(newPage);
         result = PROMOTION; //Report split.
      }//end if
   }//end if

   // Write node.
   this->myPageManager->WritePage(currPage);
   // Clean home
   delete currNode;
   this->myPageManager->ReleasePage(currPage);
   return result;
}//end InsertRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::MinMaxPromote(tLogicNode * node) {

   double iRadius, jRadius, min;
   u_int32_t numberOfEntries, idx1, idx2, i, j;
   stPage * newPage1 = new stPage(this->myPageManager->GetMinimumPageSize());
   stPage * newPage2 = new stPage(this->myPageManager->GetMinimumPageSize());

   numberOfEntries = node->GetNumberOfEntries();
   min = MAXDOUBLE;   // Largest magnitude double value

   // Is it an Index node?
   if (node->GetNodeType() == stSeqNode::INDEX) {
      stSeqIndexNode * indexNode1 = new stSeqIndexNode(newPage1, true);
      stSeqIndexNode * indexNode2 = new stSeqIndexNode(newPage2, true);

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
      delete indexNode2;
   }else{//it is a Leaf node
      stSeqLeafNode * leafNode1 = new stSeqLeafNode(newPage1, true);
      stSeqLeafNode * leafNode2 = new stSeqLeafNode(newPage2, true);

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
      delete leafNode2;
   }//end else

   // Choose representatives
   node->SetRepresentative(idx1, idx2);

   delete newPage1;
   delete newPage2;
}//end MinMaxPromote

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::SplitLeaf(
      stSeqLeafNode * oldNode, stSeqLeafNode * newNode,
      ObjectType * newObj, ObjectType * prevRep,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2) {
   tLogicNode * logicNode;
   tMSTSplitter * mstSplitter;
   ObjectType * lRep;
   ObjectType * rRep;
   u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();
   // Store the old links. This is done because the CleanAll() in node
   // erase the link in the debug mode.
   u_int32_t nextNodePageID = oldNode->GetNextNode();

   // Create the new tLogicNode
   logicNode = new tLogicNode(numberOfEntries + 1);
   logicNode->SetMinOccupation((u_int32_t )(GetMinOccupation() * (numberOfEntries + 1)));
   logicNode->SetNodeType(stSeqNode::LEAF);

   // update the maximum number of entries.
   this->SetMaxOccupation(numberOfEntries);

   // Add objects
   logicNode->AddLeafNode(oldNode);
   logicNode->AddEntry(newObj);

   // Split it.
   switch (GetSplitMethod()) {
      case stSeqTree::smMINMAX:
         this->MinMaxPromote(logicNode);
         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
         break;  //end stSeqTree::smMINMAX
      case stSeqTree::smSPANNINGTREE:
         // MST Split
         mstSplitter = new tMSTSplitter(logicNode);
         // Perform MST
         oldNode->RemoveAll();
         mstSplitter->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         // Clean home
         delete mstSplitter;
         break; //end stSeqTree::smSPANNINGTREE
      #ifdef __stDEBUG__
      default:
         throw logic_error("There is no Split method selected.");
      #endif //__stDEBUG__
   };//end switch

   // Link the nodes.
   newNode->SetNextNode(nextNodePageID);
   newNode->SetPrevNode(oldNode->GetPageID());
   oldNode->SetNextNode(newNode->GetPageID());

   // Test if the new node points to a valid node.
   if (newNode->GetNextNode()){
      // Read the next node.
      stPage * nextPage;       // Next page
      stSeqNode * nextNode; // Next leaf node.

      // Read node...
      nextPage = this->myPageManager->GetPage(newNode->GetNextNode());
      nextNode = stSeqNode::CreateNode(nextPage);

      // Link it.
      nextNode->SetPrevNode(newNode->GetPageID());

      // Write node.
      this->myPageManager->WritePage(nextPage);
      // Clean home.
      delete nextNode;
      this->myPageManager->ReleasePage(nextPage);
   }//end if

   // Update fields. We may need to change lRep and rRep.
   if (prevRep == NULL){
      // This is a root. The order of lRep and rRep is not important.
      promo1.Rep = lRep;
      promo1.Radius = oldNode->GetMinimumRadius();
      promo1.RootID = oldNode->GetPageID();
      promo2.Rep = rRep;
      promo2.Radius = newNode->GetMinimumRadius();
      promo2.RootID = newNode->GetPageID();
   }else{
      // Let's see if it is necessary to change things.
      if (prevRep->IsEqual(lRep)){
         // lRep is the prevRep. Delete it.
         delete lRep;
         promo1.Rep = NULL;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }else if (prevRep->IsEqual(rRep)){
         // rRep is the prevRep. Delete it.
         delete rRep;
         promo2.Rep = lRep;
         promo2.Radius = oldNode->GetMinimumRadius();
         promo2.RootID = oldNode->GetPageID();
         promo1.Rep = NULL;
         promo1.Radius = newNode->GetMinimumRadius();
         promo1.RootID = newNode->GetPageID();
      }else{
         // This is a root. The order of lRep and rRep is not important.
         promo1.Rep = lRep;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }//end if
   }//end if
}//end SplitLeaf

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::SplitIndex(
      stSeqIndexNode * oldNode, stSeqIndexNode * newNode,
      ObjectType * newObj1, double newRadius1,
      u_int32_t newNodeID1,
      ObjectType * newObj2, double newRadius2,
      u_int32_t newNodeID2, 
      ObjectType * prevRep,
      stSubtreeInfo & promo1, stSubtreeInfo & promo2){
   tLogicNode * logicNode;
   tMSTSplitter * mstSplitter;
   ObjectType * lRep;
   ObjectType * rRep;
   u_int32_t numberOfEntries = oldNode->GetNumberOfEntries();
   // Store the old links. This is done because the CleanAll() in node
   // erase the link in the debug mode.
   u_int32_t nextNodePageID = oldNode->GetNextNode();

   // Create the new tLogicNode
   logicNode = new tLogicNode(numberOfEntries + 2);
   logicNode->SetMinOccupation((u_int32_t )(GetMinOccupation() * (numberOfEntries + 2)));
   logicNode->SetNodeType(stSeqNode::INDEX);

   // update the maximum number of entries.
   this->SetMaxOccupation(numberOfEntries);

   // Add objects
   logicNode->AddIndexNode(oldNode);

   // Add newObj1
   logicNode->AddEntry(newObj1);
   logicNode->SetEntry(logicNode->GetNumberOfEntries() - 1,
         newNodeID1, newRadius1);

   // Will I add newObj2 ?
   if (newObj2 != NULL){
      logicNode->AddEntry(newObj2);
      logicNode->SetEntry(logicNode->GetNumberOfEntries() - 1,
            newNodeID2, newRadius2);
   }//end if

   // Split it.
   switch (GetSplitMethod()) {
      case stSeqTree::smMINMAX:
         this->MinMaxPromote(logicNode);
         // Redistribute
         oldNode->RemoveAll();
         logicNode->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         delete logicNode;
         break;  //end stSeqTree::smMINMAX
      case stSeqTree::smSPANNINGTREE:
         // MST Split
         mstSplitter = new tMSTSplitter(logicNode);
         // Perform MST
         oldNode->RemoveAll();
         mstSplitter->Distribute(oldNode, lRep, newNode, rRep, this->myMetricEvaluator);
         // Clean home
         delete mstSplitter;
         break; //end stSeqTree::smSPANNINGTREE
      #ifdef __stDEBUG__
      default:
         throw logic_error("There is no Split method selected.");
      #endif //__stDEBUG__
   };//end switch

   // Link the nodes.
   newNode->SetNextNode(nextNodePageID);
   newNode->SetPrevNode(oldNode->GetPageID());
   oldNode->SetNextNode(newNode->GetPageID());

   // Test if the new node points to a valid node.
   if (newNode->GetNextNode()){
      // Read the next node.
      stPage * nextPage;       // Next page
      stSeqNode * nextNode; // Next leaf node.

      // Read node...
      nextPage = this->myPageManager->GetPage(newNode->GetNextNode());
      nextNode = stSeqNode::CreateNode(nextPage);
      // Link it.
      nextNode->SetPrevNode(newNode->GetPageID());

      // Write node.
      this->myPageManager->WritePage(nextPage);
      // Clean home.
      delete nextNode;
      this->myPageManager->ReleasePage(nextPage);
   }//end if

   // Update fields. We may need to change lRep and rRep.
   if (prevRep == NULL){
      // This is a root. The order of lRep and rRep is not important.
      promo1.Rep = lRep;
      promo1.Radius = oldNode->GetMinimumRadius();
      promo1.RootID = oldNode->GetPageID();
      promo2.Rep = rRep;
      promo2.Radius = newNode->GetMinimumRadius();
      promo2.RootID = newNode->GetPageID();
   }else{
      // Let's see if it is necessary to change things.
      if (prevRep->IsEqual(lRep)){
         // lRep is the prevRep. Delete it.
         delete lRep;
         promo1.Rep = NULL;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }else if (prevRep->IsEqual(rRep)){
         // rRep is the prevRep. Delete it.
         delete rRep;
         promo2.Rep = lRep;
         promo2.Radius = oldNode->GetMinimumRadius();
         promo2.RootID = oldNode->GetPageID();
         promo1.Rep = NULL;
         promo1.Radius = newNode->GetMinimumRadius();
         promo1.RootID = newNode->GetPageID();
      }else{
         // This is a root. The order of lRep and rRep is not important.
         promo1.Rep = lRep;
         promo1.Radius = oldNode->GetMinimumRadius();
         promo1.RootID = oldNode->GetPageID();
         promo2.Rep = rRep;
         promo2.Radius = newNode->GetMinimumRadius();
         promo2.RootID = newNode->GetPageID();
      }//end if
   }//end if
}//end SplitIndex

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::UpdateDistances(stSeqIndexNode * node,
            ObjectType * repObj, u_int32_t repObjIdx){
   u_int32_t i;
   ObjectType * tempObj = new ObjectType();

   for (i = 0; i < node->GetNumberOfEntries(); i++){
      if (i != repObjIdx){
         tempObj->Unserialize(node->GetObject(i), node->GetObjectSize(i));
         node->GetIndexEntry(i).Distance =
            this->myMetricEvaluator->GetDistance(repObj, tempObj);
      }else{
         //it's the representative object
         node->GetIndexEntry(i).Distance = 0.0;
      }//end if
   }//end for

   //clean the house before exit.
   delete tempObj;
}//end UpdateDistances

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSeqTree::GetDistanceLimit(){
   double distance = 0;
   double distanceTemp = 0;
   u_int32_t i, j;
   ObjectType * object1 = new ObjectType();
   ObjectType * object2 = new ObjectType();
   stPage * currPage;
   stSeqNode * currNode;
   stSeqIndexNode * indexNode;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes.
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSeqNode::CreateNode(currPage);

      // Index Node cast.
      indexNode = (stSeqIndexNode *)currNode;

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
            distanceTemp = this->myMetricEvaluator->GetDistance(object1, object2);
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
   delete object2;

   //return the maximum distance between 2 objects of this tree
   return distance;
}//end GetDistanceLimit

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSeqTree::GetGreaterEstimatedDistance(){
   double distance = 0;
   double distanceTemp = 0;
   u_int32_t idx, idx2;
   ObjectType ** objects;
   u_int32_t size = 0;
   stPage * currPage, * currPage2;
   stSeqNode * currNode, * currNode2;
   stSeqIndexNode * indexNode, * indexNode2;
   stSeqLeafNode * leafNode;

   // Is there a root ?
   if (this->GetRoot()){
      // Yes. Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSeqNode::CreateNode(currPage);

      // Is it an index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         indexNode = (stSeqIndexNode *)currNode;
         // Estimate the maximum number of entries.
         objects = new ObjectType * [this->GetMaxOccupation() * indexNode->GetNumberOfEntries()];
         // For each entry...
         for (idx = 0; idx < indexNode->GetNumberOfEntries(); idx++) {
            // Get the pages.
            currPage2 = this->myPageManager->GetPage(indexNode->GetIndexEntry(idx).PageID);
            currNode2 = stSeqNode::CreateNode(currPage2);
            // Is it am index node?
            if (currNode2->GetNodeType() == stSeqNode::INDEX) {
               // Get Index node
               indexNode2 = (stSeqIndexNode *)currNode2;

               // For each entry...
               for (idx2 = 0; idx2 < indexNode2->GetNumberOfEntries(); idx2++) {
                  // Rebuild the object
                  objects[size] = new ObjectType();
                  objects[size++]->Unserialize(indexNode2->GetObject(idx2),
                                               indexNode2->GetObjectSize(idx2));
               }//end for
            }else{
               // No, it is a leaf node. Get it.
               leafNode = (stSeqLeafNode *)currNode2;
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
            this->myPageManager->ReleasePage(currPage2);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         leafNode = (stSeqLeafNode *)currNode;
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
      }//end for
      delete[] objects;
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   //return the maximum distance between 2 objects of this tree
   return distance;
}//end GetGreaterEstimatedDistance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSeqTree::GetGreaterDistance(){
   double greaterDistance = 0;
   double distanceTemp = 0;
   u_int32_t idx, idx2;
   ObjectType ** objects;
   u_int32_t size = 0;

   // Is there a root ?
   if (this->GetRoot()){
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
      }//end for
      delete[] objects;
   }//end if

   //return the maximum distance between 2 objects of this tree
   return greaterDistance;
}//end GetGreaterDistance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::GetGreaterDistance(u_int32_t pageID, ObjectType ** objects,
                                         u_int32_t & size){
   stPage * currPage;
   stSeqNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Analyze it!
            this->GetGreaterDistance(indexNode->GetIndexEntry(idx).PageID, objects, size);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
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
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end GetGreaterDistance

//==============================================================================
// Begin of Queries
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSeqTree::RangeQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSeqNode * currNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries;
   double distance;

   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   // Evaluate the root node.
   if (this->GetRoot() != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(this->GetRoot());
      currNode = stSeqNode::CreateNode(currPage);

      // Is it an Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX){
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(idx),
                               indexNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            // test if this subtree qualifies.
            if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
               // Yes! Analyze this subtree.
               this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                sample, range, distance);
            }//end if
         }//end for
         
      }else{
         // No, it is a leaf node. Get it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
            // is it a object that qualified?
            if (distance <= range){
               // Yes! Put it in the result set.
               result->AddPair(tmpObj.Clone(), distance);
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::RangeQuery(
         u_int32_t pageID, tResult * result, ObjectType * sample,
         double range, double distanceRepres){
   stPage * currPage;
   stSeqNode * currNode;
   ObjectType tmpObj;
   double distance;
   u_int32_t idx;
   u_int32_t numberOfEntries;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // For each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality to cut a subtree
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      range + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // is this a qualified subtree?
               if (distance <= range + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! Analyze it!
                  this->RangeQuery(indexNode->GetIndexEntry(idx).PageID, result,
                                    sample, range, distance);
               }//end if
            }//end if
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // use of the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      range){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // No, it is not a representative. Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               // Is this a qualified object?
               if (distance <= range){
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSeqTree::SeqLeafRangeQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSeqNode * currNode;
   stSeqLeafNode * leafNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries, idxRep;
   double distance, distRep;
   u_int32_t nextNode;

   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   // First node
   nextNode = this->GetFirstLeafNode();

   // Let's search
   while (nextNode != 0){
      currPage = this->myPageManager->GetPage(nextNode);
      currNode = stSeqNode::CreateNode(currPage);
      leafNode = (stSeqLeafNode *)currNode;
      numberOfEntries = leafNode->GetNumberOfEntries();

      idxRep = leafNode->GetRepresentativeEntry();
      // Rebuild the object
      tmpObj.Unserialize(leafNode->GetObject(idxRep), leafNode->GetObjectSize(idxRep));
      // Evaluate distance
      distRep = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

      if (distRep <= range + leafNode->GetMinimumRadius()){
         // Lets check all objects in this node
         for (idx = 0; idx < numberOfEntries; idx++){
            // use of the triangle inequality.
            if ( fabs(distRep - leafNode->GetLeafEntry(idx).Distance) <= range){
               // Test if it is the representative.
               if (idx == idxRep){
                  // It is the representative. The distance is the same as distRep
                  distance = distRep;
               }else{
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               }//end if

               // is it a object that qualified?
               if (distance <= range){
                  // Yes! Put it in the result set.
                  result->AddPair(tmpObj.Clone(), distance);
               }//end if
            }//end if
         }//end for
      }//end if

      // Next Node...
      nextNode = leafNode->GetNextNode();

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end SeqLeafRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSeqTree::SeqIndexRangeQuery(
            ObjectType * sample, double range){
   tResult * result = new tResult();  // Create result
   stPage * currPage, * currPage2;
   stSeqNode * currNode, * currNode2;
   stSeqIndexNode * indexNode;
   stSeqLeafNode * leafNode;
   ObjectType tmpObj;
   u_int32_t idx, i, numberOfLeafEntries, numberOfEntries, idxRep, idxRep2;
   double distance, distRep, distRep2;
   u_int32_t nextNode;

   result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

   // First node
   nextNode = this->GetFirstIndexNode();

   // Let's search
   while (nextNode != 0){
      currPage = this->myPageManager->GetPage(nextNode);
      currNode = stSeqNode::CreateNode(currPage);

      indexNode = (stSeqIndexNode *)currNode;
      numberOfLeafEntries = indexNode->GetNumberOfEntries();
      // Get the representative of this index node.
      idxRep = indexNode->GetRepresentativeEntry();
      // Rebuild the object
      tmpObj.Unserialize(indexNode->GetObject(idxRep), indexNode->GetObjectSize(idxRep));
      // Evaluate distance
      distRep = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
      
      for (i = 0; i < numberOfLeafEntries; i++){
         // test if this subtree qualifies.
         if ( fabs(distRep - indexNode->GetIndexEntry(i).Distance) <=
                   range + indexNode->GetIndexEntry(i).Radius){

            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(i), indexNode->GetObjectSize(i));
            // Evaluate distance
            distRep2 = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distRep2 <= range + indexNode->GetIndexEntry(i).Radius){
               // Get the leaf node.
               currPage2 = this->myPageManager->GetPage(indexNode->GetIndexEntry(i).PageID);
               currNode2 = stSeqNode::CreateNode(currPage2);
               leafNode = (stSeqLeafNode *)currNode2;
               numberOfEntries = leafNode->GetNumberOfEntries();
               idxRep2 = leafNode->GetRepresentativeEntry();

               // Lets check all objects in this node
               for (idx = 0; idx < numberOfEntries; idx++){
                  // use of the triangle inequality.
                  if ( fabs(distRep2 - leafNode->GetLeafEntry(idx).Distance) <= range){
                     // Test if it is the representative.
                     if (idx == idxRep2){
                        // It is the representative. The distance is the same as distRep2
                        distance = distRep2;
                     }else{
                        // Rebuild the object
                        tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
                        // Evaluate distance
                        distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
                     }//end if

                     // is it a object that qualified?
                     if (distance <= range){
                        // Yes! Put it in the result set.
                        result->AddPair(tmpObj.Clone(), distance);
                     }//end if
                  }//end if
               }//end for
               // Free it all
               delete currNode2;
               this->myPageManager->ReleasePage(currPage2);
            }//end if
         }//end if
      }//end for

      // Next Node...
      nextNode = indexNode->GetNextNode();

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end SeqIndexRangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSeqTree<ObjectType, EvaluatorType>::NearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Let's search
   if (this->GetRoot() != 0){
      this->NearestQuery(result, sample, MAXDOUBLE, k);
   }//end if

   return result;
}//end NearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqTree<ObjectType, EvaluatorType>::NearestQuery(tResult * result,
         ObjectType * sample, double rangeK, u_int32_t k){
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSeqNode * currNode;
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
   queue = new tDynamicPriorityQueue(STARTVALUEQUEUE, INCREMENTVALUEQUEUE);

   // Let's search
   while (pqCurrValue.PageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      rangeK + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

               if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTmpValue.PageID = indexNode->GetIndexEntry(idx).PageID;
                  pqTmpValue.Radius = indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTmpValue);
               }//end if
            }//end if
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this object with the triangle inequality.
            if ( fabs(distanceRepres - leafNode->GetLeafEntry(idx).Distance) <=
                      rangeK){
               // Rebuild the object
               tmpObj.Unserialize(leafNode->GetObject(idx),
                                  leafNode->GetObjectSize(idx));
               // When this entry is a representative, it does not need to evaluate
               // a distance, because distanceRepres is iqual to distance.
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
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
}//end NearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSeqTree::SeqLeafNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   stPage * currPage;
   stSeqNode * currNode;
   stSeqLeafNode * leafNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries, idxRep;
   double distance, distRep, rangeK;
   u_int32_t nextNode;

   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   rangeK = MAXDOUBLE;
   // First node
   nextNode = this->GetFirstLeafNode();

   // Let's search
   while (nextNode != 0){
      currPage = this->myPageManager->GetPage(nextNode);
      currNode = stSeqNode::CreateNode(currPage);
      leafNode = (stSeqLeafNode *)currNode;
      numberOfEntries = leafNode->GetNumberOfEntries();

      idxRep = leafNode->GetRepresentativeEntry();
      // Rebuild the object
      tmpObj.Unserialize(leafNode->GetObject(idxRep), leafNode->GetObjectSize(idxRep));
      // Evaluate distance
      distRep = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

      if (distRep <= rangeK + leafNode->GetMinimumRadius()){
         // Lets check all objects in this node
         for (idx = 0; idx < numberOfEntries; idx++){
            // use of the triangle inequality.
            if ( fabs(distRep - leafNode->GetLeafEntry(idx).Distance) <= rangeK){
               // Test if it is the representative.
               if (idx == idxRep){
                  // It is the representative. The distance is the same as distRep
                  distance = distRep;
               }else{
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               }//end if

               // is it a object that qualified?
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

      // Next Node...
      nextNode = leafNode->GetNextNode();

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end stSeqTree<ObjectType, EvaluatorType>::SeqLeafNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSeqTree<ObjectType, EvaluatorType>::SeqLeafNearestQuery2(
         ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   u_int32_t idx;
   stPage * currPage;
   stSeqNode * currNode;
   ObjectType tmpObj;
   double distance;
   double distanceRepres = 0;
   double minDist;
   double rangeK;
   u_int32_t numberOfEntries;
   u_int32_t pageID, nextPageID, prevPageID;
   bool stop;

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   // Root node
   pageID = this->GetRoot();
   rangeK = MAXDOUBLE;

   // Let's search
   while (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
         numberOfEntries = indexNode->GetNumberOfEntries();
         // Initialize the variable for this level.
         minDist = MAXDOUBLE;

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // try to cut this subtree with the triangle inequality.
            if ( fabs(distanceRepres - indexNode->GetIndexEntry(idx).Distance) <=
                      rangeK + indexNode->GetIndexEntry(idx).Radius){
               // Rebuild the object
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate distance
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

               if (distance <= rangeK + indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified!
                  // Is it nearest to the query object?
                  if (distance < minDist){
                     // Yes, get it.
                     pageID = indexNode->GetIndexEntry(idx).PageID;
                     minDist = distance;
                  }//end if
               }//end if
            }//end if
         }//end for
      }else{
         // It reached the leaf node.
         // Inspect all object in it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
         numberOfEntries = leafNode->GetNumberOfEntries();
         // Initialize the maximum radius.
         rangeK = MAXDOUBLE;

         // for each entry...
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Rebuild the object
            tmpObj.Unserialize(leafNode->GetObject(idx),
                               leafNode->GetObjectSize(idx));
            // When this entry is a representative, it does not need to evaluate
            // a distance, because distanceRepres is iqual to distance.
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
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
         // Get the links.
         nextPageID = leafNode->GetNextNode();
         prevPageID = leafNode->GetPrevNode();
         // Stop the process.
         pageID = 0;
         // Inspect the rest.
         SeqLeafNearestQuery2(result, sample, k, rangeK, prevPageID, nextPageID);
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }// end while

   // return the result.
   return result;
}//end SeqLeafNearestQuery2

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqTree<ObjectType, EvaluatorType>::SeqLeafNearestQuery2(tResult * result,
         ObjectType * sample, u_int32_t k, double rangeK,
         u_int32_t prevPageID, u_int32_t nextPageID){
   stPage * currPage;
   stSeqNode * currNode;
   stSeqLeafNode * leafNode;
   ObjectType tmpObj;
   u_int32_t idx, numberOfEntries, idxRep;
   double distance, distRep;
   u_int32_t pageID;
   bool next, first;

   rangeK = MAXDOUBLE;
   next = true;
   // Get the first node to be analized.
   pageID = nextPageID;

   // Let's search
   while (pageID != 0){
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);
      leafNode = (stSeqLeafNode *)currNode;
      numberOfEntries = leafNode->GetNumberOfEntries();
      // Get the pageID of the next node.
      // Next Node...
      if (next && nextPageID){
         // Get the next node.
         nextPageID = leafNode->GetNextNode();
      }else if (prevPageID){
         // Get the prev node.
         prevPageID = leafNode->GetPrevNode();
      }//end if

      idxRep = leafNode->GetRepresentativeEntry();
      // Rebuild the object
      tmpObj.Unserialize(leafNode->GetObject(idxRep), leafNode->GetObjectSize(idxRep));
      // Evaluate distance
      distRep = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

      if (distRep <= rangeK + leafNode->GetMinimumRadius()){
         // Lets check all objects in this node
         for (idx = 0; idx < numberOfEntries; idx++){
            // use of the triangle inequality.
            if ( fabs(distRep - leafNode->GetLeafEntry(idx).Distance) <= rangeK){
               // Test if it is the representative.
               if (idx == idxRep){
                  // It is the representative. The distance is the same as distRep
                  distance = distRep;
               }else{
                  // Rebuild the object
                  tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               }//end if

               // is it a object that qualified?
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

      // Get the next pageID.
      if (next && prevPageID){
         pageID = prevPageID;
         // Is it possible to get the previsous node?
         if (nextPageID){
            // Yes, enable it.
            next = !next;
         }else{
            next = true;
         }//end if
      }else if (nextPageID){
         pageID = nextPageID;
         // Is it possible to get the next node?
         if (prevPageID){
            // Yes, enable it.
            next = !next;
         }else{
            next = true;
         }//end if
      }else{
         // There is no other pageID to ananlyze. Stop the process.
         pageID = 0;
      }//end if


      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

}//end SeqLeafNearestQuery2

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * tmpl_stSeqTree::SeqIndexNearestQuery(
      ObjectType * sample, u_int32_t k, bool tie){
   tResult * result = new tResult();  // Create result
   stPage * currPage, * currPage2;
   stSeqNode * currNode, * currNode2;
   stSeqIndexNode * indexNode;
   stSeqLeafNode * leafNode;
   ObjectType tmpObj;
   u_int32_t idx, i, numberOfLeafEntries, numberOfEntries, idxRep, idxRep2;
   double distance, distRep, distRep2, rangeK;
   u_int32_t nextNode;

   result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, MAXDOUBLE, tie);

   rangeK = MAXDOUBLE;

   // First node
   nextNode = this->GetFirstIndexNode();

   // Let's search
   while (nextNode != 0){
      currPage = this->myPageManager->GetPage(nextNode);
      currNode = stSeqNode::CreateNode(currPage);

      indexNode = (stSeqIndexNode *)currNode;
      numberOfLeafEntries = indexNode->GetNumberOfEntries();
      // Get the representative of this index node.
      idxRep = indexNode->GetRepresentativeEntry();
      // Rebuild the object
      tmpObj.Unserialize(indexNode->GetObject(idxRep), indexNode->GetObjectSize(idxRep));
      // Evaluate distance
      distRep = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
      
      for (i = 0; i < numberOfLeafEntries; i++){
         // test if this subtree qualifies.
         if ( fabs(distRep - indexNode->GetIndexEntry(i).Distance) <=
                   rangeK + indexNode->GetIndexEntry(i).Radius){

            // Rebuild the object
            tmpObj.Unserialize(indexNode->GetObject(i), indexNode->GetObjectSize(i));
            // Evaluate distance
            distRep2 = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

            if (distRep2 <= rangeK + indexNode->GetIndexEntry(i).Radius){
               // Get the leaf node.
               currPage2 = this->myPageManager->GetPage(indexNode->GetIndexEntry(i).PageID);
               currNode2 = stSeqNode::CreateNode(currPage2);
               leafNode = (stSeqLeafNode *)currNode2;
               numberOfEntries = leafNode->GetNumberOfEntries();
               idxRep2 = leafNode->GetRepresentativeEntry();

               // Lets check all objects in this node
               for (idx = 0; idx < numberOfEntries; idx++){
                  // use of the triangle inequality.
                  if ( fabs(distRep2 - leafNode->GetLeafEntry(idx).Distance) <= rangeK){
                     // Test if it is the representative.
                     if (idx == idxRep2){
                        // It is the representative. The distance is the same as distRep2
                        distance = distRep2;
                     }else{
                        // Rebuild the object
                        tmpObj.Unserialize(leafNode->GetObject(idx), leafNode->GetObjectSize(idx));
                        // Evaluate distance
                        distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
                     }//end if

                     // is it a object that qualified?
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
               // Free it all
               delete currNode2;
               this->myPageManager->ReleasePage(currPage2);
            }//end if
         }//end if
      }//end for

      // Next Node...
      nextNode = indexNode->GetNextNode();

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return result;
}//end SeqIndexNearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stSeqTree<ObjectType, EvaluatorType>::PointQuery(
      ObjectType * sample){
   tResult * result = new tResult();  // Create result

   // Set information for this query
   result->SetQueryInfo(sample->Clone(), POINTQUERY);
   // Let's search
   if (this->GetRoot() != 0){
      this->PointQuery(result, sample);
   }//end if

   return result;
}//end PointQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stSeqTree<ObjectType, EvaluatorType>::PointQuery(
         tResult * result, ObjectType * sample){
   tDynamicPriorityQueue * queue;
   u_int32_t idx;
   stPage * currPage;
   stSeqNode * currNode;
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
      currPage = this->myPageManager->GetPage(pqCurrValue.PageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it a Index node?        
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
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
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);

               if (distance <= indexNode->GetIndexEntry(idx).Radius){
                  // Yes! I'm qualified! Put it in the queue.
                  pqTMPValue.PageID =  indexNode->GetIndexEntry(idx).PageID;
                  pqTMPValue.Radius =  indexNode->GetIndexEntry(idx).Radius;
                  queue->Add(distance, pqTMPValue);
               }//end if
            }//end if
         }//end for
      }else{ 
         // No, it is a leaf node. Get it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
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
               distance = this->myMetricEvaluator->GetDistance(&tmpObj, sample);
               //test if the object qualify
               if (distance == 0){
                  // Add the object.
                  result->AddPair(tmpObj.Clone(), distance);
                  // Stop the query because the object was found!
                  find = true;
               }//end if
            }//end if
         }//end for
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);

      // Go to next node.
      if (!find){
         // Search... and feed query
         stop = false;
         do{
            if (queue->Get(distance, pqCurrValue)){
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
}//end PointQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stJoinedResult<ObjectType> * tmpl_stSeqTree::NearestJoinQuery(
      stSeqTree * seqTree, u_int32_t k, bool tie){
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
stJoinedResult<ObjectType> * tmpl_stSeqTree::RangeJoinQuery(
      stSeqTree * seqTree, double range){
   // Create result
   tJoinedResult * result = new tJoinedResult();
   u_int32_t joinedPageID;
   u_int32_t nextPageID;
   tmpl_stSeqTree::tObject tmp, subRep, repObj, tmpObj, tmpJoinedObj;
   stPage * currPage, * leafPage, * joinedPage, * joinedPage2;
   stSeqNode * currNode, * joinedNode, * joinedNode2;
   stSeqIndexNode * joinedIndexNode;
   stSeqLeafNode * currLeafNode, * joinedLeafNode;
   u_int32_t numberOfEntries, joinedNumberOfEntries, leafNumberOfEntries;
   double distance, distanceRepres, distanceSubRep;
   u_int32_t i, j, idx;
   bool qualified;
   bool * mapVector;
   double * distSubRepVector;

   // Set the result.
   result->SetQueryInfo(RANGEJOINQUERY, -1, range, false);
   // Get the first index node.
   nextPageID = GetFirstLeafNode();

   // Test if the first tree has a leaf node.
   if (nextPageID != 0){
      // Ok, there is at least one leaf node.
      // Analyze all the leaf nodes.
      while (nextPageID != 0){
         // Get the node and analyze it.
         currPage = this->myPageManager->GetPage(nextPageID);
         currNode = stSeqNode::CreateNode(currPage);
         currLeafNode = (stSeqLeafNode *)currNode;
         // Get the number of entries in the
         numberOfEntries = currLeafNode->GetNumberOfEntries();

         // Get the first index pageID of the joined tree.
         joinedPageID = seqTree->GetFirstIndexNode();
         // First test if there is, at least, 2 levels in the second tree.
         if (joinedPageID != 0){
            // Allocate the resources to map the entries.
            distSubRepVector = new double[numberOfEntries];
            mapVector = new bool[numberOfEntries];
            
            // For each index node in the joinedTree...
            while (joinedPageID != 0){
               // Get the node and analyze it.
               joinedPage = seqTree->GetPageManager()->GetPage(joinedPageID);
               joinedNode = stSeqNode::CreateNode(joinedPage);
               joinedIndexNode = (stSeqIndexNode *)joinedNode;
               // Get the number of entries in the
               joinedNumberOfEntries = joinedNode->GetNumberOfEntries();
               // Get the representative of the index node.
               repObj.Unserialize(
                     joinedIndexNode->GetObject(joinedIndexNode->GetRepresentativeEntry()),
                     joinedIndexNode->GetObjectSize(joinedIndexNode->GetRepresentativeEntry()));

               // For each entry in the joined index node.
               for (i = 0; i < joinedNumberOfEntries; i++){
                  // Set with default values.
                  for (j = 0; j < numberOfEntries; j++){
                     mapVector[j] = 0.0;
                     mapVector[j] = false;
                  }//end for
                  qualified = false;
                  // For each entry in the first leaf node.
                  for (j = 0; j < numberOfEntries; j++){
                     // Analyze each leaf entry in the first tree with each
                     // subtree of the joined tree.
                     // Get the object in the leaf node.
                     tmpObj.Unserialize(currLeafNode->GetObject(j),
                                        currLeafNode->GetObjectSize(j));
                     // Calculate the distance between the representative to the object.
                     distanceRepres = this->myMetricEvaluator->GetDistance(&repObj,
                                                                     &tmpObj);

                     // It is possible to prune it?
                     if ( fabs(distanceRepres - joinedIndexNode->GetIndexEntry(i).Distance) <=
                               range + joinedIndexNode->GetIndexEntry(i).Radius){
                        // No. Calculate the distance.
                        // But first get the object.
                        subRep.Unserialize(joinedIndexNode->GetObject(i),
                                           joinedIndexNode->GetObjectSize(i));
                        distanceSubRep = this->myMetricEvaluator->GetDistance(&subRep,
                                                                        &tmpObj);
                        // is this a qualified subtree?
                        if (distanceSubRep <= range + joinedIndexNode->GetIndexEntry(i).Radius){
                           // First, do not get the leaf node.
                           // Mark the vector to analyze the leaf node at once.
                           distSubRepVector[j] = distanceSubRep;
                           mapVector[j] = true;
                           qualified = true;
                        }//end if
                     }//end if
                  }//end for
                  // Verify if there is at least one entry in currLeafNode that qualified.
                  if (qualified){
                     // Now get the leaf node.
                     joinedPage2 = seqTree->GetPageManager()->GetPage(joinedIndexNode->GetIndexEntry(i).PageID);
                     joinedNode2 = stSeqNode::CreateNode(joinedPage2);
                     // Convert to a leaf node.
                     joinedLeafNode = (stSeqLeafNode *)joinedNode2;
                     leafNumberOfEntries = joinedLeafNode->GetNumberOfEntries();

                     // For each entry in the first leaf node.
                     for (j = 0; j < numberOfEntries; j++){
                        // Is this entry qualifies?
                        if (mapVector[j]){
                           // Get the object in the leaf node.
                           tmpObj.Unserialize(currLeafNode->GetObject(j),
                                              currLeafNode->GetObjectSize(j));
                           // For each object in the joined leaf node.
                           for (idx = 0; idx < leafNumberOfEntries; idx++) {
                              // Use of the triangle inequality to cut the object.
                              if ( fabs(distSubRepVector[j] - joinedLeafNode->GetLeafEntry(idx).Distance) <= range){
                                 // Rebuild the object
                                 tmpJoinedObj.Unserialize(joinedLeafNode->GetObject(idx),
                                                          joinedLeafNode->GetObjectSize(idx));
                                 // Evaluate distance
                                 distance = this->myMetricEvaluator->GetDistance(&tmpObj,
                                                                           &tmpJoinedObj);
                                 // is this a qualified object?
                                 if (distance <= range){
                                    // Yes!!! It qualifies. Get it!
                                    result->AddJoinedTriple(tmpObj.Clone(),
                                                            tmpJoinedObj.Clone(),
                                                            distance);
                                 }//end if
                              }//end if
                           }//end for
                        }//end if
                     }//end for

                     // Clean the mess.
                     delete joinedNode2;
                     seqTree->GetPageManager()->ReleasePage(joinedPage2);
                  }//end if
               }//end for
               // Get the the next index pageID of the joined tree.
               joinedPageID = joinedIndexNode->GetNextNode();
               // Clean the mess.
               delete joinedNode;
               seqTree->GetPageManager()->ReleasePage(joinedPage);
            }//end while
            // Clean.
            delete[] distSubRepVector;
            delete[] mapVector;
         }else{
            // No. There is only one leaf node in the second tree.
            // Ok. Get the unique leaf node.
            joinedPageID = seqTree->GetFirstLeafNode();
            joinedPage2 = seqTree->GetPageManager()->GetPage(joinedPageID);
            joinedNode2 = stSeqNode::CreateNode(joinedPage2);
            // Convert to a leaf node.
            joinedLeafNode = (stSeqLeafNode *)joinedNode2;
            leafNumberOfEntries = joinedLeafNode->GetNumberOfEntries();

            // For each entry in the first leaf node.
            for (j = 0; j < numberOfEntries; j++){
               // Analyze each leaf entry in the first tree with each
               // subtree of the joined tree.
               // Get the object in the leaf node.
               tmpObj.Unserialize(currLeafNode->GetObject(j),
                                  currLeafNode->GetObjectSize(j));
               // For each object in the joined leaf node.
               for (idx = 0; idx < leafNumberOfEntries; idx++) {
                  // Rebuild the object
                  tmpJoinedObj.Unserialize(joinedLeafNode->GetObject(idx),
                                           joinedLeafNode->GetObjectSize(idx));
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(&tmpObj,
                                                            &tmpJoinedObj);
                  // is this a qualified object?
                  if (distance <= range){
                     // Yes!!! It qualifies. Get it!
                     result->AddJoinedTriple(tmpObj.Clone(),
                                             tmpJoinedObj.Clone(),
                                             distance);
                  }//end if
               }//end for
            }//end for
            // Clean the mess.
            delete joinedNode2;
            seqTree->GetPageManager()->ReleasePage(joinedPage2);
         }//end if
         // Get the the next leaf pageID of the first tree.
         nextPageID = currLeafNode->GetNextNode();
         // Clean the mess.
         delete currNode;
         this->GetPageManager()->ReleasePage(currPage);
      }//end while
   }//end if

   // Return the final result.
   return result;
}//end RangeJoinQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stJoinedResult<ObjectType> * tmpl_stSeqTree::RangeJoinQueryEnzo(
      stSeqTree * joinedtree, double range){
   // Create result
   // Create result
   tJoinedResult * globalResult = new tJoinedResult();
   // Set the result.
   globalResult->SetQueryInfo(RANGEJOINQUERY, -1, range, false);

   //if exists two trees
   if ((GetNumberOfObjects() != 0) && (joinedtree->GetNumberOfObjects() != 0)){
      double distance;
      //if exists index node
      if (GetFirstIndexNode() != 0){
         // exists index node in indexed tree
         u_int32_t indexPageID = GetFirstIndexNode();
         tObject * tmpIndex = new tObject;
         //read node
         stPage * currPageJoin = joinedtree->GetPageManager()->GetPage(
            joinedtree->GetRoot());
         stSeqNode * currNodeJoin = stSeqNode::CreateNode(currPageJoin);
         //number of entries
         u_int32_t numberOfEntriesJoin = currNodeJoin->GetNumberOfEntries();
         while(indexPageID != 0){
            // Read node...
            stPage * indexPageIndex = this->myPageManager->GetPage(indexPageID);
            stSeqIndexNode * indexNodeIndex = new stSeqIndexNode(indexPageIndex);
            //number of entries
            u_int32_t numberOfEntriesIndex = indexNodeIndex->GetNumberOfEntries();
            for (u_int32_t iIndex=0; iIndex < numberOfEntriesIndex; iIndex++){
               // Rebuild the object
               tmpIndex->Unserialize(indexNodeIndex->GetObject(iIndex),
                  indexNodeIndex->GetObjectSize(iIndex));
               // variables
               stPage * leafPageIndex = NULL;
               stSeqLeafNode * leafNodeIndex = NULL;
               tObject * tmpJoin = new tObject;
               // Is it an Index node?
               if (currNodeJoin->GetNodeType() == stSeqNode::INDEX) {
                  // node of join is index
                  stSeqIndexNode * indexNodeJoin = (stSeqIndexNode *)currNodeJoin;
                  // For each entry in node join
                  for (u_int32_t iJoin = 0; iJoin < numberOfEntriesJoin; iJoin++) {
                     // Rebuild the object
                     tmpJoin->Unserialize(indexNodeJoin->GetObject(iJoin),
                        indexNodeJoin->GetObjectSize(iJoin));
                     // Evaluate distance
                     distance = this->myMetricEvaluator->GetDistance(tmpIndex,
                        tmpJoin);
                     // is this a qualified subtree?
                     if (distance <= indexNodeJoin->GetIndexEntry(iJoin).Radius +
                           indexNodeIndex->GetIndexEntry(iIndex).Radius + range){
                        //read node
                        stPage * subPageJoin = joinedtree->GetPageManager()->GetPage(
                           indexNodeJoin->GetIndexEntry(iJoin).PageID);
                        stSeqNode * subNodeJoin = stSeqNode::CreateNode(subPageJoin);
                        //if not read
                        if(leafPageIndex == NULL){
                           // Read sub node...
                           leafPageIndex = this->myPageManager->GetPage(
                              indexNodeIndex->GetIndexEntry(iIndex).PageID);
                           leafNodeIndex = new stSeqLeafNode(leafPageIndex);
                        }//end if
                        // Yes! Analyze it!
                        JoinedTreeRangeJoinRecursive(tmpIndex,
                           indexNodeIndex->GetIndexEntry(iIndex).Radius,
                           subNodeJoin, indexNodeJoin->GetIndexEntry(iJoin).Radius,
                           joinedtree->GetPageManager(), distance, range,
                           leafNodeIndex, globalResult);
                        //free it all
                        delete subNodeJoin;
                        joinedtree->GetPageManager()->ReleasePage(subPageJoin);
                     }//end if
                  }//end for
               }else{
                  // node of join is index
                  stSeqIndexNode * leafNodeJoin = (stSeqIndexNode *)currNodeJoin;
                  // For each entry in node join
                  for (u_int32_t iJoin = 0; iJoin < numberOfEntriesJoin; iJoin++) {
                     // Evaluate distance
                     // Rebuild the object
                     tmpJoin->Unserialize(leafNodeJoin->GetObject(iJoin),
                        leafNodeJoin->GetObjectSize(iJoin));
                     distance = this->myMetricEvaluator->GetDistance(tmpIndex, tmpJoin);
                     // is this a qualified subtree?
                     if (distance <= range){
                        // Yes! Put it in the result set.
                        globalResult->AddJoinedTriple(tmpIndex->Clone(),
                                                      tmpJoin->Clone(),
                                                      distance);
                     }//end if
                  }//end for
               }//end if
               // Free it all
               delete tmpJoin;
               if(leafPageIndex != NULL){
                  delete leafNodeIndex;
                  this->myPageManager->ReleasePage(leafPageIndex);
               }//end if
            }//end for
            //next pageID
            indexPageID = indexNodeIndex->GetNextNode();
            //free it all
            delete indexNodeIndex;
            this->myPageManager->ReleasePage(indexPageIndex);
         }//end while
         delete tmpIndex;
         delete currNodeJoin;
         this->myPageManager->ReleasePage(currPageJoin);
      }else{
         // not exists index node in indexed tree
         tObject * tmp = new tObject();
         tResult * localResult;
         stPage * leafPageIndex = this->myPageManager->GetPage(GetRoot());
         stSeqLeafNode * leafNodeIndex = new stSeqLeafNode(leafPageIndex);
         u_int32_t numberOfEntriesIndex = leafNodeIndex->GetNumberOfEntries();
         for (u_int32_t iIndex=0; iIndex < numberOfEntriesIndex; iIndex++){
            // Rebuild the object
            tmp->Unserialize(leafNodeIndex->GetObject(iIndex),
               leafNodeIndex->GetObjectSize(iIndex));
            // Range Query
            localResult = new tResult();
            localResult = joinedtree->RangeQuery(tmp, range);
            // Copy the localResult objects to globalResult
            for (unsigned k = 0; k < localResult->GetNumOfEntries(); k++) {
               globalResult->AddJoinedTriple(tmp->Clone(),
                                             (*localResult)[k].GetObject()->Clone(),
                                             (*localResult)[k].GetDistance());
            }//end for
            // Cleanning.
            delete localResult;
         }//end while
         // Free it all
         delete tmp;
         delete leafNodeIndex;
         this->myPageManager->ReleasePage(leafPageIndex);
      }//end if
   }//end if
   return globalResult;
}//end RangeJoinQueryEnzo

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::JoinedTreeRangeJoinRecursive(
ObjectType * objIndex, double radiusObjIndex, stSeqNode * currNodeJoin,
double radiusObjJoin, stPageManager * PageManagerJoin, double distRepres,
const double range, stSeqLeafNode * leafNodeIndex, tJoinedResult * globalResult){

   //number of entries
   u_int32_t numberOfEntriesJoin = currNodeJoin->GetNumberOfEntries();
   tObject * tmpJoin = new tObject;
   double distance;
   // Is it an Index node?
   if (currNodeJoin->GetNodeType() == stSeqNode::INDEX) {
      // node of join is index
      stSeqIndexNode * indexNodeJoin = (stSeqIndexNode *)currNodeJoin;
      // For each entry in node join
      for (u_int32_t iJoin = 0; iJoin < numberOfEntriesJoin; iJoin++) {
         // use of the triangle inequality to cut a subtree
         if ( distRepres <= indexNodeJoin->GetIndexEntry(iJoin).Distance +
               indexNodeJoin->GetIndexEntry(iJoin).Radius +
               radiusObjIndex + range ){
            // Rebuild the object
            tmpJoin->Unserialize(indexNodeJoin->GetObject(iJoin),
               indexNodeJoin->GetObjectSize(iJoin));
            // Evaluate distance
            distance = this->myMetricEvaluator->GetDistance(objIndex,
               tmpJoin);
            // is this a qualified subtree?
            if (distance <= radiusObjIndex +
                  indexNodeJoin->GetIndexEntry(iJoin).Radius + range){
               //read node
               stPage * subPageJoin = PageManagerJoin->GetPage(
                  indexNodeJoin->GetIndexEntry(iJoin).PageID);
               stSeqNode * subNodeJoin = stSeqNode::CreateNode(subPageJoin);
               // Yes! Analyze it!
               JoinedTreeRangeJoinRecursive(objIndex, radiusObjIndex, subNodeJoin,
                  indexNodeJoin->GetIndexEntry(iJoin).Radius, PageManagerJoin,
                  distance, range, leafNodeIndex, globalResult);
               //free it all
               delete subNodeJoin;
               PageManagerJoin->ReleasePage(subPageJoin);
            }//end if
         }//end if
      }//end for
   }else{
      // node of join tree is index
      stSeqLeafNode * leafNodeJoin = (stSeqLeafNode *)currNodeJoin;
      //number of entries
      u_int32_t numberOfEntriesIndex = leafNodeIndex->GetNumberOfEntries();
      tObject * tmpIndex = new tObject;
      //create cache objJoin
      tObject ** bufObjJoin = new tObject *[numberOfEntriesJoin];
      // For each entry in node join
      for (u_int32_t iJoin = 0; iJoin < numberOfEntriesJoin; iJoin++) {
         // Rebuild the object
         bufObjJoin[iJoin] = new tObject;
         bufObjJoin[iJoin]->Unserialize(leafNodeJoin->GetObject(iJoin),
            leafNodeJoin->GetObjectSize(iJoin));
      }//end for
      // Lets check all objects in this node
      for (u_int32_t iIndex = 0; iIndex < numberOfEntriesIndex; iIndex++){
         // use of the triangle inequality to cut a subtree
         if ( distRepres <= leafNodeIndex->GetLeafEntry(iIndex).Distance +
               radiusObjJoin + range ){
            // Rebuild the object
            tmpIndex->Unserialize(leafNodeIndex->GetObject(iIndex),
               leafNodeIndex->GetObjectSize(iIndex));
            // For each entry in node join
            for (u_int32_t iJoin = 0; iJoin < numberOfEntriesJoin; iJoin++) {
               // use of the triangle inequality to cut a subtree
               if ( distRepres <= leafNodeJoin->GetLeafEntry(iJoin).Distance +
                     radiusObjIndex + range){
                  // Evaluate distance
                  distance = this->myMetricEvaluator->GetDistance(tmpIndex, bufObjJoin[iJoin]);
                  // is this a qualified subtree?
                  if (distance <= range){
                     // Yes! Put it in the result set.
                     globalResult->AddJoinedTriple(tmpIndex->Clone(),
                                                   bufObjJoin[iJoin]->Clone(),
                                                   distance);
                  }//end if
               }//end if
            }//end for
         }//end if
      }//end for
      delete tmpIndex;
      // Free it all
      for (u_int32_t iJoin = 0; iJoin < numberOfEntriesJoin; iJoin++) {
         delete bufObjJoin[iJoin];
      }//end for
      delete [] bufObjJoin;

   }//end if
   // Free it all
   delete tmpJoin;
}//end JoinedTreeRangeJoinRecursive

//==============================================================================
// End of Queries
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stTreeInfoResult * tmpl_stSeqTree::GetTreeInfo(){
   stTreeInformation * info;

   // No cache of information. I think a cahe would be a good idea.
   info = new stTreeInformation(GetHeight(), GetNumberOfObjects());

   // Let's get the information!
   GetTreeInfoRecursive(this->GetRoot(), 0, info);

   // Optimal tree
   if (info->GetMeanObjectSize() != 0){
      info->CalculateOptimalTreeInfo(int(this->myPageManager->GetMinimumPageSize() /
            info->GetMeanObjectSize()));
   }//end if

   return info;
}//end GetTreeInfo

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::GetTreeInfoRecursive(u_int32_t pageID, int level,
      stTreeInformation * info){
   stPage * currPage;
   stSeqNode * currNode;
   u_int32_t i;
   ObjectType tmp;

   // Let's search
   if (pageID != 0){
      // Update node count
      info->UpdateNodeCount(level);

      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;

         // Object count
         info->UpdateObjectCount(level, indexNode->GetNumberOfEntries());

         // Scan all entries
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            GetTreeInfoRecursive(indexNode->GetIndexEntry(i).PageID, level + 1,
                                 info);
         }//end for
      }else{
         // No, it is a leaf node. Get it.
         stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;

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
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end GetTreeInfoRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::ObjectIntersectionsRecursive(u_int32_t pageID,
      ObjectType * obj, int level, stTreeInformation * info){
   stPage * currPage;
   stSeqNode * currNode;
   u_int32_t i;
   ObjectType tmp;
   double d;

   // Let's search
   if (pageID != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it a Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Get Index node
         stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;

         // Scan all entries
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            tmp.Unserialize(indexNode->GetObject(i),
                            indexNode->GetObjectSize(i));
            d = this->myMetricEvaluator->GetDistance(&tmp, obj);
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
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end ObjectIntersectionsRecursive

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSeqTree::GetRealIndexObjectsCount(){
   stPage * currPage, * currChildPage;
   stSeqNode * currNode, * currChildNode;
   stSeqIndexNode * indexNode;
   stSeqLeafNode * leafNode;
   u_int32_t idx, totalNumberOfEntries, numberOfEntries;
   u_int32_t nextNode;

   totalNumberOfEntries = 0;
   nextNode = this->GetFirstIndexNode();

   // Let's search
   while (nextNode != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(nextNode);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // No, it is a leaf node. Get it.
         indexNode = (stSeqIndexNode *)currNode;
         // Add the total number of entries.
         numberOfEntries = indexNode->GetNumberOfEntries();

         // Call the RangeQuery
         for (idx = 0; idx < numberOfEntries; idx++) {
            // Read node...
            currChildPage = this->myPageManager->GetPage(indexNode->GetIndexEntry(idx).PageID);
            currChildNode = stSeqNode::CreateNode(currChildPage);
            // Is it an Index node?
            if (currChildNode->GetNodeType() == stSeqNode::INDEX) {
               // Opss. There is an error!
               #ifdef __stPRINTMSG__
                  cout << "\nInvalid pageID in leaf node!";
               #endif //__stPRINTMSG__
               // Stop the search.
               nextNode = 0;
               totalNumberOfEntries = 0;
            }else{
               // No, it is a leaf node. Get it.
               leafNode = (stSeqLeafNode *)currChildNode;
               // Add the total number of entries.
               totalNumberOfEntries += leafNode->GetNumberOfEntries();
            }//end if
            // Free it all
            delete currChildNode;
            this->myPageManager->ReleasePage(currChildPage);
         }//end if
         nextNode = indexNode->GetNextNode();
      }else{
         // Opss. There is an error!
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in index node!";
         #endif //__stPRINTMSG__
         // Stop the search.
         nextNode = 0;
         totalNumberOfEntries = 0;
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return totalNumberOfEntries;
}//end stDBMTree::GetRealIndexObjectsCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
u_int32_t tmpl_stSeqTree::GetRealLeafObjectsCount(){
   stPage * currPage;
   stSeqNode * currNode;
   stSeqLeafNode * leafNode;
   u_int32_t totalNumberOfEntries;
   u_int32_t nextNode;

   totalNumberOfEntries = 0;
   nextNode = this->GetFirstLeafNode();

   // Let's search
   while (nextNode != 0){
      // Read node...
      currPage = this->myPageManager->GetPage(nextNode);
      currNode = stSeqNode::CreateNode(currPage);
      // Is it an Index node?
      if (currNode->GetNodeType() == stSeqNode::INDEX) {
         // Opss. There is an error!
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in leaf node!";
         #endif //__stPRINTMSG__
         // Stop the search.
         nextNode = 0;
         totalNumberOfEntries = 0;
      }else{
         // No, it is a leaf node. Get it.
         leafNode = (stSeqLeafNode *)currNode;
         // Add the total number of entries.
         totalNumberOfEntries += leafNode->GetNumberOfEntries();
         nextNode = leafNode->GetNextNode();
      }//end else

      // Free it all
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if

   return totalNumberOfEntries;
}//end stDBMTree::GetRealLeafObjectsCount

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSeqTree::Consistency(){
   u_int32_t idx;
   stPage * currPage;
   stSeqNode * currNode;
   u_int32_t numberOfEntries;
   double distance, radius;
   ObjectType repObj, subRep, tmpObj;
   bool result = true;

   // Let's search
   if (this->GetRoot() == 0){
      // Problem!
      result = false;
   }else{
      // Read the root node
      currPage = this->myPageManager->GetPage(this->GetRoot());
      // Test the root pageID.
      if (currPage == NULL){
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in root!";
         #endif //__stPRINTMSG__
         // Problem!
         result = false;
      }else{
         currNode = stSeqNode::CreateNode(currPage);
         // Is it an Index node?
         if (currNode->GetNodeType() == stSeqNode::INDEX){
            // Get Index node
            stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
            // Get the number of entries
            numberOfEntries = indexNode->GetNumberOfEntries();
            // Check the field entries.
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Test the subtree.
               if (indexNode->GetIndexEntry(idx).PageID){
                  // Get the subtree representative.
                  subRep.Unserialize(indexNode->GetObject(idx),
                                     indexNode->GetObjectSize(idx));
                  // Call it recursively.
                  result = result && this->Consistency(indexNode->GetIndexEntry(idx).PageID,
                                                       &subRep, radius);
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
               }//end if
            }//end for
         }else{
            // No, it is a leaf node.
            // Do not do anything!
         }//end if
         // Free it all
         delete currNode;
         this->myPageManager->ReleasePage(currPage);
      }//end if
   }//end if

   return result;
}//end stDBMTree::Consistency

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSeqTree::Consistency(u_int32_t pageID, ObjectType * repObj,
      double & radius){
   u_int32_t idx;
   stPage * currPage;
   stSeqNode * currNode;
   u_int32_t numberOfEntries;
   double distance;
   ObjectType subRep, localRep, tmpObj;
   int idxRep;
   radius = MAXDOUBLE;
   bool result = true;

   // Let's search
   if (pageID == 0){
      // Problem!
      result = false;
   }else{
      // Read the root node
      currPage = this->myPageManager->GetPage(pageID);
      // Test the pageID consistency.
      if (currPage == NULL){
         #ifdef __stPRINTMSG__
            cout << "\nInvalid pageID in " << pageID << ".";
         #endif //__stPRINTMSG__
         // Problem!
         result = false;
      }else{
         currNode = stSeqNode::CreateNode(currPage);
         // Is it an Index node?
         if (currNode->GetNodeType() == stSeqNode::INDEX){
            // Get Index node
            stSeqIndexNode * indexNode = (stSeqIndexNode *)currNode;
            // Get the number of entries
            numberOfEntries = indexNode->GetNumberOfEntries();
            // Get the representative index.
            idxRep = indexNode->GetRepresentativeEntry();
            // Get the representative.
            localRep.Unserialize(indexNode->GetObject(idxRep),
                                 indexNode->GetObjectSize(idxRep));
            // Check the field entries.
            for (idx = 0; idx < numberOfEntries; idx++) {
               // Get the subtree representative.
               subRep.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Call it recursively.
               result = result && this->Consistency(indexNode->GetIndexEntry(idx).PageID,
                                                    &subRep, radius);
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
               // Calculate the distance.
               tmpObj.Unserialize(indexNode->GetObject(idx),
                                  indexNode->GetObjectSize(idx));
               // Evaluate it!
               distance = this->myMetricEvaluator->GetDistance(repObj, &tmpObj);
               // Test the distance for every entry.
               if (distance != indexNode->GetIndexEntry(idx).Distance){
                  #ifdef __stPRINTMSG__
                     cout << "\nThere is a distance problem in entry " << idx
                          << " in pageID " << indexNode->GetPageID()
                          << " distance: " << indexNode->GetIndexEntry(idx).Distance
                          << " measured " << distance << ".";
                  #endif //__stPRINTMSG__
                  // Problem!
                  result = false;
               }//end if
            }//end for
         }else{
            // No, it is a leaf node.
            // Get Index node
            stSeqLeafNode * leafNode = (stSeqLeafNode *)currNode;
            // Get the number of entries
            numberOfEntries = leafNode->GetNumberOfEntries();
            // Get the representative index.
            idxRep = leafNode->GetRepresentativeEntry();
            // Test it.
            if ((idxRep < 0) || (idxRep >= leafNode->GetNumberOfEntries())){
               #ifdef __stPRINTMSG__
                  cout << "\nThere is not a representative entry in pageID "
                       << pageID << ".";
               #endif //__stPRINTMSG__
               // Problem!
               result = false;
            }else{
               // Get the representative.
               localRep.Unserialize(leafNode->GetObject(idxRep),
                                    leafNode->GetObjectSize(idxRep));
               // Check the field entries.
               for (idx = 0; idx < numberOfEntries; idx++) {
                  // Calculate the distance.
                  tmpObj.Unserialize(leafNode->GetObject(idx),
                                     leafNode->GetObjectSize(idx));
                  // Evaluate it!
                  distance = this->myMetricEvaluator->GetDistance(&localRep, &tmpObj);
                  // Test the distance for every entry.
                  if (distance != leafNode->GetLeafEntry(idx).Distance){
                     #ifdef __stPRINTMSG__
                        cout << "\nThere is a distance problem in entry " << idx
                             << " in pageID " << leafNode->GetPageID()
                             << " distance: " << leafNode->GetLeafEntry(idx).Distance
                             << " measured " << distance << ".";
                     #endif //__stPRINTMSG__
                     // Problem!
                     result = false;
                  }//end if
               }//end for
            }//end if
         }//end if
         // Set the radius for the upper levels.
         radius = currNode->GetMinimumRadius();
         // Test if the representative is the same to the upper level.
         if (!localRep.IsEqual(repObj)){
            #ifdef __stPRINTMSG__
               cout << "\nThere is a problem in the representative entry " << idxRep
                    << " in pageID " << currPage->GetPageID()
                    << ". The representative is not the same of the upper level.";
            #endif //__stPRINTMSG__
            // Problem!
            result = false;
         }//end if
         // Free it all
         delete currNode;
         this->myPageManager->ReleasePage(currPage);
      }//end if
   }//end if

   return result;
}//end stDBMTree::Consistency

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::Optimize(){

   if (this->GetHeight() >= 3){
      ShrinkRecursive(this->GetRoot(), 0);
      // Notify modifications
      HeaderUpdate = true;
      // Don't worry. This is a debug block!!!
   #ifdef __stDEBUG__
   }else{
      cout << "Unable to perform the Shrink. This tree has only " <<
         this->GetHeight() << " level(s).\n";
   #endif //__stDEBUG__
   }//end if
}//end tmpl_stSeqTree::Optimize

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSeqTree::ShrinkRecursive(u_int32_t pageID, int level){
   stPage * currPage;
   stSeqNode * currNode;
   stSeqIndexNode * indexNode;
   double radius;
   u_int32_t i;

   // Let's search
   if (pageID != 0){

      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);

      #ifdef __stDEBUG__
         if (currNode->GetNodeType() != stSeqNode::INDEX){
            // This tree has less than 3 levels. This method will not work.
            throw logic_error("Shrink reached the bottom of the tree.");
         }//end if
      #endif //__stDEBUG__

      indexNode = (stSeqIndexNode *)currNode;

      // Where am I ?
      if (level == GetHeight() - 3){
         // Shrink next level!
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            #ifdef __stDEBUG__
               cout << "Level:" << level << ". Begin of the local Shrink of " <<
                     indexNode->GetIndexEntry(i).PageID <<
                     " which current radius is " <<
                     indexNode->GetIndexEntry(i).Radius << ".\n";
            #endif //__stDEBUG__
            indexNode->GetIndexEntry(i).Radius = Shrink(
                  indexNode->GetIndexEntry(i).PageID);

            #ifdef __stDEBUG__
               cout << "Level:" << level << ". End of the local Shrink of " <<
                     indexNode->GetIndexEntry(i).PageID <<
                     " which current radius is " <<
                     indexNode->GetIndexEntry(i).Radius << ".\n";
            #endif //__stDEBUG__

         }//end for
      }else{
         // Move on...
         for (i = 0; i < indexNode->GetNumberOfEntries(); i++){
            indexNode->GetIndexEntry(i).Radius = ShrinkRecursive(
                  indexNode->GetIndexEntry(i).PageID, level + 1);
         }//end for
      }//end if
      
      // Update my radius.
      radius = indexNode->GetMinimumRadius();
      
      // Write me and get the garbage.
      delete currNode;
      this->myPageManager->WritePage(currPage);
      this->myPageManager->ReleasePage(currPage);
      return radius;
   }else{
      // This tree is corrupted or is empty.
      throw logic_error("The given tree is corrupted or empty.");
   }//end if
}//end ShrinkRecursive

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
double tmpl_stSeqTree::Shrink(u_int32_t pageID){
   stPage * currPage;
   stSeqNode * currNode;
   stSeqIndexNode * indexNode;
   tMemLeafNode ** memLeafNodes;
   stSeqNode * tmpNode;
   stPage * tmpPage;
   stSeqLeafNode * leafNode;
   double radius;
   int maxSwaps;
   u_int32_t nodeCount;
   u_int32_t idx;
   u_int32_t i;

   // Let's search
   if (pageID != 0){

      // Read node...
      currPage = this->myPageManager->GetPage(pageID);
      currNode = stSeqNode::CreateNode(currPage);

      #ifdef __stDEBUG__
         if (currNode->GetNodeType() != stSeqNode::INDEX){
            // This tree has less than 3 levels. This method will not work.
            throw logic_error("Shrink reached the bottom of the tree.");
         }//end if
      #endif //__stDEBUG__

      // Cast currNode to stSeqIndexNode as it must be...
      indexNode = (stSeqIndexNode *)currNode;
      nodeCount = indexNode->GetNumberOfEntries();

      #ifdef __stDEBUG__
         cout << "Local Shrink in " <<
               indexNode->GetPageID() <<
               " which current radius is " <<
               indexNode->GetMinimumRadius() << ".\n";
      #endif //__stDEBUG__


      // Create  all stSeqMemLeafNodes
      memLeafNodes = new tMemLeafNode * [nodeCount];
      maxSwaps = 0;
      for (i = 0; i < nodeCount; i++){
         // Read leaf
         tmpPage = this->myPageManager->GetPage(indexNode->GetIndexEntry(i).PageID);
         tmpNode = stSeqNode::CreateNode(tmpPage);

         #ifdef __stDEBUG__
            if (tmpNode->GetNodeType() != stSeqNode::LEAF){
               // This tree has less than 3 levels. This method will not work.
               throw logic_error("Oops. This tree is corrupted.");
            }//end if
         #endif //__stDEBUG__
         leafNode = (stSeqLeafNode *) tmpNode;

         // Update maxSwaps
         maxSwaps += leafNode->GetNumberOfEntries();

         // Assemble memory version
         memLeafNodes[i] = new tMemLeafNode(leafNode);
      }//end for
      maxSwaps *= 3;

      // Execute the local Shrink
      LocalShrink(memLeafNodes, nodeCount, maxSwaps);

      // Rebuild nodes and write them. Of course, the empty ones will be disposed.
      idx = 0;
      for (i = 0; i < nodeCount; i++){
         // Dispose memory version
         if (memLeafNodes[i]->GetNumberOfEntries() != 0){
            leafNode = memLeafNodes[i]->ReleaseNode();
            delete memLeafNodes[i];

            // Update entry
            indexNode->GetIndexEntry(idx).Radius = leafNode->GetMinimumRadius();
            idx++;

            // Write back
            tmpPage = leafNode->GetPage();
            delete leafNode;
            this->myPageManager->WritePage(tmpPage);
            this->myPageManager->ReleasePage(tmpPage);
         }else{
            // Empty node
            leafNode = memLeafNodes[i]->ReleaseNode();
            delete memLeafNodes[i];

            // Remove entry
            indexNode->RemoveEntry(idx);
            #ifdef __stDEBUG__
               cout << "Node " << i << " is no more!\n";
            #endif //__stDEBUG__

            // Dispose empty node
            tmpPage = leafNode->GetPage();
            delete leafNode;
            DisposePage(tmpPage);
         }//end if
      }//end for
      delete[] memLeafNodes;

      // Update my radius.
      radius = indexNode->GetMinimumRadius();

      // Write me and get the garbage.
      delete currNode;
      this->myPageManager->WritePage(currPage);
      this->myPageManager->ReleasePage(currPage);
      return radius;
   }else{
      // This tree is corrupted or is empty.
      throw logic_error("The given tree is corrupted or empty.");
   }//end if
}//end Shrink

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void tmpl_stSeqTree::LocalShrink(
      tmpl_stSeqTree::tMemLeafNode ** memLeafNodes, int nodeCount,
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
                  if (ShrinkCanSwap(memLeafNodes[src], memLeafNodes[i],
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
}//end LocalShrink

//-----------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool tmpl_stSeqTree::ShrinkCanSwap(
      tmpl_stSeqTree::tMemLeafNode * src, tmpl_stSeqTree::tMemLeafNode * dst,
      double & distance){

   // Check to see if destination is empty         
   if (dst->GetNumberOfEntries() == 0){
      return false;
   }//end if

   // Calculate the distance between src's last object and dst's representative
   distance = this->myMetricEvaluator->GetDistance(src->LastObject(), dst->RepObject());

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
}//end ShrinkIntersects
