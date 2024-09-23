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

#include "stOmni.h"

/**
 * @file
 *
 * This file is the implementation of stOmni methods.
 *
 * @version 1.0
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 */

//-----------------------------------------------------------------------------
// class stOmniOrigNode
//-----------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stOmniOrigNode<ObjectType, EvaluatorType>::stOmniOrigNode(stPage * page, bool create) {

    this->Page = page;

    // Will I create it ?
    if (create) {
        Page->Clear();
    }//end if

    // Set elements
    this->Header = (stNodeHeader *) Page->GetData();
    this->Entries = (u_int32_t *) (Page->GetData() + sizeof (stNodeHeader));
}//end stOmniOrigNode::stOmniOrigNode

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
int stOmniOrigNode<ObjectType, EvaluatorType>::AddEntry(u_int32_t size, const unsigned char * object, double maxD) {
    u_int32_t totalsize;
    u_int32_t offs;
    double max;
    totalsize = size + sizeof (u_int32_t);
    if (totalsize <= GetFree()) {
        // Object offset
        if (Header->Occupation == 0) {
            offs = Page->GetPageSize() - size;
        } else {
            offs = Entries[Header->Occupation - 1] - size;
        }//end if

        // Write object
        Page->Write((unsigned char*) object, size, offs);

        // Update entry
        Entries[Header->Occupation] = offs;

        // Update header
        Header->Occupation++;
        if (Header->Max < maxD) {
            Header->Max = maxD;
        }
        return Header->Occupation - 1;
    } else {
        return -1;
    }//end if
}//end stOmniOrigNode::AddEntry

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
const unsigned char * stOmniOrigNode<ObjectType, EvaluatorType>::GetObject(int idx) {

    return Page->GetData() + Entries[idx];
}//end stOmniOrigNode::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
u_int32_t stOmniOrigNode<ObjectType, EvaluatorType>::GetObjectSize(int idx) {

    if (idx == 0) {
        return Page->GetPageSize() - Entries[0];
    } else {
        return Entries[idx - 1] - Entries[idx];
    }//end if
}//end stOmniOrigNode::GetObjectSize

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniOrigNode<ObjectType, EvaluatorType>::RemoveEntry(u_int32_t idx) {
    u_int32_t i;
    u_int32_t lastID;
    u_int32_t rObjSize;

    // Programmer's note: This procedure is simple but tricky! See the
    // stDBMNode structure documentation for more details.

#ifdef __stDEBUG__
    if (idx >= (int) GetNumberOfEntries()) {
        // Oops! This id doesn't exists.
        throw range_error("Invalid idx!");
    }//end if
#endif //__stDEBUG__

    // Let's remove
    lastID = Header->Occupation - 1; // The id of the last object. This
    // value will be very useful.
    // Do I need to move something ?
    if (lastID != idx) {
        // Yes, I do. Save the removed object size
        rObjSize = GetObjectSize(idx);

        // Let's move objects first. We will use memmove() from stdlib because
        // it handles the overlap between src and dst. Remember that src is the
        // offset of the last object and the dst is the offset of the last
        // object plus removed object size.
        memmove(Page->GetData() + Entries[lastID] + rObjSize,
                Page->GetData() + Entries[lastID],
                Entries[idx] - Entries[lastID]);
        // Let's move entries...
        for (i = idx; i < lastID; i++) {
            // Copy all fields with memcpy (it's faster than field copy).
            memcpy(Entries + i, Entries + i + 1, sizeof (u_int32_t));
            // Update offset by adding the removed object size to it. It will
            // reflect the previous move operation.
            Entries[i] += rObjSize;
        }//end for
    }//end if

    // Update counter...
    Header->Occupation--;
}//end stOmniOrigNode::RemoveEntry

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
u_int32_t stOmniOrigNode<ObjectType, EvaluatorType>::GetFree() {

    if (Header->Occupation == 0) {
        return Page->GetPageSize() - sizeof (stNodeHeader);
    } else {
        return Page->GetPageSize() - sizeof (stNodeHeader) -
                (sizeof (u_int32_t) * Header->Occupation) -
                (Page->GetPageSize() - Entries[Header->Occupation - 1]);
    }//end if
}//end stOmniOrigNode::GetFree




//==============================================================================
// Class stOmniLogicNode
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stOmniLogicNode<ObjectType, EvaluatorType>::stOmniLogicNode(stPageManager * pageman, EvaluatorType * metricEvaluator) {
    int i;

    myPageManager = pageman;
    myMetricEvaluator = metricEvaluator;

    if (this->myPageManager->IsEmpty()) {
        // Create it
        this->Create();
        // Default values
        Header->ObjectCount = 0;
        Header->NodeCount = 0;
        Header->MaxOccupation = 0;
    } else {
        // Use it
        this->LoadHeader();
    }//end if

    //Entries = new stIndexEntry[70000]; //@warning, should not be static
    //Infos = new stInfoEntry[70000];

}//end stOmniLogicNode<ObjectType, EvaluatorType>::stOmniLogicNode

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stOmniLogicNode<ObjectType, EvaluatorType>::~stOmniLogicNode() {
    int i;

    Entries.clear();
    Infos.clear();
}//end stOmniLogicNode<ObjectType, EvaluatorType>::~stOmniLogicNode

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
int stOmniLogicNode<ObjectType, EvaluatorType>::AddEntry(tObject * obj) {
    // The object will be added in the first page.
    // When it is full, it will create a new page and link
    // it in the begining of the list.
    //stPage * currPage;
    //stDummyNode * currNode;
    bool overflow;
    u_int32_t nextPageID;
    int id;
    double MaxD;
    tObject * Object;
    stIndexEntry Entry;
    Object = new tObject;

    // Does it fit ?
    if (obj->GetSerializedSize() > this->myPageManager->GetMinimumPageSize() - 20) {
        return false;
    }//end if

    // Adding object
    if (this->GetRoot() == 0) {
        overflow = true;
        nextPageID = 0;
    } else {
        // Get node
        currPage = this->myPageManager->GetPage(this->GetRoot());
        currNode = new tOrigNode(currPage);
        Object->Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
        MaxD = myMetricEvaluator->GetDistance(Object, obj);
        // Try to add
        id = currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize(), MaxD);
        if (id >= 0) {
            // I was able to add.
            this->myPageManager->WritePage(currPage);
            overflow = false;

            //Entries[Header->ObjectCount].ObjPageID = currPage->GetPageID();
            //Entries[Header->ObjectCount].ObjIdx = id;
            //Entries[Header->ObjectCount].IsPivot = false;
            Entry.ObjPageID = currPage->GetPageID();
            Entry.ObjIdx = id;
            Entry.IsPivot = false;

            // update the maximum number of entries.
            //this->SetMaxOccupation(currNode->GetNumberOfEntries());
        } else {
            // Oops! Overflow!
            overflow = true;
            nextPageID = currPage->GetPageID();
        }//end if

        // Clear the mess
        delete currNode;
        currNode = NULL;
        this->myPageManager->ReleasePage(currPage);
    }//end if

    // Manage overflows
    if (overflow) {
        // Oops! New root required !
        currPage = this->myPageManager->GetNewPage();
        currNode = new tOrigNode(currPage, true);
        currNode->SetNextNode(nextPageID);
        // Update the number of nodes.
        Header->NodeCount++;

        // I'll add it here
        id = currNode->AddEntry(obj->GetSerializedSize(), obj->Serialize());

        // Write the new node
        this->myPageManager->WritePage(currPage);

        // Update "tree" state
        this->SetRoot(currPage->GetPageID());
        WriteHeader();

        Entry.ObjPageID = currPage->GetPageID();
        Entry.ObjIdx = id;
        Entry.IsPivot = false;

        // Clear the mess again
        delete currNode;
        currNode = NULL;
        this->myPageManager->ReleasePage(currPage);
    }//end overflow

    Entries.insert(Entries.end(), Entry);
    //Update the number of objects.
    UpdateObjectCounter(1);

    // Write Header!
    WriteHeader();

    return true;
}//end stOmniLogicNode<ObjectType><EvaluatorType>::Add

template <class ObjectType, class EvaluatorType>
ObjectType * stOmniLogicNode<ObjectType, EvaluatorType>::GetObject(u_int32_t index) {

    ObjectType * Object;

    Object = new ObjectType();

    if (currNode == NULL) {
        currPage = this->myPageManager->GetPage(Entries[index].ObjPageID);
        currNode = new tOrigNode(currPage);
    } else if (currNode->GetPageID() != Entries[index].ObjPageID) {
        delete currNode;
        currNode = NULL;
        this->myPageManager->ReleasePage(currPage);
        currPage = this->myPageManager->GetPage(Entries[index].ObjPageID);
        currNode = new tOrigNode(currPage);
    }

    Object->Unserialize(currNode->GetObject(Entries[index].ObjIdx), currNode->GetObjectSize(Entries[index].ObjIdx));

    return Object;
}//end stOmni<ObjectType, EvaluatorType>::GetObject


//==============================================================================
// Class stOmniPivot
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stOmniPivot<ObjectType, EvaluatorType>::stOmniPivot(u_int32_t nFocus, EvaluatorType * metricEvaluator) {

    // Number of Focus
    NumFocus = nFocus;

    fociBase = new stOmniPivotEntry[NumFocus];

    /*
     *  for fastmap.
     */
#ifdef FASTMAPER
    for (int i = 0; i < NumFocus; i++) {
        fociBase[i].coords = new double[NumFocus / 2];
        for (int j = 0; j < NumFocus / 2; j++) {
            fociBase[i].coords[j] = 0;
        }
    }

    PivotDist = new double[NumFocus / 2];
    PivotDist2 = new double[NumFocus / 2];

#endif

    // To calculate distances
    myMetricEvaluator = metricEvaluator;

}//end stOmniPivot::stOmniPivot

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::UpdatePivotMaps() {
    //int axis;            // Current axis
    int pid; // Current pivot id. It's used to avoid pid = axis * 2.
    //int i;               // A generic counter

    // This method is a quite similar to ChoosePivots(). It does not need to
    // perform the FindPivots() method and works direct over the pivot array, so
    // it must not be updated everytime.
    pid = 0;
    cout << "\n pdist: ";
    for (int axis = 0; axis < NumFocus / 2; axis++) {
        // Get the distance between pivots
        PivotDist2[axis] = GetFMDistance2(fociBase[pid].Object, fociBase[pid].coords,
                fociBase[pid + 1].Object, fociBase[pid + 1].coords, axis);

        if (PivotDist2[axis] < 0) {
            PivotDist2[axis] = PivotDist2[axis]*(-1);
        }

        PivotDist[axis] = sqrt(PivotDist2[axis]);

        cout << PivotDist[axis] << " ";
        // Map all pivots
        for (int i = 0; i < NumFocus; i++) {
            if (i == pid) {
                // I'm the pivot 1
                fociBase[i].coords[axis] = 0;
                cout << " a ";
            } else if (i == pid + 1) {
                // I'm the pivot 2
                fociBase[i].coords[axis] = PivotDist[axis];
                cout << " b ";
            } else {
                // I'm someone else.
                fociBase[i].coords[axis] = FMProject(fociBase[i].Object, fociBase[i].coords, axis);
                cout << "i: " << i << " axis: " << axis << " " << fociBase[i].coords[axis] << endl;
            }//end if
        }//end for

        // Next pid
        pid = pid + 2;
    }//end for
    cout << endl;
}//end stFastMapper<ObjectType, EvaluatorType>::UpdatePivotMaps

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::FindPivot(tLogicNode * logicNode, u_int32_t numObject, int type, int part) {

    //find the pivots using HFFastMap
    if (type == 0) {
        execHFFastMap(logicNode, numObject);
    } else if (type == 1) {
        execHFFastMapPartial(logicNode, numObject, part);
    } else if (type == 2) {
        execHFFastMapCluster(logicNode, numObject, part);
    } else if (type == 3) {
        execRandomPivots(logicNode, numObject);
    } else if (type == 4) {
        execRandomCluster(logicNode, numObject, part);
    } else {
        cout << "Find Pivot type not defined";
    }

#ifdef FASTMAPER
    UpdatePivotMaps();
#endif

}//end stOmniPivot::FindPivot

//------------------------------------------------------------------------------
// execute the HF algorithm to find the foci base based on the FastMap Algorithm
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execHFFastMap(tLogicNode * logicNode, u_int32_t numObject) {
    double edge, error, bestError;
    long i, nFound, bestCandidate1, bestCandidate2;
    int steps;

    // set the first object as the base object to find the first focus
    nFound = 0;
    bestCandidate1 = 0;
    bestCandidate2 = 0;
    error = 0;
    bestError = 0;
    tObject *tmp, *best1, *best2;

    for (steps = 0; steps < 5; steps++) {
        // traverse dataset and get first foci
        best1 = logicNode->GetObject(bestCandidate1);
        for (i = 0; i < numObject; i++) {
            // compare each object with actual and keep if it is furthest than the
            // actual furthest
            tmp = logicNode->GetObject(i);
            error = myMetricEvaluator->GetDistance(tmp, best1);
            if (error > bestError) {
                bestCandidate2 = i;
                bestError = error;
            }//end if
            delete tmp;
        }//end for
        // traverse dataset and get first foci
        best2 = logicNode->GetObject(bestCandidate2);
        for (i = 0; i < numObject; i++) {
            // compare first object with actual and keep if it is furthest than the
            // actual furthest
            tmp = logicNode->GetObject(i);
            error = myMetricEvaluator->GetDistance(tmp, best2);
            if (error > bestError) {
                bestCandidate1 = i;
                bestError = error;
            }//end if
            delete tmp;
        }//end for
        delete best1;
        delete best2;
    }//end for
    cout << "\n Pivots: ";
    //@warning I really need this if state here?
    if (nFound < NumFocus) {
        // insert first focus and set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate1);
        fociBase[nFound].idx = bestCandidate1;
        logicNode->SetIsPivot(bestCandidate1);
        nFound++;
        cout << bestCandidate1;
        // insert second focus nd set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate2);
        fociBase[nFound].idx = bestCandidate2;
        logicNode->SetIsPivot(bestCandidate2);
        nFound++;
        cout << " " << bestCandidate2;
        // find the remaining focus
        edge = myMetricEvaluator->GetDistance(fociBase[0].Object, fociBase[1].Object);
        execHFRemainingFastMap(logicNode, numObject, edge, nFound);
    }//end if
}//end execHFFastMap

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execHFRemainingFastMap(tLogicNode * logicNode, u_int32_t numObject,
double edge, int nFound) {
    double error, bestError, delta;
    long i, j, bestCandidate;
    tObject *tmp;
    // find the remaining foci
    while (nFound < NumFocus) {
        bestError = (double) MAXINT;
        bestCandidate = 0;
        for (i = 0; i < numObject; i++) {
            if (logicNode->GetIsPivot(i) == false) {
                tmp = logicNode->GetObject(i);
                error = 0;
                // calculates the error
                for (j = 0; j < nFound; j++) {
                    delta = edge - myMetricEvaluator->GetDistance(fociBase[j].Object, tmp);
                    if (delta < 0) {
                        delta *= -1;
                    }//end if
                    error += delta;
                }//end for

                // compares with actual best and keep if it is better
                if (error < bestError) {
                    bestCandidate = i;
                    bestError = error;
                }
                delete tmp;
            }
        }

        // insert first focus and set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate);
        fociBase[nFound].idx = bestCandidate;
        logicNode->SetIsPivot(bestCandidate);
        cout << " " << bestCandidate;
        nFound++;
    }
    //cout << endl;
}//end execHFRemainingFastMap

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execHFFastMapPartial(tLogicNode * logicNode, u_int32_t numObject, int part) {

    double edge, error, bestError;
    long i, nFound, bestCandidate1, bestCandidate2;
    int steps;

    // set the first object as the base object to find the first focus
    nFound = 0;
    bestCandidate1 = 0;
    bestCandidate2 = 0;
    error = 0;
    bestError = 0;
    tObject *tmp, *best1, *best2;

    for (steps = 0; steps < 5; steps++) {
        // traverse dataset and get first foci
        best1 = logicNode->GetObject(bestCandidate1);
        for (i = 0; i < numObject; i++) {
            // compare each object with actual and keep if it is furthest than the
            // actual furthest
            tmp = logicNode->GetObject(i);
            error = myMetricEvaluator->GetPartialDistance(tmp, best1, part);
            if (error > bestError) {
                bestCandidate2 = i;
                bestError = error;
            }//end if
            delete tmp;
        }//end for
        // traverse dataset and get first foci
        best2 = logicNode->GetObject(bestCandidate2);
        for (i = 0; i < numObject; i++) {
            // compare first object with actual and keep if it is furthest than the
            // actual furthest
            tmp = logicNode->GetObject(i);
            error = myMetricEvaluator->GetPartialDistance(tmp, best2, part);
            if (error > bestError) {
                bestCandidate1 = i;
                bestError = error;
            }//end if
            delete tmp;
        }//end for
        delete best1;
        delete best2;
    }//end for
    //cout << "\n Pivots: ";
    //@warning I really need this if state here?
    if (nFound < NumFocus) {
        // insert first focus and set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate1);
        fociBase[nFound].idx = bestCandidate1;
        logicNode->SetIsPivot(bestCandidate1);
        nFound++;
        //cout << bestCandidate1;
        // insert second focus nd set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate2);
        fociBase[nFound].idx = bestCandidate2;
        logicNode->SetIsPivot(bestCandidate2);
        nFound++;
        //cout << " " << bestCandidate2;
        // find the remaining focus
        edge = myMetricEvaluator->GetPartialDistance(fociBase[0].Object, fociBase[1].Object, part);
        execHFRemainingFastMapPartial(logicNode, numObject, edge, nFound, part);
    }//end if
}//end execHFFastMap

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execHFRemainingFastMapPartial(tLogicNode * logicNode, u_int32_t numObject,
double edge, int nFound, int part) {
    double error, bestError, delta;
    long i, j, bestCandidate;
    tObject *tmp;
    // find the remaining foci
    while (nFound < NumFocus) {
        bestError = (double) MAXINT;
        bestCandidate = 0;
        for (i = 0; i < numObject; i++) {
            if (logicNode->GetIsPivot(i) == false) {
                tmp = logicNode->GetObject(i);
                error = 0;
                // calculates the error
                for (j = 0; j < nFound; j++) {
                    delta = edge - myMetricEvaluator->GetPartialDistance(fociBase[j].Object, tmp, part);
                    if (delta < 0) {
                        delta *= -1;
                    }//end if
                    error += delta;
                }//end for

                // compares with actual best and keep if it is better
                if (error < bestError) {
                    bestCandidate = i;
                    bestError = error;
                }
                delete tmp;
            }
        }

        // insert first focus and set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate);
        fociBase[nFound].idx = bestCandidate;
        logicNode->SetIsPivot(bestCandidate);
        //cout << " " << bestCandidate;
        nFound++;
    }
    //cout << endl;
}//end execHFRemainingFastMap

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execHFFastMapCluster(tLogicNode * logicNode, u_int32_t numObject, int part) {

    double edge, error, bestError;
    long i, nFound, bestCandidate1, bestCandidate2;
    int steps;

    // set the first object as the base object to find the first focus
    nFound = 0;
    bestCandidate1 = 0;
    bestCandidate2 = 0;
    error = 0;
    bestError = 0;
    tObject *tmp, *best1, *best2;

    i = 0;
    bool go = true;
    while (i < numObject && go) {
        if (logicNode->GetCluster(i) == part){
            bestCandidate1 = i;
            go = false;
        }
        i++;
    }


    for (steps = 0; steps < 5; steps++) {
        // traverse dataset and get first foci
        //@todo, get the first best1 as someone that belongs to the cluster

        best1 = logicNode->GetObject(bestCandidate1);

        for (i = 0; i < numObject; i++) {
            // compare each object with actual and keep if it is furthest than the
            // actual furthest
            if (logicNode->GetCluster(i) == part && logicNode->GetIsPivot(i) == false) {
                tmp = logicNode->GetObject(i);
                error = myMetricEvaluator->GetDistance(tmp, best1);
                if (error > bestError) {
                    bestCandidate2 = i;
                    bestError = error;
                }//end if
                delete tmp;
            }
        }//end for
        // traverse dataset and get first foci
        best2 = logicNode->GetObject(bestCandidate2);
        for (i = 0; i < numObject; i++) {
            // compare first object with actual and keep if it is furthest than the
            // actual furthest
            if (logicNode->GetCluster(i) == part && logicNode->GetIsPivot(i) == false) {
                tmp = logicNode->GetObject(i);
                error = myMetricEvaluator->GetDistance(tmp, best2);
                if (error > bestError) {
                    bestCandidate1 = i;
                    bestError = error;
                }//end if
                delete tmp;
            }
        }//end for
        delete best1;
        delete best2;
    }//end for
    cout << "\n Pivots: ";
    //@warning I really need this if state here?
    if (nFound < NumFocus) {
        // insert first focus and set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate1);
        fociBase[nFound].idx = bestCandidate1;
        logicNode->SetIsPivot(bestCandidate1);
        nFound++;
        cout << bestCandidate1;
        // insert second focus nd set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate2);
        fociBase[nFound].idx = bestCandidate2;
        logicNode->SetIsPivot(bestCandidate2);
        nFound++;
        cout << " " << bestCandidate2;
        // find the remaining focus
        edge = myMetricEvaluator->GetPartialDistance(fociBase[0].Object, fociBase[1].Object, part);
        execHFRemainingFastMapCluster(logicNode, numObject, edge, nFound, part);
    }//end if
}//end execHFFastMap

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execHFRemainingFastMapCluster(tLogicNode * logicNode, u_int32_t numObject,
double edge, int nFound, int part) {
    double error, bestError, delta;
    long i, j, bestCandidate;
    tObject *tmp;
    // find the remaining foci
    while (nFound < NumFocus) {
        bestError = (double) MAXINT;
        bestCandidate = 0;
        for (i = 0; i < numObject; i++) {
            if (logicNode->GetCluster(i) == part && logicNode->GetIsPivot(i) == false) {
                tmp = logicNode->GetObject(i);
                error = 0;
                // calculates the error
                for (j = 0; j < nFound; j++) {
                    delta = edge - myMetricEvaluator->GetDistance(fociBase[j].Object, tmp);
                    if (delta < 0) {
                        delta *= -1;
                    }//end if
                    error += delta;
                }//end for

                // compares with actual best and keep if it is better
                if (error < bestError) {
                    bestCandidate = i;
                    bestError = error;
                }
                delete tmp;
            }
        }

        // insert first focus and set it as pivot
        fociBase[nFound].Object = logicNode->GetObject(bestCandidate);
        fociBase[nFound].idx = bestCandidate;
        logicNode->SetIsPivot(bestCandidate);
        cout << " " << bestCandidate;
        nFound++;
    }
    //cout << endl;
}//end execHFRemainingFastMap

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execRandomPivots(tLogicNode * logicNode, u_int32_t numObject) {

    int nFound = 0;
    int iR;
    srand(20);
    //srand(125);
    // find the remaining foci
    cout << " Pt: " << flush;
    while (nFound < NumFocus) {
        //bestError = (double) MAXINT;
        //bestCandidate = 0;
        iR = rand() % numObject;
        //for(i = 0; i < numObject; i++) {
        if (logicNode->GetIsPivot(iR) == false) {
            fociBase[nFound].Object = logicNode->GetObject(iR);
            fociBase[nFound].idx = iR;
            logicNode->SetIsPivot(iR);
            nFound++;
            cout << iR << " ";
        }//end if

    }//end while
}//end execRandomPivots

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::execRandomCluster(tLogicNode * logicNode, u_int32_t numObject, int part) {

    int nFound = 0;
    int iR;
    srand(20);
    //srand(125);
    // find the remaining foci
    cout << " Pt: " << flush;
    while (nFound < NumFocus) {
        //bestError = (double) MAXINT;
        //bestCandidate = 0;
        iR = rand() % numObject;
        //for(i = 0; i < numObject; i++) {
        if (logicNode->GetCluster(iR) == part && logicNode->GetIsPivot(iR) == false) {
            fociBase[nFound].Object = logicNode->GetObject(iR);
            fociBase[nFound].idx = iR;
            logicNode->SetIsPivot(iR);
            nFound++;
            cout << iR << " ";
        }//end if

    }//end while
}//end execRandomPivots

//---------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double stOmniPivot<ObjectType, EvaluatorType>::BuildFieldDistance(
ObjectType * object, double * fieldDistance) {

    double sum = 0;

#ifdef FASTMAPER
    for (int k = 0; k < NumFocus / 2; k++) {
        fieldDistance[k] = FMProject(object, fieldDistance, k);
    }//end for
#else
    for (int k = 0; k < NumFocus; k++) {
        fieldDistance[k] = myMetricEvaluator->GetDistance(fociBase[k].Object, object);
        sum += fieldDistance[k];
        //if (fieldDistance[k] > sum)
        //    sum = fieldDistance[k];
        //myMetricEvaluator->GetPartialDistance(fociBase[k].Object, object, 1);
    }//end for
#endif

    return sum;

}//end stOmniPivot::BuildFieldDistance

//---------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmniPivot<ObjectType, EvaluatorType>::BuildFieldDistancePartial(
ObjectType * object, double * fieldDistance, int part) {

    for (int k = 0; k < NumFocus; k++) {
        fieldDistance[k] = myMetricEvaluator->GetPartialDistance(fociBase[k].Object, object, part);
    }//end for


}//end stOmniPivot::BuildFieldDistance

//---------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double stOmniPivot<ObjectType, EvaluatorType>::FMProject(
ObjectType * obj, double * map, int axis) {
    int pid;

    // This is the core of everything.
    pid = axis * 2;


    // Distances


    return (GetFMDistance2(obj, map, fociBase[pid].Object,
            fociBase[pid].coords, axis) + // d(pa, o)^2

            (PivotDist2[axis]) - // d(pa, pb)^2

            GetFMDistance2(obj, map, fociBase[pid + 1].Object, // d(pb, o)
            fociBase[pid + 1].coords, axis)) /
            (2 * PivotDist[axis]); // 2d(pa, pb)
}//end stFastMapper<ObjectType, EvaluatorType>::Project

//---------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double stOmniPivot<ObjectType, EvaluatorType>::GetFMDistance2(ObjectType * o1, double * map1, ObjectType * o2, double * map2, int axis) {

    return myMetricEvaluator->GetDistance2((ObjectType *) o1, (ObjectType *) o2) - Euclidean2(map1, map2, axis);

}//end GetFMDistance2

//==============================================================================
// Class stOmni
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stOmni<ObjectType, EvaluatorType>::stOmni(stPageManager * pageman, stPageManager * pagemanD,
u_int32_t nFocus, EvaluatorType * metricEvaluator) {

    // Number of Focus
    NumFocus = nFocus;

    myPageManager = pageman;

    myPageManagerD = pagemanD;

    myMetricEvaluator = metricEvaluator;

    myBasicMetricEvaluator = new tBasicMetricEvaluator;

    LogicNode = new tLogicNode(pageman, metricEvaluator);

    DistanceNode = new tDistanceNode(pagemanD, myBasicMetricEvaluator);

    OmniPivot = new tOmniPivot(nFocus, metricEvaluator);


}//end stOmniPivot::stOmniPivot

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
bool stOmni<ObjectType, EvaluatorType>::Add(tObject * obj) {
    //@warning eliminate this function, back compatiblity
    LogicNode->AddEntry(obj);
}//end stOmni<ObjectType><EvaluatorType>::Add

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
ObjectType * stOmni<ObjectType, EvaluatorType>::GetObject(u_int32_t index) {

    return LogicNode->GetObject(index);
}//end stOmni<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double * stOmni<ObjectType, EvaluatorType>::GetDistance(u_int32_t index) {

    return DistanceNode->GetObject(index)->GetData();
}//end stOmni<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmni<ObjectType, EvaluatorType>::BuildAllDistance() {

    tObject *tmp;
    double * FieldDistance;
    tBasicArrayObject * ObjectD;

    FieldDistance = new double[NumFocus];

    for (int i = 0; i < this->GetNumberOfObjects(); i++) {
        tmp = LogicNode->GetObject(i);
        OmniPivot->BuildFieldDistance(tmp, FieldDistance);

        //debug
        //for(int f = 0; f < NumFocus; f++){
        //  cout << FieldDistance[f] << " ";
        //}
        //cout << "\n" << flush;

        ObjectD = new tBasicArrayObject(NumFocus, FieldDistance);
        ObjectD->SetOID(i);
        DistanceNode->AddEntry(ObjectD);
        delete tmp;
    }
    delete []FieldDistance;

}//end stOmni<ObjectType, EvaluatorType>::BuildAllDistance()

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stOmni<ObjectType, EvaluatorType>::AllDistance() {

    tObject *ObjectO, *ObjO;
    tBasicArrayObject * ObjectD, *ObjD;
    double d1, d2;
    double tmp, c2 = 1, c1 = 1;

    for (int i = 0; i < this->GetNumberOfObjects(); i++) {
        ObjectO = LogicNode->GetObject(i);
        ObjectD = DistanceNode->GetObject(i);
        for (int j = 0; j < this->GetNumberOfObjects(); j++) {
            ObjO = LogicNode->GetObject(j);
            d1 = myMetricEvaluator->GetDistance(ObjectO, ObjO);
            ObjD = DistanceNode->GetObject(j);

            d2 = myBasicMetricEvaluator->GetDistance(ObjectD, ObjD);

            //cout << d1 << " " << d2 << endl;

            if (d2 > d1) {
                tmp = d2 / d1;
                if (tmp > c2)
                    c2 = tmp;
            } else {
                tmp = d1 / d2;
                if (tmp > c1)
                    c1 = tmp;
            }

            delete ObjO;
            delete ObjD;
        }
        delete ObjectO;
        delete ObjectD;
    }
    cout << "c1: " << c1 << " c2: " << c2 << " c1*c2: " << c1 * c2 << endl;

}//end stMGrid<ObjectType, EvaluatorType>::AllDistance()

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stOmni<ObjectType, EvaluatorType>::RangeQuery(
tObject * sample, double range) {
    //stPage * currPage;
    //stDummyNode * currNode;
    tResult * result;
    tObject * tmp;
    double distance;
    double *SampleD;

    SampleD = new double[NumFocus];

    // Create result
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

    OmniPivot->BuildFieldDistance(sample, SampleD);

    // Let's search
    for (int i = 0; i < LogicNode->GetNumberOfObjects(); i++) {
        tmp = LogicNode->GetObject(i);

        distance = this->myMetricEvaluator->GetDistance(tmp, sample);

        // Is it qualified ?
        if (distance <= range) {

            // Yes! I'm qualified !
            result->AddPair(tmp->Clone(), distance);
        }//end if
        delete tmp;

    }//end for
    // Return the result.

    return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stOmni<ObjectType, EvaluatorType>::NearestQuery(
tObject * sample, u_int32_t k, bool tie) {
    //stPage * currPage;
    //stDummyNode * currNode;
    tResult * result;
    tObject * tmp;
    double distance;
    double *SampleD;

    SampleD = new double[NumFocus];

    // Create result
    result = new tResult(k);
    result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

    OmniPivot->BuildFieldDistance(sample, SampleD);

    //Search at nearest cluster
    for (int i = 0; i < LogicNode->GetNumberOfObjects(); i++) {
        tmp = LogicNode->GetObject(i);

        // Evaluate distance
        distance = this->myMetricEvaluator->GetDistance(tmp, sample);

        if (result->GetNumOfEntries() < k) {
            // Unnecessary to check. Just add.
            result->AddPair(tmp, distance);
        } else {
            // Will I add ?
            if (distance <= result->GetMaximumDistance()) {
                // Yes! I'll.
                result->AddPair(tmp->Clone(), distance);
                result->Cut(k);
            }//end if
            delete tmp;
        }//end if
    }//end for

    delete SampleD;

    return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NearestQuery

//-------------------------------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stOmni<ObjectType, EvaluatorType>::RangeQueryOmniSeq(
tObject * sample, double range) {
    //stPage * currPage;
    //stDummyNode * currNode;
    tResult * result;
    tObject * tmp;
    tBasicArrayObject * ObjectD, * SampleObject;
    double distance, OmniDistance;
    double *SampleD;


    SampleD = new double[NumFocus];
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

    OmniPivot->BuildFieldDistance(sample, SampleD);
    SampleObject = new tBasicArrayObject(NumFocus, SampleD);

    for (int i = 0; i < DistanceNode->GetNumberOfObjects(); i++) {

        ObjectD = DistanceNode->GetObject(i);
        OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

        //if (OmniDistance <= (range * NumFocus)){
        if (OmniDistance <= range) {
            tmp = LogicNode->GetObject(i);
            distance = this->myMetricEvaluator->GetDistance(tmp, sample);

            if (distance <= range) {
                // Yes! I'm qualified !
                result->AddPair(tmp->Clone(), distance);
            }//end if
            delete tmp;
        }//end if
        delete ObjectD;
    }//end for
    delete SampleD;
    delete SampleObject;

    // Return the result.
    return result;
}//end stDummyTree<ObjectType><EvaluatorType>::RangeQuery

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stOmni<ObjectType, EvaluatorType>::RangeQueryOmniSeqB(tObject * sample, double range) {
    stPage * currPage;
    tOrigNode * currNode;
    tResult * result;
    tObject * tmp;
    tBasicArrayObject * ObjectD, * SampleObject;
    double distance, OmniDistance, max;
    double *SampleD;
    u_int32_t i;
    u_int32_t nextPageID;

    SampleD = new double[NumFocus];
    ObjectD = new tBasicArrayObject();
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

    OmniPivot->BuildFieldDistance(sample, SampleD);
    SampleObject = new tBasicArrayObject(NumFocus, SampleD);
    //cout << " 1 " << flush;
    // First node
    nextPageID = DistanceNode->GetRoot();
    //cout << " begin " << flush;
    // Let's search
    while (nextPageID != 0) {
        // Get node
        //cout << " 2 " << flush;
        currPage = this->myPageManagerD->GetPage(nextPageID);
        //cout << " 0.5 " << flush;
        currNode = new tOrigNode(currPage);
        //cout << " 1 " << flush;
        // Lets check all objects in this node
        if (currNode->GetNumberOfEntries() > 0) {
            currNode->GetObjectSize(0);
            max = currNode->GetMax();
            ObjectD->Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
            OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

            if (OmniDistance <= range) {
                tmp = LogicNode->GetObject(ObjectD->GetOID());
                distance = this->myMetricEvaluator->GetDistance(tmp, sample);

                if (distance <= range) {
                    // Yes! I'm qualified !
                    result->AddPair(tmp->Clone(), distance);
                }//end if
                delete tmp;
            }//end if


            if (OmniDistance <= range + max) {
                for (i = 1; i < currNode->GetNumberOfEntries(); i++) {
                    // Rebuild the object
                    currNode->GetObjectSize(i);
                    ObjectD->Unserialize(currNode->GetObject(i), currNode->GetObjectSize(i));
                    // Evaluate distance
                    OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

                    if (OmniDistance <= range) {
                        tmp = LogicNode->GetObject(ObjectD->GetOID());
                        distance = this->myMetricEvaluator->GetDistance(tmp, sample);

                        if (distance <= range) {
                            // Yes! I'm qualified !
                            result->AddPair(tmp->Clone(), distance);
                        }//end if
                        delete tmp;
                    }//end if
                    //delete ObjectD;
                }//end for
                // Next PageID...
            }
        }
        nextPageID = currNode->GetNextNode();
        // Free it all
        delete currNode;
        this->myPageManagerD->ReleasePage(currPage);
    }//end while

    delete []SampleD;
    delete SampleObject;
    delete ObjectD;

    // Return the result.
    return result;
}//end stOmni<ObjectType><EvaluatorType>::RangeQuery

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stOmni<ObjectType, EvaluatorType>::NearestQueryOmniSeq(
tObject * sample, u_int32_t k, bool tie) {
    //stPage * currPage;
    //stDummyNode * currNode;
    tResult * result;
    tObject * tmp;
    tBasicArrayObject * ObjectD, * SampleObject;
    double distance, OmniDistance;
    double *SampleD;
    //cout << "! " << flush;

    SampleD = new double[NumFocus];
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

    OmniPivot->BuildFieldDistance(sample, SampleD);
    SampleObject = new tBasicArrayObject(NumFocus, SampleD);

    for (int i = 0; i < k; i++) {
        tmp = LogicNode->GetObject(i);
        distance = this->myMetricEvaluator->GetDistance(tmp, sample);
        result->AddPair(tmp, distance);
    }//end for


    for (int i = k; i < DistanceNode->GetNumberOfObjects(); i++) {
        ObjectD = DistanceNode->GetObject(i);
        OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

        if (OmniDistance <= result->GetMaximumDistance()) {
            tmp = LogicNode->GetObject(i);
            distance = this->myMetricEvaluator->GetDistance(tmp, sample);
            if (distance <= result->GetMaximumDistance()) {
                // Yes! I'll.
                result->AddPair(tmp, distance);
                result->Cut(k);
            } else {
                delete tmp;
            }
        }//end if
        delete ObjectD;
    }//end for
    delete []SampleD;
    delete SampleObject;

    // Return the result.
    return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NNOmniSeqQuery

//==============================================================================
// Class stMOmni
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stMOmni<ObjectType, EvaluatorType>::stMOmni(stPageManager * pageman, u_int32_t nOmni, u_int32_t nFocus,
EvaluatorType * metricEvaluator) {

    // Number of Focus
    NumFocus = nFocus;

    NumOmni = nOmni;

    myPageManager = pageman;

    //myPageManagerD = pagemanD;

    myMetricEvaluator = metricEvaluator;

    myBasicMetricEvaluator = new tBasicMetricEvaluator;

    LogicNode = new tLogicNode(pageman, metricEvaluator);

    for (int i = 0; i < nOmni; i++) {
        char filename[15];
        sprintf(filename, "OmniDistance %d .dat", i);
        PageManagers.insert(PageManagers.end(), new stPlainDiskPageManager(filename, 1024 * 8));
        DistanceNode.insert(DistanceNode.end(), new tDistanceNode(PageManagers[i], myBasicMetricEvaluator));
        OmniPivot.insert(OmniPivot.end(), new tOmniPivot(nFocus, metricEvaluator));
    }

}

template <class ObjectType, class EvaluatorType>
bool stMOmni<ObjectType, EvaluatorType>::Add(tObject * obj) {

    LogicNode->AddEntry(obj);

}//end stOmni<ObjectType><EvaluatorType>::Add

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
ObjectType * stMOmni<ObjectType, EvaluatorType>::GetObject(u_int32_t index) {

    return LogicNode->GetObject(index);
}//end stOmni<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double * stMOmni<ObjectType, EvaluatorType>::GetDistance(u_int32_t index) {

    return DistanceNode[0]->GetObject(index)->GetData();
}//end stOmni<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stMOmni<ObjectType, EvaluatorType>::BuildAllDistance() {

    tObject *tmp;
    double * FieldDistance;
    tBasicArrayObject * ObjectD;

    FieldDistance = new double[NumFocus];

    for (int i = 0; i < this->GetNumberOfObjects(); i++) {
        //debug
        //for(int f = 0; f < NumFocus; f++){
        //  cout << FieldDistance[f] << " ";
        //}
        //cout << "\n" << flush;
        tmp = LogicNode->GetObject(i);
        for (int j = 0; j < NumOmni; j++) {
            //OmniPivot[j]->BuildFieldDistancePartial(tmp, FieldDistance, j);
            OmniPivot[j]->BuildFieldDistance(tmp, FieldDistance);
            ObjectD = new tBasicArrayObject(NumFocus, FieldDistance);
            ObjectD->SetOID(i);
            DistanceNode[j]->AddEntry(ObjectD);
        }
        delete tmp;
    }
    delete []FieldDistance;

}//end stOmni<ObjectType, EvaluatorType>::BuildAllDistance()

template <class ObjectType, class EvaluatorType>
void stMOmni<ObjectType, EvaluatorType>::FindPivot() {

    for (int i = 0; i < NumOmni; i++) {
        OmniPivot[i]->FindPivot(this->LogicNode, this->GetNumberOfObjects(), 1, i);
    }
}//end stOmni<ObjectType, EvaluatorType>::BuildAllDistance()

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMOmni<ObjectType, EvaluatorType>::RangeQueryOmniSeqB(tObject * sample, double range) {
    stPage * currPage;
    tOrigNode * currNode;
    tResult * result;
    tObject * tmp;
    tBasicArrayObject * ObjectD, * SampleObject;
    double distance, OmniDistance, max;
    double *SampleD, *SampleD2;
    u_int32_t i;
    u_int32_t nextPageID;
    int p = 0, ta, tn;

    SampleD = new double[NumFocus];
    SampleD2 = new double[NumFocus];
    ObjectD = new tBasicArrayObject();
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

    ta = OmniPivot[0]->BuildFieldDistance(sample, SampleD);

    for (int i = 1; i < this->NumOmni; i++) {
        tn = OmniPivot[i]->BuildFieldDistance(sample, SampleD2);
        if (tn < ta) {
            memcpy(SampleD, SampleD2, (NumFocus * sizeof (double)));
            ta < tn;
            p = i;
        }
    }

    SampleObject = new tBasicArrayObject(NumFocus, SampleD);
    //cout << " 1 " << flush;
    // First node
    nextPageID = DistanceNode[p]->GetRoot();
    //cout << " begin " << flush;
    // Let's search
    while (nextPageID != 0) {
        // Get node
        //cout << " 2 " << flush;
        currPage = this->PageManagers[p]->GetPage(nextPageID);
        //cout << " 0.5 " << flush;
        currNode = new tOrigNode(currPage);
        //cout << " 1 " << flush;
        // Lets check all objects in this node
        if (currNode->GetNumberOfEntries() > 0) {
            currNode->GetObjectSize(0);
            max = currNode->GetMax();
            ObjectD->Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
            OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

            if (OmniDistance <= range) {
                tmp = LogicNode->GetObject(ObjectD->GetOID());
                distance = this->myMetricEvaluator->GetDistance(tmp, sample);

                if (distance <= range) {
                    // Yes! I'm qualified !
                    result->AddPair(tmp->Clone(), distance);
                }//end if
                delete tmp;
            }//end if


            if (OmniDistance <= range + max) {
                for (i = 1; i < currNode->GetNumberOfEntries(); i++) {
                    // Rebuild the object
                    currNode->GetObjectSize(i);
                    ObjectD->Unserialize(currNode->GetObject(i), currNode->GetObjectSize(i));
                    // Evaluate distance
                    OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

                    if (OmniDistance <= range) {
                        tmp = LogicNode->GetObject(ObjectD->GetOID());
                        distance = this->myMetricEvaluator->GetDistance(tmp, sample);

                        if (distance <= range) {
                            // Yes! I'm qualified !
                            result->AddPair(tmp->Clone(), distance);
                        }//end if
                        delete tmp;
                    }//end if
                    //delete ObjectD;
                }//end for
                // Next PageID...
            }
        }
        nextPageID = currNode->GetNextNode();
        //cout << " Free it all";
        delete currNode;
        this->PageManagers[p]->ReleasePage(currPage);
    }//end while

    delete []SampleD;
    delete []SampleD2;
    delete SampleObject;
    delete ObjectD;

    // Return the result.
    return result;
}//end stOmni<ObjectType><EvaluatorType>::RangeQuery

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stMOmni<ObjectType, EvaluatorType>::NearestQueryOmniSeq(
tObject * sample, u_int32_t k, bool tie) {
    tResult * result;
    tObject * tmp;
    tBasicArrayObject * ObjectD, * SampleObject;
    double distance, OmniDistance;
    double *SampleD, *SampleD2;
    int p = 0, ta, tn;
    
    SampleD = new double[NumFocus];
    SampleD2 = new double[NumFocus];
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), KNEARESTQUERY, k, -1.0, tie);

    for (int i = 1; i < this->NumOmni; i++) {
        tn = OmniPivot[i]->BuildFieldDistance(sample, SampleD2);
        if (tn < ta) {
            memcpy(SampleD, SampleD2, (NumFocus * sizeof (double)));
            ta < tn;
            p = i;
        }
    }

    SampleObject = new tBasicArrayObject(NumFocus, SampleD);

    for (int i = 0; i < k; i++) {
        tmp = LogicNode->GetObject(i);
        distance = this->myMetricEvaluator->GetDistance(tmp, sample);
        result->AddPair(tmp, distance);
    }//end for


    for (int i = k; i < DistanceNode[p]->GetNumberOfObjects(); i++) {
        ObjectD = DistanceNode[p]->GetObject(i);
        OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

        if (OmniDistance <= result->GetMaximumDistance()) {
            tmp = LogicNode->GetObject(i);
            distance = this->myMetricEvaluator->GetDistance(tmp, sample);
            if (distance <= result->GetMaximumDistance()) {
                // Yes! I'll.
                result->AddPair(tmp, distance);
                result->Cut(k);
            } else {
                delete tmp;
            }
        }//end if
        delete ObjectD;
    }//end for
    delete []SampleD;
    delete []SampleD2;
    delete SampleObject;

    // Return the result.
    return result;
}//end stDummyTree<ObjectType><EvaluatorType>::NNOmniSeqQuery

//==============================================================================
// Class stClOmni
//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
stClOmni<ObjectType, EvaluatorType>::stClOmni(stPageManager * pageman, u_int32_t nCluster, u_int32_t nFocus,
EvaluatorType * metricEvaluator) {

    // Number of Focus
    NumFocus = nFocus;

    NumClusters = nCluster; // k

    myPageManager = pageman;

    //myPageManagerD = pagemanD;

    myMetricEvaluator = metricEvaluator;

    myBasicMetricEvaluator = new tBasicMetricEvaluator;

    LogicNode = new tLogicNode(pageman, metricEvaluator);

    for (int i = 0; i < NumClusters; i++) {
        char filename[15];
        sprintf(filename, "OmniClDistance %d .dat", i);
        PageManagers.insert(PageManagers.end(), new stPlainDiskPageManager(filename, 1024 * 4));
        DistanceNode.insert(DistanceNode.end(), new tDistanceNode(PageManagers[i], myBasicMetricEvaluator));
        OmniPivot.insert(OmniPivot.end(), new tOmniPivot(nFocus, metricEvaluator));
    }

    Clusters = new stCluster[NumClusters];

}

template <class ObjectType, class EvaluatorType>
bool stClOmni<ObjectType, EvaluatorType>::Add(tObject * obj) {

    LogicNode->AddEntry(obj);
    LogicNode->AddInfo();

}//end stOmni<ObjectType><EvaluatorType>::Add

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
ObjectType * stClOmni<ObjectType, EvaluatorType>::GetObject(u_int32_t index) {

    return LogicNode->GetObject(index);
}//end stOmni<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
double * stClOmni<ObjectType, EvaluatorType>::GetDistance(u_int32_t index) {

    return DistanceNode[0]->GetObject(index)->GetData();
}//end stOmni<ObjectType, EvaluatorType>::GetObject

//------------------------------------------------------------------------------

template <class ObjectType, class EvaluatorType>
void stClOmni<ObjectType, EvaluatorType>::BuildAllDistance() {

    tObject *tmp;
    double * FieldDistance;
    tBasicArrayObject * ObjectD;

    FieldDistance = new double[NumFocus];

    for (int i = 0; i < this->GetNumberOfObjects(); i++) {
        //debug
        //for(int f = 0; f < NumFocus; f++){
        //  cout << FieldDistance[f] << " ";
        //}
        //cout << "\n" << flush;
        tmp = LogicNode->GetObject(i);
        for (int j = 0; j < NumClusters; j++) {
            OmniPivot[j]->BuildFieldDistance(tmp, FieldDistance);
            ObjectD = new tBasicArrayObject(NumFocus, FieldDistance);
            ObjectD->SetOID(i);
            DistanceNode[j]->AddEntry(ObjectD);
        }
        delete tmp;
    }
    delete []FieldDistance;

}//end stOmni<ObjectType, EvaluatorType>::BuildAllDistance()

template <class ObjectType, class EvaluatorType>
void stClOmni<ObjectType, EvaluatorType>::FindPivot() {

    for (int i = 0; i < NumClusters; i++) {
        OmniPivot[i]->FindPivot(this->LogicNode, this->GetNumberOfObjects(), 4, i);
    }
}//end stOmni<ObjectType, EvaluatorType>::BuildAllDistance()

template <class ObjectType, class EvaluatorType>
void stClOmni<ObjectType, EvaluatorType>::Cluster() {

    double * tmp;
    ObjectType * ObjectD, * ObjectJ;
    int cluster;
    double minDistance, Distance, cost;

    //clone first NumCluster objects, to be the initial means.
    int test = rand() % (this->GetNumberOfObjects() / (NumClusters + 1));
    for (int k = 0; k < NumClusters; k++) {
        int idx = k*test;
        Clusters[k].ClusterMedoid = LogicNode->GetObject(idx);
        Clusters[k].maxDist = 0;
        Clusters[k].Cost = MAXINT;
    }

    int n = 0;
    bool go = true;
    while (n < 15 && go) {
        go = false;
        //cout << " it " << n;
        for (int k = 0; k < NumClusters; k++) {

            Clusters[k].Count = 0;
        }
        //pass throw all objects and put it into a cluster
        //Distribute Objects
        for (int i = 0; i < this->GetNumberOfObjects(); i++) {
            minDistance = (double) MAXINT;
            ObjectD = LogicNode->GetObject(i);
            for (int k = 0; k < NumClusters; k++) {
                Distance = myMetricEvaluator->GetDistance(Clusters[k].ClusterMedoid, ObjectD);
                if (Distance < minDistance) {
                    cluster = k;
                    minDistance = Distance;
                    LogicNode->SetCluster(i, k);
                }//end if (Distance < minDistance)

            }//end for k
            LogicNode->ResetCost(i);
            Clusters[cluster].Count += 1;
            delete ObjectD;
        }//end for i

        //Measure Costs
        for (int i = 0; i < this->GetNumberOfObjects(); i++) {
            ObjectD = LogicNode->GetObject(i);
            for (int j = i; j < this->GetNumberOfObjects(); j++) {
                cluster = LogicNode->GetCluster(i);
                if (LogicNode->GetCluster(j) == cluster) {
                    ObjectJ = LogicNode->GetObject(j);
                    cost = myMetricEvaluator->GetDistance(ObjectD, ObjectJ);
                    LogicNode->AddCost(i, cost);
                    LogicNode->AddCost(j, cost);
                    delete ObjectJ;
                }
            }
            delete ObjectD;
        }
        for (int i = 0; i < this->GetNumberOfObjects(); i++) {
            cluster = LogicNode->GetCluster(i);
            if (LogicNode->GetCost(i) < Clusters[cluster].Cost) {
                go = true;
                Clusters[cluster].Cost = LogicNode->GetCost(i);
                delete Clusters[cluster].ClusterMedoid;
                Clusters[cluster].ClusterMedoid = LogicNode->GetObject(i);
            }//end if


        }//end for k
        n++;
    }//end while

    cout << "iteraÃ§Ãµes: " << n << endl;

    for (int c = 0; c < NumClusters; c++) {
        if (Clusters[c].Count < NumFocus) {
            cout << "cluster found too small";
        }
    }

    /*for (int i = 0; i < this->GetNumberOfDistances(); i++) {
       // cluster = DistanceNode->GetCluster(i);
        //Distance = myBasicMetricEvaluator->GetDistance(Clusters[cluster].ClusterMean, DistanceNode->GetObject(i));
        if (Distance > Clusters[cluster].maxDist) {
            Clusters[cluster].maxDist = Distance;
        }
    }*/


}//end stMGrid<ObjectType, EvaluatorType>::Cluster()

template <class ObjectType, class EvaluatorType>
stResult<ObjectType> * stClOmni<ObjectType, EvaluatorType>::RangeQueryOmniSeqB(tObject * sample, double range) {
    stPage * currPage;
    tOrigNode * currNode;
    tResult * result;
    tObject * tmp;
    tBasicArrayObject * ObjectD, * SampleObject;
    double distance, OmniDistance, max, newcl;
    double *SampleD;
    u_int32_t i;
    u_int32_t nextPageID;


    SampleD = new double[NumFocus];
    ObjectD = new tBasicArrayObject();
    result = new tResult();
    result->SetQueryInfo(sample->Clone(), RANGEQUERY, -1, range, false);

    int Cl = 0;

    double ClDistance = myMetricEvaluator->GetDistance(Clusters[Cl].ClusterMedoid, sample);
    for (int k = 1; k < NumClusters; k++) {
        newcl = myMetricEvaluator->GetDistance(Clusters[k].ClusterMedoid, sample);
        if (newcl < ClDistance) {
            ClDistance = newcl;
            Cl = k;
        }
    }

    // cout << " cl: " << Cl;

    OmniPivot[Cl]->BuildFieldDistance(sample, SampleD);
    SampleObject = new tBasicArrayObject(NumFocus, SampleD);

    //cout << " 1 " << flush;
    // First node
    nextPageID = DistanceNode[Cl]->GetRoot();
    //cout << " begin " << flush;
    // Let's search
    while (nextPageID != 0) {
        // Get node
        //cout << " 2 " << flush;
        currPage = this->PageManagers[Cl]->GetPage(nextPageID);
        //cout << " 0.5 " << flush;
        currNode = new tOrigNode(currPage);
        //cout << " 1 " << flush;
        // Lets check all objects in this node
        if (currNode->GetNumberOfEntries() > 0) {
            currNode->GetObjectSize(0);
            max = currNode->GetMax();
            ObjectD->Unserialize(currNode->GetObject(0), currNode->GetObjectSize(0));
            OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

            if (OmniDistance <= range) {
                tmp = LogicNode->GetObject(ObjectD->GetOID());
                distance = this->myMetricEvaluator->GetDistance(tmp, sample);

                if (distance <= range) {
                    // Yes! I'm qualified !
                    result->AddPair(tmp->Clone(), distance);
                }//end if
                delete tmp;
            }//end if


            if (OmniDistance <= range + max) {
                for (i = 1; i < currNode->GetNumberOfEntries(); i++) {
                    // Rebuild the object
                    currNode->GetObjectSize(i);
                    ObjectD->Unserialize(currNode->GetObject(i), currNode->GetObjectSize(i));
                    // Evaluate distance
                    OmniDistance = myBasicMetricEvaluator->GetDistance(ObjectD, SampleObject);

                    if (OmniDistance <= range) {
                        tmp = LogicNode->GetObject(ObjectD->GetOID());
                        distance = this->myMetricEvaluator->GetDistance(tmp, sample);

                        if (distance <= range) {
                            // Yes! I'm qualified !
                            result->AddPair(tmp->Clone(), distance);
                        }//end if
                        delete tmp;
                    }//end if
                    //delete ObjectD;
                }//end for
                // Next PageID...
            }
        }
        nextPageID = currNode->GetNextNode();
        //cout << " Free";
        delete currNode;
        this->PageManagers[Cl]->ReleasePage(currPage);
    }//end while

    delete []SampleD;
    delete SampleObject;
    delete ObjectD;


    // Return the result.
    return result;
}//end stOmni<ObjectType><EvaluatorType>::RangeQuery
