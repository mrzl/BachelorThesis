#include "OpticalFlowBlockMatching.h"


OpticalFlowBlockMatching::OpticalFlowBlockMatching(void)
{
}


OpticalFlowBlockMatching::~OpticalFlowBlockMatching(void)
{
}

void OpticalFlowBlockMatching::apply( cv::gpu::GpuMat * mat )
{
	cv::gpu::GpuMat currentMat( *mat );
	cv::gpu::cvtColor( currentMat, currentMat, CV_BGRA2GRAY );
	if( lastMat.empty() || lastMat.size() != currentMat.size() )
	{
		lastMat = cv::gpu::GpuMat( currentMat );
	}

	
	cv::gpu::GpuMat velX( mat->size(), mat->type() );
	cv::gpu::GpuMat velY( mat->size(), mat->type() );
	cv::gpu::GpuMat buf( mat->size(), mat->type() );
	cv::gpu::calcOpticalFlowBM( lastMat, currentMat, cv::Size( 5, 5 ), cv::Size( 20, 20 ), cv::Size( 30, 30 ), true, velX, velY, buf );
	cv::Mat xy[2]; //X,Y
	cv::Mat tempBuf;
	buf.download( tempBuf );
	cv::split(tempBuf, xy);

	//calculate angle and magnitude
	cv::Mat magnitude, angle;
	cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);

	//translate magnitude to range [0;1]
	double mag_max;
	double mag_min = 0.0;
	//cv::minMaxLoc()
	cv::minMaxLoc(magnitude, &mag_min, &mag_max);
	magnitude.convertTo(magnitude, -1, 1.0/mag_max);

	//build hsv image
	cv::Mat _hsv[3], hsv;
	_hsv[0] = angle;
	_hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
	_hsv[2] = magnitude;
	cv::merge(_hsv, 3, hsv);

	//convert to BGR and show
	cv::Mat bgr;//CV_32FC3 matrix
	cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
	mat->upload( bgr );
	//buf.copyTo( *mat );

	currentMat.copyTo( lastMat );
}
