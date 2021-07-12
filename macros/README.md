# Structure

The structure of the macros folder is the following:
* `[geo]`: contains the macros to produce the geometries of HYDRA: prototype, FullBeamOut, FullBeamIn. These geometries are stored in the folder `../geometries`.
* `[sim]`: contains the macros to run the simulation. `simHYDRA.C` (and `run_simHYDRA`) will produce 2 root files: [par.root] and [sim.root], this contains information about the parameters of the simulations and the particles information, respectively. To do so it requires as input the generator that is stored in the folder `../gtpgen/ASCII`.
* `[proj]`: contains the macros to calculate the electron drift. To do so it requires as input the files produced in the sim folder and will produce in output the file `proj.root` that contains the information about the pad plane and the electron drift. The drift can be done in 2 ways: simple projection(`run_proj.C`), Langevin equation(`run_lang.C`||`run_lang_test`).
* `[vis]`: contains the macros to visualize the projection of the particles drift onto the pad plane. To do so it requires the file `proj.root`.
* `[electronics]`: contains the macro to process the drifted primary electrons with the AGET electronics(`run_ele.sh`).
*	`[Analysis]`: contains the macro to analyse the data from the simulation-> under development
