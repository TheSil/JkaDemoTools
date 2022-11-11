This is code of the Demo Cutter tool created long time ago for the Jedi Academy demo files. In 2022 I've stumbled upon it on my drive and decided to put it on Github before it gets lost. However this project is no longer developed or maintained. The code is almost without change as it was when last the update was done in 2013, only changes done were to make it compilable with Qt 6.X.X and some quick integration with conan & cmake.

To properly download and build dependencies, conan package manager is used.

Make solution for release x64 version in VS 2022:

    conan install . --install-folder build --build=missing
    cmake . -G "Visual Studio 17 2022" -B build

Downloading and building all the dependencies wiill take some time. Solution file will be generated in build/JkaDemoTools.sln. Selecting the desired project and changing the build from Debug to Release compiles fine in VS2022.

Some things that were not done during conversion:
 - the icon is not used in the Qt 6, should be simple update if needed

Following is a summary of included components based on my recolection. Some of these tools were released on jkhub.org back in the days.

# Demo Manipulator
Common source files for tools used to parse the demo files.

# Demo Cutter
GUI Qt base application for cliping the demo file into shorter and smaller demo file, based on selected time range.

# Demo Smoother
Command-line tool that optimizes selected demo file for size as well as makes the players movements smoother (remove the "laggy" movements caused by network latency).



