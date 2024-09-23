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
 * This file is the implementation of stBPlusTree methods.
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 */

//==============================================================================
// Class stBPlusTree
//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stBPlusTree<KeyType, ObjectType, Comparator>::stBPlusTree(stPageManager * pageManager, bool duplicationAllowed) {

    PageManager = pageManager;
    
    this->duplicationAllowed = duplicationAllowed;
    
    this->comparationFunctionCount = 0;

    // Load the header page
    HeaderPage = PageManager->GetHeaderPage();
    if (HeaderPage->GetPageSize() < sizeof (stBPlusTreeHeader)) {
#ifdef __stDEBUG__
        std::cout << "The page size is too small to store the tree header." << std::endl;
        throw std::logic_error("The page size is too small to store the tree header.");
#endif //__stDEBUG__
    }//end if

    // Set header
    Header = (stBPlusTreeHeader *) HeaderPage->GetData();

    // Is the tree empty?
    if (PageManager->IsEmpty()) {
        // Yes, create a default header
        HeaderPage->Clear();
        Header->Magic[0] = 'B';
        Header->Magic[1] = 'P';
        Header->Magic[2] = 'L';
        Header->Magic[3] = 'S';
        Header->RootPageID = 0;
        Header->LeftmostLeafPageID = 0;
        Header->RightmostLeafPageID = 0;
        Header->Height = 0;
        Header->ObjectCount = 0;
        Header->IndexNodeCount = 0;
        Header->LeafNodeCount = 0;

        // Schedule header page for update
        HeaderUpdate = true;
    }//end if
    else {
        HeaderUpdate = false;
    }

}//end stBPlusTree::stBPlusTree

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stBPlusTree<KeyType, ObjectType, Comparator>::~stBPlusTree() {

    if (HeaderUpdate) {
        PageManager->WriteHeaderPage(HeaderPage);
    }//end if
    PageManager->ReleasePage(HeaderPage);
}//end stBPlusTree::~stBPlusTree()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
bool stBPlusTree<KeyType, ObjectType, Comparator>::Insert(KeyType key, ObjectType * obj) {

    // Is not there a root ?
    if (Header->RootPageID == 0) {
        // No! We shall create the new LEAF root node.
        stPage * page;
        tBLeafNode * leafNode;
        
        page = PageManager->GetNewPage();
        leafNode = new tBLeafNode(PageManager, page, true);
        
        // Insert the new object.
        unsigned int insertResult = leafNode->Insert(key, obj->GetSerializedSize(), obj->Serialize(), duplicationAllowed);

        if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION) {
            // Oops. There is an error during the insertion.
            // free resources
            delete leafNode;
            PageManager->DisposePage(page);

#ifdef __stDEBUG__
            std::cout << "The page size is too small to store the first object. Increase it!" << std::endl;
            throw std::logic_error("The page size is too small to store the first object. Increase it!");
#endif //__stDEBUG__

            return false;
        }//end if

        // The new object was inserted.
        // Update the root pointer
        Header->RootPageID = page->GetPageID();

        // Update the leftmost and rightmost leaf pointers
        Header->LeftmostLeafPageID = Header->RightmostLeafPageID = Header->RootPageID;

        // Update the Height
        Header->Height++;

        // Update the leaf node count
        Header->LeafNodeCount++;

//        std::cout << std::endl << "root:";
//        for (int i=0; i<leafNode->GetNumberOfEntries(); i++) {
//            std::cout << " " << leafNode->GetKeyAt(i);
//        }
//        std::cout << std::endl;

        // free resources
        delete leafNode;
        PageManager->ReleasePage(page);
    }//end if
    else {
        // Variables passed by reference to InsertRecursive to manage splits
        u_int32_t newNodeRepRightPageID = 0;
        KeyType newNodeRepKey;
        
        // Let's search
        if (InsertRecursive(Header->RootPageID, key, obj, newNodeRepKey, newNodeRepRightPageID) == PROMOTION) {
            // Split occurred! We must create a new INDEX root node
            stPage * page;
            tBIndexNode * indexNode;
            typename tBIndexNode::stBIndexNodeEntry entry;

            page = PageManager->GetNewPage();
            indexNode = new tBIndexNode(PageManager, page, true);

            // Set the new representative entry
            entry.Key = newNodeRepKey;
            entry.RightPageID = newNodeRepRightPageID;

            //std::cout << std::endl << "up: " << entry.Key << " at 0 pageId: " << entry.RightPageID << std::endl;

            // Insert entry in the first position
            if (!indexNode->InsertEntryAt(0, entry)) {
                // Oops, an error occurred!
                // free resources
                delete indexNode;
                PageManager->DisposePage(page);

#ifdef __stDEBUG__
                std::cout << "Error inserting index entry." << std::endl;
                throw logic_error("Error inserting index entry.");
#endif //__stDEBUG__

                return false;
            }//end if

            // Set the old root as entry's left son
            indexNode->SetLeftPageIDAt(0, Header->RootPageID);

            // Update the root pointer
            Header->RootPageID = page->GetPageID();

            // Update the height
            Header->Height++;

            // Update the index node count
            Header->IndexNodeCount++;

//            std::cout << std::endl << "root: ";
//            for (int i=0; i<indexNode->GetNumberOfEntries(); i++) {
//                std::cout << indexNode->GetEntryAt(i).Key;
//            }
//            std::cout << std::endl;

            // free resources
            delete indexNode;
            PageManager->ReleasePage(page);
        }//end if
    }//end else

    // Update object count.
    Header->ObjectCount++;

    // Schedule writing the header page
    HeaderUpdate = true;

    return true;
}//end stBPlusTree::Insert()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
int stBPlusTree<KeyType, ObjectType, Comparator>::InsertRecursive(
    u_int32_t currPageID, KeyType newKey, ObjectType * newObj, KeyType & newNodeRepKey,
    u_int32_t & newNodeRepRightPageID) {

    stPage * currPage; // Current page
    stPage * newPage; // New page
    tBNode * currNode; // Current node
    tBIndexNode * indexNode; // Current index node.
    tBIndexNode * newIndexNode; // New index node for splits
    tBLeafNode * leafNode; // Current leaf node.
    tBLeafNode * newLeafNode; // New leaf node.
    u_int32_t lowerLevelPageID;

    int insertIdx; // Insert index.
    stInsertAction result; // Returning value.

    currPage = PageManager->GetPage(currPageID);
    currNode = tBNode::CreateNode(PageManager, currPage);

    // What shall I do ?
    if (currNode->GetNodeType() == tBNode::INDEX) {
        KeyType subNodeRepKey;
        u_int32_t subNodeRepRightPageID;

        // Index Node cast.
        indexNode = (tBIndexNode *) currNode;
        
//        std::cout << std::endl << "index before:";
//        for (int i=0; i<indexNode->GetNumberOfEntries(); i++) {
//            std::cout << " " << indexNode->GetEntryAt(i).Key;
//        }
//        std::cout << std::endl;

        // Which subtree must be traversed?
        insertIdx = indexNode->Find(newKey);
//        std::cout << "subtree: " << insertIdx << std::endl;

        // Which lower level page should be traversed?
        if (insertIdx < indexNode->GetNumberOfEntries()) {
            lowerLevelPageID = indexNode->GetLeftPageIDAt(insertIdx);
        }
        else {
            lowerLevelPageID = indexNode->GetRightPageIDAt(insertIdx - 1);
        }

        // Try to insert
        if (InsertRecursive(lowerLevelPageID, newKey, newObj, subNodeRepKey, subNodeRepRightPageID) == PROMOTION) {
            // A split occurred in the lower level
            // Set the new representative entry
            typename tBIndexNode::stBIndexNodeEntry newEntry;
            newEntry.Key = subNodeRepKey;
            newEntry.RightPageID = subNodeRepRightPageID;

            //std::cout << std::endl << "index up: " << newEntry.Key << " at " << insertIdx << " pageId: " << newEntry.RightPageID << std::endl;

            // Try to insert newEntry in this index node
            if (indexNode->InsertEntryAt(insertIdx, newEntry)) {
                // Success, nothing to do in the upper levels.
                result = NO_ACT;
            }//end if
            else {
                // It is necessary to split this index node
                newPage = PageManager->GetNewPage();
                newIndexNode = new tBIndexNode(PageManager, newPage, true);

                // Split it
                SplitIndex(indexNode, newIndexNode, newEntry, newNodeRepKey);
                newNodeRepRightPageID = newPage->GetPageID();

                // Update the index node count
                Header->IndexNodeCount++;

                // Free resources
                delete newIndexNode;
                PageManager->ReleasePage(newPage);

                // Report split
                result = PROMOTION;
            }//end else
        }//end if
        else {
            // Nothing to do, just return.
            result = NO_ACT;
        }//end else
        
//        std::cout << std::endl << "index after:";
//        for (int i=0; i<indexNode->GetNumberOfEntries(); i++) {
//            std::cout << " " << indexNode->GetEntryAt(i).Key;
//        }
//        std::cout << std::endl << "Free: " << indexNode->GetFree() << std::endl;
    }//end if
    else {
        // currNode is a leaf node
        leafNode = (tBLeafNode *) currNode;
        
//        std::cout << std::endl << "leaf before:";
//        for (int i=0; i<leafNode->GetNumberOfEntries(); i++) {
//            std::cout << " " << leafNode->GetKeyAt(i);
//        }
//        std::cout << std::endl << "leaf Free: " << leafNode->GetFree() << std::endl;
        
        // Try to insert the element
        unsigned int status = leafNode->Insert(newKey, newObj->GetSerializedSize(), newObj->Serialize(), duplicationAllowed);

        if (status == tBNode::ERROR) {
            // Unrecoverable error when inserting element. Abandon the insertion.
            result = NO_ACT;
        }//end if
        else if (status == tBNode::SUCCESS) {
            // Success, nothing to do in the upper levels.
            result = NO_ACT;
        }//end if
        else if (status == tBNode::DUPLICATION) {
            // Duplication in a tree that doesn't allow it, nothing to do in the upper levels.
            result = NO_ACT;
        }//end if
        else if (status == tBNode::SUCCESS_NEWOVERFLOWNODE) {
            // Update node count
            Header->LeafNodeCount++;

            // Success, nothing to do in the upper levels.
            result = NO_ACT;
        }//end if
        else {
            // A split is required
            newPage = PageManager->GetNewPage();
            newLeafNode = new tBLeafNode(PageManager, newPage, true);

            // Split leafNode
            SplitLeaf(leafNode, newLeafNode, newKey, newObj, newNodeRepKey);
            newNodeRepRightPageID = newPage->GetPageID();

            // If leafNode was the rightmost leaf, the RightmostLeaf pointer
            // needs to be updated.
            if (Header->RightmostLeafPageID == leafNode->GetPageID()) {
                Header->RightmostLeafPageID = newLeafNode->GetPageID();
            }

            // Update the leaf node count
            Header->LeafNodeCount++;

            // Free resources
            delete newLeafNode;
            PageManager->ReleasePage(newPage);

            // Report split
            result = PROMOTION;
        }//end else
        
//        std::cout << std::endl << "leaf after:";
//        for (int i=0; i<leafNode->GetNumberOfEntries(); i++) {
//            std::cout << " " << leafNode->GetKeyAt(i);
//        }
//        std::cout << std::endl << "leaf Free: " << leafNode->GetFree() << std::endl;
    }//end else

    // Clean home
    delete currNode;
    PageManager->ReleasePage(currPage);

    return result;
}//end stBPlusTree::InsertRecursive()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
void stBPlusTree<KeyType, ObjectType, Comparator>::SplitIndex(
    tBIndexNode * leftNode, tBIndexNode * rightNode,
    typename tBIndexNode::stBIndexNodeEntry newEntry, KeyType & newNodeRepKey) {

    u_int32_t leftMedianIdx;
    u_int32_t leftNumEntries;
    u_int32_t idx;

    // Get the median element and the number of entries of the left node
    leftMedianIdx = leftNode->FindMedian();
    leftNumEntries = leftNode->GetNumberOfEntries();

    // Copy left entries greater than the median to the right node
    for (idx = leftMedianIdx; idx < leftNumEntries; idx++) {
        rightNode->InsertEntryAt((idx - leftMedianIdx), leftNode->GetEntryAt(idx));
    }//end for

    // Delete such entries from the left node
    // They are deleted from the last to the median to avoid idx OutOfBounds exceptions.
    for (idx = leftNumEntries - 1; idx >= leftMedianIdx; idx--) {
        leftNode->DeleteEntryAt(idx);
    }//end for

    // In which node should newEntry be inserted?
    if (newEntry.Key <= leftNode->GetEntryAt(leftMedianIdx - 1).Key) {
        // Insert into the left one
        idx = leftNode->Find(newEntry.Key);
        leftNode->InsertEntryAt(idx, newEntry);

        // Set the last element of the left node as the median element to come up
        leftNumEntries = leftNode->GetNumberOfEntries();
        newNodeRepKey = leftNode->GetEntryAt(leftNumEntries - 1).Key;

        // Set the leftmost pointer of the right node as the entry's (right) pointer
        rightNode->SetLeftPageIDAt(0, leftNode->GetEntryAt(leftNumEntries - 1).RightPageID);

        // Delete entry
        leftNode->DeleteEntryAt(leftNumEntries - 1);
    }//end if
    else {
        // Insert into the right node
        idx = rightNode->Find(newEntry.Key);
        rightNode->InsertEntryAt(idx, newEntry);

        // Set the first element of the right node as the median element to come up
        newNodeRepKey = rightNode->GetEntryAt(0).Key;

        // Set the leftmost pointer of the right node as the entry's (right) pointer
        rightNode->SetLeftPageIDAt(0, rightNode->GetEntryAt(0).RightPageID);

        // Delete entry
        rightNode->DeleteEntryAt(0);
    }//end else

//    std::cout << std::endl << "index left:";
//    for (int i=0; i<leftNode->GetNumberOfEntries(); i++) {
//        std::cout << " " << leftNode->GetEntryAt(i).Key;
//    }
//    std::cout << std::endl << "left Free: " << leftNode->GetFree() << std::endl;
//
//    std::cout << std::endl << "index right:";
//    for (int i=0; i<rightNode->GetNumberOfEntries(); i++) {
//        std::cout << " " << rightNode->GetEntryAt(i).Key;
//    }
//    std::cout << std::endl << "left Free: " << rightNode->GetFree() << std::endl;

}//end stBPlusTree::SplitIndex()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
void stBPlusTree<KeyType, ObjectType, Comparator>::SplitLeaf(
    tBLeafNode * leftNode, tBLeafNode * rightNode, KeyType newKey, ObjectType * newObj,
    KeyType & newNodeRepKey) {

    u_int32_t numEntries;
    u_int32_t medianIdx;
    long idx;

    numEntries = leftNode->GetNumberOfEntries();
    medianIdx = leftNode->FindMedian();

//    std::cout << "medianIdx = " << medianIdx << " -- L / R: " << leftNode->GetNumberOfEntries() << " / " << rightNode->GetNumberOfEntries() << std::endl;
    
//    todo: eliminar comentarios abaixo
//    if (medianIdx == 0) {
//    if ((medianIdx == 0) || ((medianIdx == numEntries - 1) && (newKey == leftNode->GetKeyAt(medianIdx)))) {
//    if ((medianIdx == 0) || (newKey > leftNode->GetKeyAt(medianIdx))) {

    // Is node full of elements with the same key?    
    if ((medianIdx == 0) || (medianIdx == (leftNode->GetNumberOfEntriesNoOverflow() - 1))) {
        // Yes. Check if newKey should be inserted at right
        if (newKey > leftNode->GetKeyAt(medianIdx)) {
            unsigned int insertResult = rightNode->Insert(newKey, newObj->GetSerializedSize(), newObj->Serialize(), duplicationAllowed);

            if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION) {
                throw std::logic_error("1) The page size is too small to store the element.");
            }//end if

//            std::cout << std::endl << "left: ";
//            for (int i = 0; i < leftNode->GetNumberOfEntries(); i++)
//                std::cout << leftNode->GetKeyAt(i) << " ";
//            std::cout << std::endl << "right: ";
//            for (int i = 0; i < rightNode->GetNumberOfEntries(); i++)
//                std::cout << rightNode->GetKeyAt(i) << " ";

            return;
        }
    }//end if
    
//    std::cout << "numEntries: " << numEntries
//             << " | medianIdx: " << medianIdx
//             << " | newKey: " << newKey
//             << " | [" << leftNode->GetKeyAt(0) 
//             << "," << leftNode->GetKeyAt(medianIdx) 
//             << "," << leftNode->GetKeyAt(numEntries-1)
//             << "]" << std::endl;
    
//    if (leftNode->GetKeyAt(0) != leftNode->GetKeyAt(numEntries-1)) {
        // Copy entries greater than or equal to the median to the right node. We perform this
        // from the median to the end to avoid unnecessary object data memory
        // movements when inserting them in the right node.
        for (idx = medianIdx; idx < numEntries; idx++) {
            unsigned int insertResult = rightNode->Insert(leftNode->GetKeyAt(idx), leftNode->GetSerializedObjectSizeAt(idx), leftNode->GetSerializedObjectAt(idx), duplicationAllowed);

//            std::cout << idx << " ";

            if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION && insertResult != tBNode::SUCCESS_NEWOVERFLOWNODE) {
                std::cout << "insertResult: " << insertResult << " L / R: " << leftNode->GetNumberOfEntries() << " / " << rightNode->GetNumberOfEntries() << " serSize: " << newObj->GetSerializedSize() << std::endl;
                throw std::logic_error("2) The page size is too small to store the element.");
            };
        }//end for

        // Delete such entries from the left node. We perform this from the end to
        // the median to avoid unnecessary object data memory movements when deleting
        // them from the left node.
        for (idx = numEntries; idx > medianIdx; idx--) {
            leftNode->DeleteElementAt(idx - 1);
        }//end for
//    }//end if

    // Insert the new element.
    // Must the new element be inserted in the left node?
    if ((leftNode->GetNumberOfEntries() == 0) || (newKey <= leftNode->GetKeyAt(leftNode->GetNumberOfEntries() - 1))) {
        unsigned int insertResult = leftNode->Insert(newKey, newObj->GetSerializedSize(), newObj->Serialize(), duplicationAllowed);
        
        if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION) {
            std::cout << "insertResult: " << insertResult << " L / R: " << leftNode->GetNumberOfEntries() << " / " << rightNode->GetNumberOfEntries() << " serSize: " << newObj->GetSerializedSize() << std::endl;
            throw std::logic_error("3) The page size is too small to store the element.");
        };
    }//end if
    // Must it be inserted in the right node?
    else if (newKey >= rightNode->GetKeyAt(0)) {
        unsigned int insertResult = rightNode->Insert(newKey, newObj->GetSerializedSize(), newObj->Serialize(), duplicationAllowed);
        
        if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION) {
            std::cout << "insertResult: " << insertResult << std::endl;
            throw std::logic_error("4) The page size is too small to store the element.");
        };
    }// So, insert it in the node that is more free.
    else if (leftNode->GetFree() <= rightNode->GetFree()) {
        unsigned int insertResult = leftNode->Insert(newKey, newObj->GetSerializedSize(), newObj->Serialize(), duplicationAllowed);
        
        if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION) {
            throw std::logic_error("5) The page size is too small to store the element.");
        };
    } else {
        unsigned int insertResult = rightNode->Insert(newKey, newObj->GetSerializedSize(), newObj->Serialize(), duplicationAllowed);
        
        if (insertResult != tBNode::SUCCESS && insertResult != tBNode::DUPLICATION) {
            throw std::logic_error("6) The page size is too small to store the element.");
        };
    }//end else

//    std::cout << "left: ";
//    for (int i = 0; i < leftNode->GetNumberOfEntries(); i++)
//        std::cout << leftNode->GetKeyAt(i) << " ";
//    std::cout << "\nright: ";
//    for (int i = 0; i < rightNode->GetNumberOfEntries(); i++)
//        std::cout << rightNode->GetKeyAt(i) << " ";

    // Adjust the LeafNode links
    rightNode->SetPreviousPageID(leftNode->GetPageID());
    if (leftNode->GetNextPageID() != 0) {
        rightNode->SetNextPageID(leftNode->GetNextPageID());
        stPage * nextPage = PageManager->GetPage(rightNode->GetNextPageID());
        tBLeafNode * nextNode = new tBLeafNode(PageManager, nextPage, false);
        nextNode->SetPreviousPageID(rightNode->GetPageID());
        delete nextNode;
    }
    leftNode->SetNextPageID(rightNode->GetPageID());

    // Set the representative key of these two nodes as the last element
    // of leftNode
    newNodeRepKey = leftNode->GetKeyAt(leftNode->GetNumberOfEntries() - 1);

}//end stBPlusTree::SplitLeaf()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stTOResult <ObjectType, KeyType> * stBPlusTree<KeyType, ObjectType, Comparator>::QueryEqual(KeyType key) {

    this->resetComparationFunctionCount();
    
    u_int32_t currPageID;
    stPage * currPage; // Current page
    tBNode * currNode; // Current node
    tBIndexNode * indexNode; // Current index node.
    tBLeafNode * leafNode; // Current leaf node.

    tResult * result;
    ObjectType * obj;

    u_int32_t idx;
    
    this->resetComparationFunctionCount();

    // Create result
    result = new tResult();
    result->SetQueryInfo(TO_EQUALQUERY, key, key);

    currPageID = Header->RootPageID;

    // Let's search
    while (currPageID != 0) {
        currPage = PageManager->GetPage(currPageID);
        currNode = tBNode::CreateNode(PageManager, currPage);

        // What shall I do ?
        if (currNode->GetNodeType() == tBNode::INDEX) {

            // Index Node cast.
            indexNode = (tBIndexNode *) currNode;

            // Which subtree must be traversed?
            idx = indexNode->Find(key);
//            std::cout << "subtree: " << idx << std::endl;

            // Which lower level page should be traversed?
            if (idx < indexNode->GetNumberOfEntries()) {
                currPageID = indexNode->GetLeftPageIDAt(idx);
            } else {
                currPageID = indexNode->GetRightPageIDAt(idx - 1);
            }
        } else {
            // currNode is a leaf node
            // Leaf Node cast.
            leafNode = (tBLeafNode *) currNode;

            if (leafNode->FindFirst(key, idx)) {
                // The pair insertions will be O(1), because of the iterator position
                while ((idx < leafNode->GetNumberOfEntries()) && (equal(key, leafNode->GetKeyAt(idx)))) {
                    obj = new ObjectType();
                    obj->Unserialize(leafNode->GetSerializedObjectAt(idx), leafNode->GetSerializedObjectSizeAt(idx));
                    result->AddPair(obj, leafNode->GetKeyAt(idx), result->endPairs());
                    idx++;
                }
            }
            currPageID = 0;
        }

        // Clean home
        delete currNode;
        PageManager->ReleasePage(currPage);
    }//end while

    return result;
}// end stBPlusTree::QueryEqual()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stTOResult <ObjectType, KeyType> * stBPlusTree<KeyType, ObjectType, Comparator>::QueryBetween(KeyType lowerBound, KeyType upperBound) {

    this->resetComparationFunctionCount();
    
    u_int32_t currPageID;
    u_int32_t nextLeafPageID;
    stPage * currPage; // Current page
    stPage * nextLeafPage; // Next leaf page
    tBNode * currNode; // Current node
    tBIndexNode * indexNode; // Current index node.
    tBLeafNode * leafNode; // Current leaf node.
    tBLeafNode * nextLeafNode; // Next leaf node

    tResult * result;
    ObjectType * obj;

    u_int32_t idx;
    
    this->resetComparationFunctionCount();

    // Create result
    result = new tResult();
    result->SetQueryInfo(TO_BETWEENQUERY, lowerBound, upperBound);

    currPageID = Header->RootPageID;

    // Let's search
    while (currPageID != 0) {
        currPage = PageManager->GetPage(currPageID);
        currNode = tBNode::CreateNode(PageManager, currPage);

        // What shall I do ?
        if (currNode->GetNodeType() == tBNode::INDEX) {

            // Index Node cast.
            indexNode = (tBIndexNode *) currNode;

            // Which subtree must be traversed?
            idx = indexNode->Find(lowerBound);
//            std::cout << "subtree: " << idx << std::endl;

            // Which lower level page should be traversed?
            if (idx < indexNode->GetNumberOfEntries()) {
                currPageID = indexNode->GetLeftPageIDAt(idx);
            } else {
                currPageID = indexNode->GetRightPageIDAt(idx - 1);
            }
        } else {
            // currNode is a leaf node
            // Leaf Node cast.
            leafNode = (tBLeafNode *) currNode;

            leafNode->FindFirst(lowerBound, idx);

            // Get entries in the current node.
            while ((idx < leafNode->GetNumberOfEntries()) && (!less(upperBound, leafNode->GetKeyAt(idx)))) {
                obj = new ObjectType();
                obj->Unserialize(leafNode->GetSerializedObjectAt(idx), leafNode->GetSerializedObjectSizeAt(idx));
                result->AddPair(obj, leafNode->GetKeyAt(idx), result->endPairs());
                idx++;
            }

            // Get entries in the following leaf nodes.
            nextLeafPageID = leafNode->GetNextPageID();

            while (nextLeafPageID != 0) {
                nextLeafPage = PageManager->GetPage(nextLeafPageID);
                nextLeafNode = new tBLeafNode(PageManager, nextLeafPage, false);

                idx = 0;
                // Get entries in the current node.
                while ((idx < nextLeafNode->GetNumberOfEntries()) && (!less(upperBound, nextLeafNode->GetKeyAt(idx)))) {
                    obj = new ObjectType();
                    obj->Unserialize(nextLeafNode->GetSerializedObjectAt(idx), nextLeafNode->GetSerializedObjectSizeAt(idx));
                    result->AddPair(obj, nextLeafNode->GetKeyAt(idx), result->endPairs());
                    idx++;
                }
                
                // if the upperbound is already found
                if( idx == 0 || !less(upperBound, nextLeafNode->GetKeyAt(idx-1)) ) {
                    delete nextLeafNode;
                    PageManager->ReleasePage(nextLeafPage);
                    break;
                }

                // Schedule visiting the next leaf node.
                nextLeafPageID = nextLeafNode->GetNextPageID();

                // Free leaf node resources
                delete nextLeafNode;
                PageManager->ReleasePage(nextLeafPage);
            }

            currPageID = 0;
        }

        // Clean home
        delete currNode;
        PageManager->ReleasePage(currPage);
    }//end while

    return result;
}// end stBPlusTree::QueryBetween()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stTOResult <ObjectType, KeyType> * stBPlusTree<KeyType, ObjectType, Comparator>::QueryLessThanOrEqual(KeyType key) {

    this->resetComparationFunctionCount();
    
    u_int32_t currPageID;
    stPage * currPage; // Current page
    tBLeafNode * leafNode; // Current leaf node.

    tResult * result;
    ObjectType * obj;

    u_int32_t idx;
    bool stop;
    
    this->resetComparationFunctionCount();

    // Create result
    result = new tResult();
    result->SetQueryInfo(TO_LESSTHANOREQUALQUERY, 0, key);

    // Get entries from the leftmost leaf node.
    currPageID = Header->LeftmostLeafPageID;
    stop = false;

    while (currPageID != 0) {
        currPage = PageManager->GetPage(currPageID);
        leafNode = new tBLeafNode(PageManager, currPage, false);

        // Get entries
        for (idx = 0; idx < leafNode->GetNumberOfEntries(); idx++) {
            if (less(key, leafNode->GetKeyAt(idx))) {
                stop = true;
                break;
            }
            obj = new ObjectType();
            obj->Unserialize(leafNode->GetSerializedObjectAt(idx), leafNode->GetSerializedObjectSizeAt(idx));
            result->AddPair(obj, leafNode->GetKeyAt(idx), result->endPairs());
        }//end for

        // Schedule visiting the next leaf node.
        if (!stop) {
            currPageID = leafNode->GetNextPageID();
        }
        else {
            currPageID = 0;
        }

        // Free leaf node resources
        delete leafNode;
        PageManager->ReleasePage(currPage);
    }//end while

    return result;
}// end stBPlusTree::QueryLessThanOrEqual()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stTOResult <ObjectType, KeyType> * stBPlusTree<KeyType, ObjectType, Comparator>::QueryLessThan(KeyType key) {

    this->resetComparationFunctionCount();
    
    u_int32_t currPageID;
    stPage * currPage; // Current page
    tBLeafNode * leafNode; // Current leaf node.

    tResult * result;
    ObjectType * obj;

    u_int32_t idx;
    bool stop;
    
    this->resetComparationFunctionCount();

    // Create result
    result = new tResult();
    result->SetQueryInfo(TO_LESSTHANQUERY, 0, key);

    // Get entries from the leftmost leaf node.
    currPageID = Header->LeftmostLeafPageID;
    stop = false;

    while (currPageID != 0) {
        currPage = PageManager->GetPage(currPageID);
        leafNode = new tBLeafNode(PageManager, currPage, false);

        // Get entries
        for (idx = 0; idx < leafNode->GetNumberOfEntries(); idx++) {
            if (!less(leafNode->GetKeyAt(idx), key)) {
                stop = true;
                break;
            }
            obj = new ObjectType();
            obj->Unserialize(leafNode->GetSerializedObjectAt(idx), leafNode->GetSerializedObjectSizeAt(idx));
            result->AddPair(obj, leafNode->GetKeyAt(idx), result->endPairs());
        }//end for

        // Schedule visiting the next leaf node.
        if (!stop) {
            currPageID = leafNode->GetNextPageID();
        }
        else {
            currPageID = 0;
        }

        // Free leaf node resources
        delete leafNode;
        PageManager->ReleasePage(currPage);
    }//end while

    return result;
}// end stBPlusTree::QueryLessThan()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stTOResult <ObjectType, KeyType> * stBPlusTree<KeyType, ObjectType, Comparator>::QueryGreaterThanOrEqual(KeyType key) {

    this->resetComparationFunctionCount();
    
    u_int32_t currPageID;
    stPage * currPage; // Current page
    tBLeafNode * leafNode; // Current leaf node.

    tResult * result;
    ObjectType * obj;

    u_int32_t idx;
    bool stop;
    
    this->resetComparationFunctionCount();

    // Create result
    result = new tResult();
    result->SetQueryInfo(TO_GREATERTHANOREQUALQUERY, key, 0);

    // Get entries from the rightmost leaf node.
    currPageID = Header->RightmostLeafPageID;
    stop = false;

    while (currPageID != 0) {
        currPage = PageManager->GetPage(currPageID);
        leafNode = new tBLeafNode(PageManager, currPage, false);

        // Get entries
        for (idx = leafNode->GetNumberOfEntries(); idx > 0; idx--) { // I will use (idx - 1) to avoid negative values in u_int32_t
            if (less(leafNode->GetKeyAt(idx - 1), key)) {
                stop = true;
                break;
            }
            obj = new ObjectType();
            obj->Unserialize(leafNode->GetSerializedObjectAt(idx - 1), leafNode->GetSerializedObjectSizeAt(idx - 1));
            result->AddPair(obj, leafNode->GetKeyAt(idx - 1), result->beginPairs());
        }//end for

        // Schedule visiting the next leaf node.
        if (!stop) {
            currPageID = leafNode->GetPreviousPageID();
        }
        else {
            currPageID = 0;
        }

        // Free leaf node resources
        delete leafNode;
        PageManager->ReleasePage(currPage);
    }//end while

    return result;
}// end stBPlusTree::QueryGreaterThanOrEqual()

//------------------------------------------------------------------------------

template < class KeyType, class ObjectType, class Comparator >
stTOResult <ObjectType, KeyType> * stBPlusTree<KeyType, ObjectType, Comparator>::QueryGreaterThan(KeyType key) {

    this->resetComparationFunctionCount();
    
    u_int32_t currPageID;
    stPage * currPage; // Current page
    tBLeafNode * leafNode; // Current leaf node.

    tResult * result;
    ObjectType * obj;

    u_int32_t idx;
    bool stop;
    
    this->resetComparationFunctionCount();

    // Create result
    result = new tResult();
    result->SetQueryInfo(TO_GREATERTHANQUERY, key, 0);

    // Get entries from the rightmost leaf node.
    currPageID = Header->RightmostLeafPageID;
    stop = false;

    while (currPageID != 0) {
        currPage = PageManager->GetPage(currPageID);
        leafNode = new tBLeafNode(PageManager, currPage, false);

        // Get entries
        for (idx = leafNode->GetNumberOfEntries(); idx > 0; idx--) { // I will use (idx - 1) to avoid negative values in u_int32_t
            if (!less(key, leafNode->GetKeyAt(idx - 1))) {
                stop = true;
                break;
            }
            obj = new ObjectType();
            obj->Unserialize(leafNode->GetSerializedObjectAt(idx - 1), leafNode->GetSerializedObjectSizeAt(idx - 1));
            result->AddPair(obj, leafNode->GetKeyAt(idx - 1), result->beginPairs());
        }//end for

        // Schedule visiting the next leaf node.
        if (!stop) {
            currPageID = leafNode->GetPreviousPageID();
        }
        else {
            currPageID = 0;
        }

        // Free leaf node resources
        delete leafNode;
        PageManager->ReleasePage(currPage);
    }//end while

    return result;
}// end stBPlusTree::QueryGreaterThan()

//------------------------------------------------------------------------------
