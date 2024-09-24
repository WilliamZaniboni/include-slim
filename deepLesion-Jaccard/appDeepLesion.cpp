
#include <iostream>
#pragma hdrstop
#include "appDeepLesion.h"
#pragma package(smart_init)
#include <unistd.h>

void AppDeepLesion::CreateTree()
{

    SlimTree = new mySlimTree(PageManager);

    // DummyTree = new myDummyTree(PageManagerDummy);
}
void AppDeepLesion::CreateDiskPageManager()
{

    PageManager = new stPlainDiskPageManager("SlimTree.dat", 8192);
    // PageManagerDummy = new stPlainDiskPageManager("DummyTree.dat", 8192);
}

void AppDeepLesion::Run()
{
    std::cout << "\n\nAdding objects in the SlimTree";
    LoadTree(GEONAMESFILE);

    std::cout << "\n\nAdding objects in the dummy tree";
    // LoadDummyTree(GEONAMESFILE);

    std::cout << "\n\nLoading the query file";
    LoadVectorFromFile(QUERYGEONAMESFILE);

    if (queryObjects.size() > 0)
    {

        PerformQueries();
    }

    std::cout << "\n\nFinished the whole test!";
}

//------------------------------------------------------------------------------
void AppDeepLesion::Done()
{

    if (this->SlimTree != NULL)
    {
        delete this->SlimTree;
    }
    if (this->PageManager != NULL)
    {
        delete this->PageManager;
    }

    for (unsigned int i = 0; i < queryObjects.size(); i++)
    {
        delete (queryObjects.at(i));
    }
}

//------------------------------------------------------------------------------
void AppDeepLesion::LoadTree(char *fileName)
{
    ifstream in(fileName);
    DeepLesion *geoname;

    long long oid;

    double patientIndex;
    double lesionTpe;
    string patientGender;
    double patientAge;
    double d1;
    double d2;
    double x;
    double y;
    double z;

    int qtd_tags;

    int counter = 0;

    if (SlimTree != NULL)
    {

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        if (in.is_open())
        {

            std::cout << "\nLoading objects ";

            while (true)
            {
                long long prev_oid = oid;

                in >> oid;

                if (in.eof())
                    break;

                in >> patientIndex;
                in >> lesionTpe;
                in >> patientGender;
                in >> patientAge;
                in >> d1;
                in >> d2;
                in >> x;
                in >> y;
                in >> z;

                in >> qtd_tags;

                vector<int> tags;

                int tag;

                for (int i = 0; i < qtd_tags; i++)
                {
                    in >> tag;

                    tags.push_back(tag);
                }

                counter++;

                Attributes atr = Attributes(tags);
                Included incl = Included(patientAge);

                geoname = new DeepLesion(oid, atr, incl);
                SlimTree->Add(geoname);
                delete geoname;
            }
            std::cout << " Added " << SlimTree->GetNumberOfObjects() << " objects ";
            in.close();
        }
        else
        {
            std::cout << "\nProblem to open the file.";
        } // end if

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::cout << "\nCREATE SLIM Total Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]";
    }
    else
    {
        std::cout << "\n Zero object added!!";
    } // end if
} // end TApp::LoadTree

//------------------------------------------------------------------------------
void AppDeepLesion::LoadDummyTree(char *fileName)
{
    ifstream in(fileName);
    DeepLesion *geoname;

    long long oid;

    double patientIndex;
    double lesionTpe;
    string patientGender;
    double patientAge;
    double d1;
    double d2;
    double x;
    double y;
    double z;

    int qtd_tags;

    int counter = 0;

    if (DummyTree != NULL)
    {

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        if (in.is_open())
        {

            std::cout << "\nLoading objects on dummy";

            while (true)
            {
                long long prev_oid = oid;

                in >> oid;

                if (in.eof())
                    break;
                in >> patientIndex;
                in >> lesionTpe;
                in >> patientGender;
                in >> patientAge;
                in >> d1;
                in >> d2;
                in >> x;
                in >> y;
                in >> z;

                in >> qtd_tags;

                vector<int> tags;

                int tag;

                for (int i = 0; i < qtd_tags; i++)
                {
                    in >> tag;

                    tags.push_back(tag);
                }

                counter++;

                Attributes atr = Attributes(tags);
                Included incl = Included(patientAge);

                geoname = new DeepLesion(oid, atr, incl);
                DummyTree->Add(geoname);
                delete geoname;
            }
            std::cout << " Added " << DummyTree->GetNumberOfObjects() << " objects ";
            in.close();
        }
        else
        {
            std::cout << "\nProblem to open the file.";
        } // end if

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::cout << "\nCREATE Dummy Total Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]";
    }
    else
    {
        std::cout << "\n Zero object added!!";
    } // end if
}

//------------------------------------------------------------------------------
void AppDeepLesion::LoadVectorFromFile(char *fileName)
{

    ifstream in(fileName);
    DeepLesion *geoname;
    queryObjects.clear();

    long long oid;

    double patientIndex;
    double lesionTpe;
    string patientGender;
    double patientAge;
    double d1;
    double d2;
    double x;
    double y;
    double z;

    int qtd_tags;

    if (in.is_open())
    {
        std::cout << "\nLoading query objects ";

        while (true)
        {

            in >> oid;

            // cout << oid << "\n";

            if (in.eof())
                break;
            in >> patientIndex;
            in >> lesionTpe;
            in >> patientGender;
            in >> patientAge;
            in >> d1;
            in >> d2;
            in >> x;
            in >> y;
            in >> z;

            in >> qtd_tags;

            vector<int> tags;

            int tag;

            for (int i = 0; i < qtd_tags; i++)
            {
                in >> tag;

                tags.push_back(tag);
            }

            Attributes atr = Attributes(tags);

            this->queryObjects.insert(queryObjects.end(), new DeepLesion(atr));
        }

        // end while
        std::cout << " Added " << queryObjects.size() << " query objects ";
        in.close();
    }
    else
    {
        std::cout << "\nProblem to open the query file.";
        std::cout << "\n Zero object added!!\n";
    }
}

//------------------------------------------------------------------------------
void AppDeepLesion::PerformQueries()
{
    if (SlimTree)
    {

        PerformRangeQueryOneCenter();

        PerformExistsQuery();

        PerformNearestQueryOneCenter();

    } // end if

    if (DummyTree)
    {

        PerformRangeQueryOneCenterDummy();

        PerformNearestQueryOneCenterDummy();

        PerformExistsQueryDummy();
    }

} // end TApp::PerformQuery


void AppDeepLesion::PerformExistsQuery()
{

    myResult *result;
    double radius;
    // clock_t start, end;
    unsigned int size;
    unsigned int i;

    std::ofstream myfile;
    myfile.open("performance/exists_included_slim_results.txt");
    myfile << "range, avg total time, avg disk accesses, avg distance calculations, avg returned tuples\n";

    double range_list[] = {0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4};

    if (SlimTree)
    {
        int index;

        for (index = 0; index < 9; index++)
        {
            double range = range_list[index];

            std::cout << range << "|" << index << "\n";
            size = queryObjects.size();
            // reset the statistics
            PageManager->ResetStatistics();
            SlimTree->GetMetricEvaluator()->ResetStatistics();
            // start = clock();
            int tuples = 0;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (i = 0; i < size; i++)
            {
                result = SlimTree->ExistsQuery(queryObjects[i], range);

                tuples = tuples + result->GetNumOfEntries();

                delete result;
            } // end for
            // end = clock();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            myfile << range << ",";
            myfile << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / (double)size << ",";
            myfile << (double)PageManager->GetReadCount() / (double)size << ",";
            myfile << (double)SlimTree->GetMetricEvaluator()->GetDistanceCount() / (double)size << ",";
            myfile << (double)tuples / (double)size << "\n";
        }
    } // end if

    myfile.close();

} //

void AppDeepLesion::PerformExistsQueryDummy()
{

    myResult *result;
    double radius;
    // clock_t start, end;
    unsigned int size;
    unsigned int i;

    std::ofstream myfile;
    myfile.open("performance/exists_dummy_results.txt");
    myfile << "range, avg total time, avg disk accesses, avg distance calculations, avg returned tuples\n";

    double range_list[] = {0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4};

    if (DummyTree)
    {
        int index;

        for (index = 0; index < 9; index++)
        {
            double range = range_list[index];

            std::cout << range << "|" << index << "\n";

            size = queryObjects.size();
            // reset the statistics
            PageManagerDummy->ResetStatistics();
            DummyTree->GetMetricEvaluator()->ResetStatistics();
            // start = clock();
            int tuples = 0;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (i = 0; i < size; i++)
            {
                result = DummyTree->ExistsQuery(queryObjects[i], range);

                tuples = tuples + result->GetNumOfEntries();
               
                delete result;
            } // end for
            // end = clock();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
          

            myfile << range << ",";
            myfile << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / (double)size << ",";
            myfile << (double)PageManagerDummy->GetReadCount() / (double)size << ",";
            myfile << (double)DummyTree->GetMetricEvaluator()->GetDistanceCount() / (double)size << ",";
            myfile << (double)tuples / (double)size << "\n";
        }
    } // end if

    myfile.close();
}


void AppDeepLesion::PerformNearestQueryOneCenter()
{

    myResult *result;
    unsigned int size;
    unsigned int i;

    std::ofstream myfile;
    myfile.open("performance/knn_include_slim_results.txt");

    myfile << "id age distance\n";

    if (SlimTree)
    {

        size = queryObjects.size();
        PageManager->ResetStatistics();
        SlimTree->GetMetricEvaluator()->ResetStatistics();
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        int tuples = 0;
        double avg_range = 0;

        for (i = 0; i < size; i++)
        {
            result = SlimTree->NearestQuery(queryObjects[i], 5, false, true);

            tuples = tuples + result->GetNumOfEntries();

            for (size_t s = 0; s < result->GetNumOfEntries(); s++)
            {

                myfile << result->GetPair(s)->GetObject()->getOID() << " ";

                myfile << result->GetPair(s)->GetObject()->GetIncluded().GetPatientAge() << " ";

                myfile << result->GetPair(s)->GetDistance() << "\n";
            }

            delete result;
        } // end for

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";
        cout << "Disk access: " << (double)PageManager->GetReadCount() << "\n";
        cout << "Distances: " << (double)SlimTree->GetMetricEvaluator()->GetDistanceCount() << "\n";
    }
}

void AppDeepLesion::PerformNearestQueryOneCenterDummy()
{

    myResult *result;
    unsigned int size;
    unsigned int i;

    std::ofstream myfile;
    myfile.open("performance/knn_dummy_results.txt");

    myfile << "id age distance\n";
    if (DummyTree)
    {

        size = queryObjects.size();
        PageManagerDummy->ResetStatistics();
        DummyTree->GetMetricEvaluator()->ResetStatistics();
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        int tuples = 0;
        double avg_range = 0;

        for (i = 0; i < size; i++)
        {
            result = DummyTree->NearestQuery(queryObjects[i], 5, false, true);

            tuples = tuples + result->GetNumOfEntries();

            for (size_t s = 0; s < result->GetNumOfEntries(); s++)
            {

                myfile << result->GetPair(s)->GetObject()->getOID() << " ";

                myfile << result->GetPair(s)->GetObject()->GetIncluded().GetPatientAge() << " ";

                myfile << result->GetPair(s)->GetDistance() << " \n";
            }

            delete result;
        } // end for

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";
        cout << "Disk access: " << (double)PageManagerDummy->GetReadCount() << "\n";
        cout << "Distances: " << (double)DummyTree->GetMetricEvaluator()->GetDistanceCount() << "\n";
    }
}

void AppDeepLesion::PerformRangeQueryOneCenter()
{

    myResult *result;
    unsigned int size;
    unsigned int i;

    std::ofstream myfile;
    myfile.open("performance/range_include_slim_results.txt");

    myfile << "id age distance\n";

    if (SlimTree)
    {

        size = queryObjects.size();
        PageManager->ResetStatistics();
        SlimTree->GetMetricEvaluator()->ResetStatistics();
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        int tuples = 0;
        double avg_range = 0;

        for (i = 0; i < size; i++)
        {
            result = SlimTree->RangeQuery(queryObjects[i], 0.1);

            tuples = tuples + result->GetNumOfEntries();

            for (size_t s = 0; s < result->GetNumOfEntries(); s++)
            {

                myfile << result->GetPair(s)->GetObject()->getOID() << " ";

                myfile << result->GetPair(s)->GetObject()->GetIncluded().GetPatientAge() << " ";

                myfile << result->GetPair(s)->GetDistance() << "\n";
            }

            delete result;
        } // end for

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";
        cout << "Disk access: " << (double)PageManager->GetReadCount() << "\n";
        cout << "Distances: " << (double)SlimTree->GetMetricEvaluator()->GetDistanceCount() << "\n";
    }
}

void AppDeepLesion::PerformRangeQueryOneCenterDummy()
{

    myResult *result;
    unsigned int size;
    unsigned int i;

    std::ofstream myfile;
    myfile.open("performance/range_include_slim_results.txt");

    myfile << "id distance\n";

    if (DummyTree)
    {

        size = queryObjects.size();
        PageManagerDummy->ResetStatistics();
        DummyTree->GetMetricEvaluator()->ResetStatistics();
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        int tuples = 0;
        double avg_range = 0;

        for (i = 0; i < size; i++)
        {
            result = DummyTree->RangeQuery(queryObjects[i], 0.1);

            tuples = tuples + result->GetNumOfEntries();

            for (size_t s = 0; s < result->GetNumOfEntries(); s++)
            {

                myfile << result->GetPair(s)->GetObject()->getOID() << " ";

                myfile << result->GetPair(s)->GetDistance() << "\n";
            }

            delete result;
        } // end for

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";
        cout << "Disk access: " << (double)PageManagerDummy->GetReadCount() << "\n";
        cout << "Distances: " << (double)DummyTree->GetMetricEvaluator()->GetDistanceCount() << "\n";
    }
}
