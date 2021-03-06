# waver
Create WAV files from a Disc At Once (DAO) data stream.

## Prerequisites
1. GNU C Compiler
2. GNU Make
3. GNU Install

## Install
1. `cd` into the "src" directory.
2. Run the command: `make install` (as root or sudoer). The binary will be installed to /usr/local/bin

## Uninstall
1. `cd` into the "src" directory.
2. Run the command: `make uninstall` (as root or sudoer).

## Example of grabbing data and creating WAV files from an AUDIO CD
### Prerequisites
1. Package: cdrdao
  1. Package: toc2cue (is part of cdrdao)
2. waver (or bchunk if you have more time)

### Procedure
1. Create bin and toc file: `cdrdao read-cd --device /dev/sr0 --driver generic-mmc --paranoia-mode 3 foo.toc` Modify the device file according to your system if necessary. This will create the files: data.bin and foo.toc
2. Convert the table of contents file to a cue file: `toc2cue foo.toc foo.cue`
3. Create WAV files from the dao stream: `waver -b data.bin -c foo.cue -n output_wav -s` This will create the WAV files according to the track information contained in the cue file. -s swaps the bytes of the data stream.

## waver performance test results
| bin file size [byte]  | n threads  | elapsed [s]  | sys [s] | user [s] | speedup [ T(1)/T(P) ] |
|---|---|---|---|---|---|
| 620M  | 1  | 2.34  | 0.47 | 1.85 | - |
| 620M  | 2  | 1.34  | 0.70 | 1.91 | 1.75 |
| 620M  | 4  | 1.14  | 0.81 | 2.90 | 2.05 |
| 620M  | 8  | 0.85  | 0.89 | 2.57 | 2.75 |
Performed on: CPU: "Intel(R) Core(TM) i7 CPU 930 @ 2.80GHz", RAM: 24 GiB, HDD: Kingston SSD. data written on tmpfs (/tmp)

## Credits
* **Heikki Hannikainen** \<hessu\|at\|hes.iki.fi\> For sharing the sources of his "bchunk", which served important informations for this implementation.
* **Markus Thaler** \<tham\|at\|zhaw.ch\> For the cpuinfo header to determine the no of CPUs available on a machine and also his famous timer api "mtimer" to get timer values from the kernel.

