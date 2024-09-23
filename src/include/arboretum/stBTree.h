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
 * This file defines the class stBTree.
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 */

#ifndef __STBPLUSTREE_H
#define __STBPLUSTREE_H

#include <arboretum/stUtil.h>
#include <arboretum/stBNode.h>
#include <arboretum/stPageManager.h>
#include <arboretum/stResult.h>

//=============================================================================
// Class template stBPlusTree
//-----------------------------------------------------------------------------

/**
 * This class defines all behavior of the BPlusTree.
 *
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @todo Review documentation.
 * @version 1.0
 * @ingroup bplus
 */
template < class KeyType, class ObjectType, class Comparator = std::less<KeyType> >
class stBPlusTree {
public:

    /**
     * This structure defines the BPlusTree header structure. This type was left
     * public to allow the creation of debug tools.
     */
#pragma pack(1)

    typedef struct stBPlusTreeHeader {
        /**
         * Magic number. This is a short string that must contains the magic
         * string "BPLS". It is used to validate the file.
         */
        char Magic[4];

        /**
         * The root of the tree
         */
        u_int32_t RootPageID;

        /**
         * The leftmost leaf
         */
        u_int32_t LeftmostLeafPageID;

        /**
         * The rightmost leaf
         */
        u_int32_t RightmostLeafPageID;

        /**
         * The height of the tree
         */
        u_int32_t Height;

        /**
         * Total number of objects.
         */
        u_int32_t ObjectCount;

        /**
         * Total number of index nodes.
         */
        u_int32_t IndexNodeCount;

        /**
         * Total number of leaf nodes.
         */
        u_int32_t LeafNodeCount;
    } stBPlusTreeHeader;
#pragma pack()

    /**
     * Type definition for query (total order) result sets.
     */
    typedef stTOResult <ObjectType, KeyType> tResult;

    /**
     * Type definition for result set iterator.
     */
    typedef typename tResult::tItePairs tItePairs;

    /**
     * Creates a new B+ tree using a given page manager. This instance will
     * not claim the ownership of the given page manager. It means that the
     * application must dispose the page manager when it is no longer necessary.
     *
     * @param pageManager The page manager to be used by this tree.
     */
    stBPlusTree(stPageManager * pageManager, bool duplicationAllowed = true);

    /**
     * Class destructor.
     */
    virtual ~stBPlusTree();

    /**
     * This method inserts an object to the tree.
     *
     * @param key The search key.
     * @param obj The object to be added.
     * @return True if the object was successfully inserted and false otherwise.
     */
    bool Insert(KeyType key, ObjectType * obj);

    /**
     * This method return the elements that are equal to the provided key. The
     * result will be a set of pairs object/key.
     *
     * @param key The search key.
     * @warning The instance of tResult returned must be destroyed by user.
     */
    tResult * QueryEqual(KeyType key);

    /**
     * This method return the elements whose key is in the closed interval
     * defined by the provided lower and upper bounds. The result will be a set
     * of pairs object/key.
     *
     * @param lowerBound The interval's lower bound.
     * @param upperBound The interval's upper bound.
     * @warning The instance of tResult returned must be destroyed by user.
     */
    tResult * QueryBetween(KeyType lowerBound, KeyType upperBound);

    /**
     * This method return the elements that are less than or equal to the
     * provided key. The result will be a set of pairs object/key.
     *
     * @param key The search key.
     * @warning The instance of tResult returned must be destroyed by user.
     */
    tResult * QueryLessThanOrEqual(KeyType key);

        /**
     * This method return the elements that are less than the
     * provided key. The result will be a set of pairs object/key.
     *
     * @param key The search key.
     * @warning The instance of tResult returned must be destroyed by user.
     */
    tResult * QueryLessThan(KeyType key);

    /**
     * This method return the elements that are greater than or equal to the
     * provided key. The result will be a set of pairs object/key.
     *
     * @param key The search key.
     * @warning The instance of tResult returned must be destroyed by user.
     */
    tResult * QueryGreaterThanOrEqual(KeyType key);

    /**
     * This method return the elements that are greater than the
     * provided key. The result will be a set of pairs object/key.
     *
     * @param key The search key.
     * @warning The instance of tResult returned must be destroyed by user.
     */
    tResult * QueryGreaterThan(KeyType key);

    /**
     * Returns the height of the tree.
     */
    u_int32_t GetHeight() {
        return Header->Height;
    }//end GetHeight

    /**
     * Returns the maximum number of children of index nodes.
     */
    u_int32_t GetTreeOrder() {
        return 0;
    }//end GetTreeOrder

    /**
     * Returns the number of objects of this tree.
     */
    long GetNumberOfObjects() {
        return Header->ObjectCount;
    }//end GetNumberOfObjects

    /**
     * Returns the average occupation of the index nodes.
     */
    u_int32_t GetAverageIndexNodeOccupation() {
        return 0;
    }//end GetAverageIndexNodeOccupation

    /**
     * Returns the average occupation of the leaf nodes.
     */
    u_int32_t GetAverageLeafNodeOccupation() {
        return 0;
    }//end GetAverageLeafNodeOccupation

    /**
     * Returns the number of nodes of this tree.
     */
    virtual long GetNodeCount() {
        return (Header->IndexNodeCount + Header->LeafNodeCount);
    }//end GetNodeCount

    /**
     * Returns the total number of leaf nodes of this tree.
     */
    long GetLeafNodeCount() {
        return Header->LeafNodeCount;
    }//end GetLeafNodeCount

    /**
     * Returns the total number of index nodes of this tree.
     */
    long GetIndexNodeCount() {
        return Header->IndexNodeCount;
    }//end GetIndexNodeCount
    
    /**
     * Getting the query comparation counter.
     */
    u_int32_t GetComparationFunctionCount() {
        return this->comparationFunctionCount;
    }

private:

    /**
     * B+-tree node type definitions
     */
    typedef stBNode<KeyType, Comparator> tBNode;
    typedef stBIndexNode<KeyType, Comparator> tBIndexNode;
    typedef stBLeafNode<KeyType, Comparator> tBLeafNode;

    /**
     * This enumeration defines the actions to be taken after an call of
     * InsertRecursive.
     */
    enum stInsertAction {
        /**
         * No action required.
         */
        NO_ACT,

        /**
         * Split occured.
         */
        PROMOTION
    }; //end stInsertAction

    /**
     * The tree header. This variable points to data in the HeaderPage.
     */
    stBPlusTreeHeader * Header;

    /**
     * If true, the header mus be written by the page manager.
     */
    bool HeaderUpdate;

    /**
     * Pointer to the header page.
     * This page is kept in memory for faster access.
     */
    stPage * HeaderPage;

    /**
     * Page manager instance.
     */
    stPageManager * PageManager;
    
    /**
     * Indicates if the tree must permit duplicated values
     */
    bool duplicationAllowed;
    
    //u_int32_t comparationQueryCount = 0;
    u_int32_t comparationFunctionCount;
    
    /**
     * Helper function to allow user-defined KeyType comparison.
     *
     * @see less()
     */
    bool equal(const KeyType & l, const KeyType & r) {
        this->comparationFunctionCount++;
        return !Comparator()(l, r) && !Comparator()(r, l);
    }

    /**
     * Helper function to allow user-defined KeyType comparison.
     *
     * @see equal()
     */
    bool less(const KeyType & l, const KeyType & r) {
        this->comparationFunctionCount++;
        return Comparator()(l, r);
    }
    
    /**
     * Reseting the counter - called before all query functions 
     */
    void resetComparationFunctionCount() {
        this->comparationFunctionCount = 0;
    }

    /**
     * This method inserts an object in to the tree recursively.
     * This method is the core of the insertion method. It will manage
     * promotions (splits).
     *
     * <P>For each action, the returning values may assume the following
     * configurations:
     *     - NO_ACT:
     *          - Just return the inner recursion returning value.
     *     - PROMOTION:
     *          - A split occurred in the immediate lower level, manage it.
     *
     * @param currPageID Current page ID.
     * @param newObj The new object to be inserted. This instance will never
     * be destroyed.
     * @param[out] newNodeRepKey The representative key (median) that came up
     * from the immediate lower level node split (PROMOTION).
     * @param[out] newNodeRepRightPageID The representative right page ID
     * that came up from the immediate lower level node split (PROMOTION).
     * Notice that the old son must always be set as the representative left
     * page ID.
     * @return The action to be taken after the returning. See enum
     * stInsertAction for more details.
     */
    int InsertRecursive(u_int32_t currPageID, KeyType newKey, ObjectType * newObj,
            KeyType & newNodeRepKey, u_int32_t & newNodeRepRightPageID);

    /**
     * This method splits a leaf node in 2. This will get 2 nodes and will
     * redistribute the object set between these.
     *
     * <P>The old node's entries are reorganized using a temporary leaf node.
     *
     * @param oldNode The node to be splited.
     * @param newNode The new node.
     * @param newObj The new object to be added. This instance will be consumed
     * by this method.
     * @param prevRep The previous representative.
     * @param promo1 The promoted subtree. If its representative is NULL,
     * the choosen representative is equal to prevRep.
     * @param promo2 The promoted subtree. The representative of this tree will
     * never be the prevRep.
     * @todo redo the FatFactorPromote method.
     */

    void SplitIndex(tBIndexNode * leftNode, tBIndexNode * rightNode,
            typename tBIndexNode::stBIndexNodeEntry newEntry, KeyType & newNodeRepKey);

    void SplitLeaf(tBLeafNode * leftNode, tBLeafNode * rightNode, KeyType newKey,
            ObjectType * newObj, KeyType & newNodeRepKey);


}; //end stBPlusTree

// Include implementation
#include "stBTree-inl.h"

#endif //__STBPLUSTREE_H

