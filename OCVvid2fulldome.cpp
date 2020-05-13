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

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <time.h>
#include <opencv2/opencv.hpp>
#include "tinyfiledialogs.h"

#define CV_PI   3.1415926535897932384626433832795

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

void update_map( int vidlongi, int vidlati, int vidw, float aspectratio, cv::Mat &map_x, cv::Mat &map_y )
{
	float angleyrad = -(float)vidlati*CV_PI/180;	// need the minus
	float anglexrad = (float)(vidlongi+180)*CV_PI/180;	// and other manipulations 
	// to get to the correct place
	
	float mapoutside = (map_x.rows+map_x.cols)*10.0;
	
	// for the mapping to centre / resized
	int vidwpixels = round(((float)vidw/360.0) * map_x.cols);
	int vidhpixels = round((float)vidwpixels / aspectratio);
	int leftmargin = round((float)(map_x.cols - vidwpixels) / 2);
	int rightmargin = leftmargin + vidwpixels;
	int topmargin = round((float)(map_x.rows - vidhpixels) / 2);
	int bottommargin = topmargin + vidhpixels;
	// OpenCV (0,0) is top left. 
	float resizeratiox = (float)vidwpixels / (float) map_x.cols;
	float resizeratioy = (float)vidhpixels / (float) map_x.rows;
	
	// create intermediate maps
	cv::Mat resiz_x, resiz_y, mapf_x, mapf_y;
	map_x.copyTo(resiz_x);
	map_x.copyTo(resiz_y);
	map_x.copyTo(mapf_x);
	map_x.copyTo(mapf_y);
		
	// for the mapping from equi to fisheye
	// changed to pano2fisheye code for zoom in / out effect
	int xcd = floor(map_x.cols/2) - 1 ;
	int ycd = floor(map_x.rows/2) - 1 ;
		
	int xd, yd;
	float px_per_theta = map_x.cols / (2*CV_PI);
	float px_per_phi   = map_x.rows / (CV_PI/2);
	// compute destination radius and theta 
	float rd; // = sqrt(x^2+y^2);
	float theta; //= atan2(y,x);
	float rad_per_px = CV_PI / map_x.rows;
	float phiang;     // = rad_per_px * rd;
	
					
	for(int j = 0; j < map_x.rows; j++ )	// j is for y, i is for x
	{ 
		for( int i = 0; i < map_x.cols; i++ )
		{
			if( i < rightmargin 
			&& i > leftmargin 
			&& j < bottommargin 
			&& j > topmargin )
			{
			 resiz_x.at<float>(j,i) = (i-leftmargin)/resizeratiox; // x*resizeratio + leftmargin = i
			 resiz_y.at<float>(j,i) = (j-topmargin)/resizeratioy  ;
			 // we want the image flipped up down / lr from the above.
			 //~ resiz_x.at<float>(j,i) = map_x.cols - (i-leftmargin)/resizeratiox;
			 //~ resiz_y.at<float>(j,i) = map_x.rows - (j-topmargin)/resizeratioy;

			}
			
			// resiz_x to map_x mapping as in OCVWarp (old version) transformtype=0.
			
			xd = i - xcd;
			yd = j - ycd;
			if (xd == 0 && yd == 0)
			{
				theta = 0 + anglexrad;
				rd = 0;
			}
			else
			{
				theta = atan2(xd,yd) + anglexrad; 
				rd = sqrt(float(xd*xd + yd*yd));
			}
			// move theta to [-pi, pi]
			theta = fmod(theta+CV_PI, 2*CV_PI);
			if (theta < 0)
				theta = theta + CV_PI;
			theta = theta - CV_PI;	
			
			phiang = rad_per_px * rd + angleyrad; // this zooms in/out, not rotate cam
			phiang = rad_per_px * rd;
			
			if(rd <= xcd)
			{
				mapf_x.at<float>(j, i) = (float)(map_x.cols/2) + theta * px_per_theta;
				mapf_y.at<float>(j, i) = phiang * px_per_phi;
			}
				
		} // end for i
	} // end for j
	
	cv::remap(resiz_x, map_x, mapf_x, mapf_y, cv::INTER_LINEAR );
	cv::remap(resiz_y, map_y, mapf_x, mapf_y, cv::INTER_LINEAR );
	// here, the remap function maps undefined points of map_x, map_y to (0,0)
	// this is manually corrected below.
	for(int j = 0; j < map_x.rows; j++ )	// j is for y, i is for x
	{ 
		for( int i = 0; i < map_x.cols; i++ )
		{
			xd = i - xcd;
			yd = j - ycd;
			rd = sqrt(float(xd*xd + yd*yd));
			if(rd > xcd)
			{
				map_x.at<float>(j, i) = mapoutside;
				map_y.at<float>(j, i) = mapoutside;
			}
		}
	}
	
}


int main(int argc,char *argv[])
{
	bool doneflag = 0;
	bool showdisplay = 1, interactivemode=0;
	bool skipinputs = 0;
	int outputw, outputfps;
	char outputfourcc[5] = {'X','V','I','D', '\0'};
	std::string fourccstr;
	int numvids;
	std::string VidFileName[100];
	std::string NAME;
	int vidlongi[100];
	int vidlati[100];
	int vidw[100];
	float aspectratio[100];
	int looptemp=0;
	cv::VideoCapture inputVideo[100];
	bool inputEnded[100];
	std::string escapedpath;
	cv::VideoWriter outputVideo;
	int  fps, key;
	int t_start, t_end;
    unsigned long long framenum = 0;
    std::string tempstring;
     
    cv::Mat src, dst, res;
    cv::Mat dstfloat, dstmult, dstres, dstflip;
    cv::Size Sout;
    
    std::vector<cv::Mat> spl;
    cv::Mat dst2;	// temp dst, for eachvid
    cv::Mat map_x[100], map_y[100];
    cv::Mat dst_x[100], dst_y[100];
    
    if(argc > 1)
    {
		// argument can be ini file path
		std::ifstream infile(argv[1]);
		if (infile.is_open())
		  {
			  try
			  {
				std::getline(infile, tempstring);
				std::getline(infile, tempstring);
				std::getline(infile, tempstring);
				// first three lines of ini file are comments
				std::getline(infile, NAME);
				infile >> tempstring;
				infile >> outputw;
				infile >> tempstring;
				infile >> outputfps;
				infile >> tempstring;
				infile >> outputfourcc;
				infile >> tempstring;
				infile >> numvids;
				// https://stackoverflow.com/questions/6649852/getline-not-working-properly-what-could-be-the-reasons
				// dummy getline after the >> on previous line
				std::getline(infile, tempstring);
				
				for(int i=0; i<numvids; i++)
				{
					std::getline(infile, tempstring);
					std::getline(infile, tempstring);
					VidFileName[i] = tempstring;
					infile >> tempstring;
					infile >> vidlongi[i];
					infile >> tempstring;
					infile >> vidlati[i];
					infile >> tempstring;
					infile >> vidw[i];
					// dummy getline after the >> on previous line
					std::getline(infile, tempstring);
				}
				infile.close();
				skipinputs = 1;
			  } // end try
			  catch(std::ifstream::failure &readErr) 
				{
					std::cerr << "\n\nException occured when reading ini file\n"
						<< readErr.what()
						<< std::endl;
					skipinputs = 0;
				} // end catch
			 
		  } // end if isopen
		  
	  } // end if argc > 1

	
	if(!skipinputs)
	{
	// better to use getline
	// https://stackoverflow.com/questions/4999650/c-how-do-i-check-if-the-cin-buffer-is-empty
	std::cout << "ini file not supplied or unreadable. So, manual inputs ..." << std::endl;
	std::cout << "Enter Output video width (=height): ";
	std::cin >> outputw;
	std::cout << "Enter Output video framerate (fps): ";
	std::cin >> outputfps;
	std::cout << "Enter Output video fourcc (default is XVID): ";
	// a dummy getline is needed
	std::getline(std::cin, tempstring);
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
		
		VidFileName[looptemp] = escaped(std::string(OpenFileName));
		std::cout << "Enter desired position for this video as degrees longitude, back = 0: ";
		std::cin >> vidlongi[looptemp];
		std::cout << "Enter desired height for this video as degrees latitude, horizon = 0: ";
		std::cin >> vidlati[looptemp];
		std::cout << "Enter desired width for this video in degrees: ";
		std::cin >> vidw[looptemp];
		
		looptemp++;
	}
	
	std::string OpenFileNamestr = VidFileName[0];
	std::string::size_type pAt = OpenFileNamestr.find_last_of('.');                  // Find extension point
    NAME = OpenFileNamestr.substr(0, pAt) + "F" + ".avi";   // Form the new name with container
    
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
	
    // save parameters as an ini file
    pAt = NAME.find_last_of('.');                  // Find extension point
    std::string ininame = NAME.substr(0, pAt) + ".ini";   // Form the ini name 
    std::ofstream inioutfile(ininame, std::ofstream::out);
    inioutfile << "#ini_file_for_OCVvid2fulldome--Comments_start_with_#" << std::endl;
    inioutfile << "#Each_parameter_is_entered_in_the_line_below_the_comment." << std::endl;
    inioutfile << "#Output_file_path" << std::endl;
    inioutfile << NAME << std::endl;
    inioutfile << "#Outputw_pixels__=height" << std::endl;
    inioutfile << outputw << std::endl;
    inioutfile << "#Output_fps" << std::endl;
    inioutfile << outputfps << std::endl;
    inioutfile << "#Output_fourcc" << std::endl;
    inioutfile << outputfourcc << std::endl;
    inioutfile << "#Number_of_input_videos_max_is_99" << std::endl;
    inioutfile << numvids << std::endl;
	for (int i=0;i<numvids;i++)
	{
		inioutfile << "#Filename" << i << std::endl;
		inioutfile << VidFileName[i] << std::endl;
		inioutfile << "#vidlongi" << i << std::endl;
		inioutfile << vidlongi[i] << std::endl;
		inioutfile << "#vidlati" << i << std::endl;
		inioutfile << vidlati[i] << std::endl;
		inioutfile << "#vidw" << i << std::endl;
		inioutfile << vidw[i] << std::endl;
	}
		
	} // end if !skipinputs
	
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
		escapedpath = VidFileName[i];
		inputVideo[i] = cv::VideoCapture(escapedpath);
		inputVideo[i] >> src;
		aspectratio[i] = (float)src.cols / (float)src.rows; // assuming square pixels
		inputVideo[i].set(cv::CAP_PROP_POS_FRAMES, 0);
		// reset the video to the first frame.
				
		map_x[i] = cv::Mat(Sout, CV_32FC1);
		map_y[i] = cv::Mat(Sout, CV_32FC1);
		map_x[i] = cv::Scalar((outputw+outputw)*10);
		map_y[i] = cv::Scalar((outputw+outputw)*10);
		// initializing so that it points outside the image
		// so that unavailable pixels will be black
		update_map(vidlongi[i], vidlati[i], vidw[i], aspectratio[i], map_x[i], map_y[i]);
		cv::convertMaps(map_x[i], map_y[i], dst_x[i], dst_y[i], CV_16SC2);	
		// supposed to make it faster to remap
	}
	
	t_start = time(NULL);
	fps = 0;
	
	for (int i=0;i<numvids;i++)
	{
		inputEnded[i]=0;
	}
	
	// https://stackoverflow.com/questions/5907031/printing-the-correct-number-of-decimal-points-with-cout
	std::cout << std::fixed << std::setprecision(1);
	
	while(1)
	{
		dst = cv::Mat::zeros(Sout, CV_8UC3);
		// loop through all frames of input vids
		for (int i=0;i<numvids;i++)
		{
			inputVideo[i] >> src;              // read
			if (src.empty()) // check if at end
			{
				inputEnded[i]=1;
				
			}
			else
			{
			//	inputEnded[i]=0;	
			//imshow("Display",src);
			cv::resize(src, res, Sout, 0, 0, cv::INTER_LINEAR);
			cv::remap(res, dst2, dst_x[i], dst_y[i], cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0) );
			dst = dst + dst2;
			}
			
		}
		
		doneflag = 1;
		for (int i=0;i<numvids;i++)
		{
			doneflag = doneflag && inputEnded[i];
			// doneflag = 1 when all inputs are Ended
		}
		
		
		key = cv::waitKey(10);
		
		if(showdisplay)
			cv::imshow("Display", dst);
			
        std::cout << "\r";
        
        fps++;
        t_end = time(NULL);
		if (t_end - t_start >= 5)
		{
			std::cout << "Frame: " << framenum++ << "  fps: " << (float)fps/5 << "           \r";
			// extra spaces to delete previous line's characters if any
			fflush(stdout);
			t_start = time(NULL);
			fps = 0;
		}
		else
		{
			std::cout << "Frame: " << framenum++ <<  "\r";
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

				case 'u':
				case '+':
				case '=':	// increase angley
					for (int i=0;i<numvids;i++)
					{
						vidlati[i]=vidlati[i] + 1;
					}
					interactivemode = 1;
					break;
					
				case 'm':
				case '-':
				case '_':	// decrease angley
					for (int i=0;i<numvids;i++)
					{
						vidlati[i]=vidlati[i] - 1;
					}
					interactivemode = 1;
					break;
					
				case 'k':
				case '}':
				case ']':	// increase anglex
					for (int i=0;i<numvids;i++)
					{
						vidlongi[i]=vidlongi[i] + 1;
					}
					interactivemode = 1;
					break;
					
				case 'h':
				case '{':
				case '[':	// decrease anglex
					for (int i=0;i<numvids;i++)
					{
						vidlongi[i]=vidlongi[i] - 1;
					}
					interactivemode = 1;
					break;
				
				case 'U':
					// increase angley
					for (int i=0;i<numvids;i++)
					{
						vidlati[i]=vidlati[i] + 10;
					}
					interactivemode = 1;
					break;
					
				case 'M':
					// decrease angley
					for (int i=0;i<numvids;i++)
					{
						vidlati[i]=vidlati[i] - 10;
					}
					interactivemode = 1;
					break;
					
				case 'K':
					// increase anglex
					for (int i=0;i<numvids;i++)
					{
						vidlongi[i]=vidlongi[i] + 10;
					}
					interactivemode = 1;
					break;
					
				case 'H':
					// decrease anglex
					for (int i=0;i<numvids;i++)
					{
						vidlongi[i]=vidlongi[i] - 10;
					}
					interactivemode = 1;
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
		
		if(interactivemode == 1)
        {
			for (int i=0;i<numvids;i++)
			{
				map_x[i] = cv::Scalar((outputw+outputw)*10);
				map_y[i] = cv::Scalar((outputw+outputw)*10);
				update_map(vidlongi[i], vidlati[i], vidw[i], aspectratio[i], map_x[i], map_y[i]);
				cv::convertMaps(map_x[i], map_y[i], dst_x[i], dst_y[i], CV_16SC2);	
				// supposed to make it faster to remap
			}
			interactivemode = 0;
		}
        
			
	} // end of while(1)
	
	std::cout << std::endl << "Finished writing." << std::endl;
	return 0;
}
