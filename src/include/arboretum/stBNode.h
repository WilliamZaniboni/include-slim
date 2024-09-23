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
 * This file defines the BTree nodes.
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @todo Review of documentation
 */

#ifndef __STBNODE_H
#define __STBNODE_H

#include <vector>
#include <iostream>
#include <stddef.h>
#include <arboretum/stPage.h>
#include <arboretum/stPageManager.h>

//-----------------------------------------------------------------------------
// Class stBNode
//-----------------------------------------------------------------------------

/**
 * This abstract class is the basic BTree node. All classes that implement
 * BTree nodes must extend this class.
 *
 * <p>The main function of this class is to provide a way to identify a disk node
 * and create the required node instance to manipulate the node.
 *
 * <p>The structure of Index Node follows:
 * @image html bnode.png "BTree node structure"
 *
 * <p>The <b>Header</b> holds the information about the node itself.
 *     - Type: Type of this node. It may be stBNode::INDEX, stBNode::LEAF or stBNode::LEAF_OVERFLOW.
 *     - Occupation: Number of entries in this node.
 *
 * <p>The <b>Node Data</b> is the segment of the node which holds the particular information
 * of each type of the node. This class does not know how this information is organized.
 *
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @version 1.0
 * @todo Documentation review.
 * @see stBIndexNode
 * @see stBLeafNode
 * @see stBLeafOverflowNode
 * @ingroup btree
 */
template < class KeyType, class Comparator = std::less <KeyType> >
class stBNode {
public:

    /**
     * Node type.
     */
    enum stBNodeType {
        /**
         * ID of an index node.
         */
        INDEX = 0x4449, // In little endian "ID"

        /**
         * ID of a leaf node.
         */
        LEAF = 0x464C, // In little endian "LF"

        /**
         * ID of a leaf overflow node.
         */
        LEAF_OVERFLOW = 0x4F4C // In little endian "LO"
    }; //end stBNodeType

    /**
     * Return codes.
     */
    enum stBNodeReturnCode {
        /**
         * Sucess.
         */
        SUCCESS,

        /**
         * Sucessful insertion into a new BLeafOverflowNode.
         */
        SUCCESS_NEWOVERFLOWNODE,

        /**
         * Node full.
         */
        NODE_FULL,

        /**
         * Node is too small to store a single entry.
         */
        ERROR,
        
        /**
         * Node already exists in the tree that doesn't allow duplication
         */
        DUPLICATION
    };

    /**
     * This method will write the associated page, if necessary, and dispose
     * this instance and all associated resources.
     */
    virtual ~stBNode() {
        if (IsPageModified)
            PageManager->WritePage(Page);
    }//end ~stBNode()

    /**
     * Returns the type of this BTree node (Index, Leaf or LeafOverflow).
     *
     * @return the type of node.
     * @see stNodeType
     */
    stBNodeType GetNodeType() {
        return SHeader->Type;
    }//end GetNodeType()

    /**
     * Returns the number of entries in this node.
     *
     * @return the number of entries.
     */
    virtual u_int32_t GetNumberOfEntries() {
        return SHeader->Occupation;
    }//end GetNumberOfEntries()

    /**
     * Returns the associated page.
     *
     * @return The associated page.
     */
    stPage * GetPage() {
        return Page;
    }//end GetPage

    /**
     * Returns the ID of the associated page.
     *
     * @return The ID of the associated page.
     */
    u_int32_t GetPageID() {
        return Page->GetPageID();
    }//end GetPage

    /**
     * This is a virtual method that defines a interface for the instantiate
     * the correct specialization of this class.
     *
     * @param pageManager An instance of stPageManager.
     * @param page The page to be associated to the node.
     * @return An instance of the correct specialization.
     */
    static stBNode<KeyType, Comparator> * CreateNode(stPageManager * pageManager, stPage * page);

    /**
     * Remove All entries.
     */
    void RemoveAll() {

#ifdef __stDEBUG__
        u_int16_t type;
        type = SHeader->Type;
        Page->Clear();
        SHeader->Type = type;
#else
        SHeader->Occupation = 0;
#endif //__stDEBUG__
        IsPageModified = true;
    }//end RemoveAll()

protected:
    /**
     * This is the structure of the Header of a BTree node.
     */
#pragma pack(1)

    typedef struct stBNodeHeader {
        /**
         * Node type.
         */
        stBNodeType Type;

        /**
         * Number of entries.
         */
        u_int32_t Occupation;
    } stBNodeHeader; //end stBNodeHeader
#pragma pack()

    /**
     * Header of this page.
     */
    stBNodeHeader * SHeader;

    /**
     * The page manager instance.
     */
    stPageManager * PageManager;

    /**
     * The page related with this node.
     */
    stPage * Page;

    /**
     * Controls if the related page must be written on disk.
     */
    bool IsPageModified;

    /**
     * Creates a new instance of this class.
     *
     * @param pageManager An instance of stPageManager.
     * @param page The page to be associated to the node.
     * @param create Indicates if the node should be cleared (true) or not (false).
     */
    stBNode(stPageManager * pageManager, stPage * page, bool create);

    /**
     * Helper function to allow user-defined KeyType comparison.
     *
     * @see less()
     */
    static bool equal(const KeyType & l, const KeyType & r) {
        return !Comparator()(l, r) && !Comparator()(r, l);
    }

    /**
     * Helper function to allow user-defined KeyType comparison.
     *
     * @see equal()
     */
    static bool less(const KeyType & l, const KeyType & r) {
        return Comparator()(l, r);
    }

}; //end stBNode


//-----------------------------------------------------------------------------
// Class stBIndexNode
//-----------------------------------------------------------------------------
/**
 * This class implements the index node of the BTree.
 *
 * <P>The BTree index node...
 *
 * <P>The structure of Index Node follows:
 * @image html indexnode.png "Index node structure"
 *
 * <P>The <b>Header</b> holds the information about the node itself.
 *     - Type: Type of this node. It is always stBNode::INDEX (0x4449).
 *     - Occupation: Number of entries in this node.
 *
 * <P>The <b>Entry</b> holds the information of the link to the other node.
 *  - PageID: The identifier of the page which holds the root of the sub tree.
 *       - Distance: The distance of this object from the representative object.
 *       - NEntries: Number of objects in the sub tree.
 *       - Radius: Radius of the sub tree.
 *       - Offset: The offset of the object in the page. DO NOT MODIFY ITS VALUE.
 *
 * <P>The <b>Object</b> is an array of bytes that holds the information required to rebuild
 * the original object.
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @todo Documentation review.
 * @see stBNode
 * @see stBLeafNode
 * @see stBLeafOverflowNode
 * @ingroup btree
 */
// +------------------------------------------------------------------------------------+
// | Type | Occupation | LeftmostPageID | Key0 | RightPageID0 |...| Keyn | RightPageIDn |
// +------------------------------------------------------------------------------------+

template < class KeyType, class Comparator >
class stBIndexNode : public stBNode <KeyType, Comparator> {
public:
    /**
     * This type represents a btree index node entry.
     */
#pragma pack(1)

    typedef struct stBIndexNodeEntry {
        /**
         * Search key.
         */
        KeyType Key;

        /**
         * ID of the left son's page.
         */
        u_int32_t RightPageID;
    } stBIndexNodeEntry; //end stIndexEntry
#pragma pack()

    /**
     * Creates a new instance of this class. The paramenter <i>page</i> is an
     * instance of stPage that hold the node data.
     *
     * <P>The parameter <i>create</i> tells what operation will
     * be performed. True means that the page will be initialized and false
     * means that the page will be used as it is. The default value is false.
     *
     * @param pageManager An instance of stPageManager.
     * @param page The page that hold the data of this node.
     * @param create The operation to be performed.
     */
    stBIndexNode(stPageManager * pageManager, stPage * page, bool create = false);

    /**
     * Class destructor.
     */
    virtual ~stBIndexNode() {
    }

    /**
     * Inserts a new entry to this node.
     *
     * <P>This method will fail 
     *
     * @param idx The position of the entry in the array of entries.
     * @param entry The entry to be inserted.
     * @return True if entry was inserted or false if there is not enough space to hold the new entry
     * @see SetLeftPageIDAt()
     */
    bool InsertEntryAt(u_int32_t idx, stBIndexNodeEntry &entry);

    /**
     * Returns the reference of the desired entry. You may use this method to
     * read and modify the entry information.
     *
     * @param idx The position of the entry in the array of entries.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return A reference to the desired entry.
     */
    stBIndexNodeEntry & GetEntryAt(u_int32_t idx) {
#ifdef __stDEBUG__
        if (idx >= this->SHeader->Occupation) {
            throw std::logic_error("idx value is out of range. stBIndexNode::GetEntryAt()");
        }//end if
#endif //__stDEBUG__

        return Entries[idx];
    }//end GetEntryAt()

    /**
     * Deletes the idx-th entry.
     *
     * @param idx The position of the entry in the array of entries.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     */
    void DeleteEntryAt(u_int32_t idx) {
#ifdef __stDEBUG__
        if (idx >= this->SHeader->Occupation) {
            std::cout << idx << std::endl;
            throw std::logic_error("idx value is out of range. stBIndexNode::DeleteEntryAt()");
        }//end if
#endif //__stDEBUG__

        for (u_int32_t i = idx; i < (this->SHeader->Occupation - 1); i++) {
            Entries[i] = Entries[i + 1];
        }
        this->SHeader->Occupation--;

        this->IsPageModified = true;
    }//end DeleteEntryAt()

    /**
     * Sets the pageID of the key's left son.
     *
     * @param idx The position of the desired key in the array of entries
     * @param leftPageID The pageID of the left son.
     * @see InsertEntryAt()
     */
    void SetLeftPageIDAt(u_int32_t keyIdx, u_int32_t leftPageID);

    /**
     * Gets the pageID of the key's left son.
     *
     * @param keyIdx The position of the desired key in the array of entries
     * @return The pageID of the left son.
     * @see GetRightPageIDAt()
     */
    u_int32_t GetLeftPageIDAt(u_int32_t keyIdx);

    /**
     * Gets the pageID of the key's right son.
     *
     * @param idx The position of the desired key in the array of entries
     * @return The pageID of the right son.
     * @see GetLeftPageIDAt()
     */
    u_int32_t GetRightPageIDAt(u_int32_t keyIdx);

    /*
     * Find the first key's occurrence in the node. If key is not found, the index returned
     * is the place where it should be.
     *
     * @param key The search key.
     * @return The idx of the occurrence of key in node or, if key is not found,
     *  the idx of the first element that is greater than key.
     */
    u_int32_t Find(KeyType key);

    /*
     * Find the median entry in the node. It is used in split operations.
     *
     * @return The idx of the median entry in node.
     */
    u_int32_t FindMedian() {
        return ((this->SHeader->Occupation % 2) == 0 ? (this->SHeader->Occupation / 2) : (this->SHeader->Occupation / 2) + 1);
    }

    /**
     * Returns the amount of the free space in this node.
     */
    u_int32_t GetFree();

private:
    /**
     * stBNode template type.
     */
    typedef stBNode<KeyType, Comparator> tBNode;

    /**
     * Entry pointer
     */
    stBIndexNodeEntry * Entries;

    /**
     * Pointer to the leftmost son.
     */
    u_int32_t * LeftmostPageID;

}; //end stBIndexNode


//-----------------------------------------------------------------------------
// Class stBLeafOverflowNode
//-----------------------------------------------------------------------------
/**
 * This class implements the Leaf overflow node of the BTree.
 *
 * <P>The BTree leaf node...
 * The structure of Leaf Node follows:
 * @image html bleafnode.png "Leaf node structure"
 *
 * <P>The <b>Header</b> holds the information about the node itself.
 *     - Type: Type of this node. It is always stSlimNode::LEAF (0x464C).
 *     - Occupation: Number of entries in this node.
 *
 * <P>The <b>Entry</b> holds the information of the link to the other node.
 *       - Distance: The distance of this object from the representative object.
 *       - Offset: The offset of the object in the page. DO NOT MODIFY ITS VALUE.
 *
 * <P>The <b>Object</b> is an array of bytes that holds the information required
 * to rebuild the original object.
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @todo Documentation review.
 * @see stBNode
 * @see stBIndexNode
 * @see stBLeafNode
 * @ingroup btree
 */
// +-------------------------------------------------------------------------------------------------+
// | Type | Occupation | OverFlowPageID | OffSet0 |...| OffSetn | <-- blankspace --> |Objn |...|Obj0 |
// +-------------------------------------------------------------------------------------------------+
// OBS: Keys are not stored to save space.

template < class KeyType, class Comparator >
class stBLeafOverflowNode : public stBNode<KeyType, Comparator> {
public:
    /**
     * This type represents a Btree leaf overlflow node entry.
     */
#pragma pack(1)

    typedef struct stBLeafOverflowNodeEntry {
        /**
         * Offset of the object.
         * @warning NEVER MODIFY THIS FIELD. YOU MAY DAMAGE THE STRUCTURE OF
         * THIS NODE.
         */
        u_int32_t Offset;
    } stBLeafOverflowNodeEntry; //end stBLeafOverflowEntry

    typedef struct stBLeafOverflowNodeHeader {
        /**
         * Pointer to the duplicate overflow leaf.
         * It is used if the number of duplicates does not fit in a sigle leaf node
         */
        u_int32_t OverflowPageID;
    } stBLeafOverflowNodeHeader;
#pragma pack()

    /**
     * Creates a new instance of this class. The paramenter <i>page</i> is an
     * instance of stPage that hold the node data.
     *
     * <P>The parameter <i>create</i> tells what operation will
     * be performed. True means that the page will be initialized and false
     * means that the page will be used as it is. The default value is false.
     *
     * @param pageManager A page manager instance.
     * @param page The page to be associated to this node.
     * @param create The operation to be performed.
     */
    stBLeafOverflowNode(stPageManager * pageManager, stPage * page, bool create = false);

    /**
     * The class destructor.
     *
     * @warning The superclass destructor will write the node's page, if necessary.
     */
    virtual ~stBLeafOverflowNode() {
    }

    /**
     * Inserts a new object to this node. This method will return the idx of the new
     * node or a negative value for failure.
     *
     * <P>This method will fail if there is not enough space to hold the
     * new object.
     *
     * @param serializedObjectSize The size of the object in bytes.
     * @param serializedObject The object data.
     * @warning The parameter serializedObjectSize is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return The position in the std::vector Entries.
     * @see GetSerializedObjectAt()
     * @see GetSerializedObjectSizeAt()
     */
    unsigned int Insert(u_int32_t serializedObjectSize, const unsigned char * serializedObject);

    /**
     * Deletes the idx-th element in this node.
     *
     * @param idx The object's position.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     */
    void DeleteElementAt(u_int32_t idx);

    /**
     * Set the next leaf overflow page id
     */
    void SetOverflowPageID(u_int32_t pageId) {
        Header->OverflowPageID = pageId;
        this->IsPageModified = true;
    }

    /**
     * Return the next leaf overflow page id
     */
    u_int32_t GetOverflowPageID() {
        return Header->OverflowPageID;
    }

    /**
     * Return the serialized object stored in the idx-th position in this node.
     *
     * @param idx The index of the object in the node.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return A const pointer to the object.
     */
    const unsigned char * GetSerializedObjectAt(u_int32_t idx);

    /**
     * Return the serialized size of the object stored in the idx-th position in this node.
     *
     * @param idx The index of the object in the node.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return The object's serialized size.
     */
    u_int32_t GetSerializedObjectSizeAt(u_int32_t idx);

    /**
     * Returns the amount of the free space in this node.
     */
    u_int32_t GetFree();

private:
    /**
     * stBNode template type.
     */
    typedef stBNode<KeyType, Comparator> tBNode;

    /**
     * Header of this page.
     */
    stBLeafOverflowNodeHeader * Header;

    /**
     * Entry pointer
     */
    stBLeafOverflowNodeEntry * Entries;

}; //end stBLeafOverflowNode



//-----------------------------------------------------------------------------
// Class stBLeafNode
//-----------------------------------------------------------------------------
/**
 * This class implements the Leaf node of the BTree.
 *
 * <P>The BTree leaf node...
 * The structure of Leaf Node follows:
 * @image html bleafnode.png "Leaf node structure"
 *
 * <P>The <b>Header</b> holds the information about the node itself.
 *     - Type: Type of this node. It is always stSlimNode::LEAF (0x464C).
 *     - Occupation: Number of entries in this node.
 *
 * <P>The <b>Entry</b> holds the information of the link to the other node.
 *       - Distance: The distance of this object from the representative object.
 *       - Offset: The offset of the object in the page. DO NOT MODIFY ITS VALUE.
 *
 * <P>The <b>Object</b> is an array of bytes that holds the information required
 * to rebuild the original object.
 *
 * @version 1.0
 * @author Daniel dos Santos Kaster (dskaster@uel.br)
 * @todo Documentation review.
 * @see stBNode
 * @see stBIndexNode
 * @see stBLeafOverflowNode
 * @ingroup btree
 */
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Type | Occupation | PreviousPageID | NextPageID | OverflowPageID | OverflowOccupation | Key0 | OffSet0 |...| Keyn | OffSetn | <-- blankspace --> |Objn |...|Obj0 |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// OBS: Keys are ordered, but the objects themselves are stored in reversed insertion order, so the offset "pointers" are interleaved.

template < class KeyType, class Comparator >
class stBLeafNode : public stBNode<KeyType, Comparator> {
public:
    /**
     * This type represents a Btree leaf node entry.
     */
#pragma pack(1)

    typedef struct stBLeafNodeEntry {
        /**
         * Object key
         */
        KeyType Key;

        /**
         * Offset of the object.
         * @warning NEVER MODIFY THIS FIELD. YOU MAY DAMAGE THE STRUCTURE OF
         * THIS NODE.
         */
        u_int32_t Offset;
    } stBLeafNodeEntry; //end stBLeafEntry
    

    typedef struct stBLeafNodeHeader {
        /**
         * Pointer to the previous leaf.
         */
        u_int32_t PreviousPageID;

        /**
         * Pointer to the next leaf.
         */
        u_int32_t NextPageID;

        /**
         * Stores the PageId of the first overflow leaf node. A leaf overflow node is 
         * used to store duplicate entries that do not fit in the leaf node. Only
         * leaf nodes full with entries with the same key (duplicates) are allowed
         * to have one (or more) leaf overflow node.
         * 
         * @warning The leaf node is responsible for manage the leaf overflow pages
         * using its PageManager instance.
         */
        u_int32_t OverflowPageID;

        /**
         * The number of entries in the overflow node(s)
         */
        u_int32_t OverflowOccupation;
    } stBLeafNodeHeader;
#pragma pack()

    /**
     * Creates a new instance of this class. The paramenter <i>page</i> is an
     * instance of stPage that hold the node data.
     *
     * <P>The parameter <i>create</i> tells what operation will
     * be performed. True means that the page will be initialized and false
     * means that the page will be used as it is. The default value is false.
     *
     * @param pageManager An instance of stPageManager.
     * @param page The page that hold the data of this node.
     * @param create The operation to be performed.
     */
    stBLeafNode(stPageManager * pageManager, stPage * page, bool create = false);

    /**
     * Class destructor. It frees loaded overflow nodes.
     */
    virtual ~stBLeafNode();

    /**
     * Set the PageId of the previous leaf node.
     * 
     * @param pageId The previous leaf node pageId.
     */
    void SetPreviousPageID(u_int32_t pageId) {
        Header->PreviousPageID = pageId;
        this->IsPageModified = true;
    }

    /**
     * Get the PageId of the previous leaf node.
     *
     * @return The previous leaf node pageId.
     */
    u_int32_t GetPreviousPageID() {
        return Header->PreviousPageID;
    }

    /**
     * Set the PageId of the next leaf node.
     *
     * @param pageId The next leaf node pageId.
     */
    void SetNextPageID(u_int32_t pageId) {
        Header->NextPageID = pageId;
        this->IsPageModified = true;
    }

    /**
     * Get the PageId of the next leaf node.
     *
     * @return The next leaf node pageId.
     */
    u_int32_t GetNextPageID() {
        return Header->NextPageID;
    }

    /**
     * Overloaded version of stBNode::GetNumberOfEntries().
     */
    virtual u_int32_t GetNumberOfEntries() {
        return this->SHeader->Occupation + Header->OverflowOccupation;
    }//end GetNumberOfEntries()

    /**
     * stBNode::GetNumberOfEntries() in leaf node, desconsidering eventual overflow nodes.
     */
    virtual u_int32_t GetNumberOfEntriesNoOverflow() {
        return this->SHeader->Occupation;
    }//end GetNumberOfEntries()
    

    /**
     * Inserts a new object to this node. This method will return the idx of the new
     * node or a negative value for failure.
     *
     * <P>This method will fail if there is not enough space to hold the
     * new object.
     *
     * @param key The object's key in the structure.
     * @param serializedObjectSize The size of the object in bytes.
     * @param serializedObject The object data.
     * @warning The parameter serializedObjectSize is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @warning Caller must check if object size is not greater than the free space of an empty node.
     * @return The position in the std::vector Entries.
     * @see GetSerializedObjectAt()
     * @see GetSerializedObjectSizeAt()
     */
//    u_int32_t Insert(KeyType key, u_int32_t serializedObjectSize, const unsigned char * serializedObject);
    unsigned int Insert(KeyType key, u_int32_t serializedObjectSize, const unsigned char * serializedObject, bool duplicationAllowed = true);


    /**
     * Deletes the idx-th element in this node.
     *
     * @param idx The object's position.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     */
    void DeleteElementAt(u_int32_t idx);

    /*
     * Find the first key's occurrence in the node. If key is not found, the index returned
     * is the place where it should be.
     *
     * @param key The search key.
     * @param[out] The idx of the first occurrence of key in node or, if key is not found,
     *  the idx of the first element that is greater than key.
     * @return True if key was found and false otherwise.
     * @see FindLast()
     * @see Find()
     */
    bool FindFirst(KeyType key, u_int32_t &idx);

    /*
     * Find the last key's occurrence in the node. If key is not found, the index returned
     * is the place where it should be.
     *
     * @param key The search key.
     * @param[out] The idx of the last occurrence of key in node or, key is not found,
     * the idx of the first element that is greater than key.
     * @return True if key was found and false otherwise.
     * @see FindFirst()
     * @see Find()
     */
    bool FindLast(KeyType key, u_int32_t &idx);

    /*
     * Find the median entry in the node. It is used in split operations.
     * If there are duplicates of the median entry key in this leaf node, it
     * will be returned the index of the next (not duplicated) key entry.
     * @return The idx of the median entry in node.
     */
    u_int32_t FindMedian();

    /**
     * Return the key stored in the idx-th position in this node.
     *
     * @param idx The index of the object in the node.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return The key.
     */
    KeyType GetKeyAt(u_int32_t idx) {
#ifdef __stDEBUG__
        if (idx >= GetNumberOfEntries()) {
            throw std::logic_error("idx value is out of range. stBLeafNode::GetKeyAt()");
        }//end if
#endif //__stDEBUG__

        // Is idx in the LeafNode?
        if (idx < this->SHeader->Occupation) {
            // Yes, return its key.
            return Entries[idx].Key;
        }
        else {
            // No, it is in an OverflowNode.
            // As every key in the LeafNode and in the OverflowNodes is the same, any of them can be returned (e.g. the first one).
            return Entries[0].Key;
        }
    }//end GetKeyAt()


    /**
     * Return the serialized object stored in the idx-th position in this node.
     *
     * @param idx The index of the object in the node.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return A const pointer to the object.
     */
    const unsigned char * GetSerializedObjectAt(u_int32_t idx);

    /**
     * Return the serialized size of the object stored in the idx-th position in this node.
     *
     * @param idx The index of the object in the node.
     * @warning The parameter idx is not verified by this implementation
     * unless __stDEBUG__ is defined at compile time.
     * @return The object's serialized size.
     */
    u_int32_t GetSerializedObjectSizeAt(u_int32_t idx);

    /**
     * Returns the amount of the free space in this node. If it has overflow nodes,
     * this method will return the amount of free space on the last overflow node.
     */
    u_int32_t GetFree();

private:
    /**
     * stBNode template type.
     */
    typedef stBNode<KeyType, Comparator> tBNode;

    /**
     * Leaf overflow template type.
     */
    typedef stBLeafOverflowNode<KeyType, Comparator> tBLeafOverflowNode;

    /*
     * Search for key in node performing a binary search. If key is not found, the index returned
     * is the place where it should be.
     *
     * @param key The search key.
     * @param[out] The idx of a occurrence of key in node or, if key is not found,
     * the idx of the first element that is greater than key.
     * @return True if key was found and false otherwise.
     * @see FindFirst()
     * @see FindLast()
     */
    bool Find(KeyType key, u_int32_t &idx);

    /**
     * Returns the amount of the free space in this node, ignoring overflow nodes.
     */
    u_int32_t GetLeafNodeFree();

    /**
     * Header of this page.
     */
    stBLeafNodeHeader * Header;

    /**
     * Entry pointer
     */
    stBLeafNodeEntry * Entries;

    /**
     * Vector of overflow nodes
     */
    std::vector<tBLeafOverflowNode *> overflowNodes;

}; //end stBLeafNode


// Include implementation
#include "stBNode-inl.h"

#endif //__STBNODE_H
