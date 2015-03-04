#include "cvgabor.h"
#include "cvie.cpp"

#define N 10 //正規化時の局地的領域の個数
//#undef DEBUG 
#define DEBUG

//my正規化関数
IplImage *myNorm(IplImage *img){
	double p[N][N], max, min;
	int x, y;
	int z, w;
	double maxsum;
	double avg, C;

	cvMinMaxLoc(img, &min, &max);
	for(w=0; w<N; w++){
		for(z=0; z<N; z++){
			p[z][w]=0.0;
			for(y=img->height*0.1*w; y < img->height*0.1*(w+1); y++){
				for(x=img->width*0.1*z; x < img->width*0.1*(z+1); x++){
					if(p[z][w] < cvImageElem<IPL_DEPTH_32F>(img, x, y, 0)) p[z][w] = cvImageElem<IPL_DEPTH_32F>(img, x, y, 0);
					//if(p[z][w] < CV_IMAGE_ELEM(img, float, y, x)) p[z][w] = CV_IMAGE_ELEM(img, float, y, x);
				}
			}
		}
	}
    maxsum=0.0;
	for(w=0;w<N;w++){
		for(z=0;z<N;z++){
			maxsum = maxsum + p[w][z];
		}
	}
	avg = (maxsum -max) /(N*N-1);
	//double C = pow(max - avg, 2);
	C = pow(1 - avg/max, 2);
	cvConvertScale(img, img, C);
	return 0;
}

//-----------------Main関数------------------//
//IplImage *mySM(IplImage *tmp_img)
IplImage *mySM(IplImage *tmp_img)
{
	IplImage *OUT_Img, *Intensity, *Color, *Orient;
	IplImage *s[10], *re_s[9], *i_s[6];
	IplImage *rg[9], *gr[9], *by[9], *yb[9];
	IplImage *re_rg[9], *re_gr[9], *re_by[9], *re_yb[9];
	IplImage *rg_tmp, *by_tmp;
	IplImage *dif_rg[6], *dif_gr[6], *dif_by[6], *dif_yb[6];
	IplImage *RG, *BY;
	IplImage *o1[9], *o2[9], *o3[9], *o4[9];
	IplImage *re_o1[9], *re_o2[9], *re_o3[9], *re_o4[9];
	IplImage *d_o1[6], *d_o2[6], *d_o3[6], *d_o4[6];
	IplImage *Ori1, *Ori2, *Ori3, *Ori4;

	float tmp, tmp_rg, tmp_by;

	cout << "Loading Image" <<endl;
	IplImage *IN_Img =cvCreateImage( cvGetSize(tmp_img),IPL_DEPTH_32F,3);
	cvConvertScale(tmp_img, IN_Img, 1.0/255.0, 0);

	//cvNamedWindow("Original"  , CV_WINDOW_AUTOSIZE);
	//cvShowImage("Original"  , IN_Img );


//---------------------------------特徴マップ作成：輝度-------------------------------------------------//
	cout << "Creating Intensity Map" <<endl;
	//グレースケール化
	IplImage *IN_Gray =cvCreateImage( cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	//cvCvtColor(IN_Img, IN_Gray, CV_BGR2GRAY);
	for(int y=0; y<IN_Gray->height; y++){
		for(int x=0; x<IN_Gray->width; x++){
			cvImageElem<IPL_DEPTH_32F>(IN_Gray, x, y, 0) = 0.2127f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 0)
														   + 0.7151f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 1)
														   + 0.0722f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 2);
		}
	}

	//出力画像用変数のヘッダ作成と領域の確保
	OUT_Img = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);

	//スケール画像用メモリ確保
	s[0] = cvCreateImage(cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	for(int i=1; i<10; i++){
		s[i] = cvCreateImage (cvSize (s[i-1]->width / 2, s[i-1]->height / 2), IPL_DEPTH_32F, 1);
	}

	//ピラミッド実行
	cvPyrDown(IN_Gray, s[1], CV_GAUSSIAN_5x5);
	for(int i=1; i<8; i++){
		cvPyrDown(s[i], s[i+1], CV_GAUSSIAN_5x5);
		//cvResize(s[i], s[i+1]);
	}

	//リサイズ画像用メモリ確保
	for(int i=0; i<9; i++){
		re_s[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	}

	//リサイズ実行
	for(int i=0; i<9; i++){
		cvResize(s[i], re_s[i]);
	}

	//スケール間差分画像用メモリ確保
	for(int i=0; i<6; i++){
		i_s[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	}

	//差分計算
	int j=0;
	for(int i=2; i<5; i++){
	   cvAbsDiff(re_s[i], re_s[i+3], i_s[j]);
	   cvAbsDiff(re_s[i], re_s[i+4], i_s[j+1]);
	   j=j+2;
	}

	//正規化
	for(int i=0; i<6; i++){
		myNorm(i_s[i]);
	}

	//輝度特徴用メモリ確保
	Intensity = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);

	//輝度ピラミッド統合
	cvAdd(i_s[0], i_s[1], i_s[0], 0);
	cvAdd(i_s[2], i_s[3], i_s[2], 0);
	cvAdd(i_s[4], i_s[5], i_s[4], 0);
	cvAdd(i_s[0], i_s[2], Intensity, 0);
	cvAdd(i_s[4], Intensity, Intensity, 0);

	myNorm(Intensity);

	/*
	IplImage *intensity = cvCreateImage(cvGetSize(Intensity), IPL_DEPTH_8U, 1);
	cvConvertScale(Intensity, intensity, 255.0, 0);
	#ifdef DEBUG
	cvNamedWindow("Intensity"  , 1);
	cvShowImage("Intensity" , intensity);
	#endif
	//cvNormalize (Intensity, Intensity, 0.0, 200.0, CV_MINMAX, NULL);
	cvSaveImage("output/Intensity.bmp" , intensity);
    */

	//テスト
	//cvMinMaxLoc(Intensity, &min, &max);
	//cout <<min<<" "<<max;

//-----------------------------------特徴マップ作成：色--------------------------------------------//
	cout << "Creating Color Map" <<endl;
	//RGとBY用メモリ確保
	rg_tmp = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	by_tmp = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	
	for(int y=0; y<IN_Gray->height; y++){
		for(int x=0; x<IN_Gray->width; x++){
			cvImageElem<IPL_DEPTH_32F>(rg_tmp, x, y, 0)=  0.1113f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 0)
														- 0.1103f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 1)
														- 0.0364f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 2);
			cvImageElem<IPL_DEPTH_32F>(by_tmp, x, y, 0) = 0.1933f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 0)
														+ 0.5959f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 1)
														- 0.8781f*cvImageElem<IPL_DEPTH_32F>(IN_Img, x, y, 2);
		}
	}

	rg[0] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	gr[0] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	by[0] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	yb[0] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	for(int i=1; i<9; i++){
		rg[i] = cvCreateImage(cvSize(rg[i-1]->width / 2, rg[i-1]->height / 2), IPL_DEPTH_32F, 1);
		gr[i] = cvCreateImage(cvSize(gr[i-1]->width / 2, gr[i-1]->height / 2), IPL_DEPTH_32F, 1);
		by[i] = cvCreateImage(cvSize(by[i-1]->width / 2, by[i-1]->height / 2), IPL_DEPTH_32F, 1);
		yb[i] = cvCreateImage(cvSize(yb[i-1]->width / 2, yb[i-1]->height / 2), IPL_DEPTH_32F, 1);
	}

	cvThreshold(rg_tmp, rg[0], 0.0, 0.0, CV_THRESH_TOZERO);
	cvThreshold(rg_tmp, gr[0], 0.0, 0.0, CV_THRESH_TOZERO_INV);
	cvConvertScale(gr[0], gr[0], -1.0); 
	cvThreshold(by_tmp, by[0], 0.0, 0.0, CV_THRESH_TOZERO);
	cvThreshold(by_tmp, yb[0], 0.0, 0.0, CV_THRESH_TOZERO_INV);
	cvConvertScale(yb[0], yb[0], -1.0);

	//ピラミッド実行
	for(int i=0; i<8; i++){
		cvPyrDown(rg[i], rg[i+1], CV_GAUSSIAN_5x5);
		cvPyrDown(gr[i], gr[i+1], CV_GAUSSIAN_5x5);
		cvPyrDown(by[i], by[i+1], CV_GAUSSIAN_5x5);
		cvPyrDown(yb[i], yb[i+1], CV_GAUSSIAN_5x5);
	}

	//リサイズ画像用メモリ確保
	for(int i=0; i<9; i++){
		re_rg[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
		re_gr[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
		re_by[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
		re_yb[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	}

	//リサイズ実行
	for(int i=0; i<9; i++){
		cvResize(rg[i], re_rg[i], CV_INTER_CUBIC);
		cvResize(gr[i], re_gr[i], CV_INTER_CUBIC);
		cvResize(by[i], re_by[i], CV_INTER_CUBIC);
		cvResize(yb[i], re_yb[i], CV_INTER_CUBIC);
	}

	//差分画像用メモリ確保
	for(int i=0; i<6; i++){
		dif_rg[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
		dif_gr[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
		dif_by[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
		dif_yb[i] = cvCreateImage(cvGetSize(IN_Img),IPL_DEPTH_32F,1);
	}

	//差分計算
	j=0;
	for(int i=2; i<5; i++){
	   cvSub(re_rg[i], re_rg[i+3], dif_rg[j]);
	   cvSub(re_rg[i], re_rg[i+4], dif_rg[j+1]);
	   cvThreshold(dif_rg[j], dif_rg[j], 0.0, 0.0, CV_THRESH_TOZERO);
	   cvThreshold(dif_rg[j+1], dif_rg[j+1], 0.0, 0.0, CV_THRESH_TOZERO);
	  
	   cvSub(re_gr[i], re_gr[i+3], dif_gr[j]);
	   cvSub(re_gr[i], re_gr[i+4], dif_gr[j+1]);
	   cvThreshold(dif_gr[j], dif_gr[j], 0.0, 0.0, CV_THRESH_TOZERO);
	   cvThreshold(dif_gr[j+1], dif_gr[j+1], 0.0, 0.0, CV_THRESH_TOZERO);
	  
	   cvSub(re_by[i], re_by[i+3], dif_by[j]);
	   cvSub(re_by[i], re_by[i+4], dif_by[j+1]);
	   cvThreshold(dif_by[j], dif_by[j], 0.0, 0.0, CV_THRESH_TOZERO);
	   cvThreshold(dif_by[j+1], dif_by[j+1], 0.0, 0.0, CV_THRESH_TOZERO);
	  
	   cvSub(re_yb[i], re_yb[i+3], dif_yb[j]);
	   cvSub(re_yb[i], re_yb[i+4], dif_yb[j+1]);
	   cvThreshold(dif_yb[j], dif_yb[j], 0.0, 0.0, CV_THRESH_TOZERO);
	   cvThreshold(dif_yb[j+1], dif_yb[j+1], 0.0, 0.0, CV_THRESH_TOZERO);
	   j=j+2;
	}

	/*int ii=0;
	cvNamedWindow("rg" , 1);
	cvShowImage("rg"  , dif_rg[ii]);
	cvNamedWindow("by" , 1);
	cvShowImage("by"  , dif_by[ii]);
	cvNamedWindow("gr" , 1);
	cvShowImage("gr"  , dif_gr[ii]);
	cvNamedWindow("yb" , 1);
	cvShowImage("yb"  , dif_yb[ii]);
	*/	

	//正規化
	for(int i=0; i<6; i++){
		myNorm(dif_rg[i]);
		myNorm(dif_gr[i]);
		myNorm(dif_by[i]);
		myNorm(dif_yb[i]);
	}

	//色特徴用メモリ確保
	RG = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	BY = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);

	//色特徴ピラミッド統合
	for(int y=0; y<IN_Gray->height; y++){
		for(int x=0; x<IN_Gray->width; x++){
			tmp_rg=0, tmp_by=0;
			for(int i=0; i<6; i++){
				//直でcvImageElemで指定した位置に"+="できなかったんでtmp的に"float rg,by"を使いますよ 2010-08-30
				tmp_rg +=  cvImageElem<IPL_DEPTH_32F>(dif_rg[i], x, y, 0) + cvImageElem<IPL_DEPTH_32F>(dif_gr[i], x, y, 0);
				tmp_by +=  cvImageElem<IPL_DEPTH_32F>(dif_by[i], x, y, 0) + cvImageElem<IPL_DEPTH_32F>(dif_yb[i], x, y, 0);
			}
			cvImageElem<IPL_DEPTH_32F>(RG, x, y, 0) = tmp_rg;
			cvImageElem<IPL_DEPTH_32F>(BY, x, y, 0) = tmp_by;
			//cout << cvImageElem<IPL_DEPTH_32F>(RG, x, y, 0) <<endl;
		}
	}

	myNorm(RG);
	myNorm(BY);
	Color = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	cvAddWeighted(RG, 0.5, BY, 0.5, NULL, Color);
	myNorm(Color);

	/*
	IplImage *color = cvCreateImage(cvGetSize(Color), IPL_DEPTH_8U, 1);
	cvConvertScale(Color, color, 255.0, 0);
	IplImage *convRG = cvCreateImage(cvGetSize(RG), IPL_DEPTH_8U, 1);
	cvConvertScale(RG, convRG, 255.0, 0);
	IplImage *convBY = cvCreateImage(cvGetSize(BY), IPL_DEPTH_8U, 1);
	cvConvertScale(BY, convBY, 255.0, 0);
	#ifdef DEBUG
	//画像表示用ウィンドウ作成
	cvNamedWindow("RG"  , 1);
	cvNamedWindow("BY", 1);
	cvNamedWindow("Color" , 1);
	//画像表示
	cvShowImage("RG"  , RG);
	cvShowImage("BY", BY);
	cvShowImage("Color" , Color);
	#endif

	//cvNormalize (Color, Color, 0.0, 200.0, CV_MINMAX, NULL);
	cvSaveImage("output/RG.bmp"  , convRG);
	cvSaveImage("output/BY.bmp", convBY);
	cvSaveImage("output/Color.bmp" , color);
    */

//----------------------------------特徴マップ作成：方位-----------------------------------------//
	cout << "Creating Orientation Map" <<endl;
	//ｶﾞﾎﾞｰﾙﾌｨﾙﾀ作成ﾊﾟﾗﾒｰﾀｰ
	//int GaborType = CV_GABOR_PHASE;	//ﾌｨﾙﾀﾀｲﾌﾟ:CV_GABOR_(REAL:実数,IMAG:虚数,MAG:振幅,PHASE:位相)
	double	dPhi_1 = 0, //縞模様の位相ｵﾌｾｯﾄ
		    dPhi_2 = PI/4,
	        dPhi_3 = PI/2,
			dPhi_4 = PI*3/4;
	int		iNu = 1;		//標準偏差ｵﾌｾｯﾄ 設定可能範囲:-5~∞
	double	Sigma = 2*PI;	//ｶﾞｳｽ関数の標準偏差
	double	F = sqrt(2.0);	//周波数

	o1[0] = cvCreateImage(cvGetSize(IN_Gray), IPL_DEPTH_32F, 1);
	o2[0] = cvCreateImage(cvGetSize(IN_Gray), IPL_DEPTH_32F, 1);
	o3[0] = cvCreateImage(cvGetSize(IN_Gray), IPL_DEPTH_32F, 1);
	o4[0] = cvCreateImage(cvGetSize(IN_Gray), IPL_DEPTH_32F, 1);

	//0
	CvGabor *gabor1 = new CvGabor;
	gabor1->Init(dPhi_1, iNu, Sigma, F);	//初期化
	gabor1->conv_img(IN_Gray, o1[0], CV_GABOR_MAG); //フィルタリング
	//PI/4
	CvGabor *gabor2 = new CvGabor;	
	gabor2->Init(dPhi_2, iNu, Sigma, F);	
	gabor2->conv_img(IN_Gray, o2[0], CV_GABOR_MAG); 
	//PI/2
	CvGabor *gabor3 = new CvGabor;	
	gabor3->Init(dPhi_3, iNu, Sigma, F);	
	gabor3->conv_img(IN_Gray, o3[0], CV_GABOR_MAG); 
	//PI*3/4
	CvGabor *gabor4 = new CvGabor;	
	gabor4->Init(dPhi_4, iNu, Sigma, F);	
	gabor4->conv_img(IN_Gray, o4[0], CV_GABOR_MAG); 

	//スケール画像用メモリ確保
	Orient = cvCreateImage(cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	for(int i=1; i<9; i++){
		o1[i] = cvCreateImage (cvSize (o1[i-1]->width / 2, o1[i-1]->height / 2), IPL_DEPTH_32F, 1);
		o2[i] = cvCreateImage (cvSize (o2[i-1]->width / 2, o2[i-1]->height / 2), IPL_DEPTH_32F, 1);
		o3[i] = cvCreateImage (cvSize (o3[i-1]->width / 2, o3[i-1]->height / 2), IPL_DEPTH_32F, 1);
		o4[i] = cvCreateImage (cvSize (o4[i-1]->width / 2, o4[i-1]->height / 2), IPL_DEPTH_32F, 1);
	}

	//ピラミッド実行
	for(int i=0; i<8; i++){
		cvPyrDown(o1[i], o1[i+1], CV_GAUSSIAN_5x5);
		cvPyrDown(o2[i], o2[i+1], CV_GAUSSIAN_5x5);
		cvPyrDown(o3[i], o3[i+1], CV_GAUSSIAN_5x5);
		cvPyrDown(o4[i], o4[i+1], CV_GAUSSIAN_5x5);
		//cvResize(o1[i], o1[i+1]);
		//cvResize(o2[i], o2[i+1]);
		//cvResize(o3[i], o3[i+1]);
		//cvResize(o4[i], o4[i+1]);
	}

	//リサイズ画像用メモリ確保
	for(int i=0; i<9; i++){
		re_o1[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
		re_o2[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
		re_o3[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
		re_o4[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	}

	//リサイズ実行
	for(int i=0; i<9; i++){
		cvResize(o1[i], re_o1[i]);
		cvResize(o2[i], re_o2[i]);
		cvResize(o3[i], re_o3[i]);
		cvResize(o4[i], re_o4[i]);
	}

	//スケール間差分画像用メモリ確保
	for(int i=0; i<6; i++){
		d_o1[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
		d_o2[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
		d_o3[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
		d_o4[i] = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	}

	//差分計算
	j=0;
	for(int i=2; i<5; i++){
	   cvAbsDiff(re_o1[i], re_o1[i+3], d_o1[j]);
	   cvAbsDiff(re_o2[i], re_o1[i+4], d_o1[j+1]);
	   cvAbsDiff(re_o3[i], re_o2[i+3], d_o2[j]);
	   cvAbsDiff(re_o4[i], re_o2[i+4], d_o2[j+1]);
	   cvAbsDiff(re_o1[i], re_o3[i+3], d_o3[j]);
	   cvAbsDiff(re_o2[i], re_o3[i+4], d_o3[j+1]);
	   cvAbsDiff(re_o3[i], re_o4[i+3], d_o4[j]);
	   cvAbsDiff(re_o4[i], re_o4[i+4], d_o4[j+1]);
	   j=j+2;
	}

	//正規化
	for(int i=0; i<6; i++){
		myNorm(d_o1[i]);
		myNorm(d_o2[i]);
		myNorm(d_o3[i]);
		myNorm(d_o4[i]);
	}

	//方位特徴用メモリ確保
	Ori1 = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	Ori2 = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	Ori3 = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	Ori4 = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);
	Orient = cvCreateImage (cvGetSize(IN_Gray),IPL_DEPTH_32F,1);

	cvAdd(d_o1[0], d_o1[1], d_o1[0], 0);
	cvAdd(d_o1[2], d_o1[3], d_o1[2], 0);
	cvAdd(d_o1[4], d_o1[5], d_o1[4], 0);
	cvAdd(d_o1[0], d_o1[2], Ori1, 0);
	cvAdd(d_o1[4], Ori1, Ori1, 0);

	cvAdd(d_o2[0], d_o2[1], d_o2[0], 0);
	cvAdd(d_o2[2], d_o2[3], d_o2[2], 0);
	cvAdd(d_o2[4], d_o2[5], d_o2[4], 0);
	cvAdd(d_o2[0], d_o2[2], Ori2, 0);
	cvAdd(d_o2[4], Ori2, Ori2, 0);

	cvAdd(d_o3[0], d_o3[1], d_o3[0], 0);
	cvAdd(d_o3[2], d_o3[3], d_o3[2], 0);
	cvAdd(d_o3[4], d_o3[5], d_o3[4], 0);
	cvAdd(d_o3[0], d_o3[2], Ori3, 0);
	cvAdd(d_o3[4], Ori3, Ori3, 0);

	cvAdd(d_o4[0], d_o2[1], d_o4[0], 0);
	cvAdd(d_o4[2], d_o4[3], d_o4[2], 0);
	cvAdd(d_o4[4], d_o4[5], d_o4[4], 0);
	cvAdd(d_o4[0], d_o4[2], Ori4, 0);
	cvAdd(d_o4[4], Ori4, Ori4, 0);

	//正規化
		myNorm(Ori1);
		myNorm(Ori2);
		myNorm(Ori3);
		myNorm(Ori4);

	//統合
	cvAdd(Ori1, Ori2,  Ori1);
	cvAdd(Ori3, Ori4,  Ori3);
	cvAdd(Ori1, Ori3,  Orient);

	myNorm(Orient);

	/*
	IplImage *orient = cvCreateImage(cvGetSize(Orient), IPL_DEPTH_8U, 1);
	cvConvertScale(Orient, orient, 255.0, 0);
	#ifdef DEBUG
	cvNamedWindow("Orient" , 1);
	cvShowImage("Orient"  , orient);
	#endif
	//cvNormalize(Orient, Orient, 0.0, 200.0, CV_MINMAX, NULL);
	cvSaveImage("output/Orient.bmp"  , orient);
	*/

//--------------------------------------SMの統合--------------------------------------------//
	//統合
	cout << "Map Integration"<<endl;
	for(int y=0; y<IN_Gray->height; y++){
		for(int x=0; x<IN_Gray->width; x++){
			tmp =  cvImageElem<IPL_DEPTH_32F>(Intensity, x, y, 0)
					   + cvImageElem<IPL_DEPTH_32F>(Color, x, y, 0)
					   + cvImageElem<IPL_DEPTH_32F>(Orient, x, y, 0);
			cvImageElem<IPL_DEPTH_32F>(OUT_Img, x, y, 0) = tmp / 3.0f; 
		}
	}

	//tmp1 = cvCreateImage (cvSize (200, 200), OUT_Img->depth, OUT_Img->nChannels);
	//cvResize(OUT_Img, tmp1);

	IplImage *out = cvCreateImage(cvGetSize(OUT_Img), IPL_DEPTH_8U, 1);
	cvConvertScale(OUT_Img, out, 255.0, 0);

	/*
	//表示
	cvNamedWindow("SM", 1);
	//cvShowImage("SM"  , OUT_Img);
	cvShowImage("SM", out);
	cvSaveImage("output/SM.bmp", out);
	*/

//--------------------------------メモリの解放　＆　ウィンドウの破棄--------------------------------//
	//キーが押されるまで待機する
	//cvWaitKey(0);

	cout << "Releasing";

	delete gabor1; 
	delete gabor2; 
	delete gabor3; 
	delete gabor4; 
	//IPLメモリの解放
	//cvReleaseImage(&out);
	//cvReleaseImage(&OUT_Img);
	cvReleaseImage(&Intensity);
	cvReleaseImage(&Color);
	cvReleaseImage(&Orient);
	cvReleaseImage(&rg_tmp);
	cvReleaseImage(&by_tmp);
	cvReleaseImage(&RG);
	cvReleaseImage(&BY);
	cvReleaseImage(&Ori1);
	cvReleaseImage(&Ori2);
	cvReleaseImage(&Ori3);
	cvReleaseImage(&Ori4);
	cvReleaseImage(&s[9]);
	for(int i=0; i<9; i++){
		cvReleaseImage(&s[i]);
		cvReleaseImage(&re_s[i]);
		cvReleaseImage(&rg[i]);
		cvReleaseImage(&gr[i]);
		cvReleaseImage(&by[i]);
		cvReleaseImage(&yb[i]);
		cvReleaseImage(&re_rg[i]);
		cvReleaseImage(&re_gr[i]);
		cvReleaseImage(&re_by[i]);
		cvReleaseImage(&re_yb[i]);
		cvReleaseImage(&o1[i]);
		cvReleaseImage(&o2[i]);
		cvReleaseImage(&o3[i]);
		cvReleaseImage(&o4[i]);
		cvReleaseImage(&re_o1[i]);
		cvReleaseImage(&re_o2[i]);
		cvReleaseImage(&re_o3[i]);
		cvReleaseImage(&re_o4[i]);
	}
	for(int i=0; i<6; i++){
		cvReleaseImage(&i_s[i]);
		cvReleaseImage(&dif_rg[i]);
		cvReleaseImage(&dif_gr[i]);
		cvReleaseImage(&dif_by[i]);
		cvReleaseImage(&dif_yb[i]);
		cvReleaseImage(&d_o1[i]);
		cvReleaseImage(&d_o2[i]);
		cvReleaseImage(&d_o3[i]);
		cvReleaseImage(&d_o4[i]);
	}

	//return out;
	return OUT_Img;
}
