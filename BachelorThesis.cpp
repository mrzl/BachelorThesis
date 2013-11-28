#include "BachelorThesis.h"
#include "src/misc/SystemInfo.h"

#include <thread>

#include "QTimer"
#include "QFileDialog"
#include "QLabel"
#include "QRectF"
#include "QPainter"

BachelorThesis::BachelorThesis(QWidget *parent)
	: QMainWindow(parent),
	videoReader( VideoReader::Type::GPU ),
	playbackSpeed( 1 ),
	doBackgroundSubtraction( false ),
	doMeanShiftFiltering( false ),
	blurAmount( 1 )
{
	ui.setupUi(this);

	timer = Timer();	

	lukasKanadeOpticalFlowDialog = new LukasKanadeOpticalFLowDialog( this );
	hardwareInfoDialog = new HardwareInfoDialog( this );
	imageProcessorWidget = new ImageProcessorWidget( this );
	imageProcessorWidget->show();

	
	//image.setText( QString( "whut" ) );
	//image.setParent( this );
	//imageLabel.setParent( this );
	imageLabel.setWindowTitle( QString( "VideoStream" ) );
	imageLabel.show();
	
	cv::gpu::setDevice( 0 );

	connect( ui.actionOpen_File,			SIGNAL( triggered() ),				this,	SLOT( openFile() ) );
	connect( ui.verticalSlider,				SIGNAL( valueChanged( int ) ),		this,	SLOT( changePlaybackSpeed( int ) ) );
	connect( ui.pushButton,					SIGNAL( clicked() ),				this,	SLOT( startVideo() ) );
	// TODO: fix this, it should only call this slot if the slider is moved by hand
	//connect( ui.progressBarSlider,			SIGNAL( valueChanged( int) ),		this,	SLOT( jumpToFrame( int ) ) );
	connect( ui.checkBox,					SIGNAL( toggled( bool ) ),			this,	SLOT( toggleBackgroundSubtraction( bool ) ) );
	connect( ui.checkBox_2,					SIGNAL( toggled( bool ) ),			this,	SLOT( toggleMeanShiftFiltering( bool ) ) );
	connect( ui.blurSlider,					SIGNAL( valueChanged( int ) ),		this,	SLOT( blurAmountChanged( int ) ) );
	connect( ui.actionOpen_Sample,			SIGNAL( triggered() ),				this,	SLOT( openSampleFile() ) );
	
	connect( ui.actionPyrLukasKanade,		SIGNAL( triggered() ),				this,	SLOT( openLukasKanadeWindow( ) ) );
	connect( ui.actionHardware_Info,		SIGNAL( triggered() ),				this,	SLOT( openHardwareInfoDialog() ) );

	connect( lukasKanadeOpticalFlowDialog,	SIGNAL( itersValueChanged( int ) ), this,	SLOT( changeLKIters( int ) ) );

	connect( &imageLabel, SIGNAL( setRoi( int, int, int, int ) ), this, SLOT( setRoi( int, int, int, int ) ) );
	
}

BachelorThesis::~BachelorThesis()
{
	delete hardwareInfoDialog;
	delete lukasKanadeOpticalFlowDialog;
	delete imageProcessorWidget;
}

void BachelorThesis::loadImage() 
{
	if( videoReader.isOpen() )
	{
		timer.start();
		cv::gpu::GpuMat * imageToProcess;
		cv::gpu::GpuMat finishedImage;

		for( int i = 0; i < playbackSpeed; i++ )
		{
			imageToProcess = videoReader.getNextImage_GPU();
		}
		QRect roi = imageLabel.getRoi();

		// get roi from image
		if( roi.isEmpty() || roi.width() >= imageToProcess->cols || roi.height() >= imageToProcess->rows )
		{
			roi = QRect( 0, 0, imageToProcess->cols, imageToProcess->rows );
		} 
		cv::gpu::GpuMat section( roi.width(), roi.height(), imageToProcess->type() );
		QRect selectedRect = QRect( roi.x(), roi.y(), roi.width(), roi.height() );
		cv::Rect cvSelectedRect = cv::Rect( selectedRect.x(), selectedRect.y(), selectedRect.width(), selectedRect.height() );
		std::cout << "section: x=" << section.cols << " y=" << section.rows << std::endl;
		std::cout << "cvSelectedRect: x=" << cvSelectedRect.width << " y:" << cvSelectedRect.height << std::endl;
		//imageToProcess->copyTo( section( cvSelectedRect ) );
		cv::gpu::GpuMat tempMat = *imageToProcess;
		//*imageToProcess( cvSelectedRect).copyTo( section );
		tempMat(cvSelectedRect).copyTo( section );
		pipeline.addImage( &section );
		pipeline.start();
		finishedImage = pipeline.getFinishedImage();

		cv::gpu::GpuMat tempMat2 = *imageToProcess;
		finishedImage.copyTo( tempMat2( cv::Rect( roi.x(), roi.y(), roi.width(), roi.height() ) ) );
		

		//frameHandler.display( &finishedImage, 0 );
		QPixmap imagePixmap = QPixmap::fromImage( this->mat2QImage( cv::Mat( tempMat2 ) ) );
		//QPixmap map( QSize( 200, 200 ) );
		//painter.set
		
		
		imageLabel.setPixmap( imagePixmap );
		imageLabel.setMaximumHeight( tempMat2.cols );
		imageLabel.setMaximumWidth( tempMat2.rows );
		//image.setSizeIncrement( imagePixmap.size().width(), imagePixmap.size().height() );
		imageLabel.adjustSize();
		timer.stop();
		timer.store();
		std::cout << "it took by average:" << timer.getAverageTimeStdString() << "ms." << std::endl;
		std::cout << "lates was: " << timer.getLatestStdString() << "ms." << std::endl;
		QString elapsed;
		elapsed.append( QString( "%1" ).arg( videoReader.getNormalizedProgress() ) );

		ui.label->setText( elapsed );
		ui.progressBarSlider->setValue( videoReader.getCurrentFrameNr() );
	}
	else
	{
		// if the file/stream is not opened then all windows should be closed
		//frameHandler.closeAllWindows();
		imageLabel.close();
		//videoReader.close();
	}
}

void BachelorThesis::openFile( void )
{
	QString fileName = QFileDialog::getOpenFileName( this, tr( "Open File" ), "", tr( "MP4 (*.mp4);; AVI (*.avi)" ) );

	//frameHandler.createNewOutput( "VIDEO_GPU", 0, cv::WINDOW_OPENGL );
	imageLabel.show();
	videoReader.open( fileName.toStdString() );
	ui.progressBarSlider->setMaximum( videoReader.getMaxFrames() );
}

void BachelorThesis::changePlaybackSpeed( int _playbackSpeed )
{
	this->playbackSpeed = _playbackSpeed;
}

void BachelorThesis::startVideo( void )
{
	QTimer * timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT(loadImage() ) );
	timer->start( 30 );

	ui.pushButton->setText( QString( "Pause" ) );
}

void BachelorThesis::jumpToFrame( int _frameNr )
{
	videoReader.jumpToFrame( _frameNr );
}

void BachelorThesis::toggleBackgroundSubtraction( bool _doBackgroundSubtraction )
{
	doBackgroundSubtraction = _doBackgroundSubtraction;
}

void BachelorThesis::blurAmountChanged( int _blurAmount )
{
	std::cout << "BlurAmount changed to " << _blurAmount << std::endl;
	this->blurAmount = _blurAmount;
}

void BachelorThesis::openLukasKanadeWindow( void )
{
	lukasKanadeOpticalFlowDialog->show();
}

void BachelorThesis::changeLKIters( int _iters )
{
	std::cout << "LKOF: iters: " << _iters << std::endl;
	//this->lkflow.setIters( _iters );
}

void BachelorThesis::changeLKWinSize( int _winSize )
{
	std::cout << "LKOF: winSize: " << _winSize << std::endl;
	//this->lkflow.setWinSize( _winSize );
}

void BachelorThesis::changeLKMaxlevel( int _maxLevel )
{
	std::cout << "LKOF: maxLevel: " << _maxLevel << std::endl;
	//this->lkflow.setMaxLevel( _maxLevel );
}

void BachelorThesis::openSampleFile( void )
{
	//frameHandler.createNewOutput( "VIDEO_GPU", 0, cv::WINDOW_OPENGL );
	imageLabel.show();
	std::string fileName = "G:\\DB\\Dropbox\\BA\\code\\BachelorThesis\\BachelorThesis\\Fri_Oct_11_compilation.mp4";

	videoReader.open( fileName );
	ui.progressBarSlider->setMaximum( videoReader.getMaxFrames() );

}

void BachelorThesis::toggleMeanShiftFiltering( bool _doMeanShiftFiltering )
{
	this->doMeanShiftFiltering = _doMeanShiftFiltering;
}

void BachelorThesis::openHardwareInfoDialog( void )
{
	this->hardwareInfoDialog->show();
}

QImage BachelorThesis::mat2QImage( cv::Mat const& src )
{
	cv::Mat temp; // make the same cv::Mat
	cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
	QImage dest((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	QImage dest2(dest);
	dest2.detach(); // enforce deep copy
	return dest2;
}

void BachelorThesis::setRoi( int x, int y, int w, int h )
{
	this->roi.setRoi( x, y, w, h );
	std::cout << "I GOT A NEW ROI" << std::endl;
}
