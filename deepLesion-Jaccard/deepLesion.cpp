
#pragma hdrstop
#include "deepLesion.h"
#pragma package(smart_init)
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

const uint8_t *DeepLesion::Serialize()
{

   long long *d;

   if (Serialized == NULL)
   {

      Serialized = new uint8_t[GetSerializedSize()];

      d = (long long *)Serialized;

      d[0] = OID;

      memcpy(Serialized + (sizeof(long long)), attributes.Serialize(), attributes.GetSerializedSize());
   }

   return Serialized;
}

const uint8_t *Attributes::Serialize()
{

   int *d;
   uint8_t *Serialized = new uint8_t[GetSerializedSize()];

   d = (int *)Serialized;

   for (size_t i = 0; i < Tags.size(); i++)
   {
      d[i] = Tags[i];
   }

   return Serialized;
}

const uint8_t *Included::Serialize()
{

   uint8_t *Serialized = new uint8_t[GetSerializedSize()];

   double *d;

   d = (double *)Serialized;

   d[0] = PatientAge;

   return Serialized;
}

const uint8_t *DeepLesion::IncludedSerialize()
{

   long long *d;

   if (Serialized == NULL)
   {

      Serialized = new uint8_t[GetIncludedSerializedSize()];

      d = (long long *)Serialized;

      d[0] = OID;

      memcpy(Serialized + (sizeof(long long)), attributes.Serialize(), attributes.GetSerializedSize());

      memcpy(Serialized + (sizeof(long long)) + attributes.GetSerializedSize(), included.Serialize(), included.GetSerializedSize());
   }

   return Serialized;
}

void DeepLesion::Unserialize(const uint8_t *data, size_t datasize)
{

   long long *d;

   d = (long long *)data;

   OID = d[0];

   attributes.Unserialize(data + sizeof(long long), datasize - sizeof(long long));

   if (Serialized != NULL)
   {
      delete[] Serialized;
      Serialized = NULL;
   }
}

void DeepLesion::IncludedUnserialize(const uint8_t *data, size_t datasize)
{
   long long *d;
   d = (long long *)data;

   OID = d[0];

   attributes.Unserialize(data + sizeof(long long), datasize - sizeof(long long) - included.GetSerializedSize());

   included.Unserialize(data + sizeof(long long) + attributes.GetSerializedSize(), datasize - sizeof(long long) - attributes.GetSerializedSize());

   if (Serialized != NULL)
   {
      delete[] Serialized;
      Serialized = NULL;
   }
}

void Attributes::Unserialize(const uint8_t *data, size_t datasize)
{

   int *d;

   d = (int *)data;

   Tags.clear();

   int couter = datasize / sizeof(int);

   for (size_t i = 0; i < couter; i++)
   {

      int tag = d[i];
      Tags.push_back(tag);
   }
}

void Included::Unserialize(const uint8_t *data, size_t datasize)
{

   double *d;

   d = (double *)data;

   PatientAge = d[0];
}
