// --- General Media Defs. for TPC
//----------------------------------------------------------
// Source: http://fairroot.gsi.de/?q=node/34
//
// The following parameters are needed:
//
// * int ncomp       - number of components in the material (ncomp= 1 for a basic material and <1 or >1 for a mixture
//                     If ncomp > 0 the array wm contains the proportion by weight of each material in the mixture.
//                     If ncomp < 0 the array wm contains the proportion by number of atoms of each kind.
//                     For more detailed information see NLMAT in Geant3 routine GSMIXT
// * float aw[ncomp] - atomic weights A for the components
// * float an[ncomp] - atomic numbers Z for the components
// * float dens      - density DENS in g cm(**-3)
// * float wm[ncomp] - weights WMAT of each component in a mixture (only for a mixture)
// * int   sensflag  - sensitivity flag ISVOL
// * int   fldflag   - fieldflag IFIELD
// * float fld       - maximum field value FIELDM in kilogauss
// * float epsil     - boundary crossing precision EPSIL
// * int   npckov    - number of values used to define the optical properties of the medium.
//
// The variable npckov is 0 for all media except some special media used for the
// Rich where the tracking of the Cerenkov photons is necessary. These media have
// additinal parameters
//
// * float ppckov[npckov] - photon momentum in eV
// * float absco[npckov]  - absorption length in case of dielectric and of absorption  probabilities in case of a metal
// * float effic[npckov]  - detection efficiency
// * float rindex[npckov] - refraction index for a dielectric, rindex[0]=0 for a metal
//
// Remark: In the present program version a mixture may contain a maximum of 5
// components. If this is not sufficient one has to change MAXCOMP in
// hgeomedium.h.
//
// The following parameters are normally not read. The default values are -1 and
// the real values are automatically calculated by Geant. If you want to set these
// values by yourself, you must type the keyword AUTONULL in your media file.
// After this keyword all media must contain these additional 4 parameters before
// the Cerenkov (int npckov).
//
// * float madfld  - maximum angular deviation TMAXFD due to field
// * float maxstep - maximum step permitted STEMAX (CM)
// * float maxde   - maximum fractional energy loss DEEMAX
// * float minstep - minimum value for step STMIN
//
//
// example)
//
// air 3 14.01 16. 39.95.  7.  9.  18. 1.205e-3  .755  .231  .014
// 0 1 3.  .001
// 0
//
//----------------------------------------------------------
Air                3  14.01  16.  39.95  7.  8.  18.  1.205e-3  .78 .21  .01
                   1  1  20.  .001
                   0

air                3  14.01  16.  39.95  7.  8.  18.  1.205e-3  .78 .21  .01
                   1  1  20.  .001
                   0

vacuum             1  1.e-16  1.e-16  1.e-16
                   0  1  20.  .001
                   0

LH2                1  1.01   1.   0.0715
                   1  1  20.  .001
                   0

CH2                2  1.00794  12.0107  1.  6.  1.0  0.857  0.143
                   0  1  20.  .001
                   0

H2                 1  1.007940  1.  0.070800
                   0  1  20.  .001
                   0

carbon             1  12.011  6.0  2.265
                   0  1  20.  .001
                   0

gold               1  196.97  79.  19.3
                   0  1  20.  .001
                   0
lead               1  207.1  82.  11.34
                   0  1  20.  .001
                   0
silicon            1  28.0855 14.0 2.33
                   1  1  20.  .001
                   0
aluminium          1  26.98 13. 2.7
                   0  1  20.  .001
                   0
magAl		   1  26.98 13. 2.7
                   1  1  20.  .001
                   0
beryllium          1  9.01  4.0 1.848
                   0  0  20.  .001
                   0

iron               1  55.847  26.  7.87
                   1  1  20.  .001
                   0

mylar              3   12.01   1.01   16.0   6.   1.   8.   1.397   0.624935   0.0420439   0.333021
                   1  1  20.  .001
                   0

rohacell           4  12.01   1.01   16.0    14.01  6.   1.   8.   7.  0.075   0.6014  0.0805  0.3154  0.00276
                   1  1  20.  .001
                   0

kapton             4  12.01   1.01   16.0    14.01  6.   1.   8.   7.  1.42   0.691133  0.026362  0.209235  0.073270
                   1  1  20.  .001
                   0

inox               3  12.01   55.9   52.0   6.   26.   24.    8.02   0.001   0.829   0.17
                   1  1  20.  .001
                   0

CF4                2  12.01  19.01  6.  9.  0.0036586  0.1364   0.8636
                   0  1  20.  .001
                   0

iso                2  12.01   1.01  6.  1.  0.002506  0.8263   0.1737
                   0  1  20.  .001
                   0


Ar                 1  39.948   18.  0.0016581
                   1  1  20.  .001
                   0


mix                4  12.01   19.0   1.01   39.948   6.  9.   1.   18.   0.0019836   0.0452577   0.129531   0.005211558   0.82
                   1  1  20.  .001
                   0


NaI                2  22.989770  126.904470  11.  53.  3.670  0.15337  0.84663
                   0  1  20.  .001
                   0


BC408              2  1.00794  12.0107  1.  6.  1.032  0.084  0.916
                   1  1  20.  .001
                   0

copper             1  63.54  29.  8.96
                   0  0  20.  .001
                   0
G10                4  12.01 1.008 16. 28.09 6.  1.  8.  14. 1.7 0.259
0.288 0.248 0.205
                   0  0  20.  .001
                   0
polypropylene	   -2	12.01 1.008 6. 1. 0.074 3  6
                   0  0  20.  .001
                   0

carbondioxide      -2  12.01  16.0  6. 8. 1.9768e-3 1  2
                   0  0  20.  .001
                   0
goldcoatedcopper   2  63.54  196.97 29.  79.  9.0634  .99 .01
                   0  0  20.  .001
                   0

CarbonFibre        3  12.010700  1.007940  15.999400  6.000000  1.000000  8.000000  1.690000  0.844907  0.042543  0.112550
                   1  1  20.  0.001
                   0

LaCl               4  138.91  35.453  35.453  35.453  57.00  17.00  17.00  17.00  3.79  0.566  0.144  0.144  0.144
                   1  1  20.  0.001
                   0

LaBr               4  138.91  79.909  79.909  79.909  57.00  35.00  35.00  35.00  5.29  0.366  0.211  0.211  0.211
                   1  1  20.  0.001
                   0

Tefflon            2  12.0107  18.9994  6.00  9.00  2.2  0.333  0.667
                   1  1  20.  0.001
                   0

plasticForGFI      2  12.0107  1.007940  6.00  1.00  1.0320  0.914708  0.085292
                   1  1  20.  0.001
                   0

plasticForTOF      2  12.0107  1.007940  6.00  1.00  1.0320  0.914708  0.085292
                   1  1  20.  0.001
                   0

plasticFormTOF     2  12.0107  1.007940  6.00  1.00  1.0320  0.914708  0.085292
                   1  1  20.  0.001
                   0

PbWO               3  207.19  183.85  16.0  82.0  74.0  8.0  8.28  0.4553  0.4040  0.1407
                   1  1  20. 0.001
                   0

RPCglass           -2  28.09  16.  14.  8.  2.55  1.  2.
                   0  0  20.  .001
                   0

CsI               -2  132.9054  126.9045 55.  53. 4.53  0.511549  0.488451
                  1  1  20. .00001
             	  2
		  1.77       50000.            1.0        1.0003
		  10.5       50000.            1.0        1.0003

PMTglass          -2  28.09  16.  14.  8.  2.2  1.  2.
                   0  1  20.  .00001
                   2
                   1.77     50000.         1.0        1.45
                   10.5     50000.         1.0        1.45

PMTvacuum          3  14.01  16.  39.95  7.  8.  18.  1.205e-10  .78   .21  .01
                   0  1  20.  .001
                   2
                   1.77     100000.         1.0        1.0
                   10.5     100000.         1.0        1.0

freon10            -3  12.01 1.008  19.  6. 1. 9. 3.75e-3  2  2  4
                   0  0  20.  .001
                   0
isobutan           -2  12.01 1.008  6.  1.  2.7e-3  4  10
                   0  0  20.  .001
                   0

sulfurhexafluoride -2  32.06 19. 16. 9. 2.7e-3 1 6
                   0  0  20.  .001
                   0

RPCgas             4  12.01  1.008  19.0  32.06  6. 1. 9. 16.  3.75E-3
.227 .248 .511 .014
                   1  0  20.  .001
                   0

copper             1  63.54  29.  8.96
                   0  0  20.  .001
                   0
G10                4  12.01 1.008 16. 28.09 6.  1.  8.  14. 1.7 0.259
0.288 0.248 0.205
                   0  0  20.  .001
                   0
polypropylene	   -2	12.01 1.008 6. 1. 0.074 3  6
                   0  0  20.  .001
                   0

carbondioxide      -2  12.01  16.0  6. 8. 1.9768e-3 1  2
                   0  0  20.  .001
                   0
goldcoatedcopper   2  63.54  196.97 29.  79.  9.0634  .99 .01
                   0  0  20.  .001
                   0
//After AUTONULL the material definition has to include additional parameters
//	name 	ncomp 	aw(1)[A1] 	aw(2)[A2] 	an(1)[Z1] 	an(2)[Z2] 	dens[g/cm^3] 	wm(1)[%] 	wm(2)[%]
//	sensflag[bool] 	fldflag[bool] 	fld[kG] 	epsil[precision]
//	madlfd 	maxstep[cm] 	maxde[%] 	minstep[cm]
//	npckov
AUTONULL

//P10 at 1atm. maxstep=150um and minstep=0.01 um
     
P10								 3	12.011	1.008	39.95 6.	1.	18.	1.74e-3		 0.0623			0.0155			0.9222
									 1			1					20.				0.001
                   99. 		0.15   	 	0.001 		0.000001
									 0
