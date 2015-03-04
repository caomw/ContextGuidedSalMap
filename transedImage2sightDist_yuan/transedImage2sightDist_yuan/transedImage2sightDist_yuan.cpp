// transedImage2sightDist_yuan.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <windows.h>

#include <cv.h>
#include <highgui.h>

#include "cvgabor.h"

using namespace std;

#define FEATURE_FILE "FeatureVector.txt"
#define IMAGE_FILE "1.jpg"
#define ENABLE_SHOW_IMAGE 0

#define N_SCALE 4
#define N_ORIENTATION 6
#define N_SEGMENT 4
#define GAUSSIAN_SIGMA_OFFSET 1
#define GAUSSIAN_SIGMA 2*PI

#define N_FEATURE 384
#define N_PC 25
#define PCA_AVG_FILE "pca_avg.txt"
#define PCA_SIGMA_FILE "pca_sigma.txt"
#define PCA_W_FILE "pca_w.txt"
#define PCA_PC_FILE "pca_pc.txt"
#define GSM_FILE "Param_GSM.txt"
#define N_GSM 3
#define REGRESSION_FILE "regression_param.txt"

#define SD 0.2

#define CONTEXT_FILE "context.txt" 

typedef struct {
	float alpha;
	float mu[N_PC];
	float covariance[N_PC][N_PC];
} Param_GSM;

typedef struct {
	float mean;
	float B[N_PC];
} Param_reg;

void PrintMatrix(CvMat *Matrix,int Rows,int Cols,int Channels)
{
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            for(int k=0;k<Channels;k++)
            {
                printf("%.2f ",cvGet2D(Matrix,i,j).val[k]);
            }
        }
        printf("\n");
    }
}

void SampleFilter(IplImage *img, int p[N_SEGMENT][N_SEGMENT]){
	//double p[N][N]
	double x, y;
	int i, j, pos, count;

	for(j=0; j<N_SEGMENT; j++){
		for(i=0; i<N_SEGMENT; i++){
			count = 0;
			p[i][j] = 0.0; //‰Šú‰»
			for(y=img->height/N_SEGMENT*j; y < img->height/N_SEGMENT*(j+1); y++){
				for(x=img->width/N_SEGMENT*i; x < img->width/N_SEGMENT*(i+1); x++){
					pos = (y * img->widthStep) + (x * img->nChannels);
					p[i][j] += img->imageData[pos];
					count++;
				}
			}
			p[i][j] = p[i][j] / count;
		}
	}
}

int main(int argc, char* argv[])
{
	// Load image
	char* filename = IMAGE_FILE;
	int if_show_image = ENABLE_SHOW_IMAGE;
	if(argc >= 2){
		filename = argv[1];
	}
	if(argc == 3 && !strcmp(argv[2],"s")){
		cout << argv[2] <<endl;
		if_show_image = 1;
	}
	ifstream fin_test(filename);
	if (!fin_test){
		cout<<"Error: Image "<<filename<<" not found."<<endl;
		return 2;
	}
	cout<<"Load image "<<filename<<endl;
	IplImage *in_img = cvLoadImage (filename, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	CvSize in_size = cvGetSize(in_img);
	if(if_show_image){
		cvNamedWindow("in", 1);
		cvShowImage("in", in_img);
		cvWaitKey(0);
		cvDestroyWindow("in");
	}
	
	// Convert image to GRAY
	cout<<"Convert image to GRAY."<<endl;
	IplImage *gray_img = cvCreateImage(in_size,IPL_DEPTH_8U,1);
	cvCvtColor(in_img, gray_img, CV_BGR2GRAY);
	if(if_show_image){
		cvNamedWindow("gray", 1);
		cvShowImage("gray", gray_img);
		cvWaitKey(0);
		//cvDestroyWindow("gray");
	}

	// Generate feature images
	cout<<"Generate feature images with "<<N_SCALE<<" scales, each with "<<N_ORIENTATION<<" orientations..."<<endl;
	IplImage *feature_imgs[N_SCALE][N_ORIENTATION];
	CvGabor *gabor = new CvGabor;
	for(int i=0; i<N_SCALE; i++){
		cout<<"Scale "<<i+1<<endl;
		for(int j=0; j<N_ORIENTATION; j++){
			feature_imgs[i][j] = cvCreateImage(in_size, IPL_DEPTH_8U, 1);
			gabor->Init(PI*j/N_ORIENTATION, GAUSSIAN_SIGMA_OFFSET, GAUSSIAN_SIGMA, sqrt(0.5*(i+1)));
			gabor->conv_img(gray_img, feature_imgs[i][j], CV_GABOR_MAG);
		}
	}
	
	// Calculate and save the feature vector
	cout<<"Calculate and save the feature vector..."<<endl;
	int sf[N_SEGMENT][N_SEGMENT];
	ofstream fout_feature;
	fout_feature.open(FEATURE_FILE);
	for(int j=0; j<N_ORIENTATION; j++){
		for(int i=0; i<N_SCALE; i++){
			SampleFilter(feature_imgs[i][j], sf);
			for(int x=0; x<N_SEGMENT; x++){
				for(int y=0; y<N_SEGMENT; y++){
					fout_feature << sf[x][y] << " ";
					//cout << sf[x][y] << ",";
				}
			}
		}
	}
	fout_feature.close();
	
	// Release the memory of the images
	cvReleaseImage(&in_img);
	//cvReleaseImage(&gray_img);
	for(int i=0; i<N_SCALE; i++){
		for(int j=0; j<N_ORIENTATION; j++){
			cvReleaseImage(&feature_imgs[i][j]);
		}
	}

	// Calculate the principle component
	cout<<"Calculate and save the principle component"<<endl;
	float *array1 = new float[N_FEATURE]();
	float *array2 = new float[N_FEATURE*N_FEATURE]();
	float *pca_avg = new float[N_FEATURE]();
	float *pca_sigma = new float[N_FEATURE]();
	
	/* center x */
	ifstream fin_pca_avg, fin_pca_sigma, fin_pca_x;
	fin_pca_avg.open(PCA_AVG_FILE);
	fin_pca_sigma.open(PCA_SIGMA_FILE);
	fin_pca_x.open(/*"testVector.txt"*/FEATURE_FILE);
	for(int i=0;i<N_FEATURE;i++){
		fin_pca_avg >> pca_avg[i];
		fin_pca_sigma >> pca_sigma[i];
		fin_pca_x >> array1[i];
		array1[i] = (array1[i] - pca_avg[i])/pca_sigma[i];	// centered
	}
	fin_pca_avg.close();
	fin_pca_sigma.close();
	fin_pca_x.close();

	ifstream fin_pca_w;
	fin_pca_w.open(PCA_W_FILE);
	for(int i=0;i<N_FEATURE*N_FEATURE;i++){
		fin_pca_w >> array2[i];
	}
	fin_pca_w.close();

	CvMat *X = cvCreateMat(N_FEATURE,1,CV_32FC1);
    CvMat *W = cvCreateMat(N_FEATURE,N_FEATURE,CV_32FC1);
    CvMat *ResultMatrix = cvCreateMat(N_FEATURE,1,CV_32FC1);
	cvSetData(X,array1,X->step);
    cvSetData(W,array2,W->step);
	cvmMul(W,X,ResultMatrix);
	
	ofstream fout_pc;
	fout_pc.open(PCA_PC_FILE);
	float pc[N_PC];
	for(int i=0;i<N_PC;i++)
    {
        pc[i] = cvGet2D(ResultMatrix,i,0).val[0];
		//cout<<pc[i]<<endl;
		fout_pc << pc[i] <<" ";
    }
	fout_pc.close();
	delete [] array1;
	delete [] array2;
	delete [] pca_avg;
	delete [] pca_sigma;
	cvReleaseMat(&X);
	cvReleaseMat(&W);
	cvReleaseMat(&ResultMatrix);

	//calculate each GSM probability and choose one as its cluster
	Param_GSM *p_gsm = new Param_GSM[3];
	
	ifstream fin_gsm;
	fin_gsm.open(GSM_FILE);
	for(int i=0;i<N_GSM;i++){
		fin_gsm >> p_gsm[i].alpha;
		for(int j=0;j<N_PC;j++){
			fin_gsm >> p_gsm[i].mu[j];
		}
		for(int r=0;r<N_PC;r++){
			for(int c=0;c<N_PC;c++){
				fin_gsm >> p_gsm[i].covariance[r][c];
			}
		}
	}
	fin_gsm.close();
	
	float prob[N_GSM];
	float model_max_prob = 0.0;
	int model_index = 0;
	for(int i=0;i<N_GSM;i++){
		CvMat *Xpc = cvCreateMat(1,N_PC,CV_32FC1);
		cvSetData(Xpc,pc,Xpc->step);
		CvMat *mu = cvCreateMat(1,N_PC,CV_32FC1);
		cvSetData(mu,p_gsm[i].mu,mu->step);
		CvMat *X_minus_mu = cvCreateMat(1,N_PC,CV_32FC1);
		cvSub(Xpc, mu, X_minus_mu); // X - mu

		CvMat *X_minus_mu_t = cvCreateMat(N_PC,1,CV_32FC1);
		cvTranspose(X_minus_mu, X_minus_mu_t); // (X - mu)t

		CvMat *sigma = cvCreateMat(N_PC,N_PC,CV_32FC1);
		cvSetData(sigma,p_gsm[i].covariance,sigma->step);
		CvMat *sigma_inv = cvCreateMat(N_PC,N_PC,CV_32FC1);
		cvInvert(sigma,sigma_inv); // inv(sigma)
		
		CvMat *tmp_matrix = cvCreateMat(1,N_PC,CV_32FC1);
		cvmMul(X_minus_mu,sigma_inv,tmp_matrix);

		CvMat *result_value = cvCreateMat(1,1,CV_32FC1);
		cvmMul(tmp_matrix,X_minus_mu_t,result_value);
		float re = cvGet2D(result_value,0,0).val[0];

		double det = cvDet(sigma);

		prob[i] = 1/( sqrt(det) * pow(2*PI,N_PC/2) ) * exp(-0.5*re);
		if(prob[i] > model_max_prob){
			model_max_prob = prob[i];
			model_index = i;
		}

		cout<<re<<" "<<prob[i]<<endl;

		cvReleaseMat(&Xpc);
		cvReleaseMat(&mu);
		cvReleaseMat(&X_minus_mu);
		cvReleaseMat(&X_minus_mu_t);
		cvReleaseMat(&sigma);
		cvReleaseMat(&sigma_inv);
		cvReleaseMat(&tmp_matrix);
		cvReleaseMat(&result_value);
	}
	cout<<"model index(0-2) is: "<<model_index<<endl;
	delete [] p_gsm;

	//calculate average sight
	Param_reg *param_reg = new Param_reg[3];
	ifstream fin_reg;
	fin_reg.open(REGRESSION_FILE);
	for(int i=0;i<N_GSM;i++){
		fin_reg >> param_reg[i].mean;
		for(int j = 0;j < N_PC;j++){
			fin_reg >> param_reg[i].B[j];
		}
	}
	fin_reg.close();

	float pc_multiply_B = 0.0;
	for(int i = 0;i<N_PC;i++){
		pc_multiply_B += pc[i] * param_reg[model_index].B[i];
	}
	float mean_sight = param_reg[model_index].mean + pc_multiply_B;

	cout<<"mean sight is: "<<mean_sight<<endl;


	delete [] param_reg;
	
	ofstream fout_context;
	fout_context.open(CONTEXT_FILE, ofstream::out | ofstream::app);
	fout_context << filename << "," << mean_sight <<endl;
	fout_pc.close();

	// generate context image
	// we don't need to create this image; instead, we can first calculate the transformed context position with 'mean_sight'.
	if(if_show_image){
		IplImage *context_img = cvCreateImage(in_size, 8, 1);
		IplImage *dst_img = cvCreateImage(in_size, 8, 1);
		double tmp;
		for(int y=0;y<context_img->height;y++){
			for(int x=0;x<context_img->width;x++){
				tmp = 255 * exp( -pow(y-mean_sight*context_img->height, 2) / (2 * pow(SD*context_img->height, 2)) );
				((uchar*)(context_img->imageData + context_img->widthStep*y))[x] = tmp;
			}
		}
		cvNamedWindow("context", 1);
		cvShowImage("context", context_img);
		cvWaitKey(0);
		cvAddWeighted(context_img,0.6, gray_img,1, NULL, dst_img); // combine gray and context
		cvNormalize(dst_img, dst_img, 255.0, 0, CV_MINMAX);
		cvNamedWindow("dist", 1);
		cvShowImage("dist", dst_img);
		cvWaitKey(0);
		cvReleaseImage(&context_img);
		cvReleaseImage(&dst_img);
	}
	cvReleaseImage(&gray_img);
	//system("pause");
	return 0;
}