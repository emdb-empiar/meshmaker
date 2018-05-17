/*
 * meshmaker
 *
 * Generate an StL mesh from an MRC/MAP file at some contour
 *
 * Usage: meshmaker file.map [<clevel> [<outfile.stl>|<outfile.vtk>]]
 *
 * Author: Paul K. Korir, PhD
 * Email: pkorir@ebi.ac.uk, paul.korir@gmail.com
 * Date: 2016-09-26
 * License: Apache
 *
 * Version History:
 * 2016-09-26 - 0.1: basic version outputs either STL or VTK
 * 2017-03-22 - 0.2: basic version outputs VTP (XML)
 * 2018-05-14 - 0.3: add a smoothing and triangle-strip step to reduce VTP files
 */

// standard headers
#include <exception>
#include <cstdlib>

// VTK headers
#include "vtkSmartPointer.h"
#include "vtkMRCReader.h"
#include "vtkContourFilter.h"
#include "vtkTriangleFilter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkDecimatePro.h"
#include "vtkStripper.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkSTLWriter.h"
#include "vtkPolyDataWriter.h"

using namespace std;

// types
struct args {
	float clevel = 0.0;
	string out_fn = "out";
	string map_fn = "";
	string out_format = "vtp";
	int decimate = 0; // don't decimate by default
	int smooth = 0; // smoothen
	int smooth_iter = 20; // number of smoothing iterations
	float target_reduction = 0.9;
	int ascii = 0; // output not ASCII but BINARY (if = 1 then ASCII)
	int uint64 = 0; // headers of vtp are NOT in uint64 but in uint32
	int int32 = 0; // vtkIdType used are 64 bit instead of 32 bit 
	int verbose = 0; // do not show verbose output
	string out_fn_full = "";
}; 

void print_usage(void) {
string usage_string = "\
usage: meshmaker [options] file.map\n\
\n\
Generate a mesh from the MAP/MRC file using the specified options\n\
\n\
Options:\n\
\t-c/--clevel <float>\n\t\t\tthe contour level at which to build the surface [default: 0.0]\n\
\t-o/--output <str>\n\t\t\tthe prefix of the output file to be combined with the extension (see below) [default: out]\n\
\t-S/--stl\toutput in STL format\n\
\t-V/--vtk\toutput in VTK format\n\
\t-X/--vtp\toutput in VTP format [default]\n\
\t-D/--decimate\tperform progressive decimation to eliminate superfluous polygons [default: false]\n\
\t-s/--smooth\tsmooth the generated surface [default: false]\n\
\t-i/--smooth-iter <int>\n\t\t\tnumber of iterations for smoothing (only applies if -s/--smooth is specified[default: 20]\n\
\t-t/--target-reduction <float>\n\t\t\tset the target reduction in the number of polygon in interval (0, 1) [default: 0.9]\n\
\t-A/--ascii\tsave data as ASCII as opposed to BINARY [default: false]\n\
\t-U/--uint64\tsave VTP headers using UInt64 as opposed to UInt32 [default: false]\n\
\t-I/--int32\tuse Int32 for vtkIdType instead of Int64 [default: false]\n\
\t-h/--help\tshow this help\n\
\t-v/--verbose\tverbose output\n";
	cerr << usage_string << endl;
}

// parse command-line arguments
struct args parse_args(int argc, char **argv) {
	struct args cargs;
	int i = 1;
	int _abort = 0;
	
	while (i < argc) {	
		// clevel	
		if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--clevel") == 0) {
			try {
				cargs.clevel = stof(argv[i+1]);				
			}
			catch (exception& e) {
				cerr << "exception caught: " << e.what() << endl;
				_abort = 1;
			}
			i += 2;		
		}
		// output prefix
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
			cargs.out_fn = argv[i+1];
			i += 2;
		}
		// output format: STL
		else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--stl") == 0) {
			cargs.out_format = "stl";
			i++;
		}
		// output format: VTK
		else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--vtk") == 0) {
			cargs.out_format = "vtk";
			i++;
		}
		// output format: VTP (XML)
		else if (strcmp(argv[i], "-X") == 0 || strcmp(argv[i], "--vtp") == 0) {
			cargs.out_format = "vtp";
			i++;
		}
		// decimate the mesh
		else if (strcmp(argv[i], "-D") == 0 || strcmp(argv[i], "--decimate") == 0) {
			cargs.decimate = 1;
			i++;
		}
		// smooth the mesh
		else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--smooth") == 0) {
		    cargs.smooth = 1;
		    i++;
		}
		// smooth iterations
		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--smooth-iter") == 0) {
		    cargs.smooth_iter = stoi(argv[i+1]);
		    i++;
		}
		// target reduction for decimation
		else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--target-reduction") == 0) {
			try {
				cargs.target_reduction = stof(argv[i+1]);
			} catch (exception& e) {
				cerr << "exception caught: " << e.what() << endl;
				_abort = 1;
			}
			// ensure that the target reduction is between 0 and 1 exclusive
			if (cargs.target_reduction <= 0 || cargs.target_reduction >= 1) {
				cerr << "Target reduction out of range (0, 1): " << cargs.target_reduction << endl;
				_abort = 1;
			}
			i += 2;
		}
		// ASCII
		else if (strcmp(argv[i], "-A") == 0 || strcmp(argv[i], "--ascii") == 0) {
			cargs.ascii = 1;
			i++;
		}
		// header are uint64
		else if (strcmp(argv[i], "-U") == 0 || strcmp(argv[i], "--uint64") == 0) {
			cargs.uint64 = 1;
			i++;
		}
		// vtkIdType set to Int32
		else if (strcmp(argv[i], "-I") == 0 || strcmp(argv[i], "--int32") == 0) {
			cargs.int32 = 1;
			i++;
		}
		// verbose
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			cargs.verbose = 1;
			i++;
		}
		// help
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_usage();
			abort();
		}
		// map file
		else { // only one positional argument
			try {
				cargs.map_fn = argv[i];				
			}
			catch (exception& e) {
				cerr << "exception caught: " << e.what() << endl;
				_abort = 1; 
			}
			i++;
		}
	}
	
	// out_fn_full
	cargs.out_fn_full = cargs.out_fn + "." + cargs.out_format;
	
	// sanity checks
	// make sure that map_fn is not an empty string
	if (cargs.map_fn.compare("") == 0) {
		cerr << "Input MAP/MRC file not specified. Aborting..." << endl;
		_abort = 1;
	}
	
	// header uint64 is only relevant for vtp
	if (cargs.uint64 == 1 && cargs.out_format.compare("vtp") != 0) {
		cerr << "Warning: header set to UInt64 with non-vtp output format (" << cargs.out_format << ")" << endl;
	}
	
	// abort if we have to (after seeing all errors)
	if (_abort) {
		abort();
	}
	return cargs;
}

int main(int argc, char **argv)
{
	// get the args
	struct args cargs = parse_args(argc, argv);

	if (cargs.verbose)
		cout << "Reading MRC/MAP file..." << cargs.map_fn << endl;
	vtkSmartPointer<vtkMRCReader> reader = vtkSmartPointer<vtkMRCReader>::New();
	reader->SetFileName(cargs.map_fn.c_str());

    // contour
	if (cargs.verbose)
		cout << "Running contour filter at level " << cargs.clevel << "..." << endl;
	vtkSmartPointer<vtkContourFilter> cfilt = vtkSmartPointer<vtkContourFilter>::New();
	cfilt->SetInputConnection(reader->GetOutputPort());
	cfilt->SetValue(0, cargs.clevel);

	vtkSmartPointer<vtkTriangleFilter> tfilt = vtkSmartPointer<vtkTriangleFilter>::New();
	vtkSmartPointer<vtkSmoothPolyDataFilter> sfilt = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	vtkSmartPointer<vtkDecimatePro> dfilt = vtkSmartPointer<vtkDecimatePro>::New();
	if (cargs.decimate || cargs.smooth) {
	    // triangulate
		if (cargs.verbose)
			cout << "Running triangle filter..." << endl;
		tfilt->SetInputConnection(cfilt->GetOutputPort());

	    // smooth
		if (cargs.smooth) {
            if (cargs.verbose)
                cout << "Running smoothing filter with " << cargs.smooth_iter << " iterations..." << endl;
		    sfilt->SetInputConnection(tfilt->GetOutputPort());
		    sfilt->SetNumberOfIterations(cargs.smooth_iter);
		}

        // decimate
        if (cargs.decimate) {
            if (cargs.verbose)
                cout << "Running progressive decimation filter with " << cargs.target_reduction << " target reduction..." << endl;
            if (cargs.smooth)
                dfilt->SetInputConnection(sfilt->GetOutputPort());
            else
                dfilt->SetInputConnection(tfilt->GetOutputPort());
            dfilt->SetTargetReduction(cargs.target_reduction);
            dfilt->PreserveTopologyOn();
		}
	}

    // triangle strips
    vtkSmartPointer<vtkStripper> strip = vtkSmartPointer<vtkStripper>::New();
    if (cargs.verbose)
        cout << "Generating triangle strips..." << endl;
    if (cargs.decimate || cargs.smooth) {
        if (cargs.decimate)
            strip->SetInputConnection(dfilt->GetOutputPort());
        else
            strip->SetInputConnection(sfilt->GetOutputPort());
    }
    else {
        strip->SetInputConnection(cfilt->GetOutputPort());
    }
    strip->SetMaximumLength(1000);

	if (cargs.verbose)
		cout << "Writing output to '" << cargs.out_fn_full.c_str() << "'..." << endl;

	if (cargs.out_format.compare("stl") == 0){
		vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
//		if (cargs.decimate)
//			writer->SetInputConnection(dfilt->GetOutputPort());
//		else
//			writer->SetInputConnection(cfilt->GetOutputPort());
        writer->SetInputConnection(strip->GetOutputPort());
		writer->SetFileName(cargs.out_fn_full.c_str());
		if (cargs.ascii)
			writer->SetFileTypeToASCII();
		else
			writer->SetFileTypeToBinary();
		writer->Write();
	}
	else if (cargs.out_format.compare("vtk") == 0){
		vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
		writer->SetFileName(cargs.out_fn_full.c_str());
//		if (cargs.decimate)
//			writer->SetInputConnection(dfilt->GetOutputPort());
//		else
//			writer->SetInputConnection(cfilt->GetOutputPort());
        writer->SetInputConnection(strip->GetOutputPort());
		if (cargs.ascii)
			writer->SetFileTypeToASCII();
		else
			writer->SetFileTypeToBinary();
		writer->Write();
	} else if (cargs.out_format.compare("vtp") == 0){
		vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		writer->SetFileName(cargs.out_fn_full.c_str());
		// decimate
//		if (cargs.decimate)
//			writer->SetInputConnection(dfilt->GetOutputPort());
//		else
//			writer->SetInputConnection(cfilt->GetOutputPort());
        writer->SetInputConnection(strip->GetOutputPort());
		// vtkIdType
		if (cargs.int32)
			writer->SetIdTypeToInt32();
		else
			writer->SetIdTypeToInt64();
		// format
		if (cargs.ascii)
			writer->SetDataModeToAscii();
		else
			writer->SetDataModeToBinary();
		// header bits
		if (cargs.uint64) {
			if (cargs.verbose)
				cout << "Using UInt64 headers..." << endl;			
			writer->SetHeaderTypeToUInt64();
		}
		else
			if (cargs.verbose)
				cout << "Using UInt32 headers..." << endl;
			writer->SetHeaderTypeToUInt32();
		writer->Write();
	}

	return EXIT_SUCCESS;
}
