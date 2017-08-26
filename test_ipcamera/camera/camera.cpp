#include "camera.hpp"

// Constructor

Camera::Camera(int id) :x_(0),
						y_(0),
						width_(640),
						height_(480),
						camera_(id),
						imgOriginal_(),
						imgHSV_(),
						imgThresholded_(),

						objMoments_(),
						objMoment01_(0.0),
						objMoment10_(0.0),
						objArea_(0.0),

						fail_(false),
						errorString_(""),

						lowH_(0),
						highH_(255),
						lowS_(0),
						highS_(255),
						lowV_(0),
						highV_(255) 
{
        if ( !camera_.isOpened() )  // If not success, exit program
        {
                this->setFail();
                this->setErrorStr( "Failed to open camera." );
        }
}

// Destructor
Camera::~Camera()
{
	width_ = -1;
	height_ = -1;
    x_ = -1;
    y_ = -1;

    camera_.~VideoCapture();

    imgOriginal_.~Mat();
    imgHSV_.~Mat();
    imgThresholded_.~Mat();

    objMoments_.~Moments();
    objMoment01_ = -1;
    objMoment10_ = -1;
    objArea_     = -1;

    fail_ = true;
    errorString_ = "";
}

void
Camera::setSize(int width, int height)
{
	width_ = width;
	height_ = height;
	camera_.set(CV_CAP_PROP_FRAME_WIDTH, width_);
	camera_.set(CV_CAP_PROP_FRAME_HEIGHT, height_);
	MAX_OBJECT_AREA = height_*width_ * 2 / 3;
}
void
Camera::getFrame()
{
	start_ = cv::getTickCount();
    int ret = 0;
    ret = camera_.read(imgOriginal_); // Reading new frame from video stream
    if ( !ret )
    {
            this->setFail();
            this->setErrorStr( "Failed to read frame from video stream." );
            return;
    }
}
void
Camera::detectBall()
{
	cv::cvtColor(imgOriginal_,imgHSV_,cv::COLOR_BGR2HSV);
	//filter HSV image between values and store filtered image to
	//threshold matrix
	cv::inRange(imgHSV_,cv::Scalar(lowH_,lowS_,lowV_),cv::Scalar(highH_,highS_,highV_),imgThresholded_);
	//perform morphological operations on thresholded image to eliminate noise
	//and emphasize the filtered object(s)
	morphOps(imgThresholded_);
	//pass in thresholded frame to our object tracking function
	//this function will return the x and y coordinates of the
	//filtered object
	trackFilteredObject(x_,y_,imgThresholded_,imgOriginal_);
	//show frames
}
void
Camera::showCamera()
{
	double fps = cv::getTickFrequency() / (cv::getTickCount() - start_);
	putText(imgOriginal_, "FPS : " + intToString((int)fps)   , cv::Point(10, height_ - 10), 2, 1, cv::Scalar(0, 255, 0), 2);
	cv::imshow( "Original", imgOriginal_); // Showing the original image
	cv::imshow("Threshold", imgThresholded_); // Showing the original image
}
int
Camera::getX()
{
        return x_;
}

int
Camera::getY()
{
        return y_;
}

void
Camera::setFail()
{
        fail_ = true;
}

bool
Camera::fail()
{
        return fail_;
}

void
Camera::setErrorStr( std::string errorString)
{
        errorString_ = errorString;
}

std::string
Camera::getErrorStr()
{
        return errorString_;
}
void 
Camera::drawObject(int x, int y, cv::Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, cv::Point(x, y), 20, cv::Scalar(0, 255, 0), 2);
	if (y - 25>0)
		line(frame, cv::Point(x, y), cv::Point(x, y - 25), cv::Scalar(0, 255, 0), 2);
	else line(frame, cv::Point(x, y), cv::Point(x, 0), cv::Scalar(0, 255, 0), 2);
	if (y + 25<height_)
		line(frame, cv::Point(x, y), cv::Point(x, y + 25), cv::Scalar(0, 255, 0), 2);
	else line(frame, cv::Point(x, y), cv::Point(x, height_), cv::Scalar(0, 255, 0), 2);
	if (x - 25>0)
		line(frame, cv::Point(x, y), cv::Point(x - 25, y), cv::Scalar(0, 255, 0), 2);
	else line(frame, cv::Point(x, y), cv::Point(0, y), cv::Scalar(0, 255, 0), 2);
	if (x + 25<width_)
		line(frame, cv::Point(x, y), cv::Point(x + 25, y), cv::Scalar(0, 255, 0), 2);
	else line(frame, cv::Point(x, y), cv::Point(width_, y), cv::Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), cv::Point(x, y + 30), 1, 1, cv::Scalar(0, 255, 0), 2);

}
void 
Camera::morphOps(cv::Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}
void 
Camera::trackFilteredObject(int &x, int &y, cv::Mat threshold, cv::Mat &cameraFeed) {

	cv::Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	std::vector< std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				cv::Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", cv::Point(0, 50), 2, 1, cv::Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", cv::Point(0, 50), 1, 2, cv::Scalar(0, 0, 255), 2);
	}
}
std::string 
Camera::intToString(int number) {
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void 
Camera::createTrackbars(){
	//create window for trackbars
    cv::namedWindow("TB",0);
	//create memory to store trackbar name on window
	/*char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN", lowH_);
	sprintf( TrackbarName, "H_MAX", highH_);
	sprintf( TrackbarName, "S_MIN", lowS_);
	sprintf( TrackbarName, "S_MAX", highS_);
	sprintf( TrackbarName, "V_MIN", lowV_);
	sprintf( TrackbarName, "V_MAX", highV_);*/
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
    cv::createTrackbar( "H_MIN", "TB", &lowH_, 255);
    cv::createTrackbar( "H_MAX", "TB", &highH_, 255);
    cv::createTrackbar( "S_MIN", "TB", &lowS_, 255);
    cv::createTrackbar( "S_MAX", "TB", &highS_, 255);
    cv::createTrackbar( "V_MIN", "TB", &lowV_, 255);
    cv::createTrackbar( "V_MAX", "TB", &highV_, 255);


}

void
Camera::setHSVParam(int lowH, int highH, int lowS, int highS, int lowV, int highV)
{
	lowH_ = lowH;
	highH_ = highH;
	lowS_ = lowS;
	highS_ = highS;
	lowV_ = lowV;
	highV_ = highV;
}