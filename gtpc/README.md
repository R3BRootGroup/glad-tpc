Naming scheme for the classes:

R3BGTPC[function] e.g. R3BGTPCSetup, R3BGTPCLangevin

- R3BGTPC: 						it's the core of the simulation.
- R3BGTPCLangevin: 		Electron drift using the langevin equations.
- R3BGTPCProjector: 	Electron drift using a simple linear projector toward the pad plane.
- R3BGTPCSetup: 			necessary for the creation of the different HYDRA geometries, target and to choose the electronics. It's used to read the dimensions from a txt file.

Inputs folder cointains the file.txt where the setup dimensions are defined: everything it's in [cm] and [deg].
