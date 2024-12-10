/******************************************************************************
 *   Copyright (C) 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2018-2025 Members of R3B Collaboration                     *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU Lesser General Public Licence (LGPL) version 3,        *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "R3BGTPCProjector.h"
#include "R3BMCTrack.h"
#include "TClonesArray.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "TF1.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"
using namespace std;

R3BGTPCProjector::R3BGTPCProjector()
    : FairTask("R3BGTPCProjector")
    , fGTPCPoints(NULL)
    , fGTPCCalDataCA(NULL)
    , fGTPCProjPoint(NULL)
    , MCTrackCA(NULL)

{
    fEIonization = 0;
    fDriftVelocity = 0;
    fTransDiff = 0;
    fLongDiff = 0;
    fFanoFactor = 0;
    fSizeOfVirtualPad = 0;
    fTimeBinSize = 0;
    fHalfSizeTPC_X = 0.;
    fHalfSizeTPC_Y = 0.;
    fHalfSizeTPC_Z = 0.;
    fDetectorType = 0;
    outputMode = 0;
    fTPCMap = std::make_shared<R3BGTPCMap>();
}

R3BGTPCProjector::~R3BGTPCProjector()
{

    if (fGTPCPoints)
    {
        fGTPCPoints->Delete();
        delete fGTPCPoints;
    }
    if (fGTPCCalDataCA)
    {
        fGTPCCalDataCA->Delete();
        delete fGTPCCalDataCA;
    }
    /*if (fGTPCProjPoint)
       {

         fGTPCProjPoint->Delete();
         delete fGTPCProjPoint;
     }*/
    if (MCTrackCA)
    {
        MCTrackCA->Delete();
        delete MCTrackCA;
    }
}

void R3BGTPCProjector::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    if (!run)
    {
        LOG(fatal) << "R3BGTPCProjector::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(fatal) << "R3BGTPCProjector::SetParContainers: No runtime database";
        return;
    }

    fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!fGTPCGeoPar)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGeoPar";
        return;
    }
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

void R3BGTPCProjector::SetParameter()
{
    fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]-> typical value for a
                                                      // gas detector tens of eV
    fDriftVelocity = fGTPCGasPar->GetDriftVelocity(); // [cm/ns]-> Minos TPC with a Efield=152 V/cm
    fTransDiff = fGTPCGasPar->GetTransDiff();         // [cm^2/ns]?
    fLongDiff = fGTPCGasPar->GetLongDiff();           // [cm^2/ns]?
    fFanoFactor = fGTPCGasPar->GetFanoFactor();
    fSizeOfVirtualPad = fGTPCGeoPar->GetPadSize(); // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    fHalfSizeTPC_X = fGTPCGeoPar->GetActiveRegionx() / 2.;
    fHalfSizeTPC_Y = fGTPCGeoPar->GetActiveRegiony() / 2.;
    fHalfSizeTPC_Z = fGTPCGeoPar->GetActiveRegionz() / 2.;
    fDetectorType = fGTPCGeoPar->GetDetectorType();

    // TODO
    // From electronic properties
    // fDriftEField = fGTPCElecPar->GetDriftEField();     // drift E field in V/m
    // fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // time step for drift
    // params calculation
    fTimeBinSize = 1000.0; // fGTPCElecPar->GetTimeBinSize();     // time step for
                           // drift params calculation
}

InitStatus R3BGTPCProjector::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(fatal) << "R3BGTPCProjector::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    {
        LOG(fatal) << "R3BGTPCProjector::Init No GTPCPoint!";
        return kFATAL;
    }
    fGTPCPoints = (TClonesArray*)ioman->GetObject("GTPCPoint");
    // Input: TClonesArray of R3BMCTrack
    if ((TClonesArray*)ioman->GetObject("MCTrack") == nullptr)
    {
        LOG(fatal) << "R3BMCTrack::Init No MCTrack!";
        return kFATAL;
    }
    MCTrackCA = (TClonesArray*)ioman->GetObject("MCTrack");

    fGTPCCalDataCA = new TClonesArray("R3BGTPCCalData");
    fGTPCProjPoint = new TClonesArray("R3BGTPCProjPoint");
    if (outputMode == 0)
    { // Output: TClonesArray of R3BGTPCCalData
        ioman->Register("GTPCCalData", GetName(), fGTPCCalDataCA, kTRUE);
    }
    else if (outputMode == 1)
    { // Output: TClonesArray of R3BGTPCProjPoint

        ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPoint, kTRUE);
    }

    SetParameter();

    // Pad plane generation
    fTPCMap->GeneratePadPlane();
    fPadPlane = fTPCMap->GetPadPlane();

    if (fPadPlane == NULL)
    {
        std::cout << " R3BGTPCProjector::Init() error! - Could not retrieve pad "
                     "plane. Exiting..."
                  << "\n";
        return kERROR;
    }

    return kSUCCESS;
}

InitStatus R3BGTPCProjector::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}

void R3BGTPCProjector::SetDriftParameters(Double_t ion,
                                          Double_t driftv,
                                          Double_t tDiff,
                                          Double_t lDiff,
                                          Double_t fanoFactor)
{
    fEIonization = ion;      // [GeV]
    fDriftVelocity = driftv; // [cm/ns]
    fTransDiff = tDiff;      // [cm^(-1/2)]
    fLongDiff = lDiff;       // [cm^(-1/2)]
    fFanoFactor = fanoFactor;
}

void R3BGTPCProjector::SetSizeOfVirtualPad(Double_t size)
{
    fSizeOfVirtualPad = size; // 1 means pads of 1cm^2, 5 means pads of 2mm^2, ...
}

void R3BGTPCProjector::Exec(Option_t*)
{

    if (outputMode == 0)
    {
        fGTPCCalDataCA->Clear("C");
    }
    else if (outputMode == 1)
    {
        fGTPCProjPoint->Clear("C");
    }

    Int_t nPoints = fGTPCPoints->GetEntries();
    LOG(info) << "R3BGTPCProjector: processing " << nPoints << " points";
    if (nPoints < 2)
    {
        LOG(info) << "Not enough hits for digitization! (<2)";
        return;
    }

    R3BGTPCPoint* aPoint;
    Int_t presentTrackID = -10; // control of the point trackID
    Double_t xPre, yPre, zPre;
    Double_t xPost, yPost, zPost;
    Double_t projX, projZ, projTime;
    Double_t energyDep = 0.;
    Double_t timeBeforeDrift = 0.;
    Bool_t readyToProject = kFALSE;
    Bool_t padFound = kFALSE;
    Int_t electrons = 0;
    Int_t flucElectrons = 0;
    Int_t generatedElectrons = 0;
    Double_t stepX, stepY, stepZ;
    Double_t driftDistance, driftTime;
    Double_t sigmaLongAtPadPlane;
    Double_t sigmaTransvAtPadPlane;
    Int_t evtID = 0;
    for (Int_t i = 0; i < nPoints; i++)
    {
        aPoint = (R3BGTPCPoint*)fGTPCPoints->At(i);
        evtID = aPoint->GetEventID();
        Int_t PDGCode = 0, MotherId = 0;
        Double_t Vertex_x0 = 0, Vertex_y0 = 0, Vertex_z0 = 0, Vertex_px0 = 0, Vertex_py0 = 0, Vertex_pz0 = 0;
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
                LOG(fatal) << "R3BGTPCProjector::Exec: Problem 2 in point logic";
                break;
            }
            if (readyToProject != kTRUE)
            {
                LOG(fatal) << "R3BGTPCProjector::Exec: Problem 3 in point logic";
                break;
            }
            if (aPoint->GetTrackStatus() == 10100 || aPoint->GetTrackStatus() == 1000000)
            { // exiting the gas volume or dissappearing
                readyToProject = kFALSE;
            }

            xPost = aPoint->GetX();
            yPost = aPoint->GetY();
            zPost = aPoint->GetZ();
            energyDep = aPoint->GetEnergyLoss();
            timeBeforeDrift = aPoint->GetTime();
        }
        // primary electrons produced by the ionization
        electrons = energyDep / fEIonization;
        // electron number fluctuates as the square root of the
        // Fano factor times the number of electrons
        flucElectrons = pow(fFanoFactor * electrons, 0.5);
        generatedElectrons = gRandom->Gaus(electrons, flucElectrons); // generated electrons

        // step in each direction for an homogeneous electron creation position
        // along the track
        stepX = (xPost - xPre) / generatedElectrons;
        stepY = (yPost - yPre) / generatedElectrons;
        stepZ = (zPost - zPre) / generatedElectrons;

        // taken a mean driftDistance for the calculation of the sigmaLong and
        // sigmaTrans improve (make the calculation individual for electron) if
        // needed, but probably slower
        Double_t yApprox = (yPost + yPre) / 2;
        driftDistance = yApprox + fHalfSizeTPC_Y;
        // cout<<"DriftDistance="<<driftDistance<<"	yApprox="<<yApprox<<endl;
        sigmaLongAtPadPlane = sqrt(driftDistance * 2 * fLongDiff / fDriftVelocity);
        sigmaTransvAtPadPlane = sqrt(driftDistance * 2 * fTransDiff / fDriftVelocity);

        for (Int_t ele = 1; ele <= generatedElectrons; ele++) // following each electrons from production to pad
        {
            driftTime = ((yPre + stepY * ele) + fHalfSizeTPC_Y) / fDriftVelocity;
            projX = gRandom->Gaus(xPre + stepX * ele, sigmaTransvAtPadPlane);
            projZ = gRandom->Gaus(zPre + stepZ * ele, sigmaTransvAtPadPlane);
            projTime = gRandom->Gaus(driftTime + timeBeforeDrift, sigmaLongAtPadPlane / fDriftVelocity);
            // cout<<"projTime="<<projTime<<"		driftTime="<<driftTime<<"
            // timeBeforeDrift="<<timeBeforeDrift<<endl; cout<<"ProjZ="<<projZ<<"
            // ProjX="<<projX<<endl; obtain padID for projX, projZ (simple algorithm)
            // for the Prototype the algorithm assigns a pad number which depends on
            // the projX and projZ, taking into consideration the Offset (that depends
            // on the position inside GLAD), the X and Z where the pad plane starts.
            // Since the prototype pad plane has 44x126 pads, to avoid repetition in
            // the padID Z is multiplied by 45 and since the size of the pad is 2x2
            // mm^2, X and Z are divided by 0.2 cm ZOffset- z-> the first pad row in
            // the laboratory frame
            double ZOffset = 272.7;
            // XOffset-x-> the first pad column in the laboratory frame
            double XOffset = 5.8;

            if (projZ < ZOffset)
                projZ = ZOffset;
            if (projZ > ZOffset + 2 * fHalfSizeTPC_Z)
                projZ = ZOffset + 2 * fHalfSizeTPC_Z;
            if (projX < XOffset)
                projX = XOffset;
            if (projX > XOffset + 2 * fHalfSizeTPC_X)
                projX = XOffset + 2 * fHalfSizeTPC_X;

            // std::cout<<" proj Z "<<projZ<<" - proj Y "<<projY<<"\n";
            Int_t padID = fPadPlane->Fill((projZ - ZOffset) * 10.0,
                                          (projX - XOffset) * 10.0); // in mm

            // Deprecated code to remove
            /*(if (fDetectorType == 1)
                padID = (44) * (Int_t)((projZ - ZOffset) / 0.2) + (Int_t)((projX -
            XOffset) / 0.2); // 2mm else padID = (2 * fHalfSizeTPC_X *
            fSizeOfVirtualPad) * (Int_t)((projZ - ZOffset) * fSizeOfVirtualPad) +
                        (Int_t)((projX - XOffset) * fSizeOfVirtualPad); // FULL HYDRA
            padplane has not been decided yet
        */

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
            else if (outputMode == 1)
            {

                Int_t nProjPoints = fGTPCProjPoint->GetEntriesFast();

                for (Int_t pp = 0; pp < nProjPoints; pp++)
                {
                    if (((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->GetVirtualPadID() == padID)
                    {
                        // already existing R3BGTPCProjPoint... add time and electron
                        ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->AddCharge();                      //
                        ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->SetTimeDistr(projTime / 1000, 1); // micros
                        padFound = kTRUE;
                        break;
                    }
                }
                if (!padFound)
                {
                    new ((*fGTPCProjPoint)[nProjPoints]) R3BGTPCProjPoint(padID,
                                                                          projTime / 1000, // micros
                                                                          1,
                                                                          evtID,
                                                                          PDGCode,
                                                                          MotherId,
                                                                          Vertex_x0,
                                                                          Vertex_y0,
                                                                          Vertex_z0,
                                                                          Vertex_px0,
                                                                          Vertex_py0,
                                                                          Vertex_pz0);
                }
                padFound = kFALSE;
            }
        }

        xPre = xPost;
        yPre = yPost;
        zPre = zPost;

    } // Simulated points
    LOG(info) << "R3BGTPCProjector: produced " << fGTPCProjPoint->GetEntries() << " projPoints";
}

void R3BGTPCProjector::Finish() {}

ClassImp(R3BGTPCProjector)
