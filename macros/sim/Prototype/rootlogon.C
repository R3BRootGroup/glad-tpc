
{
    gSystem->Load("libR3BGTPC");

    cout << "[INFO] simulation macros directory for glad-tpc" << endl;

    TStyle* mcStyle = new TStyle("mcStyle", "Simone's Root Styles");
    mcStyle->SetPalette(1, 0); // avoid horrible default color scheme
    mcStyle->SetOptStat(1111);
    mcStyle->SetOptTitle(0);
    mcStyle->SetOptDate(0);
    mcStyle->SetTitleOffset(1.2, "y"); // default canvas options
    mcStyle->SetCanvasDefW(500);
    mcStyle->SetCanvasDefH(500);
    mcStyle->SetCanvasColor(0); // canvas...
    mcStyle->SetCanvasBorderMode(0);
    mcStyle->SetCanvasBorderSize(0);
    mcStyle->SetPadBottomMargin(0.1); // margins...
    mcStyle->SetPadTopMargin(0.1);
    mcStyle->SetPadLeftMargin(0.1);
    mcStyle->SetPadRightMargin(0.1);
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
