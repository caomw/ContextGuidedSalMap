#include <stdio.h>
#include <iostream>
#include <cv.h>
#include <highgui.h>

#include "cvie.cpp"

using namespace std;

#define SD 0.2

IplImage *mySM(IplImage *img);

typedef struct{
	double L0;
	double L1;
	double R0;
	double R1;
} ParseInfo;

int main(int argc, char *argv[]){
	
	char* imagefile = argv[1];
	double context = atof(argv[2]);
	ParseInfo parse_info;
	parse_info.L0 = atof(argv[3]);
	parse_info.L1 = atof(argv[4]);
	parse_info.R0 = atof(argv[5]);
	parse_info.R1 = atof(argv[6]);
	
	int if_show_image = 0;
	if(argc==8 && !strcmp(argv[7],"s")){
		if_show_image = 1;
	}

	/*
	char* imagefile = "houses.JPG";
	double context = 0.801393;
	ParseInfo parse_info;
	parse_info.L0 = 395;
	parse_info.L1 = 489;
	parse_info.R0 = 199;
	parse_info.R1 = 516;
	*/
	IplImage *in_img = cvLoadImage( imagefile ,CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	if(in_img == NULL){
		printf("Can't Load File !\n");
		exit(0);
	}

	int sign = 0;
	context = context * in_img->height; 
	double R_context = context, L_context = context;
	if(parse_info.L1-parse_info.L0 > parse_info.R1-parse_info.R0){
		R_context = (context - parse_info.L0) * (parse_info.R1 - parse_info.R0) / (parse_info.L1 - parse_info.L0) + parse_info.R0;
		L_context = context;
		sign = 1;
	}
	else if(parse_info.L1-parse_info.L0<parse_info.R1-parse_info.R0){
		L_context = (context - parse_info.R0) * (parse_info.L1 - parse_info.L0) / (parse_info.R1 - parse_info.R0) + parse_info.L0;
		R_context = context;
		sign = -1;
	}

	IplImage * context_img = cvCreateImage(cvGetSize(in_img), IPL_DEPTH_32F, 1);
	double sigma, mu_Y;
	for(int y=0;y<context_img->height;y++){
		for(int x=0;x<context_img->width;x++){
			mu_Y = (R_context - L_context) / context_img->width * x + L_context;
			if(sign==1){
				sigma = context_img->height * SD - context_img->height * SD / 3 * x/(double)in_img->width; //hama
			}else if(sign==-1){
				sigma = context_img->height * SD / 3 * ( 2 + x/(double)in_img->width); //hama
			}
			else{
				sigma = context_img->height * SD;
			}
			cvImageElem<IPL_DEPTH_32F>(context_img, x, y, 0) = exp( -pow(y-mu_Y, 2) / (2 * pow(sigma, 2)) );
		}
	}

	IplImage *sm_img = mySM(in_img);
	IplImage *dst_img1 = cvCreateImage(cvGetSize(in_img), IPL_DEPTH_32F, 1);
	IplImage *dst_img2 = cvCreateImage(cvGetSize(in_img), 8, 1);
	IplImage *sm_img2 = cvCreateImage(cvGetSize(in_img), 8, 1);
	IplImage *context_img2 = cvCreateImage(cvGetSize(in_img), 8, 1);

	cvAddWeighted(context_img,0.15, sm_img,1, NULL, dst_img1);
	cvNormalize(dst_img1, dst_img2, 255.0, 0, CV_MINMAX);
	cvNormalize(sm_img, sm_img2, 255.0, 0, CV_MINMAX);
	cvConvertScale(context_img, context_img2, 255.0, 0);

	if(if_show_image){
		cvNamedWindow("in", 1);
		cvNamedWindow("context", 1);
		cvNamedWindow("sm", 1);
		cvNamedWindow("dst", 1);
		cvShowImage("in", in_img);
		cvShowImage("context", context_img2);
		cvShowImage("sm", sm_img2);
		cvShowImage("dst", dst_img2);
		cvWaitKey(0);
	}
	cvSaveImage( "context.jpg", context_img2, 0);
	cvSaveImage( "sm.jpg", sm_img2, 0);
	cvSaveImage( "dst.jpg", dst_img2, 0);

	cvReleaseImage(&in_img);
	cvReleaseImage(&context_img);
	cvReleaseImage(&sm_img);
	cvReleaseImage(&context_img2);
	cvReleaseImage(&sm_img2);
	cvReleaseImage(&dst_img1);
	cvReleaseImage(&dst_img2);

	if(if_show_image){
		cvDestroyWindow("in");
		cvDestroyWindow("context");
		cvDestroyWindow("sm");
		cvDestroyWindow("dst");
	}
}