#include "Modules/FrameList.h"
#include "Modules/BackgroundModelling/BackgroundModel.h"
#include "Modules/BackgroundModelling_simple/BackgroundModel_simple.h"
#include "Modules/ForegroundProcessing/ForegroundProcessor.h"
#include "Modules/ObjectIdentification/Identification.h"
#include "Modules/Prediction/Kalman.h"
#include "Modules/Evaluation/Evaluation.h"

#include "Modules/Profiler.h"

#define PROFILER_INIT() ProfilerClock c;
#define PROFILER_RESET() c.reset();
#define PROFILE(string) frameList.setTime(string, c.getTime()); c.lap();
#define PROFILE_TOTALTIME() frameList.setTime("Total Time", c.getTotalTime());
#define PROFILE_TOTALFPS() frameList.setTime("Total FPS", c.getTotalFPS());

void sampleDown(Mat & source, Mat & destination);

int main()
{
	// Profiler init
	PROFILER_INIT();

	// Frame container
	FrameList frameList(10);	// Keep a history of up to 100 frames (might be used by some modules)
	
	// Modules
	BackgroundModelling_simple::BackgroundModel backgroundModel;
	//BackgroundModelling::BackgroundModel backgroundModel;
	ForegroundProcessing::ForegroundProcessor foregroundProcessor;
	Identification::Identifier identifier;
	Prediction::Kalman kalmanFilter;
	Evaluation evaluate(&frameList);

	// Init
	foregroundProcessor.setAlgortihm(ForegroundProcessing::AREA);
	foregroundProcessor.init(50, 3, 5, 50, 3);
	identifier.init(Identification::Ultimate);
	evaluate.readXML2FrameList("clip1.xml");
	
	
	// Load frame source
	frameList.open("camera1.mov");
	
	// Create windows
	namedWindow("Info",CV_WINDOW_AUTOSIZE);
	namedWindow("Background",CV_WINDOW_AUTOSIZE);
	namedWindow("Foreground",CV_WINDOW_AUTOSIZE);
	namedWindow("Tracking",CV_WINDOW_AUTOSIZE);
	namedWindow("BackgroundModel",CV_WINDOW_AUTOSIZE);

	// Track objects through all frames, as long as there are frames to query
	while (frameList.queryNextFrame())
	{
		
		// Reset profiler
		PROFILER_RESET();

		sampleDown(frameList.getLatestFrame().image, frameList.getLatestFrame().image);
		
		// Do the nessecary processing
		backgroundModel.update(frameList.getFrames());						PROFILE("BackgroundModel");
		foregroundProcessor.segmentForeground(frameList.getLatestFrame());	PROFILE("ForegroundSeg.");
		identifier.identify(frameList.getFrames());							PROFILE("Identification");	
		kalmanFilter.predict(frameList.getLatestFrame());					PROFILE("Kalman Prediction");
		//evaluate.currentFrame();											PROFILE("Evaluation");

		
		// Display result
		frameList.display("Tracking");
		frameList.displayForeground("Foreground");
		frameList.displayBackground("Background");
		frameList.displayBackgroundEstimate("BackgroundModel");
		//frameList.displayBackgroundCertainty("BackgroundCertainty");
				
		// Give the GUI time to render
		waitKey(1);															PROFILE("Display");

		// Optional pause between each frame
		// waitKey(0);
						
		// Read next frame from source
		//frameList.queryNextFrame();											
																			PROFILE("QueryNextFrame");										
																			PROFILE_TOTALTIME();
																			PROFILE("Total FPS");
																			PROFILE_TOTALFPS();

				

		// Display info
		frameList.displayInfo("Info");

	} 
	waitKey(0);

	return 0;
}

void sampleDown(Mat & source, Mat & destination)
{
	int stepSize = 1;
	Mat temp = source.clone();
	destination = Mat(temp.size().height/stepSize, temp.size().width/stepSize, CV_8UC3);		
	for(int r1=0, r2=0; r1 < temp.size().height; r1+=stepSize, r2++){
		for(int c1=0, c2=0; c1 < temp.size().width; c1+=stepSize, c2++){
			destination.at<Vec3b>(r2, c2) = temp.at<Vec3b>(r1, c1);
		}
	}
}