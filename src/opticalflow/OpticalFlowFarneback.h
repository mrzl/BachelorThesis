#pragma once

#include < vector >

#include "QObject"

#include "opencv2/core/core.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include "src/opticalflow/dialogs/OpticalFlowFarnebackConfigDialog.h"

class OpticalFlowFarneback : QWidget
{
	Q_OBJECT
public:
	// parameters and opticallfowdraw from here https://code.ros.org/trac/opencv/browser/trunk/opencv/samples/c/fback.cpp?rev=2271
	OpticalFlowFarneback(void);
	~OpticalFlowFarneback(void);

	void calc( cv::Mat * image );
	void drawOptFlowMap(const cv::Mat& flow, cv::Mat& cflowmap, int step, double scale, const cv::Scalar& color);


	void setPyrScale( double _pyr_scale );
	void setPolySigma( double _poly_sigma );
	void setLevels( int _levels );
	void setWinSize( int _winSize );
	void setIterations( int _iterations );
	void setPolyN( int poly_n );
	void setStepSize( int _stepSize );
	void setScale( double _scale );
private:
	OpticalFlowFarnebackConfigDialog * gui;
	cv::Mat previousImage;
	double pyr_scale, poly_sigma, stepSize;
	int levels, winSize, iterations, poly_n, scale;

public slots:
	void changedWinSize( int _winSize );
	void changedPyrScale( int _pyr_scale );
	void changedPolySigma( int _polySigma );
	void changedLevels( int _levels );
	void changedIterations( int _iterations );
	void changedPolyN( int _poly_n );
	void changedStepSize( int _stepsize );
	void changedScale( int _scale );
};
