#include "MedianFilterStep.h"


MedianFilterStep::MedianFilterStep( QWidget *parent ) :
	iterations( 1 ),
	kernelSize( 1 )
{
	controls = new MedianFilterStepDialog( this );

	connect( controls, SIGNAL( iterationsChanged( int ) ), this, SLOT( iterationsChanged( int ) ) );
	connect( controls, SIGNAL( kernelSizeChanged( int ) ), this, SLOT( kernelSizeChanged( int ) ) );
}


MedianFilterStep::~MedianFilterStep(void)
{
	delete controls;
}

void MedianFilterStep::apply( cv::gpu::GpuMat * image )
{
	if( isActive()  )
	{
		cv::Mat im;
		image->download( im );
		for( int i = 0; i < iterations; i++ )
		{
			cv::medianBlur( im, im, kernelSize );
			//cv::medianBlur( *image, processedImage, getKernelSize() );
			//processedImage.copyTo( *image );
		}
		image->upload( im );
	}
}

void MedianFilterStep::toggleConfigWindow( void )
{
	controls->setVisible( !controls->isVisible() );
}

void MedianFilterStep::iterationsChanged( int _it )
{
	this->iterations = _it;
}

void MedianFilterStep::kernelSizeChanged( int _k )
{
	this->kernelSize = _k;
	if( this->kernelSize % 2 == 0 )
	{
		this->kernelSize++;
	}
	//this->setKernelSize( _k );
}
