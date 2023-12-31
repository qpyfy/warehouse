#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
// #include<opencv2/opencv_modules.hpp>
// #include<opencv2/imgproc.hpp>
// #include<opencv2/highgui.hpp>
//#include<opencv2/imgcodecs.hpp>
// #include<opencv2/objdetect.hpp>
#include<iostream>
#include <math.h>
using namespace std;
using namespace cv;

VideoCapture cap1("/home/tang/opencv/ood_blue.mp4");
Mat img;
Mat kernel = getStructuringElement(MORPH_RECT, Size(7, 7));
Mat minkernel = getStructuringElement(MORPH_RECT, Size(7, 7));

vector<Point> point(Point2f *biggest)
{
    vector<Point> NewPoint;
    vector<int> sumPoint, subPoint;
    for (int i = 0; i < 4; i++)
    {
        sumPoint.push_back(biggest[i].x + biggest[i].y);
        subPoint.push_back(biggest[i].x - biggest[i].y);
    }
    NewPoint.push_back(biggest[min_element(sumPoint.begin(), sumPoint.end()) - sumPoint.begin()]);
    NewPoint.push_back(biggest[max_element(subPoint.begin(), subPoint.end()) - subPoint.begin()]);
    NewPoint.push_back(biggest[min_element(subPoint.begin(), subPoint.end()) - subPoint.begin()]);
    NewPoint.push_back(biggest[max_element(sumPoint.begin(), sumPoint.end()) - sumPoint.begin()]);
    return NewPoint;
}
int main()
{
    int a = 100;
    int hmax = 109, smax = 255, vmax = 255;
    int hmin = 21, smin = 0, vmin = 100;
    /* int hmax = 10, smax = 255, vmax = 255;
     int hmin = 0, smin = 46, vmin = 46;*/
    //namedWindow("test");
    //createTrackbar("hmax", "test", &hmax, 179);
    //createTrackbar("smax", "test", &smax, 255);
    //createTrackbar("vmax", "test", &vmax, 255);
    //createTrackbar("hmin", "test", &hmin, 179);
    //createTrackbar("smin", "test", &smin, 255);
    //createTrackbar("vmin", "test", &vmin, 255);
    //createTrackbar("a", "test", &a, 1000);
    //Mat imgHSV, imgGaussian;

    while (1)
    {

        cap1.open("/home/tang/opencv/ood_blue.mp4");

        while (cap1.read(img))
        {

            // cvtColor(img,imgHSV ,COLOR_RGB2HSV);
            // GaussianBlur(imgHSV, imgGaussian, Size(9, 9), 9);
            // Scalar lower(hmin, smin, vmin);
            // Scalar upper(hmax, smax, vmax);
            ////二值化
            // Mat imgRange;
            // inRange(imgHSV, lower, upper, imgRange);
            double time1 = (double)getTickCount();

            Mat imgGray;
            cvtColor(img, imgGray, COLOR_BGR2GRAY);
            Mat imgBinaryGray;
            threshold(imgGray, imgBinaryGray, 137, 255, THRESH_BINARY);
            GaussianBlur(imgBinaryGray, imgBinaryGray, Size(3, 3), 0);
            // imshow("G", imgBinaryGray);
            vector<Mat> channl;
            split(img, channl);
            Mat channlb;
            subtract(channl[0], channl[2], channlb);
            //blur(channlb, channlb, Size(3, 3));
            Mat imgBinarychannl;
            threshold(channlb, imgBinarychannl, 70, 255, THRESH_BINARY);
            // dilate(imgBinarychannl, imgBinarychannl, kernel);
            Mat imgEnd = imgBinarychannl & imgBinaryGray;
            // imshow("End", imgEnd);

            Mat imgCanny;
            Canny(imgEnd, imgCanny, 50, 100);

            // imshow("HSV", imgHSV);
            vector<Vec4i> hierarchy;
            vector<vector<Point>> counter;
            dilate(imgCanny, imgCanny, kernel);
            erode(imgCanny, imgCanny, kernel);

            // erode(imgCanny, imgCanny, minkernel);
            // dilate(imgCanny, imgCanny, kernel);
            findContours(imgCanny, counter, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            // 获取轮廓
            vector<RotatedRect> rectarr(counter.size());
            vector<vector<Point>> conPoly(counter.size());

            for (int i = 0; i < counter.size(); i++)
            {

                float area = contourArea(counter[i]);
                if (area > 100)
                {
                    float peri = arcLength(counter[i], true);
                    approxPolyDP(counter[i], conPoly[i], 0.02 * peri, true);
                }
            }

            for (int i = 0; i < conPoly.size(); i++)
            {

                if (conPoly[i].size() != 0)
                {
                    RotatedRect temp = minAreaRect(conPoly[i]);
                    // if (temp.boundingRect().height < temp.boundingRect().width)continue;
                    rectarr[i] = temp;
                    Point2f *rectpoint = new Point2f[4];
                    rectarr[i].points(rectpoint);
                    for (int j = 0; j < 4; j++)
                    {

                        line(img, rectpoint[j % 4], rectpoint[(j + 1) % 4], Scalar(0, 0, 255), 2);
                    }
                }
            }
            // 匹配
            int flag[100] = {0};
            for (int i = 0; i < rectarr.size(); i++)
            {
                int best = -1;
                float mindistance, mink, minangle;
                if (flag[i] == 1)
                    continue;
                for (int j = i; j < rectarr.size(); j++)
                {
                    if (rectarr[i].center.y - rectarr[j].center.y > rectarr[i].boundingRect().height)
                        continue;
                    float angle = rectarr[i].angle - rectarr[j].angle;
                    if (angle < 10 && angle > -10)
                    {
                        float distance = sqrt(pow(rectarr[i].center.x - rectarr[j].center.x, 2) + pow(rectarr[i].center.y - rectarr[j].center.y, 2));
                        if (distance < rectarr[i].boundingRect().height * 3 && distance > 10)
                        {
                            float k = (rectarr[i].boundingRect().width / rectarr[i].boundingRect().height) - (rectarr[j].boundingRect().width / rectarr[j].boundingRect().height);
                            if (k < 10 && k > -10)
                            {

                                if (best == -1)
                                {
                                    best = j;
                                    mindistance = distance;
                                    mink = k;
                                    minangle = angle;
                                }
                                else if (angle < minangle)
                                {
                                    best = j;
                                    mindistance = distance;
                                    mink = k;
                                    minangle = angle;
                                }
                                else if (distance < mindistance)
                                {
                                    best = j;
                                    mindistance = distance;
                                    mink = k;
                                    minangle = angle;
                                }

                                else if (k < mink)
                                {
                                    best = j;
                                    mindistance = distance;
                                    mink = k;
                                    minangle = angle;
                                }
                            }
                        }
                    }
                }
                if (best != -1 && flag[best] != 1)
                {

                    Point2f *startpoint = new Point2f[4], *endpoint = new Point2f[4];
                    rectarr[i].points(startpoint);
                    rectarr[best].points(endpoint);
                    vector<Point> start = point(startpoint);
                    vector<Point> end = point(endpoint);
                    // line(img, rectarr[i].center, rectarr[best].center, Scalar(255, 0, 0), 3);
                    line(img, start[0], end[0], Scalar(255, 0, 0), 3);
                    line(img, start[3], end[3], Scalar(0, 255, 0), 3);
                    flag[i] = 1;
                    flag[best] = 1;
                }
            }

            // 将没找到与最近的相连
            for (int i = 0; i < rectarr.size(); i++)
            {
                if (flag[i] == 0)
                {
                    int best = -1;
                    float mindistance = -1;
                    for (int j = 0; j < rectarr.size(); j++)
                    {
                        if (rectarr[i].center.y - rectarr[j].center.y > rectarr[i].boundingRect().height)
                            continue;
                        if (flag[j] != 1 && j != i)
                        {
                            float distance = sqrt(pow(rectarr[i].center.x - rectarr[j].center.x, 2) + pow(rectarr[i].center.y - rectarr[j].center.y, 2));
                            if (mindistance == -1 && distance < 200)
                            {
                                mindistance = distance;
                                best = j;
                            }
                            else if (mindistance > distance && distance < 200)
                            {
                                mindistance = distance;
                                best = j;
                            }
                        }
                    }
                    if (best != -1)
                    {
                        Point2f *startpoint = new Point2f[4], *endpoint = new Point2f[4];
                        rectarr[i].points(startpoint);
                        rectarr[best].points(endpoint);
                        vector<Point> start = point(startpoint);
                        vector<Point> end = point(endpoint);
                        // line(img, rectarr[i].center, rectarr[best].center, Scalar(255, 0, 0), 3);
                        line(img, start[0], end[0], Scalar(255, 0, 0), 3);
                        line(img, start[3], end[3], Scalar(0, 255, 0), 3);
                        flag[best] = 1;
                        flag[i] = 1;
                    }
                }
            }

            time1 = (double)getTickCount() - time1;
            double t = time1  / (double)getTickFrequency();

            putText(img, to_string((int)(1.0 / t)), Point(70, 70), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255, 0, 0));
            imshow("img", img);
            waitKey(1);
        }
    }
}
