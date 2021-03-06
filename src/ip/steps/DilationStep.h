#pragma once

#include "ProcessingStep.h"

#include "src/ip/dialogs/dilationstepwidget.h"
#include "QObject"
#include "QWidget"

class DilationStep : public ProcessingStep
{
	Q_OBJECT
public:
	DilationStep( QWidget *parent = 0 );
	~DilationStep(void);

	void apply( cv::gpu::GpuMat * image );

	void toggleConfigWindow();

public slots:
	void kernelSizeChangedD( int ksize );
	void iterationChangedD( int iterations );

private:
	DilationStepWidget * controls;
	int iterations;
};

