# affca
Argumentation Framework using Formal Concept Analysis

AFFCA is a prototype for computing extensions of Abstract Argumentation 
Frameworks (AFs) using algorithms from Formal Concept Analysis (FCA). 
Currently it only supports computation of stable extensions. Support for
other types of extensions are on the way.

# Compiling
Download and extract the sources into the folder "affca" and execute the 
following commands on a UNIX-like operating system. You will need the automake
and the autoconf packages.

```
$ cd affca
$ aclocal 
$ autoconf
$ automake --add-missing
$ autoreconf
```
If everything works out, you should have a script called "configure".
Run this script:
```
$ ./configure
```

If this also works out you should have the necessary Makefiles. As next
execute:

```
$ make
```
Now affca should be compiled and available under the folder "src".

A statically compiled binary for Linux is provided under under the  "Releases"
section on GitHub.

# Running
AFFCA supports only the simplified index-based format newly introduced for ICCMA 2023 Competition. For details
of this format please see: https://iccma2023.github.io/rules.html#input-format

Usage:
```
$ ./affca -a <algorithm> -p <problem> -o <output file> -f <input file>

<algorithm>     One of the algorithms "next-closure" or "norris".
<problem>       Currently supported: "SE-ST" and "EE-ST"
<output file>   Name of the file for the results.
<input file>    Input argumentation framework in the format mentioned above.
```

# Test files
Test files for the submission to JELIA 2023 are available under:
https://drive.google.com/file/d/188zYTf68OSvdtBYdUCxeJ0Jq4s6xHwrD/view?usp=sharing

# Scripts
A bunch of scripts for processing the input frameworks are provieded under the "scripts" directory.
```
af2apx.py       Python script for converting the index-based format to the older apx-format.
apx2af.py       For conversion in the other direction.
analyze_apx.py  For creating stastical information like size and density from a framework in apx-Format.
create-af.py    Creates a test framework with a given size and density.
