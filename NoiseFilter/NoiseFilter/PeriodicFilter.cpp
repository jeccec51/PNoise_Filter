#include "pch.h"
#include "PeriodicFilter.h"
#include <numeric>

using namespace cv;
using namespace std;


BandReject_Noise_Filter::BandReject_Noise_Filter(const std::string& imPath)
{
	m_bFileLoadError = true;
	m_InImage = cv::imread(imPath, cv::IMREAD_GRAYSCALE);
	if (m_InImage.empty())
	{
		std::cout << "Error opening image" << endl;
	}
	else
	{
		if (m_InImage.channels() > 1)
		{
			cvtColor(m_InImage, m_GrayImage, cv::COLOR_RGB2GRAY);
		}
		else
		{
			m_GrayImage = m_InImage;
		}
		m_bFileLoadError = false;
	}
}

BandReject_Noise_Filter::BandReject_Noise_Filter()
{
	m_bFileLoadError = true;
	m_nInterations = 0;
}


BandReject_Noise_Filter::~BandReject_Noise_Filter()
{
}

bool BandReject_Noise_Filter::LoadImageData(void* pImageData, const int& nHeight,
	const int& nWidth)
{
	m_bFileLoadError = true;
	float* pImData = static_cast<float*>(pImageData);
	if (pImData != nullptr &&
		nHeight > 0 &&
		nWidth > 0 &&
		sizeof(pImageData) == sizeof(float)
		)
	{
		cv::Mat imageBuffer = cv::Mat(nWidth, nHeight, CV_32F, pImData);
		m_InImage = imageBuffer;
		m_bFileLoadError = false;
	}
	return m_bFileLoadError;
}

bool BandReject_Noise_Filter::Filter_Periodic_Noise()
{
	bool bFilterError = false;
	//Updates the member variables corresponding to Spectrum
	Compute2DSpectrum();
	//Filter out the low frequency components, to preserve the non periodic data
	LowPassFilter();
	ApplyFilter();
	return bFilterError;
}

cv::Mat BandReject_Noise_Filter::GetOutPutImage()
{
	cv::Mat matOp;
	m_matOutput.convertTo(matOp, CV_8UC1);
	return matOp;
}

void BandReject_Noise_Filter::ApplyFilter()
{
	m_matLPF = FFtShift(m_matLPF);
	Mat planes[2] = { Mat_<float>(m_GrayImage.clone()), Mat::zeros(m_GrayImage.size(), CV_64F) };
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI, DFT_SCALE);
	Mat planesH[2] = { Mat_<double>(m_matLPF.clone()),
		Mat::zeros(m_matLPF.size(), CV_64F) };
	Mat complexH;
	merge(planesH, 2, complexH);
	Mat complexIH;
	mulSpectrums(complexI, complexH, complexIH, 0);
	idft(complexIH, complexIH);
	split(complexIH, planes);
	m_matOutput = planes[0];
}


void BandReject_Noise_Filter::ApplyLPF(const cv::Mat& matLPF, cv::Mat& matSpectrum)
{
	cv::Mat matTmpMask;
	int nTypeImg = matSpectrum.type();
	matSpectrum.convertTo(matSpectrum, CV_8UC1);
	matLPF.convertTo(matLPF, CV_8UC1);
	cv::bitwise_and(matSpectrum, matLPF, matSpectrum);
	matSpectrum.convertTo(matSpectrum, nTypeImg);
}


int BandReject_Noise_Filter::FindLowPassCutoff(cv::Mat& matMagSpectrum, cv::Mat& matLPF)
{
	cv::Mat matLPFMask = Mat::zeros(matMagSpectrum.size(), CV_8UC1);
	int nX = matMagSpectrum.cols / 2;
	int nY = matMagSpectrum.rows / 2;
	int nMinimumDimention = min(matMagSpectrum.rows, matMagSpectrum.cols);
	cv::Point ptCenter(nX, nY);
	cv::Mat matSpectrum1 = matMagSpectrum;
	matSpectrum1.convertTo(matMagSpectrum, CV_8UC1);
	std::vector<double> LPFCutoffList;
	int nLPFRad = 0, nIdx = 0;
	for (nLPFRad = 2; nLPFRad <= nMinimumDimention / 2; nLPFRad++)
	{
		nIdx++;
		cv::circle(matLPFMask, ptCenter, nLPFRad, Scalar::all(255), -1);
		cv::bitwise_and(matSpectrum1, matLPFMask, matSpectrum1);
		LPFCutoffList.push_back(CalculateAverage(matSpectrum1));
		ApplyLPF(matLPFMask, matSpectrum1);
		if ((nIdx >=3) && (PeakDetect(LPFCutoffList[nIdx - 3], LPFCutoffList[nIdx - 2], LPFCutoffList[nIdx - 1])))
		{
			break;
		}
	}
	circle(matLPF, ptCenter, nLPFRad, Scalar::all(255), -1);
	return nLPFRad;
}



void BandReject_Noise_Filter::LowPassFilter()
{
	//First we Apply an LPF to filter out the High Frequency Components
	cv::Mat matLowFrequencyMask = cv::Mat::zeros(m_matPSD.size(), CV_64F);
	cv::Mat matThrPSD, matThrMag, MatMag, MatPSD;
	bool bUsePSDCirCle = false;
	m_matMAgnitudeSpectrum.convertTo(MatMag, CV_8UC1);
	//Un-Normalize the spectrum
	MatMag = MatMag * Parms::kHighestIntensityLevel;
	//Do an initial OTSU thresholding on the spectrum to clusted High and Low frequency components
	cv::threshold(MatMag, matThrMag, Parms::kLowestIntensityLevel, Parms::kHighestIntensityLevel, 
		THRESH_BINARY | THRESH_OTSU);
	//Compute LPF Cut off frequency
	int nRadius = FindLowPassCutoff(MatMag, matLowFrequencyMask);
	m_matLPF = matLowFrequencyMask;
}



void BandReject_Noise_Filter::Compute2DSpectrum()
{
	cv::Mat matSpatialImage = m_GrayImage;
	cv::Rect fftRegion = Rect(0, 0, matSpatialImage.cols & -2, matSpatialImage.rows & -2);
	ComputePSD(matSpatialImage, m_matPSD, m_matMAgnitudeSpectrum,
		m_matLogPSD, m_matLogMAgnitudeSpectrum, 1);
	m_matPSD = FFtShift(m_matPSD);
	m_matMAgnitudeSpectrum = FFtShift(m_matMAgnitudeSpectrum);
	m_matLogPSD = FFtShift(m_matLogPSD);
	m_matLogMAgnitudeSpectrum = FFtShift(m_matLogMAgnitudeSpectrum);
	cv::normalize(m_matPSD, m_matPSD, 0, 255, NORM_MINMAX);
	cv::normalize(m_matMAgnitudeSpectrum, m_matMAgnitudeSpectrum, 0, 255, NORM_MINMAX);
	}

void BandReject_Noise_Filter::ComputePSD(const cv::Mat& matSpatial, cv::Mat& matPSDImage,
	cv::Mat& matMagSpectrum, cv::Mat& matLogPSD, cv::Mat& matLogMagSpectrum, int nLogFlag)
{
	//Function returns both the power spectral density and magnitude spectrum of incoming image, 
	//logarithmic and raw version
	cv::Mat planes[2] = { cv::Mat_<float>(matSpatial.clone()),
		cv::Mat::zeros(matSpatial.size(), CV_64F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);
	cv::dft(complexI, complexI);
	cv::split(complexI, planes);            // planes[0] = Re(DFT(I)), planes[1] = Im(DFT(I))
	cv::Mat matMagnitudeSpectrum;
	cv::magnitude(planes[0], planes[1], matMagSpectrum);
	planes[0].at<float>(0) = 0;
	planes[1].at<float>(0) = 0;
	// compute the PSD = sqrt(Re(DFT(I))^2 + Im(DFT(I))^2)^2
	magnitude(planes[0], planes[1], matPSDImage);        //imgPSD = sqrt(Power spectrum density)
	pow(matPSDImage, 2, matPSDImage);                         //it needs ^2 in order to get PSD
	// logPSD = log(1 + PSD)
	matLogPSD = matPSDImage + Scalar::all(1);
	log(matLogPSD, matLogPSD);
	log(matMagSpectrum, matLogMagSpectrum);
}

cv::Mat BandReject_Noise_Filter::FFtShift(const cv::Mat& matInputSpectrum)
{
	/*Shifts the FFt quadrants for better visualization*/
	cv::Mat matOutputSpectrum = matInputSpectrum.clone();
	int nCenterX = matOutputSpectrum.cols / 2;
	int nCenterY = matOutputSpectrum.rows / 2;
	Mat matQuadrant0(matOutputSpectrum, Rect(0, 0, nCenterX, nCenterY));
	Mat matQuadrant1(matOutputSpectrum, Rect(nCenterX, 0, nCenterX, nCenterY));
	Mat matQuadrant2(matOutputSpectrum, Rect(0, nCenterY, nCenterX, nCenterY));
	Mat matQuadrant3(matOutputSpectrum, Rect(nCenterX, nCenterY, nCenterX, nCenterY));
	Mat matTmp;
	matQuadrant0.copyTo(matTmp);
	matQuadrant3.copyTo(matQuadrant0);
	matTmp.copyTo(matQuadrant3);
	matQuadrant1.copyTo(matTmp);
	matQuadrant2.copyTo(matQuadrant1);
	matTmp.copyTo(matQuadrant2);
	return matOutputSpectrum;
}

double BandReject_Noise_Filter::CalculateAverage(cv::Mat& matImage)
{
	int nNZ = cv::countNonZero(matImage);
	double dPixelsum = cv::sum(matImage)[0];
	double dAverage = (nNZ > 0) ? dPixelsum / static_cast<double>(nNZ) : 0;
	return dAverage;
}
