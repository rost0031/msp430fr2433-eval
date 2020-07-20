For MSP430 CPU generation 1 and 2, we recommend to add the following additional compiler flags:
    -mlarge -mcode-region=either -mdata-region=lower
To check if your device is CPU generation 1 or 2, open the devices.csv file (${INSTALL_DIR}/include) and look up the column CPU_TYPE.
For MSP430 CPU generation 0, those flags should not be added to the Makefile.
