#include "opencv2/videoio.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

int main()
{
    cv::VideoCapture capture(0);//打开摄像头，获取图像。
    if (capture.isOpened()) {
        cv::Mat frame;
        capture >> frame;
        imwrite("./haha.png", frame);
    }
}