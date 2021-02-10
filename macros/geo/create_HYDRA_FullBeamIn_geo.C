#include "TFile.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoTrd2.h"
#include "TGeoArb8.h"
#include "TMath.h"
#include <iomanip>
#include <iostream>

TGeoRotation* zeroRot = new TGeoRotation;
TGeoTranslation* zeroTrans = new TGeoTranslation;
TGeoCombiTrans* zeroRotTrans = new TGeoCombiTrans(*zeroTrans, *zeroRot);

// Create a null translation to be used in the world
TGeoTranslation* fGlobalTrans = new TGeoTranslation();

TGeoManager* gGeoMan = NULL;

//________________________HYDRA parameters
Double_t TPCLx;
Double_t TPCLy;
Double_t TPCLz;
Double_t FrameThickness;
Double_t ActiveRegionx;
Double_t ActiveRegiony;
Double_t ActiveRegionz;
Double_t Windowx;
Double_t Windowy;
Double_t Windowz;

Double_t WorldSizeX;
Double_t WorldSizeY;
Double_t WorldSizeZ;

void ConstructTPC(TGeoVolume* pWorld);

string VERSION;
void create_tpc_geo(string geoTag = "FullBeamIn", const char* version = "v2")
{
		VERSION= std::string(version);													
    fGlobalTrans->SetTranslation(0.0, 0.0, 0.0);

    // -------   Load media from media file   -----------------------------------
    FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
    FairGeoInterface* geoFace = geoLoad->getGeoInterface();
    TString geoPath = gSystem->Getenv("VMCWORKDIR");
    TString medFile = geoPath + "/glad-tpc/geometry/media_tpc.geo";
    geoFace->setMediaFile(medFile);
    geoFace->readMedia();
    gGeoMan = gGeoManager;
    // --------------------------------------------------------------------------

    // -------   Geometry file name (output)   ----------------------------------
    TString geoFileName = geoPath + "/glad-tpc/geometry/HYDRA_";
    geoFileName = geoFileName + geoTag+ "."+ VERSION + ".geo.root";
    // --------------------------------------------------------------------------

    // -----------------   Get and create the required media    -----------------
    FairGeoMedia* geoMedia = geoFace->getMedia();
    FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

    FairGeoMedium* mMy = geoMedia->getMedium("mylar");
    if (!mMy)
        Fatal("Main", "FairMedium mylar not found");
    geoBuild->createMedium(mMy);
    TGeoMedium* pMy = gGeoMan->GetMedium("mylar");
    if (!pMy)
        Fatal("Main", "Medium mylar not found");

    FairGeoMedium* mAl = geoMedia->getMedium("aluminium");
    if (!mAl)
        Fatal("Main", "FairMedium aluminium not found");
    geoBuild->createMedium(mAl);
    TGeoMedium* pAl = gGeoMan->GetMedium("aluminium");
    if (!pAl)
        Fatal("Main", "Medium aluminium not found");
    // this is the T2K gas mixture
    FairGeoMedium* mmix = geoMedia->getMedium("mix");
    if (!mmix)
        Fatal("Main", "FairMedium mix not found");
    geoBuild->createMedium(mmix);
    TGeoMedium* pmix = gGeoMan->GetMedium("mix");
    if (!pmix)
        Fatal("Main", "Medium mix not found");

    FairGeoMedium* mvacuum = geoMedia->getMedium("vacuum");
    if (!mvacuum)
        Fatal("Main", "FairMedium vacuum not found");
    geoBuild->createMedium(mvacuum);
    TGeoMedium* pvacuum = gGeoMan->GetMedium("vacuum");
    if (!pvacuum)
        Fatal("Main", "Medium vacuum not found");

    FairGeoMedium* mair = geoMedia->getMedium("Air");
    if (!mair)
        Fatal("Main", "FairMedium Air not found");
    geoBuild->createMedium(mair);
    TGeoMedium* pair = gGeoMan->GetMedium("Air");
    if (!pair)
        Fatal("Main", "Medium Air not found");

    //_________________________________

    // --------------   Create geometry and top volume  -------------------------
    gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
    gGeoMan->SetName("GTPCgeom");
    TGeoVolume* top = new TGeoVolumeAssembly("TOP");
    gGeoMan->SetTopVolume(top);

    top->SetVisContainers(kTRUE);
    // --------------------------------------------------------------------------

    cout << "\n \033[1;31m Warning\033[0m: the detector you are building is " << geoTag<<"."<<VERSION << "!!!!!\n" << endl;
    
    R3BGTPCSetup* setup = new R3BGTPCSetup(geoTag, 1);
    
    TPCLx = setup->GetTPCLx() / 2.;                   // cm
    TPCLy = setup->GetTPCLy() / 2.;                   // cm
    TPCLz = setup->GetTPCLz() / 2.;                   // cm
    FrameThickness = setup->GetFrameThickness() / 2.; // cm

    ActiveRegionx = setup->GetActiveRegionx() / 2.; // cm
    ActiveRegiony = setup->GetActiveRegiony() / 2.; // cm
    ActiveRegionz = setup->GetActiveRegionz() / 2.; // cm

    Windowx = setup->GetWindowx() / 2.; // cm
    Windowy = setup->GetWindowy() / 2.; // cm
    Windowz = setup->GetWindowz() / 2.; // cm

    delete setup;

    /*
      TPCLx 							 	 =22.5														//cm
      TPCLy 							   =22.5.														//cm
      TPCLz  							   =120.														//cm

      ActiveRegionx					 =4.															//cm
      ActiveRegiony					 =30.															//cm
      ActiveRegionz					 =90.															//cm
      FrameThickness	 			 = 1 															//cm
      */

    // World definition

    WorldSizeX = 3.5 * TPCLx;
    WorldSizeY = 3.5 * TPCLy;
    WorldSizeZ = 2 * TPCLz;

    // GTPC main area->World definition
    TGeoVolume* pAWorld = gGeoManager->GetTopVolume();

    TGeoShape* tpc_box = new TGeoBBox("GTPC_Box", WorldSizeX, WorldSizeY, WorldSizeZ);

    TGeoVolume* pWorld = new TGeoVolume("GTPC_box", tpc_box, pvacuum);
		pWorld->SetInvisible();
    ConstructTPC(pWorld);

    //_____________________________Globle definition of TPC position in cm
    
    TGeoRotation* rot_chamber = new TGeoRotation();
    rot_chamber->RotateY(-14.);
    TGeoCombiTrans* t0 = new TGeoCombiTrans(-21.5, 0., 225., rot_chamber);
    top->AddNode(pWorld, 0, t0);

    //____________________________________
    gGeoMan->CloseGeometry();
    gGeoMan->CheckOverlaps(0.001);
    gGeoMan->PrintOverlaps();
    gGeoMan->Test();

    cout << "vis option " << gGeoMan->GetVisOption() << " =====  vis level " << gGeoMan->GetVisLevel() << endl;

    TFile* geoFile = new TFile(geoFileName, "RECREATE");
    top->Draw("ogl");
    top->Write();
    geoFile->Close();
}
//________________________________________________________________
void ConstructTPC(TGeoVolume* pWorld)
{
    // Material-----------------------------------------------------------------
    TGeoMedium* FrameMaterial = gGeoManager->GetMedium("aluminium");
    TGeoMedium* GasMaterial = gGeoManager->GetMedium("mix");
    TGeoMedium* WindowMaterial = gGeoManager->GetMedium("mylar");

    // Sensitive volumes-------------------------------------------------------------------
    TGeoShape* solidFrame = 0;
    TGeoVolume* logicFrame = 0;
    TGeoShape* solidGas = 0;
    TGeoVolume* logicGas = 0;
    TGeoShape* solidActiveRegion = 0;
    TGeoVolume* logicActiveRegion = 0;
    TGeoShape* solidActiveRegion1 = 0;			//Version2
    TGeoVolume* logicActiveRegion1 = 0;			//Version2
    TGeoShape* solidFWindow = 0;
    TGeoVolume* logicFWindow = 0;
    TGeoShape* solidBWindow = 0;
    TGeoVolume* logicBWindow = 0;

   
    // Aluminium frame is the same for the diffent versions_______________________________________
    TGeoShape *frame_box =new TGeoTrd2("frame_box", 
    																		 1.15 * TPCLx, //
    																		 3.3 * TPCLx, 
    																		 1.5 * TPCLy, //
    																		 2.5 * TPCLy,	//
    																		 			 TPCLz);
    																		 
		TGeoShape* frame_window1 = new TGeoBBox("frame_window1", 
                                            2 * Windowx,
                                            Windowy,
                                            FrameThickness);
    TGeoShape* frame_window2 = new TGeoBBox("frame_window2", 
                                            6  * Windowx,
                                            9  * Windowy,
                                            FrameThickness);
                                          
		//Windows translation
    TGeoTranslation* tc1 = new TGeoTranslation("tc1", 0, 0, -(TPCLz -  FrameThickness));
    tc1->RegisterYourself();
    TGeoTranslation* tc2 = new TGeoTranslation("tc2", 0, 0, (TPCLz - FrameThickness));
    tc2->RegisterYourself();

    solidFrame = new TGeoCompositeShape("Frame", "frame_box -frame_window1:tc1 -frame_window2:tc2");
    logicFrame = new TGeoVolume("Frame", solidFrame, FrameMaterial);
    
		//Placing logicframe in the world
    pWorld->AddNode(logicFrame, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));
    
    // Gas region-----------------------------------------
    solidGas = new TGeoTrd2("gas",
                            1.15*  TPCLx - 2 * FrameThickness,
                            3.3 * TPCLx - 2 * FrameThickness,
                            1.5 * TPCLy - 2 * FrameThickness,
                            2.5 * TPCLy - 2 * FrameThickness,
                            TPCLz - 2 * FrameThickness);

    logicGas = new TGeoVolume("gas", solidGas, GasMaterial);
    
    //Placing GAS in the Frame
    logicFrame->AddNode(logicGas, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));  
    
    // Mylar windows---------------------------------------
    solidFWindow = new TGeoBBox("front_window", 2 * Windowx, 
    																						Windowy, 
    																						Windowz);
    logicFWindow = new TGeoVolume("front_window", solidFWindow, WindowMaterial);

    solidBWindow = new TGeoBBox("back_window", 6  * Windowx, 
    																					 9 * Windowy, 
    																							 Windowz);
    logicBWindow = new TGeoVolume("back_window", solidBWindow, WindowMaterial);
    //Placing logicWindows in the Frame
    logicFrame->AddNode(logicFWindow, 0, new TGeoCombiTrans(*tc1, *zeroRot));
    logicFrame->AddNode(logicBWindow, 0, new TGeoCombiTrans(*tc2, *zeroRot));  
    
//---------------The Active Region is different for the different versions-------------------------//  
     
		TGeoRotation rot_Aregion;					//Active region rotation
		rot_Aregion.RotateZ(180);
		TGeoRotation rot_Aregion1;				//Active region rotation, only v2
		rot_Aregion1.RotateZ(180);

//Version 1->	1 active region with constant height________________________________________
    if(VERSION.compare("v1")==0)
    {
    	 Double_t volume[]={  -ActiveRegionx   ,  ActiveRegiony ,
														 ActiveRegionx   ,  ActiveRegiony ,
														 ActiveRegionx   , -ActiveRegiony ,
														-ActiveRegionx   , -ActiveRegiony ,
														-3.3*ActiveRegionx ,  ActiveRegiony ,
													   ActiveRegionx   ,  ActiveRegiony ,
														 ActiveRegionx   , -ActiveRegiony ,
													  -3.3*ActiveRegionx , -ActiveRegiony  
  									 			}; 
    	solidActiveRegion =
        new TGeoArb8("Active_region",
        							ActiveRegionz,
        							volume);
      rot_Aregion.RotateY(10.6);
		  cout<<"The active region is an Arb8 with dimensions: x1=" <<	2*ActiveRegionx 	<<
		  																									 " x2=" <<	3.3*ActiveRegionx	  <<
		  																									 "	y="	<<	2*ActiveRegiony	  <<
		  																									 "	z="	<<	2*ActiveRegionz	  <<endl;
    }
    
//Version 2->  2 active regions with different height_____________________________________
	     									    
   if(VERSION.compare("v2")==0)
    {
		   Double_t volume1[]={ -ActiveRegionx   ,  ActiveRegiony ,
														 			 5.   		 ,  ActiveRegiony ,
														 			 5.    		 , -ActiveRegiony ,
														-ActiveRegionx   , -ActiveRegiony ,
																	-13.25      ,  ActiveRegiony ,
													         5         ,  ActiveRegiony ,
														       5         , -ActiveRegiony ,
													        -13.25      , -ActiveRegiony  
  									 			};   
			Double_t volume2[]={  
																	    -13.6 ,  18. ,
																		   5.    ,  18. ,
																		   5.    , -18. ,
																	    -13.6 , -18. ,
												 -3.32*ActiveRegionx    ,  18. ,
																	     5     ,  18. ,
																		   5     , -18. ,
												 -3.32*ActiveRegionx    , -18. 
													};  	
      rot_Aregion.RotateY(9.35);
    	solidActiveRegion =
        new TGeoArb8("Active_region",
        							49./2,
        							volume1);
      rot_Aregion1.RotateY(9.45);
      solidActiveRegion1 =
        new TGeoArb8("Active_region",
        							49./2,
        							volume2);
      cout<<"The active region are 2 Arb8. \nArb8 1, with dimensions: x1="<<	11.	<<
																																		"	x2="<<	20.8	<<
																																		"	y="	<<	26.	<<
																																		"	z="	<<	49.	<<
		  																		"\nArb8 2, with dimensions: x1="<<	21.16	<<
																																		"	x2="<<	30.<<
																																		"	y="	<<	36.	<<
																																		"	z="	<<	49.	<<endl;
    }

//Version 3->  1 active regions with increasing height____________________________________
    if(VERSION.compare("v3")==0)
    {
    	 Double_t volume[]={  -ActiveRegionx   ,  ActiveRegiony ,
														 ActiveRegionx   ,  ActiveRegiony ,
														 ActiveRegionx   , -ActiveRegiony ,
														-ActiveRegionx   , -ActiveRegiony ,
														-3.3*ActiveRegionx ,  1.65*ActiveRegiony ,
													   ActiveRegionx   ,  1.65*ActiveRegiony ,
														 ActiveRegionx   , -1.65*ActiveRegiony ,
													  -3.3*ActiveRegionx , -1.65*ActiveRegiony  
  									 			}; 
    	solidActiveRegion =
        new TGeoArb8("Active_region",
        							ActiveRegionz,
        							volume);
      rot_Aregion.RotateY(10.6);
		  cout<<"The active region is a Arb8 with dimensions: x1="<<	2*ActiveRegionx	  <<
		  																									"	x2="<<	4*ActiveRegionx	  <<
		  																									"	y1="<<	2*ActiveRegiony		<<
		  																									"	y2="<<	3.5*ActiveRegiony	<<		
		  																									"	z="	<<	2*ActiveRegionz		<<endl;
    }
    //Logical volume active region
    logicActiveRegion = new TGeoVolume("Active_region", solidActiveRegion, GasMaterial); 
    
    //Putting logicActiveRegion inside GAS

    if (VERSION.compare("v2")==0)
    {
      TGeoCombiTrans* fTPCTrans  = new TGeoCombiTrans(20.25, 0., -25., new TGeoRotation(rot_Aregion));
    	logicGas->AddNode(logicActiveRegion, 0, fTPCTrans);
      TGeoCombiTrans* fTPCTrans1 = new TGeoCombiTrans(29, 0., 26.5,new TGeoRotation(rot_Aregion1) );
      logicActiveRegion1 = new TGeoVolume("Active_region", solidActiveRegion1, GasMaterial);			
      logicGas->AddNode(logicActiveRegion1, 0, fTPCTrans1);
    	logicActiveRegion1->SetLineColor(kMagenta);															
    } 
    else
  	{
  	  TGeoCombiTrans* fTPCTrans = new TGeoCombiTrans(26.5, 0., 4, new TGeoRotation(rot_Aregion));//24 0 2
    	logicGas->AddNode(logicActiveRegion, 0, fTPCTrans);
  	}
    
    // Some colours to better distinguish the logic volumes    
    logicFrame->SetLineColor(kBlue);
    logicGas->SetLineColor(kRed);
    logicFWindow->SetLineColor(kYellow);
    logicBWindow->SetLineColor(kOrange);
    logicActiveRegion->SetLineColor(kMagenta);          

}
