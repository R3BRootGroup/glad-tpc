#include "fitter.h"
#include "R3BGTPCFitter.h"

int main(int argc, char* argv[])
{

    Bool_t fInteractiveMode = 0;
    Double_t M_Ener = 0.139570; // Pion mass in GeV

    // Histograms
    TH1F* momentum = new TH1F("momentum", "momentum", g1000, 0, 2.0);
    TH1F* momentumXtr = new TH1F("momentumXtr", "momentumXtr", 1000, 0, 2.0);
    TH1F* energy = new TH1F("energy", "energy", 1000, 0, 2.0);
    TH2F* XYVertex = new TH2F("XYVertex", "XYVertex", 1000, -25, 25, 1000, -25, 25);
    TH2F* XZVertex = new TH2F("XZVertex", "XZVertex", 1000, -25, 25, 1000, 200, 300);
    TH1F* thetaH = new TH1F("thetaH", "thetaH", 1000, -180.0, 180.0);
    TH1F* phiH = new TH1F("phiH", "phiH", 1000, -180.0, 180.0);
    TH1F* thetaIniH = new TH1F("thetaIniH", "thetaIniH", 1000, -180.0, 180.0);
    TH1F* phiIniH = new TH1F("phiIniH", "phiIniH", 1000, -180.0, 180.0);
    TH1F* phiDeltaH = new TH1F("phiDeltaH", "phiDeltaH", 1000, -180.0, 180.0);
    TH1F* xVertexH = new TH1F("xVertexH", "xVertexH", 1000, -50, 50);
    TH1F* yVertexH = new TH1F("yVertexH", "yVertexH", 1000, -50, 50);
    TH1F* zVertexH = new TH1F("zVertexH", "zVertexH", 1000, 200, 300);

    // Paths
    TString dir = getenv("VMCWORKDIR");

    TString geoManFile = dir + "/glad-tpc/geometry/HYDRA_Prototype.geoMan.root";

    std::cout << " Geometry file : " << geoManFile.Data() << "\n";

    TString fileName = dir + "/glad-tpc/macros/tracking/output_tracking2.root";
    TFile* file = new TFile(fileName.Data(), "READ");

    std::cout << "Tracking file : " << fileName.Data() << "\n";

    // GENFIT geometry
    new TGeoManager("Geometry", "HYDRA geometry");
    TGeoManager::Import(geoManFile.Data());
    // genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.0, 20.0, 0.0)); //
    // genfit::FieldManager::getInstance()->useCache(true, 8);
    // auto fieldPointer = GFBField::GetInstance("R3BGladMap","A",0,1.75,163.4);
    // genfit::FieldManager::getInstance()->init(fieldPointer);
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
    // genfit::MaterialEffects::getInstance()->setNoEffects(true);
    // genfit::MaterialEffects::getInstance()->ignoreBoundariesBetweenEqualMaterials(true);
    // genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(false);
    // genfit::MaterialEffects::getInstance()->setEnergyLossBrems(false);
    // genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(false);
    // genfit::MaterialEffects::getInstance()->setNoiseBrems(false);
    // genfit::MaterialEffects::getInstance()->setNoiseCoulomb(false);

    // Target
    genfit::SharedPlanePtr fTargetPlane;
    // TVector3 posTargetIni(-2.7, 0.0, 237.0);
    TVector3 posTargetIni(4.2, 0, 277.9);
    // TVector3 posTargetIni(4.2,0,260.5);
    TVector3 normalTarget(0, 0, 1);
    fTargetPlane = genfit::SharedPlanePtr(new genfit::DetPlane(posTargetIni, normalTarget));

    // event display
    genfit::EventDisplay* display;

    if (fInteractiveMode)
        display = genfit::EventDisplay::getInstance();

    R3BGTPCFitter* fitter = new R3BGTPCFitter();

    TTree* tree = (TTree*)file->Get("evt");
    Int_t nEvents = tree->GetEntries();
    std::cout << " Number of events : " << nEvents << std::endl;

    TTreeReader Reader1("evt", file);
    TTreeReaderValue<TClonesArray> trackArray(Reader1, "GTPCTrackData");
    // Reader1.SetEntriesRange(0, nEvents);

    TFile* file_out = new TFile("genfit_pions.root", "RECREATE");
    TTree* t_out = new TTree("tree", "combined");

    Int_t event_no;
    t_out->Branch("event_no", &event_no, "event_no/I");
    Int_t Ntrack;
    t_out->Branch("Ntrack", &Ntrack, "Ntrack/I");
    UInt_t gf_Niter[100];
    t_out->Branch("gf_Niter", &gf_Niter, "gf_Niter[Ntrack]/I");
    Double_t gf_TrackLen[100], gf_fChi2[100], gf_bChi2[100], gf_fNdf[100], gf_bNdf[100];
    t_out->Branch("gf_TrackLen", &gf_TrackLen, "gf_TrackLen[Ntrack]/D");
    t_out->Branch("gf_fChi2", &gf_fChi2, "gf_fChi2[Ntrack]/D");
    t_out->Branch("gf_bChi2", &gf_bChi2, "gf_bChi2[Ntrack]/D");
    t_out->Branch("gf_fNdf", &gf_fNdf, "gf_fNdf[Ntrack]/D");
    t_out->Branch("gf_bNdf", &gf_bNdf, "gf_bNdf[Ntrack]/D");
    Double_t gf_Px[100], gf_Py[100], gf_Pz[100], gf_P[100], gf_Theta[100], gf_Phi[100];
    t_out->Branch("gf_Px", &gf_Px, "gf_Px[Ntrack]/D");
    t_out->Branch("gf_Py", &gf_Py, "gf_Py[Ntrack]/D");
    t_out->Branch("gf_Pz", &gf_Pz, "gf_Pz[Ntrack]/D");
    t_out->Branch("gf_P", &gf_P, "gf_P[Ntrack]/D");
    t_out->Branch("gf_Theta", &gf_Theta, "gf_Theta[Ntrack]/D");
    t_out->Branch("gf_Phi", &gf_Phi, "gf_Phi[Ntrack]/D");
    Double_t gf_X[100], gf_Y[100], gf_Z[100];
    t_out->Branch("gf_X", &gf_X, "gf_X[Ntrack]/D");
    t_out->Branch("gf_Y", &gf_Y, "gf_Y[Ntrack]/D");
    t_out->Branch("gf_Z", &gf_Z, "gf_Z[Ntrack]/D");
    Double_t gf_Pin[100], gf_Thetain[100], gf_Phiin[100];
    t_out->Branch("gf_Pin", &gf_Pin, "gf_Pin[Ntrack]/D");
    t_out->Branch("gf_Thetain", &gf_Thetain, "gf_Thetain[Ntrack]/D");
    t_out->Branch("gf_Phiin", &gf_Phiin, "gf_Phiin[Ntrack]/D");
    Double_t gf_Pxin[100], gf_Pyin[100];
    t_out->Branch("gf_Pxin", &gf_Pxin, "gf_Pxin[Ntrack]/D");
    t_out->Branch("gf_Pyin", &gf_Pyin, "gf_Pyin[Ntrack]/D");
    Double_t gf_Xin[100], gf_Yin[100], gf_Zin[100];
    t_out->Branch("gf_Xin", &gf_Xin, "gf_Xin[Ntrack]/D");
    t_out->Branch("gf_Yin", &gf_Yin, "gf_Yin[Ntrack]/D");
    t_out->Branch("gf_Zin", &gf_Zin, "gf_Zin[Ntrack]/D");

    TFile* file_gen = new TFile("./gen_sim2.root");
    TTree* t_gen = (TTree*)file_gen->Get("tree");

    Double_t TPC_Xin, TPC_Yin, TPC_Zin, TPC_Pin, TPC_Theta, TPC_Phi;
    t_gen->SetBranchAddress("TPC_Xin", &TPC_Xin);
    t_gen->SetBranchAddress("TPC_Yin", &TPC_Yin);
    t_gen->SetBranchAddress("TPC_Zin", &TPC_Zin);
    t_gen->SetBranchAddress("TPC_Pin", &TPC_Pin);
    t_gen->SetBranchAddress("TPC_Theta", &TPC_Theta);
    t_gen->SetBranchAddress("TPC_Phi", &TPC_Phi);

    for (Int_t i = 0; i < nEvents; i++)
    { // nEvents
        // for (Int_t i=0; i<1000; i++){
        // Reset variables

        std::cout << cGREEN << " ------ Event Number : " << i << cNORMAL << "\n";

        Reader1.Next();

        Int_t nTracks = trackArray->GetEntries();
        std::cout << " Number of tracks " << nTracks << "\n";

        event_no = i;
        Ntrack = nTracks;

        t_gen->GetEvent(i);
        posTargetIni.SetXYZ(TPC_Xin, TPC_Yin, TPC_Zin);

        R3BGTPCTrackData** trackData;
        trackData = new R3BGTPCTrackData*[nTracks];

        for (auto iTrack = 0; iTrack < nTracks; ++iTrack)
        {
            trackData[iTrack] = (R3BGTPCTrackData*)(trackArray->At(iTrack));

            fitter->Init();
            genfit::Track* fitTrack;

            fitTrack = fitter->FitTrack(trackData[iTrack], TPC_Pin, TPC_Theta, TPC_Phi);

            TVector3 pos_res;
            TVector3 mom_res;
            TMatrixDSym cov_res;
            Double_t pVal = 0;
            Double_t bChi2 = 0, fChi2 = 0, bNdf = 0, fNdf = 0;

            try
            {

                if (fitTrack && fitTrack->hasKalmanFitStatus())
                {
                    genfit::MaterialEffects::getInstance()->setNoEffects(false);
                    auto KalmanFitStatus = fitTrack->getKalmanFitStatus();
                    auto trackRep = fitTrack->getTrackRep(0); // Only one representation is sved for the moment.

                    if (KalmanFitStatus->isFitConverged())
                    {
                        // KalmanFitStatus->Print();
                        genfit::MeasuredStateOnPlane fitState = fitTrack->getFittedState();
                        // fitState.Print();
                        fitState.getPosMomCov(pos_res, mom_res, cov_res);
                        fChi2 = KalmanFitStatus->getForwardChi2();
                        bChi2 = KalmanFitStatus->getBackwardChi2();
                        fNdf = KalmanFitStatus->getForwardNdf();
                        bNdf = KalmanFitStatus->getBackwardNdf();

                        gf_Niter[iTrack] = KalmanFitStatus->getNumIterations();
                        gf_TrackLen[iTrack] = KalmanFitStatus->getTrackLen();
                        gf_fChi2[iTrack] = fChi2;
                        gf_bChi2[iTrack] = bChi2;
                        gf_fNdf[iTrack] = fNdf;
                        gf_bNdf[iTrack] = bNdf;

                        gf_Px[iTrack] = mom_res.Px();
                        gf_Py[iTrack] = mom_res.Py();
                        gf_Pz[iTrack] = mom_res.Pz();
                        gf_P[iTrack] = mom_res.Mag();
                        gf_Theta[iTrack] = mom_res.Theta() * 180. / TMath::Pi();
                        gf_Phi[iTrack] = mom_res.Phi() * 180. / TMath::Pi();
                        gf_X[iTrack] = pos_res.X();
                        gf_Y[iTrack] = pos_res.Y();
                        gf_Z[iTrack] = pos_res.Z();

                        if (fInteractiveMode)
                            display->addEvent(fitTrack);

                        momentum->Fill(mom_res.Mag());
                        Double_t E = TMath::Sqrt(TMath::Power(mom_res.Mag(), 2) + TMath::Power(M_Ener, 2)) - M_Ener;
                        energy->Fill(E);

                        thetaIniH->Fill(mom_res.Theta() * TMath::RadToDeg());
                        phiIniH->Fill(mom_res.Phi() * TMath::RadToDeg());
                        Double_t phiIni = mom_res.Phi() * TMath::RadToDeg();

                        TVector3 momTarget(-99999, -99999, -99999);
                        TVector3 posTarget(-99999, -99999, -99999);
                        Float_t stepXtr = -1.0;

                        // Extrapolation
                        std::cout << " -------- Extrapolation -------- "
                                  << "\n";

                        try
                        {

                            genfit::MaterialEffects::getInstance()->setNoEffects(true);
                            trackRep->extrapolateToPoint(fitState, posTargetIni);
                            // trackRep -> extrapolateToPlane(fitState, fTargetPlane);
                            momTarget = fitState.getMom();
                            posTarget = fitState.getPos();

                            std::cout << " Position: ";
                            posTarget.Print();
                            std::cout << " Momentum: ";
                            momTarget.Print();

                            gf_Pin[iTrack] = momTarget.Mag();
                            gf_Thetain[iTrack] = momTarget.Theta() * 180. / TMath::Pi();
                            gf_Phiin[iTrack] = momTarget.Phi() * 180. / TMath::Pi();
                            gf_Pxin[iTrack] = momTarget.X();
                            gf_Pyin[iTrack] = momTarget.Y();
                            gf_Xin[iTrack] = posTarget.X();
                            gf_Yin[iTrack] = posTarget.Y();
                            gf_Zin[iTrack] = posTarget.Z();

                            // posTarget.Print();

                            /*for (auto iStep = 0; iStep < 50; ++iStep) {

                              //std::cout<<" Step length : "<<stepXtr*iStep<<"\n";
                              trackRep->extrapolateBy(fitState, stepXtr );
                                        momTarget = fitState.getMom();
                                        posTarget = fitState.getPos();
                            std::cout<<" Position: ";
                            posTarget.Print();
                            std::cout<<" Momentum: ";
                            momTarget.Print();

                            }*/

                            thetaH->Fill(momTarget.Theta() * TMath::RadToDeg());
                            phiH->Fill(momTarget.Phi() * TMath::RadToDeg());
                            Double_t phiEnd = momTarget.Phi() * TMath::RadToDeg();
                            phiDeltaH->Fill(phiIni - phiEnd);
                            XYVertex->Fill(posTarget.X(), posTarget.Y());
                            XZVertex->Fill(posTarget.X(), posTarget.Z());
                            xVertexH->Fill(posTarget.X());
                            yVertexH->Fill(posTarget.Y());
                            zVertexH->Fill(posTarget.Z());
                            momentumXtr->Fill(momTarget.Mag());
                        }
                        catch (genfit::Exception& e)
                        {
                        }

                    } // Fit converged
                }
            }
            catch (std::exception& e)
            {
                std::cout << " " << e.what() << "\n";
                continue;
            }

        } // Tracks

        t_out->Fill();
        if (trackData)
            delete trackData;
    }

    if (fInteractiveMode)
    {

        TCanvas* c1 = new TCanvas();
        c1->Divide(2, 2);
        c1->Draw();
        c1->cd(1);
        momentum->Draw();
        c1->cd(2);
        energy->Draw();
        c1->cd(3);
        XYVertex->Draw("zcol");
        c1->cd(4);
        XZVertex->Draw("zcol");

        TCanvas* c2 = new TCanvas();
        c2->Divide(3, 3);
        c2->cd(1);
        thetaIniH->Draw();
        c2->cd(2);
        phiIniH->Draw();
        c2->Draw();
        c2->cd(3);
        thetaH->Draw();
        c2->cd(4);
        phiH->Draw();
        c2->cd(5);
        phiDeltaH->Draw();
        c2->cd(6);
        momentumXtr->Draw();

        TCanvas* c3 = new TCanvas();
        c3->Divide(2, 2);
        c3->cd(1);
        xVertexH->Draw();
        c3->cd(2);
        yVertexH->Draw();
        c3->cd(3);
        zVertexH->Draw();

        // open event display
        display->open();

    } // Interactive mode

    file_gen->Close();
    file_out->cd();
    file_out->Write();
    file_out->Close();
    return 0;
}

HelixTrackModel::HelixTrackModel(const TVector3& pos, const TVector3& mom, double charge)
{

    mom_ = mom.Mag();

    TVector3 B = genfit::FieldManager::getInstance()->getFieldVal(pos);

    // B must point in Z direction
    assert(B.X() == 0);
    assert(B.Y() == 0);

    double Bz = B.Z();

    // calc helix parameters
    TVector3 dir2D(mom);
    dir2D.SetZ(0);
    dir2D.SetMag(1.);
    R_ = 100. * mom.Perp() / (0.0299792458 * Bz) / fabs(charge);
    sgn_ = 1;
    if (charge < 0)
        sgn_ = -1.;
    center_ = pos + sgn_ * R_ * dir2D.Orthogonal();
    alpha0_ = (pos - center_).Phi();

    theta_ = mom.Theta();

    // std::cout<<"radius " << R_ << "  center ";
    // center_.Print();
}

TVector3 HelixTrackModel::getPos(double tracklength) const
{

    TVector3 pos;

    double angle = alpha0_ - sgn_ * tracklength / R_ * sin(theta_);

    TVector3 radius(R_, 0, 0);
    radius.SetPhi(angle);
    pos = center_ + radius;
    pos.SetZ(center_.Z() - sgn_ * ((alpha0_ - angle) * R_ * tan(theta_ - M_PI / 2.)));

    return pos;
}

void HelixTrackModel::getPosMom(double tracklength, TVector3& pos, TVector3& mom) const
{

    double angle = alpha0_ - sgn_ * tracklength / R_ * sin(theta_);

    TVector3 radius(R_, 0, 0);
    radius.SetPhi(angle);
    pos = center_ + radius;
    pos.SetZ(center_.Z() - sgn_ * ((alpha0_ - angle) * R_ * tan(theta_ - M_PI / 2.)));

    mom.SetXYZ(1, 1, 1);
    mom.SetTheta(theta_);
    mom.SetPhi(angle - sgn_ * M_PI / 2.);
    mom.SetMag(mom_);

    /*std::cout<<"tracklength " << tracklength << "\n";
    std::cout<<"angle " << angle << "\n";
    std::cout<<"radius vector "; radius.Print();
    std::cout<<"pos "; pos.Print();
    std::cout<<"mom "; mom.Print();*/
}
