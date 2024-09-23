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
* This file defines the abstract class stObject.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STOBJECT_H
#define __STOBJECT_H


/**
* This abstract class implements the stObject interface. This interface is required
* by the objects that will be indexed by the metric trees implemented by this
* library. Since these metric trees are implemented as class templates, any
* class that implements all methods defined in this interface may be indexed by
* a metric tree.
*
* <P>This class may be used as the base class for classes that implements the
* objects which will be indexed by a metric tree but this use is not recommended.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @see stMetricEvaluator
* @ingroup userinterface
*/
//template <class T>
class stObject{
   public:

      /**
      * Builds a new uninitialized instance of this class. The resulting instance must be
      * ready to be initializated by a Unserialize() call.
      *
      * @note This constructor is required to fulfil the stObject interface.
      */
      stObject(){};
      
      
      /** 
       * Destroy intance of this class.
       */
      virtual ~stObject(){};


      //template <class T>
      //T Get(u_int32_t idx) {};
      virtual void * Get(u_int32_t idx) = 0;

      virtual u_int32_t GetSize() = 0;



      /**
      * This method must create a clone (copy) of this class. The new instance must have
      * the same behaviour of the original instance but can not keep any links to its creator
      * (they are independent). In other words, the clone will not reflect any changes done in
      * the original class.
      *
      * @return The clone (copy) of this object.
      * @note This method is required to fulfil the stObject interface.
      */
      virtual stObject * Clone() = 0;

      /**
      * This method must return True if the object in parameter is equal (has the same content)
      *  of the object that call it.
      *
      * @param obj Object that will be used in comparison.
      * @return whether the obj has the same content.
      * @see Clone()
      * @note This method is required to fulfil the stObject interface.
      */
      virtual bool IsEqual(stObject* obj) = 0;

      /**
      * This method must return the size of the serialized version of this object in bytes.
      *
      * @see Serialize()
      * @see Unserialize()
      * @note This method is required to fulfil the stObject interface.
      */
      virtual u_int32_t GetSerializedSize() = 0;

      /**
      * This method must create a serialized version of this instance.
      * The serialized version is an array of bytes that will hold all information
      * required to rebuild this object. For example, if the object
      * is a string, the serialized form may be the string itself without the terminator
      * character (since it is a sequence of bytes).
      *
      * <P>This array will be used by the SlimTree to store the object into a disk page
      * and to recover it (by calling Unserialize()) and its size given by GetSerializedSize().
      *
      * <P>The effect of the of A->Unserialize(B->Serialize(), B->GetSerializedSize())
      * must be the same of A=B->Clone().
      *
      * <P>Implementation note: The SlimTree will not claim the ownership of the returned
      * array. The object must dispose it when it is not required. In case of the serialized
      * version be a pointer to an internal array, no copy operation will be necessary.
      *
      * @return The serialized version of this object.
      * @see GetSerializedSize()
      * @see Unserialized()
      * @note This method is required to fulfil the stObject interface.
      */
      virtual const unsigned char * Serialize() = 0;

      /**
      * This method must rebuild the object from its serialized version.
      * The serialized version is an array of bytes that holds all information
      * required to rebuild an object. For example, if the object is a string,
      * the serialized form may be the string itself without the terminator
      * character (since it is a sequence of bytes).
      *
      * <P>This method will be used by the SlimTree to rebuild the object to compare it with
      * another object (the sample object in a query or another rebuilt object).
      *
      * <P>The effect of the of A->Unserialize(B->Serialize(), B->GetSerializedSize())
      * must be the same of A=B->Clone().
      *
      * <P>Implementation note: The SlimTree is the owner of the data array. This means
      * that this array may vanish at any time after the call of Unserialize(). There is
      * also another restriction. The Unserialize() method must never modify the contents
      * of the data array.
      *
      * @param data The serialized object.
      * @param datasize The size (in bytes) of the data array.
      * @see Serialized()
      * @note This method is required to fulfil the stObject interface.
      */
      virtual void Unserialize(const unsigned char * data, u_int32_t datasize) = 0;
      
      /**
      * This method must return something that identifies uniquely the
      * object.
      *
      * @return An Object Identifier (OID).
      */
      virtual long GetOID(){
        return 0;
      }//end GetOID()
};//end stObject

#endif // __STOBJECT_H
