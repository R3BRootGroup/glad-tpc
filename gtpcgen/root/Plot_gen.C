{

    gStyle->SetPalette(55);

    TChain* t_data = new TChain("tree_input");
    char tmp_str[200];

    sprintf(tmp_str, "./input_beam_dcm_C12C12_1.9.root");
    t_data->Add(tmp_str);

    /*TChain* t_data2 = new TChain("t1");
    char tmp_str2[200];

    sprintf(tmp_str2,"./digi_sim_gen_3M.root");
    t_data2->Add(tmp_str2);
*/
    TCanvas* c01 = new TCanvas("c01", "c01", 600, 600);
    // t_data->Draw("EKin/A>>hP1(100,0,5000)","PDGCode==3122","HIST");
    t_data->Draw("EKin/A>>hP1(100,0,5000)", "A==3&&Z==1&&S==-1", "HIST");
    hP1->GetXaxis()->SetTitle("E_{kin} [MeV]");
    hP1->GetYaxis()->SetTitle("Counts");
    hP1->SetLineWidth(2);
    hP1->SetLineColor(11003);
    hP1->SetFillStyle(3002);
    hP1->SetFillColor(11003);
    hP1->GetXaxis()->SetNdivisions(505);
    hP1->Fit("gaus");
    hP1->GetFunction("gaus")->SetLineColor(1);
    hP1->GetFunction("gaus")->Draw("same");

    TCanvas* c02 = new TCanvas("c02", "c02", 600, 600);
    // t_data->Draw("px/pz>>hP2(100,-0.5,0.5)","PDGCode==3122","HIST");
    t_data->Draw("px/pz>>hP2(100,-0.5,0.5)", "A==3&&Z==1&&S==-1", "HIST");
    hP2->GetXaxis()->SetTitle("px/pz");
    hP2->GetYaxis()->SetTitle("Counts");
    hP2->SetLineWidth(2);
    hP2->SetLineColor(8000);
    hP2->SetFillStyle(3002);
    hP2->SetFillColor(8000);
    hP2->GetXaxis()->SetNdivisions(505);
    hP2->GetYaxis()->SetNdivisions(505);
    hP2->Fit("gaus");
    hP2->GetFunction("gaus")->SetLineColor(1);
    hP2->GetFunction("gaus")->Draw("same");

    TCanvas* c03 = new TCanvas("c03", "c03", 600, 600);
    // t_data->Draw("py/pz>>hP3(100,-0.5,0.5)","PDGCode==3122");
    t_data->Draw("py/pz>>hP3(100,-0.5,0.5)", "A==3&&Z==1&&S==-1");
    hP3->GetXaxis()->SetTitle("py/pz");
    hP3->GetYaxis()->SetTitle("Counts");
    hP3->SetLineColor(9001);
    hP3->SetFillStyle(3002);
    hP3->SetFillColor(9001);
    hP3->GetXaxis()->SetNdivisions(505);
    hP3->GetYaxis()->SetNdivisions(505);
    hP3->Fit("gaus");
    hP3->GetFunction("gaus")->SetLineColor(1);
    hP3->GetFunction("gaus")->Draw("same");
}
