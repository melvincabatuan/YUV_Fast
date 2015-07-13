#include "io_github_melvincabatuan_yuv_MainActivity.h"
#include <android/bitmap.h>
#include <stdlib.h>

#include <opencv2/imgproc/imgproc.hpp>


#define  LOG_TAG    "YUV"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
 

 

void extractVU( const cv::Mat &image, cv::Mat &V, cv::Mat &U ){

	int nRows = image.rows;   // number of lines
    int nCols = image.cols;   // number of columns  

    /// Convert to 1D array if Continuous
    if (image.isContinuous()) {
        nCols = nCols * nRows;
		nRows = 1; // it is now a 
	}   

    for (int j=0; j<nRows; j++) {
    
        /// Pointer to start of the row
        //uchar* data   = image.ptr<uchar>(j);
        //uchar* colorV = V.ptr<uchar>(j);
        //uchar* colorU = U.ptr<uchar>(j);
        
        uchar* data = reinterpret_cast<uchar*>(image.data);
        uchar* colorV = reinterpret_cast<uchar*>(V.data);
        uchar* colorU = reinterpret_cast<uchar*>(U.data);

		for (int i = 0; i < nCols; i += 2) {
		        // assign each pixel to V and U
                *colorV++ = *data++; //  [0,255]
                *colorU++ = *data++; //  [0,255]   
        }
    }
}



cv::Mat imageU, imageV;
cv::Mat output;


/*
 * Class:     io_github_melvincabatuan_yuv_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_io_github_melvincabatuan_yuv_MainActivity_process
  (JNIEnv * pEnv, jobject pClass, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent;

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   ///  YUV420sp source --->  cv::Mat
    cv::Mat src(bitmapInfo.height+ bitmapInfo.height/2, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
  
   ///  BGRA --->  cv::Mat
    cv::Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);

/***************************************************************************************************/

   if (imageV.empty())
       imageV = cv::Mat(bitmapInfo.height/2, bitmapInfo.width/2, CV_8UC1);

   if (imageU.empty())
       imageU = cv::Mat(bitmapInfo.height/2, bitmapInfo.width/2, CV_8UC1);
              
   extractVU( src(cv::Range(bitmapInfo.height, bitmapInfo.height + bitmapInfo.height/2), cv::Range(0, bitmapInfo.width)), imageV, imageU);
   
   if (output.empty())
       output = cv::Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC1);
   
   //cv::pyrUp(imageV, output);
   cv::pyrUp(imageU, output);

   cv::cvtColor( output, mbgra, CV_GRAY2BGRA);

    /// YUV420spNV21 gray ---> BGRA
    //cv::cvtColor(src(cv::Range(0, bitmapInfo.height), cv::Range(0, bitmapInfo.width)), mbgra, CV_GRAY2BGRA);

/***************************************************************************************************/

    /// Release Java byte buffer and unlock backing bitmap
    pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();
}
