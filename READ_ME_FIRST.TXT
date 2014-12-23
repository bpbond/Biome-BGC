This is the BiomeBGC 4.2 (Final Release) source code release.

New in this version are command line arguments, ascii output and an 
external nitrogen deposition file.

No Science changes have been made in this version. However, the co2
input file is now correctly read. The model now looks up the co2 entry
for the model year entry correclty. In pervious versions, the co2
entries were simply read starting with the first entry in the co2 file.

Other than that, Backward compaitibility has been preserved. If your 
BiomeBGC 4.1.x ini files and input files do not work, please email:

Andrew Neuschwander: andrew@ntsg.umt.edu.

BUILDING ON WINDOWS WITH VISUAL STUDIO:
BiomeBGC can now be build with Visual Studio (Tested with VS2003).
Use the "BiomeBGC for Windows.sln" solution file in 
src/Visual Studio/bgclib
This will build a pointbgc.exe win32 command line application.

BUILDING ON UNIX or LINUX:
You need an ansi C compiler and 'make'. Go into the src directory,
edit the makefile to suit your platform and compiler. Then type
'make' or 'make test'. The resulting binary, 'bgc' will be placed
in the top level directory.

* For information on command line usage, see USAGE.TXT

* For information on source code changes, see CHANGES.TXT

* For information on who has contributed to this model, see CREDITS.TXT

* For copyright and distribution information, see copyright.txt
