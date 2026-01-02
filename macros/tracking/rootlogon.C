
{
    gSystem->Load("libR3BGTPC");

    cout << "[INFO] simulation macros directory for glad-tpc" << endl;

    TStyle* mcStyle = new TStyle("mcStyle", "Simone's Root Styles");
    mcStyle->SetPalette(1, 0); // avoid horrible default color scheme
    // mcStyle->SetOptStat(0);
    Int_t font = 133;
    Double_t tsize = 34;
    mcStyle->SetTitleFont(font, "x");
    mcStyle->SetTitleFont(font, "y");
    mcStyle->SetTitleSize(tsize, "x");
    mcStyle->SetTitleSize(tsize, "y");
    mcStyle->SetTitleSize(tsize, "z");
    mcStyle->SetTitleFont(font, "z");
    mcStyle->SetOptTitle(0);
    mcStyle->SetOptDate(0);
    mcStyle->SetTitleOffset(1.3, "y"); // default canvas options
    mcStyle->SetTitleOffset(1., "x");
    mcStyle->SetLabelFont(font, "x");
    mcStyle->SetLabelFont(font, "y");
    mcStyle->SetLabelSize(tsize, "x");
    mcStyle->SetLabelSize(tsize, "y");
    mcStyle->SetLabelSize(tsize, "z");
    mcStyle->SetLabelFont(font, "z");
    mcStyle->SetCanvasDefW(600);
    mcStyle->SetCanvasDefH(600);
    mcStyle->SetCanvasColor(0); // canvas...
    mcStyle->SetCanvasBorderMode(0);
    mcStyle->SetCanvasBorderSize(0);
    mcStyle->SetPadBottomMargin(0.13); // margins...
    mcStyle->SetPadTopMargin(0.04);    // 0.02
    mcStyle->SetPadLeftMargin(0.18);

    mcStyle->SetPadRightMargin(0.02);
    mcStyle->SetPadGridX(0); // grids,
    mcStyle->SetPadGridY(0);
    mcStyle->SetPadTickX(1);
    mcStyle->SetPadTickY(1);
    mcStyle->SetFrameBorderMode(0);
    mcStyle->SetPaperSize(20, 24); // US letter size
    gROOT->SetStyle("mcStyle");
    cout << "Styles are Set!" << endl;
    return;
}
