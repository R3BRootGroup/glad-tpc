# GLAD-TPC Software [![license](https://img.shields.io/badge/License-GPLv3-blue.svg)](COPYRIGHT) [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.14291537.svg)](https://doi.org/10.5281/zenodo.14291537)

[![static analysis](https://github.com/R3BRootGroup/glad-tpc/actions/workflows/static_analysis.yml/badge.svg)](https://github.com/R3BRootGroup/glad-tpc/actions/workflows/static_analysis.yml) [![CI-CD](https://github.com/R3BRootGroup/glad-tpc/actions/workflows/main.yml/badge.svg)](https://github.com/R3BRootGroup/glad-tpc/actions/workflows/main.yml)
[![dashboard](https://img.shields.io/badge/dashboard-glad--tpc-blue?labelColor=gray&style=flat)](https://cdash.gsi.de/index.php?project=R3BRoot)

## The GLAD-TPC Framework

The GLAD-TPC (Time Projection Chamber) detector, also known as HYDRA, is part of the R3B (Reactions with Relativistic Radioactive Beams) experimental setup at the GSI/FAIR research center (Facility for Antiproton and Ion Research). The GLAD-TPC software allows for Monte Carlo simulations and experimental data analysis, enabling seamless integration with the data analysis workflows of other R3B detectors within the [R3BRoot](https://github.com/R3BRootGroup/R3BRoot) software. GLAD-TPC software is a source distribution with recurring releases for MacOS and Linux.

## Discussion Forum

For the software-related user support you can post a new topic on our [forum](https://forum.gsi.de/index.php?t=index&cat=40&).

## License

GLAD-TPC is distributed under the terms of the GNU Lesser General Public Licence version 3 ([LGPLv3](https://github.com/R3BRootGroup/glad-tpc/blob/dev/LICENSE)).

## Release Information
Please visit [releases](https://github.com/R3BRootGroup/glad-tpc/releases)

## Contributing
Please ask your questions, request features, and report issues by [creating a github issue](https://github.com/R3BRootGroup/glad-tpc/issues/new/choose).

## Using the Project Template

GLAD-TPC as part of R3BRoot delivers various project templates that can be used as a starting point for anybody who would like to build simulation and reconstruction algorithms. The project Templates are located in the [R3BRoot/template](https://github.com/R3BRootGroup/R3BRoot/tree/dev/template) directory.

## Code Formatting
The GLAD-TPC project (as part of R3BRoot) uses clang-format-16 to ensure a common code formatting. The script "apply-format.sh" can be used for this purpose: 
~~~bash
. apply-format.sh
~~~

## Step by Step Installation

### Required software

First, you will need to install FairSoft, FairRoot and R3BRoot. For more details: [R3BRoot README](https://github.com/R3BRootGroup/R3BRoot/blob/dev/README.md). 
- FaiSoft version: nov22p1 or later
- FairROOT version: v18.2.1 or later
- R3BROOT version: dec25 or later

### Configure and compile

Follow the instructions in the [R3BRoot README](https://github.com/R3BRootGroup/R3BRoot/blob/dev/README.md).

## Simulations

For the general R3BRoot simulations, follow the instructions in [R3BRoot README](https://github.com/R3BRootGroup/R3BRoot/blob/dev/README.md).

The steps to properly run the glad-tpc simulation:

1. Create all the necessary files to run the code:
	1. The particle generator needs to be created, to do so go in the folder `/glad-tpc/gtpgen/` and run the macro `INCL_Background_ASCIIGenerator.cc` to generate the bkg or `PhaseSpaceDecay_hypertriton_ASCIIGenerator.cc` to generate the hypertriton decay. The instructions are inside the macros. The output of the macros could be found in the folder `/glad-tpc/gtpgen/ASCII/`
	2. The geometry of the detector you want to use should be already in the folder `/glad-tpc/geometry/` if it is not the case, go in the folder `/glad-tpc/macros/geo/` and run the macro for the geometry you want.

2. How to run the simulation, in the folder `/glad-tpc/macros/sim` there are 2 macros:
	1. `simHYDRA.C`: Needs the files created in step 1 to produce the events.
	2. `run_simHYDRA`: It requires the definition of the number of events, geometry and generator you want to use.
	In output will produce `par.root`, `sim.root` and two `.png` figures (these to check that the chamber is well placed).

3. Check the simulation output, in the folder of the geometry chosen ,e.g. Prototype, there are the following macros:
	1. `eventDisplay.C`: To see the geometry and the particles event by event. Be careful, close it from the terminal (.q).
	2. `checkAll.C`: Checks the primary, Points and Hits characteristics.

4. Electron drift, in the folder `/glad-tpc/macros/proj/` there are several macros, the important ones are:
	1. `run_proj.root`: Simple projection of the tracks onto the pad planes, produce in output the file `proj.root`.
	2. `readProjPoints.C`: General macro for checking the projPoints from the projector.

5. Visualization of the pad plane, in the folder `/glad-tpc/macro/vis` there is the macro `readVPadPlane.C`: This macro plots the output of the glad-tpc projector: plots the `R3BGTPCProjPoint` which contains the virtual pads calculated after the projection of the track.

6. Electronics response, in the folder `/glad-tpc/macro/electronics` there is the macro `Electronics_MT.C`: This macro simulate the electronics response.

## How to run the simulations

It is possible to use the bash script `run_full.sh` to run all these steps at once. BE CAREFUL, set first the wanted parameters in the different folders.


## Data Analysis

> ...Under development...

## Spell Checking

To avoid common typos in the codebase and documentation, we use [codespell](https://github.com/codespell-project/codespell).

Check for spelling mistakes using the ignore list defined in `.codespell-ignorewords`:

~~~bash
codespell -I .codespell-ignorewords
~~~

If you want to automatically fix the detected typos:

~~~bash
codespell -w -I .codespell-ignorewords
~~~

## Tested systems

The following systems are tested regularly.

| **OS Name** | **Arch** | **OS Version** | **Compiler**  | **CMake**       | **C++ Version** |
| ----------- | -------- | -------------- | ------------- | --------------- | --------------- |
| Almalinux   | x86\_64  | 9.3            | GCC 11.4.1    | 3.27.9 / 4.0.3  | C++17 / C++20   |
| Almalinux   | x86\_64  | 9.4            | GCC 14.2.0    | 3.30.6          | C++17           |
| RHEL        | x86\_64  | 9.6            | GCC 14.2.0    | 3.30.6          | C++17           |
| Debian      | x86\_64  | 11             | GCC 10.2.1    | 3.27.4 / 3.30.0 | C++17           |
| Debian      | x86\_64  | 12             | GCC 12.2.0    | 3.27.4 / 3.30.0 | C++17 / C++20   |
| Debian      | x86\_64  | 13             | GCC 14.2.0    | 3.31.6 / 4.0.3  | C++17           |
| Ubuntu      | x86\_64  | 24.04          | GCC 13.3.0    | 3.28.3 / 4.0.3  | C++17 / C++20   |
| Ubuntu      | x86\_64  | 25.04          | GCC 14.2.0    | 3.31.6 / 4.0.3  | C++17           |

## More Information

* [Static analyzer using Clang-tidy](https://github.com/R3BRootGroup/R3BRoot/blob/dev/config/clang_tidy/README.md)
* [CMake build system for R3BRoot/glad-tpc](https://github.com/R3BRootGroup/R3BRoot/blob/dev/doc/cmake_usage.md)
* [How to use an unmerged pull request](https://github.com/R3BRootGroup/R3BRoot/blob/dev/doc/git_usage.md#fetch-the-update-from-an-unmerged-pull-request-pr)
