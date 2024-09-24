
#ifndef deepLesion
#define deepLesion

#include <cstdlib>
#include <cstdint>
#include <math.h>
#include <string>
#include <vector>
#include <time.h>
#include <ostream>
#include <iostream>
#include <algorithm>
using namespace std;

#include <arboretum/stUtil.h>
#include <hermes/DistanceFunction.h>

class Included
{
private:
    double PatientAge;

public:
    Included()
    {
    }
    Included(
        double patientAge)
    {

        PatientAge = patientAge;
    }

    size_t GetSerializedSize()
    {

        size_t len = (sizeof(double));

        return len;
    }

    double GetPatientAge()
    {
        return PatientAge;
    }

    const uint8_t *Serialize();

    void Unserialize(const uint8_t *data, size_t datasize);
};

class Attributes
{
private:
    vector<int> Tags;

public:
    Attributes()
    {
    }
    Attributes(vector<int> tags)
    {
        Tags = tags;
    };
    const uint8_t *Serialize();

    void Unserialize(const uint8_t *data, size_t datasize);

    vector<int> GetTags()
    {
        return Tags;
    }

    size_t GetSerializedSize()
    {

        return (sizeof(int)) * Tags.size();
    }

    bool IsEqual(Attributes obj)
    {
        bool result = true;

        for (size_t i = 0; i < Tags.size(); i++)
        {

            if (Tags[i] != obj.GetTags()[i])
            {

                result = false;
            }
        }

        return result;
    }
};

class DeepLesion
{
public:
    DeepLesion()
    {

        Serialized = NULL;
    }

    // ORDER BY LOGIC
    bool operator<(DeepLesion obj2)
    {
        return GetIncluded().GetPatientAge() > obj2.GetIncluded().GetPatientAge();
    }

    DeepLesion(long long oid, Attributes atr, Included inc)
    {
        OID = oid;
        attributes = atr;
        included = inc;

        Serialized = NULL;
    }

    DeepLesion(long long oid, Attributes atr)
    {
        OID = oid;
        attributes = atr;

        Serialized = NULL;
    }

    DeepLesion(Attributes atr)
    {

        attributes = atr;

        Serialized = NULL;
    }

    ~DeepLesion()
    {

        if (Serialized != NULL)
        {
            delete[] Serialized;
        }
    }

    Attributes GetAttributes()
    {
        return attributes;
    }

    Included GetIncluded()
    {
        return included;
    }

    DeepLesion *Clone()
    {
        return new DeepLesion(OID, attributes, included);
    }
    bool IsEqual(DeepLesion *obj)
    {

        return attributes.IsEqual(obj->GetAttributes());
    }

    size_t GetSerializedSize()
    {

        return (sizeof(long long)) + attributes.GetSerializedSize();
    }

    size_t GetIncludedSerializedSize()
    {
        return (sizeof(long long)) + attributes.GetSerializedSize() + included.GetSerializedSize();
    }

    const uint8_t *Serialize();

    const uint8_t *IncludedSerialize();

    void Unserialize(const uint8_t *data, size_t datasize);
    void IncludedUnserialize(const uint8_t *data, size_t datasize);

    long long getOID()
    {
        return OID;
    }

private:
    long long OID;

    Attributes attributes;

    Included included;

    uint8_t *Serialized;

}; // end TMapPoint

class DeepLesionDistanceEvaluator : public DistanceFunction<DeepLesion>
{
public:
    DeepLesionDistanceEvaluator()
    {
    }

    double GetDistance(DeepLesion &obj1, DeepLesion &obj2)
    {

        updateDistanceCount(); // Update Statistics

        return distance(obj1.GetAttributes().GetTags(), obj2.GetAttributes().GetTags());
    }

    bool GetFilter(DeepLesion &obj1, DeepLesion &obj2)
    {
        /*

        // IF YOU WANT TO DISCARD THE QUERY CENTER AS ANSWER
        if (obj1.IsEqual(&obj2))
        {
            return false;
        }
        */

        /*
        // IF YOU WANT WHERE CLAUSE
        if (obj1.GetIncluded().GetGender() == "M")
        {
            return true;
        }

        return false;

        */

        // NO FILTER APPLIED
        return true;
    }

    double getDistance(DeepLesion &obj1, DeepLesion &obj2)
    {

        updateDistanceCount(); // Update Statistics

        return distance(obj1.GetAttributes().GetTags(), obj2.GetAttributes().GetTags());
    }

    // Retirado de : https://www.geeksforgeeks.org/program-distance-two-points-earth/

    // Function to return the Jaccard index of two sets
    double jaccard_index(vector<int> s1, vector<int> s2)
    {
        // Sizes of both the sets
        double size_s1 = s1.size();
        double size_s2 = s2.size();

        std::vector<int> out;
        std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                              std::back_inserter(out));

        // Size of the intersection set
        double size_in = out.size();

        // Calculate the Jaccard index
        // using the formula
        double jaccard_in = size_in / (size_s1 + size_s2 - size_in);

        // cout << jaccard_in << "\n";

        // Return the Jaccard index
        return jaccard_in;
    }

    double distance(vector<int> tags1, vector<int> tags2)
    {

        return 1 - jaccard_index(tags1, tags2);
    }
};

#endif