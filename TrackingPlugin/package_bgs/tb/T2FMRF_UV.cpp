#include "T2FMRF_UV.h"

T2FMRF_UV::T2FMRF_UV() : firstTime(true), frameNumber(0), showOutput(true), threshold(9.0), alpha(0.01), 
gaussians(3), km(2), kv(0.9)
{
  std::cout << "T2FMRF_UV()" << std::endl;
}

T2FMRF_UV::~T2FMRF_UV()
{
  std::cout << "~T2FMRF_UV()" << std::endl;
}

void T2FMRF_UV::process(const cv::Mat &img_input, cv::Mat &img_output)
{
  if(img_input.empty())
    return;

  loadConfig();

  if(firstTime)
    saveConfig();

  frame = new IplImage(img_input);
  
  if(firstTime)
    frame_data.ReleaseMemory(false);
  frame_data = frame;

  if(firstTime)
  {
    int width	= img_input.size().width;
    int height = img_input.size().height;

    lowThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    lowThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    highThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    highThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    params.SetFrameSize(width, height);
    params.LowThreshold() = threshold;
    params.HighThreshold() = 2*params.LowThreshold();
    params.Alpha() = alpha;
    params.MaxModes() = gaussians;
    params.Type() = TYPE_T2FMRF_UV;
    params.KM() = km; // Factor control for the T2FMRF-UM [0,3] default: 2
    params.KV() = kv; // Factor control for the T2FMRF-UV [0.3,1] default: 0.9

    bgs.Initalize(params);
    bgs.InitModel(frame_data);

    old_labeling = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    old = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

    mrf.height = height;
	  mrf.width = width;
    mrf.Build_Classes_OldLabeling_InImage_LocalEnergy();

    firstTime = false;
  }

  bgs.Subtract(frameNumber, frame_data, lowThresholdMask, highThresholdMask);
  cvCopyImage(lowThresholdMask.Ptr(), old);

  /************************************************************************/
	/* the code for MRF, it can be noted when using other methods   */
	/************************************************************************/
	//the optimization process is done when the foreground detection is stable, 
	if(frameNumber >= 10)
	{
		gmm = bgs.gmm();
		hmm = bgs.hmm();
		mrf.background2 = frame_data.Ptr();
		mrf.in_image = lowThresholdMask.Ptr();
		mrf.out_image = lowThresholdMask.Ptr();
		mrf.InitEvidence2(gmm,hmm,old_labeling);
		mrf.ICM2();
		cvCopyImage(mrf.out_image, lowThresholdMask.Ptr());
	}

  cvCopyImage(old, old_labeling);

  lowThresholdMask.Clear();
  bgs.Update(frameNumber, frame_data, lowThresholdMask);
  
  cv::Mat foreground(highThresholdMask.Ptr());

  if(showOutput)
    cv::imshow("T2FMRF-UV", foreground);
  
  foreground.copyTo(img_output);

  delete frame;
  frameNumber++;
}

void T2FMRF_UV::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/T2FMRF_UV.xml", 0, CV_STORAGE_WRITE);

  cvWriteReal(fs, "threshold", threshold);
  cvWriteReal(fs, "alpha", alpha);
  cvWriteReal(fs, "km", km);
  cvWriteReal(fs, "kv", kv);
  cvWriteInt(fs, "gaussians", gaussians);
  cvWriteInt(fs, "showOutput", showOutput);

  cvReleaseFileStorage(&fs);
}

void T2FMRF_UV::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/T2FMRF_UV.xml", 0, CV_STORAGE_READ);
  
  threshold = cvReadRealByName(fs, 0, "threshold", 9.0);
  alpha = cvReadRealByName(fs, 0, "alpha", 0.01);
  km = cvReadRealByName(fs, 0, "km", 2);
  kv = cvReadRealByName(fs, 0, "kv", 0.9);
  gaussians = cvReadIntByName(fs, 0, "gaussians", 3);
  showOutput = cvReadIntByName(fs, 0, "showOutput", true);

  cvReleaseFileStorage(&fs);
}
