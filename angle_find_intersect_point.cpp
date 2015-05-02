#include "opencv\cv.h"
#include "opencv\highgui.h"
#include <math.h>

using namespace cv;
using namespace std;

//Функция поиска пересечения линий
Point2f computeIntersect(Vec4i a, Vec4i b)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
	{
		Point2f pt;
		pt.x = ((x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4)) / d;
		pt.y = ((x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4)) / d;
		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}

int main(int argc, char** argv)
{
	Mat src, dst, color_dst;

	//Имя картинки задаётся первым параметром
	char* filename = argc >= 2 ? argv[1] : "C:\\Users\\Max\\Pictures\\angle2.jpg";

	//Вариант 1 - загрузка цветной картинки
	//src = imread(filename);

	//Вариант 2 - загрузка картинки и преобразование её в чб
	src = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);

	//Детектируем границы картинки src и выдаем полученное в dst
	Canny(src, dst, 80, 150, 3);

	//Размываем
	GaussianBlur(dst, dst, Size(5,5), 1);

	//конвертируем изображение dst в цветное color_dst
	cvtColor(dst, color_dst, CV_GRAY2BGR);

	/* standart Hough transform
	vector<Vec2f> lines;
	HoughLines(dst, lines, 1, CV_PI / 180, 100);

	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0];
		float theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		Point pt1(cvRound(x0 + 1000 * (-b)),
			cvRound(y0 + 1000 * (a)));
		Point pt2(cvRound(x0 - 1000 * (-b)),
			cvRound(y0 - 1000 * (a)));
		line(color_dst, pt1, pt2, Scalar(0, 0, 255), 3, 8);
	}
	*/

	// probabilistic Hough transform
	vector<Vec4i> lines;		//вектор для хранения линий
	HoughLinesP(dst, lines, 1, CV_PI / 180, 80, 30, 10);

	//Рисуем линии по точкам
	for (size_t i = 0; i < lines.size(); i++)
	{
		line(color_dst, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 1, 8);
	}
	
	//Создаем вектор точек пересечения линий
	vector<Point2f> corners;
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = i + 1; j < lines.size(); j++)
		{
			Point2f pt = computeIntersect(lines[i], lines[j]);
			if (pt.x >= 0 && pt.y >= 0)
			{
				corners.push_back(pt);
			}
				
		}
	}

	//Рисуем точки пересечения линий
	for (size_t i = 0; i < corners.size(); i++)
	{
		Point center(corners[i].x, corners[i].y);
		circle(color_dst, center, 5, Scalar(255, 0, 0));
	}

	namedWindow("Source", 1);
	imshow("Source", src);

	namedWindow("Detected Lines", 1);
	imshow("Detected Lines", color_dst);

	waitKey(0);

	// освобождаем ресурсы
	cvDestroyAllWindows();

	system("PAUSE");

	return 0;
}
