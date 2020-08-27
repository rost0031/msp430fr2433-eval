This is a project that shows running of QPC 6.8.2 on MSP430FR2433 devkit. In order to compile:
release: make clean; make CONF=rel -j all
spy:     make clean; make CONF=spy -j all
dbg:     make clean; make CONF=dbg -j all

QSPY functions at 115200 baudrate on this build. While TX (output) works, the RX (input) does not. It had to be compiled out 
due to QS_rxParse() function is rather large and causes the build to not fit into the flash on the msp430fr2433.