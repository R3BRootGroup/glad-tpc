/*
Author: Simone Velardita
Date:		09/02/2021
Macro for the electronics readout. All the times should be in ns
*/
#include <TCanvas.h>
#include <TFile.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TSystem.h>
#include <math.h>
//Loading bar
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void loadfunction(double &percentage)
{
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
using namespace std;
//functions for the electronics response
double polya(double x[], double p[])
	{
	double val = (pow(p[1]+1, p[1]+1) / TMath::Gamma(p[1]+1))*pow(x[0]/p[0], p[1]) * exp(-(p[1]+1) * x[0] / p[0]);
	return val;
	}
double conv(double x[], double p[])
{
	double val=0.;
	for(int i=0; i<(p[0]-2)/2; i++)
	{
		if(!(x[0]<p[2*i+2+1] || x[0]>1000000.)) 
			val += p[2*i+2] * 22.68113723 * exp(-3.*(x[0]-p[2*i+2+1])/p[1]) * sin((x[0]-p[2*i+2+1])/p[1]) * pow((x[0]-p[2*i+2+1])/p[1], 3);
	}
	return(val);
}
//TODO The fit is not working right now
double conv_fit(double x[], double p[])
{
	double val=0.;
	if(!(x[0]<p[1] || x[0]>1000000.)) val += p[0] * 22.68113723 * exp(-3.*(x[0]-p[1])/p[2]) * sin((x[0]-p[1])/p[2]) * pow((x[0]-p[1])/p[2], 3) + p[3];
	else val += p[3];
	return(val);
}

void reader(const char* inputSimFile)
{
    TStopwatch timer;
    timer.Start();
    // SETUP-> For the moment is optimized for the Prototype only, to be generalized in the future
    string geoTag="Prototype";
    R3BGTPCSetup* setup = new R3BGTPCSetup(geoTag, 0);
    Double_t fHalfSizeTPC_X = setup->GetActiveRegionx() / 2.;   // X (row) 		[cm]
    Double_t fHalfSizeTPC_Y = setup->GetActiveRegiony() / 2.;   // Y (time) 	[cm]
    Double_t fHalfSizeTPC_Z = setup->GetActiveRegionz() / 2.;   // Z (column) [cm]
    Double_t fSizeOfVirtualPad = setup->GetPadSize(); 					// 1: pads of 1cm^2 , 5: pads of 4mm^2
    Double_t fDriftVelocity=setup->GetDriftVelocity();					// [cm/ns]
		Bool_t kPrototype=kTRUE;
		delete setup;
		R3BGTPCSetup* electronics = new R3BGTPCSetup("Electronic",0);
		//electronics value
		Double_t shapingtime= electronics->GetShapingTime();					//[ns]
		Double_t TimeBinSize= electronics->GetTimeBinSize();					//[ns]
		Double_t NoiseRMS=    electronics->GetNoiseRMS();							//[n.e-]
		Double_t thr =       (electronics->GetThreshold())*NoiseRMS;	//[n.e-]
		Double_t Gain=        electronics->GetGain();
		Double_t Theta=       electronics->GetTheta();
		Int_t NPads=12844;	 //total n of padID 5632 TODO [the loop should be optimized]
		delete electronics;	
    //root style
    gROOT->SetStyle("Default");
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(11);//only the n. of entries in the plot
    gStyle->SetOptFit(0);

    cout << "Reading input simulation file " << inputSimFile << endl;
    // HERE THE HISTOGRAMS ARE DEFINED...
    UInt_t histoBins = 2 * fHalfSizeTPC_X * fSizeOfVirtualPad;
    UInt_t histoBins2 = 2 * fHalfSizeTPC_Z * fSizeOfVirtualPad;
    const int max_time= (int) ((2*fHalfSizeTPC_Y)/ fDriftVelocity + 6*shapingtime); 	

	 LOG(INFO) << " Detector:\033[1;31m " << geoTag << "!!!!!\033[0m" << endl    
   		  << "Values taken for the visualization" << endl
      	<< "bins in X: " << histoBins << endl
      	<< "bins in Z: " << histoBins2 << endl
      	<< "X size (half-length of box): " << fHalfSizeTPC_X << endl
      	<< "Y size (half-length of box (drift)): " << fHalfSizeTPC_Y << endl
      	<< "Z size (half-length of box): " << fHalfSizeTPC_Z << endl;
	 LOG(INFO) << "\033[1;32m Electronic parameters\033[0m:"<< endl
      	<< "Shaping time: " << shapingtime <<" [ns]"<< endl
      	<< "Time bin size: " << TimeBinSize<<" [ns]" << endl
      	<< "Max Drift time: "<<max_time<<" [ns]"<<endl
      	<< "NoiseRMS: " << NoiseRMS << endl
      	<< "Threshold: " << thr << endl
      	<< "Average Gain for Polya: " << Gain << endl
      	<< "Theta parameter for Polya: "<<Theta<<endl;
		//The th2 definition
		TH2D* Hitmap=0;
    TH2D* htrackInPads = 0;
    TH2D* hdriftTimeInPads = 0;
    TH2D* hdepth1InPads = 0;
    TH2D* hdepth2InPads = 0;
    
    htrackInPads = new TH2D("htrackInPads",
                            "All tracks in the XZ Pads Plane",
                            histoBins2,
                            0,
                            2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                            histoBins,
                            0,
                            2 * fHalfSizeTPC_X * fSizeOfVirtualPad); // in [pad number]
    htrackInPads->SetYTitle("X [pad number]");
    htrackInPads->SetXTitle("Z [pad number]");

    hdriftTimeInPads = new TH2D("hdriftTimeInPads",
                                "All tracks in the XZ Pads Plane with drift time",
                            histoBins2,
                            0,
                            2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                            histoBins,
                            0,
                            2 * fHalfSizeTPC_X * fSizeOfVirtualPad); // in [pad number]
    hdriftTimeInPads->SetYTitle("X [pad number]");
    hdriftTimeInPads->SetXTitle("Z [pad number]");

    hdepth1InPads = new TH2D("hdepth1InPads",
                             "track In the Drift-Z Pads Plane",
                             max_time/10,
                             0,
                             max_time, 
                             histoBins2,
                             0,
                             2 * fHalfSizeTPC_Z * fSizeOfVirtualPad);
    hdepth1InPads->SetYTitle("Z [pad number]");
    hdepth1InPads->SetXTitle("(drift) time [us]");

    hdepth2InPads = new TH2D("hdepth2InPads",
                             "track In the Drift-X Pads Plane",
                             max_time/10,
                             0,
                             max_time, 
                             histoBins,
                             0,
                             2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
    hdepth2InPads->SetYTitle("X [pad number]");
    hdepth2InPads->SetXTitle("(drift) time [us]");
    Hitmap = new TH2D("Hitmap",
                            "Hitmap XZ Pads Plane",
                            histoBins2,
                            0,
                            2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                            histoBins,
                            0,
                            2 * fHalfSizeTPC_X * fSizeOfVirtualPad); // in [pad number]
    Hitmap->SetYTitle("X [pad number]");
    Hitmap->SetXTitle("Z [pad number]");
    Hitmap->SetTitleOffset(2.,"XYZ");
		//input
    TFile* simFile = TFile::Open(inputSimFile);
    TTree* TEvt = (TTree*)simFile->Get("evt");
    Int_t nevents = TEvt->GetEntries();
    //output
		TFile *fout=new TFile("ele_output.root","RECREATE");
	  TTree *tout = new TTree("tout","Realistic TPC events");
	  std::vector<double> x_Pad,z_Pad, t_Pad, q_Pad;	
	  double Et_Pad;
	  std::vector<int> pad, eventID;
    std::vector<double> time;
    TH1D *h_conv;
		TF1 *f_conv, *fit;
		int npads=0;
	  std::vector<int> n_Pads;
	  tout->Branch("x_Pad",&x_Pad);
	  tout->Branch("eventID",&eventID);
	  tout->Branch("pad",&pad);
	  tout->Branch("time",&time);
	  tout->Branch("z_Pad",&z_Pad);
	  tout->Branch("t_Pad",&t_Pad);
	  tout->Branch("q_Pad",&q_Pad);
	  tout->Branch("Et_Pad",&Et_Pad);
	  tout->Branch("n_Pads",&n_Pads);
    cout << "nevents=" << nevents << endl;

    // GTPCProjPoints
    TClonesArray* gtpcProjPointCA;
    R3BGTPCProjPoint* ppoint = new R3BGTPCProjPoint;
    gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
    TBranch* branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
    branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);
		//Electronics response
    Int_t padsPerEvent = 0;
    Int_t nb = 0;
    bool PadTouch[20000];				//!<Touched pads
		const int samples = (int) (((2*fHalfSizeTPC_Y)/ fDriftVelocity + 6*shapingtime)/TimeBinSize);
		for(int k=0;k<NPads;k++)
		{
		PadTouch[k]=false;
		}
		//Loop over the number of events
    for (Int_t l = 0; l < nevents; l++)
    {
      double percentage=l/(double)(nevents*1.);
      loadfunction(percentage);
		  gtpcProjPointCA->Clear();
		  nb+= TEvt->GetEvent(l);
		  padsPerEvent = gtpcProjPointCA->GetEntries();
		  //for each event I create the vector that contains all the info about padID(x,z) and projtime(t)
		  for (Int_t h = 0; h < padsPerEvent; h++)
		  {	
		  ppoint = (R3BGTPCProjPoint*)gtpcProjPointCA->At(h);  
		  Int_t padID=ppoint->GetVirtualPadID();
		  PadTouch[padID] = true;
		  pad.push_back(padID);  
		  Double_t tPad = ((TH1S*)(ppoint->GetTimeDistribution()))->GetMean();
		  time.push_back(tPad);
		  } 
			//NOISE_____________________________________________________________________________________________________________
			//Polya equation MM amplification
			TF1 *fonc_gain = new TF1("fonc_gain", polya, 0., 10000., 2);
			fonc_gain->FixParameter(0, Gain);
			fonc_gain->FixParameter(1, Theta);
			double noise[max_time];
			double tab_noise[max_time];
			npads=0;
			//Loop over all the pads that are hit during the event ->padID is considered
			for(int i=0; i<=NPads; i++)
			{
				//NOISE
				for(int ii=0; ii<max_time; ii++)
				{
					noise[ii] = gRandom->Gaus(0., NoiseRMS);
				}
				//Sampling the noise
				for(int ii=0; ii<samples; ii++)
				{
					tab_noise[ii] = noise[(int) (ii*TimeBinSize)];
				}
				double max_noise = tab_noise[TMath::LocMax(samples, tab_noise)];
				double t_max_noise = TMath::LocMax(samples, tab_noise)*TimeBinSize;
				//________________________________________________________________________________________________________________
				char s[10];
				sprintf(s, "%d", i);
				int nb_param=0;
				// nb_param-> for each e- that reachs the pad will increment by 2 the number of parameters:
				// 1 parameter for the charge and 1 for the time
				for(unsigned int j=0; j<pad.size(); j++)
				{
					if(pad[j] == i) nb_param+=2;
				}
				//p2:[2*n. of e-, shapingtime, (charge e-)#1, (time e-)#1, (charge e-)#2, ..., (time e-)#n]
				double p2[nb_param+2];
				p2[0] = nb_param+2;
				p2[1] = shapingtime;
				//amplification of the signal due to MicroMegas layer
				int ii=0;
				int kk=0;
				double telec[1000000];
				Int_t xPad, zPad;
				//Looping over the total number of pad hit
				for(unsigned int j=0; j<pad.size(); j++)
				{
					if(pad[j] == i)
					{
						//Gain Polya distribution
						double val_gain = fonc_gain->GetRandom();
						double charge = val_gain;
						p2[2*ii+2+1] = time[j]*1000;//the time should be in nanosec
						p2[2*ii+2] = charge;
						//saving the times to do barycentre
						telec[kk] = time[j]*1000;
						ii++;
						kk++;
						//Calculating the padxz
						xPad = pad[j] % (Int_t)(100);
				  	zPad = (pad[j] - xPad) / (100); 
					}
				}
				// temps barycentre
				double t0 = 0.;
				for(int ii=0; ii<kk; ii++)
				{
					t0 = t0 + telec[ii];
				}
				//average time of all the e- when they arrive to the pad
				t0 = t0 / kk;

				//Filling histos_________________________________________________________________________________________________
				h_conv = new TH1D("After shaping", (string(s) + " after shaping").c_str(), samples, 0., max_time);
				f_conv = new TF1("F_conv", conv, 0.,max_time, int(p2[0]));
				
				//N(t)->Signal
				for(int ii=0; ii<p2[0]; ii++)
				{
					f_conv->FixParameter(ii, p2[ii]);
				}

				//N(t)+Noise sampled
				if(PadTouch[i])
				{ 		
					for(int jj=0; jj<samples; jj++)
					{
						h_conv->Fill(jj*TimeBinSize,(f_conv->Eval(jj*TimeBinSize)) + noise[(int) (jj*TimeBinSize)]);
					}
				}
				
				double t_rec=0.;
				double max,time0; 
				Double_t integral;
				if(PadTouch[i])
				{ 		
					max = h_conv->GetBinContent(h_conv->GetMaximumBin());
					time0 = h_conv->GetMaximumBin()*TimeBinSize-TimeBinSize/2.;
					
					npads++;
				}
				else
				{
					max = max_noise;
					time0 = t_max_noise;
				}
				//Fitting the signal to extract tpad and qpad TODO it's not working
				fit = new TF1("FIT", conv_fit, 0., max_time, 4);
				fit->SetParLimits(0, 0., 100000000);
				fit->SetParameter(0, max);
				fit->SetParLimits(1, time0-shapingtime*2., time0);
				fit->SetParameter(1, time0-shapingtime);
				fit->FixParameter(2, p2[1]);
				fit->SetParameter(3, 10.);
				fit->SetNpx(1000000);

				/*if(PadTouch[i])
				{ 
					h_conv->Fit(fit, "NOQ"); //Q=quiet
					t_rec = fit->GetParameter(1);
					integral = fit->GetMaximum(0., max_time);
				}		*/
				if(PadTouch[i]) 
				{
					t_rec = time0;
					integral = max;
				}
				
				//electronics readout
				if(max > thr&&PadTouch[i])
				{
				Hitmap->Fill(zPad, xPad);
				htrackInPads->Fill(zPad, xPad, integral);
				hdriftTimeInPads->Fill(zPad, xPad,t_rec);
				hdepth1InPads->Fill(t_rec,zPad,integral);
				hdepth2InPads->Fill(t_rec,xPad,integral);				
				x_Pad.push_back(xPad);
				z_Pad.push_back(zPad);
				t_Pad.push_back(t_rec);
				q_Pad.push_back(integral);
				eventID.push_back(l);
				Et_Pad+=integral;

				} 
				delete h_conv;
				delete f_conv;
				delete fit;
			}
		n_Pads.push_back(npads);
		tout->Fill();
		for(int i=0; i<NPads; i++)
		{
		PadTouch[i] = false;
		}
		time.clear();
		pad.clear();
		eventID.clear();
		x_Pad.clear();
		z_Pad.clear();
		t_Pad.clear();
		q_Pad.clear();
		n_Pads.clear();
		Et_Pad=0;
		}
	 	cout<<"\n"<<endl;
	 	TCanvas* c3 = new TCanvas("c3", "Pads in pad (XZ) plane", 0, 0, 1250, 950);
    TLatex l;
    l.SetTextAlign(12);
    l.SetTextSize(0.05);

    c3->SetFillColor(0);
    c3->Divide(2, 2);
    c3->Draw();
    TVirtualPad* c3_1 = c3->cd(1);
    c3_1->SetLogz();
    htrackInPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad,
                "Color code: induced charge");

    TVirtualPad* c3_2 = c3->cd(2);
    // c3_2->SetLogz();
    hdriftTimeInPads->Draw("ZCOL");

    TLatex l2;
    l2.SetTextAlign(12);
    l2.SetTextSize(0.05);
    l2.DrawLatex(
        0.2 * fHalfSizeTPC_Z * fSizeOfVirtualPad, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: drift time");

    TVirtualPad* c3_3 = c3->cd(3);
    c3_3->SetLogz();
    hdepth1InPads->Draw("ZCOL");
    //dumm->Draw();

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.3*max_time, 2.04 * fHalfSizeTPC_Z * fSizeOfVirtualPad, "Color code: induced charge");

    TVirtualPad* c3_4 = c3->cd(4);
    c3_4->SetLogz();
    hdepth2InPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.3*max_time, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: induced charge");

    gPad->Modified();
    gPad->Update();
    
    TCanvas *c2=new TCanvas("c2","Hitmap",0,0,1800,1500);
    c2->SetFillColor(0);
    c2->Divide(1,2);
    c2->cd(1);
    Hitmap->Draw("COLZ");
    c2->cd(2);
    Hitmap->Draw("lego2");    
    c2->Draw();
    
    // OUTPUT FILE
    c3->Print("output_readout.ps(");
    c2->Print("output_readout.ps)");
   fout->Write();
	 fout->Close();
   timer.Stop();   
   cout << "Macro finished succesfully!" << endl;
   if(timer.RealTime()<60.)cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
   else cout << "Real time: " << timer.RealTime()/60. << "min, CPU time: " << timer.CpuTime()/60 << "min" << endl;
} 
