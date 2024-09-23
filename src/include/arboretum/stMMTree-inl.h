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
* This file is the implementation of stMMTree methods.
* $Author: marcos $
*
* @author Ives Renê Venturini Pola (ives@icmc.usp.br)
*/

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stMMTree<ObjectType, EvaluatorType>::stMMTree(stPageManager * pageman):
      stMetricTree<ObjectType, EvaluatorType>(pageman) {
   // check if it should create or just read it
   if (this->myPageManager->IsEmpty()) {
      // Create it
      this->Create();
   }else{
      // Use it
      this->LoadHeader();
   }//end if
}//end stMMTree<ObjectType><EvaluatorType>::stMMTree

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stMMTree<ObjectType, EvaluatorType>::Add(tObject * obj) {
   /// The object will be added in the page witch have room for it.
   /// When the page reached is full, this method will create a new page and link
   /// it to the proprer region.

   stPage * currPage;
   stMMNode * currNode;
   stMMNode * ParentNode = NULL;
   stPage * ParentPage = NULL;
   tObject * objtmp = NULL;
   double distance1;
   double distance2;
   int id;

   // Check if it fit in the page
   if (obj->GetSerializedSize() > this->myPageManager->GetMinimumPageSize() -
       sizeof(unsigned char) - sizeof(double) - (NUMBEROFREGIONS * sizeof(u_int32_t)) -
       sizeof(u_int32_t)) {
      return false;
   }//end if

   // Adding object
   if (this->Header->Root == 0) {
      //Create new page
      currPage = this->myPageManager->GetNewPage();
      Header->NodeCount++;
      currNode = new stMMNode(currPage, true);
      currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
      Header->Root = currPage->GetPageID();
      WriteHeader();
      this->myPageManager->WritePage(currPage);
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }else{
      // Get root
      currPage = this->myPageManager->GetPage(this->Header->Root);
      currNode = new stMMNode(currPage);

      // Try to add
      id = currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
      if (id >= 0) {
         // Added!
         // Calculate the distance.
         objtmp = new tObject();
         objtmp->Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
         distance1 = this->myMetricEvaluator->GetDistance(objtmp, obj);
         // Store it in the header.
         currNode->SetDistance(distance1);
         this->myPageManager->WritePage(currPage);
         delete currNode;
         this->myPageManager->ReleasePage(currPage);
      }else{
         objtmp = new tObject();
         
         while (currNode != NULL) {

            if (ParentNode != NULL) {
               delete ParentNode;
            }//end if

            ParentNode = currNode;
            ParentPage = currPage;

            objtmp->Unserialize(ParentNode->GetObject(0),
                                ParentNode->GetObjectSize(0));
            distance1 = this->myMetricEvaluator->GetDistance(objtmp, obj);

            objtmp->Unserialize(ParentNode->GetObject(1),
                                ParentNode->GetObjectSize(1));
            distance2 = this->myMetricEvaluator->GetDistance(objtmp, obj);

            if ((distance1 < ParentNode->GetDistance()) &&
                (distance2 < ParentNode->GetDistance())) {
                
               //Visit region 1
               if (ParentNode->GetChildPageID(0) <= 0) {
                  //there is no child node = create, store the object, and leave
                  currPage = this->myPageManager->GetNewPage();
                  Header->NodeCount++;
                  currNode = new stMMNode(currPage, true);
                  currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                  ParentNode->SetChildPageID(0, currPage->GetPageID());
                  this->myPageManager->WritePage(currPage);
                  this->myPageManager->WritePage(ParentPage);
                  delete ParentNode;
                  delete currNode;
                  this->myPageManager->ReleasePage(ParentPage);
                  this->myPageManager->ReleasePage(currPage);
                  currNode = NULL;
                  currPage = NULL;
               }else{
                  //there is a child node, inspect it
                  currPage = this->myPageManager->GetPage(ParentNode->GetChildPageID(0));
                  currNode = new stMMNode(currPage);

                  if (currNode->GetNumberOfEntries() == 1) {
                     //Add the entry, store the distance and leave
                     currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                     //calculate the distance
                     objtmp->Unserialize(currNode->GetObject(0),
                                         currNode->GetObjectSize(0));
                     distance1 = this->myMetricEvaluator->GetDistance(objtmp, obj);
                     //store it in the header
                     currNode->SetDistance(distance1);
                     //clean
                     this->myPageManager->WritePage(currPage);
                     delete currNode;
                     this->myPageManager->ReleasePage(currPage);
                     currNode = NULL;
                  }//end if
               }//end if
            }else if ((distance1 < ParentNode->GetDistance()) &&
                      (distance2 >= ParentNode->GetDistance())) { //end if region 1
               //Visit region 2
               if (ParentNode->GetChildPageID(1) <= 0) {
                  //there is no child node = create, store the object, and leave
                  currPage = this->myPageManager->GetNewPage();
                  Header->NodeCount++;
                  currNode = new stMMNode(currPage, true);
                  currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                  ParentNode->SetChildPageID(1, currPage->GetPageID());
                  this->myPageManager->WritePage(currPage);
                  this->myPageManager->WritePage(ParentPage);
                  delete ParentNode;
                  delete currNode;
                  this->myPageManager->ReleasePage(ParentPage);
                  this->myPageManager->ReleasePage(currPage);
                  currNode = NULL;
                  currPage = NULL;
               }else{
                  //there is a child node, inspect it
                  currPage = this->myPageManager->GetPage(ParentNode->GetChildPageID(1));
                  currNode = new stMMNode(currPage);

                  if (currNode->GetNumberOfEntries() == 1) {
                     //Add the entry, store the distance and leave
                     currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                     //calculate the distance
                     objtmp->Unserialize(currNode->GetObject(0),
                                         currNode->GetObjectSize(0));
                     distance1 = this->myMetricEvaluator->GetDistance(objtmp, obj);
                     //store it in the header
                     currNode->SetDistance(distance1);
                     //clean
                     this->myPageManager->WritePage(currPage);
                     delete currNode;
                     this->myPageManager->ReleasePage(currPage);
                     currNode = NULL;
                  }//end if
               }//end if
            }else if ((distance1 >= ParentNode->GetDistance()) &&
                      (distance2 < ParentNode->GetDistance())) { //end if region 2
               //Visit region 3
               if (ParentNode->GetChildPageID(2) <= 0) {
                  //there is no child node = create, store the object, and leave
                  currPage = this->myPageManager->GetNewPage();
                  Header->NodeCount++;
                  currNode = new stMMNode(currPage, true);
                  currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                  ParentNode->SetChildPageID(2, currPage->GetPageID());
                  this->myPageManager->WritePage(currPage);
                  this->myPageManager->WritePage(ParentPage);
                  delete ParentNode;
                  delete currNode;
                  this->myPageManager->ReleasePage(ParentPage);
                  this->myPageManager->ReleasePage(currPage);
                  currNode = NULL;
                  currPage = NULL;
               }else{
                  //there is a child node, inspect it
                  currPage = this->myPageManager->GetPage(ParentNode->GetChildPageID(2));
                  currNode = new stMMNode(currPage);

                  if (currNode->GetNumberOfEntries() == 1) {
                     //Add the entry, store the distance and leave
                     currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                     //calculate the distance
                     objtmp->Unserialize(currNode->GetObject(0),
                                         currNode->GetObjectSize(0));
                     distance1 = this->myMetricEvaluator->GetDistance(objtmp, obj);
                     //store it in the header
                     currNode->SetDistance(distance1);
                     //clean
                     this->myPageManager->WritePage(currPage);
                     delete currNode;
                     this->myPageManager->ReleasePage(currPage);
                     currNode = NULL;
                  }//end if
               }//end if
            }else if ((distance2 >= ParentNode->GetDistance()) &&
                      (distance1 >= ParentNode->GetDistance())) {
               // Visit region 4
               if (ParentNode->GetChildPageID(3) <= 0) {
                  //there is no child node = create, store the object, and leave
                  currPage = this->myPageManager->GetNewPage();
                  Header->NodeCount++;
                  currNode = new stMMNode(currPage, true);
                  currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                  ParentNode->SetChildPageID(3, currPage->GetPageID());
                  this->myPageManager->WritePage(currPage);
                  this->myPageManager->WritePage(ParentPage);
                  delete ParentNode;
                  delete currNode;
                  this->myPageManager->ReleasePage(ParentPage);
                  this->myPageManager->ReleasePage(currPage);
                  currNode = NULL;
                  currPage = NULL;
               }else{
                  //there is a child node, inspect it
                  currPage = this->myPageManager->GetPage(ParentNode->GetChildPageID(3));
                  currNode = new stMMNode(currPage);

                  if (currNode->GetNumberOfEntries() == 1) {
                     //Add the entry, store the distance and leave
                     currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());
                     //calculate the distance
                     objtmp->Unserialize(currNode->GetObject(0),
                                         currNode->GetObjectSize(0));
                     distance1 = this->myMetricEvaluator->GetDistance(objtmp, obj);
                     //store it in the header
                     currNode->SetDistance(distance1);
                     //clean
                     this->myPageManager->WritePage(currPage);
                     delete currNode;
                     this->myPageManager->ReleasePage(currPage);
                     currNode = NULL;
                  }//end if
               }//end if
            }//end if region 4

            //Try to Balance ?
            switch (Header->InsertionMethod) {
               case stMMTree::imTRY2BALANCE :
                  if ((currNode != NULL) && (ParentNode != NULL)) {
                     if (ExaustiveBalance(ParentNode, currNode, obj)) {
                        currNode = NULL;
                     }//end if
                  }//end if
                  break;  //end stMMTree::imTRY2BALANCE

               case stMMTree::imNOBALANCE : //No Balance trying
                  break;  //end stMMTree::imNOBALANCE
                  
            }//end switch
         }//end while
      }//end if
   }//end if

   if (objtmp != NULL) {
      delete objtmp;
   }//end if

   return true;
}//end stMMTree<ObjectType><EvaluatorType>::Add

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stMMTree<ObjectType, EvaluatorType>::HaveChild(u_int32_t page) {
   stPage   * currPage;
   stMMNode * currNode;
   int i;

   if (page <= 0) {
      return false;
   }else{
      currPage = this->myPageManager->GetPage(page);
      currNode = new stMMNode(currPage);

      for (i = 0; i < NUMBEROFREGIONS; i++) {
         if (currNode->GetChildPageID(i) > 0) {
            delete currNode;
            return true;
         }//end if
      }//end for

      delete currNode;
      return false;
   }//end if
}//end stMMTree<ObjectType, EvaluatorType>::HaveChild

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMMTree<ObjectType, EvaluatorType>::CountObjs(stMMNode * Parent) {
   stPage * currPage;
   stMMNode * currNode;
   int idx;
   int objCount = 0;

   objCount = Parent->GetNumberOfEntries();

   for (idx = 0; idx < NUMBEROFREGIONS; idx++) {
      if (Parent->GetChildPageID(idx) > 0) {
         currPage = this->myPageManager->GetPage(Parent->GetChildPageID(idx));
         currNode = new stMMNode(currPage);
         objCount += currNode->GetNumberOfEntries();
         delete currNode;
      }//end if
   }//end for

   return objCount;
}//end stMMTree<ObjectType, EvaluatorType>::CountObjs

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
int stMMTree<ObjectType, EvaluatorType>::Associate(stMMNode * Parent,
                                                   tObject ** objects) {
   stPage * currPage;
   stMMNode * currNode;
   tObject tmp;
   int nobjs = 0;
   int idx;

   tmp.Unserialize(Parent->GetObject(0), Parent->GetObjectSize(0));
   objects[nobjs++] = tmp.Clone();
   tmp.Unserialize(Parent->GetObject(1), Parent->GetObjectSize(1));
   objects[nobjs++] = tmp.Clone();

   for (idx = 0; idx < NUMBEROFREGIONS; idx++) {
      if (Parent->GetChildPageID(idx) > 0) {
         currPage = this->myPageManager->GetPage(Parent->GetChildPageID(idx));
         currNode = new stMMNode(currPage);

         tmp.Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
         objects[nobjs++] = tmp.Clone();

         if (currNode->GetNumberOfEntries() == 2) {
            tmp.Unserialize(currNode->GetObject(1), currNode->GetObjectSize(1));
            objects[nobjs++] = tmp.Clone();
         }//end if
         delete currNode;
      }//end if
   }//end for
   return nobjs;
}//end stMMTree<ObjectType, EvaluatorType>::Associate

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stMMTree<ObjectType, EvaluatorType>::Distribute(tObject ** objects,
                                          int numObjs, int * regions) {

   int i, j, k;
   int c[NUMBEROFREGIONS];
   bool ret;

   for (i = 0; i < NUMBEROFUNKNOWN; i++) {
      for (j = 0; j < NUMBEROFUNKNOWN; j++) {
         distances[i][j] = -1;
      }//end for
   }//end for

   for (i = 0; i < numObjs; i++) {
      for (j = i + 1; j <= numObjs; j++) {
         if ((j != 1) && (i != j)) {

            for (k = 0; k < NUMBEROFREGIONS; k++) {
               c[k] = 0;
               regions[k] = -1;
            }//end for
            if (distances[i][j] == -1) {
              distances[i][j] = this->myMetricEvaluator->GetDistance(objects[i], objects[j]);
              distances[j][i] = distances[i][j];
            }//end if

            regions[i] = 0;
            regions[j] = 0;

            for (k = 0; k <= numObjs; k++) {
               if ((k != i) && (k != j)) {
                  if (distances[k][i] == -1) {
                     distances[i][k] = this->myMetricEvaluator->GetDistance(objects[k], objects[i]);
                     distances[k][i] = distances[i][k];
                  }//end if
                  if (distances[k][j] == -1) {
                     distances[j][k] = this->myMetricEvaluator->GetDistance(objects[k], objects[j]);
                     distances[k][j] = distances[j][k];
                  }//end if
                  if ((distances[k][i] < distances[i][j]) &&
                      (distances[k][j] < distances[i][j])) {
                     //region 1
                     if ((++c[0]) == 3) {
                        // stop the loop
                        k = numObjs + 1;
                     }else{
                        regions[k] = 1;
                     }//end if
                  }else if ((distances[k][i] < distances[i][j]) &&
                            (distances[k][j] >= distances[i][j])) {
                     //region 2
                     if ((++c[1]) == 3) {
                        // stop the loop
                        k = numObjs + 1;
                     }else{
                        regions[k] = 2;
                     }//end if
                  }else if ((distances[k][i] >= distances[i][j]) &&
                            (distances[k][j] < distances[i][j])) {
                     //region 3
                     if ((++c[2]) == 3) {
                        // stop the loop
                        k = numObjs + 1;
                     }else{
                        regions[k] = 3;
                     }//end if
                  }else{
                    //region 4
                     if ((++c[3]) == 3) {
                        // stop the loop
                        k = numObjs + 1;
                     }else{
                        regions[k] = 4;
                     }//end if
                  }//end if
               }//end if
            }//end if
         }//end for

         ret = true;
         for (k = 0; k < NUMBEROFREGIONS; k++) {
            ret = (ret && (c[k] < 3));
         }//end if
         if (ret) {
            // Found feasible distribution.
            return true;
         }//end if
      }//end for
   }//end for

   // didnt found an alternative distribution
   return false;
}//end stMMTree<ObjectType, EvaluatorType>::Distribute

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMMTree<ObjectType, EvaluatorType>::Realocate(tObject ** objects,
                          int numObjs, stMMNode * Parent, int * regions) {

   int i, j;
   int u0, u1, u2, u3, u4;
   stPage * pages[NUMBEROFREGIONS];
   stMMNode * nodes[NUMBEROFREGIONS];
   int idx;

   for (idx = 0; idx < NUMBEROFREGIONS; idx++) {
      pages[idx] = NULL;
      nodes[idx] = NULL;
      if (Parent->GetChildPageID(idx) > 0) {
        pages[idx] = this->myPageManager->GetPage(Parent->GetChildPageID(idx));
        nodes[idx] = new stMMNode(pages[idx], true);
      }//end if
   }//end for

   Parent->Clear();

   for (j = 0; j <= numObjs; j++) {
      switch (regions[j]) {
         case 0 :
            if (Parent->GetNumberOfEntries() ==  0) {
               Parent->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());
               u0 = j;
            }else{
               Parent->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());
               if (distances[u0][j] != -1) {
                  Parent->SetDistance(distances[u0][j]);
               }else{
                  Parent->SetDistance(this->myMetricEvaluator->GetDistance(objects[u0], objects[j]));
               }//end if
            }//end if
            break; //end case 0

         case 1:
            if (nodes[0] == NULL) {
               pages[0] = this->myPageManager->GetNewPage();
               Header->NodeCount++;
               Parent->SetChildPageID(0, pages[0]->GetPageID());
               nodes[0] = new stMMNode(pages[0], true);
            }//end if
            if (nodes[0]->GetNumberOfEntries() == 0) {
               nodes[0]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());
               u1 = j;
            }else{
                nodes[0]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());

                if (distances[u1][j] != -1 ) {
                   nodes[0]->SetDistance(distances[u1][j]);
                }else{
                   nodes[0]->SetDistance(this->myMetricEvaluator->GetDistance(objects[u1], objects[j]));
                }//end if
            }//end if
            break; //end case 1

         case 2:
            if (nodes[1] == NULL) {
               pages[1] = this->myPageManager->GetNewPage();
               Header->NodeCount++;
               Parent->SetChildPageID(1, pages[1]->GetPageID());
               nodes[1] = new stMMNode(pages[1], true);
            }//end if
            if (nodes[1]->GetNumberOfEntries() == 0) {
                nodes[1]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());
                u2 = j;
            }else{
               nodes[1]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());

               if (distances[u2][j] != -1) {
                  nodes[1]->SetDistance(distances[u2][j]);
               }else{
                  nodes[1]->SetDistance(this->myMetricEvaluator->GetDistance(objects[u2], objects[j]));
               }//end if
            }//end if
            break; //end case 2

         case 3:
            if (nodes[2] == NULL) {
               pages[2] = this->myPageManager->GetNewPage();
               Header->NodeCount++;
               Parent->SetChildPageID(2, pages[2]->GetPageID());
               nodes[2] = new stMMNode(pages[2], true);
            }//end if
            if (nodes[2]->GetNumberOfEntries() == 0) {
               nodes[2]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());
               u3 = j;
            }else{
               nodes[2]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());

               if (distances[u3][j] != -1 ) {
                  nodes[2]->SetDistance(distances[u3][j]);
               }else{
                  nodes[2]->SetDistance(this->myMetricEvaluator->GetDistance(objects[u3], objects[j]));
               }//end if
            }//end if
            break; //end case 3

         case 4:
            if (nodes[3] == NULL) {
               pages[3] = this->myPageManager->GetNewPage();
               Header->NodeCount++;
               Parent->SetChildPageID(3, pages[3]->GetPageID());
               nodes[3] = new stMMNode(pages[3], true);
            }//end if
            if (nodes[3]->GetNumberOfEntries() == 0) {
                nodes[3]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());
                u4 = j;
            }else{
               nodes[3]->AddEntry(objects[j]->GetSerializedSize(), objects[j]->Serialize());

               if (distances[u4][j] != -1 ) {
                  nodes[3]->SetDistance(distances[u4][j]);
               }else{
                  nodes[3]->SetDistance(this->myMetricEvaluator->GetDistance(objects[u4], objects[j]));
               }//end if
            }//end if
            break; //end case 4
      }//end switch
   }//end for

   for (idx = 0; idx < NUMBEROFREGIONS; idx++) {
      // Dispose empty nodes.
      if (nodes[idx] != NULL) {
         if (nodes[idx]->GetNumberOfEntries() == 0) {
            this->myPageManager->DisposePage(pages[idx]);
            Header->NodeCount--;
            Parent->SetChildPageID(idx, 0);
         }//end if
      }//end if
      
      // Clean allocated nodes.
      if (nodes[idx] != NULL) {
         delete nodes[idx];
      }//end if
   }//end for
}//end stMMTree<ObjectType, EvaluatorType>::Realocate

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
bool stMMTree<ObjectType, EvaluatorType>::ExaustiveBalance(
                       stMMNode * Parent, stMMNode * & curr, tObject * obj) {

   tObject * objects[NUMBEROFUNKNOWN];
   int regions[NUMBEROFUNKNOWN];
   int numObjs;
   bool ret = false;

   if ((curr->GetNumberOfEntries()==2) && !HaveChild(Parent->GetChildPageID(0)) &&
       !HaveChild(Parent->GetChildPageID(1)) && !HaveChild(Parent->GetChildPageID(2)) &&
       !HaveChild(Parent->GetChildPageID(3))) {
       
      numObjs = CountObjs(Parent);

      if ((numObjs < 8) && (numObjs > 3)) {
         //copy the objects in the vector "objects"
         numObjs = Associate(Parent, objects);
         objects[numObjs] = obj->Clone();

         //try to distribute the elements
         if (Distribute(objects, numObjs, regions)) {
            // Got a better distribution, alocate the elements now,
            // ponting "curr" to the node with 1 element or empty, to insert
            // the new one.
            Realocate(objects, numObjs, Parent, regions);
            ret = true;
         }//end if

         for (int i = 0; i <= numObjs; i++) {
            delete objects[i];
         }//end if
      }//end if
   }//end if
   return ret;
}//end stMMTree<ObjectType, EvaluatorType>::ExaustiveBalance

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMMTree<ObjectType, EvaluatorType>::RangeQuery(
                              tObject * sample, double range) {
   tResult * result;

   if (Header->Root == 0) {
      return NULL;
   }else{
      // Create result
      result = new tResult();
      result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);
      //calls the routine to search the nodes recursively
      this->RangeQuery(sample, range, result, Header->Root);
      return result;
   }//end if
}//end stMMTree<ObjectType><EvaluatorType>::RangeQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMMTree<ObjectType, EvaluatorType>::RangeQuery(tObject * sample,
                       double range, tResult * result, u_int32_t page) {

   stPage * currPage;
   stMMNode * currNode;
   tObject tmp;
   double distance1, distance2;

   currPage = this->myPageManager->GetPage(page);
   currNode = new stMMNode(currPage);

   // Rebuild the first object
   tmp.Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
   // Evaluate the distance
   distance1 = this->myMetricEvaluator->GetDistance(&tmp, sample);
   //verify the range radius
   if (distance1 <= range) {
      //Add to result
      result->AddPair(tmp.Clone(), distance1);
   }//end if

   if (currNode->GetNumberOfEntries() == 2) {
      //Have two objects

      // Rebuild the second object
      tmp.Unserialize(currNode->GetObject(1), currNode->GetObjectSize(1));

      // Evaluate the distance
      distance2 = this->myMetricEvaluator->GetDistance(&tmp, sample);

      //verify the range radius
      if (distance2 <= range) {
         //Add to result
         result->AddPair(tmp.Clone(), distance2);
      }//end if

      //verify if needs to visit de child nodes

      //intercept region 1 ?
      if ((distance2 < range + currNode->GetDistance()) &&
          (distance1 < range + currNode->GetDistance())) {
         if (currNode->GetChildPageID(0) > 0) {
            RangeQuery(sample, range, result, currNode->GetChildPageID(0));
         }//end if
      }//end if
      //intercept region 2 ?
      if ((distance1 < range + currNode->GetDistance()) &&
          (distance2 + range >= currNode->GetDistance())) {
         if (currNode->GetChildPageID(1) > 0) {
            RangeQuery(sample, range, result, currNode->GetChildPageID(1));
         }//end if
      }//end if
      //intercept region 3 ?
      if ((distance1 + range >= currNode->GetDistance()) &&
          (distance2 < range + currNode->GetDistance())) {
         if (currNode->GetChildPageID(2) > 0) {
            RangeQuery(sample, range, result, currNode->GetChildPageID(2));
         }//end if
      }//end if
      //intercept region 4 ?
      if ((distance2 + range >= currNode->GetDistance()) &&
          (distance1 + range >= currNode->GetDistance())) {
         if (currNode->GetChildPageID(3) > 0) {
            RangeQuery(sample, range, result, currNode->GetChildPageID(3));
         }//end if
      }//end if
   }//end if

   //clean
   delete currNode;
   this->myPageManager->ReleasePage(currPage);
}//end RangeQuery 

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMMTree<ObjectType, EvaluatorType>::NearestQuery(
                     tObject * sample, u_int32_t k, bool tie) {
   tResult * result;

   if (Header->Root == 0) {
      return NULL;
   }else{
      // Create result
      result = new tResult();
      result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

      //calls the routine to search the nodes recursively
      NearestQuery(sample, k, result, Header->Root, tie);

      return result;
   }//end if
}//end NearestQuery

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMMTree<ObjectType, EvaluatorType>::NearestQuery(tObject * sample,
                       u_int32_t k, tResult *result, u_int32_t page, bool tie) {

   stPage * currPage;
   stMMNode * currNode;
   tObject tmp;
   double distance1, distance2;

   if (page > 0) {
      currPage = this->myPageManager->GetPage(page);
      currNode = new stMMNode(currPage);

      // Rebuild the first object
      tmp.Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
      // Evaluate the distance
      distance1 = this->myMetricEvaluator->GetDistance(&tmp, sample);

      //Add to result
      if (result->GetNumOfEntries() < k) {
         //Add to result
         result->AddPair(tmp.Clone(), distance1);
      }else{
         if (distance1 < result->GetMaximumDistance()) {
            result->AddPair(tmp.Clone(), distance1);
            result->Cut(k);
         }//end if
      }//end if

      if (currNode->GetNumberOfEntries() == 2) {
         //Have two objects

         // Rebuild the second object
         tmp.Unserialize(currNode->GetObject(1), currNode->GetObjectSize(1));

         // Evaluate the distance
         distance2 = this->myMetricEvaluator->GetDistance(&tmp, sample);

         //verify the range radius
         if (result->GetNumOfEntries() < k) {
            //Add to result
            result->AddPair(tmp.Clone(), distance2);
         }else{
            if (distance1 < result->GetMaximumDistance()) {
               result->AddPair(tmp.Clone(), distance2);
               result->Cut(k);
            }//end if
         }//end if
 
         // verify if needs to visit the child nodes
         switch (this->Header->NearestMethod) {
            case stMMTree::nmNORMAL :
               if ((distance2 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                   (distance1 < result->GetMaximumDistance() + currNode->GetDistance())) {
                  NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
               }//end if
               if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                   (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                  NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
               }//end if
               if ((distance2 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                   (distance1 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                  NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
               }//end if
               if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                   (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                  NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
               }//end if
               break; //end stMMTree::nmNORMAL

            case stMMTree::nmGUIDED :
               if ((distance1 < currNode->GetDistance()) && (distance2 < currNode->GetDistance())) {
                  // point query in region 1
                  NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);

                  if (distance1 <= distance2) {
                     // next is the region 2, verifying if intercepts
                     if ((distance2 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                         (distance1 < result->GetMaximumDistance() + currNode->GetDistance())) {
                        NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
                     }//end if
                     if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                         (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                        NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
                     }//end if
                     if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                         (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                        NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
                     }//end if
                  }else{
                     // next is the region 3
                     if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                         (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                        NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
                     }//end if
                     if ((distance2 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                         (distance1 < result->GetMaximumDistance() + currNode->GetDistance())) {
                        NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
                     }//end if
                     if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                         (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                        NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
                     }//end if
                  }//end if
               }else{
                  if ((distance1 < currNode->GetDistance()) && (distance2 >= currNode->GetDistance())) {
                     // point query in region 2
                     NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);

                     if (distance2 - currNode->GetDistance() < currNode->GetDistance() - distance1) {
                        //next is the region 1
                        if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                            (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                           NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
                        }//end if
                        if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                            (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                           NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
                        }//end if
                        if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                            (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                           NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
                        }//end if
                     }else{
                        //next is the region 4
                        if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                            (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                           NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
                        }//end if
                        if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                            (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                            NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
                        }//end if
                        if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                            (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                           NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
                        }//end if
                     }//end if
                  }else{
                     if ((distance1 >= currNode->GetDistance()) && (distance2 < currNode->GetDistance())) {
                        //point query in region 3
                        NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);

                        if (distance1 - currNode->GetDistance() < currNode->GetDistance() - distance2) {
                           //next is the region 1
                           if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
                           }//end if
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
                           }//end if
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
                           }//end if
                        }else{
                           //next is the region 4
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 + result->GetMaximumDistance() >= currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);
                           }//end if
                           if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
                           }//end if
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
                           }//end if
                        }//end if
                     }else{
                        //point query in region 4
                        this->NearestQuery(sample, k, result, currNode->GetChildPageID(3), tie);

                        if (distance1 <= distance2) {
                           //next is the region 2
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
                           }//end if
                           if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
                           }//end if
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
                           }//end if
                        }else{
                           //next is the region 3
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(2), tie);
                           }//end if
                           if ((distance1 < result->GetMaximumDistance() + currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(0), tie);
                           }//end if
                           if ((distance1 + result->GetMaximumDistance() >= currNode->GetDistance()) &&
                               (distance2 < result->GetMaximumDistance() + currNode->GetDistance())) {
                              NearestQuery(sample, k, result, currNode->GetChildPageID(1), tie);
                           }//end if
                        }//end if
                     }//end if
                  }//end if
               }//end if
               break; // end stMMTree::nmGUIDED
         }//end switch
      }//end if have 2 objs

      //clean
      delete currNode;
      this->myPageManager->ReleasePage(currPage);
   }//end if
}//end

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMMTree<ObjectType, EvaluatorType>::GetHeights(int &min, int &max) {

   if (Header->Root > 0) {
      min = MAXINT;
      max = 0;
      MinMax(Header->Root, 0, min, max);
   }//end if
}//end stMMTree<ObjectType, EvaluatorType>::GetHeights

//------------------------------------------------------------------------------
template <class ObjectType, class EvaluatorType>
void stMMTree<ObjectType, EvaluatorType>::MinMax(u_int32_t page, int alt,
                                                 int & altMin, int & altMax) {
   register int i; 
   stPage * currPage;
   stMMNode * currNode;

   currPage = this->myPageManager->GetPage(page);
   currNode = new stMMNode(currPage);

   for (i = 0; i < NUMBEROFREGIONS; i++) {
      if (currNode->GetChildPageID(i) <= 0) {
         if (alt > altMax) {
            altMax = alt;
         }//end if
         if (alt < altMin) {
            altMin = alt;
         }//end if
      }else{
          MinMax(currNode->GetChildPageID(i), alt+1, altMin, altMax);
      }//end if
   }//end for
   
   delete currNode;
}//end stMMTree<ObjectType, EvaluatorType>::MinMax
