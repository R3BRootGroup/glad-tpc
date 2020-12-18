#include "R3BGTPCSetup.h"
ClassImp(R3BGTPCSetup)

R3BGTPCSetup::R3BGTPCSetup() {
  ReadConfigurationFile("ConfigFileSetup.txt",0);
}

R3BGTPCSetup::R3BGTPCSetup(int verbose) {
  ReadConfigurationFile("ConfigFileSetup.txt",verbose);
}

R3BGTPCSetup::~R3BGTPCSetup(){
}

void R3BGTPCSetup::ReadConfigurationFile(string Config,int verbose) {
  string LineBuffer;
  string DataBuffer;

  // --------------------------------------------------------------------------
  bool cTargetRadius       = false;
  bool cTargetLength       = false;
  bool cChamberInnerRadius     = false;
  bool cChamberThickness       = false;
  bool cChamberLength          = false;
  bool cInnerRohacellThickness = false;
  bool cKaptonThickness        = false;
  bool cOuterRohacellThickness = false;
  bool cTPCRadiusExt       = false;
  bool cWindowThickness    = false;
  
  bool cTPCLx       = false;
  bool cTPCLy       = false;
  bool cTPCLz       = false;
  
  bool cTargetAngle        = false;
  bool cTargetOffsetX      = false;
  bool cTargetOffsetY      = false;
  bool cTargetOffsetZ      = false;
  bool cGladAngle          = false;
  bool cGladOffsetX        = false;
  bool cGladOffsetY        = false;
  bool cGladOffsetZ        = false;
  bool cFieldAngle         = false;
  bool cFieldOffsetX       = false;
  bool cFieldOffsetY       = false;
  bool cFieldOffsetZ       = false;
  
  bool cOutputFileName       = false;
  
  TString dir = getenv("VMCWORKDIR");
  TString Path = dir+"/glad-tpc/gtpc/inputs/"+Config;
  ifstream ConfigFile;
  ConfigFile.open(Path.Data());
  
  if (ConfigFile.is_open())
    cout << " Configuration file for setup " << Config << " loading " << endl;
  else {
    cout << " Error, no configuration file for setup " << Config << " found" << endl;
    return;
  }
  
  while (!ConfigFile.eof()) {
    //Pick-up next line
    getline(ConfigFile, LineBuffer);
    //Search for comment Symbol: %
    if (LineBuffer.compare(0, 1, "%") == 0) {   /*Do  Nothing*/;}
    
    else if (LineBuffer.compare(0,12, "TargetRadius") == 0 && cTargetRadius == false) {
      cTargetRadius = true ;
      string test = LineBuffer.substr(12,LineBuffer.length()-12);
      TargetRadius=atof(test.c_str());
      if(verbose==1)
	cout << "TargetRadius = " << TargetRadius<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,12, "TargetLength") == 0 && cTargetLength == false) {
      cTargetLength = true ;
      string test = LineBuffer.substr(12,LineBuffer.length()-12);
      TargetLength=atof(test.c_str());
      if(verbose==1)
	cout << "TargetLength = " << TargetLength<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,18, "ChamberInnerRadius") == 0 && cChamberInnerRadius == false) {
      cChamberInnerRadius = true ;
      string test = LineBuffer.substr(18,LineBuffer.length()-18);
      ChamberInnerRadius=atof(test.c_str());
      if(verbose==1)
	cout << "ChamberInnerRadius = " << ChamberInnerRadius<<" cm"<<endl;
      }
    else if (LineBuffer.compare(0,16, "ChamberThickness") == 0 && cChamberThickness == false) {
      cChamberThickness = true ;
      string test = LineBuffer.substr(16,LineBuffer.length()-16);
      ChamberThickness=atof(test.c_str());
      if(verbose==1)
	cout << "ChamberThickness = " << ChamberThickness<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,13, "ChamberLength") == 0 && cChamberLength == false) {
      cChamberLength = true ;
      string test = LineBuffer.substr(13,LineBuffer.length()-13);
      ChamberLength=atof(test.c_str());
      if(verbose==1)
	cout << "ChamberLength = " << ChamberLength<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,22, "InnerRohacellThickness") == 0 && cInnerRohacellThickness == false) {
      cInnerRohacellThickness = true ;
      string test = LineBuffer.substr(22,LineBuffer.length()-22);
      InnerRohacellThickness=atof(test.c_str());
      if(verbose==1)
	cout << "InnerRohacellThickness = " << InnerRohacellThickness<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,15, "KaptonThickness") == 0 && cKaptonThickness == false) {
      cKaptonThickness = true ;
      string test = LineBuffer.substr(15,LineBuffer.length()-15);
      KaptonThickness=atof(test.c_str());
      if(verbose==1)
	cout << "KaptonThickness = " << KaptonThickness<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,22, "OuterRohacellThickness") == 0 && cOuterRohacellThickness == false) {
      cOuterRohacellThickness = true ;
      string test = LineBuffer.substr(22,LineBuffer.length()-22);
      OuterRohacellThickness=atof(test.c_str());
      if(verbose==1)
	cout << "OuterRohacellThickness = " << OuterRohacellThickness<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,12, "TPCRadiusExt") == 0 && cTPCRadiusExt == false) {
      cTPCRadiusExt = true ;
      string test = LineBuffer.substr(12,LineBuffer.length()-12);
      TPCRadiusExt=atof(test.c_str());
      if(verbose==1)
	cout << "TPCRadiusExt = " << TPCRadiusExt<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,5, "TPCLx") == 0 && cTPCLx == false) {
      cTPCLx = true ;
      string test = LineBuffer.substr(5,LineBuffer.length()-5);
      TPCLx=atof(test.c_str());
      if(verbose==1)
	cout << "TPCLx = " << TPCLx<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,5, "TPCLy") == 0 && cTPCLy == false) {
      cTPCLy = true ;
      string test = LineBuffer.substr(5,LineBuffer.length()-5);
      TPCLy=atof(test.c_str());
      if(verbose==1)
	cout << "TPCLy = " << TPCLy<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,5, "TPCLz") == 0 && cTPCLz == false) {
      cTPCLz = true ;
      string test = LineBuffer.substr(5,LineBuffer.length()-5);
      TPCLz=atof(test.c_str());
      if(verbose==1)
	cout << "TPCLz = " << TPCLz<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,15, "WindowThickness") == 0 && cWindowThickness == false) {
      cWindowThickness = true ;
      string test = LineBuffer.substr(15,LineBuffer.length()-15);
      WindowThickness=atof(test.c_str());
      if(verbose==1)
	cout << "WindowThickness = " << WindowThickness<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,11, "TargetAngle") == 0 && cTargetAngle == false) {
      cTargetAngle = true ;
      string test = LineBuffer.substr(11,LineBuffer.length()-11);
      TargetAngle=atof(test.c_str());
      if(verbose==1)
	cout << "TargetAngle = " << TargetAngle<<" deg"<<endl;
    }
    else if (LineBuffer.compare(0,13, "TargetOffsetX") == 0 && cTargetOffsetX == false) {
      cTargetOffsetX = true ;
      string test = LineBuffer.substr(13,LineBuffer.length()-13);
      TargetOffsetX=atof(test.c_str());
      if(verbose==1)
	cout << "TargetOffsetX = " << TargetOffsetX<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,13, "TargetOffsetY") == 0 && cTargetOffsetY == false) {
      cTargetOffsetY = true ;
      string test = LineBuffer.substr(13,LineBuffer.length()-13);
      TargetOffsetY=atof(test.c_str());
      if(verbose==1)
	cout << "TargetOffsetY = " << TargetOffsetY<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,13, "TargetOffsetZ") == 0 && cTargetOffsetZ == false) {
      cTargetOffsetZ = true ;
      string test = LineBuffer.substr(13,LineBuffer.length()-13);
      TargetOffsetZ=atof(test.c_str());
      if(verbose==1)
	cout << "TargetOffsetZ = " << TargetOffsetZ<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,9, "GladAngle") == 0 && cGladAngle == false) {
      cGladAngle = true ;
      string test = LineBuffer.substr(9,LineBuffer.length()-9);
      GladAngle=atof(test.c_str());
      if(verbose==1)
	cout << "GladAngle = " << GladAngle<<" deg"<<endl;
    }
    else if (LineBuffer.compare(0,11, "GladOffsetX") == 0 && cGladOffsetX == false) {
      cGladOffsetX = true ;
      string test = LineBuffer.substr(11,LineBuffer.length()-11);
      GladOffsetX=atof(test.c_str());
      if(verbose==1)
	cout << "GladOffsetX = " << GladOffsetX<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,11, "GladOffsetY") == 0 && cGladOffsetY == false) {
      cGladOffsetY = true ;
      string test = LineBuffer.substr(11,LineBuffer.length()-11);
      GladOffsetY=atof(test.c_str());
      if(verbose==1)
	cout << "GladOffsetY = " << GladOffsetY<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,11, "GladOffsetZ") == 0 && cGladOffsetZ == false) {
      cGladOffsetZ = true ;
      string test = LineBuffer.substr(11,LineBuffer.length()-11);
      GladOffsetZ=atof(test.c_str());
      if(verbose==1)
	cout << "GladOffsetZ = " << GladOffsetZ<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,10, "FieldAngle") == 0 && cFieldAngle == false) {
      cFieldAngle = true ;
      string test = LineBuffer.substr(10,LineBuffer.length()-10);
      FieldAngle=atof(test.c_str());
      if(verbose==1)
	cout << "FieldAngle = " << FieldAngle<<" deg"<<endl;
    }
    else if (LineBuffer.compare(0,12, "FieldOffsetX") == 0 && cFieldOffsetX == false) {
      cFieldOffsetX = true ;
      string test = LineBuffer.substr(12,LineBuffer.length()-12);
      FieldOffsetX=atof(test.c_str());
      if(verbose==1)
	cout << "FieldOffsetX = " << FieldOffsetX<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,12, "FieldOffsetY") == 0 && cFieldOffsetY == false) {
      cFieldOffsetY = true ;
      string test = LineBuffer.substr(12,LineBuffer.length()-12);
      FieldOffsetY=atof(test.c_str());
      if(verbose==1)
	cout << "FieldOffsetY = " << FieldOffsetY<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,12, "FieldOffsetZ") == 0 && cFieldOffsetZ == false) {
      cFieldOffsetZ = true ;
      string test = LineBuffer.substr(12,LineBuffer.length()-12);
      FieldOffsetZ=atof(test.c_str());
      if(verbose==1)
	cout << "FieldOffsetZ = " << FieldOffsetZ<<" cm"<<endl;
    }
    else if (LineBuffer.compare(0,14, "OutputFileName") == 0 && cOutputFileName == false) {
      cOutputFileName = true ;
      string test = LineBuffer.substr(14,LineBuffer.length()-14);
      OutputFileName = test.c_str();
      OutputFileName = OutputFileName.Strip(TString::kBoth,' ');
      if(verbose==1)
	cout << "OutputFileName = " << OutputFileName<<endl;
    }
  }
  ConfigFile.close();
  return   ;
}

void R3BGTPCSetup::Print() {
  cout << "TargetRadius = " << TargetRadius<<" cm"<<endl;
  cout << "TargetLength = " << TargetLength<<" cm"<<endl;
  cout << "ChamberInnerRadius = " << ChamberInnerRadius<<" cm"<<endl;
  cout << "ChamberThickness = " << ChamberThickness<<" cm"<<endl;
  cout << "ChamberLength = " << ChamberLength<<" cm"<<endl;
  cout << "InnerRohacellThickness = " << InnerRohacellThickness<<" cm"<<endl;
  cout << "KaptonThickness = " << KaptonThickness<<" cm"<<endl;
  cout << "OuterRohacellThickness = " << OuterRohacellThickness<<" cm"<<endl;
  cout << "TPCRadiusExt = " << TPCRadiusExt<<" cm"<<endl;
  cout << "TPCLx = " << TPCLx<<" cm"<<endl;
  cout << "TPCLy = " << TPCLy<<" cm"<<endl;
  cout << "TPCLz = " << TPCLz<<" cm"<<endl;
  cout << "WindowThickness = " << WindowThickness<<" cm"<<endl;
  cout << "TargetAngle = " << TargetAngle<<" deg"<<endl;
  cout << "TargetOffsetX = " << TargetOffsetX<<" cm"<<endl;
  cout << "TargetOffsetY = " << TargetOffsetY<<" cm"<<endl;
  cout << "TargetOffsetZ = " << TargetOffsetZ<<" cm"<<endl;
  cout << "GladAngle = " << GladAngle<<" deg"<<endl;
  cout << "GladOffsetX = " << GladOffsetX<<" cm"<<endl;
  cout << "GladOffsetY = " << GladOffsetY<<" cm"<<endl;
  cout << "GladOffsetZ = " << GladOffsetZ<<" cm"<<endl;
  cout << "FieldAngle = " << FieldAngle<<" deg"<<endl;
  cout << "FieldOffsetX = " << FieldOffsetX<<" cm"<<endl;
  cout << "FieldOffsetY = " << FieldOffsetY<<" cm"<<endl;
  cout << "FieldOffsetZ = " << FieldOffsetZ<<" cm"<<endl;
  cout << "OutputFileName = " << OutputFileName<<endl;
}
