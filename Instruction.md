The steps to properly run the simulation.
1. Create all the necessary files to run the code:
	a- The particle generator needs to be created, to do so go in the folder /glad-tpc/gtpgen/ and run the macro INCL_Background_ASCIIGenerator.cc to generate the bkg or PhaseSpaceDecay_hypertriton_ASCIIGenerator.cc to generate the hypertriton decay. The instructions are inside the macros. The output of the macros could be found in the folder /glad-tpc/gtpgen/ASCII/ .
	b- The geometry of the detector you want to use should be already in the folder /glad-tpc/geometry/ if it is not the case, go in the folder /glad-tpc/macros/geo/ and run the macro for the geometry you want.
2. How to run the simulation, in the folder /glad-tpc/macros/sim there are 2 macros:
	a- simHYDRA.C		-> Needs the files created in step 1 to produce the events.
	b- run_simHYDRA	-> It requires the definition of the number of events, geometry and generator you want to use.
	In output will produce par.root, sim.root and two .png figures (these to check that the chamber is well placed).
3. Check the simulation output, in the folder of the geometry chosen ,e.g. Prototype, there are the following macros:
	a- eventDisplay.C -> To see the geometry and the particles event by event. Be careful, close it from the terminal (.q).
	b- checkAll.C			-> Checks the primary, Points and Hits characteristics.
4. Electron drift, in the folder /glad-tpc/macros/proj/ there are several macros, the important ones are:
	a- run_proj.root 		-> Simple projection of the tracks onto the pad planes, produce in output the file proj.root.
	b- readProjPoints.C	-> General macro for checking the projPoints from the projector.
5. Visualization of the pad plane, in the folder /glad-tpc/macro/vis there is the macro readVPadPlane.C: This macro plots the output of the glad-tpc projector: plots the R3BGTPCProjPoint which contains the virtual pads calculated after the projection of the track.
