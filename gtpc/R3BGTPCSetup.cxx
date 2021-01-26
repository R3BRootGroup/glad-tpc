// Class to read the setup file of the detector, it's a common class for the 3 different designs.
// This simplify the code, one has to be just careful to the use of the function R3BGTPCSetup,
// because now it requires a second argument to be used.
// Author S. Velardita
#include "R3BGTPCSetup.h"
ClassImp(R3BGTPCSetup)

    R3BGTPCSetup::R3BGTPCSetup()
{
    ReadConfigurationFile("HYDRAprototype_FileSetup.txt", 0);
}

R3BGTPCSetup::R3BGTPCSetup(std::string detector, int verbose)
{
    if (TString(detector).Contains("Prototype"))
        ReadConfigurationFile("HYDRAprototype_FileSetup.txt", verbose);
    else if (TString(detector).Contains("Fullv1"))
        ReadConfigurationFile("HYDRAFullv1_FileSetup.txt", verbose);
    else if (TString(detector).Contains("Fullv2"))
        ReadConfigurationFile("HYDRAFullv2_FileSetup.txt", verbose);
    else
        cout << "The detector is not well defined!!!!!!!!!!" << endl;
}

R3BGTPCSetup::~R3BGTPCSetup() {}

void R3BGTPCSetup::ReadConfigurationFile(string Config, int verbose)
{
    string LineBuffer;
    string DataBuffer;

    bool cTargetRadius = false;
    bool cTargetLength = false;
    bool cTargetAngle = false;
    bool cTargetOffsetX = false;
    bool cTargetOffsetY = false;
    bool cTargetOffsetZ = false;

    bool cActiveRegionx = false;
    bool cActiveRegiony = false;
    bool cActiveRegionz = false;
    bool cFrameThickness = false;
    bool cTPCLx = false;
    bool cTPCLy = false;
    bool cTPCLz = false;
    bool cWindowx = false;
    bool cWindowy = false;
    bool cWindowz = false;

    bool cGladAngle = false;
    bool cGladOffsetX = false;
    bool cGladOffsetY = false;
    bool cGladOffsetZ = false;
    bool cFieldAngle = false;
    bool cFieldOffsetX = false;
    bool cFieldOffsetY = false;
    bool cFieldOffsetZ = false;

    bool cOutputFileName = false;

    TString dir = getenv("VMCWORKDIR");
    TString Path = dir + "/glad-tpc/gtpc/inputs/" + Config;
    ifstream ConfigFile;
    ConfigFile.open(Path.Data());

    if (ConfigFile.is_open())
        cout << " Configuration file for setup " << Config << " loading " << endl;
    else
    {
        cout << " Error, no configuration file for setup " << Config << " found" << endl;
        return;
    }

    while (!ConfigFile.eof())
    {
        // Pick-up next line
        getline(ConfigFile, LineBuffer);
        // Search for comment Symbol: %
        if (LineBuffer.compare(0, 1, "%") == 0)
        { /*Do  Nothing*/
            ;
        }

        //---------------------------------------------------------

        else if (LineBuffer.compare(0, 12, "TargetRadius") == 0 && cTargetRadius == false)
        {
            cTargetRadius = true;
            string test = LineBuffer.substr(12, LineBuffer.length() - 12);
            TargetRadius = atof(test.c_str());
            if (verbose == 1)
                cout << "TargetRadius = " << TargetRadius << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 12, "TargetLength") == 0 && cTargetLength == false)
        {
            cTargetLength = true;
            string test = LineBuffer.substr(12, LineBuffer.length() - 12);
            TargetLength = atof(test.c_str());
            if (verbose == 1)
                cout << "TargetLength = " << TargetLength << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 11, "TargetAngle") == 0 && cTargetAngle == false)
        {
            cTargetAngle = true;
            string test = LineBuffer.substr(11, LineBuffer.length() - 11);
            TargetAngle = atof(test.c_str());
            if (verbose == 1)
                cout << "TargetAngle = " << TargetAngle << " deg" << endl;
        }
        else if (LineBuffer.compare(0, 13, "TargetOffsetX") == 0 && cTargetOffsetX == false)
        {
            cTargetOffsetX = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            TargetOffsetX = atof(test.c_str());
            if (verbose == 1)
                cout << "TargetOffsetX = " << TargetOffsetX << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 13, "TargetOffsetY") == 0 && cTargetOffsetY == false)
        {
            cTargetOffsetY = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            TargetOffsetY = atof(test.c_str());
            if (verbose == 1)
                cout << "TargetOffsetY = " << TargetOffsetY << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 13, "TargetOffsetZ") == 0 && cTargetOffsetZ == false)
        {
            cTargetOffsetZ = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            TargetOffsetZ = atof(test.c_str());
            if (verbose == 1)
                cout << "TargetOffsetZ = " << TargetOffsetZ << " cm" << endl;
        }

        else if (LineBuffer.compare(0, 13, "ActiveRegionx") == 0 && cActiveRegionx == false)
        {
            cActiveRegionx = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            ActiveRegionx = atof(test.c_str());
            if (verbose == 1)
                cout << "ActiveRegionx = " << ActiveRegionx << " cm" << endl;
        }

        else if (LineBuffer.compare(0, 13, "ActiveRegiony") == 0 && cActiveRegiony == false)
        {
            cActiveRegiony = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            ActiveRegiony = atof(test.c_str());
            if (verbose == 1)
                cout << "ActiveRegiony = " << ActiveRegiony << " cm" << endl;
        }

        else if (LineBuffer.compare(0, 13, "ActiveRegionz") == 0 && cActiveRegionz == false)
        {
            cActiveRegionz = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            ActiveRegionz = atof(test.c_str());
            if (verbose == 1)
                cout << "ActiveRegionz = " << ActiveRegionz << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 14, "FrameThickness") == 0 && cFrameThickness == false)
        {
            cFrameThickness = true;
            string test = LineBuffer.substr(14, LineBuffer.length() - 14);
            FrameThickness = atof(test.c_str());
            if (verbose == 1)
                cout << "FrameThickness = " << FrameThickness << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 5, "TPCLx") == 0 && cTPCLx == false)
        {
            cTPCLx = true;
            string test = LineBuffer.substr(5, LineBuffer.length() - 5);
            TPCLx = atof(test.c_str());
            if (verbose == 1)
                cout << "TPCLx = " << TPCLx << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 5, "TPCLy") == 0 && cTPCLy == false)
        {
            cTPCLy = true;
            string test = LineBuffer.substr(5, LineBuffer.length() - 5);
            TPCLy = atof(test.c_str());
            if (verbose == 1)
                cout << "TPCLy = " << TPCLy << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 5, "TPCLz") == 0 && cTPCLz == false)
        {
            cTPCLz = true;
            string test = LineBuffer.substr(5, LineBuffer.length() - 5);
            TPCLz = atof(test.c_str());
            if (verbose == 1)
                cout << "TPCLz = " << TPCLz << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 7, "Windowx") == 0 && cWindowx == false)
        {
            cWindowx = true;
            string test = LineBuffer.substr(7, LineBuffer.length() - 7);
            Windowx = atof(test.c_str());
            if (verbose == 1)
                cout << "Windowx = " << Windowx << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 7, "Windowy") == 0 && cWindowy == false)
        {
            cWindowy = true;
            string test = LineBuffer.substr(7, LineBuffer.length() - 7);
            Windowy = atof(test.c_str());
            if (verbose == 1)
                cout << "Windowy = " << Windowy << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 7, "Windowz") == 0 && cWindowz == false)
        {
            cWindowz = true;
            string test = LineBuffer.substr(7, LineBuffer.length() - 7);
            Windowz = atof(test.c_str());
            if (verbose == 1)
                cout << "Windowz = " << Windowz << " cm" << endl;
        }

        else if (LineBuffer.compare(0, 9, "GladAngle") == 0 && cGladAngle == false)
        {
            cGladAngle = true;
            string test = LineBuffer.substr(9, LineBuffer.length() - 9);
            GladAngle = atof(test.c_str());
            if (verbose == 1)
                cout << "GladAngle = " << GladAngle << " deg" << endl;
        }
        else if (LineBuffer.compare(0, 11, "GladOffsetX") == 0 && cGladOffsetX == false)
        {
            cGladOffsetX = true;
            string test = LineBuffer.substr(11, LineBuffer.length() - 11);
            GladOffsetX = atof(test.c_str());
            if (verbose == 1)
                cout << "GladOffsetX = " << GladOffsetX << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 11, "GladOffsetY") == 0 && cGladOffsetY == false)
        {
            cGladOffsetY = true;
            string test = LineBuffer.substr(11, LineBuffer.length() - 11);
            GladOffsetY = atof(test.c_str());
            if (verbose == 1)
                cout << "GladOffsetY = " << GladOffsetY << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 11, "GladOffsetZ") == 0 && cGladOffsetZ == false)
        {
            cGladOffsetZ = true;
            string test = LineBuffer.substr(11, LineBuffer.length() - 11);
            GladOffsetZ = atof(test.c_str());
            if (verbose == 1)
                cout << "GladOffsetZ = " << GladOffsetZ << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 10, "FieldAngle") == 0 && cFieldAngle == false)
        {
            cFieldAngle = true;
            string test = LineBuffer.substr(10, LineBuffer.length() - 10);
            FieldAngle = atof(test.c_str());
            if (verbose == 1)
                cout << "FieldAngle = " << FieldAngle << " deg" << endl;
        }
        else if (LineBuffer.compare(0, 12, "FieldOffsetX") == 0 && cFieldOffsetX == false)
        {
            cFieldOffsetX = true;
            string test = LineBuffer.substr(12, LineBuffer.length() - 12);
            FieldOffsetX = atof(test.c_str());
            if (verbose == 1)
                cout << "FieldOffsetX = " << FieldOffsetX << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 12, "FieldOffsetY") == 0 && cFieldOffsetY == false)
        {
            cFieldOffsetY = true;
            string test = LineBuffer.substr(12, LineBuffer.length() - 12);
            FieldOffsetY = atof(test.c_str());
            if (verbose == 1)
                cout << "FieldOffsetY = " << FieldOffsetY << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 12, "FieldOffsetZ") == 0 && cFieldOffsetZ == false)
        {
            cFieldOffsetZ = true;
            string test = LineBuffer.substr(12, LineBuffer.length() - 12);
            FieldOffsetZ = atof(test.c_str());
            if (verbose == 1)
                cout << "FieldOffsetZ = " << FieldOffsetZ << " cm" << endl;
        }
        else if (LineBuffer.compare(0, 14, "OutputFileName") == 0 && cOutputFileName == false)
        {
            cOutputFileName = true;
            string test = LineBuffer.substr(14, LineBuffer.length() - 14);
            OutputFileName = test.c_str();
            OutputFileName = OutputFileName.Strip(TString::kBoth, ' ');
            if (verbose == 1)
                cout << "OutputFileName = " << OutputFileName << endl;
        }
    }
    ConfigFile.close();
    return;
}

void R3BGTPCSetup::Print()
{

    cout << "TargetRadius = " << TargetRadius << " cm" << endl;
    cout << "TargetLength = " << TargetLength << " cm" << endl;
    cout << "TargetAngle = " << TargetAngle << " deg" << endl;
    cout << "TargetOffsetX = " << TargetOffsetX << " cm" << endl;
    cout << "TargetOffsetY = " << TargetOffsetY << " cm" << endl;
    cout << "TargetOffsetZ = " << TargetOffsetZ << " cm" << endl;

    cout << "ActiveRegionx = " << ActiveRegionx << " cm" << endl;
    cout << "ActiveRegiony = " << ActiveRegiony << " cm" << endl;
    cout << "ActiveRegionz = " << ActiveRegionz << " cm" << endl;
    cout << "FrameThickness = " << FrameThickness << " cm" << endl;
    cout << "TPCLx = " << TPCLx << " cm" << endl;
    cout << "TPCLy = " << TPCLy << " cm" << endl;
    cout << "TPCLz = " << TPCLz << " cm" << endl;
    cout << "Windowx = " << Windowx << " cm" << endl;
    cout << "Windowy = " << Windowy << " cm" << endl;
    cout << "Windowz = " << Windowz << " cm" << endl;

    cout << "GladAngle = " << GladAngle << " deg" << endl;
    cout << "GladOffsetX = " << GladOffsetX << " cm" << endl;
    cout << "GladOffsetY = " << GladOffsetY << " cm" << endl;
    cout << "GladOffsetZ = " << GladOffsetZ << " cm" << endl;
    cout << "FieldAngle = " << FieldAngle << " deg" << endl;
    cout << "FieldOffsetX = " << FieldOffsetX << " cm" << endl;
    cout << "FieldOffsetY = " << FieldOffsetY << " cm" << endl;
    cout << "FieldOffsetZ = " << FieldOffsetZ << " cm" << endl;
    cout << "OutputFileName = " << OutputFileName << endl;
}
