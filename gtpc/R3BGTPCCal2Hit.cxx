/******************************************************************************
 *   Copyright (C) 2019 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2019 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "TClonesArray.h"
#include "TMath.h"

#include "R3BGTPC.h"
#include "R3BGTPCCal2Hit.h"
#include "R3BGladFieldMap.h"

#include "TVirtualMC.h" //Included this to use gRandom ??? Maybe more simple


// R3BGTPCCal2Hit: Constructor
R3BGTPCCal2Hit::R3BGTPCCal2Hit()
    : PadCoordArr(boost::extents[5632][4][2])
    , FairTask("R3B GTPC Cal to Hit")
    //    , fHitParams(NULL)
    , fHit_Par(NULL)
    , fCalCA(NULL)
    , fHitCA(NULL)
    , fTPCMap(NULL)
    , fOnline(kFALSE)
    , fLangevinBack(kTRUE)
{
    fTPCMap = std::make_shared<R3BGTPCMap>();
}

R3BGTPCCal2Hit::~R3BGTPCCal2Hit()
{
    LOG(INFO) << "R3BGTPCCal2Hit: Delete instance";
    if (fHitCA)
        delete fHitCA;
    if (fCalCA)
        delete fCalCA;
}

void R3BGTPCCal2Hit::SetParContainers()
{
    // Reading GTPCHitPar from FairRuntimeDb
    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    if (!rtdb)
    {
        LOG(ERROR) << "R3BGTPCCal2Hit:: FairRuntimeDb not opened!";
    }



    fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!fGTPCGeoPar)
    {
        LOG(FATAL) << "R3BGTPCCal2Hit::SetParContainers: No R3BGTPCGeoPar";
        return;
    }

    fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!fGTPCGasPar)
    {
        LOG(FATAL) << "R3BGTPCCal2Hit::SetParContainers: No R3BGTPCGasPar";
        return;
    }
    fGTPCElecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!fGTPCElecPar)
    {
        LOG(FATAL) << "R3BGTPCCal2Hit::SetParContainers: No R3BGTPCElecPar";
        return;
    }

    fHit_Par = (R3BGTPCHitPar*)rtdb->getContainer("GTPCHitPar");
    if (!fHit_Par)
    {
        LOG(ERROR) << "R3BGTPCCal2Hit::Init() Couldn't get handle on GTPCHitPar container";
    }
    else
    {
        LOG(INFO) << "R3BGTPCCal2Hit:: GTPCHitPar container open";
    }

}

void R3BGTPCCal2Hit::SetParameter()
{
    // From gas properties
    fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]-> typical value for a gas detector tens of eV
    fDriftVelocity = fGTPCGasPar->GetDriftVelocity(); // [cm/ns]-> Minos TPC with a Efield=152 V/cm
    fTransDiff = fGTPCGasPar->GetTransDiff();         // [cm^2/ns]?
    fLongDiff = fGTPCGasPar->GetLongDiff();           // [cm^2/ns]?
    fFanoFactor = fGTPCGasPar->GetFanoFactor();
    // From geo properties
    fHalfSizeTPC_X = fGTPCGeoPar->GetActiveRegionx() / 2.;
    fHalfSizeTPC_Y = fGTPCGeoPar->GetActiveRegiony() / 2.;
    fHalfSizeTPC_Z = fGTPCGeoPar->GetActiveRegionz() / 2.;
    fSizeOfVirtualPad = fGTPCGeoPar->GetPadSize(); // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    fDetectorType = fGTPCGeoPar->GetDetectorType();
    // From electronic properties
    fDriftEField = fGTPCElecPar->GetDriftEField();     // drift E field in V/m
    fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // time step for drift params calculation
    fTimeBinSize = fGTPCElecPar->GetTimeBinSize();     // time step for drift params calculation

    //--- Parameter Container ---
    /*
        fHitParams = new TArrayF();
        fHitParams = fHit_Par->GetHitParams(); // Array with the Hit parameters
    */
}

InitStatus R3BGTPCCal2Hit::Init()
{
    LOG(INFO) << "R3BGTPCCal2Hit::Init() ";
    assert(!fHitCA); // in case someone calls Init() twice.

    // INPUT DATA - Cal
    FairRootManager* ioManager = FairRootManager::Instance();
    if (!ioManager)
        LOG(fatal) << "Init: No FairRootManager";

    fCalCA = (TClonesArray*)ioManager->GetObject("GTPCCalData");
    if (!fCalCA)
        LOG(fatal) << "Init: No R3BGTPCCalData";

    // Register output - Hit
    fHitCA = new TClonesArray("R3BGTPCHitData", 50);
    if (!fOnline)
    {
        ioManager->Register("GTPCHitData", "GTPC Hit", fHitCA, kTRUE);
    }
    else
    {
        ioManager->Register("GTPCHitData", "GTPC Hit", fHitCA, kFALSE);
    }

    SetParameter();

    fTPCMap->GeneratePadPlane();

    return kSUCCESS;
}

InitStatus R3BGTPCCal2Hit::ReInit()
{
    SetParContainers();
    SetParameter();
    return kSUCCESS;
}


void R3BGTPCCal2Hit::Exec(Option_t* opt)
{
    Reset(); // Reset entries in output arrays, local arrays

    if (!fHit_Par)
    {
        LOG(WARNING) << "R3BGTPCCal2Hit::NO Container Parameter!!";
    }

    // ALGORITHMS FOR HIT FINDING
    // Nb of CrystalHits in current event
    Int_t nCals = fCalCA->GetEntries();
    LOG(INFO) << "R3BGTPCCal2Hit: processing " << nCals << " CalPads";

    if (!nCals)
        cout << "\033[1;31m NO CalPads\033[0m" << endl;

    Double_t x = 0, y = 0, z = 0, lW = 0, ene = 0;
    R3BGladFieldMap* gladField = (R3BGladFieldMap*)FairRunAna::Instance()->GetField(); //B Field
    if (!gladField)
        cout << "\033[1;31m NO gladField\033[0m" << endl;


    R3BGTPCCalData** calData;
    calData = new R3BGTPCCalData*[nCals];

    for (Int_t i = 0; i < nCals; i++)
    {
        calData[i] = (R3BGTPCCalData*)(fCalCA->At(i));
        UShort_t pad = calData[i]->GetPadId();
        std::vector<UShort_t> adc_cal = calData[i]->GetADC();

        Double_t max = 0; // TODO: Different algorithms for finding pulses have to be implemented here.
        Double_t time = 0;
        for (auto iadc = 0; iadc < adc_cal.size(); ++iadc)
        {
            // std::cout<<iadc<<" "<<adc_cal[iadc]<<"\n";
            if (adc_cal[iadc] > max)
            {
                max = adc_cal[iadc];
                time = iadc;
            }
        }

        auto PadCenterCoord = fTPCMap->CalcPadCenter(pad);

        if (PadCenterCoord[0] > -9000)
        {
            z = PadCenterCoord[0]/10.0; //[cm] (PadCenterCoord on mm)
            x = PadCenterCoord[1]/10.0;
            y = -fHalfSizeTPC_Y + 0.01;
        }
        //if (PadCenterCoord[0] < -9000){std::cout << PadCenterCoord[0] << '\n';}
        // TODO: To parameter container
        double ZOffset = 272.7; // [cm]
        double XOffset = 5.8; // [cm]
        x = x + XOffset;
        z = z + ZOffset;

        /*Int_t iniTB = 512;                          // Initial Time Bucket
        Double_t driftVel = 1.0;                    // cm/us
        Double_t samplingRate = 0.080;              // us 12.5 MHz
        y = (time - 512) * driftVel * samplingRate; // cm         */

        // Reconstruction without Langevin
        if (fLangevinBack == kFALSE)
        {
            // TODO: To parameter container
            Double_t samplingRate = 0.080;              // us 12.5 MHz
            Int_t iniTB = 512;                          // Initial Time Bucket
            Double_t driftVel = 1.0;                    // cm/us

            y = (time - 512) * driftVel * samplingRate; // cm
        }
        if (fLangevinBack == kTRUE)
        {
            //This is what I have to do
            //Add parameters here:

            time = time * fTimeBinSize; //moving from TimeBuckets to ns
            Double_t accDriftTime = time; //copy for loop discounting
            Double_t E_x = 0;
            Double_t E_z = 0;

            Double_t E_y = fDriftEField; // in V/m or V/cm ?

            Double_t B_x = 0;
            Double_t B_y = 0;
            Double_t B_z = 0;
            Double_t moduleB = 0;

            Double_t vDrift_x = 0;
            Double_t vDrift_y = 0;
            Double_t vDrift_z = 0;

            Double_t cteMult = 0;
            Double_t cteMod = 0;
            Double_t productEB = 0;

            Double_t sigmaLongStep;
            Double_t sigmaTransvStep;
            Double_t recoverDriftTimeStep = 0.;
            Bool_t recoverStep = kTRUE;

            //Double_t driftVel = 1.0;
            Double_t mu = fDriftVelocity / E_y; // [cm^2 ns^-1 V^-1]

            LOG(DEBUG) << "R3BGTPCCal2Hit::Exec, INITIAL VALUES: \tTimeToRun=" << accDriftTime << " [ns]"
                       << " \tx=" << x << "  \ty=" << y << " \tz=" << z << " [cm]";
            //Calculation Loop:
            Int_t while_counter = 0;
            while (accDriftTime > 0.)//Iteramos hasta agotar el tiempo
            {
              while_counter += 1;

              B_x = 1e-14 * gladField->GetBx(x, y, z); // Field components return in [kG], moved to [V ns cm^-2]
              B_y = 1e-14 * gladField->GetBy(x, y, z);
              B_z = 1e-14 * gladField->GetBz(x, y, z);
              //std::cout << "MAGNETIC FIELD || Bx: "<<B_x<< " \tBy: "<<B_y<<" \tBz: "<<B_z<< " \tITERATION "<<while_counter<<'\n';


              moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // in [V ns cm^-2]
              cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // adimensional
              cteMult = mu * cteMod;                                    // [cm^2 V^-1 ns^-1]

              productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; [V^2 ns cm^-3]

              //Velocidades de deriva
              vDrift_x = cteMult * (mu * (E_y * B_z) + mu * mu * productEB * B_x); // cte * (Ex + mu*(E_y*B_z-E_z*B_y) + mu*mu*productEB*B_x); [cm/ns]
              vDrift_y = cteMult * (E_y + mu * mu * productEB * B_y); // cte * (Ey + mu*(E_z*B_x-E_x*B_z) + mu*mu*productEB*B_y); [cm/ns]
              vDrift_z = cteMult * (mu * (-E_y * B_x) + mu * mu * productEB * B_z); // cte * (Ez + mu*(E_x*B_y-E_y*B_x) + mu*mu*productEB*B_z); [cm/ns]
              LOG(DEBUG) << "R3BGTPCCal2Hit::Exec, DRIFT VELOCITIES: vDrift_x=" << vDrift_x << " vDrift_y=" << vDrift_y
                         << " vDrift_z=" << vDrift_z << " [cm/ns]";

              //Ajustamos ultimo paso
              if (accDriftTime - fDriftTimeStep < 0.0)
              {
                if (recoverStep){recoverDriftTimeStep = fDriftTimeStep;}
                recoverStep = kFALSE;
                fDriftTimeStep = accDriftTime;
              }

              sigmaTransvStep = TMath::Sqrt(fDriftTimeStep * 2 * fTransDiff * cteMod); // should be reduced by the factor cteMod=cteMult/mu
              sigmaLongStep = TMath::Sqrt(fDriftTimeStep * 2 * fLongDiff); // should be the same scaled to the length of the step
              //Invertimos los signos de velocidad*tiempo para ir para atras en lugar de hacia delante
              x = gRandom->Gaus(x - vDrift_x * fDriftTimeStep, sigmaTransvStep); // [cm]

              y = gRandom->Gaus(y + vDrift_y * fDriftTimeStep, sigmaLongStep); // [cm]
              z = gRandom->Gaus(z - vDrift_z * fDriftTimeStep, sigmaTransvStep); // [cm]
              accDriftTime = accDriftTime - fDriftTimeStep;
              LOG(DEBUG) << "R3BGTPCCal2Hit::Exec, NEW VALUES: accDriftTime=" << accDriftTime << " [ns]"
                         << " x=" << x << " y=" << y << " z=" << z << " [cm]";

            }
            //cout << "\033[1;31m While iterations\033[0m: " << while_counter << endl;
            if (recoverDriftTimeStep)
            {
                fDriftTimeStep = recoverDriftTimeStep;
                recoverDriftTimeStep = 0.;
                recoverStep = kTRUE;
            }
        }

        //LOG(INFO) << "RESULTS Pad: " << pad << "; Reco(x, y, z): (" << x << ", " << y << ", " << z << ")" << endl << endl;

        // NB: Translation into GLAD frame for physics.
        //Only adding valid Hits
        if (PadCenterCoord[0] > -9000)
        {
          AddHitData(x, y, z , lW, max);
          //if (x>15.){std::cout << "\033[1;31m Error\033[0m" << '\n';}
        }
    }

    if (calData)
        delete calData;
    return;
}

void R3BGTPCCal2Hit::Finish() {}

void R3BGTPCCal2Hit::Reset()
{
    LOG(DEBUG) << "Clearing HitData Structure";
    if (fHitCA)
        fHitCA->Clear();
}

R3BGTPCHitData* R3BGTPCCal2Hit::AddHitData(Double_t x, Double_t y, Double_t z, Double_t longWidth, Double_t energy)
{
    // It fills the R3BGTPCCalData
    TClonesArray& clref = *fHitCA;
    Int_t size = clref.GetEntriesFast();
    return new (clref[size]) R3BGTPCHitData(x, y, z, longWidth, energy);
}

ClassImp(R3BGTPCCal2Hit)
