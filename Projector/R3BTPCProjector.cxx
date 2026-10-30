/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "R3BTPCProjector.h"
#include "R3BMCTrack.h"
#include "TClonesArray.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "TF1.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"

#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoNode.h" // also provides TGeoIterator
#include "TRandom.h"

#include "TMath.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
using namespace std;

// ---------------------------------------------------------------------------
R3BTPCProjector::R3BTPCProjector()
    : FairTask("R3BTPCProjector")
    , fGTPCPoints(NULL)
    , fGTPCProjPoint(NULL)
    , fGTPCMapOutput(NULL)
    , fGTPCCalDataCA(NULL)
    , MCTrackCA(NULL)
    , fEIonization(0.)
    , fDriftVelocity(0.)
    , fTransDiff(0.)
    , fLongDiff(0.)
    , fFanoFactor(0.)
    , fTimeBinSize(0.)
    , fDriftEField(0.)
    , fDriftTimeStep(0.)
    , outputMode(0)
    , fReadoutSide(+1)
    , fActiveMinX(0.)
    , fActiveMaxX(0.)
    , fActiveMinY(0.)
    , fActiveMaxY(0.)
    , fActiveMinZ(0.)
    , fActiveMaxZ(0.)
    , fActiveSizeX(0.)
    , fActiveSizeY(0.)
    , fActiveSizeZ(0.)
    , fGeometryConfigured(kFALSE)
    , fPadRowsX(0)
    , fPadColumnsZ(0)
    , fPadSizeXcm(0.)
    , fPadSizeZcm(0.)
    , fPadGapXcm(0.)
    , fPadGapZcm(0.)
    , fPadGeometryConfigured(kFALSE)
    , fUseManualDriftParameters(kFALSE)
    , fGTPCGasPar(NULL)
    , fGTPCElecPar(NULL)
    , fTPCMap(std::make_shared<R3BGTPCMap>())
{
}

// R3BTPCProjector::~R3BTPCProjector()
// {

//     if (fGTPCPoints)
//     {
//         fGTPCPoints->Delete();
//         delete fGTPCPoints;
//     }
//     if (fGTPCCalDataCA)
//     {
//         fGTPCCalDataCA->Delete();
//         delete fGTPCCalDataCA;
//     }
//     if (MCTrackCA)
//     {
//         MCTrackCA->Delete();
//         delete MCTrackCA;
//     }
// }

R3BTPCProjector::~R3BTPCProjector()
{
    // Input branches are owned by FairRootManager.
    // fGTPCPoints = NULL;
    // MCTrackCA = NULL;

    // if (fGTPCCalDataCA)
    // {
    //     fGTPCCalDataCA->Delete();
    //     delete fGTPCCalDataCA;
    //     fGTPCCalDataCA = NULL;
    // }

    // if (fGTPCProjPoint)
    // {
    //     fGTPCProjPoint->Delete();
    //     delete fGTPCProjPoint;
    //     fGTPCProjPoint = NULL;
    // }

    fGTPCPoints = nullptr;
    fGTPCProjPoint = nullptr;
    fGTPCMapOutput = nullptr;
    fGTPCCalDataCA = nullptr;
    MCTrackCA = nullptr;

    fGTPCGasPar = nullptr;
    fGTPCElecPar = nullptr;
}

void R3BTPCProjector::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    if (!run)
    {
        LOG(fatal) << "R3BTPCProjector::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(fatal) << "R3BTPCProjector::SetParContainers: No runtime database";
        return;
    }

    // fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    // if (!fGTPCGeoPar)
    // {
    //     LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGeoPar";
    //     return;
    // }

    fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!fGTPCGasPar)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGasPar";
        return;
    }
    fGTPCElecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!fGTPCElecPar)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCElecPar";
        return;
    }
}

void R3BTPCProjector::SetParameter()
{
    if (!fGTPCGasPar || !fGTPCElecPar)
    {
        LOG(error) << "R3BTPCProjector::SetParameter: GTPCGasPar or GTPCElePar is missing";
        return;
    }

    // ---------------------------------------------------------
    // Gas / transport parameters
    // ---------------------------------------------------------

    if (!fUseManualDriftParameters)
    {

        fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]
        fDriftVelocity = fGTPCGasPar->GetDriftVelocity(); // [cm/ns]
        fTransDiff = fGTPCGasPar->GetTransDiff();         // [cm^2/ns]
        fLongDiff = fGTPCGasPar->GetLongDiff();           // [cm^2/ns]
        fFanoFactor = fGTPCGasPar->GetFanoFactor();
    }
    else
    {
        LOG(info) << "R3BTPCProjector: using manual drift parameters";
    }
    // ---------------------------------------------------------
    // Electronics quantities needed by projector
    // ---------------------------------------------------------
    fDriftEField = fGTPCElecPar->GetDriftEField();     // drift E field in V/m
    fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // time step for drift

    fTimeBinSize = fGTPCElecPar->GetTimeBinSize(); // [ns]

    // ---------- Old Implimentation ------------------- //

    // fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]-> typical value for a
    //                                                   // gas detector tens of eV
    // fDriftVelocity = fGTPCGasPar->GetDriftVelocity(); // [cm/ns]-> Minos TPC with a Efield=152 V/cm
    // fTransDiff = fGTPCGasPar->GetTransDiff();         // [cm^2/ns]?
    // fLongDiff = fGTPCGasPar->GetLongDiff();           // [cm^2/ns]?
    // fFanoFactor = fGTPCGasPar->GetFanoFactor();
    // fHalfSizeTPC_X = fGTPCGeoPar->GetActiveRegionx() / 2.;
    // fHalfSizeTPC_Y = fGTPCGeoPar->GetActiveRegiony() / 2.;
    // fHalfSizeTPC_Z = fGTPCGeoPar->GetActiveRegionz() / 2.;
    // fDetectorType = fGTPCGeoPar->GetDetectorType();
    // fOffsetX = fGTPCGeoPar->GetGladOffsetX(); // X offset [cm]
    // fOffsetZ = fGTPCGeoPar->GetGladOffsetZ(); // Z offset [cm]

    // fDriftEField = fGTPCElecPar->GetDriftEField();     // drift E field in V/m
    // fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // time step for drift

    // fGTPCElecPar->GetTimeBinSize(); // [ns]
    // fGTPCElecPar->GetTimeBinSize(); // drift params calculation

    std::cout << "\n========== PROJECTOR PARAMETERS ==========\n"
              << "EIonization   = " << fEIonization << " GeV\n"
              << "DriftVelocity = " << fDriftVelocity << " cm/ns\n"
              << "TransDiff     = " << fTransDiff << " cm2/ns\n"
              << "LongDiff      = " << fLongDiff << " cm2/ns\n"
              << "FanoFactor    = " << fFanoFactor << "\n"
              << "DriftEField   = " << fDriftEField << " V/cm\n"
              << "==========================================\n";
}

void R3BTPCProjector::SetReadoutSide(Int_t side)
{
    if (side != +1 && side != -1)
    {
        LOG(error) << "R3BTPCProjector::SetReadoutSide: side must be +1 or -1, got " << side;
        return;
    }
    fReadoutSide = side;
}

void R3BTPCProjector::SetPadGeometry(Int_t rowsX,
                                     Int_t columnsZ,
                                     Double_t padSizeXcm,
                                     Double_t padSizeZcm,
                                     Double_t gapXcm,
                                     Double_t gapZcm)
{
    if (rowsX <= 0 || columnsZ <= 0 || !std::isfinite(padSizeXcm) || padSizeXcm <= 0. || !std::isfinite(padSizeZcm) ||
        padSizeZcm <= 0. || !std::isfinite(gapXcm) || gapXcm < 0. || !std::isfinite(gapZcm) || gapZcm < 0.)
    {
        LOG(error) << "R3BTPCProjector::SetPadGeometry: invalid pad geometry";
        fPadGeometryConfigured = kFALSE;
        return;
    }

    fPadRowsX = rowsX;
    fPadColumnsZ = columnsZ;
    fPadSizeXcm = padSizeXcm;
    fPadSizeZcm = padSizeZcm;
    fPadGapXcm = gapXcm;
    fPadGapZcm = gapZcm;
    fPadGeometryConfigured = kTRUE;
}

void R3BTPCProjector::SetDriftParameters(Double_t Eion,
                                         Double_t driftVelo,
                                         Double_t TransDiff,
                                         Double_t LongDiff,
                                         Double_t fanoFactor)
{
    fEIonization = Eion;        // [GeV]
    fDriftVelocity = driftVelo; // [cm/ns]
    fTransDiff = TransDiff;     // [cm^(-1/2)]
    fLongDiff = LongDiff;       // [cm^(-1/2)]
    fFanoFactor = fanoFactor;
    fUseManualDriftParameters =
        kTRUE; // True only when user set the drfir parameters in the macro otherwise defaults from ascii file.
}

Bool_t R3BTPCProjector::ConfigureFromGeometry()
{
    fGeometryConfigured = kFALSE;

    if (!gGeoManager || !gGeoManager->GetTopVolume())
    {
        LOG(error) << "R3BTPCProjector::ConfigureFromGeometry: no TGeo geometry loaded";
        return kFALSE;
    }

    TGeoIterator it(gGeoManager->GetTopVolume());
    TGeoNode* node = nullptr;
    TString activePath;

    while ((node = it()))
    {
        if (node->GetVolume() && TString(node->GetVolume()->GetName()) == "Active_region")
        {
            it.GetPath(activePath);
            break;
        }
    }

    if (!node)
    {
        LOG(error) << "R3BTPCProjector::ConfigureFromGeometry: Active_region not found";
        return kFALSE;
    }

    auto* box = dynamic_cast<TGeoBBox*>(node->GetVolume()->GetShape());
    if (!box)
    {
        LOG(error) << "R3BTPCProjector::ConfigureFromGeometry: Active_region is not found and not a TGeoBBox";
        return kFALSE;
    }

    if (!gGeoManager->cd(activePath.Data()))
    {
        LOG(error) << "R3BTPCProjector::ConfigureFromGeometry: cannot go to " << activePath;
        return kFALSE;
    }

    fActiveLocalToMaster = *gGeoManager->GetCurrentMatrix();

    const Double_t* origin = box->GetOrigin();
    fActiveMinX = origin[0] - box->GetDX();
    fActiveMaxX = origin[0] + box->GetDX();
    fActiveMinY = origin[1] - box->GetDY();
    fActiveMaxY = origin[1] + box->GetDY();
    fActiveMinZ = origin[2] - box->GetDZ();
    fActiveMaxZ = origin[2] + box->GetDZ();

    fActiveSizeX = fActiveMaxX - fActiveMinX;
    fActiveSizeY = fActiveMaxY - fActiveMinY;
    fActiveSizeZ = fActiveMaxZ - fActiveMinZ;

    if (fActiveSizeX <= 0. || fActiveSizeY <= 0. || fActiveSizeZ <= 0.)
    {
        LOG(error) << "R3BTPCProjector::ConfigureFromGeometry: invalid Active_region dimensions";
        return kFALSE;
    }

    fGeometryConfigured = kTRUE;
    LOG(info) << "R3BTPCProjector geometry: ";
    LOG(info) << "R3BTPCProjector: Active_region path = " << activePath;
    LOG(info) << "R3BTPCProjector: local active X/Y/Z = " << fActiveMinX << " .. " << fActiveMaxX << " ... "
              << fActiveMinY << " .. " << fActiveMaxY << " ... " << fActiveMinZ << " .. " << fActiveMaxZ << " cm";
    LOG(info) << "  local size X/Y/Z [cm]  = " << fActiveSizeX << " / " << fActiveSizeY << " / " << fActiveSizeZ;
    LOG(info) << "R3BTPCProjector: local readout side = " << (fReadoutSide > 0 ? "+Y" : "-Y")
              << ", plane Y = " << (fReadoutSide > 0 ? fActiveMaxY : fActiveMinY) << " cm";

    return kTRUE;
}

InitStatus R3BTPCProjector::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(fatal) << "R3BTPCProjector::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    // if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    // {
    //     LOG(fatal) << "R3BTPCProjector::Init No GTPCPoint!";
    //     return kFATAL;
    // }
    // fGTPCPoints = (TClonesArray*)ioman->GetObject("GTPCPoint");
    // // Input: TClonesArray of R3BMCTrack
    // if ((TClonesArray*)ioman->GetObject("MCTrack") == nullptr)
    // {
    //     LOG(fatal) << "R3BMCTrack::Init No MCTrack!";
    //     return kFATAL;
    // }

    // MCTrackCA = (TClonesArray*)ioman->GetObject("MCTrack");

    fGTPCPoints = (TClonesArray*)ioman->GetObject("GTPCPoint");
    if (!fGTPCPoints)
    {
        LOG(error) << "R3BTPCProjector::Init: No GTPCPoint branch";
        return kFATAL;
    }

    MCTrackCA = (TClonesArray*)ioman->GetObject("MCTrack");
    if (!MCTrackCA)
    {
        LOG(error) << "R3BTPCProjector::Init: No MCTrack branch";
        return kFATAL;
    }

    // --------------- to do make it better ---------- //
    if (!fGTPCGasPar || !fGTPCElecPar)
        SetParContainers();

    if (!fGTPCGasPar || !fGTPCElecPar)
        return kFATAL;

    SetParameter();

    if (fEIonization <= 0. || fDriftVelocity <= 0. || fTransDiff <= 0. || fLongDiff <= 0. || fFanoFactor <= 0.)
    {
        LOG(error) << "R3BTPCProjector::Init: EIonization must be set properly before running ";
        return kFATAL;
    }

    if (!ConfigureFromGeometry())
        return kFATAL;

    if (!fPadGeometryConfigured)
    {
        LOG(error) << "R3BTPCProjector::Init: SetPadGeometry(...) must be called "
                   << "before run->Init()";
        return kFATAL;
    }

    if (!fTPCMap->ConfigurePadGeometry(
            fActiveSizeX, fActiveSizeZ, fPadRowsX, fPadColumnsZ, fPadSizeXcm, fPadSizeZcm, fPadGapXcm, fPadGapZcm))
    {
        LOG(error) << "R3BTPCProjector::Init: pad matrix does not fit Active_region";
        return kFATAL;
    }

    fTPCMap->GeneratePadPlane();
    // fPadPlane = fTPCMap->GetPadPlane();
    if (!fTPCMap->GetPadPlane())
    {
        LOG(error) << "R3BTPCProjector::Init: failed to create virtual pad plane";
        return kFATAL;
    }

    if (outputMode == 0)
    { // Output: TClonesArray of R3BGTPCCalData
        // ioman->Register("GTPCCalData", GetName(), fGTPCCalDataCA, kTRUE);

        fGTPCCalDataCA = new TClonesArray("R3BGTPCCalData");
        ioman->Register("GTPCCalData", GetName(), fGTPCCalDataCA, kTRUE);
    }
    else if (outputMode == 1)
    { // Output: TClonesArray of R3BGTPCProjPoint
        // ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPoint, kTRUE);

        fGTPCProjPoint = new TClonesArray("R3BGTPCProjPoint");
        ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPoint, kTRUE);

        // Persist the SAME configured R3BGTPCMap used by the projector.
        // This avoids redefining rows/columns/pad size/gaps in the digitizer.
        fGTPCMapOutput = new TClonesArray("R3BGTPCMap");
        ioman->Register("GTPCMap", GetName(), fGTPCMapOutput, kTRUE);
    }
    else
    {
        LOG(error) << "R3BTPCProjector::Init: Invalid outputMode = " << outputMode;
        return kFATAL;
    }

    // LOG(info) << "R3BTPCProjector: configured " << fTPCMap->GetNumberOfPads() << " pads ("
    //           << fPadRowsX << " x " << fPadColumnsZ << ")";

    // Pad plane generation
    // fTPCMap->GeneratePadPlane();
    // fPadPlane = fTPCMap->GetPadPlane();

    // if (fPadPlane == NULL)
    // {
    //     std::cout << " R3BTPCProjector::Init() error! - Could not retrieve pad "
    //                  "plane. Exiting..."
    //               << "\n";
    //     return kERROR;
    // }

    LOG(info) << "R3BTPCProjector pad setup:";
    LOG(info) << "  rows X / columns Z     = " << fTPCMap->GetNumberOfRowsX() << " / "
              << fTPCMap->GetNumberOfColumnsZ();
    LOG(info) << "  number of pads         = " << fTPCMap->GetNumberOfPads();
    LOG(info) << "  pad X/Z [cm]           = " << fTPCMap->GetPadSizeXcm() << " / " << fTPCMap->GetPadSizeZcm();
    LOG(info) << "  gap X/Z [cm]           = " << fTPCMap->GetGapXcm() << " / " << fTPCMap->GetGapZcm();
    LOG(info) << "  physical span X/Z [cm] = " << fTPCMap->GetGridSpanXcm() << " / " << fTPCMap->GetGridSpanZcm();

    return kSUCCESS;
}

// InitStatus R3BTPCProjector::ReInit()
// {
//     SetParContainers();
//     return kSUCCESS;
// }

InitStatus R3BTPCProjector::ReInit()
{
    SetParContainers();

    if (!fGTPCGasPar || !fGTPCElecPar)
        return kFATAL;

    SetParameter();

    if (!ConfigureFromGeometry())
        return kFATAL;

    return kSUCCESS;
}

void R3BTPCProjector::Exec(Option_t*)
{

    if (outputMode == 0)
    {
        // fGTPCCalDataCA->Clear("C");
        fGTPCCalDataCA->Delete();
    }
    else if (outputMode == 1)
    {
        // fGTPCProjPoint->Clear("C");
        fGTPCProjPoint->Delete();

        if (fGTPCMapOutput)
        {
            fGTPCMapOutput->Delete();

            auto* mapOut = new ((*fGTPCMapOutput)[0]) R3BGTPCMap();

            const Bool_t mapOK = mapOut->ConfigurePadGeometry(fTPCMap->GetSizeXcm(),
                                                              fTPCMap->GetSizeZcm(),
                                                              fTPCMap->GetNumberOfRowsX(),
                                                              fTPCMap->GetNumberOfColumnsZ(),
                                                              fTPCMap->GetPadSizeXcm(),
                                                              fTPCMap->GetPadSizeZcm(),
                                                              fTPCMap->GetGapXcm(),
                                                              fTPCMap->GetGapZcm());

            if (!mapOK)
            {
                LOG(fatal) << "R3BTPCProjector::Exec: failed to persist R3BGTPCMap";
                return;
            }
        }
    }

    // Int_t nPoints = fGTPCPoints->GetEntries();
    Int_t nPoints = fGTPCPoints ? fGTPCPoints->GetEntriesFast() : 0;
    LOG(info) << "R3BTPCProjector: processing " << nPoints << " points";
    if (nPoints < 2)
    {
        LOG(info) << "Not enough hits for digitization! (<2)";
        return;
    }

    // R3BGTPCPoint* aPoint;
    Int_t presentTrackID = -10; // control of the point trackID

    Bool_t readyToProject = kFALSE;

    Double_t xPre, yPre, zPre;
    Double_t xPost, yPost, zPost;
    Double_t projX, projZ, projTime;
    Double_t energyDep = 0.;
    Double_t timeBeforeDrift = 0.;
    // Bool_t readyToProject = kFALSE;
    Bool_t padFound = kFALSE;
    Double_t electrons = 0.0;
    Int_t flucElectrons = 0;
    Double_t sigmaElectrons = 0.0;
    Int_t generatedElectrons = 0;
    Double_t stepX, stepY, stepZ;
    Double_t driftDistance, driftTime;
    Double_t sigmaLongAtPadPlane;
    Double_t sigmaTransvAtPadPlane;
    Int_t evtID = 0;

    Int_t PDGCode = 0, MotherId = 0;

    Double_t Vertex_x0 = 0, Vertex_y0 = 0, Vertex_z0 = 0, Vertex_px0 = 0, Vertex_py0 = 0, Vertex_pz0 = 0;

    for (Int_t i = 0; i < nPoints; i++)
    {

        // aPoint = (R3BGTPCPoint*)fGTPCPoints->At(i);
        R3BGTPCPoint* aPoint = (R3BGTPCPoint*)fGTPCPoints->At(i);

        evtID = aPoint->GetEventID();

        if (aPoint->GetTrackStatus() == 11000 || aPoint->GetTrackStatus() == 10010010 ||
            aPoint->GetTrackStatus() == 10010000 || aPoint->GetTrackStatus() == 10011000)
        {
            // entering the gas volume or new track inside the gas (is 10010010 or
            // 10010000??)
            presentTrackID = aPoint->GetTrackID();

            xPre = aPoint->GetX();
            yPre = aPoint->GetY();
            zPre = aPoint->GetZ();

            // std::cout<<" xPre "<<xPre<<" - yPre "<<yPre<<" - zPre "<<zPre<<"\n";
            R3BMCTrack* Track = (R3BMCTrack*)MCTrackCA->At(presentTrackID);

            if (!Track)
                continue;

            PDGCode = Track->GetPdgCode();
            MotherId = Track->GetMotherId();

            Vertex_x0 = Track->GetStartX();
            Vertex_y0 = Track->GetStartY();
            Vertex_z0 = Track->GetStartZ();

            Vertex_px0 = Track->GetPx();
            Vertex_py0 = Track->GetPy();
            Vertex_pz0 = Track->GetPz();

            readyToProject = kTRUE;
            continue; // no energy deposited in this point, just taking in entrance
                      // coordinates
        }
        else
        { // any other case
            if (presentTrackID != aPoint->GetTrackID())
            {
                cout << aPoint->GetTrackStatus() << "  " << endl;
                LOG(fatal) << "R3BTPCProjector::Exec: Problem 2 in point logic";
                break;
            }
            if (readyToProject != kTRUE)
            {
                LOG(fatal) << "R3BTPCProjector::Exec: Problem 3 in point logic";
                break;
            }
            if (aPoint->GetTrackStatus() == 10100 || aPoint->GetTrackStatus() == 1000000)
            { // exiting the gas volume or disappearing
                readyToProject = kFALSE;
            }

            xPost = aPoint->GetX();
            yPost = aPoint->GetY();
            zPost = aPoint->GetZ();
            energyDep = aPoint->GetEnergyLoss();
            timeBeforeDrift = aPoint->GetTime();
        }

        // ---- Master -> local Active_region coordinates
        Double_t preMaster[3] = { xPre, yPre, zPre };
        Double_t postMaster[3] = { xPost, yPost, zPost };
        Double_t preLocal[3] = { 0., 0., 0. };
        Double_t postLocal[3] = { 0., 0., 0. };

        fActiveLocalToMaster.MasterToLocal(preMaster, preLocal);
        fActiveLocalToMaster.MasterToLocal(postMaster, postLocal);

        // 1. Calculate average primary electrons produced by the ionization
        electrons = energyDep / fEIonization;

        // electron number fluctuates as the square root of the
        // Fano factor times the number of electrons

        // Line below is misleading old implimentation, as a Fanofactor is F = sigma^2 / mu (mean so electrons)
        // Since Fano factor is inserted as a input it is actually calculating the variance (sigma)
        // flucElectrons = pow(fFanoFactor * electrons, 0.5);

        // Therefore it should actually be
        // 2. Correct Equation: This calculates SIGMA (standard deviation), not the count.
        sigmaElectrons = sqrt(fFanoFactor * electrons);

        // generatedElectrons = gRandom->Gaus(electrons, flucElectrons); // generated electrons

        // 3. To protect againts negative or nonphysical gausssian fluc, so rounded to nearest
        generatedElectrons = std::max(0, static_cast<Int_t>(std::lround(gRandom->Gaus(electrons, sigmaElectrons))));

        if (generatedElectrons <= 0)
        {
            xPre = xPost;
            yPre = yPost;
            zPre = zPost;
            continue;
        }

        // step in each direction for an homogeneous electron creation position
        // along the track
        // stepX = (xPost - xPre) / generatedElectrons;
        // stepY = (yPost - yPre) / generatedElectrons;
        // stepZ = (zPost - zPre) / generatedElectrons;

        // stepX = (postLocal[0] - preLocal[0]) / generatedElectrons;
        // stepY = (postLocal[1] - preLocal[1]) / generatedElectrons;
        // stepZ = (postLocal[2] - preLocal[2]) / generatedElectrons;

        constexpr Double_t kTolerance = 1.e-6;
        // taken a mean driftDistance for the calculation of the sigmaLong and
        // sigmaTrans improve (make the calculation individual for electron) if
        // needed, but probably slower
        // Double_t yApprox = (yPost + yPre) / 2;
        // driftDistance = yApprox + fHalfSizeTPC_Y;
        // // cout<<"DriftDistance="<<driftDistance<<"	yApprox="<<yApprox<<endl;

        Double_t yMid = 0.5 * (preLocal[1] + postLocal[1]);
        Double_t driftMid = (fReadoutSide > 0) ? (fActiveMaxY - yMid) : (yMid - fActiveMinY);
        driftMid = std::clamp(driftMid, 0.0, fActiveSizeY);

        // sigmaLongAtPadPlane = sqrt(driftDistance * 2 * fLongDiff / fDriftVelocity);
        // sigmaTransvAtPadPlane = sqrt(driftDistance * 2 * fTransDiff / fDriftVelocity);
        // Double_t sigmaTime = sigmaLongAtPadPlane / fDriftVelocity; // [ns]

        // One common diffusion width for all electrons created in this step.
        sigmaLongAtPadPlane = std::sqrt(2. * fLongDiff * driftMid / fDriftVelocity);

        sigmaTransvAtPadPlane = std::sqrt(2. * fTransDiff * driftMid / fDriftVelocity);

        Double_t sigmaTime = sigmaLongAtPadPlane / fDriftVelocity;

        for (Int_t ele = 0; ele < generatedElectrons; ele++) // following each electrons from production to pad
        {
            // ------------------------------------------------------------
            // 1. Random production position along this Geant4 step
            // ------------------------------------------------------------
            Double_t fraction = gRandom->Uniform(0., 1.);

            Double_t electronX = preLocal[0] + fraction * (postLocal[0] - preLocal[0]);

            Double_t electronY = preLocal[1] + fraction * (postLocal[1] - preLocal[1]);

            Double_t electronZ = preLocal[2] + fraction * (postLocal[2] - preLocal[2]);

            // Double_t electronX = preLocal[0] + fraction * stepX ;

            // Double_t electronY = preLocal[1] + fraction * stepY;

            // Double_t electronZ = preLocal[2] + fraction * stepZ;

            // ---- Drift distance to the selected LOCAL readout surface
            driftDistance = (fReadoutSide > 0) ? (fActiveMaxY - electronY) : (electronY - fActiveMinY);

            if (driftDistance < -kTolerance || driftDistance > fActiveSizeY + kTolerance)
                continue;

            driftDistance = std::clamp(driftDistance, 0.0, fActiveSizeY);

            driftTime = driftDistance / fDriftVelocity; // New

            // Diffusion accumulated over this electron's own drift distance.
            // sigmaLongAtPadPlane = sqrt(2. * fLongDiff * driftDistance / fDriftVelocity);
            // sigmaTransvAtPadPlane = sqrt(2. * fTransDiff * driftDistance / fDriftVelocity);

            projX = gRandom->Gaus(electronX, sigmaTransvAtPadPlane);
            projZ = gRandom->Gaus(electronZ, sigmaTransvAtPadPlane);
            // projTime = gRandom->Gaus(driftTime + timeBeforeDrift, sigmaLongAtPadPlane / fDriftVelocity);
            projTime = gRandom->Gaus(driftTime + timeBeforeDrift, sigmaTime);

            // ---- Pad lookup (R3BGTPCMap expects offsets from the local minima)
            Double_t planeX = projX - fActiveMinX;
            Double_t planeZ = projZ - fActiveMinZ;
            Int_t padID = fTPCMap->FindPad(planeZ, planeX);
            if (padID < 0)
            {
                continue;
            }

            if (outputMode == 0)
            { // Output: TClonesArray of R3BGTPCCalData
                Int_t nCalData = fGTPCCalDataCA->GetEntriesFast();
                for (Int_t pp = 0; pp < nCalData; pp++)
                {
                    if (((R3BGTPCCalData*)fGTPCCalDataCA->At(pp))->GetPadId() == padID)
                    {
                        // already existing R3BGTPCCalData... add time and electron
                        projTime = projTime / fTimeBinSize; // moving from ns to binsize
                        if (projTime < 0)
                            projTime = 0; // Fills (first) underflow bin
                        else if (projTime > 511)
                            projTime = 511; // Fills (last) overflow bin
                        ((R3BGTPCCalData*)fGTPCCalDataCA->At(pp))->SetADC(projTime);
                        padFound = kTRUE;
                        break;
                    }
                }
                if (!padFound)
                {
                    std::vector<UShort_t> adc(512, 0);
                    projTime = projTime / fTimeBinSize; // moving from ns to binsize
                    if (projTime < 0)
                        projTime = 0; // Fills (first) underflow bin
                    else if (projTime > 511)
                        projTime = 511; // Fills (last) overflow bin
                    adc.at(projTime)++;

                    new ((*fGTPCCalDataCA)[nCalData]) R3BGTPCCalData(padID, adc);
                }
                padFound = kFALSE;
            }
            // else if (outputMode == 1)
            // {

            //     Int_t nProjPoints = fGTPCProjPoint->GetEntriesFast();

            //     for (Int_t pp = 0; pp < nProjPoints; pp++)
            //     {
            //         if (((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->GetVirtualPadID() == padID)
            //         {
            //             // already existing R3BGTPCProjPoint... add time and electron
            //             ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->AddCharge();                      //
            //             ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->SetTimeDistr(projTime / 1000, 1); // micros
            //             padFound = kTRUE;
            //             break;
            //         }
            //     }
            //     if (!padFound)
            //     {
            //         new ((*fGTPCProjPoint)[nProjPoints]) R3BGTPCProjPoint(padID,
            //                                                               projTime / 1000, // micros
            //                                                               1,
            //                                                               evtID,
            //                                                               PDGCode,
            //                                                               MotherId,
            //                                                               Vertex_x0,
            //                                                               Vertex_y0,
            //                                                               Vertex_z0,
            //                                                               Vertex_px0,
            //                                                               Vertex_py0,
            //                                                               Vertex_pz0);
            //     }
            //     padFound = kFALSE;
            // }

            else if (outputMode == 1)
            {
                // --------------------------------------------------------
                // ProjPoint path:
                // exact electron arrival times are stored directly in ns.
                // --------------------------------------------------------

                R3BGTPCProjPoint* padPoint = nullptr;

                const Int_t nProjPoints = fGTPCProjPoint->GetEntriesFast();

                for (Int_t pp = 0; pp < nProjPoints; pp++)
                {
                    auto* candidate = static_cast<R3BGTPCProjPoint*>(fGTPCProjPoint->At(pp));

                    if (candidate->GetVirtualPadID() == padID)
                    {
                        padPoint = candidate;
                        break;
                    }
                }

                if (padPoint)
                {
                    // Another primary electron reached the same pad.
                    // Keep the exact landing point (map plane coordinates) and
                    // the MCTrack index: the VMM3a digitizer needs both for
                    // resistive-layer charge sharing and MC matching.
                    padPoint->AddElectron(projTime, planeX, planeZ, presentTrackID);
                }
                else
                {
                    // First primary electron reaching this pad.
                    new ((*fGTPCProjPoint)[nProjPoints]) R3BGTPCProjPoint(padID,
                                                                          projTime, // already ns
                                                                          planeX,   // [cm], R3BGTPCMap plane convention
                                                                          planeZ,   // [cm], R3BGTPCMap plane convention
                                                                          presentTrackID,
                                                                          PDGCode,
                                                                          MotherId,
                                                                          Vertex_x0,
                                                                          Vertex_y0,
                                                                          Vertex_z0,
                                                                          Vertex_px0,
                                                                          Vertex_py0,
                                                                          Vertex_pz0);
                }
            }
        }

        xPre = xPost;
        yPre = yPost;
        zPre = zPost;

    } // Simulated points
    if (outputMode == 1 && fGTPCProjPoint)
    {
        LOG(info) << "R3BTPCProjector: produced " << fGTPCProjPoint->GetEntriesFast() << " projPoints";
    }
    else if (outputMode == 0 && fGTPCCalDataCA)
    {
        LOG(info) << "R3BTPCProjector: produced " << fGTPCCalDataCA->GetEntriesFast() << " CalData objects";
    }
}

void R3BTPCProjector::Finish() {}

ClassImp(R3BTPCProjector)
