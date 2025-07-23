# BalaGrayIter

**Balanced Gray code Iterator; custom solver that finds optimally balanced Gray orders of mixed radix permutations**

The BalaGray crawler generates balanced Gray [orderings](https://victimofleisure.github.io/atonal/BalaGraySetsTable.htm) of the 65 prime-form
[interval sets](https://www.chriskorda.com/atonal/IntervalSets.htm). The orderings also optimize maximum span length. The author
uses these orderings as chord progressions for atonal [music](https://victimofleisure.github.io/music.html). On a Dell P15
laptop, the crawler takes around fifteen minutes to create all the orderings.
Each interval set is written to its own text file. When all interval sets
have been processed, the program collects the most optimal orderings and
saves them in three formats: HTML, CSV, and PLM. The latter format is for
the author's [Polymeter MIDI Sequencer](https://victimofleisure.github.io/Polymeter/), which is freely available.

The source code is standard C++ and should compile cleanly in any modern environment.

## Citation

This software is connected to a [research paper](https://doi.org/10.5281/zenodo.16369463). If you use BalaGray in your research, please cite:

Korda, C. (2025). *Granular 12-Tone Harmony via Mixed-Radix Balanced Gray Codes:  
Interval Set Partitions and the BalaGray Solver*. [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.16369463.svg)](https://doi.org/10.5281/zenodo.16369463)
