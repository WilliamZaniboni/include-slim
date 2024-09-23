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
 * This file defines a generic array object that implements all methods required
 * by the stObject interface. This object may be used in combination with the
 * metric evaluators defined in the file stBasicMetricEvaluator.h.
 *
 * @version 2.0

 */
#ifndef __MULTIOBJECT_H
#define __MULTIOBJECT_H


#include <stdexcept>
#include <arboretum/stObject.h>

#include <string.h>
#include <fstream>

class InstanceObject {
public:

    InstanceObject() {

    }

    typedef stBasicArrayObject <double> BasicArrayDoubleObject;
    typedef stBasicStringObject <long> BasicStringObject;

    static stObject * GetInstance(int type) {

        switch (type) {
            case 1:
                return (new BasicArrayDoubleObject());
                //case 2:
                //return (new BasicStringObject());
            default:
                return NULL;
        };
    }

    static stObject * ReadObject(int type, ifstream * in, int size) {

        switch (type) {
            case 1:
                BasicArrayDoubleObject * basicArray;
                double values[size];
                for (int i = 0; i < size; i++) {
                    *in >> values[i];
                }//end for
                basicArray = new BasicArrayDoubleObject(size, values);
                return basicArray;
                //case 2:

                //return (new BasicStringObject());
            default:
                return NULL;
        };
    }

};


//==============================================================================
//  class ClassArrayObject
//------------------------------------------------------------------------------

/**
 * This class is a modification of stBasicArrayObject that includes a classification
 * @author Adriano Arantes Paterlini (paterlini@gmail.com)
 * @see stObject
 * @ingroup user
 */
template <class DataType, class OIDType = int>
class ClassArrayObject : public stObject {
public:

    /**
     * Default constructor. It creates an array with 0 elements. Use SetSize()
     * to change the number of entries before use this instance.
     *
     * <P>This constructor is required by the stObject interface.
     */
    ClassArrayObject() {
        Size = 0;
        TrueData = NULL;
        Data = NULL;
        OID = NULL;
        Class = NULL;
    }//end ClassArrayObject

    /**
     * Creates a new instance of this class with n entries. If the parameter
     * data is not NULL, the content pointed by it will be used to initializate
     * this instance.
     *
     * @param n Number of entries.
     * @param data The initial values of NULL (default).
     */
    ClassArrayObject(u_int32_t n, const DataType * data = NULL) {

        // Allocate data
        Size = 0;
        TrueData = NULL;
        Data = NULL;
        OID = NULL;
        SetSize(n);

        // Initialize values
        if (data != NULL) {
            SetData(data);
        }//end if
    }//end ClassArrayObject

    /**
     * Disposes this instance and releases all associated resources.
     */
    virtual ~ClassArrayObject() {

        if (TrueData != NULL) {
            delete[] TrueData;
        }//end if
    }//end ~ClassArrayObject

    /**
     * Copies the content of the array data to this object. The array pointed
     * by data must have at least GetSize() entries.
     *
     * @param data The array with the values.
     */
    void SetData(const DataType * data) {

        memcpy(Data, data, sizeof (DataType) * Size);
    }//end SetData

    /**
     * Returns the pointer to the data array.
     */
    DataType * GetData() {
        return Data;
    }//end GetData

    /**
     * Returns the number of the elements of this array.
     */
    u_int32_t GetSize() {
        return Size;
    }//end u_int32_t

    /**
     * This operator can be used to access all positions of the array for
     * reading or writing.
     *
     * @param idx The index of the element.
     * @warning This operator does not check the range of idx.
     */
    DataType & operator [] (int idx) {
        return Data[idx];
    }//end operator []

    /**
     * Gets the value of the element at a given position.
     *
     * @param idx The index of the desired entry.
     * @exception out_of_range If the index is out of range.
     */
    DataType Get(u_int32_t idx) {
        if (idx >= Size) {
            throw out_of_range("Index out of range.");
        } else {
            return Data[idx];
        }//end if
    }

    /**
     * Returns the OID associated with this instance.
     *
     * @warning This method returns nothing if the object size is 0.
     */
    long GetOID() {
        if (OID == NULL) {
            return 0;
        } else {
            return * OID;
        }//end if
    }//end GetOID

    /**
     * Sets the OID associated with this instance.
     *
     * @param oid The new OID value.
     * @warning This method does nothing if the object size is 0.
     */
    void SetOID(OIDType oid) {
        if (OID != NULL) {
            *OID = oid;
        }//end if
    }//end SetOID

    /**
     * Returns the Class associated with this instance.
     *
     * @warning This method returns nothing if the object size is 0.
     */
    int GetClass() {
        if (Class == NULL) {
            return 0;
        } else {
            return * Class;
        }//end if
    }//end GetClass

    /**
     * Sets the Class associated with this instance.
     *
     * @param oid The new Class value.
     * @warning This method does nothing if the object size is 0.
     */
    void SetClass(int cl) {
        if (Class != NULL) {
            *Class = cl;
        }//end if
    }//end SetClass

    // stObject interface

    /**
     * @copydoc stObject::Clone()
     */
    ClassArrayObject * Clone() {
        ClassArrayObject * tmp = new ClassArrayObject(Size, Data);
        tmp->SetOID(this->GetOID());
        tmp->SetClass(this->GetClass());
        return tmp;
    }//end Clone

    /**
     * @copydoc stObject::GetSerializedSize()
     */
    u_int32_t GetSerializedSize() {
        return sizeof (DataType) * Size + sizeof (OIDType) + sizeof (int);
    }//end GetSerializedSize

    /**
     * @copydoc stObject::Serialize()
     */
    const unsigned char * Serialize() {
        return TrueData;
    }//end Serialize

    /**
     * @copydoc stObject::Unserialize()
     */
    void Unserialize(const unsigned char * data, u_int32_t dataSize) {

        // Resize data
        SetSize((dataSize - sizeof (OIDType) - sizeof (int)) / sizeof (DataType));

        memcpy(TrueData, data, dataSize);
    }//end Unserialize

    /**
     * @copydoc stObject::IsEqual()
     */
    bool IsEqual(stObject * obj) {
        bool equal;
        bool stop;
        u_int32_t i;

        if (GetSize() != ((ClassArrayObject*) obj)->GetSize()) {
            return false;
        }//end if

        // Scanning...
        equal = true;
        i = 0;
        stop = (i >= GetSize());
        while (!stop) {
            if (Data[i] != ((ClassArrayObject*) obj)->Data[i]) {
                stop = true;
                equal = false;
            } else {
                i++;
                stop = (i >= GetSize());
            }//end if
        }//end while

        return equal;
    }

    /**
     * Sets the size of this array. All previous values will be lost.
     *
     * @param n The new number of objects of this array.
     */
    void SetSize(u_int32_t n) {

        if (Size != n) {
            if (TrueData != NULL) {
                delete[] TrueData;
            }//end if

            // Create array.
            TrueData = new unsigned char[(sizeof (DataType) * n) + sizeof (OIDType) + sizeof (int) ];
            Size = n;
            OID = (OIDType *) TrueData;
            Class = (int *) (TrueData + sizeof (OIDType));
            Data = (DataType *) (TrueData + sizeof (OIDType) + sizeof (int));
            *OID = 0; // Set OID to 0
            *Class = 0; //Set Class to 0
        }//end if
    }//end SetSize

private:

    /**
     * Data array. It points to a position in the TrueData array.
     */
    DataType * Data;

    /**
     * OID Type. It points to a position in the TrueData array.
     */
    OIDType * OID;


    int * Class;

    /**
     * This is the true data array. Both OID and Data points
     * to positions in this array.
     */
    unsigned char * TrueData;

    /**
     * Number of entries.
     */
    u_int32_t Size;


}; //end ClassArrayObject




//==============================================================================
//  class MultipleObject
//------------------------------------------------------------------------------

class MultipleObject : public stObject {
//class MultipleObject {
public:

    MultipleObject() {
        NumObjs = 0;
    }//end ClassArrayObject

    virtual ~MultipleObject() {
        for (int i = 0; i < Objects.size(); i++) {
            delete Objects[i];
        }
        Objects.clear();
        Instancetype.clear();
    }//end ~ClassArrayObject

    //u_int32_t GetSize() {
    //    return SizeOne + SizeTwo;
    //}//end u_int32_t

    void * Get(u_int32_t idx) {

        if (idx < Objects.size()) {
            return Objects[idx];
        } else {
            return NULL;
        }

    }//end GetDataOne

    /**
     * Insert an object at the of the vector
     */
    void AddObj(stObject *Obj, int type) {

        Objects.insert(Objects.end(), Obj);
        Instancetype.insert(Instancetype.end(), type);
    }//end AddObj

    /**
     * Returns the Class associated with this instance.
     */
    u_int32_t GetSize() {

        return Objects.size();

    }//end GetNumObjs

    MultipleObject * Clone() {

        MultipleObject * tmp = new MultipleObject();
        //copy all Objects
        tmp->SetOID(this->OID);
        tmp->SetClass(this->Class);
        for (int i = 0; i < Objects.size(); i++) {
            stObject *Obj = Objects[i];
            tmp->AddObj(Obj->Clone(), Instancetype[i]);
        }//endfor        

        return tmp;
    }//end Clone

    u_int32_t GetSerializedSize() {
        u_int32_t tmp = 0;
        tmp = sizeof (int) + sizeof (long) + sizeof (int);
        for (int i = 0; i < Objects.size(); i++) {
            tmp += (sizeof (u_int32_t) + sizeof (int) + Objects[i]->GetSerializedSize());
        }//endfor
        return tmp;
    }//end GetSerializedSize

    const unsigned char * Serialize() {
        unsigned char * serial = new unsigned char[this->GetSerializedSize()];
        u_int32_t acc = 0;

        int * serNum = (int *) serial;
        serNum[0] = Objects.size();
        acc = sizeof (int);
        long * serOID = (long *) (serial + acc);
        serOID[0] = this->OID;
        acc += sizeof (long);
        int * serClass = (int *) (serial + acc);
        serClass[0] = this->Class;
        acc += sizeof (int);

        for (int i = 0; i < Objects.size(); i++) {
            u_int32_t * serSize = (u_int32_t *) (serial + acc);
            serSize[0] = Objects[i]->GetSerializedSize();
            acc += sizeof (u_int32_t);
            int * serInstType = (int *) (serial + acc);
            serInstType[0] = Instancetype[i];
            acc += sizeof (int);
            unsigned char * serObj = (unsigned char *) (serial + acc);
            long tmpsize = Objects[i]->GetSerializedSize();
            ;
            memcpy(serObj, Objects[i]->Serialize(), tmpsize);
            acc += Objects[i]->GetSerializedSize();
        }
        return serial;
    }//end Serialize

    void Unserialize(const unsigned char * data, u_int32_t dataSize) {
        //clean
        for (int i = 0; i < Objects.size(); i++) {
            delete (Objects.at(i));
            Objects.clear();
            Instancetype.clear();
        }//endfor
        u_int32_t acc = 0;
        int numobj;
        int objsize;
        int * serNum = (int *) data;
        numobj = serNum[0];
        acc += sizeof (int);
        long * serOID = (long *) (data + acc);
        this->OID = serOID[0];
        acc += sizeof (long);
        int * serClass = (int *) (data + acc);
        this->Class = serClass[0];
        acc += sizeof (int);
        for (int i = 0; i < numobj; i++) {
            u_int32_t * serSize = (u_int32_t *) (data + acc);
            objsize = serSize[0];
            acc += sizeof (u_int32_t);
            int * serInsType = (int *) (data + acc);
            Instancetype.insert(Instancetype.end(), serInsType[0]);
            acc += sizeof (int);
            unsigned char * serObj = (unsigned char *) (data + acc);
            stObject * Obj = ::InstanceObject::GetInstance(Instancetype[i]);
            Obj->Unserialize(serObj, objsize);
            Objects.insert(Objects.end(), Obj);
            acc += objsize;
        }//end Unserialize
    }

    bool IsEqual(stObject * multiobj) {

        bool eq = true;

        for (int i = 0; i < Objects.size(); i++) {
            stObject *thisObj = Objects[i];
            stObject *obj = (stObject *)((MultipleObject *) multiobj)->Get(i);
            eq = eq && (thisObj->IsEqual(obj));
        }//endfor

        return eq;
    }

    long GetOID() {
        return OID;
    }//end GetOID

    void SetOID(long oid) {
        OID = oid;
    }//end SetOID

    /**
     * Returns the Class associated with this instance.
     */
    int GetClass() {
        return Class;
    }//end GetClass

    /**
     * Sets the Class associated with this instance.
     * @param oid The new Class value.
     */
    void SetClass(int cl) {
        Class = cl;
    }//end SetClass

    long OID;

    int Class;

private:

    vector <stObject *> Objects;

    vector <int> Instancetype;

    int NumObjs;

}; //end MultipleObject

//#include <arboretum/MyObject-inl.h>

#endif //__MULTIOBJECT_H
