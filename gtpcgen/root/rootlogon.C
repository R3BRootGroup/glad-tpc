#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TSystem.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

{

    Double_t stops[8] = { 0.0000, 0.20, 0.3500, 0.50, 0.650, 0.8, 0.9, 1.00 };
    // DeepSea
    //  Double_t red[9]   = {
    //  0./255.,  9./255., 13./255., 17./255., 24./255.,  32./255.,  27./255.,  25./255.,  29./255.};
    // Double_t green[9] = {  0./255.,  0./255.,  0./255.,  2./255., 37./255.,  74./255., 113./255., 160./255.,
    // 221./255.}; Double_t blue[9]  = { 28./255., 42./255., 59./255., 78./255., 98./255., 129./255., 154./255.,
    // 184./255., 221./255.}; Lake Double_t red[9]   = {  57./255.,  72./255.,  94./255., 117./255., 136./255.,
    // 154./255., 174./255., 192./255., 215./255.}; Double_t green[9] = {   0./255.,  33./255.,  68./255., 109./255.,
    // 140./255., 171./255., 192./255., 196./255., 209./255.}; Double_t blue[9] = { 116./255., 137./255., 173./255.,
    // 201./255., 200./255., 201./255., 203./255., 190./255., 187./255.}; Ocean Double_t red[9]   =
    // { 14./255.,  7./255.,  2./255.,  0./255.,  5./255.,  11./255.,  55./255., 131./255., 229./255.}; Double_t
    // green[9] = {105./255., 56./255., 26./255.,  1./255., 42./255.,  74./255., 131./255., 171./255., 229./255.};
    // Double_t blue[9] = { 2./255., 21./255., 35./255., 60./255., 92./255., 113./255., 160./255., 185./255.,
    // 229./255.}; AquaMarine Double_t red[9]   = { 145./255., 166./255., 167./255., 156./255., 131./255., 114./255.,
    // 101./255., 112./255., 132./255.}; Double_t green[9] = { 158./255., 178./255., 179./255., 181./255., 163./255.,
    // 154./255., 144./255., 152./255., 159./255.}; Double_t blue[9] = { 190./255., 199./255., 201./255., 192./255.,
    // 176./255., 169./255., 160./255., 166./255., 190./255.}; Custom
    Double_t red[8] = { 17. / 255., 0. / 255., 0. / 255., 0. / 255., 0. / 255., 0. / 255., /*186./255.,*/ 158. / 255.,
                        241. / 255. };
    Double_t green[8] = {
        23. / 255., 25. / 255., 91. / 255., 156. / 255., 213. / 255., 253. / 255., /*249./255.,*/ 255. / 255.,
        255. / 255.
    };
    Double_t blue[8] = {
        133. / 255., 255. / 255., 255. / 255., 253. / 255., 253. / 255., 247. / 255., /*244./255.,*/ 255. / 255.,
        255. / 255.
    };

    // Int_t NewPalette  = TColor::CreateGradientColorTable(8, stops, red, green, blue, 255);
    // gStyle->SetNumberContours(255);

    Int_t ci = 2000; // thesis dark blue
    TColor* color = new TColor(ci, 17. / 255., 23. / 255., 133. / 255.);
    Int_t cii = 3000; // thesis blue
    TColor* color2 = new TColor(cii, 0. / 255., 156. / 255., 253. / 255.);
    Int_t ciii = 4000; // thesis cyan
    TColor* color3 = new TColor(ciii, 0. / 255., 253. / 255., 247. / 255.);
    Int_t bkg = 6000; // thesis dark blue
    TColor* color5 = new TColor(bkg, 255. / 255., 255. / 255., 255. / 255.);
    Int_t bkg1 = 7000; // thesis dark blue
    TColor* color6 = new TColor(bkg1, 152. / 255., 152. / 255., 152. / 255.);
    Int_t gr = 5000; // thesis dark blue
                     // TColor *color4 = new TColor(gr,255./255.,102./255.,0./255.);
    TColor* color4 = new TColor(gr, 102. / 255., 204. / 255., 51. / 255.);
    Int_t gr9 = 8000; // thesis dark blue
    TColor* color7 = new TColor(gr9, 146. / 255., 0. / 255., 255. / 255.);

    Int_t gr91 = 9001;
    TColor* color91 = new TColor(gr91, 51. / 255., 204. / 255., 102. / 255.);
    Int_t gr92 = 9002;
    TColor* color92 = new TColor(gr92, 255. / 255., 48. / 255., 48. / 255.);
    Int_t gr93 = 9003;
    TColor* color93 = new TColor(gr93, 49. / 255., 204. / 255., 44. / 255.);
    Int_t gr94 = 9004;
    // TColor *color94 = new TColor(gr94,49./255.,204./255.,204./255.);
    TColor* color94 = new TColor(gr94, 20. / 255., 220. / 255., 245. / 255.);

    Int_t gr95 = 11001;
    TColor* color95 = new TColor(gr95, 204. / 255., 152. / 255., 255. / 255.);
    Int_t gr96 = 11002;
    TColor* color96 = new TColor(gr96, 48. / 255., 0. / 255., 102. / 255.);
    Int_t gr97 = 11003;
    TColor* color97 = new TColor(gr97, 255. / 255., 204. / 255., 0. / 255.);

    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameFillColor(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasColor(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetPadColor(0);
    // gStyle->SetStatColor(0);

    /// gStyle->SetPaperSize(20,26);
    // gStyle->SetPadTopMargin(0.1);//(0.1);
    // gStyle->SetPadRightMargin(0.15);//(0.1);//for quadratic and not 2D;; 0.15 for not
    // gStyle->SetPadBottomMargin(0.15);
    // gStyle->SetPadLeftMargin(0.15);
    gStyle->SetPadTopMargin(0.08);    //(0.05);//(0.1); //0.01
    gStyle->SetPadRightMargin(0.07);  //(0.1);//for quadratic and not 2D;; 0.15 for not // 0.001
    gStyle->SetPadBottomMargin(0.15); // 0.17
    gStyle->SetPadLeftMargin(0.15);   // 0.17
    gStyle->SetTitleXOffset(1.);
    gStyle->SetTitleYOffset(1.1);
    Int_t font = 133;
    Double_t tsize = 40;  // 50 4 plots // 44 1 plot
    Double_t tsize2 = 37; // 48 plots // 44 1 plot
    gStyle->SetTextFont(font);
    // gStyle->SetTextSize(tsize);
    gStyle->SetTextSize(30);
    gStyle->SetLabelFont(font, "x");
    gStyle->SetTitleFont(font, "x");
    gStyle->SetLabelFont(font, "y");
    gStyle->SetTitleFont(font, "y");
    gStyle->SetLabelFont(font, "z");
    gStyle->SetTitleFont(font, "z");
    gStyle->SetLabelSize(tsize2, "x");
    gStyle->SetTitleSize(tsize, "x");
    gStyle->SetLabelSize(tsize2, "y");
    gStyle->SetTitleSize(tsize, "y");
    gStyle->SetLabelSize(tsize2, "z");
    gStyle->SetTitleSize(tsize, "z");
    gStyle->SetLegendFont(font);
    gStyle->SetLegendTextSize(28); //(0.07);
                                   // gStyle->SetLegendTextSize(40); // 40 4 plots // 36 1 plot
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFillColor(0);
    gStyle->SetFillStyle(0);
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(1.2); //(1.2);
    gStyle->SetHistLineWidth(2.);
    gStyle->SetLineStyleString(2, "[12 12]");
    // gStyle->SetErrorX(0.0001);
    // gStyle->SetEndErrorSize(0.);
    // gStyle->SetOptTitle(0);
    // gStyle->SetOptStat(0);
    // gStyle->SetOptFit(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    // gStyle->SetLineScalePS(3);

    // TGaxis::SetMaxDigits(3);

    gStyle->SetMarkerColor(2000);
    gStyle->SetLineColor(2000);

    // gROOT->ForceStyle();

    return;
}
