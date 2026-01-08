# Fitter

This class generates an executable that works using fitter. To its compilation, proceed as follows:

1. Download GenFit from the GitHub repository:

```bash
git clone https://github.com/GenFit/GenFit.git
```

2. Compile, install and export the corresponding path:

```bash
(from GenFit)

mkdir build; cd build
cmake .. -DCMAKE_INSTALL_PREFIX=..
cmake --build .
cmake --install .

export GENFIT=/path/to/GenFit
```

3. Recompile R3BRoot/glad-tpc installation (this will generate the dictionaries of some libraries that need GenFit).

4. From the fitter directory, compile the executable:
```bash
mkdir build; cd build
cmake ..
make
```
