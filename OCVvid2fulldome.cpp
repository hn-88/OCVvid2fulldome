/*
 * OCVvid2fulldome.cpp
 * 
 * Warps "flat" video files to fit 180 degree fisheye "fulldome masters" using the OpenCV framework. 
 * Appends 'F.avi' to the first filename and saves as default codec (XVID avi) in the same folder.
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

#include <string>

#include <time.h>
#include <opencv2/opencv.hpp>
#include "tinyfiledialogs.h"

#define CV_PI   3.1415926535897932384626433832795


// some global variables

std::string escaped(const std::string& input)
{
	// https://stackoverflow.com/questions/48260879/how-to-replace-with-in-c-string
	// needed for windows paths for opencv
    std::string output;
    output.reserve(input.size());
    for (const char c: input) {
        switch (c) {
            case '\a':  output += "\\a";        break;
            case '\b':  output += "\\b";        break;
            case '\f':  output += "\\f";        break;
            case '\n':  output += "\\n";        break;
            case '\r':  output += "\\r";        break;
            case '\t':  output += "\\t";        break;
            case '\v':  output += "\\v";        break;
            default:    output += c;            break;
        }
    }

    return output;
}

void update_map( int vidlongi, int vidw, cv::Mat &map_x, cv::Mat &map_y )
{
	//float vidlongrad = 
	if (vidlongi==1)
	{
		for( int j = 0; j < map_x.rows; j++ )
		{ 
			for( int i = 0; i < map_x.cols; i++ )
			{
				if( i < map_x.cols*0.5 &&  j < map_x.rows*0.5 )
				{
                 map_x.at<float>(j,i) = 2*( i - map_x.cols*0.25 ) + 0.5 ;
                 map_y.at<float>(j,i) = 2*( j - map_x.rows*0.25 ) + 0.5 ;
                }
			} // end for i
		} // end for j
	} // end if
	
	if (vidlongi==2)
	{
		for( int j = 0; j < map_x.rows; j++ )
		{ 
			for( int i = 0; i < map_x.cols; i++ )
			{
				if( i > map_x.cols*0.5 &&  j < map_x.rows*0.5 )
				{
                 map_x.at<float>(j,i) = 2*( i - map_x.cols*0.25 ) + 0.5 ;
                 map_y.at<float>(j,i) = 2*( j - map_x.rows*0.25 ) + 0.5 ;
                }
			} // end for i
		} // end for j
	} // end if
	
	if (vidlongi==3)
	{
		for( int j = 0; j < map_x.rows; j++ )
		{ 
			for( int i = 0; i < map_x.cols; i++ )
			{
				if( i < map_x.cols*0.5 &&  j > map_x.rows*0.5 )
				{
                 map_x.at<float>(j,i) = 2*( i - map_x.cols*0.25 ) + 0.5 ;
                 map_y.at<float>(j,i) = 2*( j - map_x.rows*0.25 ) + 0.5 ;
                }
			} // end for i
		} // end for j
	} // end if
	
	if (vidlongi==4)
	{
		for( int j = 0; j < map_x.rows; j++ )
		{ 
			for( int i = 0; i < map_x.cols; i++ )
			{
				if( i > map_x.cols*0.5 &&  j > map_x.rows*0.5 )
				{
                 map_x.at<float>(j,i) = 2*( i - map_x.cols*0.25 ) + 0.5 ;
                 map_y.at<float>(j,i) = 2*( j - map_x.rows*0.25 ) + 0.5 ;
                }
			} // end for i
		} // end for j
	} // end if

}


int main(int argc,char *argv[])
{
	bool doneflag = 0;
	bool showdisplay = 1;
	int outputw, outputfps;
	char outputfourcc[5] = {'X','V','I','D', '\0'};
	std::string fourccstr;
	int numvids;
	std::string VidFileName[100];
	int vidlongi[100];
	int vidw[100];
	int looptemp=0;
	cv::VideoCapture inputVideo[100];
	std::string escapedpath;
	cv::VideoWriter outputVideo;
	int  fps, key;
	int t_start, t_end;
    unsigned long long framenum = 0;
     
    cv::Mat src, dst, res;
    cv::Mat dstfloat, dstmult, dstres, dstflip;
    cv::Size Sout;
    
    std::vector<cv::Mat> spl;
    cv::Mat dst2;	// temp dst, for eachvid
    cv::Mat map_x[100], map_y[100];
    cv::Mat dst_x[100], dst_y[100];
	
	// better to use getline
	// https://stackoverflow.com/questions/4999650/c-how-do-i-check-if-the-cin-buffer-is-empty
	std::cout << "Enter Output video width (=height): ";
	std::cin >> outputw;
	std::cout << "Enter Output video framerate (fps): ";
	std::cin >> outputfps;
	std::cout << "Enter Output video fourcc (default is XVID): ";
	std::getline(std::cin, fourccstr);
	if(fourccstr.empty())
	{
		// use the default
	}
	else
	{
		outputfourcc[0]=fourccstr.at(0);
		outputfourcc[1]=fourccstr.at(1);
		outputfourcc[2]=fourccstr.at(2);
		outputfourcc[3]=fourccstr.at(3);
	}
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
	
	std::string OpenFileNamestr = VidFileName[0];
	std::string::size_type pAt = OpenFileNamestr.find_last_of('.');                  // Find extension point
    std::string NAME = OpenFileNamestr.substr(0, pAt) + "F" + ".avi";   // Form the new name with container
    
	char const * SaveFileName = tinyfd_saveFileDialog(
		"Now enter the output video file name, like output.mp4",
		"",
		0,
		NULL,
		NULL);

	if (! SaveFileName)
	{
		tinyfd_messageBox(
			"No output file chosen.",
			"Will be saved as inputfilename + F.avi",
			"ok",
			"info",
			1);
		 
	}
	else
	{
		escapedpath = escaped(std::string(SaveFileName));
		NAME = escapedpath;
	}
	Sout = cv::Size(outputw,outputw);
	outputVideo.open(NAME, outputVideo.fourcc(outputfourcc[0], outputfourcc[1], outputfourcc[2], outputfourcc[3]), 
        outputfps, Sout, true);
    if (!outputVideo.isOpened())
    {
        std::cout  << "Could not open the output video for write: " << NAME << std::endl;
        return -1;
    }
     

	for (int i=0;i<numvids;i++)
	{
		//std::cout<<VidFileName[i]<<" " << vidlongi[i] << " " << vidw[i] << std::endl;
		escapedpath = escaped(VidFileName[i]);
		inputVideo[i] = cv::VideoCapture(escapedpath);
		map_x[i] = cv::Mat(Sout, CV_32FC1);
		map_y[i] = cv::Mat(Sout, CV_32FC1);
		map_x[i] = cv::Scalar((outputw+outputw)*10);
		map_y[i] = cv::Scalar((outputw+outputw)*10);
		// initializing so that it points outside the image
		// so that unavailable pixels will be black
		update_map(vidlongi[i], vidw[i], map_x[i], map_y[i]);
		cv::convertMaps(map_x[i], map_y[i], dst_x[i], dst_y[i], CV_16SC2);	
		// supposed to make it faster to remap
	
		
	}
	
	t_start = time(NULL);
	fps = 0;
	
	while(1)
	{
		dst = cv::Mat::zeros(Sout, CV_8UC3);
		// loop through all frames of input vids
		for (int i=0;i<numvids;i++)
		{
			inputVideo[i] >> src;              // read
			if (src.empty()) break;         // check if at end
			//imshow("Display",src);
			cv::resize(src, res, Sout, 0, 0, cv::INTER_LINEAR);
			cv::remap(res, dst2, dst_x[i], dst_y[i], cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0) );
			dst = dst + dst2;
			
		}
		
		key = cv::waitKey(10);
		
		if(showdisplay)
			cv::imshow("Display", dst);
			
        std::cout << "\r";
        
        fps++;
        t_end = time(NULL);
		if (t_end - t_start >= 5)
		{
			printf("Frame: %llu fps: %.1f           \r", framenum++, float(fps)/5 );
			// extra spaces to delete previous line's characters if any
			fflush(stdout);
			t_start = time(NULL);
			fps = 0;
		}
		else
		{
			printf("Frame: %llu  \r", framenum++ );
			fflush(stdout);
		}
			
        
       //outputVideo.write(res); //save or
       outputVideo << dst;
       
       
       switch (key)
				{

				case 27: //ESC key
				case 'x':
				case 'X':
					doneflag = 1;
					break;

					
				case 'D':
				case 'd':
					// toggle display
					if(showdisplay)
						showdisplay=0;
					else
						showdisplay=1;
					break;	
					
				default:
					break;
				
				}
				
		if (doneflag == 1)
		{
			break;
		}
        
			
	} // end of while(1)
	
	return 0;
}
