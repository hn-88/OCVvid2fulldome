/*
 * OCVvid2fulldome.cpp
 * 
 * Warps "flat" video files to fit 180 degree fisheye "fulldome masters" using the OpenCV framework. 
 * Appends timestamp to the first filename and saves as default codec (DIVX avi) in the same folder.
 * 
 * first commit:
 * Hari Nandakumar
 * 10 May 2020
 * 
 * 
 */

//#define _WIN64
//#define __unix__

// references 
// http://paulbourke.net/geometry/transformationprojection/
// equations in figure at http://paulbourke.net/dome/dualfish2sphere/
// http://paulbourke.net/dome/dualfish2sphere/diagram.pdf

#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <unistd.h>
#endif

#include <string.h>

#include <time.h>
#include <opencv2/opencv.hpp>
#include "tinyfiledialogs.h"

#define CV_PI   3.1415926535897932384626433832795


// some global variables


int main(int argc,char *argv[])
{
	int outputw;
	int numvids;
	std::string VidFileName[100];
	int vidlongi[100];
	int vidw[100];
	int looptemp=0;
	
	std::cout << "Enter Outputw: ";
	std::cin >> outputw;
	std::cout << "Enter number of videos (max=99): ";
	std::cin >> numvids;
	
	char const * FilterPatterns[2] =  { "*.avi","*.*" };
	char const * OpenFileName;
	
	while(looptemp<numvids)
	{
		 OpenFileName = tinyfd_openFileDialog(
			"Open a video file",
			"",
			2,
			FilterPatterns,
			NULL,
			0);

		if (! OpenFileName)
		{
			tinyfd_messageBox(
				"Error",
				"No file chosen. ",
				"ok",
				"error",
				1);
			return 1 ;
		}
		
		VidFileName[looptemp] = std::string(OpenFileName);
		std::cout << "Enter desired position for this video as degrees longitude, back = 0: ";
		std::cin >> vidlongi[looptemp];
		std::cout << "Enter desired width for this video in degrees: ";
		std::cin >> vidw[looptemp];
		
		looptemp++;
	}
	
	for (int i=0;i<numvids;i++)
		std::cout<<VidFileName[i]<<" " << vidlongi[i] << " " << vidw[i] << std::endl;
	
	return 0;
}
