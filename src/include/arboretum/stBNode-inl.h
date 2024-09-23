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
 * This file implements the Btree node classes
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 */

//------------------------------------------------------------------------------
// class stBNode
//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
stBNode<KeyType, Comparator> * stBNode<KeyType, Comparator>::CreateNode(stPageManager * pageManager, stPage * page) {
    stBNodeHeader * header;

    header = (stBNodeHeader *) (page->GetData());

    switch (header->Type) {
        case INDEX:
            // Create an index page
            return new stBIndexNode<KeyType, Comparator>(pageManager, page, false);
        case LEAF:
            // Create a leaf page
            return new stBLeafNode<KeyType, Comparator>(pageManager, page, false);
        case LEAF_OVERFLOW:
            // Create a leaf overflow node
            return new stBLeafOverflowNode<KeyType, Comparator>(pageManager, page, false);
        default:
            return NULL;
    }//end switch
}//end stBNode::CreateNode()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
stBNode<KeyType, Comparator>::stBNode(stPageManager * pageManager, stPage * page, bool create) {
    PageManager = pageManager;
    Page = page;
    SHeader = (stBNodeHeader *) (Page->GetData());

    if (create) {
#ifdef __stDEBUG__
        Page->Clear();
#endif //__stDEBUG__
        SHeader->Occupation = 0;
        IsPageModified = true;
    }//end if
    else {
        IsPageModified = false;
    }
}//end stBNode::stBNode()


//------------------------------------------------------------------------------
// class stBIndexNode
//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
stBIndexNode<KeyType, Comparator>::stBIndexNode(stPageManager * pageManager, stPage * page, bool create) :
stBNode<KeyType, Comparator>(pageManager, page, create) {

    // Set member variable's pointers
    LeftmostPageID = (u_int32_t *) (((unsigned char *) this->SHeader) + sizeof (typename tBNode::stBNodeHeader));
    Entries = (stBIndexNodeEntry *) (((unsigned char *) LeftmostPageID) + sizeof (u_int32_t));

    // Initialize page
    if (create) {
        this->SHeader->Type = tBNode::INDEX;
        *LeftmostPageID = 0;
    }//end if
}//end stBIndexNode::stBIndexNode()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
bool stBIndexNode<KeyType, Comparator>::InsertEntryAt(u_int32_t idx, stBIndexNodeEntry &entry) {

    // Does it fit?
    if (GetFree() < sizeof(stBIndexNodeEntry)) {
        // No, it doesn't.
        return false;
    }

    // Make room for the new entry
    for (u_int32_t i = this->SHeader->Occupation; i > idx; i--) {
        Entries[i] = Entries[i - 1];
    }

    // Insert it
    Entries[idx] = entry;

    // Update # of entries
    this->SHeader->Occupation++;

    // Schedule page for writing
    this->IsPageModified = true;

    return true;
}//end stBIndexNode::InsertEntryAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
void stBIndexNode<KeyType, Comparator>::SetLeftPageIDAt(u_int32_t keyIdx, u_int32_t leftPageID) {

#ifdef __stDEBUG__
   if (keyIdx >= this->SHeader->Occupation) {
       throw std::logic_error("idx value is out of range. stBIndexNode::SetLeftPageIDAt()");
   }//end if
#endif //__stDEBUG__

   if (keyIdx == 0) {
       *LeftmostPageID = leftPageID;
   }
   else {
       Entries[keyIdx - 1].RightPageID = leftPageID;
   }

    // Schedule page for writing
    this->IsPageModified = true;
}//end stBIndexNode::SetLeftPageIDAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBIndexNode<KeyType, Comparator>::GetLeftPageIDAt(u_int32_t keyIdx) {

#ifdef __stDEBUG__
   if (keyIdx >= this->SHeader->Occupation) {
       throw std::logic_error("idx value is out of range. stBIndexNode::GetLeftPageIDAt()");
   }//end if
#endif //__stDEBUG__

   if (keyIdx == 0) {
       return *LeftmostPageID;
   }

   return Entries[keyIdx - 1].RightPageID;
}//end stBIndexNode::GetLeftPageIDAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBIndexNode<KeyType, Comparator>::GetRightPageIDAt(u_int32_t keyIdx) {

#ifdef __stDEBUG__
   if (keyIdx >= this->SHeader->Occupation) {
       throw std::logic_error("idx value is out of range. stBIndexNode::GetRighttPageIDAt()");
   }//end if
#endif //__stDEBUG__

   return Entries[keyIdx].RightPageID;
}//end stBIndexNode::GetRightPageIDAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBIndexNode<KeyType, Comparator>::Find(KeyType key) {

    int first = 0;
    int last = this->SHeader->Occupation - 1;
    int middle;

    // Perform a binary search
    while (first <= last) {
        middle = (first + last) / 2;
        if (this->equal(key, Entries[middle].Key)) {

            // Element found
            return middle;
        }
        if (this->less(key, Entries[middle].Key))
            last = middle - 1;
        else first = middle + 1;
    }

    // Return the position that key should be
    return first;
}//end stBIndexNode::Find()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBIndexNode<KeyType, Comparator>::GetFree() {
    u_int32_t usedSize;

    // Fixed size
    usedSize = sizeof (typename tBNode::stBNodeHeader) + sizeof (u_int32_t);

    // Entries
    if (this->SHeader->Occupation > 0) {
        usedSize += (sizeof (stBIndexNodeEntry) * this->SHeader->Occupation);
    }//end if

    /**
     * *** WARNING! ***
     *
     * The following /50 is for DEBUG reasons.
     */
//    return ((this->Page->GetPageSize()/50) - usedSize);

    return (this->Page->GetPageSize() - usedSize);
}//end stBIndexNode::GetFree()


//------------------------------------------------------------------------------
// class stBLeafNode
//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
stBLeafNode<KeyType, Comparator>::stBLeafNode(stPageManager * pageManager, stPage * page, bool create) :
stBNode<KeyType, Comparator>(pageManager, page, create) {

    // Set member variable's pointers
    Header = (stBLeafNodeHeader *) (((unsigned char *) this->SHeader) + sizeof (typename tBNode::stBNodeHeader));
    Entries = (stBLeafNodeEntry *) (((unsigned char *) Header) + sizeof (stBLeafNodeHeader));

    // Initialize page
    if (create) {
        this->SHeader->Type = tBNode::LEAF;
        Header->PreviousPageID = 0;
        Header->NextPageID = 0;
        Header->OverflowPageID = 0;
        Header->OverflowOccupation = 0;
    }//end if
    else {
        if (Header->OverflowPageID != 0) {
            stPage * overflowPage;
            tBLeafOverflowNode * overflowNode;
            u_int32_t nextOverflowPageID;

            // Load overflow nodes
            nextOverflowPageID = Header->OverflowPageID;
            while (nextOverflowPageID != 0) {
                overflowPage = this->PageManager->GetPage(nextOverflowPageID);
                overflowNode = new tBLeafOverflowNode(this->PageManager, overflowPage);

                // Save its instance pointer
                overflowNodes.push_back(overflowNode);
                nextOverflowPageID = overflowNode->GetOverflowPageID();
            }//end while
        }//end if
    }//end else
}//end stBLeafNode::stBLeafNode()


//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
stBLeafNode<KeyType, Comparator>::~stBLeafNode() {
    stPage * overflowPage;
    for (typename std::vector<tBLeafOverflowNode *>::iterator it = overflowNodes.begin(); it != overflowNodes.end(); ++it) {
        //cout << "Overflow delete. Size: " << (*it)->GetNumberOfEntries() << endl;
        overflowPage = (*it)->GetPage();
        delete (*it);
        this->PageManager->ReleasePage(overflowPage);
    }
}//end stBLeafNode::~stBLeafNode()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
unsigned int stBLeafNode<KeyType, Comparator>::Insert(KeyType key, u_int32_t serializedObjectSize, const unsigned char * serializedObject, bool duplicationAllowed) {

    u_int32_t entrySize;
    u_int32_t idx;

#ifdef __stDEBUG__
    if (serializedObjectSize == 0) {
        throw std::logic_error("The object size is 0.");
    }//end if
#endif //__stDEBUG__

    entrySize = serializedObjectSize + sizeof (stBLeafNodeEntry);

    // Does it fit in the leaf node?
    if (entrySize > GetLeafNodeFree()) {
        // No, it doesn't.
        // Is it a case of duplicate insert in leaf overflow node?
        if ((this->SHeader->Occupation > 0) && (key == Entries[0].Key) && (key == Entries[this->SHeader->Occupation - 1].Key)) {

            stPage * newOverflowPage;
            tBLeafOverflowNode * newOverflowNode;
            bool newNodeCreated = false;

            // Have not this node a leaf overflow node yet?
            if (overflowNodes.empty()) {
                // Allocate a new overflow node
                newOverflowPage = this->PageManager->GetNewPage();
                newOverflowNode = new tBLeafOverflowNode(this->PageManager, newOverflowPage, true);
                
                // Link it to the BLeafNode and save its instance pointer
                Header->OverflowPageID = newOverflowNode->GetPageID();
                overflowNodes.push_back(newOverflowNode);
                newNodeCreated = true;
            }

            // Try to insert the object in the last overflow node
            if (overflowNodes.back()->Insert(serializedObjectSize, serializedObject) == tBNode::SUCCESS) {
                // Update # of overflow entries
                Header->OverflowOccupation++;
                this->IsPageModified = true;
                if (newNodeCreated)
                    return tBNode::SUCCESS_NEWOVERFLOWNODE;
                else
                    return tBNode::SUCCESS;
            }

            if (newNodeCreated) {
                // Error! Object did not fit in the fresh created overflow node.
                // Dispose new overflow node
                newOverflowNode = (tBLeafOverflowNode *) overflowNodes.back();
                overflowNodes.pop_back();
                newOverflowPage = newOverflowNode->GetPage();
                delete newOverflowNode;
                this->PageManager->DisposePage(newOverflowPage);

                // Unlink it from BLeafNode
                Header->OverflowPageID = 0;

                return tBNode::ERROR;
            }

            // Allocate a new overflow node
            newOverflowPage = this->PageManager->GetNewPage();
            newOverflowNode = new tBLeafOverflowNode(this->PageManager, newOverflowPage, true);

            // Link it to the last overflow node and save its instance pointer
            overflowNodes.back()->SetOverflowPageID(newOverflowNode->GetPageID());
            overflowNodes.push_back(newOverflowNode);

            // Try to insert the object again
            if (overflowNodes.back()->Insert(serializedObjectSize, serializedObject) == tBNode::SUCCESS) {
                // Update # of overflow entries
                Header->OverflowOccupation++;
                this->IsPageModified = true;
                return tBNode::SUCCESS_NEWOVERFLOWNODE;
            }

            // Error! Object did not fit in the fresh created overflow node.
            // Dispose new overflow node
            newOverflowNode = (tBLeafOverflowNode *) overflowNodes.back();
            overflowNodes.pop_back();
            newOverflowPage = newOverflowNode->GetPage();
            delete newOverflowNode;
            this->PageManager->DisposePage(newOverflowPage);

            // Unlink it from the last overflow node
            overflowNodes.back()->SetOverflowPageID(0);

#ifdef __stDEBUG__
            cout << "The page size is too small to store the element." << endl;
            throw std::logic_error("The page size is too small to store the element.");
#endif //__stDEBUG__

            return tBNode::ERROR;
        }//end if (overflow handling)
        else {
            // No, it is a case of split
            return tBNode::NODE_FULL;
        }
    }//end if

    // Is element the first being inserted?
    if (this->SHeader->Occupation == 0) {
        // First object
        idx = 0;
    }//end if
    else {
        // Any other
        // Find the right position in the key sorted list of entries
        if (FindLast(key, idx)) {
            // Insertion of a duplicated key in a tree that doesn't allow duplication
            if (!duplicationAllowed) {
                return tBNode::DUPLICATION;
            }
            idx++; // Insert element after exiting ones with the same key
        }

        u_int32_t lastIdx;
        lastIdx = this->SHeader->Occupation - 1; // It will always be non-negative

        if (idx <= lastIdx) {
            // Make room for the new object.
            // We will move left all object data with key greater than Entries[idx].Key
            // at once using memmove() from stdlib, because it handles the overlap between
            // src and dst. Remember that object data raise from the end of the page.
            if (idx == 0) {
                memmove(this->Page->GetData() + Entries[lastIdx].Offset - serializedObjectSize,
                    this->Page->GetData() + Entries[lastIdx].Offset,
                    this->Page->GetPageSize() - Entries[lastIdx].Offset);
            }//end if
            else {
                memmove(this->Page->GetData() + Entries[lastIdx].Offset - serializedObjectSize,
                    this->Page->GetData() + Entries[lastIdx].Offset,
                    Entries[idx - 1].Offset - Entries[lastIdx].Offset);
            }//end else

            // Make room for the new entry
            for (u_int32_t i = (lastIdx + 1); i > idx; i--) {
                Entries[i].Key = Entries[i - 1].Key;
                Entries[i].Offset = Entries[i - 1].Offset - serializedObjectSize;
            }//end for
        }//end if
    }//end else

    // Set the object data offset
    if (idx == 0) {
        Entries[idx].Offset = this->Page->GetPageSize() - serializedObjectSize;
    }//end if
    else {
        Entries[idx].Offset = Entries[idx - 1].Offset - serializedObjectSize;
    }//end else

    // Insert the current entry
    Entries[idx].Key = key;

    // Insert the object
    memcpy((void *) (this->Page->GetData() + Entries[idx].Offset), (void *) serializedObject, serializedObjectSize);

    // Update # of entries
    this->SHeader->Occupation++;
    this->IsPageModified = true;

    return tBNode::SUCCESS;
}//end stBLeafNode::Insert()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
void stBLeafNode<KeyType, Comparator>::DeleteElementAt(u_int32_t idx) {

#ifdef __stDEBUG__
    if ((idx >= GetNumberOfEntries()) || (GetNumberOfEntries() == 0)){
        throw std::logic_error("idx value is out of range. stBLeafNode::DeleteElementAt()");
    }//end if
#endif //__stDEBUG__

    u_int32_t lastIdx;
    u_int32_t delObjectSize;

    lastIdx = this->SHeader->Occupation - 1; // It will always be non-negative
    delObjectSize = GetSerializedObjectSizeAt(idx);

    // Is object in the leaf node?
    if (idx <= lastIdx) {
        // Is it necessary to move object data?
        if (idx < lastIdx) {
            // We will move right all object data with key greater than Entries[idx].Key
            // at once using memmove() from stdlib, because it handles the overlap between
            // src and dst. Remember that object data raise from the end of the page.
            memmove(this->Page->GetData() + Entries[lastIdx].Offset + delObjectSize,
                    this->Page->GetData() + Entries[lastIdx].Offset,
                    Entries[idx].Offset - Entries[lastIdx].Offset);
        }//end if

        // Update Offsets
        for (u_int32_t i = idx; i < lastIdx; i++) {
            Entries[i].Key = Entries[i + 1].Key;
            Entries[i].Offset = Entries[i + 1].Offset + delObjectSize;
        }//end for

        // Update # of entries
        this->SHeader->Occupation--;
        this->IsPageModified = true;
    }//end if
    else {
        // Has node overflow page(s)?
        if (!overflowNodes.empty()) {
            u_int32_t numObjects;
            numObjects = this->SHeader->Occupation;

            // Search for idx in overflow nodes
            for (typename std::vector<tBLeafOverflowNode *>::iterator it = overflowNodes.begin(); it != overflowNodes.end(); ++it) {
                // Is the index in this overflow node?
                if (idx < (numObjects + (*it)->GetNumberOfEntries())) {
                    // Yes, delete the object
                    (*it)->DeleteElementAt(idx - numObjects);

                    // Did overflow node become empty?
                    if ((*it)->GetNumberOfEntries() == 0) {
                        // Update OverflowPageID links
                        if (it != overflowNodes.begin()) {
                            tBLeafOverflowNode * prevOverflowNode = *(it - 1);
                            prevOverflowNode->SetOverflowPageID((*it)->GetOverflowPageID());
                        }//end if
                        else {
                            Header->OverflowPageID = (*it)->GetOverflowPageID();
                        }//end else

                        // Dispose overflow node
                        stPage * overflowPage = (*it)->GetPage();
                        delete (*it);
                        this->PageManager->DisposePage(overflowPage);
                        overflowNodes.erase(it);
                    }//end if

                    // Update # of entries
                    Header->OverflowOccupation--;
                    this->IsPageModified = true;

                    break;
                }//end if
                numObjects += (*it)->GetNumberOfEntries();
            }//end for
        }//end if
        else {
#ifdef __stDEBUG__
            throw std::logic_error("Overflow node missing. stBLeafNode::DeleteElementAt()");
#endif //__stDEBUG__
            return;
        }//end else
    }//end else

}//end stBLeafNode::DeleteElementAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
bool stBLeafNode<KeyType, Comparator>::Find(KeyType key, u_int32_t &idx) {

    int first = 0;
    int last = this->SHeader->Occupation - 1;
    int middle;

    // Perform a binary search
    while (first <= last) {
        middle = (first + last) / 2;
        if (tBNode::equal(key, Entries[middle].Key)) {

            // Element found
            idx = middle;
            return true;
        }
        if (tBNode::less(key, Entries[middle].Key))
            last = middle - 1;
        else first = middle + 1;
    }

    // Set idx with the position that key should be inserted
    idx = first;
    return false;
}//end stBLeafNode::Find()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
bool stBLeafNode<KeyType, Comparator>::FindFirst(KeyType key, u_int32_t &idx) {

    if (!Find(key, idx)) {
        // Element was not found
        return false;
    }

    // Element found! Find its first occurrence
    while ((idx > 0) && (tBNode::equal(key, Entries[idx - 1].Key)))
        idx--;

    return true;

}//end stBLeafNode::FindFirst()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
bool stBLeafNode<KeyType, Comparator>::FindLast(KeyType key, u_int32_t &idx) {

    if (!Find(key, idx)) {
        // Element was not found
        return false;
    }

    // Element found! Find its last occurrence
    if (!overflowNodes.empty())
        // All keys are equal. Return the last one.
        idx = (this->SHeader->Occupation + Header->OverflowOccupation) - 1;
    else
        // Search for the last duplicate in the leaf node
        while ((idx < (this->SHeader->Occupation - 1)) && (tBNode::equal(key, Entries[idx + 1].Key)))
            idx++;

    return true;
}//end stBLeafNode::FindLast()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBLeafNode<KeyType, Comparator>::FindMedian() {

    long idx;
    long up;
    long down;

    // Get the ceil of the median
    idx = ((this->SHeader->Occupation % 2) == 0 ? (this->SHeader->Occupation / 2) : (this->SHeader->Occupation / 2) + 1);

    // Look backwards for the FIRST DUPLICATED key
    down = idx;
    while ((down > 0) && (tBNode::equal(Entries[down - 1].Key, Entries[down].Key)))
        down--;

    
    // Are there duplicates backwards?
    if (down != idx) {
        // Look forward for the NEXT NOT DUPLICATED key, to avoid splitting entries with
        // duplicated keys, which must be always in a same leaf node
        up = idx;
        while ((up < (this->SHeader->Occupation - 1)) && (tBNode::equal(Entries[up].Key, Entries[up + 1].Key)))
            up++;

        // Set idx as the entry that are the closest to the median and do not
        // split duplicate key entries.
//        idx = ((up - idx) <= (idx - down)) ? (up + 1) : down;
        idx = ((up - idx) <= (idx - down)) && (up < (this->SHeader->Occupation - 1)) ? (up + 1) : down;
    }

    return idx;
}//end stBLeafNode::FindMedian()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
const unsigned char * stBLeafNode<KeyType, Comparator>::GetSerializedObjectAt(u_int32_t idx) {

#ifdef __stDEBUG__
    if (idx >= (this->SHeader->Occupation + Header->OverflowOccupation)) {
        throw std::logic_error("idx value is out of range. stBLeafNode::GetSerializedObjectAt()");
    }//end if
#endif //__stDEBUG__

    // Is object in the leaf node?
    if (idx < this->SHeader->Occupation) {
        return this->Page->GetData() + Entries[idx].Offset;
    }

    // Has node overflow page(s)?
    if (!overflowNodes.empty()) {
        u_int32_t numObjects;
        numObjects = this->SHeader->Occupation;

        // Search for idx in overflow nodes
        for (typename std::vector<tBLeafOverflowNode *>::iterator it = overflowNodes.begin(); it != overflowNodes.end(); ++it) {
            // Is the index in this overflow node?
            if (idx < (numObjects + (*it)->GetNumberOfEntries())) {
                // Yes, get the object

                return (*it)->GetSerializedObjectAt(idx - numObjects);
            }
            numObjects += (*it)->GetNumberOfEntries();
        }
    }//end if

    // Did not returned yet? Error!
    return NULL;
}//end stBLeafNode::GetSerializedObjectAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBLeafNode<KeyType, Comparator>::GetSerializedObjectSizeAt(u_int32_t idx) {
#ifdef __stDEBUG__
    if (idx >= (this->SHeader->Occupation + Header->OverflowOccupation)) {
        throw std::logic_error("idx value is out of range. stBLeafNode::GetSerializedObjectSizeAt()");
    }//end if
#endif //__stDEBUG__

    // Is object in the leaf node?
    if (idx < this->SHeader->Occupation) {
        if (idx == 0) {
            // First object
            return this->Page->GetPageSize() - Entries[0].Offset;
        } else {
            // Any other
            return Entries[idx - 1].Offset - Entries[idx].Offset;
        }//end if
    }

    // Has node overflow page(s)?
    if (!overflowNodes.empty()) {
        u_int32_t numObjects;
        numObjects = this->SHeader->Occupation;

        // Search for idx in overflow nodes
        for (typename std::vector<tBLeafOverflowNode *>::iterator it = overflowNodes.begin(); it != overflowNodes.end(); ++it) {
            // Is the index in this overflow node?
            if (idx < (numObjects + (*it)->GetNumberOfEntries())) {
                // Yes, get the object

                return (*it)->GetSerializedObjectSizeAt(idx - numObjects);
            }
            numObjects += (*it)->GetNumberOfEntries();
        }
    }//end if

    // Did not returned yet? Error!
    return 0;
}//end stBLeafNode::GetSerializedObjectSizeAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBLeafNode<KeyType, Comparator>::GetLeafNodeFree() {
    u_int32_t usedSize;

    // Fixed size
    usedSize = sizeof (typename tBNode::stBNodeHeader) + sizeof (stBLeafNodeHeader);

    // Entries
    if (this->SHeader->Occupation > 0) {

        usedSize +=
                // Total size of entries
                (sizeof (stBLeafNodeEntry) * this->SHeader->Occupation) +
                // Total object size
                (this->Page->GetPageSize() - Entries[this->SHeader->Occupation - 1].Offset);
    }//end if

    return this->Page->GetPageSize() - usedSize;
}//end stBLeafNode::GetLeafNodeFree()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBLeafNode<KeyType, Comparator>::GetFree() {
    if (overflowNodes.empty())
        // Free space in the leaf node
        return GetLeafNodeFree();

    else
        // Free space in the last overflow node
        return overflowNodes.back()->GetFree();

}//end stBLeafNode::GetFree()


//------------------------------------------------------------------------------
// class stBLeafOverflowNode
//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
stBLeafOverflowNode<KeyType, Comparator>::stBLeafOverflowNode(stPageManager * pageManager, stPage * page, bool create) :
stBNode<KeyType, Comparator>(pageManager, page, create) {

    // Set member variable's pointers
    Header = (stBLeafOverflowNodeHeader *) (((unsigned char *) this->SHeader) + sizeof (typename tBNode::stBNodeHeader));
    Entries = (stBLeafOverflowNodeEntry *) (((unsigned char *) Header) + sizeof (stBLeafOverflowNodeHeader));

    // Initialize page
    if (create) {

        this->SHeader->Type = tBNode::LEAF_OVERFLOW;
        Header->OverflowPageID = 0;
    }//end if
}//end stBLeafOverflowNode::stBLeafOverflowNode()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
unsigned int stBLeafOverflowNode<KeyType, Comparator>::Insert(u_int32_t serializedObjectSize, const unsigned char * serializedObject) {

    u_int32_t entrySize;

#ifdef __stDEBUG__
    if (serializedObjectSize == 0) {
        throw std::logic_error("The object size is 0.");
    }//end if
#endif //__stDEBUG__

    // Does it fit ?
    entrySize = serializedObjectSize + sizeof (stBLeafOverflowNodeEntry);
    if (entrySize > GetFree()) {
        // No, it doesn't.
        return tBNode::NODE_FULL;
    }//end if

    // Adding the object. Take care with these pointers or you will destroy the
    // node. The idea is to put the entry in the last position and the object
    // in reverse order of insertion
    if (this->SHeader->Occupation == 0) {
        Entries[this->SHeader->Occupation].Offset = this->Page->GetPageSize() - serializedObjectSize;
    } else {
        Entries[this->SHeader->Occupation].Offset = Entries[this->SHeader->Occupation - 1].Offset - serializedObjectSize;
    }//end if
    memcpy((void *) (this->Page->GetData() + Entries[this->SHeader->Occupation].Offset), (void *) serializedObject, serializedObjectSize);

    // Update # of entries
    this->SHeader->Occupation++;

    // Schedule page for writing
    this->IsPageModified = true;

    // Return the entry idx
//    return this->SHeader->Occupation - 1;
    return tBNode::SUCCESS;

}//end stBLeafOverflowNode::Insert()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
void stBLeafOverflowNode<KeyType, Comparator>::DeleteElementAt(u_int32_t idx) {

#ifdef __stDEBUG__
    if (idx >= this->SHeader->Occupation) {
        throw std::logic_error("idx value is out of range. stBLeafOverflowNode::DeleteElementAt()");
    }//end if
#endif //__stDEBUG__

    u_int32_t lastIdx;
    u_int32_t delObjectSize;

    lastIdx = this->SHeader->Occupation - 1; // It will always be non-negative
    delObjectSize = GetSerializedObjectSizeAt(idx);

    // Is it necessary to move object data?
    if (idx < lastIdx) {
        // We will move right all object data with key greater than Entries[idx].Key
        // at once using memmove() from stdlib, because it handles the overlap between
        // src and dst. Remember that object data raise from the end of the page.
        memmove(this->Page->GetData() + Entries[lastIdx].Offset + delObjectSize,
                this->Page->GetData() + Entries[lastIdx].Offset,
                Entries[idx].Offset - Entries[lastIdx].Offset);
    }//end if

    // Update Offsets
    for (u_int32_t i = idx; i < lastIdx; i++) {
        Entries[i].Offset = Entries[i + 1].Offset + delObjectSize;
    }//end for

    // Update # of entries
    this->SHeader->Occupation--;
    this->IsPageModified = true;

}//end stBLeafOverflowNode::DeleteElementAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
const unsigned char * stBLeafOverflowNode<KeyType, Comparator>::GetSerializedObjectAt(u_int32_t idx) {
#ifdef __stDEBUG__
    if (idx >= this->SHeader->Occupation) {

        throw std::logic_error("idx value is out of range. stBLeafOverflowNode::GetSerializedObjectAt()");
    }//end if
#endif //__stDEBUG__

    return this->Page->GetData() + Entries[idx].Offset;
}//end stBLeafOverflowNode::GetSerializedObjectAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBLeafOverflowNode<KeyType, Comparator>::GetSerializedObjectSizeAt(u_int32_t idx) {
#ifdef __stDEBUG__
    if (idx >= this->SHeader->Occupation) {
        throw std::logic_error("idx value is out of range. stBLeafOverflowNode::GetSerializedObjectSizeAt()");
    }//end if
#endif //__stDEBUG__

    if (idx == 0) {
        // First object
        return this->Page->GetPageSize() - Entries[0].Offset;
    } else {
        // Any other

        return Entries[idx - 1].Offset - Entries[idx].Offset;
    }//end if
}//end stBLeafOverflowNode::GetSerializedObjectSizeAt()

//------------------------------------------------------------------------------

template < class KeyType, class Comparator >
u_int32_t stBLeafOverflowNode<KeyType, Comparator>::GetFree() {
    u_int32_t usedSize;

    // Fixed size
    usedSize = sizeof (typename tBNode::stBNodeHeader) + sizeof (stBLeafOverflowNodeHeader);

    // Entries
    if (this->SHeader->Occupation > 0) {

        usedSize +=
                // Total size of entries
                (sizeof (stBLeafOverflowNodeEntry) * this->SHeader->Occupation) +
                // Total object size
                (this->Page->GetPageSize() - Entries[this->SHeader->Occupation - 1].Offset);
    }//end if

    return this->Page->GetPageSize() - usedSize;
}//end stBLeafOverflowNode::GetFree()
