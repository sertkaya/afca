# affca
Argumentation Framework using Formal Concept Analysis

AFFCA is a prototype for computing extensions of Abstract Argumentation 
Frameworks (AFs) using algorithms from Formal Concept Analysis (FCA). 

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
$ ./affca -p <problem> -f <input file> -a <argument>

<problem>       Currently supported: "DC-CO" 
<output file>   Name of the file for the results.
<input file>    Input argumentation framework in the format mentioned above.

optionally the algorithm can be specified using: 
$ ./affca -l <algorithm> -p <problem> -o <output file> -f <input file>

<algorithm>     One of the algorithms "cbo", "subgraph-cbo".
```
