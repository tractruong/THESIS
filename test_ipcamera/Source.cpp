#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include "camera\camera.hpp"
#include "pid\pid.hpp"
#include <string>
using namespace cv;
using namespace std;

Camera camera(0);

int main(int argc, char* argv[])
{
	camera.setSize(640, 480);
	camera.createTrackbars();
	while(1)
	{
		string error = camera.getErrorStr();
		if (error != "")
			cout << error << endl;

		camera.getFrame();
		camera.detectBall();
		camera.showCamera();

		if (waitKey(1) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			camera.~Camera();
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return 0;
}