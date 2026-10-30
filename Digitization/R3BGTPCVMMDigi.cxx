#include "R3BGTPCVMMDigi.h"

#include <iostream>

void R3BGTPCVMMDigi::Print(const Option_t*) const
{
    std::cout << "R3BGTPCVMMDigi  pad " << fPadId << "  ADC " << fADC << "  time " << fTimeNs << " ns"
              << (fSaturated ? "  SATURATED" : "") << (fNeighbor ? "  NEIGHBOUR" : "") << (fNoise ? "  NOISE" : "")
              << std::endl;
}

ClassImp(R3BGTPCVMMDigi)
