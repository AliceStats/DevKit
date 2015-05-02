Alice Development Kit
---------------------

This is a development kit to be used alongside Alice.

The Devkit consists of an embedded web server that provides a clean interface to
all available Entities and Stringtables.
In addition some basic game information is included such as heroes picked, current
in-game time and the number of messages parsed.

![Image](https://raw.github.com/AliceStats/DevKit/master/doc/screenshot.png)

Requirements
------------

 - Linux or Mac OSX (Haven't tried building this on windows yet)
 - Boost 1.53+
 - NodeJS's `http_parser` library, included as a git submodule
 - CMake
 - Alice

Building
--------

 - Run `git submodule int && git submodule update`
 - Install(!) alice to a location of your choice. Compiling alone does not suffice.
 - Run `mkdir build && cd build`
 - Run `cmake .. -DALICE_ROOT=/absolute/paths/onlycmake -DCMAKE_INSTALL_PREFIX=../dist` (Absolute path!)
 - Run `make install`

License
-------

Apache 2.0
