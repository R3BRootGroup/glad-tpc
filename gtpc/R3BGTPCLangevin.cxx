#include "R3BGTPCLangevin.h"

#include "TClonesArray.h"
#include "TMath.h"
//#include "TVirtualMC.h"
//#include "TVirtualMagField.h"

#include "R3BGladFieldMap.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "R3BGTPCSetup.h"

std::string tag= "Prototype";
std::string tag1= "FullBeamOut";	  
std::string tag2= "FullBeamIn";	  
std::string TAG = tag;

R3BGTPCLangevin::R3BGTPCLangevin()
    : FairTask("R3BGTPCLangevin")
{
    // ALL UNITS IN cm, ns, V/cm, Tesla and GeV
		R3BGTPCSetup* setup = new R3BGTPCSetup(TAG,1);
		LOG(INFO) << "\033[1;31m Warning\033[0m: The detector is: " << TAG;
    fEIonization = setup->GetEIonization();  // [GeV]-> typical value for a gas detector tens of eV
    fDriftVelocity = setup->GetDriftVelocity(); // [cm/ns]-> Minos TPC with a Efield=152 V/cm
    fTransDiff = setup->GetTransDiff(); // [cm^2/ns]?
    fLongDiff = setup->GetLongDiff();   // [cm^2/ns]?
    fFanoFactor = setup->GetFanoFactor();
    fSizeOfVirtualPad = setup->GetPadSize(); // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    fHalfSizeTPC_X = 0.; // to be filled during Init()
    fHalfSizeTPC_Y = 0.; // to be filled during Init()
    fHalfSizeTPC_Z = 0.; // to be filled during Init()
    delete setup;
}

R3BGTPCLangevin::~R3BGTPCLangevin()
{
    fGTPCPoints->Clear();
    fGTPCProjPoint->Clear();
}

void R3BGTPCLangevin::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    if (!run)
    {
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No runtime database";
        return;
    }
    // PREPARE PROPER PARAMETER CONTAINERS
    /*fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("R3BGTPCGeoPar");
      if (!fGTPCGeoPar) {
      LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGeoPar";
      return;
      }
    */
    /*fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("R3BGTPCGasPar");
      if (!fGTPCGasPar) {
      LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGasPar";
      return;
      }
    */
}

InitStatus R3BGTPCLangevin::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(FATAL) << "R3BGTPCLangevin::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    {
        LOG(FATAL) << "R3BGTPCLangevin::Init No GTPCPoint!";
        return kFATAL;
    }
    fGTPCPoints = (TClonesArray*)ioman->GetObject("GTPCPoint");

    // Output: TClonesArray of R3BGTPCProjPoint
    fGTPCProjPoint = new TClonesArray("R3BGTPCProjPoint");
    ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPoint, kTRUE);

    /*   //PREPARE PROPER PARAMETER CONTAINERS FOR DRIFT PARAMETERS
         fEIonize  = fPar->GetEIonize()/1000000000; // [GeV]
         fVelDrift = fPar->GetDriftVelocity(); // [cm/us]
         fCoefT    = fPar->GetCoefDiffusionTrans()*sqrt(10.); // [cm^(-1/2)] to [mm^(-1/2)]
         fCoefL    = fPar->GetCoefDiffusionLong()*sqrt(10.);  // [cm^(-1/2)] to [mm^(-1/2)]
    */

    R3BGTPCSetup* setup = new R3BGTPCSetup("Prototype",1);
    fHalfSizeTPC_X = setup->GetActiveRegionx() / 2.;
    fHalfSizeTPC_Y = setup->GetActiveRegiony() / 2.;
    fHalfSizeTPC_Z = setup->GetActiveRegionz() / 2.;
    delete setup;

    return kSUCCESS;
}

void R3BGTPCLangevin::SetDriftParameters(Double_t ion,
                                         Double_t driftv,
                                         Double_t tDiff,
                                         Double_t lDiff,
                                         Double_t fanoFactor)
{
    fEIonization = ion;       // [GeV]
    fDriftVelocity = driftv;  // [cm/ns]
    fTransDiff = tDiff;       // [cm^(-1/2)]
    fLongDiff = lDiff;        // [cm^(-1/2)]
    fFanoFactor = fanoFactor; //
}

void R3BGTPCLangevin::SetSizeOfVirtualPad(Double_t size)
{
    fSizeOfVirtualPad = size; // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
}

void R3BGTPCLangevin::Exec(Option_t*)
{

    fGTPCProjPoint->Clear("C");

    Int_t nPoints = fGTPCPoints->GetEntries();
    LOG(INFO) << "R3BGTPCLangevin: processing " << nPoints << " points";
    if (nPoints < 2)
    {
        LOG(INFO) << "Not enough hits for digitization! (<2)";
        return;
    }

    R3BGladFieldMap* gladField = (R3BGladFieldMap*)FairRunAna::Instance()->GetField();

    R3BGTPCPoint* aPoint;
    // R3BGTPCProjPoint* aProjPoint;
    Int_t presentTrackID = -10; // control of the point trackID
    Double_t xPre, yPre, zPre;
    Double_t xPost, yPost, zPost;
    Double_t projX, projZ, projTime;
    Double_t energyDep = 0.;
    Double_t timeBeforeDrift = 0.;
    Bool_t readyToProject = kFALSE;
    Bool_t virtualPadFound = kFALSE;
    Int_t electrons = 0;
    Int_t flucElectrons = 0;
    Int_t generatedElectrons = 0;
    Double_t stepX, stepY, stepZ;
    Double_t driftDistance, driftTime;
    Double_t sigmaLongAtPadPlane;
    Double_t sigmaTransvAtPadPlane;
    Int_t evtID;

    for (Int_t i = 0; i < nPoints; i++)
    {
        aPoint = (R3BGTPCPoint*)fGTPCPoints->At(i);
        evtID = aPoint->GetEventID();
        if (aPoint->GetTrackStatus() == 11000 || aPoint->GetTrackStatus() == 10010010 ||
            aPoint->GetTrackStatus() == 10010000 || aPoint->GetTrackStatus() == 10011000)
        {
            // entering the gas volume or new track inside the gas (is 10010010 or 10010000??)
            presentTrackID = aPoint->GetTrackID();
            xPre = aPoint->GetX();
            yPre = aPoint->GetY();
            zPre = aPoint->GetZ();
            readyToProject = kTRUE;
            continue; // no energy deposited in this point, just taking in entrance coordinates
        }
        else
        { // any other case
            if (presentTrackID != aPoint->GetTrackID())
            {
                LOG(FATAL) << "R3BGTPCLangevin::Exec: Problem 2 in point logic";
                break;
            }
            if (readyToProject != kTRUE)
            {
                LOG(FATAL) << "R3BGTPCLangevin::Exec: Problem 3 in point logic";
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
            timeBeforeDrift = aPoint->GetTime(); // REALLY IN ns?????
        }

        electrons = energyDep / fEIonization;
        // electron number fluctuates as the square root of the
        // Fano factor times the number of electrons
        flucElectrons = pow(fFanoFactor * electrons, 0.5);
        generatedElectrons = gRandom->Gaus(electrons, flucElectrons); // generated electrons

        // step in each direction for an homogeneous electron creation position along the track
        stepX = (xPost - xPre) / generatedElectrons;
        stepY = (yPost - yPre) / generatedElectrons;
        stepZ = (zPost - zPre) / generatedElectrons;

        // taken a mean driftDistance for the calculation of the sigmaLong and sigmaTrans
        // improve (make the calculation individual for electron) if needed, but probably slower
        Double_t yApprox = (yPost + yPre) / 2;
        driftDistance = yApprox + fHalfSizeTPC_Y;
        sigmaLongAtPadPlane = sqrt(driftDistance * 2 * fLongDiff / fDriftVelocity);
        sigmaTransvAtPadPlane = sqrt(driftDistance * 2 * fTransDiff / fDriftVelocity);

        // Using here m, V, T, s just to simplify in SI system...
        Double_t E_x = 0;
        Double_t E_z = 0;
        Double_t E_y = 100000; //(1000 V/cm) TODO check value, move to parameter container
        Double_t B_x = 0;
        Double_t B_y = 0;
        Double_t B_z = 0;
        Double_t moduleB = 0;
        Double_t ele_x = 0;
        Double_t ele_y = 0;
        Double_t ele_z = 0;
        Double_t vDrift_x = 0;
        Double_t vDrift_y = 0;
        Double_t vDrift_z = 0;
        Double_t accDriftTime = 0;
        Double_t cteMult = 0;
        Double_t cteMod = 0;
        Double_t productEB = 0;
        Double_t sigmaLongStep;
        Double_t sigmaTransvStep;

        Double_t driftTimeStep = 500;               // 100ns TODO set as variable or move to parameter container
        Double_t mu = 1.E+5 * fDriftVelocity / E_y; // [m2 s-1 V-1] TODO check value, move to parameter container

        for (Int_t ele = 1; ele <= generatedElectrons; ele++)
        {
            ele_x = xPre + stepX * ele; // complete homogeneous electron creation along the step [cm]
            ele_y = yPre + stepY * ele;
            ele_z = zPre + stepZ * ele;
            accDriftTime = timeBeforeDrift;
            driftTimeStep = 500;

            LOG(DEBUG) << "R3BGTPCLangevin::Exec, INITIAL VALUES: timeBeforeDrift=" << accDriftTime << " [ns]"
                       << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";

            while (ele_y > -fHalfSizeTPC_Y)
            {                                                      // while not reaching the pad plane [cm]
                B_x = 0.1 * gladField->GetBx(ele_x, ele_y, ele_z); // Field components return in [kG], moved to [T]
                B_y = 0.1 * gladField->GetBy(ele_x, ele_y, ele_z);
                B_z = 0.1 * gladField->GetBz(ele_x, ele_y, ele_z);

                moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // in [T]
                cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // SI
                cteMult = mu * cteMod;                                    // SI

                // assuming only vertical electric field in the next four lines
                productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; SI

                vDrift_x = cteMult *
                           (mu * (E_y * B_z) +
                            mu * mu * productEB * B_x); // cte * (Ex + mu*(E_y*B_z-E_z*B_y) + mu*mu*productEB*B_x); SI
                vDrift_y = cteMult * (E_y + mu * mu * productEB *
                                                B_y); // cte * (Ey + mu*(E_z*B_x-E_x*B_z) + mu*mu*productEB*B_y); SI
                vDrift_z = cteMult *
                           (mu * (-E_y * B_x) +
                            mu * mu * productEB * B_z); // cte * (Ez + mu*(E_x*B_y-E_y*B_x) + mu*mu*productEB*B_z); SI

                LOG(DEBUG) << "R3BGTPCLangevin::Exec, timeBeforeDrift=vDrift_x=" << vDrift_x << " vDrift_y=" << vDrift_y
                           << " vDrift_z=" << vDrift_z << " [m/s]";

                // adjusting the last step before the pad plane
                if (ele_y - 1.E-7 * vDrift_y * driftTimeStep < -fHalfSizeTPC_Y)
                    driftTimeStep = (ele_y + fHalfSizeTPC_Y) / (1.E-7 * vDrift_y); // vDrift back to [cm/ns]

                // reducing sigmaTransv (see http://web.ift.uib.no/~lipniack/detectors/lecture5/detector5.pdf)
                // as B~B_y and E=E_y, let's simplify and apply the reduction to the transversal coefficient without
                // projections
                sigmaTransvStep =
                    sqrt(driftTimeStep * 2 * fTransDiff * cteMod); // should be reduced by the factor cteMod=cteMult/mu
                sigmaLongStep =
                    sqrt(driftTimeStep * 2 * fLongDiff); // should be the same scaled to the length of the step
                ele_x =
                    gRandom->Gaus(ele_x + 1.E-7 * vDrift_x * driftTimeStep, sigmaTransvStep); // vDrift back to [cm/ns]
                ele_y = gRandom->Gaus(ele_y - 1.E-7 * vDrift_y * driftTimeStep, sigmaLongStep);
                ele_z = gRandom->Gaus(ele_z + 1.E-7 * vDrift_z * driftTimeStep, sigmaTransvStep);
                accDriftTime = accDriftTime + driftTimeStep;

                LOG(DEBUG) << "R3BGTPCLangevin::Exec, accDriftTime=" << accDriftTime << " [ns]"
                           << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
            }

            projX = ele_x;
            projZ = ele_z;
            projTime = accDriftTime;

            // obtain padID for projX, projZ (simple algorithm)
            // the algorithm assigns a pad number which depends on the XZ size of the chamber,
            // according to the fSizeOfVirtualPad parameter: if it is 1, the pad size is cm^2
            // and padID goes from 0 to 2*fHalfSizeTPC_X in the first row (ZOffset),
            // from 2*fHalfSizeTPC_X to 4*fHalfSizeTPC_X in the second row (ZOffset+1), ...
            // if fSizeOfVirtualPad=0.1, then padID goes from 0 to 20*fHalfSizeTPC_X for (Z~200.0),
            // from 20*fHalfSizeTPC_X to 40*fHalfSizeTPC_X  (Z~200.0), ...
            // Avoid first moving out of the virtual pad plane limits
            //ZOffset- z of the first pad row in the laboratory frame
            double ZOffset=272.7;
            //XOffset-y of the first pad column in the laboratory frame
            double XOffset=5.8;
            if (projZ < ZOffset)
                projZ = ZOffset;
            if (projZ > ZOffset + 2 * fHalfSizeTPC_Z)
                projZ = ZOffset + 2 * fHalfSizeTPC_Z;   
            if (projX < XOffset)
                projX = XOffset;
            if (projX > XOffset + 2 * fHalfSizeTPC_X)
                projX = XOffset + 2 * fHalfSizeTPC_X;
            Int_t padID = (2 * fHalfSizeTPC_X * fSizeOfVirtualPad) * (Int_t)((projZ - ZOffset) * fSizeOfVirtualPad) +
                          (Int_t)((projX - XOffset) * fSizeOfVirtualPad);

            Int_t nProjPoints = fGTPCProjPoint->GetEntriesFast();
            for (Int_t pp = 0; pp < nProjPoints; pp++)
            {
                if (((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->GetVirtualPadID() == padID)
                {
                    // already existing R3BGTPCProjPoint... add time and electron
                    ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->AddCharge();
                    ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->SetTimeDistr(projTime / 1000, 1); // micros
                    virtualPadFound = kTRUE;
                    break;
                }
            }
            if (!virtualPadFound)
            {
                new ((*fGTPCProjPoint)[nProjPoints]) R3BGTPCProjPoint(padID, projTime, 1, evtID);
            }
            virtualPadFound = kFALSE;
        }
        xPre = xPost;
        yPre = yPost;
        zPre = zPost;
    }
    LOG(INFO) << "R3BGTPCLangevin: produced " << fGTPCProjPoint->GetEntries() << " projPoints";
}

void R3BGTPCLangevin::Finish() {}

ClassImp(R3BGTPCLangevin)
