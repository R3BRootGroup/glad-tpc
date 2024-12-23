void test_padplane()
{

    gSystem->Load("libR3BGTPCMap.so");
    auto* gladTPC = new R3BGTPCMap();
    gladTPC->GeneratePadPlane();

    TH2Poly* padplane = gladTPC->GetPadPlane();

    for (auto i = 0; i < 200; ++i)
    {

        Int_t bin = padplane->Fill(i + 0.5 + 272.0, i + 0.5 + 5.0, i * 100);
    }

    auto c = new TCanvas("R3BTPCPlane", "R3BTPCPlane", 1000, 1000);
    gStyle->SetPalette(1);
    padplane->Draw("COL L");

    gPad->Update();
}
