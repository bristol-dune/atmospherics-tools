# ROOT CAF Read Project

## Overview

A macro to read Pandora reconstructed PFPs and write their properties on an event level to flat a ROOT tree.

## Dependecies

In order to read the reco PFP in the CAF file, the `StandardRecord` library must be compiled first. The `duneanasel` package is used as a wrapper.

To build and install the duneanasel, follow these steps:

1. **Conda environment**
    Log into `hm01.dice.priv` using ssh, and create or activate an oscillation analysis conda environment (the environment must have `ROOT`.

2. **Update the repository**:

    Get the new code from online repository
    ```sh
    git clone git@github.com:DUNE/duneanasel.git
    ```

3. **Build the project using CMake**:

    ```sh
    cd duneanasel
    mkdir build
    cd build
    cmake .. -Dduneanasel_USE_SRProxy=OFF -DDUNE_ANAOBJ_BRANCH=v03_12_00
    make
    ```

## ROOT environment

In order for the ROOT macro to find the StandardRecord header files, environment variables needs to be set as below.

1. **Set env variables**:
    ```sh
    export ROOT_INCLUDE_PATH=./duneanasel/build/_deps/duneanaobj-src/
    export LD_LIBRARY_PATH=./duneanasel/build/:$LD_LIBRARY_PATH
    ```

## Usage

### caf_reader.c macro

This macro iterates through all events in the CAF file and extracts Pandora Reco PFP information on an event level, and writes the new variables into a flat `ROOT` tree.

1. **Run CAF reader macro**:
`N.B. The CAF file path must be updated in the ROOT macro if a different CAF file is to be used.`
    ```sh
    cd ../..
    root -l caf_reader.c
    ```