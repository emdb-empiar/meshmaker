meshmaker: Generating VTK PolyData from CCP4 Files
======================================================

Please following the following steps to get ``meshmaker`` working.

Install CMake
------------------------------

If you have Homebrew installed:

.. code:: bash

	brew install cmake

Without Homebrew, download and install CMake from `the CMake website. <https://cmake.org>`__
	
Install VTK
------------------------------

With Homebrew:

.. code:: bash

	brew install vtk

Without Homebrew, download and build from `source. <https://www.vtk.org/download/>`__
	
Obtain source and prepare for build
------------------------------------------------------------

Clone ``meshmaker`` and enter ``meshmaker`` directory:

.. code:: bash

	user@mac ~ $ git clone https://github.com/emdb-empiar/meshmaker.git
	Cloning into 'meshmaker'...
	remote: Counting objects: 9, done.
	remote: Compressing objects: 100% (9/9), done.
	remote: Total 9 (delta 1), reused 5 (delta 0), pack-reused 0
	Unpacking objects: 100% (9/9), done.
	user@mac ~ $ cd meshmaker
	user@mac ~ $ mkdir build
	user@mac ~ $ cd build
	
Generate the ``Makefile``
------------------------------

.. code:: bash
	
	user@mac ~ $ cmake ..
	-- The C compiler identification is AppleClang 8.1.0.8020042
	-- The CXX compiler identification is AppleClang 8.1.0.8020042
	-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc
	-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc -- works
	-- Detecting C compiler ABI info
	-- Detecting C compiler ABI info - done
	-- Detecting C compile features
	-- Detecting C compile features - done
	-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
	-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ -- works
	-- Detecting CXX compiler ABI info
	-- Detecting CXX compiler ABI info - done
	-- Detecting CXX compile features
	-- Detecting CXX compile features - done
	-- Configuring done
	-- Generating done
	-- Build files have been written to: /Users/pkorir/Downloads/meshmaker/build

Build
------------------------------

.. code:: bash

	user@mac ~ $ make
	Scanning dependencies of target meshmaker
	[ 50%] Building CXX object CMakeFiles/meshmaker.dir/meshmaker.cpp.o
	[100%] Linking CXX executable meshmaker.app/Contents/MacOS/meshmaker
	[100%] Built target meshmaker

The executable for MacOSX is in the meshmaker.app/Contents/MacOS/ folder. You can use it directly like so:

.. code:: bash

	user@mac ~ $ meshmaker.app/Contents/MacOS/meshmaker -h
	usage: meshmaker [options] file.map
	
	Generate a mesh from the MAP/MRC file using the specified options
	
	Options:
		-c/--clevel <float>
				the contour level at which to build the surface [default: 0.0]
		-o/--output <str>
				the prefix of the output file to be combined with the extension (see below) [default: out]
		-S/--stl	output in STL format
		-V/--vtk	output in VTK format
		-X/--vtp	output in VTP format [default]
		-D/--decimate	perform progressive decimation to eliminate superfluous polygons [default: false]
		-t/--target-reduction <float>
				set the target reduction in the number of polygon in interval (0, 1) [default: 0.9]
		-A/--ascii	save data as ASCII as opposed to BINARY [default: false]
		-U/--uint64	save VTP headers using UInt64 as opposed to UInt32 [default: false]
		-I/--int32	user Int32 for vtkIdType instead of Int64 [default: false]
		-h/--help	show this help
		-v/--verbose	verbose output
	
	Abort trap: 6
	
*Optional*: Install
------------------------------

.. code:: bash

	make install

To specify a custom install prefix run

.. code:: bash

	user@mac ~ $ ccmake ..
	
and modify the ``CMAKE_INSTALL_PREFIX`` variable.


