# Ventilator Panel Software

This is the software required to run the VITAL ventilator panel software. For a full-description please see
the accompaning software documentation document (SDD). Is should be procided alongside this software.

## Build Instructions

This code should be built with the STM32 Cube IDE. Import the project and build using the integrated tools.

## Unit Tests

This package is provided with unit tests that ensure the code is functioning correctly. These unit tests build and
run outside the STM32 cube IDE in a standard unix-like enviornment. They were tested on Mac OSX using llvm. They
should also function with gcc on Linux.

Run using the below instructions and look for the "ALL SUCCESS" message output to know they all passed. These
tests will fast-fail if an error is detected printing the line that failed where possible.

Building and running unit tests:

```
cd Test
make
```

Cleaning unit tests:

```
cd Test
rm -r bin
```
