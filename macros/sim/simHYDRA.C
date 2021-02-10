/*
Author: Simone Velardita
Date:		27/04/2021
Macro for running the simulation. 
HOW TO USE:
	$	root -l
	$	.L simHYDRA.C
	$	simHYDRA(nevt,"Detector","version","generator")
ALTERNATIVE:
	use the macro run_simHYDRA.C
*/
void simHYDRA(Int_t nEvents = 0, TString GEOTAG = "Prototype", TString version= "v1", TString generator= "good_evt")
{
 Bool_t storeTrajectories = kTRUE;
 Bool_t magnet = kTRUE;
 Float_t fieldScale = -1.;
    
 TString transport = "TGeant4";
 cout<<"The generator used is:\033[1;32m"<< generator<<endl;
 TString inputFile;
 TString outFile;
 TString parFile;
 Bool_t printGLAD=kTRUE; 			 						//print the inner glad vessel and the HYDRA detector
 if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        outFile="./Prototype/sim.root";
        parFile="./Prototype/par.root";
        if (generator.CompareTo("good_evt") == 0)
        {
         inputFile ="../../gtpcgen/ASCII/input" + GEOTAG + "_3LH.dat";
        }
        if (generator.CompareTo("bkg_evt") == 0)
        {
         inputFile ="../../gtpcgen/ASCII/input" + GEOTAG + "_bkg.dat";
        }
    }

 if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        parFile="./FullBeamOut/par.root";
        outFile="./FullBeamOut/sim.root";
        if (generator.CompareTo("good_evt") == 0)
        {
         inputFile ="../../gtpcgen/ASCII/input" + GEOTAG + "_3LH.dat";
        }
        if (generator.CompareTo("bkg_evt") == 0)
        {
         inputFile ="../../gtpcgen/ASCII/input" + GEOTAG + "_bkg.dat";
        }
    }
    
 if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG<<"."<<version<< endl;
				if (version.CompareTo("v1")==0)
				{
				    parFile="./FullBeamIn/v1/par.root";
        		outFile="./FullBeamIn/v1/sim.root";
				}
				else if (version.CompareTo("v2")==0)
				{
				    parFile="./FullBeamIn/v2/par.root";
        		outFile="./FullBeamIn/v2/sim.root";
				}
				else if (version.CompareTo("v3")==0)
				{
				    parFile="./FullBeamIn/v3/par.root";
        		outFile="./FullBeamIn/v3/sim.root";
				}
        if (generator.CompareTo("good_evt") == 0)
        {
         inputFile ="../../gtpcgen/ASCII/input" + GEOTAG + "_3LH.dat";
        }
        if (generator.CompareTo("bkg_evt") == 0)
        {
         inputFile ="../../gtpcgen/ASCII/input" + GEOTAG + "_bkg.dat";
        }
    }

 Int_t randomSeed = 335566; // 0 for time-dependent random numbers

 // ------------------------------------------------------------------------

 TString dir = getenv("VMCWORKDIR");
 char str[1000];
 sprintf(str, "GEOMPATH=%s/glad-tpc/geometry", dir.Data());
 putenv(str);

 // ----    Debug option   -------------------------------------------------
 gDebug = 0;

 // -----   Timer   --------------------------------------------------------
 TStopwatch timer;
 timer.Start();

 // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName(transport);            // Transport engine
    run->SetOutputFile(outFile.Data()); // Output file
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media_tpc.geo"); // Materials

    // -----   Create R3B geometry --------------------------------------------
    // R3B Cave definition
    FairModule* cave = new R3BCave("CAVE");
    cave->SetGeometryFileName("r3b_cave.geo");
    run->AddModule(cave);

    // To skip the detector comment out the line with: run->AddModule(...

    // GLAD
    run->AddModule(new R3BGladMagnet("glad_v17_flange.geo.root")); // GLAD should not be moved or rotated

    // --- GLAD-TPC detectors 
    if (GEOTAG.CompareTo("Prototype") == 0)
    {	    
    			run->AddModule(new R3BTarget("C12 target","passive/Target.geo.root",{-0.7,0.,192.}));		
    			run->AddModule(new R3BGTPC("HYDRA_Prototype.geo.root"));
    }	
    else if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {	    
    			run->AddModule(new R3BTarget("C12 target","passive/Target.geo.root",{0.,0.,147.5}));			
    			run->AddModule(new R3BGTPC("HYDRA_FullBeamOut.geo.root"));
    }
    else if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {	    
    			run->AddModule(new R3BTarget("C12target","passive/Target.geo.root",{0.,0.,161.}));
    					 if (version.CompareTo("v1")==0) run->AddModule(new R3BGTPC("HYDRA_FullBeamIn.v1.geo.root"));
    			else if (version.CompareTo("v2")==0) run->AddModule(new R3BGTPC("HYDRA_FullBeamIn.v2.geo.root"));
    			else if (version.CompareTo("v3")==0) run->AddModule(new R3BGTPC("HYDRA_FullBeamIn.v3.geo.root"));
    }

    // -----   Create R3B  magnetic field ----------------------------------------
    // NB: <D.B>
    // If the Global Position of the Magnet is changed
    // the Field Map has to be transformed accordingly
    R3BGladFieldMap* magField = new R3BGladFieldMap("R3BGladMap");
    magField->SetScale(fieldScale);

    if (magnet == kTRUE)
    {
        run->SetField(magField);
    }
    else
    {
        run->SetField(NULL);
    }

    // -----   Create PrimaryGenerator   --------------------------------------
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    
    if (generator.CompareTo("box") == 0)
    {
    Int_t pdgId =-211;     // pi-
    Double32_t theta1 = 0; // polar angle distribution
    Double32_t theta2 = 1.;
    Double32_t momentum = 1.;
    FairBoxGenerator* boxGen = new FairBoxGenerator(pdgId, 1);
    boxGen->SetThetaRange(theta1, theta2);
    boxGen->SetPRange(momentum, momentum * 2.0);
    boxGen->SetPhiRange(0, 360);
    boxGen->SetXYZ(0.0, 0.0, 215.);
    primGen->AddGenerator(boxGen);
    }
    
		if (generator.CompareTo("bkg_evt") == 0||generator.CompareTo("good_evt") == 0)
    {
		R3BAsciiGenerator* gen = new R3BAsciiGenerator((inputFile).Data());
    primGen->AddGenerator(gen);
		}
    run->SetGenerator(primGen);
    run->SetStoreTraj(storeTrajectories);

    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");

    // -----   Initialize simulation run   ------------------------------------
    run->Init();
    TVirtualMC::GetMC()->SetRandom(new TRandom3(randomSeed));

    // ------  Increase nb of step for CALO
    Int_t nSteps = -15000;
    TVirtualMC::GetMC()->SetMaxNStep(nSteps);

    // -----   Runtime database   ---------------------------------------------
    R3BFieldPar* fieldPar = (R3BFieldPar*)rtdb->getContainer("R3BFieldPar");
    if (NULL != magField)
    {
        fieldPar->SetParameters(magField);
        fieldPar->setChanged();
    }
    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();

    // -----   Start run   ----------------------------------------------------
    if (nEvents > 0)
    {
        run->Run(nEvents);
    }

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is " << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;
    
if(printGLAD){
    				// Snap a picture of the geometry
    				// If this crashes, set "OpenGL.SavePicturesViaFBO: no" in your .rootrc or just comment
						gStyle->SetCanvasPreferGL(kTRUE);
				    gGeoManager->GetTopVolume()->Draw("ogl");
				    TGLViewer* v = (TGLViewer*)gPad->GetViewer3D();
				    v->SetStyle(TGLRnrCtx::kOutline);
			 			v->GetClipSet()->SetClipType(TGLClip::EType(1));
				    v->RequestDraw();
				    v->SetCurrentCamera(TGLViewer::kCameraOrthoXnOZ);
				    v->SavePicture("GLAD-top.png");
        }
}
