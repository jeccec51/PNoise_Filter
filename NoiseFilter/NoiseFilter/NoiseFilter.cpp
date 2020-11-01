// NoiseFilter.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "NoiseFilter.h"


// This is an example of an exported variable
NOISEFILTER_API int nNoiseFilter=0;

// This is an example of an exported function.
NOISEFILTER_API int fnNoiseFilter(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
CNoiseFilter::CNoiseFilter()
{
    return;
}
