This is code of the Demo Cutter tool created long time ago for the Jedi Academy demo files. In 2022 I've stumbled upon it on my drive and decided to put it on Github before it gets lost. However this project is no longer developed or maintained. The code is almost without change as it was when last the update was done in 2013, only changes done were to make it compilable with Qt 6.X.X and some quick integration with conan & cmake.

To properly download and build dependencies, conan package manager is used.

Make solution for release x64 version in VS 2022:

    conan install . --install-folder build --build=missing
    cmake . -G "Visual Studio 17 2022" -B build

Downloading and building all the dependencies wiill take some time. Solution file will be generated in build/JkaDemoTools.sln. Selecting the DemoCutter project and changing the build from Debug to Release compiles fine in VS2022.


Common demo manipulation tools were stored in DemoManipulator folder from which it was included in other projects such as DemoCutter.  

Some things that were not done during conversion:
 - the icon is not used in the Qt 6, should be simple update if needed
 - I haven't included other minor tools such as Demo Smoother or Chat extractor, I might add that in the future


