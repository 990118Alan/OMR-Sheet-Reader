#include<opencv2/opencv.hpp>
#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<vector>
#include<cmath>
#include<iomanip>
#include<algorithm>
#define fastio ios::sync_with_stdio(false),cin.tie(nullptr)
#define PI 3.1415926535
using namespace std;
using namespace cv;
bool compare(const pair<double, double>& a, const pair<double, double>& b) {
	return a.second > b.second;
}
int main(int argc, char* argv[])
{
	//第一部分 校正圖片
	//------------------------------------
	fastio;
	ofstream fout;
	fout.open(argv[2]);
	Mat image = imread(argv[1]);
	//灰化
	Mat GrayImage;
	cvtColor(image, GrayImage, COLOR_BGR2GRAY);
	// 邊緣檢測
	Mat edges;
	threshold(GrayImage, edges, 80, 255, THRESH_BINARY_INV);
	// 找到輪廓
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point> possible_point;
	vector<Point2f> four_corner(4);
	findContours(edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//resize(edges, edges, Size(edges.rows * 0.5, edges.cols * 0.5));
	//imshow("edgss", edges);
	//waitKey(0);

	// 繪製輪廓
	Mat contourImage = Mat::zeros(edges.size(), CV_8UC3);
	for (size_t i = 0; i < contours.size(); ++i) {
		Scalar color = Scalar(0, 255, 0); // 輪廓顏色：綠色
		double area = contourArea(contours[i]);
		double perimeter = arcLength(contours[i], true);
		Rect boundingRect = cv::boundingRect(contours[i]);
		double height = boundingRect.height > boundingRect.width ? boundingRect.height : boundingRect.width;
		double width = boundingRect.height > boundingRect.width ? boundingRect.height : boundingRect.width;

		if ((height / width) < 1.1 && (height / width) > 0.9 && perimeter > 30 && area > 30 && area < 1000) {
			// 輪廓周長
			Mat rectangle = edges(boundingRect);
			//imshow("rectangle", rectangle);
			//waitKey(0);
			int whitePixelCount = countNonZero(rectangle);
			if (whitePixelCount > (rectangle.total() / 1.8)) {
				possible_point.push_back(contours[i][0]);//插入符合條件的點
			}
		}
	}
	//resize(contourImage, contourImage, Size(contourImage.rows * 0.5, contourImage.cols * 0.5));
	//imshow("edgess", contourImage);
	//waitKey(0);
	//插入四個定位點
	double left_top = 10000, left_down = 10000, right_down = 10000, right_top = 10000;
	for (int i = 0; i < possible_point.size(); i++) {
		if (sqrt(pow(possible_point[i].x, 2) + pow(possible_point[i].y, 2)) < left_top) {
			left_top = sqrt(pow(possible_point[i].x, 2) + pow(possible_point[i].y, 2));
			four_corner[0] = possible_point[i];
		}
		if (sqrt(pow(possible_point[i].x, 2) + pow(possible_point[i].y - 1476, 2)) < left_down) {
			left_down = sqrt(pow(possible_point[i].x, 2) + pow(possible_point[i].y - 1476, 2));
			four_corner[2] = possible_point[i];
		}
		if (sqrt(pow(possible_point[i].x - 1476, 2) + pow(possible_point[i].y, 2)) < right_top) {
			right_top = sqrt(pow(possible_point[i].x - 1476, 2) + pow(possible_point[i].y, 2));
			four_corner[1] = possible_point[i];
		}
		if (sqrt(pow(possible_point[i].x - 1476, 2) + pow(possible_point[i].y - 1476, 2)) < right_down) {
			right_down = sqrt(pow(possible_point[i].x - 1476, 2) + pow(possible_point[i].y - 1476, 2));
			four_corner[3] = possible_point[i];
		}
	}
	//four_corner[0] = (possible_point[possible_point.size() - 2].x < possible_point[possible_point.size() - 1].x) ? possible_point[possible_point.size() - 2] : possible_point[possible_point.size() - 1];
	//four_corner[1] = (possible_point[possible_point.size() - 2].x < possible_point[possible_point.size() - 1].x) ? possible_point[possible_point.size() - 1] : possible_point[possible_point.size() - 2];
	//four_corner[2] = (possible_point[0].x < possible_point[1].x) ? possible_point[0] : possible_point[1];
	//four_corner[3] = (possible_point[0].x < possible_point[1].x) ? possible_point[1] : possible_point[0];
	//取得透視矩陣

	vector<Point2f> des_corner(4);
	des_corner[0] = Point(0, 0);
	des_corner[1] = Point(image.cols, 0);
	des_corner[2] = Point(0, image.rows);
	des_corner[3] = Point(image.cols, image.rows);
	//取得透視變換矩陣
	Mat warpmatrix = getPerspectiveTransform(four_corner, des_corner);
	Mat warpImage(image);
	//進行透視變換
	warpPerspective(image, warpImage, warpmatrix, image.size());
	//-----------------------------------------------------------------
	// 第二部分 讀取答案
	//resize(contourImage, contourImage, Size(contourImage.rows * 0.5, contourImage.cols * 0.5));
	//resize(warpImage, warpImage, Size(warpImage.rows * 0.5, warpImage.cols * 0.5));
	//imshow("contourImage", contourImage);
	//imshow("warp", warpImage);
	//waitKey(0);
	//灰化
	cvtColor(warpImage, GrayImage, COLOR_BGR2GRAY);
	//2值化
	threshold(GrayImage, edges, 110, 255, THRESH_BINARY_INV);

	//找輪廓
	contours.clear();
	hierarchy.clear();
	findContours(edges, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	contourImage = Mat::zeros(edges.size(), CV_8UC3);
	vector<int> split_line;
	for (size_t i = 0; i < contours.size(); ++i) {
		Scalar color = Scalar(0, 0, 255); // 輪廓顏色：綠色
		double area = contourArea(contours[i]);
		double perimeter = arcLength(contours[i], true);
		if (area > 1500 && area < 6000) {//尋找中間分隔線
			split_line.push_back(contours[i][0].y);
		}
	}

	vector<Point> reference_point;

	//插入左側參考點
	for (size_t i = 0; i < contours.size(); i++) {
		Scalar color = Scalar(0, 0, 255); // 輪廓顏色：綠色
		double area = contourArea(contours[i]);
		double perimeter = arcLength(contours[i], true);
		if (perimeter < 100 && perimeter >40 && area > 50 && area < 250 && contours[i][0].y < split_line[0] && contours[i][0].y > split_line[1] && contours[i][0].x < 200 && contours[i][0].x > 60) {
			Rect boundingRect = cv::boundingRect(contours[i]);
			Mat rectangle = edges(boundingRect);
			int whitePixelCount = countNonZero(rectangle);
			if (whitePixelCount > (rectangle.total() / 1.7)) {
				reference_point.push_back(contours[i][0]);//插入符合條件的點
			}
		}
	}

	//尋找與參考點相同高度的點
	vector<double> Answer_area[27];
	for (size_t i = 0; i < contours.size(); ++i) {
		Scalar color = Scalar(0, 0, 255); // 輪廓顏色：綠色
		double area = contourArea(contours[i]);
		double perimeter = arcLength(contours[i], true);
		if (perimeter < 100 && perimeter > 30 && area > 30 && area < 250 && contours[i][0].y < split_line[0] && contours[i][0].y > split_line[1]) {
			Rect boundingRect = cv::boundingRect(contours[i]);
			Mat rectangle = edges(boundingRect);
			//imshow("rectangle", rectangle);
			//waitKey(0);
			int whitePixelCount = countNonZero(rectangle);
			if (whitePixelCount > (rectangle.total() / 1.6)) {
				for (int j = 3; j < 27; j++) {
					if (fabs(contours[i][0].y - reference_point[j].y) < 15 && fabs(contours[i][0].x - reference_point[j].x) > 1e-8) {
						if (Answer_area[j].size() >= 1) {
							if (abs(contours[i][0].x - Answer_area[j][Answer_area[j].size() - 1]) > 5) {
								Answer_area[j].push_back(contours[i][0].x);
							}
						}
						else {
							Answer_area[j].push_back(contours[i][0].x);
						}
					}
				}
			}
			// 輪廓周長
			//cout << "輪廓周長：" << perimeter << endl;
			//插入符合條件的點
		}
	}

	string Ans_str = "";
	for (int i = 26; i > 2; i--) {
		if (Answer_area[i].size() == 1) {
			if (240 <= Answer_area[i][0] && Answer_area[i][0] <= 310) {
				Ans_str += "1";
			}
			else if (310 < Answer_area[i][0] && Answer_area[i][0] <= 360) {
				Ans_str += "2";
			}
			else if (360 < Answer_area[i][0] && Answer_area[i][0] <= 420) {
				Ans_str += "3";
			}
			else if (420 < Answer_area[i][0] && Answer_area[i][0] <= 480) {
				Ans_str += "4";
			}
			else if (480 < Answer_area[i][0] && Answer_area[i][0] <=540) {
				Ans_str += "5";
			}
			else if (540 < Answer_area[i][0] && Answer_area[i][0] <= 610) {
				Ans_str += "6";
			}
			else if (610 < Answer_area[i][0] && Answer_area[i][0] <= 670) {
				Ans_str += "7";
			}
			else if (670 < Answer_area[i][0] && Answer_area[i][0] <= 730) {
				Ans_str += "8";
			}
			else if (730 < Answer_area[i][0] && Answer_area[i][0] <= 800) {
				Ans_str += "9";
			}
			else if (800 < Answer_area[i][0] && Answer_area[i][0] <= 860) {
				Ans_str += "0";
			}
			else if (860 < Answer_area[i][0] && Answer_area[i][0] <= 920) {
				Ans_str += "A";
			}
			else if (920 < Answer_area[i][0] && Answer_area[i][0] <= 970) {
				Ans_str += "B";
			}
		}
		else if (Answer_area[i].size() > 1) {
			Ans_str += "M";
		}
		else if (Answer_area[i].size() == 0) {
			Ans_str += "X";
		}
	}
	fout << Ans_str << endl;

	//fout.close();
}