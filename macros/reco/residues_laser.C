////////////////////////////////////////////////////////////////////////////////
//    - Macro for study the differences between GTPCPoints and GTPCHitData.
//
//    - Author: David García Allo
//    - Usage: root -l residues.C
////////////////////////////////////////////////////////////////////////////////

using namespace std;

void residues_laser(TString simFilename = "sim.root", TString recoFilename = "output_reco_laser.root")
{
    // Timer for runtime check
    TStopwatch timer;
    timer.Start();

    // Input files: simulation and reconstruction -> Compare Points and Hits
    TString simFilePath;
    TString recoFilePath;

    // Setting up env and paths
    TString workDir = gSystem->Getenv("VMCWORKDIR");                         // Setting work directory
    simFilePath = workDir + "/glad-tpc/macros/sim/Prototype/" + simFilename; // File paths
    recoFilePath = workDir + "/glad-tpc/macros/reco/" + recoFilename;

    // Opening Files
    TFile* simFile = new TFile(simFilePath);
    TFile* recoFile = new TFile(recoFilePath);

    // Checking that opened both files
    if (simFile->IsOpen() && recoFile->IsOpen())
    {
        std::cout << "[INFO] Files correctly opened!" << std::endl;
    }
    else
    {
        std::cout << "[ERROR] Files not opened!" << std::endl;
        exit(1);
    }

    // ------------------- Extracting info of the TREE ---------------------------

    // HITS INFO
    TTree* recoTree;
    TClonesArray* GTPCHitDataCA;
    TBranch* branchGTPCHitData;
    R3BGTPCHitData** hits;                                  // Posible error as in the simFile
    recoTree = (TTree*)recoFile->Get("evt");                // Get Tree
    GTPCHitDataCA = new TClonesArray("R3BGTPCHitData", 5);  // Create the array //What arg 5 means? Check it
    branchGTPCHitData = recoTree->GetBranch("GTPCHitData"); // Get the branch of Hits
    branchGTPCHitData->SetAddress(&GTPCHitDataCA);          // GTPCHitDataCA ref to the branch

    Long64_t reco_events = recoTree->GetEntries();

    auto* hzx = new TH2D("hxz", "XZ", 128, 0, 25.6, 44, 0, 8.8);
    auto* hxy = new TH2D("hxy", "XY", 150, 0, 30, 44, 0, 8.8);
    auto* hzy = new TH2D("hzy", "ZY", 128, 0, 25.6, 150, 0, 30.);

    auto* hzx2 = new TH2D("hxz2", "XZ", 128, 0, 25.6, 44, 0, 8.8);
    auto* hxy2 = new TH2D("hxy2", "XY", 150, 0, 30, 44, 0, 8.8);
    auto* hzy2 = new TH2D("hzy2", "ZY", 128, 0, 25.6, 150, 0, 30.);

    hzx->GetXaxis()->SetTitle("Z");
    hxy->GetXaxis()->SetTitle("Y");
    hzy->GetXaxis()->SetTitle("Z");

    hzx->GetYaxis()->SetTitle("X");
    hxy->GetYaxis()->SetTitle("X");
    hzy->GetYaxis()->SetTitle("Y");

    Int_t eventHits;

    // Main loop through all events
    for (Int_t i = 0; i < reco_events; i++)
    {
        // Getting the hits in event i
        GTPCHitDataCA->Clear();
        recoTree->GetEvent(i);
        eventHits = GTPCHitDataCA->GetEntries();

        // Create points and hits arrays and fill them
        if (eventHits > 0)
        {
            hits = new R3BGTPCHitData*[eventHits];
            for (Int_t j = 0; j < eventHits; j++)
            {
                hits[j] = (R3BGTPCHitData*)GTPCHitDataCA->At(j);
            }
        }

        // Create variables
        Double_t x;
        Double_t y;
        Double_t z;

        for (Int_t j = 0; j < eventHits; j++)
        {
            x = hits[j]->GetX();
            y = hits[j]->GetY() + 14.7;
            z = hits[j]->GetZ();

            hzx->Fill(z, x);
            hxy->Fill(y, x);
            hzy->Fill(z, y);

            std::cout << x << " " << y << " " << z << '\n';
        }

        // Event i finishes ==> Deleting the arrays
        if (eventHits)
            delete[] hits;
    }

    auto* gr = new TGraph2D(100);
    Double_t xval, yval, zval;
    Double_t fAlpha, fBeta, fZIn, fYIn;

    fAlpha = 11.67 / 180. * TMath::Pi();
    fBeta = 1.36 / 180. * TMath::Pi();
    fZIn = 6.98;
    fYIn = 24.312;

    for (Int_t i = 0; i < 100; i++)
    {
        Double_t xval = i * cos(fBeta) * sin(fAlpha);
        Double_t zval = fZIn + i * cos(fBeta) * cos(fAlpha);
        Double_t yval = fYIn + i * sin(fBeta);

        hzx2->Fill(zval, xval);
        hxy2->Fill(yval, xval);
        hzy2->Fill(zval, yval);
    }

    gr->SetPoint(0, 0, 0, 0);

    // Ending the macro
    auto* c = new TCanvas("", "", 8 * 128, 8 * 44);
    hzx2->Fit("pol1");
    hzx->Draw();
    hzx2->GetFunction("pol1")->Draw("same");
    auto* c1 = new TCanvas("", "", 150 * 8, 44 * 8);
    hxy2->Fit("pol1");
    hxy->Draw();
    hxy2->GetFunction("pol1")->Draw("same");
    auto* c2 = new TCanvas("", "", 5 * 128, 5 * 150);
    hzy2->Fit("pol1");
    hzy->Draw();
    hzy2->GetFunction("pol1")->Draw("same");

    // gr->Draw("same");
    // auto *c = new TCanvas();

    timer.Stop();
    cout << "\n[INFO] Macro finished succesfully !!" << endl;
    cout << "Real time: " << timer.RealTime() << "s; CPU time: " << timer.CpuTime() << "s" << endl;
}
