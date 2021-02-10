// Class to read the setup file of the detector, it's a common class for the 4 different designs.
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
    if (TString(detector).CompareTo("Prototype")==0)
        ReadConfigurationFile("HYDRAprototype_FileSetup.txt", verbose);
    else if (TString(detector).CompareTo("FullBeamOut")==0)
        ReadConfigurationFile("HYDRAFullBeamOut_FileSetup.txt", verbose);
    else if (TString(detector).CompareTo("FullBeamIn")==0)
        ReadConfigurationFile("HYDRAFullBeamIn_FileSetup.txt", verbose);
	 	else if (TString(detector).CompareTo("Target")==0)
        ReadConfigurationFile("Target_FileSetup.txt", verbose);
   	else if (TString(detector).CompareTo("Electronic")==0)
        ReadConfigurationFile("Electronic_FileSetup.txt", verbose);
    else
        cout << "The detector is not well defined!!!!!!!!!!" << endl;
}

R3BGTPCSetup::~R3BGTPCSetup() {}

void R3BGTPCSetup::ReadConfigurationFile(string Config, int verbose)
{
    string LineBuffer;
    string DataBuffer;
		//Electronic
   	bool cTimeBin   	= false;	
   	bool cThreshold 	= false;
   	bool cGain      	= false;
   	bool cNoiseRMS  	= false;
   	bool cTheta     	= false;
   	bool cShapingTime = false;
   	//Target
    bool cTargetRadius = false;
    bool cTargetLength = false;
    bool cTargetAngle = false;
    bool cTargetOffsetX = false;
    bool cTargetOffsetY = false;
    bool cTargetOffsetZ = false;
		//TPC
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
    //Drift
    bool cPadSize = false;
    bool cLongDiff = false;
    bool cTransDiff = false;
    bool cFanoFactor = false;
    bool cEIonization = false;
    bool cDriftVelocity = false;
		//GLAD
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

        //Electronic_____________________________________________________________
    		else if (LineBuffer.compare(0,7, "TimeBin") == 0 && cTimeBin == false) 
    		{
         	cTimeBin = true ;
	 				string test = LineBuffer.substr(7,LineBuffer.length()-7);
         	TimeBinSize=atof(test.c_str());
         	if(verbose==1)
         				cout << "Time bin = "<<TimeBinSize<<" ns" << endl   ;
     		}
     		else if (LineBuffer.compare(0,9, "Threshold") == 0 && cThreshold == false) 
     		{
         	cThreshold = true ;
	 				string test = LineBuffer.substr(9,LineBuffer.length()-9);
         	Threshold=atof(test.c_str());
         	if(verbose==1)
         				cout << "Detection Threshold = "<<Threshold<<" times noise rms" << endl   ;
     		}
      	else if (LineBuffer.compare(0,4, "Gain") == 0 && cGain == false) 
      	{
         	cGain = true ;
	 				string test = LineBuffer.substr(4,LineBuffer.length()-4);
         	Gain=atof(test.c_str());
         	if(verbose==1)
         				cout << "Detection Gain = "<<Gain << endl   ;
     		}
      	else if (LineBuffer.compare(0,5, "Theta") == 0 && cTheta == false) 
      	{
         	cTheta = true ;
	 				string test = LineBuffer.substr(5,LineBuffer.length()-5);
         	Theta=atof(test.c_str());
         	if(verbose==1)
         				cout << "Parameter theta for Polya = "<<Theta << endl   ;
     		}
      	else if (LineBuffer.compare(0,8, "NoiseRMS") == 0 && cNoiseRMS == false) 
      	{
        	cNoiseRMS = true ;
	 				string test = LineBuffer.substr(8,LineBuffer.length()-8);
         	NoiseRMS=atof(test.c_str());
         	if(verbose==1)
         				cout << "Noise background = "<<NoiseRMS<<" e- RMS" << endl   ;
     		}
      	else if (LineBuffer.compare(0,11, "ShapingTime") == 0 && cShapingTime == false) 
      	{
         	cShapingTime = true ;
	 				string test = LineBuffer.substr(11,LineBuffer.length()-11);
         	ShapingTime=atoi(test.c_str());
         	if(verbose==1)
         				cout << "ShapingTime = "<<ShapingTime<<" ns" << endl   ;
     		}
     		//Target_________________________________________________________________
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
				//TPC____________________________________________________________________
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
        //Drift__________________________________________________________________
        else if (LineBuffer.compare(0, 7, "PadSize") == 0 && cPadSize == false)
        {
            cPadSize = true;
            string test = LineBuffer.substr(7, LineBuffer.length() - 7);
            PadSize = atof(test.c_str());
            if (verbose == 1)
                cout << "PadSize = " << PadSize << " " << endl;
        }
        else if (LineBuffer.compare(0, 8, "LongDiff") == 0 && cLongDiff == false)
        {
            cLongDiff = true;
            string test = LineBuffer.substr(8, LineBuffer.length() - 8);
            LongDiff = atof(test.c_str());
            if (verbose == 1)
                cout << "LongDiff = " << LongDiff << " cm^2/ns" << endl;
        }
        else if (LineBuffer.compare(0, 9, "TransDiff") == 0 && cTransDiff == false)
        {
            cTransDiff = true;
            string test = LineBuffer.substr(9, LineBuffer.length() - 9);
            TransDiff = atof(test.c_str());
            if (verbose == 1)
                cout << "TransDiff = " << TransDiff << " cm^2/ns" << endl;
        }
        else if (LineBuffer.compare(0, 10, "FanoFactor") == 0 && cFanoFactor == false)
        {
            cFanoFactor = true;
            string test = LineBuffer.substr(10, LineBuffer.length() - 10);
            FanoFactor = atof(test.c_str());
            if (verbose == 1)
                cout << "FanoFactor = " << FanoFactor << " " << endl;
        }
        else if (LineBuffer.compare(0, 11, "EIonization") == 0 && cEIonization == false)
        {
            cEIonization = true;
            string test = LineBuffer.substr(11, LineBuffer.length() - 11);
            EIonization = atof(test.c_str());
            if (verbose == 1)
                cout << "EIonization = " << EIonization << " GeV" << endl;
        }
        else if (LineBuffer.compare(0, 13, "DriftVelocity") == 0 && cDriftVelocity == false)
        {
            cDriftVelocity = true;
            string test = LineBuffer.substr(13, LineBuffer.length() - 13);
            DriftVelocity = atof(test.c_str());
            if (verbose == 1)
                cout << "DriftVelocity = " << DriftVelocity << " cm/ns" << endl;
        }
				//GLAD___________________________________________________________________
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
		//Electronic_______________________________________________________________
   	cout << "Time bin = "<<TimeBinSize<<" ns" << endl   ;
    cout << "Detection Threshold = "<<Threshold<<" times noise rms" << endl   ;
    cout << "Detection Gain = "<<Gain << endl   ;
    cout << "Parameter theta for Polya = "<<Theta << endl   ;
    cout << "Noise background = "<<NoiseRMS<<" e- RMS" << endl   ;
    cout << "ShapingTime = "<<ShapingTime<<" ns" << endl   ;
		//Target___________________________________________________________________
    cout << "TargetRadius = " << TargetRadius << " cm" << endl;
    cout << "TargetLength = " << TargetLength << " cm" << endl;
    cout << "TargetAngle = " << TargetAngle << " deg" << endl;
    cout << "TargetOffsetX = " << TargetOffsetX << " cm" << endl;
    cout << "TargetOffsetY = " << TargetOffsetY << " cm" << endl;
    cout << "TargetOffsetZ = " << TargetOffsetZ << " cm" << endl;
		//TPC______________________________________________________________________
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
    //Drift____________________________________________________________________
    cout << "PadSize = " << PadSize << " " << endl;
    cout << "LongDiff = " << LongDiff << " cm^2/ns" << endl;
    cout << "TransDiff = " << TransDiff << " cm^2/ns" << endl;
    cout << "FanoFactor = " << FanoFactor << " " << endl;
    cout << "EIonization = " << EIonization << " GeV" << endl;
    cout << "DriftVelocity = " << DriftVelocity << " cm/ns" << endl;
		//GLAD___________________________________________________________________
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
