jumpdir
====================================================================================================

`jumpdir` is a heuristic directory change command that learns about your environment as you navigate
from directory to directory (including network-mapped directories). It's based on a tool I wrote
many years ago that was quite successful (`setdir`), but for various reasons I'm re-writing it from
the ground up.

It is currently in a very rough preliminary state, having undergone multiple false starts. In
particular, I am trying to work out the best way to achieve proper Unicode handling with
cross-platform capability. Both of these, I think, are fairly settled now, particularly with C++'s
new `std::filesystem` as a way to navigate filesystems on many platforms.

Before re-engaging in this project, I'm working to finalize the transition of [pathmatch][] to use
`std::filesystem`. In addition, I may choose to adopt [LSON][] as my data storage format, which will
also delay things quite a bit.


Building
----------
This project uses the CMake build tool. CMake is a meta-build system that locates and uses your
local development tools to build the project if possible.

To build, first install [CMake][https://cmake.org/]. Then go to the project root directory and run
the following command:

    cmake -B build

This will locate your installed development tools and configure your project build in the `build/`
directory. After that, whenever you want a new build, run this command:

    cmake --build build

This will build a debug version of the project, located in `build/Debug/`. To build a release
version, run

    cmake --build build --config release

You can find the built release executable in `build/Release/`.



----
Steve Hollasch <steve@hollasch.net>



[pathmatch]: https://github.com/hollasch/pathmatch
[LSON]:      https://github.com/hollasch/lson
