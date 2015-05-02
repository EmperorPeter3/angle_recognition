#include "opencv\cv.h"
#include "opencv\highgui.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

IplImage* image = 0;
IplImage* gray = 0;
IplImage* dst = 0;
IplImage* smooth = 0;

void houghLine(IplImage* original, float accuracy = 0.1)
{
	assert(original != 0);

	IplImage *src = 0, *rgb = 0;
	IplImage *bin = 0;
	IplImage *phase = 0;
	IplImage* angle = 0;

	char* filename = "C:\\Users\\Max\\Pictures\\angle.png";

	angle = cvLoadImage(filename, 1);

	src = cvCloneImage(original);

	// create the color image
	rgb = cvCreateImage(cvGetSize(src), 8, 3);
	cvConvertImage(src, rgb, CV_GRAY2BGR);

	// binary image - for contours
	bin = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCanny(src, bin, 50, 200);

	//cvNamedWindow( "bin", 1 );
	//cvShowImage( "bin", bin );

	// max distance from  origin - it's diagonal's length
	int RMax = cvRound(sqrt((double)(src->width*src->width + src->height*src->height)));

	// image for Hough's phase space (r, f)
	// 0 < r < RMax
	// 0 < f < 2*PI
	phase = cvCreateImage(cvSize(RMax, 180), IPL_DEPTH_16U, 1);
	cvZero(phase);

	int x = 0, y = 0, r = 0, f = 0;
	float theta = 0.0;

	// cycle for contour's pixels
	for (y = 0; y< bin->height; y++)
	{
		uchar* ptr = (uchar*)(bin->imageData + y * bin->widthStep);
		for (x = 0; x<bin->width; x++)
		{
			if (ptr[x]>0)
			{ // contour's pixel? consider all possible directs, which can pass through the point
				for (f = 0; f<180; f++)
				{ // all possible slope's angles
					short* ptrP = (short*)(phase->imageData + f * phase->widthStep);
					for (r = 0; r<RMax; r++)
					{ // all possible distance from origin
						theta = float(f*CV_PI / 180.0); // degrees -> radians

						// if solution is good (accuracy is more than in beginning)
						if (abs(((y)*sin(theta) + (x)*cos(theta)) - r) < accuracy)
						{
							ptrP[r]++; // counter ++ for this point
						}
					}
				}
			}
		}
	}

	//cvNamedWindow( "phase", 1 );
	//cvShowImage( "phase", phase );

	// increase the phase image
	IplImage* phaseImage = cvCreateImage(cvSize(phase->width * 3, phase->height * 3), IPL_DEPTH_16U, 1);
	cvResize(phase, phaseImage);

	//cvNamedWindow( "phaseImage", 1 );
	//cvShowImage( "phaseImage", phaseImage);

	// Choose a point in the phase space, which scored the highest number of hits
	unsigned int MaxPhaseValue = 0;
	float Theta = 0;
	int R = 0;
	for (f = 0; f<180; f++)
	{ // all possible slope's angles
		short* ptrP = (short*)(phase->imageData + f * phase->widthStep);
		for (r = 0; r<RMax; r++)
		{ // all possible distance from origin
			if (ptrP[r]>MaxPhaseValue)
			{
				MaxPhaseValue = ptrP[r];
				Theta = f;
				R = r;
			}
		}
	}

#if 1
	printf("[M] %d\n", MaxPhaseValue);

	// normalization
	float scaler = 0xFFFFFFFF / (float)MaxPhaseValue;
	for (y = 0; y<phaseImage->height; y++)
	{
		short* ptr = (short*)(phaseImage->imageData + y * phaseImage->widthStep);
		for (x = 0; x<phaseImage->width; x++)
		{
			ptr[x] *= scaler;
		}
	}
	//cvNamedWindow( "phaseImage2", 1 );
	//cvShowImage( "phaseImage2", phaseImage);
#endif

	// draw a line through the points for (R, Teta) (which we have in end of transform)
	Theta = Theta*CV_PI / 180.0;

	for (y = 0; y<angle->height; y++)
	{
		uchar* ptr = (uchar*)(angle->imageData + y * angle->widthStep);
		for (x = 0; x<angle->width; x++){
			if (cvRound(((y)* sin(Theta) + (x)* cos(Theta))) == R){
				ptr[3 * x] = 0;
				ptr[3 * x + 1] = 0;
				ptr[3 * x + 2] = 255;
			}
		}
	}

	cvNamedWindow("line", 1);
	cvShowImage("line", angle);

	// release resources
	cvReleaseImage(&src);
	cvReleaseImage(&rgb);

	cvReleaseImage(&bin);
	cvReleaseImage(&phase);
	cvReleaseImage(&phaseImage);
}

int angle_recognition(int argc, char* argv[])
{
	char* filename = argc == 2 ? argv[1] : "C:\\Users\\Max\\Pictures\\angle.png";
	image = cvLoadImage(filename, 1);

	printf("Image: %s\n", filename);
	cout << "Image size: " << cvGetSize(image).height << " x " << cvGetSize(image).width << endl;
	assert(image != 0);

	gray = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	dst = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);

	cvNamedWindow("original", CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("gray",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("cvCanny", CV_WINDOW_AUTOSIZE);

	//   into the gray's gradation
	cvCvtColor(image, gray, CV_RGB2GRAY);

	//   Canny's detector
	cvCanny(gray, dst, 300, 850, 5);


	//cvCanny( const CvArr* image, CvArr* edges, double threshold1, double threshold2, int  aperture_size CV_DEFAULT(3) );

	//image (gray) — одноканальное изображение для обработки (градации серого)
	//edges (dst) — одноканальное изображение для хранения границ, найденных функцией
	//threshold1 (10) — порог минимума
	//threshold2 (100) — порог максимума
	//aperture_size (3) — размер для оператора Собеля


	cvShowImage("original", image);
	//cvShowImage("gray",gray);
	cvShowImage("cvCanny", dst);

	// Hough's transform
	houghLine(dst);

	cvWaitKey(0);

	// release resources
	cvReleaseImage(&image);
	cvReleaseImage(&gray);
	cvReleaseImage(&dst);

	cvDestroyAllWindows();

	return 0;
}
