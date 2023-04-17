#include "BitmapIO.h"

#include <assert.h>
// common dialog
#include <commdlg.h>
#include <float.h> // for double
#include <math.h> // for pow
#include <stdlib.h> // for rand
#include <stdio.h> // for scanf
#include <time.h> // for time
// for GET_X_LPARAM, GET_Y_LPARAM
#include <windowsx.h>

// Disable sign match warnings.
#pragma warning(disable : 4018)
// Disable type casting warnings.
#pragma warning(disable : 4244)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
// Disable security func warnings.
#pragma warning(disable : 4996)
// Disable memory uninited warnings.
#pragma warning(disable : 6001)
// Disable buffer read/write warnings.
#pragma warning(disable : 6385)
#pragma warning(disable : 6386)
// Allow mismatched wWinMain annotations.
#pragma warning(disable : 28251)

#define myAssert(Expression) \
    if ((Expression) == NULL) \
    { MessageBox(NULL, L"发生了一个未知错误, 应用程序即将退出.", L"错误", MB_OK | MB_ICONERROR); exit(1); }

//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------

WCHAR gAppName[] = L"BitmapViewer";

// Window Class Identifier
WCHAR* gMainWndName = gAppName;
WCHAR gSecondWndName[] = L"Secondary";
WCHAR gGrayTransWndName[] = L"GrayTrans";
// Gtw: Gray transform window
WCHAR gGtwHistDispWndName[] = L"GtwHistDisp";
WCHAR gDomainFilterWndName[] = L"DomainFilter";
WCHAR gDomainCustCoreWndName[] = L"DomainCustCore";
WCHAR gAppAboutWndName[] = L"AppAbout";
// New window members from v2.0.
WCHAR gThresholdWndName[] = L"Threshold";

// Application Handle
HINSTANCE gInstance;

// Main window & Child windows (legacy)
HWND gMainWnd,
     gSecondWnd,
     gGrayTransWnd,
     gGtwHistDispWnd,
     gDomainFilterWnd,
     gDomainCustCoreWnd,
     gAppAboutWnd;

// Extension Windows (v2.0)
// REMARK: After upgraded to v2.0, BitmapViewer's new child windows should
// be classified into [extension windows] rather than [old child windows].
HWND gThresholdWnd;

// Remember to modify [valid window count] after add a new window.

// Image data of main window
MyBGRA* gImage = NULL;
MyBmpInfo gBmpInfo;
// We have to forward declare here since MyComplex type hasn't been defined yet.
typedef struct tagMyComplex MyComplex;
MyComplex* gSpectral = NULL;

// System metrics info
UINT gCxIcon = 0, gCyIcon = 0;
UINT gCaptionHeight = 0;
UINT gCharWidth = 0, gCharHeight = 0;
UINT gCxVertScr = 0, gCyVertScr = 0;
UINT gCxHorzScr = 0, gCyHorzScr = 0;
UINT gMaxClntWidth = 0, gMaxClntHeight = 0;

// App Icon
HICON gIcon;
MyBGRA* gAppIconData;

// Main window menu
HMENU gMenu;
// Main accelerator table
HACCEL gAccel;

// Whether the image of main window is connected to an external file.
BOOL gHasExternalFile = FALSE;
// Used for common dialog. gFileName (full path), gTitleName(body name).
WCHAR gFileName[MAX_PATH], gTitleName[MAX_PATH];

// Child Window ID (legacy)
#define SECOND_WND 100
#define GRAY_TRANS_WND 101
#define GTW_HIST_DISP_WND 102
#define DOMAIN_FILTER_WND 103
#define DOMAIN_CUST_CORE_WND 104
#define APP_ABOUT_WND 105

// Extension Window ID (v2.0)
// REMARK: After upgraded to v2.0, BitmapViewer's new child windows should
// be classified into [extension windows] rather than [old child windows].
#define THRESHOLD_WND 106

// Main Menu ID
//-------------------------------------
#define IDM_FILE 0
#define IDM_FILE_NEW_256x256 1001
#define IDM_FILE_NEW_512x512 1002
#define IDM_FILE_NEW_1024x1024 1003
#define IDM_FILE_OPEN 1004
#define IDM_FILE_SECOND 1005
#define IDM_FILE_SAVE 1006
#define IDM_FILE_SAVE_AS 1007
#define IDM_FILE_EXP_TXT 1008
//-------------------------------------
#define IDM_GRAY 1
// empirical formula
#define IDM_GRAY_EMPI 1101
#define IDM_GRAY_EVEN 1102
// gamma correction
#define IDM_GRAY_GAMMA 1103
#define IDM_GRAY_R 1104
#define IDM_GRAY_G 1105
#define IDM_GRAY_B 1106
// boundary expansion
#define IDM_GRAY_2xBLACK 1107
#define IDM_GRAY_2xMIRROR 1108
#define IDM_GRAY_2xCOPY 1109
// threshold process
#define IDM_GRAY_THRESHOLD 1110
//-------------------------------------
#define IDM_DOMAIN 2
// SPAF: SPAtial Filter
#define IDM_SPAF_BOX 1201
// gaussian core
#define IDM_SPAF_GAUS 1202
// median value
#define IDM_SPAF_MEDI 1203
// 2nd derivative
#define IDM_SPAF_LAPLACE 1204
// 1st derivative
#define IDM_SPAF_SOBEL 1205
// custom core
#define IDM_SPAF_CUST 1206 // DISABLE
// SPEF: SPEctral Filter
// power spectral (magnitude2)
#define IDM_SPEF_POWER 1207
// phase spectral (atan2(y,x))
#define IDM_SPEF_PHASE 1208
// FFT accelerated types
#define IDM_SPEF_POWER_FFT 1209
#define IDM_SPEF_PHASE_FFT 1210
// custom core
#define IDM_SPEF_CUST 1211 // DISABLE
#define MY_DOMAIN_IDM_COUNT 11
// Remember to update [domain idm count] after add a new menu item.
//-------------------------------------
#define IDM_TRANSFER_FUNC 3
// ILPF: Idea Low Pass Filter
#define IDM_TFUNC_ILPF 1301
// GLPF: Gaussian Low Pass Filter
#define IDM_TFUNC_GLPF 1302
// BLPF: Butterworth Low Pass Filter
#define IDM_TFUNC_BLPF 1303
// HPF: Ideal, Gaussian, Butterworth
#define IDM_TFUNC_IHPF 1304
#define IDM_TFUNC_GHPF 1305
#define IDM_TFUNC_BHPF 1306
// Homomorphic Filter
#define IDM_TFUNC_HOMO 1307
#define MY_TFUNC_IDM_COUNT 7
// Remember to update [tfunc idm count] after add a new menu item.
//-------------------------------------
#define IDM_NOISE_MODEL 4
// normal
#define IDM_NMODEL_GAUSSIAN 1401
// gaussian^2
#define IDM_NMODEL_RAYLEIGH 1402
// gamma distribution
#define IDM_NMODEL_ERLANG 1403
// EXP: Exponential
#define IDM_NMODEL_EXP 1404
// constant distribution
#define IDM_NMODEL_UNIFORM 1405
// SANDP: Salt & Pepper
#define IDM_NMODEL_SANDP 1406
//-------------------------------------
#define IDM_HELP 5
#define IDM_EASTER_EGG 1501
#define IDM_APP_ABOUT 1502
#define IDM_WEB_HOME 1503
// Remember to modify [en/disable operation menu] after add a new menu.

//-------------------------------------------------------------------------------------------------
// Helper Function Declaration
//-------------------------------------------------------------------------------------------------

// Window Process Function
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SecondWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GrayTransWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GtwHistDispWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DomainFilterWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DomainCustCoreWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AppAboutWndProc(HWND, UINT, WPARAM, LPARAM);

// New wndproc members from v2.0.
LRESULT CALLBACK ThresholdWndProc(HWND, UINT, WPARAM, LPARAM);

HICON myLoadAppIcon();

HMENU myLoadMainMenu();
HACCEL myLoadMainAccel();

UINT myValidWndCount();

RECT myGetSecondWndInitSize();

// Process the image with logarithm transform: z = c * log(1 + r)
// 
// @param pData: the origin image to process.
// @param pInfo: points to image pixel info.
// @param c: see the formula listed above.
//
void myLogarithmTrans(MyBGRA* pData, MyBmpInfo* pInfo, double c);

// Process the image with gamma transform: z = r^gamma
// 
// @param pData: the origin image to process.
// @param pInfo: points to image pixel info.
// @param gamma: see the formula listed above.
//
void myExpGammaTrans(MyBGRA* pData, MyBmpInfo* pInfo, double gamma);

// Operation menus are those that can export or modify the image of main window.
void myEnableOperationMenus();
void myDisableOperationMenus();

OPENFILENAME gOpenFileName;
void myInitFileDialogInfo(HWND hwnd);
BOOL myOpenFileDialog(HWND hwnd, PWSTR szFileName, PWSTR szTitleName);
BOOL mySaveFileDialog(HWND hwnd, PWSTR szFileName, PWSTR szTitleName);

// Return whether RGB channels are equal.
BOOL myIsImageGrayScale(MyBGRA * pData, MyBmpInfo * pInfo);

// Display image into window.
//
// @param hdc: handle to device context.
// @param offsetX: x destination coordinate.
// @param offsetY: y destination coordinate.
// @param cxValid: destination mask width.
// @param cyValid: destination mask height.
// @param imgX: x source coordinate.
// @param imgY: y source coordinate.
// @param pData: points to image pixel data.
// @param pInfo: points to image pixel info.
//
void myDisplayImage(HDC hdc, int offsetX, int offsetY, UINT cxValid, UINT cyValid,
                    int imgX, int imgY, MyBGRA* pData, MyBmpInfo* pInfo);

RECT myGetGrayTransWndInitSize();

// Extract gray scale of source image (colorful) into destination image (gray).
//
// @param pDst: gray destination image.
// @param pSrc: colorful source image.
// @param pInfo: points to image pixel info.
//
// Empirical
void myExtractGrayEmpi(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo);
// Arithmetic Mean
void myExtractGrayEven(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo);
// R - Channel
void myExtractGrayR(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo);
// G - Channel
void myExtractGrayG(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo);
// B - Channel
void myExtractGrayB(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo);
// Corrected with Gamma Coefficient
void myExtractGrayGamma(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo);

// Populate pHist with computed histogram result (8-bit gray scale).
void myComputeHistogramResult(MyBGRA* pData, MyBmpInfo* pInfo, double pHist[256]);

// Display histogram result into window.
//
// @param hdc: handle to device context.
// @param offsetX: x coordinate of origin of histogram.
// @param offsetY: y coordinate of origin of histogram.
// @param pHist: points to histogram distribution data.
// @param nGrayScaleStep: increment step of vertical lines.
// @param yHistStretch: ratio multiplied to vertical lines.
//
void myDisplayHistogramResult(HDC hdc, int offsetX, int offsetY, double pHist[256],
                              UINT8 nGrayScaleStep, double yHistStretch);

// Process the image with histogram equalization.
void myEqualizeHistogram(MyBGRA* pData, MyBmpInfo* pInfo, double pHist[256]);

// @see myDisplayHistogramResult. @see GtwHistDispWndProc for aAnchors, nPointCount.
void myDisplayHistRgltFormatLines(HDC hdc, POINT aAnchors[256], UINT nPointCount);

// Populate pTargetHist with computed histogram result.
// Note anchor points are set in window coordinate.
//  
// [Conversion Formula]:
// gray_scale = x - 5 (there's a 5-pixel padding at the left of histogram);
// P_raw = 256 - y (the window coordinate flips Y-Axis; need to normalize).
// 
// @see GtwHistDispWndProc for more info about the magic number above (5, 256).
//
// @param pTargetHist: where to store the result.
// @param aAnchors: gray scale coordinate values.
// @param nPointCount: available point count in aAnchors.
// @param rgltStretch: power exponent of anchor point y-value.
//
void myComputeRgltHistFormat(double pTargetHist[256], POINT aAnchors[256], UINT nPointCount, double rgltStretch);

// Process the image with histogram regulation.
//
// @param pData: the image to be regulated.
// @param pInfo: points to image pixel info.
// @param pHist: origin image's histogram result (related to pData).
// @param pTargetHist: histogram with regulation format (from user).
//
void myRegulateHistogram(MyBGRA* pData, MyBmpInfo* pInfo, double pHist[256], double pTargetHist[256]);

RECT myGetSpafGeneWndInitSize();

// Allocate a buffer and populate the buffer with mirrored image data.
// Note [real increment size] = 2 * [expand size] (sum of both sides).
//
// @param pData: the origin image to expand.
// @param pInfo: points to image pixel info.
// @param xExpandSize: how far out should expand in horizontal.
// @param yExpandSize: how far out should expand in vertical.
// 
// @return the mirrored pixel data buffer.
//
MyBGRA* myExpandMirrorImage(MyBGRA* pData, MyBmpInfo* pInfo, UINT xExpandSize, UINT yExpandSize);

// Populate new pixels as black.
MyBGRA* myExpand2xBlackImage(MyBGRA* pData, MyBmpInfo* pInfo);
// Populate new pixels as mirrored.
MyBGRA* myExpand2xMirrorImage(MyBGRA* pData, MyBmpInfo* pInfo);
// Populate new pixels as boundary color.
MyBGRA* myExpand2xCopyImage(MyBGRA* pData, MyBmpInfo* pInfo);

// Define these funcs to customize square separable filter.
// @see mySquareSeparableFilter for details.
//
typedef void(*myHorzStrideHandler)(
    double* sumColor, // temporary buffer to store horizontal result
    MyBGRA* pSrc, // source image buffer to read & process
    MyBmpInfo* pInfo, // points to image pixel info
    UINT currIdx, // current handled pixel index in temporary buffer
    int orgOffset, // origin horizontal offset (regardless of border-mode)
    int horzOffset, // related horizontal offset in source image
    void* extraData);
// Used to customize square separable filter.
typedef void(*myVertStrideHandler)(
    double* result, // temporary register to store pixel (horz + vert) result
    double* sumColor, // temporary buffer stores horizontal result
    MyBmpInfo* pInfo, // points to image pixel info
    UINT currIdx, // current handled pixel index of final result
    int orgOffset, // origin vertical offset (regardless of border-mode)
    int vertOffset, // related vertical offset in temporary buffer
    void* extraData);
// Used to customize square separable filter.
typedef void(*myResultHandler)(
    MyBGRA* pData, // destination image buffer to write
    MyBmpInfo* pInfo, // points to image pixel info
    UINT currIdx, // current handled pixel index of final result
    double result, // temporary register stores pixel (horz + vert) result
    void* extraData);

// Process the image with a square (m x m) separable filter.
// 
// @param pData: destination image buffer to write.
// @param pSrc: source image buffer to read & process.
// @param pInfo: points to image pixel info.
// @param nBorderMode: how the filter populate border pixels.
// @param halfLen: length of square convolution core.
// 
// @param fnHorz: custom handler for each pixel in horizontal stride.
// @param fnVert: custom handler for each pixel in vertical stride.
// @param fnResult: custom handler to get final target result for each pixel.
// 
// @param horzExtraData, vertExtraData, resultExtraData: [ used to pass custom data for specific filter. ]
//
void mySquareSeparableFilter(MyBGRA* pData, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT nBorderMode, int halfLen,
                             myHorzStrideHandler fnHorz, void* horzExtraData,
                             myVertStrideHandler fnVert, void* vertExtraData,
                             myResultHandler fnResult, void* resultExtraData);

#define MY_FILTER_FUNC_PARAMS MyBGRA* pData, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT nBorderMode

// Process the image with a spatial box filter.
// @see mySquareSeparableFilter for details about MY_FILTER_FUNC_PARAMS.
// 
// @param iNormCoef_m: also known as box size (square core, m x m).
//
void myDomainBoxFilter(MY_FILTER_FUNC_PARAMS, int iNormCoef_m);

// Process the image with a spatial gaussian filter.
// @see mySquareSeparableFilter for details about MY_FILTER_FUNC_PARAMS.
// 
// @param sigma: standard deviation of gaussian function (used for blur grade).
// @param blurRadius: also known as half length (square core, [2r+1] x [2r+1]).
// 
// In general, core size (2*blurRadius) would better be floor(6*sigma) to match 3-sigma position.
//
void myDomainGaussianFilter(MY_FILTER_FUNC_PARAMS, double sigma, int blurRadius);

// Process the image with a median value filter.
// @see mySquareSeparableFilter for details about MY_FILTER_FUNC_PARAMS.
// 
// @param halfLen: [ core-size = 2 * half-len + 1 ].
//
// A non-linear filter, which is very useful for removing pepper noise.
//
void myDomainMedianFilter(MY_FILTER_FUNC_PARAMS, int halfLen);

// Sharpen the image with 2nd derivative.
void myDomainLaplaceFilter(MY_FILTER_FUNC_PARAMS);

// Sharpen the image with 1st derivative.
void myDomainSobelFilter(MY_FILTER_FUNC_PARAMS);

/*
* We define a simple (no optimization) complex math package here to help with Fourier transform.
*/

typedef struct tagMyComplex { long double real, imag; } MyComplex;

/*
* Prefix "mycp" means "my + Complex".
*/

// Return square of magnitude (aka power) of the complex number.
long double mycpMagnitude2(MyComplex* z)
{ return z->real * z->real + z->imag * z->imag; }

// Return magnitude (aka voltage) of the complex number.
long double mycpMagnitude(MyComplex* z)
{ return sqrt(z->real * z->real + z->imag * z->imag); }

// Return phase of the complex number.
long double mycpPhase(MyComplex* z)
{ return (z->real == 0 && z->imag == 0) ? 0 : atan2(z->imag, z->real); }

// Invert [z].
void mycpMinusInPlace(MyComplex* z)
{ z->real = -z->real; z->imag = -z->imag; }

// Return inverted [z].
MyComplex mycpMinus(MyComplex* z)
{ MyComplex g = { -z->real, -z->imag }; return g; }

// Make conjugate of [z].
void mycpConjInPlace(MyComplex* z)
{ z->imag = -z->imag; }

// Return conjugate of [z].
MyComplex mycpConj(MyComplex* z)
{ MyComplex g = { z->real, -z->imag }; return g; }

// Populate [g] with [z1 + z2].
void mycpAddInPlace(MyComplex* z1, MyComplex* z2, MyComplex* g)
{ g->real = z1->real + z2->real; g->imag = z1->imag + z2->imag; }

// Return [z1 + z2].
MyComplex mycpAdd(MyComplex* z1, MyComplex* z2)
{ MyComplex g = { z1->real + z2->real, z1->imag + z2->imag }; return g; }

// Populate [g] with [z1 - z2].
void mycpSubInPlace(MyComplex* z1, MyComplex* z2, MyComplex* g)
{ g->real = z1->real - z2->real; g->imag = z1->imag - z2->imag; }

// Return [z1 - z2].
MyComplex mycpSub(MyComplex* z1, MyComplex* z2)
{ MyComplex g = { z1->real - z2->real, z1->imag - z2->imag }; return g; }

// Populate [g] with [z1 * z2].
void mycpMulInPlace(MyComplex* z1, MyComplex* z2, MyComplex* g)
{
    // Note the multiplication depends both of the real and imag part,
    // so we must use a intermidiate variable to store the 1st result!
    // (In case that the user calls with  [ g = z1 ]  or  [ g = z2 ] )
    long double tmpReal = z1->real * z2->real - z1->imag * z2->imag;
    g->imag = z1->real * z2->imag + z1->imag * z2->real;
    g->real = tmpReal; // In case real changed before calculate imag.
}

// Populate [g] with [c * z].
void mycpMulScalarInPlace(long double c, MyComplex* z, MyComplex* g)
{ g->real = c * z->real; g->imag = c * z->imag; }

// Return [z1 * z2].
MyComplex mycpMul(MyComplex* z1, MyComplex* z2)
{
    MyComplex g =
    {
        z1->real * z2->real - z1->imag * z2->imag,
        z1->real * z2->imag + z1->imag * z2->real
    };
    return g;
}

// Return [c * z].
MyComplex mycpMulScalar(long double c, MyComplex* z)
{ MyComplex g = { c * z->real, c * z->imag }; return g; }

// Populate [g] with [z1 / z2].
void mycpDivInPlace(MyComplex* z1, MyComplex* z2, MyComplex* g)
{
    long double z2mag = z2->real * z2->real + z2->imag * z2->imag;
    long double tmpReal = (z1->real * z2->real + z1->imag * z2->imag) / z2mag;
    g->imag = (z1->imag * z2->real - z1->real * z2->imag) / z2mag;
    g->real = tmpReal; // @see mycpMulInPlace for the necessity of tmpReal.
}

// Return [z1 / z2].
MyComplex mycpDiv(MyComplex* z1, MyComplex* z2)
{
    long double z2mag = z2->real * z2->real + z2->imag * z2->imag;
    MyComplex g =
    {
        (z1->real * z2->real + z1->imag * z2->imag) / z2mag,
        (z1->imag * z2->real - z1->real * z2->imag) / z2mag
    };
    return g;
}

/*
* We don't provide methods to divide a complex with a scalar,
* since it can be done easily by calling mycpMulScalar(1 / c, z).
*/

// Populate [z] with [e^(j * power)].
void mycpEjInPlace(long double power, MyComplex* z)
{ z->real = cos(power); z->imag = sin(power); }

// Return [e^(j * power)].
MyComplex mycpEj(long double power)
{ MyComplex z = { cos(power), sin(power) }; return z; }

// 1-Dimension DFT/IDFT (in place).
//
// @param pDst: transformed results.
// @param Psrc: origin complex data.
// @param L: convolution array length.
// @param incre: 1 or L, for 2-D data.
//
void mycpDFT(MyComplex* pDst, MyComplex* pSrc, UINT L, UINT incre);
void mycpIDFT(MyComplex* pDst, MyComplex* pSrc, UINT L, UINT incre);

// Get radix 2 of the number;
// return UINT_MAX if failed.
UINT myGetRadix2(UINT num);

// 1-Dimension FFT/IFFT (in place).
//
// @param pData: complex data array.
// @param R: array length [radix 2].
// @param incre: 1 or L, for 2-D data.
//
void mycpFFT(MyComplex* pData, UINT R, UINT incre);
void mycpIFFT(MyComplex* pData, UINT R, UINT incre);

// 2-Dimension DFT/IDFT (heap alloc).
//
// @param pData: origin complex data.
// @param M: convolution core width.
// @param N: convolution core height.
//
// @return related freq-domain result [M x N].
//
MyComplex* mycpDFT2(MyComplex* pData, UINT M, UINT N);
MyComplex* mycpIDFT2(MyComplex* pData, UINT M, UINT N);

// 2-Dimension FFT/IFFT (heap alloc). [Optimizaed from DFT].
//
// @param pData: origin complex data.
// @param M: convolution core width.
// @param RM: core width [radix 2].
// @param N: convolution core height.
// @param RN: core height [radix 2].
//
// @return related freq-domain result [M x N].
//
MyComplex* mycpFFT2(MyComplex* pData, UINT M, UINT RM, UINT N, UINT RN);
MyComplex* mycpIFFT2(MyComplex* pData, UINT M, UINT RM, UINT N, UINT RN);

// @see myLogarithmTrans, myExpGammaTrans my for more detials.
// Populate pDst with normalized logarithm/exp-gamma of real parts in pSrc.
void mycpLogarithmTrans(MyBGRA* pDst, MyComplex* pSrc, UINT M, UINT N, long double c);
void mycpExpGammaTrans(MyBGRA* pDst, MyComplex* pSrc, UINT M, UINT N, long double gamma);

// Populate destination image with the frequency power-spectral of source image.
// To make the freq-spectral shown at the center, (-1)^(x+y) is applied before transform.
// To make the freq-spectral brighter, a logarithm transform is applied after transform.
//
// @param pDst: the destination image.
// @param pSrc: the source image.
// @param pInfo: points to image pixel info.
// @param dLogParam: [c] in [c * log(1 + r)].
// 
// @return origin calculated power spectral.
//
MyComplex* myPowerSpectral(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, long double dLogParam);
// FFT accelerated version of power spectral.
// @see mycpFFT2 for details about RM and RN.
MyComplex* myPowerSpectralFFT(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT RM, UINT RN, long double dLogParam);

// Populate destination image with the frequency phase-spectral of source image.
// To make the freq-spectral shown at the center, (-1)^(x+y) is applied before transform.
// To make the freq-spectral brighter, a logarithm transform is applied after transform.
//
// @param pDst: the destination image.
// @param pSrc: the source image.
// @param pInfo: points to image pixel info.
// @param dLogParam: [c] in [c * log(1 + r)].
// 
// @return origin calculated phase spectral.
//
MyComplex* myPhaseSpectral(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, long double dLogParam);
// FFT accelerated version of phase spectral.
// @see mycpFFT2 for details about RM and RN.
MyComplex* myPhaseSpectralFFT(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT RM, UINT RN, long double dLogParam);

// Populate the destination image with rebuilt pixels from spectrals.
// The effective power and phase data are stored in their real parts.
//
// @param dst: the destination image.
// @param info: the image pixel info.
// @param power: source power spectral.
// @param phase: source phase spectral.
//
void mySpectralReconstruct(MyBGRA* dst, MyBmpInfo* info, MyComplex* power, MyComplex* phase);
// FFT accelerated version of spectral reconstruct.
// @see mycpFFT2 for details about RM and RN.
void mySpectralReconstructFFT(MyBGRA* dst, MyBmpInfo* info, UINT RM, UINT RN, MyComplex* power, MyComplex* phase);

// Expand origin image to radix 2 of FFT.
//
// @param info: size info after padding.
// @param origin: points to origin image.
// @param originInfo: points to origin size info.
//
// @return expanded image after padding.
//
MyBGRA* myPaddingZeroImageFFT(MyBmpInfo* info, MyBGRA* origin, MyBmpInfo* originInfo);

// Expand origin spectral to radix 2 of FFT.
//
// @param info: size info after padding.
// @param origin: points to origin spectral.
// @param originInfo: points to origin size info.
//
// @return expanded spectral after padding.
//
MyComplex* myPaddingZeroSpectralFFT(MyComplex* origin, MyBmpInfo* originInfo);

// Process the image spectral with ideal low/high pass filter.
//
// @param spectral: spectral to be processed.
// @param info: size info of target spectral.
// @param center, radius: ILPF/IHPF required args.
// @param nFilterCount: number of required args.
//
void myTfuncIdealLPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* radius, UINT nFilterCount);
void myTfuncIdealHPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* radius, UINT nFilterCount);

// Process the image spectral with gaussian low/high pass filter.
//
// @param spectral: spectral to be processed.
// @param info: size info of target spectral.
// @param center, sigma: GLPF/GHPF required args.
// @param nFilterCount: number of required args.
//
void myTfuncGaussianLPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* sigma, UINT nFilterCount);
void myTfuncGaussianHPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* sigma, UINT nFilterCount);

// Process the image spectral with butterworth low/high pass filter.
//
// @param spectral: spectral to be processed.
// @param info: size info of target spectral.
// @param center, cutoff: BLPF/BHPF required args.
// @param nOrder: control shape of the curve.
// @param nFilterCount: number of required args.
//
void myTfuncButterworthLPF(MyComplex* spectral, MyBmpInfo* info,
                           POINT* center, double* cutoff, double nOrder, UINT nFilterCount);
void myTfuncButterworthHPF(MyComplex* spectral, MyBmpInfo* info,
                           POINT* center, double* cutoff, double nOrder, UINT nFilterCount);

// Process the image spectral with homomorphic filter (gaussian fitting).
//
// @param spectral: spectral to be processed.
// @param info: size info of target spectral.
// @param center, sigma: homomorphic filter required args.
// @param c: control slope of fitted curve.
// @param gammaLow: low threshold of fitted curve.
// @param gammaHigh: high threshold of fitted curve.
// @param nFilterCount: number of required args.
//
void myTfuncHomomorphicFilter(MyComplex* spectral, MyBmpInfo* info,
    POINT* center, double* sigma, double c, double gammaLow, double gammaHigh, UINT nFilterCount);

RECT myGetThresholdWndInitSize();

// Perform threshold processing on the image.
// 
// Note the related spectral should be discarded after threshold processing.
// If it is the spectral what you want to modify, use transfer-func instead.
//
// @param pDst: destination image buffer to write.
// @param pSrc: source image buffer to read value.
// @param pInfo: points to image pixel info.
// @param threshold: 0 or 255 boundary line.
//
void myThresholdProcess(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT8 threshold);

// Error function (inverse), approximated with numeric method.
double myErf(double x);
double myErfInverse(double x);

// Apply noise with gaussian distribution.
// 
// @param u: mean value.
// @param sigma: standard deviation.
//
void myApplyGaussianNoise(MyBGRA* data, MyBmpInfo* info, double u, double sigma);

// Apply noise with rayleigh distribution.
//
// @param a, b:
//              p(z) = 0, if z < a;
// otherwise:
//              p(z) = 2/b * (z - a) * e^(-(z-a)^2 / b).
// @remark
//              mean value  = a + sqrt(pi * b / 4)
//              variance    = b * (4 - pi) / 4
//
void myApplyRayleighNoise(MyBGRA* data, MyBmpInfo* info, double a, double b);

// Apply noise with erlang distribution.
//
// @param a, b:
//              p(z) = 0, if z < 0;
// otherwise:
//              p(z) = a^b * z^(b-1) * e^(-a * z) / (b-1)!.
// @remark
//              mean value  = b / a
//              variance    = b / a^2
//
void myApplyErlangNoise(MyBGRA* data, MyBmpInfo* info, double a, int b);

// Apply noise with exponential distribution.
//
// Speical case of Erlang noise when b = 1.
//
void myApplyExponentialNoise(MyBGRA* data, MyBmpInfo* info, double a);

// Apply noise with uniform distribution.
void myApplyUniformNoise(MyBGRA* data, MyBmpInfo* info, double start, double end);

// Apply salt & pepper noise.
// 
// @param salt: white pixel probability.
// @param pepper: black pixel probability.
//
void myApplySaltAndPepperNoise(MyBGRA* data, MyBmpInfo* info, double salt, double pepper);

//-------------------------------------------------------------------------------------------------
// Main Function
//-------------------------------------------------------------------------------------------------

#if defined(__GNUC__)
// There's no "wWinMain" for MinGW, so we simply use "WinMain" instead here.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nShowCmd)
#elif defined(_MSC_VER)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
#endif
{
    MSG msg;
    WNDCLASS wndclass;

    gInstance = hInstance;
    
    // Query system metrics info.
    gCxIcon = GetSystemMetrics(SM_CXICON);
    gCyIcon = GetSystemMetrics(SM_CYICON);

    gCaptionHeight = GetSystemMetrics(SM_CYCAPTION);

    gCharWidth = LOWORD(GetDialogBaseUnits());
    gCharHeight = HIWORD(GetDialogBaseUnits());

    gCxVertScr = GetSystemMetrics(SM_CXVSCROLL);
    gCyVertScr = GetSystemMetrics(SM_CYVSCROLL);
    gCxHorzScr = GetSystemMetrics(SM_CXHSCROLL);
    gCyHorzScr = GetSystemMetrics(SM_CYHSCROLL);

    gMaxClntWidth = GetSystemMetrics(SM_CXFULLSCREEN);
    // We have to take menu size into consideration.
    gMaxClntHeight = GetSystemMetrics(SM_CYFULLSCREEN) -
                     GetSystemMetrics(SM_CYMENU);

    UINT originStyle;
    // Main Window
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = MainWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = (gIcon = myLoadAppIcon());
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = gMainWndName;
    RegisterClass(&wndclass);

    // Second Window
    wndclass.lpfnWndProc = SecondWndProc;
    wndclass.lpszClassName = gSecondWndName;
    RegisterClass(&wndclass);

    // Gray Transform Window
    wndclass.lpfnWndProc = GrayTransWndProc;
    wndclass.lpszClassName = gGrayTransWndName;
    RegisterClass(&wndclass);

    // GTW Histogram Display Window
    originStyle = wndclass.style;
    wndclass.style |= CS_DBLCLKS;
    wndclass.lpfnWndProc = GtwHistDispWndProc;
    wndclass.lpszClassName = gGtwHistDispWndName;
    RegisterClass(&wndclass);
    wndclass.style = originStyle;

    // Domain Filter Window
    wndclass.lpfnWndProc = DomainFilterWndProc;
    wndclass.lpszClassName = gDomainFilterWndName;
    RegisterClass(&wndclass);

    // Domain Custom Core Window
    wndclass.lpfnWndProc = DomainCustCoreWndProc;
    wndclass.lpszClassName = gDomainCustCoreWndName;
    RegisterClass(&wndclass);

    // App About Window
    wndclass.lpfnWndProc = AppAboutWndProc;
    wndclass.lpszClassName = gAppAboutWndName;
    RegisterClass(&wndclass);

    // Threshold Window
    wndclass.lpfnWndProc = ThresholdWndProc;
    wndclass.lpszClassName = gThresholdWndName;
    RegisterClass(&wndclass);

    // Create main menu.
    gMenu = myLoadMainMenu();
    // Create main window.
    gMainWnd = CreateWindow(
        gMainWndName,
        L"Bitmap Viewer",
        WS_OVERLAPPEDWINDOW | WS_EX_COMPOSITED | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        gMenu,
        hInstance,
        NULL);
    // Create main accelerator table.
    gAccel = myLoadMainAccel();

    ShowWindow(gMainWnd, nShowCmd);
    UpdateWindow(gMainWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(gMainWnd, gAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Note we have created custom accelerator table.
    DestroyAcceleratorTable(gAccel);

    return (int)msg.wParam;
}

//-------------------------------------------------------------------------------------------------
// Helper Function Definition
//-------------------------------------------------------------------------------------------------

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                              Main Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// Use double-digit for main window widgets;
// 80 ~ 99 are available for the time being.
#define MAIN_MOVE_BTN 80
#define MAIN_SELECT_BTN 81
#define MAIN_CLIP_BTN 82
#define MAIN_EYEDROPPER_BTN 83
#define MAIN_FILL_BTN 84
#define MAIN_BRUSH_BTN 85
#define MAIN_BRUSH_RADIUS_LBL 86
#define MAIN_BRUSH_RADIUS_EDT 87
#define MAIN_COLOR_BLOCK_BTN 88

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    // Tool Bar
    static HWND
        btnMove, // Active to move the picture around.
        btnSelect, // Active to select a rectangle area.
        btnClip, // Push to clip current selected area.
        btnEyedropper, // Active to extract color under cursor.
        btnFill, // Push to fill up current selected area.
        btnBrush, // Active to draw pixels on the picture.
        lblBrushRadius,
        edtBrushRadius, // Modify brush pixel block radius.
        btnColorBlock; // Modify brush pixel pure color.

    // Current selected tool operation.
    static HWND currOperation;
    // Whether the operation could be done.
    static BOOL bReadyForWork = FALSE;

    // Available display area for image.
    static int validWidth = 0;
    static int validHeight = 0;

    // Work with Move tool operation.
    static int xImgAnchor = 0, yImgAnchor = 0;

    // Work with Select, Clip & Fill tool operation.
    static RECT selectArea = { 0, 0, 0, 0 };
    static HPEN slctDashPen;

    // Work with Brush tool operation.
    static UINT nBrushRadius = 10;

    // Current selected color for Fill & Brush.
    static MyBGRA currColor = { 255, 255, 255, 255 };
    static COLORREF aCustomColors[16];
    static CHOOSECOLOR sChooseColor; // used to create color dialog
    static HBRUSH brColorBlock; // used to erase color block bkgn

    switch (message)
    {
    case WM_CREATE:
    {
        // Initialize common dialog utils.
        myInitFileDialogInfo(hwnd);

        // Grayed all operations until user open an image.
        myDisableOperationMenus();

        // Initialize choose color struct.
        CHOOSECOLOR* pcc = &sChooseColor;
        pcc->lStructSize = sizeof(CHOOSECOLOR);
        pcc->hwndOwner = hwnd;
        pcc->hInstance = NULL; // HWND

        // Designate/Return user selected color.
        pcc->rgbResult = RGB(
            currColor.R, currColor.G, currColor.B);

        pcc->lpCustColors = aCustomColors;
        pcc->Flags = CC_RGBINIT;
        pcc->lCustData = 0; // NULL
        pcc->lpfnHook = NULL;
        pcc->lpTemplateName = NULL;

        // Create brush to erase color block's bkgn.
        brColorBlock = CreateSolidBrush(pcc->rgbResult);

        RECT rc;
        GetClientRect(hwnd, &rc);
        // Create tool bar widgets.
#define MY_CREATE_MAIN_BTN(Btn_Handle, Btn_Text, Btn_ID, Btn_Idx) \
    MY_CREATE_MAIN_BTN_EX(Btn_Handle, Btn_Text, Btn_ID, 5 + Btn_Idx * (8 * gCharWidth + 5))

#define MY_CREATE_MAIN_BTN_EX(Btn_Handle, Btn_Text, Btn_ID, Btn_Pos) \
    Btn_Handle = CreateWindow( \
        L"button", \
        Btn_Text, \
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, \
        Btn_Pos, \
        rc.bottom - 2 * gCharHeight - 5, \
        8 * gCharWidth, \
        2 * gCharHeight, \
        hwnd, \
        (HMENU)Btn_ID, \
        gInstance, \
        NULL); /* Hide on created */ \
        ShowWindow(Btn_Handle, SW_HIDE);

        MY_CREATE_MAIN_BTN(btnMove, L"移动 M", MAIN_MOVE_BTN, 0);
        MY_CREATE_MAIN_BTN(btnSelect, L"选取 S", MAIN_SELECT_BTN, 1);
        MY_CREATE_MAIN_BTN(btnClip, L"裁剪 C", MAIN_CLIP_BTN, 2);
        MY_CREATE_MAIN_BTN(btnEyedropper, L"吸管 E", MAIN_EYEDROPPER_BTN, 3);
        MY_CREATE_MAIN_BTN(btnFill, L"填充 F", MAIN_FILL_BTN, 4);
        MY_CREATE_MAIN_BTN(btnBrush, L"画刷 B", MAIN_BRUSH_BTN, 5);

        // Create brush radius text label & edit line.
        lblBrushRadius = CreateWindow(
            L"static",
            L"半径",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            35 + 48 * gCharWidth,
            rc.bottom - 3 * gCharHeight / 2 - 5,
            4 * gCharWidth,
            gCharHeight,
            hwnd,
            (HMENU)MAIN_BRUSH_RADIUS_LBL,
            gInstance,
            NULL);
        ShowWindow(lblBrushRadius, SW_HIDE);
        edtBrushRadius = CreateWindow(
            L"edit",
            L"10",
            WS_CHILD | WS_VISIBLE,
            40 + 52 * gCharWidth,
            rc.bottom - 3 * gCharHeight / 2 - 5,
            2 * gCharWidth + 5,
            gCharHeight,
            hwnd,
            (HMENU)MAIN_BRUSH_RADIUS_EDT,
            gInstance,
            NULL);
        ShowWindow(edtBrushRadius, SW_HIDE);

        // Create color select button.
        btnColorBlock = CreateWindow(
            L"button",
            NULL,
            // Use BS_OWNERDRAW to customize bkgn of color block.
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            50 + 54 * gCharWidth,
            rc.bottom - 2 * gCharHeight - 5,
            2 * gCharHeight,
            2 * gCharHeight,
            hwnd,
            (HMENU)MAIN_COLOR_BLOCK_BTN,
            gInstance,
            NULL);
        ShowWindow(btnColorBlock, SW_HIDE);

#undef MY_CREATE_MAIN_BTN_EX
#undef MY_CREATE_MAIN_BTN

        validWidth = gBmpInfo.nWidth;
        validHeight = gBmpInfo.nHeight;
        // Create select rect frame pen.
        slctDashPen = CreatePen(PS_DASH, 1, RGB(255, 0, 0));

        // Select move as initial operation.
        currOperation = btnMove;
        SendMessage(btnMove, BM_SETSTATE, 1, 0);

        return 0;
    }
    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    // Tool bar widget accelerators.
    case WM_KEYDOWN:
    {
        // There's no need to erase since double-buffer is enabled.
        InvalidateRect(hwnd, NULL, FALSE);

        // Active Clip selected state button when hold down.
        if (wParam == 'C')
        {
            SendMessage(btnClip, BM_SETSTATE, 1, 0);
            // Trigger operation immediately.
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(MAIN_CLIP_BTN, 0), 0);
            return 0;
        }

#define MY_SEND_BTN_MSG(Btn_Handle, Btn_Key, Btn_ID) \
    if (wParam == Btn_Key) SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(Btn_ID, 0), 0)

        MY_SEND_BTN_MSG(btnMove, 'M', MAIN_MOVE_BTN);
        MY_SEND_BTN_MSG(btnSelect, 'S', MAIN_SELECT_BTN);
        MY_SEND_BTN_MSG(btnClip, 'C', MAIN_CLIP_BTN);
        MY_SEND_BTN_MSG(btnEyedropper, 'E', MAIN_EYEDROPPER_BTN);
        MY_SEND_BTN_MSG(btnFill, 'F', MAIN_FILL_BTN);
        MY_SEND_BTN_MSG(btnBrush, 'B', MAIN_BRUSH_BTN);

        POINT pt;
        // Force repaint cursor.
        GetCursorPos(&pt);
        SetCursorPos(pt.x, pt.y);

#undef MY_SEND_BTN_MSG

        return 0;
    }
    case WM_KEYUP:
    {
        if (wParam == 'C')
        {
            // Cancel Clip button selected state after key up.
            SendMessage(btnClip, BM_SETSTATE, 0, 0);
        }
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        static int csorCurrX, csorCurrY;
        static int csorLastX, csorLastY;

        csorCurrX = GET_X_LPARAM(lParam);
        csorCurrY = GET_Y_LPARAM(lParam);

        if (bReadyForWork) // in workspace
        {
            //------------------------------------------------------------------- Move Tool
            if (currOperation == btnMove)
            {
                if (wParam & MK_LBUTTON)
                {
                    // Note +/- to match image coordinate.
                    xImgAnchor -= (csorCurrX - csorLastX);
                    yImgAnchor += (csorCurrY - csorLastY);

                    // Clamp image anchor offset into valid range.
                    // X offset: 0 ~ (ImgW - ValidW)
                    int xOffsetMost = gBmpInfo.nWidth - validWidth;
                    xImgAnchor = max(0, min(xOffsetMost, xImgAnchor));
                    // Y offset: (ValidH - ImgH) ~ 0
                    int yOffsetMost = validHeight - gBmpInfo.nHeight;
                    yImgAnchor = max(yOffsetMost, min(0, yImgAnchor));

                    // There's no need to erase since double-buffer is enabled.
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            //------------------------------------------------------------------- Select Tool
            else if (currOperation == btnSelect)
            {
                if (wParam & MK_LBUTTON)
                {

#define MY_LEFT_SPACE 1
#define MY_RIGHT_SPACE 2
#define MY_TOP_SPACE 3
#define MY_BOTTOM_SPACE 4

                    // Note the move direction is significant!
                    static int xMoveDir = MY_RIGHT_SPACE, yMoveDir = MY_BOTTOM_SPACE;

                    // From image coordinate to window coordinate.
                    OffsetRect(&selectArea, 18 - abs(xImgAnchor), 18 - abs(yImgAnchor));

                    switch (xMoveDir)
                    {
                    case MY_LEFT_SPACE:
                        if (csorCurrX > selectArea.right) // Enter right space.
                        {
                            xMoveDir = MY_RIGHT_SPACE;
                            selectArea.right = csorCurrX;
                        }
                        else selectArea.left = csorCurrX;
                        break;
                    case MY_RIGHT_SPACE:
                        if (csorCurrX < selectArea.left) // Enter left space.
                        {
                            xMoveDir = MY_LEFT_SPACE;
                            selectArea.left = csorCurrX;
                        }
                        else selectArea.right = csorCurrX;
                        break;
                    default:
                        break;
                    }

                    switch (yMoveDir)
                    {
                    case MY_TOP_SPACE:
                        if (csorCurrY > selectArea.bottom) // Enter bottom space.
                        {
                            yMoveDir = MY_BOTTOM_SPACE;
                            selectArea.bottom = csorCurrY;
                        }
                        else selectArea.top = csorCurrY;
                        break;
                    case MY_BOTTOM_SPACE:
                        if (csorCurrY < selectArea.top) // Enter top space.
                        {
                            yMoveDir = MY_TOP_SPACE;
                            selectArea.top = csorCurrY;
                        }
                        else selectArea.bottom = csorCurrY;
                        break;
                    default:
                        break;
                    }

                    // From window coordinate to image coordinate.
                    OffsetRect(&selectArea, -18 + abs(xImgAnchor), -18 + abs(yImgAnchor));

#undef MY_LEFT_SPACE
#undef MY_RIGHT_SPACE
#undef MY_TOP_SPACE
#undef MY_BOTTOM_SPACE

                    // Clamp to available image area. Cast UINT to INT to avoid overflow.
                    selectArea.left = max(0, min((int)gBmpInfo.nWidth, selectArea.left));
                    selectArea.right = max(0, min((int)gBmpInfo.nWidth, selectArea.right));
                    selectArea.top = max(0, min((int)gBmpInfo.nHeight, selectArea.top));
                    selectArea.bottom = max(0, min((int)gBmpInfo.nHeight, selectArea.bottom));

                    // There's no need to erase since double-buffer is enabled.
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            //------------------------------------------------------------------- Eyedropper Tool
            else if (currOperation == btnEyedropper)
            {
                if (wParam & MK_LBUTTON)
                {
                    currColor = gImage[csorCurrX - 18 + abs(xImgAnchor) + // Note image Y-axis is flipped.
                        (gBmpInfo.nHeight - (csorCurrY - 18 + abs(yImgAnchor))) * gBmpInfo.nWidth];

                    DeleteObject(brColorBlock); // Recreate bkgn pen with picked color.
                    brColorBlock = CreateSolidBrush(RGB(currColor.R, currColor.G, currColor.B));

                    InvalidateRect(btnColorBlock, NULL, TRUE);
                }
            }
            //------------------------------------------------------------------- Brush Tool
            else if (currOperation == btnBrush)
            {
                // Notify show hint rect.
                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);

                if (wParam & MK_LBUTTON)
                {
                    int xStart = csorCurrX - nBrushRadius - 18 + abs(xImgAnchor); xStart = max(0, xStart);
                    int yStart = csorCurrY - nBrushRadius - 18 + abs(yImgAnchor); yStart = max(0, yStart);
                    int xEnd = xStart + 2 * (int)nBrushRadius + 1; xEnd = min((int)gBmpInfo.nWidth, xEnd);
                    int yEnd = yStart + 2 * (int)nBrushRadius + 1; yEnd = min((int)gBmpInfo.nHeight, yEnd);
                    // Paint square block with brush color.
                    for (int i = xStart; i < xEnd; ++i)
                        for (int j = yStart; j < yEnd; ++j)
                        {
                            // Note image Y-axis is flipped.
                            gImage[i + (gBmpInfo.nHeight - j - 1) * gBmpInfo.nWidth] = currColor;
                        }
                    // Process spectral by the way (if has).
                    // IMPORTANT: We don't support complex spectral operations. The power value
                    // is simply set to 0 when the brush tool is applied (whatever the color is).
                    if (gSpectral != NULL)
                    {
                        for (int i = xStart; i < xEnd; ++i)
                            for (int j = yStart; j < yEnd; ++j)
                            {
                                // Note spectral Y-axis is flipped.
                                gSpectral[i + (gBmpInfo.nHeight - j - 1) * gBmpInfo.nWidth].real = 0;
                            }
                    }
                    // There's no need to erase since double-buffer is enabled.
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
        }
        csorLastX = csorCurrX;
        csorLastY = csorCurrY;
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        SetFocus(hwnd);

        int csorCurrX = GET_X_LPARAM(lParam);
        int csorCurrY = GET_Y_LPARAM(lParam);

        if (bReadyForWork) // in workspace
        {
            // Note these fields are only valid when select is verified,
            // i.e. left != 0 || right != 0 || top != 0 || bottom != 0

            //------------------------------------------------------------------- Select Tool
            if (currOperation == btnSelect)
            {
                int tmpX = csorCurrX - 18 + abs(xImgAnchor);
                int tmpY = csorCurrY - 18 + abs(yImgAnchor);

                // We must cast UINT to INT to avoid overflow.
                tmpX = max(0, min((int)gBmpInfo.nWidth, tmpX));
                tmpY = max(0, min((int)gBmpInfo.nHeight, tmpY));

                // Rest select rect frame to button down position.
                selectArea.left = selectArea.right = tmpX;
                selectArea.top = selectArea.bottom = tmpY;
            }
            //------------------------------------------------------------------- Eyedropper Tool
            else if (currOperation == btnEyedropper)
            {
                currColor = gImage[csorCurrX - 18 + abs(xImgAnchor) + // Note image Y-axis is flipped.
                    (gBmpInfo.nHeight - (csorCurrY - 18 + abs(yImgAnchor))) * gBmpInfo.nWidth];

                DeleteObject(brColorBlock); // Recreate bkgn pen with picked color.
                brColorBlock = CreateSolidBrush(RGB(currColor.R, currColor.G, currColor.B));

                InvalidateRect(btnColorBlock, NULL, TRUE);
            }
            //------------------------------------------------------------------- Fill Tool
            else if (currOperation == btnFill)
            {
                int i, j, idx; // Declare common index variables here.

                // Populate selected (inversely) area if verified.
                if (selectArea.left != 0 || selectArea.right != 0 ||
                    selectArea.top != 0 || selectArea.bottom != 0)
                {
                    // Selected area size.
                    int tmpImgW = selectArea.right - selectArea.left;
                    int tmpImgH = selectArea.bottom - selectArea.top;
                    // Note image Y-axis is flipped.
                    int yStartIdx = gBmpInfo.nHeight - selectArea.top - tmpImgH;
                     
                    // From window coordinate to image coordinate.
                    int tmpCsorX = csorCurrX - 18 + abs(xImgAnchor);
                    int tmpCsorY = csorCurrY - 18 + abs(yImgAnchor);

                    // Selected
                    if (tmpCsorX >= selectArea.left && tmpCsorX <= selectArea.right &&
                        tmpCsorY >= selectArea.top && tmpCsorY <= selectArea.bottom)
                    {
                        for (i = 0; i < tmpImgW; ++i)
                            for (j = 0; j < tmpImgH; ++j)
                            {
                                idx = (i + selectArea.left) + (yStartIdx + j) * gBmpInfo.nWidth;
                                gImage[idx] = currColor; // Fill up internal area with pure color.
                            }
                    }
                    // Inversely Selected
                    else
                    {
                        // Left
                        for (i = 0; i < selectArea.left; ++i)
                            for (j = 0; j < gBmpInfo.nHeight; ++j)
                            {
                                idx = i + j * gBmpInfo.nWidth;
                                gImage[idx] = currColor;
                            }
                        // Top
                        for (i = selectArea.left; i < selectArea.left + tmpImgW; ++i)
                            for (j = 0; j < yStartIdx; ++j)
                            {
                                idx = i + j * gBmpInfo.nWidth;
                                gImage[idx] = currColor;
                            }
                        // Right
                        for (i = selectArea.left + tmpImgW; i < gBmpInfo.nWidth; ++i)
                            for (j = 0; j < gBmpInfo.nHeight; ++j)
                            {
                                idx = i + j * gBmpInfo.nWidth;
                                gImage[idx] = currColor;
                            }
                        // Bottom
                        for (i = selectArea.left; i < selectArea.left + tmpImgW; ++i)
                            for (j = yStartIdx + tmpImgH; j < gBmpInfo.nHeight; ++j)
                            {
                                idx = i + j * gBmpInfo.nWidth;
                                gImage[idx] = currColor;
                            }
                    }
                }
                // Populate the whole image otherwise.
                else
                {
                    for (i = 0; i < gBmpInfo.nWidth; ++i)
                        for (j = 0; j < gBmpInfo.nHeight; ++j)
                        {
                            gImage[i + j * gBmpInfo.nWidth] = currColor;
                        }
                }
                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
            }
            //------------------------------------------------------------------- Brush Tool
            else if (currOperation == btnBrush)
            {
                int xStart = csorCurrX - nBrushRadius - 18 + abs(xImgAnchor); xStart = max(0, xStart);
                int yStart = csorCurrY - nBrushRadius - 18 + abs(yImgAnchor); yStart = max(0, yStart);
                int xEnd = xStart + 2 * nBrushRadius + 1; xEnd = min(gBmpInfo.nWidth, xEnd);
                int yEnd = yStart + 2 * nBrushRadius + 1; yEnd = min(gBmpInfo.nHeight, yEnd);
                // Paint square block with brush color.
                for (int i = xStart; i < xEnd; ++i)
                    for (int j = yStart; j < yEnd; ++j)
                    {
                        // Note image Y-axis is flipped.
                        gImage[i + (gBmpInfo.nHeight - j - 1) * gBmpInfo.nWidth] = currColor;
                    }
                // Process spectral by the way (if has).
                // IMPORTANT: We don't support complex spectral operations. The power value
                // is simply set to 0 when the brush tool is applied (whatever the color is).
                if (gSpectral != NULL)
                {
                    for (int i = xStart; i < xEnd; ++i)
                        for (int j = yStart; j < yEnd; ++j)
                        {
                            // Note spectral Y-axis is flipped.
                            gSpectral[i + (gBmpInfo.nHeight - j - 1) * gBmpInfo.nWidth].real = 0;
                        }
                }
                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
    case WM_LBUTTONUP:
    {
        int csorCurrX = GET_X_LPARAM(lParam);
        int csorCurrY = GET_Y_LPARAM(lParam);

        if (bReadyForWork) // in workspace
        {
            //------------------------------------------------------------------- Select Tool
            if (currOperation == btnSelect)
            {
                // Check whether last select is valid.
                if (selectArea.left == selectArea.right ||
                    selectArea.top == selectArea.bottom)
                {
                    // Cancel origin selected rect frame.
                    SetRectEmpty(&selectArea);
                }
                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
    case WM_SIZE:
    {
        // Display tool bar when image loaded.
        static BOOL bShowToolBar = FALSE;
        if (!bShowToolBar && gImage != NULL)
        {
            bShowToolBar = TRUE;
            ShowWindow(btnMove, SW_SHOW);
            ShowWindow(btnSelect, SW_SHOW);
            ShowWindow(btnClip, SW_SHOW);
            ShowWindow(btnEyedropper, SW_SHOW);
            ShowWindow(btnFill, SW_SHOW);
            ShowWindow(btnBrush, SW_SHOW);
            ShowWindow(lblBrushRadius, SW_SHOW);
            ShowWindow(edtBrushRadius, SW_SHOW);
            ShowWindow(btnColorBlock, SW_SHOW);
        }

        RECT rc;
        GetClientRect(hwnd, &rc);
        int clntWidth = rc.right - rc.left;
        int clntHeight = rc.bottom - rc.top;

        // Recalculate available image area.
        validWidth = rc.right - rc.left - 36;
        validHeight = rc.bottom - rc.top - 41 - 2 * gCharHeight;
        // Clamp to origin image size when window is larger.
        validWidth = min(validWidth, gBmpInfo.nWidth);
        validHeight = min(validHeight, gBmpInfo.nHeight);

        // Always reset image anchor when resized.
        xImgAnchor = yImgAnchor = 0;
        // Always reset select rect frame when resized.
        selectArea.left = selectArea.top = selectArea.right = selectArea.bottom = 0;

        // Keep tool bar attached to window bottom.
#define MY_STICK_BOTTOM_BTN(Btn_Handle, Btn_Idx) \
    GetWindowRect(Btn_Handle, &rc); \
    MoveWindow(Btn_Handle, \
        5 + Btn_Idx * (8 * gCharWidth + 5), \
        clntHeight - 5 - 2 * gCharHeight, \
        rc.right - rc.left, rc.bottom - rc.top, TRUE)

        MY_STICK_BOTTOM_BTN(btnMove, 0);
        MY_STICK_BOTTOM_BTN(btnSelect, 1);
        MY_STICK_BOTTOM_BTN(btnClip, 2);
        MY_STICK_BOTTOM_BTN(btnEyedropper, 3);
        MY_STICK_BOTTOM_BTN(btnFill, 4);
        MY_STICK_BOTTOM_BTN(btnBrush, 5);

#undef MY_STICK_BOTTOM_BTN

        GetWindowRect(lblBrushRadius, &rc);
        MoveWindow(lblBrushRadius, 35 + 48 * gCharWidth,
                   clntHeight - 3 * gCharHeight / 2 - 5,
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);

        GetWindowRect(edtBrushRadius, &rc);
        MoveWindow(edtBrushRadius, 40 + 52 * gCharWidth,
                   clntHeight - 3 * gCharHeight / 2 - 5,
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);

        GetWindowRect(btnColorBlock, &rc);
        MoveWindow(btnColorBlock, 50 + 54 * gCharWidth,
                   clntHeight - 2 * gCharHeight - 5,
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_SETFOCUS:
        // Close app about window if focused.
        if (IsWindow(gAppAboutWnd)) DestroyWindow(gAppAboutWnd);
        return 0;

    case WM_SETCURSOR:
    {
        if (gImage != NULL)
        {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // Get valid area boundary.
            int tmpWndRightMost = rc.right - 18;
            int tmpImgRightMost = 18 + gBmpInfo.nWidth;
            int tmpWndBottomMost = rc.bottom - 23 - 2 * gCharHeight;
            int tmpImgBottomMost = 18 + gBmpInfo.nHeight;

            // Select cursor appearance according to current tool operation.
#define MY_SET_CURSOR(Btn_Handle, Cursor_ID) \
    (currOperation == Btn_Handle) SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(Cursor_ID)))

#define MY_SET_CURSOR_IF(Btn_Handle, Cursor_ID) if MY_SET_CURSOR(Btn_Handle, Cursor_ID)
#define MY_SET_CURSOR_ELIF(Btn_Handle, Cursor_ID) else if MY_SET_CURSOR(Btn_Handle, Cursor_ID)

            // Check whether above the image.
            
            // Clamp to valid image area for these tool operations.
            if (pt.x >= 18 && pt.x < min(tmpImgRightMost, tmpWndRightMost) &&
                pt.y >= 18 && pt.y < min(tmpImgBottomMost, tmpWndBottomMost))
            {
                MY_SET_CURSOR_IF(btnMove, IDC_SIZEALL);
                MY_SET_CURSOR_ELIF(btnSelect, IDC_CROSS);
                MY_SET_CURSOR_ELIF(btnEyedropper, IDC_PIN);
                MY_SET_CURSOR_ELIF(btnFill, IDC_HAND);
                MY_SET_CURSOR_ELIF(btnBrush, IDC_PERSON);

                bReadyForWork = TRUE; // cursor in workspace

                return 0;
            }
            // We decide that cursor area is more larger for Select tool.
            else if (pt.x > 0 && pt.x < rc.right &&
                     pt.y > 0 && pt.y < (rc.bottom - 5 - 2 * gCharHeight))
            {
                if (currOperation == btnSelect)
                {
                    SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS)));
                    bReadyForWork = TRUE; // cursor in Select tool workspace
                    return 0;
                }
            }

            // There's no need to erase since double-buffer is enabled.
            if (bReadyForWork) InvalidateRect(hwnd, NULL, FALSE);
            bReadyForWork = FALSE; // cursor out of workspace

#undef MY_SET_CURSOR
#undef MY_SET_CURSOR_IF
#undef MY_SET_CURSOR_ELIF
        }
        break; // Use default cursor in other places.
    }
    case WM_PAINT:
    {
        hdc = BeginPaint(hwnd, &ps);

        RECT rcClnt;
        GetClientRect(hwnd, &rcClnt);
        //--------------------------------------------------------------------- Double Buffer Start
        HDC hMemDC = CreateCompatibleDC(hdc); // Acquire copy device context.
        HBITMAP hMemBmp = CreateCompatibleBitmap( // Draw on temporary bitmap.
            hdc, rcClnt.right - rcClnt.left, rcClnt.bottom - rcClnt.top);
        SelectObject(hMemDC, hMemBmp);
        // Erase with master window's bkgn brush.
        FillRect(hMemDC, &rcClnt, (HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND));
        //---------------------------------------------------------------------

        int x, y;
        if (gImage == NULL)
        {
            static WCHAR szHomeLabel[] = L"新建或打开一个图片";

            SIZE tmpSize;
            GetTextExtentExPoint(hMemDC, szHomeLabel, _countof(szHomeLabel) - 1, INT_MAX, NULL, NULL, &tmpSize);

            x = (width - tmpSize.cx) / 2;
            y = (height - tmpSize.cy) / 2;
            // Display hint text in the center of main window.
            TextOut(hMemDC, x, y, szHomeLabel, _countof(szHomeLabel) - 1);
        }
        else
        {
            int xValidRight = 18 + validWidth;
            int yValidBottom = 18 + validHeight;

            // We have to do this to stick the image at left top,
            // since Y-axis is flipped in origin pixel buffer.
            int tmpImgY = validHeight - gBmpInfo.nHeight;

            // Display image from left top corner of main window.
            myDisplayImage(hMemDC, 18, 18, max(0, validWidth), max(0, validHeight),
                           xImgAnchor, abs(min(0, tmpImgY)) + yImgAnchor, gImage, &gBmpInfo);
            
            // Display selected rect frame (if verified).
            if (selectArea.left != 0 || selectArea.right != 0 || selectArea.top != 0 || selectArea.bottom != 0)
            {
                HPEN tmpPen = SelectObject(hMemDC, slctDashPen);
                HBRUSH tmpBrush = SelectObject(hMemDC, GetStockObject(NULL_BRUSH));

                // Coordinate: image -> window -> image.
                OffsetRect(&selectArea, 18 - abs(xImgAnchor), 18 - abs(yImgAnchor));
                Rectangle(hMemDC, max(18, selectArea.left), max(18, selectArea.top),
                          min(xValidRight, selectArea.right), min(yValidBottom, selectArea.bottom));
                OffsetRect(&selectArea, -18 + abs(xImgAnchor), -18 + abs(yImgAnchor));

                SelectObject(hMemDC, tmpPen);
                SelectObject(hMemDC, tmpBrush);
            }

            // Display brush radius hint (if using brush tool).
            if (currOperation == btnBrush && bReadyForWork)
            {
                POINT hintPT;
                GetCursorPos(&hintPT);
                ScreenToClient(hwnd, &hintPT);

                // Hint rect visible boundary.
                int hintLeft = hintPT.x - (int)nBrushRadius, hintTop = hintPT.y - (int)nBrushRadius;
                // Note [FillRect] wouldn't draw the right bottom border of the rect,
                // so we need to expand the right bottom coordinate with 1 pixel here.
                int hintRight = hintPT.x + (int)nBrushRadius + 1, hintBottom = hintPT.y + (int)nBrushRadius + 1;

                // In case paint out of valid area.
                int xMostRight = 18 + validWidth, yMostBottom = 18 + validHeight;

                RECT hintRC = { max(18, hintLeft), max(18, hintTop),
                    min(xMostRight, hintRight), min(yMostBottom, hintBottom) };

                FillRect(hMemDC, &hintRC, brColorBlock);
            }
        }

        //--------------------------------------------------------------------- Double Buffer End
        BitBlt(hdc, 0, 0, rcClnt.right - rcClnt.left,
               rcClnt.bottom - rcClnt.top, hMemDC, 0, 0, SRCCOPY);
        DeleteDC(hMemDC);
        DeleteObject(hMemBmp);
        //---------------------------------------------------------------------

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CTLCOLORBTN:
    {
        // Overwrite the subroutine to customize color block button.
        // Update color block's background with user selected color.
        if ((HWND)lParam == btnColorBlock)
        {
            HDC hdcColorBlock = (HDC)wParam;

            RECT rc;
            GetClientRect(btnColorBlock, &rc);
            // Select pen & brush.
            HPEN tmpPen = SelectObject(hdcColorBlock, GetStockObject(BLACK_PEN));
            HBRUSH tmpBrush = SelectObject(hdcColorBlock, GetStockObject(NULL_BRUSH));
            // Draw indicator rectangle border.
            Rectangle(hdcColorBlock, rc.left, rc.top, rc.right, rc.bottom);
            // Restore device context.
            SelectObject(hdcColorBlock, tmpPen);
            SelectObject(hdcColorBlock, tmpBrush);

            return (LRESULT)brColorBlock;
        }
        // goto default window process
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
#define MY_SET_BTN_STATE(Btn_Handle, Btn_ID) \
    SendMessage(Btn_Handle, BM_SETSTATE, (LOWORD(wParam) == Btn_ID) ? 1 : 0, 0)

#define MY_POLLSET_BTN_STATES SetFocus(hwnd); \
    MY_SET_BTN_STATE(btnMove, MAIN_MOVE_BTN); \
    MY_SET_BTN_STATE(btnSelect, MAIN_SELECT_BTN); \
    MY_SET_BTN_STATE(btnEyedropper, MAIN_EYEDROPPER_BTN); \
    MY_SET_BTN_STATE(btnFill, MAIN_FILL_BTN); \
    MY_SET_BTN_STATE(btnBrush, MAIN_BRUSH_BTN)

        // Simulate radio button tool bar.
        case MAIN_MOVE_BTN:
            currOperation = btnMove;
            goto update_tool_bar_state;
            return 0;
        case MAIN_SELECT_BTN:
            currOperation = btnSelect;
            goto update_tool_bar_state;
            return 0;
        case MAIN_CLIP_BTN:
        {
            SetFocus(hwnd);
            int tmpImgW = selectArea.right - selectArea.left;
            int tmpImgH = selectArea.bottom - selectArea.top;
            // Verified area found.
            if (tmpImgW > 0 && tmpImgW <= gBmpInfo.nWidth &&
                tmpImgH > 0 && tmpImgH <= gBmpInfo.nHeight)
            {
                MyBGRA* tmpImg = (MyBGRA*)malloc(tmpImgW * tmpImgH * sizeof(MyBGRA));
                myAssert(tmpImg);
                // Copy part area data from origin image.
                for (int i = 0; i < tmpImgW; ++i)
                    for (int j = 0; j < tmpImgH; ++j)
                    {
                        int srcIdx = (i + selectArea.left) + // Note image Y-axis is flipped.
                            (gBmpInfo.nHeight - selectArea.top - tmpImgH + j) * gBmpInfo.nWidth;

                        tmpImg[i + j * tmpImgW] = gImage[srcIdx];
                    }
                free(gImage);
                // Replace origin image with clipped one.
                gImage = tmpImg;
                gBmpInfo.nWidth = tmpImgW;
                gBmpInfo.nHeight = tmpImgH;

                // Resize main window to suit clipped image.
                if (!IsMaximized(hwnd))
                {
                    RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 41 + 2 * gCharHeight };
                    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                    LONG tmpW = rc.right - rc.left, tmpH = rc.bottom - rc.top;
                    GetWindowRect(hwnd, &rc);
                    MoveWindow(hwnd, rc.left, rc.top, tmpW, tmpH, TRUE);
                }
                else SendMessage(hwnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(gMaxClntWidth, gMaxClntHeight));

                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        case MAIN_EYEDROPPER_BTN:
            currOperation = btnEyedropper;
            goto update_tool_bar_state;
            return 0;
        case MAIN_FILL_BTN:
            currOperation = btnFill;
            goto update_tool_bar_state;
            return 0;
        case MAIN_BRUSH_BTN:
            currOperation = btnBrush;
            // Notify show hint rect.
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            goto update_tool_bar_state;
            return 0;

        update_tool_bar_state: MY_POLLSET_BTN_STATES; return 0;

#undef MY_SET_BTN_STATE
#undef MY_POLLSET_BTN_STATES

        case MAIN_BRUSH_RADIUS_EDT:
        {
            WCHAR szRadiusParam[10];
            GetWindowText(edtBrushRadius, szRadiusParam, 10);
            int iUserInput = 1;
            if (swscanf(szRadiusParam, L"%i", &iUserInput) == 1)
                nBrushRadius = (UINT)iUserInput;
            // Make sure brush radius is positive.
            nBrushRadius = max(1, nBrushRadius);
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case MAIN_COLOR_BLOCK_BTN:
        {
            // User selected color.
            if (ChooseColor(&sChooseColor))
            {
                COLORREF color = sChooseColor.rgbResult;

                // Update user selected color.
                currColor.R = (color >>  0) & 0x0FF;
                currColor.G = (color >>  8) & 0x0FF;
                currColor.B = (color >> 16) & 0x0FF;

                // Color block button's background will be updated in WM_CTLCOLORBTN.
                DeleteObject(brColorBlock);
                brColorBlock = CreateSolidBrush(color);
                InvalidateRect(btnColorBlock, NULL, TRUE);
            }
            return 0;
        }
        case IDM_FILE_NEW_256x256:
        case IDM_FILE_NEW_512x512:
        case IDM_FILE_NEW_1024x1024:
        {
            gBmpInfo.nWidth = gBmpInfo.nHeight = (0x100 << (LOWORD(wParam) - IDM_FILE_NEW_256x256));
            // Create image with specific size.
            if (gImage != NULL) free(gImage);
            size_t nByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
            gImage = (MyBGRA*)malloc(nByteSize);
            myAssert(gImage);
            // Initialized as pure black image.
            ZeroMemory(gImage, nByteSize);

            // Unlink current opened image file.
            gHasExternalFile = FALSE;
            SetWindowText(gMainWnd, L"Untitled - (未保存)");

            // Resize main window to suit created image.
            if (!IsMaximized(hwnd))
            {
                RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 41 + 2 * gCharHeight };
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                LONG tmpW = rc.right - rc.left, tmpH = rc.bottom - rc.top;
                GetWindowRect(hwnd, &rc);
                MoveWindow(hwnd, rc.left, rc.top, tmpW, tmpH, TRUE);
            }
            else SendMessage(hwnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(gMaxClntWidth, gMaxClntHeight));

            // Have fun with operations.
            myEnableOperationMenus();
            DrawMenuBar(hwnd);

            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case IDM_FILE_OPEN:
        {
            static WCHAR tmpFileName[MAX_PATH] = L"\0";
            static WCHAR tmpTitleName[MAX_PATH] = L"\0";
            // Show Open-File dialog.
            if (myOpenFileDialog(hwnd, tmpFileName, tmpTitleName))
            {
                lstrcpyW(gFileName, tmpFileName);
                lstrcpyW(gTitleName, tmpTitleName);

                MyBmpInfo tmpInfo;
                MyBGRA* tmpImg = myReadBmp(gFileName, &tmpInfo);
                if (tmpImg == NULL)
                {
                    MessageBox(hwnd, L"无法导入该 BMP 图片, 可能文件已损坏.", L"警告", MB_OK | MB_ICONEXCLAMATION);
                    return 0;
                }

                // Update main window caption.
                SetWindowText(hwnd, gFileName);
                // Now the image has been connected with an external file.
                gHasExternalFile = TRUE;

                // Replace main window background with selected image.
                if (gImage != NULL) free(gImage);
                if (gSpectral != NULL)
                {
                    free(gSpectral);
                    gSpectral = NULL;
                }
                gImage = tmpImg;
                gBmpInfo = tmpInfo;

                // Resize main window to suit selected image.
                if (!IsMaximized(hwnd))
                {
                    RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 41 + 2 * gCharHeight };
                    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                    LONG tmpW = rc.right - rc.left, tmpH = rc.bottom - rc.top;
                    GetWindowRect(hwnd, &rc);
                    MoveWindow(hwnd, rc.left, rc.top, tmpW, tmpH, TRUE);
                }
                else SendMessage(hwnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(gMaxClntWidth, gMaxClntHeight));

                // Have fun with operations.
                myEnableOperationMenus();
                DrawMenuBar(hwnd);

                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
            }
            // Always clear this to make sure the filename input line
            // keeps empty when the user re-open the open file dialog.
            tmpFileName[0] = L'\0';
            tmpTitleName[0] = L'\0';
            return 0;
        }
        /*
        * Second window is used to maintain another image while there's already an image in main window,
        * which can be used to process interaction operations, like image-add, image-multiply etc ......
        */
        case IDM_FILE_SECOND:
        {
            // Focus window if existed.
            if (IsWindow(gSecondWnd))
            {
                ShowWindow(gSecondWnd, SW_NORMAL);
                SetFocus(gSecondWnd);
                return 0;
            }
            // Check whether the image is grayscale.
            if (!myIsImageGrayScale(gImage, &gBmpInfo))
            {
                MessageBox(hwnd, L"只允许在第二窗口中使用灰度图像.", L"提示", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            RECT rc = myGetSecondWndInitSize();
            // Create window with quried size.
            gSecondWnd = CreateWindow(
                gSecondWndName,
                L"第二窗口",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                left,
                top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                gMainWnd,
                NULL,
                gInstance,
                NULL);
            return 0;
        }
        case IDM_FILE_SAVE:
        {
            if (gHasExternalFile)
            {
                myWriteBmp(gFileName, gImage, &gBmpInfo);
                SetWindowText(hwnd, gFileName);
                return 0;
            }
            // fall through
        }
        case IDM_FILE_SAVE_AS:
        {
            if (mySaveFileDialog(hwnd, gFileName, gTitleName))
            {
                myWriteBmp(gFileName, gImage, &gBmpInfo);
                SetWindowText(hwnd, gFileName);
                // Now the image has been connected with an external file.
                gHasExternalFile = TRUE;
            }
            return 0;
        }
        case IDM_FILE_EXP_TXT:
        {
            if (gImage != NULL) // Only export when there's an image.
            {
                static WCHAR szTxtName[MAX_PATH], szTxtBodyName[MAX_PATH];

                OPENFILENAME* ofn = &gOpenFileName;
                // Change from *.bmp to *.txt.
                ofn->lpstrFilter = L"文本文件 (*.txt)\0*.txt\0";
                ofn->lpstrDefExt = L"txt";

                if (mySaveFileDialog(hwnd, szTxtName, szTxtBodyName))
                    myWriteTxt(szTxtName, gImage, &gBmpInfo);

                // Restore origin dialog info.
                myInitFileDialogInfo(hwnd);
            }
            else MessageBox(hwnd, L"工作区为空, 无法导出文本文件.", L"警告", MB_OK | MB_ICONEXCLAMATION);
            return 0;
        }
        case IDM_GRAY_EMPI:
        case IDM_GRAY_EVEN:
        case IDM_GRAY_GAMMA:
        case IDM_GRAY_R:
        case IDM_GRAY_G:
        case IDM_GRAY_B:
        {
            RECT rc = myGetGrayTransWndInitSize();
            // Create window with quried size.
            gGrayTransWnd = CreateWindow(
                gGrayTransWndName,
                NULL,
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                left,
                top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                gMainWnd,
                NULL,
                gInstance,
                // Pass gray-type to child window to present different results.
                (LPVOID)LOWORD(wParam));
            return 0;
        }
        case IDM_GRAY_2xBLACK:
        {
            if (gImage != NULL)
            {
                MyBGRA* tmpImg = myExpand2xBlackImage(gImage, &gBmpInfo);
                myAssert(tmpImg);
                free(gImage);
                gImage = tmpImg;
                goto after_2x_resized;
            }
            return 0;
        }
        case IDM_GRAY_2xMIRROR:
        {
            if (gImage != NULL)
            {
                MyBGRA* tmpImg = myExpand2xMirrorImage(gImage, &gBmpInfo);
                myAssert(tmpImg);
                free(gImage);
                gImage = tmpImg;
                goto after_2x_resized;
            }
            return 0;
        }
        case IDM_GRAY_2xCOPY:
        {
            if (gImage != NULL)
            {
                MyBGRA* tmpImg = myExpand2xCopyImage(gImage, &gBmpInfo);
                myAssert(tmpImg);
                free(gImage);
                gImage = tmpImg;
                goto after_2x_resized;
            }
            return 0;
        }
        after_2x_resized:
        {
            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            gBmpInfo.nWidth *= 2; gBmpInfo.nHeight *= 2;

            // Resize main window to suit expanded image.
            if (!IsMaximized(hwnd))
            {
                RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 41 + 2 * gCharHeight };
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                LONG tmpW = rc.right - rc.left, tmpH = rc.bottom - rc.top;
                GetWindowRect(hwnd, &rc);
                MoveWindow(hwnd, rc.left, rc.top, tmpW, tmpH, TRUE);
            }
            else SendMessage(hwnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(gMaxClntWidth, gMaxClntHeight));

            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);

            return 0;
        }
        case IDM_GRAY_THRESHOLD:
        {
            RECT rc = myGetThresholdWndInitSize();
            // Create window with quried size.
            gThresholdWnd = CreateWindow(
                gThresholdWndName,
                L"阈值处理",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                left,
                top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                gMainWnd,
                NULL,
                gInstance,
                NULL);
            return 0; 
        }
        // space-domain
        case IDM_SPAF_BOX:
        case IDM_SPAF_GAUS:
        case IDM_SPAF_MEDI:
        case IDM_SPAF_LAPLACE:
        case IDM_SPAF_SOBEL:
        // freq-domain
        case IDM_SPEF_POWER:
        case IDM_SPEF_PHASE:
        case IDM_SPEF_POWER_FFT:
        case IDM_SPEF_PHASE_FFT:
        // transfer-func
        case IDM_TFUNC_ILPF:
        case IDM_TFUNC_GLPF:
        case IDM_TFUNC_BLPF:
        case IDM_TFUNC_IHPF:
        case IDM_TFUNC_GHPF:
        case IDM_TFUNC_BHPF:
        case IDM_TFUNC_HOMO:
        // noise-model
        case IDM_NMODEL_GAUSSIAN:
        case IDM_NMODEL_RAYLEIGH:
        case IDM_NMODEL_ERLANG:
        case IDM_NMODEL_EXP:
        case IDM_NMODEL_UNIFORM:
        case IDM_NMODEL_SANDP:
        {
            // TODO: support Erlang noise mode.
            if (LOWORD(wParam) == IDM_NMODEL_ERLANG)
            {
                MessageBox(hwnd, L"抱歉, 我们暂时不支持爱尔兰噪声模型.", L"提示", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            if (LOWORD(wParam) >= IDM_TFUNC_ILPF && LOWORD(wParam) < IDM_NMODEL_GAUSSIAN && gSpectral == NULL)
            {
                MessageBox(hwnd, L"频谱为空, 无法调用传递函数.", L"提示", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            RECT rc = myGetSpafGeneWndInitSize();
            // Create window with quried size.
            gDomainFilterWnd = CreateWindow(
                gDomainFilterWndName,
                NULL,
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                left,
                top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                gMainWnd,
                NULL,
                gInstance,
                // Pass core/tfunc type to child window to create different widgets.
                (LPVOID)LOWORD(wParam));
            return 0;
        }
        // space-domain
        case IDM_SPAF_CUST:
        // freq-domain
        case IDM_SPEF_CUST:
        {
            MessageBox(hwnd, L"抱歉, 我们暂时不支持自定义卷积核!", L"提示", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        case IDM_EASTER_EGG:
        {
            static UINT nUserSelectCnt = 0;
            static WCHAR* szEasterEggs[] =
            {
                L"你点我干啥?",
                L"锘挎槬鐪犱笉瑙夋檽锛",
                L"有本事你再点我试试?",
                L"你再点也没有用了(⊙ˍ⊙)",
                L"好吧, 这下是真的没有了."
            };

            // Only trigger easter egg when there's an image.
            if (gImage == NULL) return 0;

            ++nUserSelectCnt;
            if (nUserSelectCnt <= 3 || nUserSelectCnt == 6)
            {
                MENUITEMINFO mii;
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_TYPE;
                mii.fType = MFT_STRING;
                mii.dwTypeData = szEasterEggs[min(4, nUserSelectCnt)];
                // Change menu item text when selected.
                SetMenuItemInfo(gMenu, IDM_EASTER_EGG, FALSE, &mii);
            }

            // Trigger easter egg.
            if (nUserSelectCnt == 3)
            {
                SetWindowText(hwnd, L"可恶, 彩蛋被你找到了!");

                UINT xExpandSize = gBmpInfo.nWidth / 2;
                UINT yExpandSize = gBmpInfo.nHeight / 2;
                MyBGRA* tmpImage = myExpandMirrorImage(gImage, &gBmpInfo, xExpandSize, yExpandSize);
                myAssert(tmpImage);

                // Destroy the precious image of evil user!
                free(gImage);
                gImage = tmpImage;
                gBmpInfo.nWidth += 2 * xExpandSize;
                gBmpInfo.nHeight += 2 * yExpandSize;

                goto resize_and_repaint_main_window;
            }

            // Trigger final easter egg.
            if (nUserSelectCnt == 6)
            {
                SetWindowText(hwnd, L"邪恶的用户!");

                MyBGRA* tmpImage = (MyBGRA*)malloc(512 * 512 * sizeof(MyBGRA));
                myAssert(tmpImage);

                srand((unsigned int)time(NULL));
                // Populate with random colors.
                for (UINT i = 0; i < 512; ++i)
                    for (UINT j = 0; j < 512; ++j)
                    {
                        UINT idx = i + j * 512;
                        tmpImage[idx].R = rand() % 256;
                        tmpImage[idx].G = rand() % 256;
                        tmpImage[idx].B = rand() % 256;
                        tmpImage[idx].A = rand() % 256;
                    }

                // Destroy the precious image of evil user!
                free(gImage);
                gImage = tmpImage;
                gBmpInfo.nWidth = 512;
                gBmpInfo.nHeight = 512;

                goto resize_and_repaint_main_window;
            }

            // Increment the count for subsequent select.
            if (nUserSelectCnt > 6)
            {
                WCHAR szTmp[30];
                wsprintf(szTmp, L"你已经进行了 %d 次毁灭性的点击.", nUserSelectCnt);
                SetWindowText(hwnd, szTmp);
            }
            return 0;

            resize_and_repaint_main_window:
            {
                // Update main menu.
                myEnableOperationMenus();
                DrawMenuBar(hwnd);

                // Resize main window to suit generated image.
                if (!IsMaximized(hwnd))
                {
                    RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 41 + 2 * gCharHeight };
                    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                    UINT tmpClntW = rc.right - rc.left, tmpClntH = rc.bottom - rc.top;
                    GetWindowRect(hwnd, &rc);
                    MoveWindow(hwnd, rc.left, rc.top, tmpClntW, tmpClntH, TRUE);
                }
                else SendMessage(hwnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(gMaxClntWidth, gMaxClntHeight));

                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        case IDM_APP_ABOUT:
        {
            // Display popup window in the center of main window.
            gAppAboutWnd = CreateWindow(
                gAppAboutWndName,
                NULL,
                WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                left + (width - 256) / 2,
                top + (height - 256) / 2,
                256,
                256,
                gMainWnd,
                NULL,
                gInstance,
                NULL);
            return 0;
        }
        case IDM_WEB_HOME:
        {
#define MY_WEB_HOME L"https://github.com/yiyaowen/BitmapViewer/wiki/使用手册"
            // Open web explorer to project home page.
            ShellExecute(hwnd, NULL, L"microsoft-edge:" MY_WEB_HOME, NULL, NULL, SW_SHOW);
#undef MY_WEB_HOME
            return 0;
        }
        default:
            return 0;
        }
    }
    case WM_DESTROY:
        // Note we have create these custom pens & brushes.
        DeleteObject(slctDashPen);
        DeleteObject(brColorBlock);
        // Note we have allocated memory for these buffers.
        if (gSpectral != NULL) free(gSpectral);
        if (gImage != NULL) free(gImage);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                            Second Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// Second Window - 100
// Its child window should start from 100_01.
#define SUBW_MAIN_TO_SUB_BTN 10001
#define SUBW_SUB_TO_MAIN_BTN 10002
#define SUBW_IMPORT_IMAGE_BTN 10003
// invert gray scale
#define SUBW_INVERT_PIXEL_BTN 10004
// main + sub
#define SUBW_MAIN_ADD_SUB_BTN 10005
// main - sub / sub - main
#define SUBW_MAIN_SUB_SUB_BTN 10006
#define SUBW_SUB_SUB_MAIN_BTN 10007
// main * sub
#define SUBW_MAIN_MUL_SUB_BTN 10008
// main ÷ sub / sub ÷ main
#define SUBW_MAIN_DIV_SUB_BTN 10009
#define SUBW_SUB_DIV_MAIN_BTN 10010
// logarithm transform
#define SUBW_LOG_TRANS_PARAM_LBL 10011
#define SUBW_LOG_TRANS_PARAM_EDT 10012
#define SUBW_LOG_TRANS_EXECU_BTN 10013
// exponential transform
#define SUBW_EXP_TRANS_GAMMA_LBL 10014
#define SUBW_EXP_TRANS_GAMMA_EDT 10015
#define SUBW_EXP_TRANS_EXECU_BTN 10016
// spectrum reconstruction
#define SUBW_SPE_RECON_LBL 10017
#define SUBW_SPE_RECON_MAIN_POWER_SUB_PHASE_BTN 10018
#define SUBW_SPE_RECON_MAIN_PHASE_SUB_POWER_BTN 10019
#define SUBW_SPE_RECON_MAIN_POWER_SUB_PHASE_FFT_BTN 10020
#define SUBW_SPE_RECON_MAIN_PHASE_SUB_POWER_FFT_BTN 10021

LRESULT CALLBACK SecondWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;
    static MyBmpInfo bmpInfo;
    static MyComplex* spectral = NULL;

    // Bottom panel buttons.
    static HWND btnMainToSub;
    static HWND btnSubToMain;
    static HWND btnImportImage;

    // Right panel buttons.
    static HWND btnInvertPixel;

    static HWND btnMainAddSub;

    static HWND btnMainSubSub;
    static HWND btnSubSubMain;

    static HWND btnMainMulSub;

    static HWND btnMainDivSub;
    static HWND btnSubDivMain;

    // log, exp widgets.
    static HWND lblLogParam;
    static HWND edtLogParam;
    static double dLogParam = 1;
    static HWND btnLogExecu;

    static HWND lblExpGamma;
    static HWND edtExpGamma;
    static double dExpGamma = 2;
    static HWND btnExpExecu;

    // spectrum reconstruction
    static HWND lblSpeRecon;
    static HWND btnSpeReconMainPowerSubPhase;
    static HWND btnSpeReconMainPhaseSubPower;
    static HWND btnSpeReconMainPowerSubPhaseFFT;
    static HWND btnSpeReconMainPhaseSubPowerFFT;

    static LONG yMostBottom;
    
    switch (message)
    {
    case WM_CREATE:
        // Create empty 256x256 image if main window is empty.
        if (gImage == NULL)
        {
            image = (MyBGRA*)malloc(65536 * sizeof(MyBGRA));
            myAssert(image);
            bmpInfo.nWidth = bmpInfo.nHeight = 256;
            ZeroMemory(image, 65536 * sizeof(MyBGRA));
        }
        // Copy image data from main window.
        else
        {
            size_t nPixelCount = gBmpInfo.nWidth * gBmpInfo.nHeight;
            image = (MyBGRA*)malloc(nPixelCount * sizeof(MyBGRA));
            myAssert(image);
            bmpInfo = gBmpInfo;
            memcpy(image, gImage, nPixelCount * sizeof(MyBGRA));

            // Copy spectral data by the way (if has).
            if (gSpectral != NULL)
            {
                spectral = (MyComplex*)malloc(nPixelCount * sizeof(MyComplex));
                myAssert(spectral);
                memcpy(spectral, gSpectral, nPixelCount * sizeof(MyComplex));
            }
        }

        // Create button widgets.
        RECT rc;
        GetClientRect(hwnd, &rc);
        int clntWidth = rc.right - rc.left;
        int clntHeight = rc.bottom - rc.top;

        int tmpBtnW = 10 * gCharWidth,
            tmpBtnH = 2 * gCharHeight;
        int tmpVert = clntHeight - tmpBtnH - 6;
        // Create main-to-sub button.
        btnMainToSub = CreateWindow(
            L"button",
            L"主-->副",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            24 + 16 * gCharWidth,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_MAIN_TO_SUB_BTN,
            gInstance,
            NULL);

        int tmpLastHorz = 24 + 16 * gCharWidth + tmpBtnW + 5;
        // Create sub-to-main button.
        btnSubToMain = CreateWindow(
            L"button",
            L"副-->主",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            tmpLastHorz,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_SUB_TO_MAIN_BTN,
            gInstance,
            NULL);

        tmpLastHorz += (tmpBtnW + 5);
        // Create import-image button.
        btnImportImage = CreateWindow(
            L"button",
            L"导入图像",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            tmpLastHorz,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_IMPORT_IMAGE_BTN,
            gInstance,
            NULL);

        // Create right panel buttons.
#define MY_CREATE_SUBW_BTN(Btn_Handle, Btn_Text, Btn_Idx, Btn_ID) \
        Btn_Handle = CreateWindow( \
            L"button", \
            Btn_Text, \
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, \
            12, \
            5 + Btn_Idx * (tmpBtnH + 10), \
            16 * gCharWidth, \
            tmpBtnH, \
            hwnd, \
            (HMENU)Btn_ID, \
            gInstance, \
            NULL)

        MY_CREATE_SUBW_BTN(btnInvertPixel, L"反转灰度", 0, SUBW_INVERT_PIXEL_BTN);

        MY_CREATE_SUBW_BTN(btnMainAddSub, L"主 + 副", 1, SUBW_MAIN_ADD_SUB_BTN);

        MY_CREATE_SUBW_BTN(btnMainSubSub, L"主 - 副", 2, SUBW_MAIN_SUB_SUB_BTN);
        MY_CREATE_SUBW_BTN(btnSubSubMain, L"副 - 主", 3, SUBW_SUB_SUB_MAIN_BTN);

        MY_CREATE_SUBW_BTN(btnMainMulSub, L"主 x 副", 4, SUBW_MAIN_MUL_SUB_BTN);

        MY_CREATE_SUBW_BTN(btnMainDivSub, L"主 ÷ 副", 5, SUBW_MAIN_DIV_SUB_BTN);
        MY_CREATE_SUBW_BTN(btnSubDivMain, L"副 ÷ 主", 6, SUBW_SUB_DIV_MAIN_BTN);

#undef MY_CREATE_SUBW_BTN

        // Create logarithm transform text label, edit line & button.
        lblLogParam = CreateWindow(
            L"static",
            L"clog(1+r), c",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            5,
            10 * gCharWidth,
            gCharHeight,
            hwnd,
            (HMENU)SUBW_LOG_TRANS_PARAM_LBL,
            gInstance,
            NULL);
        edtLogParam = CreateWindow(
            L"edit",
            L"1",
            WS_CHILD | WS_VISIBLE,
            36 + bmpInfo.nWidth + 28 * gCharWidth,
            5,
            4 * gCharWidth,
            gCharHeight,
            hwnd,
            (HMENU)SUBW_LOG_TRANS_PARAM_EDT,
            gInstance,
            NULL);
        btnLogExecu = CreateWindow(
            L"button",
            L"对数变换",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            10 + gCharHeight,
            16 * gCharWidth,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_LOG_TRANS_EXECU_BTN,
            gInstance,
            NULL);

        // Create exponential transform text label, edit line & button.
        lblExpGamma = CreateWindow(
            L"static",
            L"gamma",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            20 + 3 * gCharHeight,
            10 * gCharWidth,
            gCharHeight,
            hwnd,
            (HMENU)SUBW_EXP_TRANS_GAMMA_LBL,
            gInstance,
            NULL);
        edtExpGamma = CreateWindow(
            L"edit",
            L"2",
            WS_CHILD | WS_VISIBLE,
            36 + bmpInfo.nWidth + 28 * gCharWidth,
            20 + 3 * gCharHeight,
            4 * gCharWidth,
            gCharHeight,
            hwnd,
            (HMENU)SUBW_EXP_TRANS_GAMMA_EDT,
            gInstance,
            NULL);
        btnExpExecu = CreateWindow(
            L"button",
            L"伽马变换",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            25 + 4 * gCharHeight,
            16 * gCharWidth,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_EXP_TRANS_EXECU_BTN,
            gInstance,
            NULL);

        // Create spectrum reconstruction text label & buttons.
        lblSpeRecon = CreateWindow(
            L"static",
            L"从频谱重建图像",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            35 + 6 * gCharHeight,
            16 * gCharWidth,
            gCharHeight + 4,
            hwnd,
            (HMENU)SUBW_SPE_RECON_LBL,
            gInstance,
            NULL);
        btnSpeReconMainPowerSubPhase = CreateWindow(
            L"button",
            L"主功率 副相位",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            44 + 7 * gCharHeight,
            16 * gCharWidth,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_SPE_RECON_MAIN_POWER_SUB_PHASE_BTN,
            gInstance,
            NULL);
        btnSpeReconMainPhaseSubPower = CreateWindow(
            L"button",
            L"主相位 副功率",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            49 + 9 * gCharHeight,
            16 * gCharWidth,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_SPE_RECON_MAIN_PHASE_SUB_POWER_BTN,
            gInstance,
            NULL);
        btnSpeReconMainPowerSubPhaseFFT = CreateWindow(
            L"button",
            L"主功率 副相位 FFT",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            54 + 11 * gCharHeight,
            16 * gCharWidth,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_SPE_RECON_MAIN_POWER_SUB_PHASE_FFT_BTN,
            gInstance,
            NULL);
        btnSpeReconMainPhaseSubPowerFFT = CreateWindow(
            L"button",
            L"主相位 副功率 FFT",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            36 + bmpInfo.nWidth + 16 * gCharWidth,
            59 + 13 * gCharHeight,
            16 * gCharWidth,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_SPE_RECON_MAIN_PHASE_SUB_POWER_FFT_BTN,
            gInstance,
            NULL);

        yMostBottom = max(70 + 14 * gCharHeight, // left panel
                          64 + 15 * gCharHeight); // right panel

        // Suit window to display all widgets.
        RECT tmpRC = { 0, 0,
            ((gImage == NULL) ? 256 : gBmpInfo.nWidth) + 48 + 32 * gCharWidth,
            ((gImage == NULL) ? 256 : gBmpInfo.nHeight) + 2 * gCharHeight + 12 };
        // Compare origin height with suited height.
        tmpRC.bottom = max(yMostBottom, tmpRC.bottom);
        AdjustWindowRect(&tmpRC, WS_OVERLAPPEDWINDOW, FALSE);
        UINT suitWidth = tmpRC.right - tmpRC.left, suitHeight = tmpRC.bottom - tmpRC.top;
        GetWindowRect(hwnd, &tmpRC);
        MoveWindow(hwnd, tmpRC.left, tmpRC.top, suitWidth, suitHeight, TRUE);
        return 0;

    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int clntWidth = rc.right - rc.left;
        int clntHeight = rc.bottom - rc.top;

        // Keep those buttons attached to window bottom.
#define MY_STICK_BOTTOM_BTN(Btn_Handle, Btn_Offset_X) \
        GetWindowRect(Btn_Handle, &rc); \
        MoveWindow(Btn_Handle, Btn_Offset_X, \
                   clntHeight - 6 - 2 * gCharHeight, \
                   rc.right - rc.left, rc.bottom - rc.top, TRUE)

        MY_STICK_BOTTOM_BTN(btnMainToSub, 24 + 16 * gCharWidth);
        MY_STICK_BOTTOM_BTN(btnSubToMain, 29 + 26 * gCharWidth);
        MY_STICK_BOTTOM_BTN(btnImportImage, 34 + 36 * gCharWidth);

#undef MY_STICK_BOTTOM_BTN

        // Keep those widgets attached to window right.
#define MY_STICK_RIGHT_LBL(Lbl_Handle, Lbl_Offset_Y) \
        GetWindowRect(Lbl_Handle, &rc); \
        MoveWindow(Lbl_Handle, \
                   clntWidth - 12 - 16 * gCharWidth, \
                   Lbl_Offset_Y, \
                   rc.right - rc.left, rc.bottom - rc.top, TRUE)

#define MY_STICK_RIGHT_BTN MY_STICK_RIGHT_LBL

#define MY_STICK_RIGHT_EDT(Edt_Handle, Edt_Offset_Y) \
        GetWindowRect(Edt_Handle, &rc); \
        MoveWindow(Edt_Handle, \
                   clntWidth - 12 - 4 * gCharWidth, \
                   Edt_Offset_Y, \
                   rc.right - rc.left, rc.bottom - rc.top, TRUE)

        MY_STICK_RIGHT_LBL(lblLogParam, 5);
        MY_STICK_RIGHT_EDT(edtLogParam, 5);
        MY_STICK_RIGHT_BTN(btnLogExecu, 10 + gCharHeight);

        MY_STICK_RIGHT_LBL(lblExpGamma, 20 + 3 * gCharHeight);
        MY_STICK_RIGHT_EDT(edtExpGamma, 20 + 3 * gCharHeight);
        MY_STICK_RIGHT_BTN(btnExpExecu, 25 + 4 * gCharHeight);

        MY_STICK_RIGHT_LBL(lblSpeRecon, 35 + 6 * gCharHeight);
        MY_STICK_RIGHT_BTN(btnSpeReconMainPowerSubPhase, 44 + 7 * gCharHeight);
        MY_STICK_RIGHT_BTN(btnSpeReconMainPhaseSubPower, 49 + 9 * gCharHeight);
        MY_STICK_RIGHT_BTN(btnSpeReconMainPowerSubPhaseFFT, 54 + 11 * gCharHeight);
        MY_STICK_RIGHT_BTN(btnSpeReconMainPhaseSubPowerFFT, 59 + 13 * gCharHeight);

#undef MY_STICK_RIGHT_EDT
#undef MY_STICK_RIGHT_BTN
#undef MY_STICK_RIGHT_LBL

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        myDisplayImage(hdc, 24 + 16 * gCharWidth, 0, bmpInfo.nWidth, bmpInfo.nHeight, 0, 0, image, &bmpInfo);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case SUBW_MAIN_TO_SUB_BTN: // main --> sub
        {
            // Abort copy when there's no image in main window.
            if (gImage == NULL)
            {
                MessageBox(hwnd, L"主窗口为空, 无法复制图像.", L"提示", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            else if (!myIsImageGrayScale(gImage, &gBmpInfo))
            {
                MessageBox(hwnd, L"只允许在第二窗口中使用灰度图像.", L"提示", MB_OK | MB_ICONINFORMATION);
                return 0;
            }

            // Sub-image must not be empty at this point.
            free(image);
            size_t nPixelCount = gBmpInfo.nWidth * gBmpInfo.nHeight;
            image = (MyBGRA*)malloc(nPixelCount * sizeof(MyBGRA));
            myAssert(image);
            bmpInfo = gBmpInfo;
            memcpy(image, gImage, nPixelCount * sizeof(MyBGRA));

            // Copy spectral data by the way (if has).
            if (gSpectral != NULL)
            {
                if (spectral != NULL) free(spectral);
                spectral = (MyComplex*)malloc(nPixelCount * sizeof(MyComplex));
                myAssert(spectral);
                memcpy(spectral, gSpectral, nPixelCount * sizeof(MyComplex));
            }

            // Resize to suit copyed image.
            RECT tmpRC = { 0, 0,
                bmpInfo.nWidth + 48 + 32 * gCharWidth,
                bmpInfo.nHeight + 2 * gCharHeight + 12 };
            // Compare origin height with suited height.
            tmpRC.bottom = max(yMostBottom, tmpRC.bottom);
            AdjustWindowRect(&tmpRC, WS_OVERLAPPEDWINDOW, FALSE);
            UINT suitWidth = tmpRC.right - tmpRC.left, suitHeight = tmpRC.bottom - tmpRC.top;
            GetWindowRect(hwnd, &tmpRC);
            MoveWindow(hwnd, tmpRC.left, tmpRC.top, suitWidth, suitHeight, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_SUB_TO_MAIN_BTN: //  sub --> main
        {
            // Only change main window image when no child window opened.
            if (myValidWndCount() != 1)
            {
                MessageBox(hwnd, L"无法更改主窗口图片, 因为还有子窗口未关闭.", L"提示", MB_OK | MB_ICONINFORMATION);
                return 0;
            }

            // Main-image might be empty at this point.
            if (gImage != NULL) free(gImage);
            size_t nPixelCount = bmpInfo.nWidth * bmpInfo.nHeight;
            gImage = (MyBGRA*)malloc(nPixelCount * sizeof(MyBGRA));
            myAssert(gImage);
            gBmpInfo = bmpInfo;
            memcpy(gImage, image, nPixelCount * sizeof(MyBGRA));

            if (gSpectral != NULL)
            {
                free(gSpectral);
                gSpectral = NULL;
            }
            // Copy spectral data by the way (if has).
            if (spectral != NULL)
            {
                gSpectral = (MyComplex*)malloc(nPixelCount * sizeof(MyComplex));
                myAssert(gSpectral);
                memcpy(gSpectral, spectral, nPixelCount * sizeof(MyComplex));
            }

            // Resize main window to suit copyed image.
            if (!IsMaximized(gMainWnd))
            {
                RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 41 + 2 * gCharHeight };
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                int tmpW = rc.right - rc.left;
                int tmpH = rc.bottom - rc.top;
                GetWindowRect(gMainWnd, &rc);
                MoveWindow(gMainWnd, rc.left, rc.top, tmpW, tmpH, TRUE);
            }
            else SendMessage(gMainWnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(gMaxClntWidth, gMaxClntHeight));

            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            // Update operation menu state.
            myEnableOperationMenus();
            DrawMenuBar(gMainWnd);

            // Repaint main window image.
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(gMainWnd, NULL, FALSE);
            return 0;
        }
        case SUBW_IMPORT_IMAGE_BTN: // import image
        {
            static MyBmpInfo tmpInfo;
            static WCHAR tmpFileName[MAX_PATH], tmpTitleName[MAX_PATH];

            // Show Open-File dialog.
            if (myOpenFileDialog(hwnd, tmpFileName, tmpTitleName))
            {
                // Check whether is grayscale.
                MyBGRA* tmpImage = myReadBmp(tmpFileName, &tmpInfo);
                if (tmpImage == NULL)
                {
                    MessageBox(hwnd, L"无法导入该 BMP 图片, 可能文件已损坏.", L"警告", MB_OK | MB_ICONEXCLAMATION);
                    return 0;
                }
                if (!myIsImageGrayScale(tmpImage, &tmpInfo))
                {
                    MessageBox(gMainWnd, L"只允许在第二窗口中使用灰度图像.", L"提示", MB_OK | MB_ICONINFORMATION);
                    // Note we have allocated memory for temporary image data.
                    free(tmpImage);
                    return 0;
                }

                // Replace second window background image with selected image.
                if (image != NULL) free(image);
                image = tmpImage;
                bmpInfo = tmpInfo;
                // Release spectral data (if has).
                if (spectral != NULL) { free(spectral); spectral = NULL; }

                // Resize to suit selected image.
                RECT tmpRC = { 0, 0,
                    bmpInfo.nWidth + 48 + 32 * gCharWidth,
                    bmpInfo.nHeight + 2 * gCharHeight + 12 };
                // Compare origin height with suited height.
                tmpRC.bottom = max(yMostBottom, tmpRC.bottom);
                AdjustWindowRect(&tmpRC, WS_OVERLAPPEDWINDOW, FALSE);
                UINT suitWidth = tmpRC.right - tmpRC.left, suitHeight = tmpRC.bottom - tmpRC.top;
                GetWindowRect(hwnd, &tmpRC);
                MoveWindow(hwnd, tmpRC.left, tmpRC.top, suitWidth, suitHeight, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Always clear this to make sure the filename input line
            // keeps empty when the user re-open the open file dialog.
            tmpFileName[0] = L'\0';
            tmpTitleName[0] = L'\0';
            return 0;
        }
        case SUBW_INVERT_PIXEL_BTN: // Invert gray scale.
        {
            UINT i, j, idx;
            // Process origin pixels.
            for (i = 0; i < bmpInfo.nWidth; ++i)
                for (j = 0; j < bmpInfo.nHeight; ++j)
                {
                    idx = i + j * bmpInfo.nWidth;
                    image[idx].R = image[idx].G = image[idx].B = (255 - image[idx].R);
                }
            // Process related spectral.
            if (spectral != NULL)
            {
                long double tmpMin = LDBL_MAX, tmpMax = LDBL_MIN;
                // Find min, max values.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        tmpMin = min(tmpMin, spectral[idx].real);
                        tmpMax = max(tmpMax, spectral[idx].real);
                    }
                long double tmpVal = tmpMin + tmpMax;
                // Invert real part.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real = tmpVal - spectral[idx].real;
                    }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }

#define MY_CHECK_MAIN_SUB_IMG \
    if (gImage == NULL) goto main_window_empty; \
    if (gBmpInfo.nWidth != bmpInfo.nWidth || \
        gBmpInfo.nHeight != bmpInfo.nHeight) \
        goto main_sub_size_not_matched; \
    /* Allocate a temporary buffer to store intermidiate grayscale. */ \
    double* tmpImage = (double*)malloc( \
        bmpInfo.nWidth * bmpInfo.nHeight * sizeof(double)); \
    myAssert(tmpImage)

// Tmp_Max might be changed in this macro!
#define MY_NORMALIZE_SUB_IMG(Tmp_Min, Tmp_Max) \
    Tmp_Max -= Tmp_Min; \
    /* Normalize to 0 ~255. */ \
    for (i = 0; i < bmpInfo.nWidth; ++i) \
        for (j = 0; j < bmpInfo.nHeight; ++j) \
        { \
            idx = i + j * bmpInfo.nWidth; \
            tmpImage[idx] = 255 * ((tmpImage[idx] - Tmp_Min) / Tmp_Max); \
            image[idx].R = image[idx].G = image[idx].B = (UINT8)min(255, max(0, tmpImage[idx])); \
        } \
    /* Note we have allocated memory for temporary image data. */ \
    free(tmpImage); \
    /* Release origin related spectral data (if has). */ \
    if (spectral != NULL) { free(spectral); spectral = NULL; }

        case SUBW_MAIN_ADD_SUB_BTN: // main + sub
        {
            UINT i, j, idx;
            if (spectral == NULL || gSpectral == NULL) // Process origin pixels.
            {
                MY_CHECK_MAIN_SUB_IMG;

                double tmpMin = 255, tmpMax = 0;
                // Add, find min, max for later normalization.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        tmpImage[idx] = (double)image[idx].R + (double)gImage[idx].R;
                        // Update min, max values.
                        tmpMin = min(tmpImage[idx], tmpMin);
                        tmpMax = max(tmpImage[idx], tmpMax);
                    }

                MY_NORMALIZE_SUB_IMG(tmpMin, tmpMax);
            }
            else // Process related spectral.
            {
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real += gSpectral[idx].real;
                    }
                mycpLogarithmTrans(image, spectral, bmpInfo.nWidth, bmpInfo.nHeight, dLogParam);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_MAIN_SUB_SUB_BTN: // main - sub
        {
            UINT i, j, idx;
            if (spectral == NULL || gSpectral == NULL) // Process origin pixels.
            {
                MY_CHECK_MAIN_SUB_IMG;

                double tmpMin = 255, tmpMax = 0;
                // Subtract, find min, max for later normalization.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        tmpImage[idx] = (double)gImage[idx].R - (double)image[idx].R;
                        // Update min, max values.
                        tmpMin = min(tmpImage[idx], tmpMin);
                        tmpMax = max(tmpImage[idx], tmpMax);
                    } 

                MY_NORMALIZE_SUB_IMG(tmpMin, tmpMax);
            }
            else // Process related spectral.
            {
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real = gSpectral[idx].real - spectral[idx].real;
                    }
                mycpLogarithmTrans(image, spectral, bmpInfo.nWidth, bmpInfo.nHeight, dLogParam);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_SUB_SUB_MAIN_BTN: // sub - main
        {
            UINT i, j, idx;
            if (spectral == NULL || gSpectral == NULL) // Process origin pixels.
            {
                MY_CHECK_MAIN_SUB_IMG;

                double tmpMin = 255, tmpMax = 0;
                // Subtract, find min, max for later normalization.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        tmpImage[idx] = (double)image[idx].R - (double)gImage[idx].R;
                        // Update min, max values.
                        tmpMin = min(tmpImage[idx], tmpMin);
                        tmpMax = max(tmpImage[idx], tmpMax);
                    }

                MY_NORMALIZE_SUB_IMG(tmpMin, tmpMax);
            }
            else // Process related spectral.
            {
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real -= gSpectral[idx].real;
                    }
                mycpLogarithmTrans(image, spectral, bmpInfo.nWidth, bmpInfo.nHeight, dLogParam);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_MAIN_MUL_SUB_BTN: // main * sub
        {
            UINT i, j, idx;
            if (spectral == NULL || gSpectral == NULL) // Process origin pixels.
            {
                MY_CHECK_MAIN_SUB_IMG;

                double tmpMin = 255, tmpMax = 0;
                // Multiply, find min, max for later normalization.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        tmpImage[idx] = (double)image[idx].R * (double)gImage[idx].R;
                        // Update min, max values.
                        tmpMin = min(tmpImage[idx], tmpMin);
                        tmpMax = max(tmpImage[idx], tmpMax);
                    }

                MY_NORMALIZE_SUB_IMG(tmpMin, tmpMax);
            }
            else // Process related spectral.
            {
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real *= gSpectral[idx].real;
                    }
                mycpLogarithmTrans(image, spectral, bmpInfo.nWidth, bmpInfo.nHeight, dLogParam);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_MAIN_DIV_SUB_BTN: // main / sub
        {
            UINT i, j, idx;
            if (spectral == NULL || gSpectral == NULL) // Process origin pixels.
            {
                MY_CHECK_MAIN_SUB_IMG;

                double tmpMin = 255, tmpMax = 0;
                // Divide, find min, max for later normalization.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        // We simply truncate here. Be careful not to divide by zero!
                        tmpImage[idx] = (double)gImage[idx].R / (double)max(1e-3, image[idx].R);
                        // Update min, max values.
                        tmpMin = min(tmpImage[idx], tmpMin);
                        tmpMax = max(tmpImage[idx], tmpMax);
                    }

                MY_NORMALIZE_SUB_IMG(tmpMin, tmpMax);
            }
            else // Process related spectral.
            {
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real = gSpectral[idx].real / spectral[idx].real;
                    }
                mycpLogarithmTrans(image, spectral, bmpInfo.nWidth, bmpInfo.nHeight, dLogParam);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_SUB_DIV_MAIN_BTN: // sub / main
        {
            UINT i, j, idx;
            if (spectral == NULL || gSpectral == NULL) // Process origin pixels.
            {
                MY_CHECK_MAIN_SUB_IMG;

                double tmpMin = 255, tmpMax = 0;
                // Divide, find min, max for later normalization.
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        // We simply truncate here. Be careful not to divide by zero!
                        tmpImage[idx] = (double)image[idx].R / (double)max(1e-3, image[idx].R);
                        // Update min, max values.
                        tmpMin = min(tmpImage[idx], tmpMin);
                        tmpMax = max(tmpImage[idx], tmpMax);
                    }

                MY_NORMALIZE_SUB_IMG(tmpMin, tmpMax);
            }
            else // Process related spectral.
            {
                for (i = 0; i < bmpInfo.nWidth; ++i)
                    for (j = 0; j < bmpInfo.nHeight; ++j)
                    {
                        idx = i + j * bmpInfo.nWidth;
                        spectral[idx].real /= spectral[idx].real;
                    }
                mycpLogarithmTrans(image, spectral, bmpInfo.nWidth, bmpInfo.nHeight, dLogParam);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }

#undef MY_NORMALIZE_SUB_IMG
#undef MY_CHECK_MAIN_SUB_IMG

        case SUBW_LOG_TRANS_PARAM_EDT:
        {
            WCHAR szLogParam[10];
            GetWindowText(edtLogParam, szLogParam, 10);
            float iUserInput = 1;
            if (swscanf(szLogParam, L"%f", &iUserInput) == 1)
                dLogParam = (double)iUserInput;
            // Make sure c in clog(1+r) is not negative.
            dLogParam = max(0, dLogParam);
            return 0;
        }
        case SUBW_LOG_TRANS_EXECU_BTN:
        {
            myLogarithmTrans(image, &bmpInfo, dLogParam);
            InvalidateRect(hwnd, NULL, TRUE);
            // Release spectral data (if has).
            if (spectral != NULL) { free(spectral); spectral = NULL; }
            return 0;
        }
        case SUBW_EXP_TRANS_GAMMA_EDT:
        {
            WCHAR szGamma[10];
            GetWindowText(edtExpGamma, szGamma, 10);
            float iUserInput = 1;
            if (swscanf(szGamma, L"%f", &iUserInput) == 1)
                dExpGamma = (double)iUserInput;
            // Make sure gamma exponential is not negative.
            dExpGamma = max(0, dExpGamma);
            return 0;
        }
        case SUBW_EXP_TRANS_EXECU_BTN:
        {
            myExpGammaTrans(image, &bmpInfo, dExpGamma);
            InvalidateRect(hwnd, NULL, TRUE);
            // Release spectral data (if has).
            if (spectral != NULL) { free(spectral); spectral = NULL; }
            return 0;
        }
        case SUBW_SPE_RECON_MAIN_POWER_SUB_PHASE_BTN:
        {
            if (gImage == NULL) goto main_window_empty;
            if (gBmpInfo.nWidth != bmpInfo.nWidth ||
                gBmpInfo.nHeight != bmpInfo.nHeight)
                goto main_sub_size_not_matched;
            if (spectral == NULL || gSpectral == NULL) goto empty_spectral;
            // main: power, sub: phase
            mySpectralReconstruct(image, &bmpInfo, gSpectral, spectral);
            // Clear origin spectral info.
            free(spectral);
            spectral = NULL;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_SPE_RECON_MAIN_PHASE_SUB_POWER_BTN:
        {
            if (gImage == NULL) goto main_window_empty;
            if (gBmpInfo.nWidth != bmpInfo.nWidth ||
                gBmpInfo.nHeight != bmpInfo.nHeight)
                goto main_sub_size_not_matched;
            if (spectral == NULL || gSpectral == NULL) goto empty_spectral;
            // main: phase, sub: power
            mySpectralReconstruct(image, &bmpInfo, spectral, gSpectral);
            // Clear origin spectral info.
            free(spectral);
            spectral = NULL;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_SPE_RECON_MAIN_POWER_SUB_PHASE_FFT_BTN:
        {
            if (gImage == NULL) goto main_window_empty;
            if (gBmpInfo.nWidth != bmpInfo.nWidth ||
                gBmpInfo.nHeight != bmpInfo.nHeight)
                goto main_sub_size_not_matched;
            if (spectral == NULL || gSpectral == NULL) goto empty_spectral;

            // Padding zeros for non-radix-2 FFT.
            UINT RM = myGetRadix2(bmpInfo.nWidth);
            UINT RN = myGetRadix2(bmpInfo.nHeight);
            MyBmpInfo tmpInfo;
            MyBGRA* tmpImg = myPaddingZeroImageFFT(&tmpInfo, image, &bmpInfo);
            MyComplex* tmpPower = myPaddingZeroSpectralFFT(gSpectral, &bmpInfo);
            MyComplex* tmpPhase = myPaddingZeroSpectralFFT(spectral, &bmpInfo);

            // main: power, sub: phase
            mySpectralReconstructFFT(tmpImg, &tmpInfo, RM, RN, tmpPower, tmpPhase);

            // Copy from padding buffers.
            for (UINT i = 0; i < bmpInfo.nWidth; ++i)
                for (UINT j = 0; j < bmpInfo.nHeight; ++j)
                {
                    UINT dstIdx = i + j * bmpInfo.nWidth;
                    UINT srcIdx = i + j * tmpInfo.nWidth;
                    image[dstIdx] = tmpImg[srcIdx];
                }
            free(tmpImg); free(tmpPower); free(tmpPhase);

            // Clear origin spectral info.
            free(spectral);
            spectral = NULL;

            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case SUBW_SPE_RECON_MAIN_PHASE_SUB_POWER_FFT_BTN:
        {
            if (gImage == NULL) goto main_window_empty;
            if (gBmpInfo.nWidth != bmpInfo.nWidth ||
                gBmpInfo.nHeight != bmpInfo.nHeight)
                goto main_sub_size_not_matched;
            if (spectral == NULL || gSpectral == NULL) goto empty_spectral;

            // Padding zeros for non-radix-2 FFT.
            UINT RM = myGetRadix2(bmpInfo.nWidth);
            UINT RN = myGetRadix2(bmpInfo.nHeight);
            MyBmpInfo tmpInfo;
            MyBGRA* tmpImg = myPaddingZeroImageFFT(&tmpInfo, image, &bmpInfo);
            MyComplex* tmpPower = myPaddingZeroSpectralFFT(spectral, &bmpInfo);
            MyComplex* tmpPhase = myPaddingZeroSpectralFFT(gSpectral, &bmpInfo);

            // main: phase, sub: power
            mySpectralReconstructFFT(tmpImg, &tmpInfo, RM, RN, tmpPower, tmpPhase);

            // Copy from padding buffers.
            for (UINT i = 0; i < bmpInfo.nWidth; ++i)
                for (UINT j = 0; j < bmpInfo.nHeight; ++j)
                {
                    UINT dstIdx = i + j * bmpInfo.nWidth;
                    UINT srcIdx = i + j * tmpInfo.nWidth;
                    image[dstIdx] = tmpImg[srcIdx];
                }
            free(tmpImg); free(tmpPower); free(tmpPhase);

            // Clear origin spectral info.
            free(spectral);
            spectral = NULL;

            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        default:
            return 0;
        }
    empty_spectral:
        MessageBox(hwnd, L"频谱为空, 无法进行操作.", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    main_window_empty:
        MessageBox(hwnd, L"主窗口为空, 无法进行操作.", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    main_sub_size_not_matched:
        MessageBox(hwnd, L"主-副图像尺寸不匹配, 无法进行操作.", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    case WM_DESTROY:
        // Release allocated spectral data (if has).
        if (spectral != NULL) { free(spectral); spectral = NULL; }
        // Note we have allocated memory for image data.
        free(image);
        // Move focus to main window in case of accident hide.
        SetFocus(gMainWnd);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                    Gray Transform Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// GTW: Gray Transform Window - 101
// Its child window should start from 101_01.
#define GTW_SHOW_HIST_BTN 10101
// UPDT: update
#define GTW_UPDT_HIST_BTN 10102
#define GTW_HIST_STEP_LBL 10103
#define GTW_HIST_STEP_EDT 10104
// STRC: stretch
#define GTW_HIST_STRC_LBL 10105
#define GTW_HIST_STRC_EDT 10106
// CHNG: change
#define GTW_SAVE_CHNG_BTN 10107

#define GTW_ON_HIST_DISP_WND_CLOSED 10108

LRESULT CALLBACK GrayTransWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image;
    static UINT nGrayType;

    static HWND btnShowHist;
    static BOOL bShowHistResult = FALSE;

    static HWND btnUpdateHist;

    static HWND lblHistStep;
    static HWND edtHistStep;
    static UINT8 nGrayScaleStep = 1;

    static HWND lblHistStretch;
    static HWND edtHistStretch;
    static double yHistStretch = 15;

    static LONG_PTR aHistInfoPtr[] = {
        (LONG_PTR)& nGrayScaleStep,
        (LONG_PTR)& yHistStretch };

    static HWND btnApplyChange;

    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
        // Get selected gray transform type.
        nGrayType = (UINT)pCreateStruct->lpCreateParams;

        // We decide that user can only open one window with the same type at the same time.
        EnableMenuItem(gMenu, IDM_FILE, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_GRAY, MF_GRAYED | MF_BYPOSITION);

        // Allocate memory for image data of child window.
        image = (MyBGRA*)malloc(sizeof(MyBGRA) * gBmpInfo.nWidth * gBmpInfo.nHeight); 
        myAssert(image);

        // Acquire selected menu item text.
        WCHAR szMenuText[20] = L"灰度变换 - ";
        GetMenuString(gMenu, nGrayType, szMenuText + 7, 13, MF_BYCOMMAND);
        // Set child window caption text.
        SetWindowText(hwnd, szMenuText);

        // Extract gray value in different way.
        switch (nGrayType)
        {
        case IDM_GRAY_EMPI:
            myExtractGrayEmpi(image, gImage, &gBmpInfo);
            break;
        case IDM_GRAY_EVEN:
            myExtractGrayEven(image, gImage, &gBmpInfo);
            break;
        case IDM_GRAY_GAMMA:
            myExtractGrayGamma(image, gImage, &gBmpInfo);
            break;
        case IDM_GRAY_R:
            myExtractGrayR(image, gImage, &gBmpInfo);
            break;
        case IDM_GRAY_G:
            myExtractGrayG(image, gImage, &gBmpInfo);
            break;
        case IDM_GRAY_B:
            myExtractGrayB(image, gImage, &gBmpInfo);
            break;
        default: break;
        }

        RECT rc;
        GetClientRect(hwnd, &rc);
        int clntWidth = rc.right - rc.left;
        int clntHeight = rc.bottom - rc.top;

        int tmpBtnW = 12 * gCharWidth,
            tmpBtnH = 2 * gCharHeight;
        int tmpVert = clntHeight - tmpBtnH - 12;
        // Create show-histogram button.
        btnShowHist = CreateWindow(
            L"button",
            L"显示直方图",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_SHOW_HIST_BTN,
            gInstance,
            NULL);

        int tmpLastHorz = 5 + tmpBtnW;
        // Create update-histogram button
        btnUpdateHist = CreateWindow(
            L"button",
            L"更新直方图",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            tmpLastHorz + 5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_UPDT_HIST_BTN,
            gInstance,
            NULL);
        EnableWindow(btnUpdateHist, FALSE);

        tmpLastHorz += (5 + tmpBtnW);
        tmpBtnW = 6 * gCharWidth;
        // Create histogram-step text label & edit line;
        lblHistStep = CreateWindow(
            L"static",
            L"步长",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            tmpLastHorz + 5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_HIST_STEP_LBL,
            gInstance,
            NULL);

        tmpLastHorz += (5 + tmpBtnW);
        tmpBtnW = 4 * gCharWidth;
        edtHistStep = CreateWindow(
            L"edit",
            L"1",
            WS_CHILD | WS_VISIBLE,
            tmpLastHorz + 5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_HIST_STEP_EDT,
            gInstance,
            NULL);

        tmpLastHorz += (5 + tmpBtnW);
        tmpBtnW = 6 * gCharWidth;
        // Create histogram-stretch text label & edit line;
        lblHistStretch = CreateWindow(
            L"static",
            L"拉伸",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            tmpLastHorz + 5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_HIST_STRC_LBL,
            gInstance,
            NULL);

        tmpLastHorz += (5 + tmpBtnW);
        tmpBtnW = 4 * gCharWidth;
        edtHistStretch = CreateWindow(
            L"edit",
            L"15",
            WS_CHILD | WS_VISIBLE,
            tmpLastHorz + 5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_HIST_STRC_EDT,
            gInstance,
            NULL);

        tmpLastHorz += (5 + tmpBtnW);
        tmpBtnW = 14 * gCharWidth;
        // Create apply-change button.
        btnApplyChange = CreateWindow(
            L"button",
            L"应用至主窗口",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            tmpLastHorz + 5,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)GTW_SAVE_CHNG_BTN,
            gInstance,
            NULL);

        // Make sure all child widgets presented.
        tmpLastHorz += (10 + tmpBtnW);
        UINT cxRightOffset = max(gBmpInfo.nWidth + 24, (UINT)tmpLastHorz);

        // Resize window to suit right offset.
        if (cxRightOffset != gBmpInfo.nWidth + 24)
        {
            rc.left = 0; rc.top = 0;
            rc.right = cxRightOffset; rc.bottom = clntHeight;
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
            // Only change window width.
            MoveWindow(hwnd, pCreateStruct->x, pCreateStruct->y,
                       rc.right - rc.left, pCreateStruct->cy, TRUE);
        }
        return 0;
    }
    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int clntWidth = rc.right - rc.left;
        int clntHeight = rc.bottom - rc.top;

        int tmpBtnH = 2 * gCharHeight;
        int tmpVert = clntHeight - tmpBtnH - 5;

        POINT corner; // left top position

        // We decide to keep those widgets attached to window bottom.
        // Note GetWindowRect returns position in screen coordinate.
#define MY_MOVE_GRAY_TRANS_WIDGET(Widget_Handle) \
        GetWindowRect(Widget_Handle, &rc); \
        corner.x = rc.left; corner.y = rc.top; \
        ScreenToClient(hwnd, &corner); \
        MoveWindow(Widget_Handle, corner.x, tmpVert, rc.right - rc.left, rc.bottom - rc.top, TRUE);

        MY_MOVE_GRAY_TRANS_WIDGET(btnShowHist)
        MY_MOVE_GRAY_TRANS_WIDGET(btnUpdateHist)
        MY_MOVE_GRAY_TRANS_WIDGET(lblHistStep)
        MY_MOVE_GRAY_TRANS_WIDGET(edtHistStep)
        MY_MOVE_GRAY_TRANS_WIDGET(lblHistStretch)
        MY_MOVE_GRAY_TRANS_WIDGET(edtHistStretch)
        MY_MOVE_GRAY_TRANS_WIDGET(btnApplyChange)

#undef MY_MOVE_GRAY_TRANS_WIDGET

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        myDisplayImage(hdc, 12, 0, gBmpInfo.nWidth, gBmpInfo.nHeight, 0, 0, image, &gBmpInfo);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case GTW_SHOW_HIST_BTN: // Create child window and show histogram result.
        {
            // Show or Hide?
            if (bShowHistResult = !bShowHistResult)
            {
                // Update show-histogram button.
                SetWindowText(btnShowHist, L"关闭直方图");
                // Update update-histogram button.
                EnableWindow(btnUpdateHist, TRUE);

                RECT rc = { 0, 0,
                    271 + 14 * gCharWidth, // 5 + 256 + 5 + btn_width + 5
                    266 + 2 * gCharHeight }; // 256 + 5 + btn_height + 5
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
                // Create and show histogram result window.
                gGtwHistDispWnd = CreateWindow(
                    gGtwHistDispWndName,
                    L"直方图统计结果",
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                    left,
                    top,
                    rc.right - rc.left,
                    rc.bottom - rc.top,
                    hwnd,
                    NULL,
                    gInstance,
                    // Pass image data to child window to compute histogram result.
                    (LPVOID)image);
            }
            else
            {
                // Update show-histogram button.
                SetWindowText(btnShowHist, L"显示直方图");
                // Update update-histogram button.
                EnableWindow(btnUpdateHist, FALSE);

                // Close histogram result window.
                DestroyWindow(gGtwHistDispWnd);
                return 0; // don't update if destroyed
            }
            // fall through
        }
        case GTW_UPDT_HIST_BTN: // Re-compute and display histogram result.
        {
            // We must designate the command ID explicitly here
            // since it might fall through from the message above.
            SendMessage(gGtwHistDispWnd, WM_COMMAND, (WPARAM)GTW_UPDT_HIST_BTN, (LPARAM)aHistInfoPtr);
            return 0;
        }
        case GTW_HIST_STEP_EDT: // Get gray scale step from user input.
        {
            if (HIWORD(wParam) == EN_UPDATE)
            {
                WCHAR szStep[10];
                GetWindowText(edtHistStep, szStep, 10);
                int iUserInput = 1;
                if (swscanf(szStep, L"%d", &iUserInput) == 1)
                    nGrayScaleStep = (UINT8)iUserInput;
                // In case of user input 0.
                nGrayScaleStep = max(1, nGrayScaleStep);

                SendMessage(gGtwHistDispWnd, WM_COMMAND, wParam, (LPARAM)nGrayScaleStep);
            }
            return 0;
        }
        case GTW_HIST_STRC_EDT: // Get vertical stretch from user input.
        {
            if (HIWORD(wParam) == EN_UPDATE)
            {
                WCHAR szStretch[10];
                GetWindowText(edtHistStretch, szStretch, 10);
                float iUserInput = 1;
                if (swscanf(szStretch, L"%f", &iUserInput) == 1)
                    yHistStretch = (double)iUserInput;
                // In case user input 0 or negative.
                yHistStretch = max(1, yHistStretch);

                SendMessage(gGtwHistDispWnd, WM_COMMAND, wParam, (LPARAM)yHistStretch);
            }
            return 0;
        }
        case GTW_SAVE_CHNG_BTN: // Replace main window background with changed image.
        {
            memcpy(gImage, image, gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA));

            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            // Update operation menu state.
            myEnableOperationMenus();
            EnableMenuItem(gMenu, IDM_GRAY, MF_GRAYED | MF_BYPOSITION);
            DrawMenuBar(gMainWnd);

            // Repaint main window image.
            InvalidateRect(gMainWnd, NULL, TRUE);
            return 0;
        }
        case GTW_ON_HIST_DISP_WND_CLOSED: // Send when histogram-display window closed.
        {
            bShowHistResult = FALSE;
            // Update show-histogram button.
            SetWindowText(btnShowHist, L"显示直方图");
            // Update update-histogram button.
            EnableWindow(btnUpdateHist, FALSE);
            return 0;
        }
        default:
            return 0;
        }
    }
    case WM_DESTROY:
        // Restore config variables.
        bShowHistResult = FALSE;
        nGrayScaleStep = 1;
        yHistStretch = 15;
        // Note we have allocated memory for image data.
        free(image);
        image = NULL;
        // If this is the last child window to destroy.
        if (myValidWndCount() == 2)
        {
            EnableMenuItem(gMenu, IDM_FILE, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        }
        // Make the menu selectable again since the window is closed.
        EnableMenuItem(gMenu, IDM_GRAY, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        DrawMenuBar(gMainWnd); // Force the main window to repaint the menu.
        SetFocus(gMainWnd); // Move focus to main window in case of accident hide.
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                             GTW Histogram Display Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// HDW: Hist Display Window - 102
// Its child window should start from 102_01.
// EQUA: equalize
#define GTW_HDW_EQUA_HIST_BTN 10201
// RGLT: regulate
#define GTW_HDW_RGLT_HIST_LBL 10202
// DESC: a broken line that describes histogram regulartion.
#define GTW_HDW_STAT_DESC_BTN 10203
#define GTW_HDW_OVER_DESC_BTN 10204
#define GTW_HDW_CLEA_DESC_BTN 10205
#define GTW_HDW_SHOW_DESC_BTN 10206
// STRC: stretch
#define GTW_HDW_RGLT_STRC_LBL 10207
#define GTW_HDW_RGLT_STRC_EDT 10208

int myAnchorPointCompareFunc(const void* left, const void* right)
{
    // Store anchor points ascending by x coordinate.
    return ((POINT*)left)->x - ((POINT*)right)->x;
}

LRESULT CALLBACK GtwHistDispWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;

    // @see GrayTransWndProc
    static UINT8 nGrayScaleStep = 1;
    static double yHistStretch = 15;

    static HWND btnEquaHist;
    static double histArray[256];

    static HWND btnRgltHist;
    static double targetHistArray[256];

    static HWND btnStatDesc;
    static HWND btnOverDesc;
    // Whether user is designating anchor points.
    static BOOL bIsPainting = FALSE;
    // Store user designated anchor points of broken lines.
    static POINT anchorPoints[256];
    // How many anchor points available in anchorPoints?
    static UINT nPaintedCount = 0;

    static HWND btnCleaDesc;

    static HWND btnShowDesc;
    // Whether to show broken lines over histogram result.
    static BOOL bShowPaints = TRUE;

    static HWND lblRgltStretch;
    static HWND edtRgltStretch;
    // Power exponent of anchor point y-value.
    // The bigger this variable is, the greater the relative difference
    // in probability distribution of the regulation histogram will be.
    static double rgltStretch = 1;

    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
        // Get image data from parent window.
        image = (MyBGRA*)pCreateStruct->lpCreateParams;

        // Create equalize-histogram button.
        btnEquaHist = CreateWindow(
            L"button",
            L"直方图均衡化",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            133 - 7 * gCharWidth, // (266 - btn_width) / 2
            261, // 256 + 5 (hist_height + btn_padding)
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_EQUA_HIST_BTN,
            gInstance,
            NULL);

        int tmpLastVert = 5;
        int tmpVerticalPadding = 12;
        // Create regulate-histogram button.
        btnRgltHist = CreateWindow(
            L"button", 
            L"直方图规定化",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            266, // 5 + 256 + 5
            tmpLastVert,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_RGLT_HIST_LBL,
            gInstance,
            NULL);
        EnableWindow(btnRgltHist, FALSE);

        tmpLastVert += (2 * gCharHeight + tmpVerticalPadding);
        // Create start-description button.
        btnStatDesc = CreateWindow(
            L"button",
            L"开始描点",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            266, // 5 + 256 + 5
            tmpLastVert,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_STAT_DESC_BTN,
            gInstance,
            NULL);

        tmpLastVert += (2 * gCharHeight + tmpVerticalPadding);
        // Create over-description button.
        btnOverDesc = CreateWindow(
            L"button",
            L"结束描点",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            266, // 5 + 256 + 5
            tmpLastVert,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_OVER_DESC_BTN,
            gInstance,
            NULL);
        EnableWindow(btnOverDesc, FALSE);

        tmpLastVert += (2 * gCharHeight + tmpVerticalPadding);
        // Create clear-description button.
        btnCleaDesc = CreateWindow(
            L"button",
            L"清除折线",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            266, // 5 + 256 + 5
            tmpLastVert,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_CLEA_DESC_BTN,
            gInstance,
            NULL);

        tmpLastVert += (2 * gCharHeight + tmpVerticalPadding);
        // Create clear-description button.
        btnShowDesc = CreateWindow(
            L"button",
            L"隐藏折线", // Present result by default.
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            266, // 5 + 256 + 5
            tmpLastVert,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_SHOW_DESC_BTN,
            gInstance,
            NULL);

        tmpLastVert += (2 * gCharHeight + tmpVerticalPadding);
        // Create regulation-stretch text label & edit line;
        lblRgltStretch = CreateWindow(
            L"static",
            L"规定系数",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            266, // 5 + 256 + 5
            tmpLastVert,
            9 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_RGLT_STRC_LBL,
            gInstance,
            NULL);

        edtRgltStretch = CreateWindow(
            L"edit",
            L"1",
            WS_CHILD | WS_VISIBLE,
            266 + 10 * gCharWidth, // 5 + 256 + 5 + padding
            tmpLastVert,
            4 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)GTW_HDW_RGLT_STRC_EDT,
            gInstance,
            NULL);

        // Compute histogram result for the first time.
        myComputeHistogramResult(image, &gBmpInfo, histArray);
        InvalidateRect(hwnd, NULL, TRUE); // i'm very careful
        return 0;
    }
    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        myDisplayHistogramResult(hdc, 5, 256, histArray, nGrayScaleStep, yHistStretch);

        if (bShowPaints)
            myDisplayHistRgltFormatLines(hdc, anchorPoints, nPaintedCount);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case GTW_HDW_EQUA_HIST_BTN: // Re-compute and display histogram result.
        {
            // Change image data.
            myEqualizeHistogram(image, &gBmpInfo, histArray);
            // Update histogram result with changed image data.
            myComputeHistogramResult(image, &gBmpInfo, histArray);

            // Notify self and parent window to repaint.
            InvalidateRect(hwnd, NULL, TRUE);
            InvalidateRect(gGrayTransWnd, NULL, TRUE);
            return 0;
        }
        case GTW_HDW_RGLT_HIST_LBL: // Regulate histgoram with specific format.
        {
            // We need to extract regulation format from anchor points firstly.
            myComputeRgltHistFormat(targetHistArray, anchorPoints, nPaintedCount, rgltStretch);
            myRegulateHistogram(image, &gBmpInfo, histArray, targetHistArray);

            // Update histogram result with changed image data.
            myComputeHistogramResult(image, &gBmpInfo, histArray);

            // Notify self and parent window to repaint.
            InvalidateRect(hwnd, NULL, TRUE);
            InvalidateRect(gGrayTransWnd, NULL, TRUE);
            return 0;
        }
        case GTW_HDW_STAT_DESC_BTN: // Start recording regulation format anchors.
        {
            bIsPainting = TRUE;
            EnableWindow(btnRgltHist, FALSE);
            EnableWindow(btnStatDesc, FALSE);
            EnableWindow(btnOverDesc, TRUE);
            // Always show broken lines when start recording.
            bShowPaints = TRUE;
            SetWindowText(btnShowDesc, L"隐藏折线");
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case GTW_HDW_OVER_DESC_BTN: // End recording regulation format anchors.
        {
            bIsPainting = FALSE;
            EnableWindow(btnRgltHist, TRUE);
            EnableWindow(btnStatDesc, TRUE);
            EnableWindow(btnOverDesc, FALSE);
            // Complement user designated anchors.
            if (nPaintedCount >= 2)
            {
                // Stick endpoints to axes.
                anchorPoints[0].x = 5;
                anchorPoints[nPaintedCount - 1].x = 260;
            }
            else // Horizontal line by default.
            {
                nPaintedCount = 2;
                // left 5-pixel padding
                anchorPoints[0].x = 5;
                anchorPoints[0].y = 128;
                anchorPoints[1].x = 260;
                anchorPoints[1].y = 128;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case GTW_HDW_CLEA_DESC_BTN: // Remove all regulation format anchors.
        {
            nPaintedCount = 0;
            EnableWindow(btnRgltHist, FALSE);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case GTW_HDW_SHOW_DESC_BTN: // Show or hide regulation format lines.
        {
            if (bShowPaints = !bShowPaints)
                SetWindowText(btnShowDesc, L"隐藏折线");
            else
                SetWindowTextW(btnShowDesc, L"显示折线");
            // Notify self to repaint.
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case GTW_HDW_RGLT_STRC_EDT: // Get regulation stretch from user input.
        {
            WCHAR szStretch[10];
            GetWindowText(edtRgltStretch, szStretch, 10);
            float iUserInput = 1;
            if (swscanf(szStretch, L"%f", &iUserInput) == 1)
                rgltStretch = (double)iUserInput;
            // In case user input 0 or negative.
            // Although it's useful to set regulation stretch less than 1,
            // we decide to make it always doing amplification by default.
            // Besides, too big stretch will cause double overflow (so clamp below 100).
            rgltStretch = min(100, max(1, rgltStretch));
            return 0;
        }
        case GTW_HIST_STEP_EDT: // Receive histogram gray scale step update message.
        {
            nGrayScaleStep = (UINT8)lParam;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case GTW_HIST_STRC_EDT: // Receive histogram vertical stretch update message.
        {
            yHistStretch = (double)lParam;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case GTW_UPDT_HIST_BTN:
        {
            LONG_PTR* pHistInfo = (LONG_PTR*)lParam;
            // aHistInfoPtr from parent window stores 2 ptr:
            // 1st is UINT8 ptr to step, 2nd is double ptr to stretch.
            nGrayScaleStep = *((UINT8*)pHistInfo[0]);
            yHistStretch = *((double*)pHistInfo[1]);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        default:
            return 0;
        }
        return 0;
    }
    case WM_LBUTTONDBLCLK:
    {
        if (bIsPainting)
        {
            int xClk = LOWORD(lParam);
            int yClk = HIWORD(lParam);
            // left 5-pixel padding
            xClk = min(260, max(5, xClk));
            yClk = min(256, max(1, yClk));

            UINT idx = UINT_MAX;
            if (nPaintedCount == 0) // first anchor point
            {
                ++nPaintedCount; idx = 0;
            }
            else
            {
                // Check whether there's conflicted x coordinate.
                for (UINT i = 0; i < nPaintedCount; ++i)
                {
                    if (anchorPoints[i].x == xClk)
                    {
                        idx = i; // Overwrite origin data if existed.
                        break;
                    }
                }
            }
            if (idx == UINT_MAX) // no first, no conflict
            {
                ++nPaintedCount;
                nPaintedCount = min(256, nPaintedCount);
                // Excess anchor will overwrite last anchor by default.
                idx = nPaintedCount - 1;
            }
            anchorPoints[idx].x = xClk;
            anchorPoints[idx].y = yClk;

            // Store anchor points ascending by x coordinate.
            qsort(anchorPoints, nPaintedCount, sizeof(POINT), myAnchorPointCompareFunc);

            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_DESTROY:
        // Restore config variables.
        bIsPainting = FALSE;
        nPaintedCount = 0;
        bShowPaints = TRUE;
        // Notify parent window to resume show-histogram button.
        SendMessage(gGrayTransWnd, WM_COMMAND, GTW_ON_HIST_DISP_WND_CLOSED, 0);
        // Move focus to parent window in case of accident hide.
        SetFocus(gGrayTransWnd);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                     Domain Filter Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// Domain Filter Window - 103
// Its child window should start from 103_01.
#define DOMAIN_GENE_APPLY_BTN 10301
#define DOMAIN_GENE_SAVE_MAIN_BTN 10302
#define DOMAIN_GENE_BORDER_MODE_BTN 10303
// Label range: 10310 ~ 10329
#define DOMAIN_GENE_PARAM_LBL 10310
// Edit line range: 10330 ~ 10349
#define DOMAIN_GENE_PARAM_EDT 10330

/*
* A parameter widget line is a group of text label and edit line.
* Different SPAF-type windows would be bound to different counts.
* We introduce this mechanism to help decrease duplicated codes for window creating,
* since all filters share the same config mode (a pair of parameter key and value).
* When we want to introduce a new type of filter, we only need to designate the count
* of parameter it needs and set detailed parameter key and value later in switch-case.
*/
void myCreateParamWidgets(HWND hwnd, int nParamCount, int xLeftPos, HWND lblParam[10], HWND edtParam[10])    
{
    for (int i = 0; i < nParamCount; ++i)
    {
        int yVertPos = 12 + i * 3 * gCharHeight;
        // Create parameter text label.
        lblParam[i] = CreateWindow(
            L"static",
            NULL,
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            xLeftPos,
            yVertPos,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)(DOMAIN_GENE_PARAM_LBL + i),
            gInstance,
            NULL);
        // Create parameter edit line.
        edtParam[i] = CreateWindow(
            L"edit",
            NULL,
            WS_CHILD | WS_VISIBLE,
            xLeftPos + 15 * gCharWidth,
            yVertPos,
            10 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)(DOMAIN_GENE_PARAM_EDT + i),
            gInstance,
            NULL);
    }
}

// Make sure widget lines stick to right border of window.
void myMoveParamWidgets(int nParamCount, int xLeftPos, HWND lblParam[10], HWND edtParam[10])
{
    RECT rc; // Declare common rect here.

    for (int i = 0; i < nParamCount; ++i) // Note xLeftPos is in screen coordinate.
    {
        int yVertPos = 12 + i * 3 * gCharHeight;
        // Update text label.
        GetWindowRect(lblParam[i], &rc);
        MoveWindow(lblParam[i], xLeftPos, yVertPos,
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);
        // Update edit line.
        GetWindowRect(edtParam[i], &rc);
        MoveWindow(edtParam[i], xLeftPos + 15 * gCharWidth, yVertPos,
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }
}

// Customize cursor for different transfer functions.
void myDrawTransferFuncCursor(HDC hdc, UINT filterType, int iParamVal[20], double dParamVal[20],
                              int xParamVal[20], int yParamVal[20], POINT* markPos, int markIdx)
{
    switch (filterType)
    {
    case IDM_TFUNC_ILPF:
    case IDM_TFUNC_BLPF:
    case IDM_TFUNC_IHPF:
    case IDM_TFUNC_BHPF:
    case IDM_TFUNC_HOMO:
    {
        int tmpR = (int)dParamVal[2 * markIdx + 1];
        // circle
        Ellipse(hdc, markPos->x - tmpR, markPos->y - tmpR, markPos->x + tmpR, markPos->y + tmpR);
        // horz cross
        MoveToEx(hdc, markPos->x - (tmpR + 5), markPos->y, NULL);
        LineTo(hdc, markPos->x + (tmpR + 5), markPos->y);
        // vert cross
        MoveToEx(hdc, markPos->x, markPos->y - (tmpR + 5), NULL);
        LineTo(hdc, markPos->x, markPos->y + (tmpR + 5));
        break;
    }
    case IDM_TFUNC_GLPF:
    case IDM_TFUNC_GHPF:
    {
        int sigma = (int)dParamVal[2 * markIdx + 1];
        int _3sig = (int)(dParamVal[2 * markIdx + 1] * 3);
        // sigma circle
        Ellipse(hdc, markPos->x - sigma, markPos->y - sigma, markPos->x + sigma, markPos->y + sigma);
        // 3-sigma circle
        Ellipse(hdc, markPos->x - _3sig, markPos->y - _3sig, markPos->x + _3sig, markPos->y + _3sig);
        // horz cross
        MoveToEx(hdc, markPos->x - (_3sig + 5), markPos->y, NULL);
        LineTo(hdc, markPos->x + (_3sig + 5), markPos->y);
        // vert cross
        MoveToEx(hdc, markPos->x, markPos->y - (_3sig + 5), NULL);
        LineTo(hdc, markPos->x, markPos->y + (_3sig + 5));
    }
    default:
        break;
    }
}

LRESULT CALLBACK DomainFilterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;
    static MyComplex* spectral = NULL;
    
    static HWND btnApply;
    static UINT nFilterType;

    static HWND btnSaveMain;

    static HWND b3sBorderMode;
    
    // Where the right panel should start in horizontal.
    static UINT xBottomPanelRight = 0;
    // Where the bottom panel should start in vertical.
    static UINT yRightPanelBottom = 0;

    // Decide the filter how to populate border pixels (BM: Border Mode).
    // We borrow the flags of auto-3-state-button opportunely.

#define MY_SPAF_BM_BLACK ( BST_UNCHECKED )
#define MY_SPAF_BM_MIRROR ( BST_CHECKED )
#define MY_SPAF_BM_DUPLICATE ( BST_INDETERMINATE )

    static UINT nBorderMode = MY_SPAF_BM_MIRROR;

    static HWND lblParam[20];
    static HWND edtParam[20];
    // We store [int] and [double] value arrays separately here,
    // as one text can be translated to different numeric types.
    static int iParamVal[20];
    static double dParamVal[20];
    static int xParamVal[20], yParamVal[20];
    
    // How many key-value pairs need for each filter type?
    static int nParamCount[] =
    {
        // space-domain

        1, // Box Filter
        2, // Gaussian Filter
        1, // Median Filter
        0, // Laplace Filter
        0, // Sobel Filter
        0, // Custom Core

        // freq-domain

        1, // Power Spectral
        1, // Phase Spectral
        1, // Power Spectral FFT
        1, // Phase Spectral FFT
        0, // Custom Core

        // transfer-func

        10, // Ideal LPF
        10, // Gaussian LPF
        11, // Butterworth LPF
        10, // Ideal HPF
        10, // Gaussian HPF
        11, // Butterworth HPF
        11, // Homomorphic Filter

        // noise-model

        2, // Gaussian
        2, // Rayleigh
        2, // Erlang
        1, // Exponential
        2, // Uniform
        2, // Salt & Pepper
    };

    // Available display area for image.
    static int validWidth = 0;
    static int validHeight = 0;

    static bCsorInValidArea = FALSE;

    // Image display offset.
    static int xImgAnchor = 0, yImgAnchor = 0;

    // Marker information of transfer-func window.
    static POINT markerPositions[10];

    static COLORREF markerColors[10] =
    {
        0x0000FF, // Red
        0x007FFF, // Orange
        0x00FFFF, // Yellow
        0x00FF00, // Green
        0xFFFF00, // Cyan
        0xFF0000, // Blue
        0xFF008B, // Purple
        0x000000, // Black
        0xFFFFFF, // White
        0x888888  // Gray
    };
    static HPEN markerPens[10];
    // Next marker to place.
    static UINT currMarkerIdx = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
        // Get selected spatial filter type.
        nFilterType = (UINT)pCreateStruct->lpCreateParams;

        // We decide that user can only open one window with the same type at the same time.
        EnableMenuItem(gMenu, IDM_FILE, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_DOMAIN, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_TRANSFER_FUNC, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_NOISE_MODEL, MF_GRAYED | MF_BYPOSITION);

        // Allocate memory for image data.
        size_t nPixelCount = gBmpInfo.nWidth * gBmpInfo.nHeight;
        image = (MyBGRA*)malloc(nPixelCount * sizeof(MyBGRA));
        myAssert(image);
        memcpy(image, gImage, nPixelCount * sizeof(MyBGRA));
        // Copy spectral by the way (if has).
        if (gSpectral != NULL)
        {
            spectral = (MyComplex*)malloc(nPixelCount * sizeof(MyComplex));
            myAssert(spectral);
            memcpy(spectral, gSpectral, nPixelCount * sizeof(MyComplex));
        }

        // Acquire selected menu item text.
        WCHAR szDomainFilterMenuText[20] = L"卷积滤波 - ";
        WCHAR szTransferFuncMenuText[20] = L"传递函数 - ";
        WCHAR szNoisingModelMenuText[20] = L"噪声模型 - ";

        WCHAR* pSelectedMenuText =
            (nFilterType >= IDM_TFUNC_ILPF) ?
                ((nFilterType >= IDM_NMODEL_GAUSSIAN) ?
                szNoisingModelMenuText :
                szTransferFuncMenuText) :
            szDomainFilterMenuText;

        GetMenuString(gMenu, nFilterType, pSelectedMenuText + 7, 13, MF_BYCOMMAND);
        // Set child window caption text.
        SetWindowText(hwnd, pSelectedMenuText);

        // Create apply-all button.
        btnApply = CreateWindow(
            L"button",
            L"计算结果",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            12,
            gBmpInfo.nHeight + 6,
            10 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)DOMAIN_GENE_APPLY_BTN,
            gInstance,
            NULL);

        // Create save-to-main button.
        btnSaveMain = CreateWindow(
            L"button",
            L"保存至主窗口",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            18 + 10 * gCharWidth,
            gBmpInfo.nHeight + 6,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)DOMAIN_GENE_SAVE_MAIN_BTN,
            gInstance,
            NULL);

        // Create border-mode button.
        b3sBorderMode = CreateWindow(
            L"button",
            L"边界模式: 镜像",
            WS_CHILD | WS_VISIBLE | BS_AUTO3STATE,
            24 + 24 * gCharWidth,
            gBmpInfo.nHeight + 6,
            16 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)DOMAIN_GENE_BORDER_MODE_BTN,
            gInstance,
            NULL);
        CheckDlgButton(hwnd, DOMAIN_GENE_BORDER_MODE_BTN, MY_SPAF_BM_MIRROR /* Checked */);

        // Record right most of bottom panel.
        xBottomPanelRight = 36 + 40 * gCharWidth;

        UINT iParamIndex =
            (nFilterType >= IDM_TFUNC_ILPF) ?
                ((nFilterType >= IDM_NMODEL_GAUSSIAN) ?
                (nFilterType - IDM_NMODEL_GAUSSIAN + MY_DOMAIN_IDM_COUNT + MY_TFUNC_IDM_COUNT) :
                (nFilterType - IDM_TFUNC_ILPF + MY_DOMAIN_IDM_COUNT)) :
            (nFilterType - IDM_SPAF_BOX);
        // Create related parameter widget lines for specific filter/tfunc/nmodel type.
        myCreateParamWidgets(hwnd, nParamCount[iParamIndex],
                             gBmpInfo.nWidth + 24, lblParam, edtParam);

        // Record bottom most of right panel.
        yRightPanelBottom = 12 + nParamCount[iParamIndex] * 3 * gCharHeight;

        // Decide detailed parameter description for each filter type.
        switch (nFilterType)
        {
        case IDM_SPAF_BOX: // Box Filter
        {
            SetWindowText(lblParam[0], L"归一系数");
            SetWindowText(edtParam[0], L"1");
            iParamVal[0] = 1; // m x m, norm coef
            break;
        }
        case IDM_SPAF_GAUS: // Gaussian Filter
        {
            SetWindowText(lblParam[0], L"强度 (1_sigma)");
            SetWindowText(edtParam[0], L"1");
            dParamVal[0] = 1; // standard deviation
            SetWindowText(lblParam[1], L"半径 (3_sigma)");
            SetWindowText(edtParam[1], L"3");
            iParamVal[1] = 3; // would better be 3-sigma
            break;
        }
        case IDM_SPAF_MEDI: // Median Filter
        {
            SetWindowText(lblParam[0], L"覆盖半径");
            SetWindowText(edtParam[0], L"1");
            iParamVal[0] = 1;
            break;
        }
        case IDM_SPEF_POWER: // Power Spectral
        case IDM_SPEF_PHASE: // Phase Spectral
        case IDM_SPEF_POWER_FFT:
        case IDM_SPEF_PHASE_FFT:
        {
            SetWindowText(lblParam[0], L"clog(1+r), c");
            SetWindowText(edtParam[0], L"1");
            dParamVal[0] = 1;
            break;
        }
        case IDM_TFUNC_ILPF:
        case IDM_TFUNC_IHPF:
        {
#define MY_ILPF_PARAM_LINE(Index) \
    SetWindowText(lblParam[2 * Index - 2], L"光标 " L#Index L", 中心"); \
    SetWindowText(edtParam[2 * Index - 2], L"-1, -1"); \
    SetWindowText(lblParam[2 * Index - 1], L"光标 " L#Index L", 半径"); \
    SetWindowText(edtParam[2 * Index - 1], L"10"); dParamVal[2 * Index - 2] = 10

            MY_ILPF_PARAM_LINE(1);
            MY_ILPF_PARAM_LINE(2);
            MY_ILPF_PARAM_LINE(3);
            MY_ILPF_PARAM_LINE(4);
            MY_ILPF_PARAM_LINE(5);

#undef MY_ILPF_PARAM_LINE
            break;
        }
        case IDM_TFUNC_GLPF:
        case IDM_TFUNC_GHPF:
        {
#define MY_GLPF_PARAM_LINE(Index) \
    SetWindowText(lblParam[2 * Index - 2], L"光标 " L#Index L", 中心"); \
    SetWindowText(edtParam[2 * Index - 2], L"-1, -1"); \
    SetWindowText(lblParam[2 * Index - 1], L"光标 " L#Index L", sigma"); \
    SetWindowText(edtParam[2 * Index - 1], L"10"); dParamVal[2 * Index - 2] = 10

            MY_GLPF_PARAM_LINE(1);
            MY_GLPF_PARAM_LINE(2);
            MY_GLPF_PARAM_LINE(3);
            MY_GLPF_PARAM_LINE(4);
            MY_GLPF_PARAM_LINE(5);

#undef MY_GLPF_PARAM_LINE
            break;
        }
        case IDM_TFUNC_BLPF:
        case IDM_TFUNC_BHPF:
        {
#define MY_BLPF_PARAM_LINE(Index) \
    SetWindowText(lblParam[2 * Index - 2], L"光标 " L#Index L", 中心"); \
    SetWindowText(edtParam[2 * Index - 2], L"-1, -1"); \
    SetWindowText(lblParam[2 * Index - 1], L"光标 " L#Index L", 截止频率"); \
    SetWindowText(edtParam[2 * Index - 1], L"10"); dParamVal[2 * Index - 2] = 10

            MY_BLPF_PARAM_LINE(1);
            MY_BLPF_PARAM_LINE(2);
            MY_BLPF_PARAM_LINE(3);
            MY_BLPF_PARAM_LINE(4);
            MY_BLPF_PARAM_LINE(5);

            SetWindowText(lblParam[10], L"Butterworth 阶数");
            SetWindowText(edtParam[10], L"2.25"); dParamVal[10] = 2.25;

#undef MY_BLPF_PARAM_LINE
            break;
        }
        case IDM_TFUNC_HOMO:
        {
#define MY_HOMO_PARAM_LINE(Index) \
    SetWindowText(lblParam[2 * Index - 2], L"光标 " L#Index L", 中心"); \
    SetWindowText(edtParam[2 * Index - 2], L"-1, -1"); \
    SetWindowText(lblParam[2 * Index - 1], L"光标 " L#Index L", 拟合 sigma"); \
    SetWindowText(edtParam[2 * Index - 1], L"10"); dParamVal[2 * Index - 2] = 10

            MY_HOMO_PARAM_LINE(1);
            MY_HOMO_PARAM_LINE(2);
            MY_HOMO_PARAM_LINE(3);
            MY_HOMO_PARAM_LINE(4);

            SetWindowText(lblParam[8], L"偏斜度 c");
            SetWindowText(edtParam[8], L"5"); dParamVal[8] = 5;

            SetWindowText(lblParam[9], L"Gamma Low");
            SetWindowText(edtParam[9], L"0.5"); dParamVal[9] = 0.5;

            SetWindowText(lblParam[10], L"Gamma High");
            SetWindowText(edtParam[10], L"5"); dParamVal[10] = 5;

#undef MY_BLPF_PARAM_LINE
            break;
        }
        case IDM_NMODEL_GAUSSIAN:
        {
            SetWindowText(lblParam[0], L"均值 u");
            SetWindowText(edtParam[0], L"0");
            dParamVal[0] = 0; // u, mean value
            SetWindowText(lblParam[1], L"标准差 sigma");
            SetWindowText(edtParam[1], L"10");
            dParamVal[1] = 10; // sigma, standard deviation
            break;
        }
        case IDM_NMODEL_RAYLEIGH:
        {
            // u = a + sqrt(pi * b / 4)
            // sigma^2 = b * (4 - pi) / 4
            SetWindowText(lblParam[0], L"参数 a");
            SetWindowText(edtParam[0], L"0");
            dParamVal[0] = 0; // parameter a
            SetWindowText(lblParam[1], L"参数 b");
            SetWindowText(edtParam[1], L"500");
            dParamVal[1] = 500; // parameter b
            break;
        }
        case IDM_NMODEL_ERLANG:
        {
            // u = b / a
            // sigma^2 = b / a^2
            SetWindowText(lblParam[0], L"参数 a");
            SetWindowText(edtParam[0], L"1");
            dParamVal[0] = 1; // parameter a
            SetWindowText(lblParam[1], L"参数 b");
            SetWindowText(edtParam[1], L"2");
            iParamVal[1] = 2; // parameter b
            break;
        }
        // Special case of erlang distribution when b = 1.
        case IDM_NMODEL_EXP:
        {
            // u = 1 / a
            // sigma^2 = 1 / a^2
            SetWindowText(lblParam[0], L"保持概率 a");
            SetWindowText(edtParam[0], L"0.1");
            dParamVal[0] = 0.1; // parameter a
            break;
        }
        case IDM_NMODEL_UNIFORM:
        {
            SetWindowText(lblParam[0], L"起始点 a");
            SetWindowText(edtParam[0], L"-25");
            dParamVal[0] = -25; // start point
            SetWindowText(lblParam[1], L"结束点 b");
            SetWindowText(edtParam[1], L"+25");
            dParamVal[1] = +25; // end point
            break;
        }
        case IDM_NMODEL_SANDP:
        {
            SetWindowText(lblParam[0], L"白点概率 P1");
            SetWindowText(edtParam[0], L"0.1");
            dParamVal[0] = 0.1; // salt probability
            SetWindowText(lblParam[1], L"黑点概率 P2");
            SetWindowText(edtParam[1], L"0.1");
            dParamVal[1] = 0.1; // pepper probability
            break;
        }
        default:
            break;
        }

        // Initialize helper variables.
        validWidth = gBmpInfo.nWidth;
        validHeight = gBmpInfo.nHeight;

        // Create marker utils for transfer-func window.
        if (nFilterType >= IDM_TFUNC_ILPF && nFilterType < IDM_NMODEL_GAUSSIAN)
        {
            int i; // Declare common index variables here.
            for (i = 0; i < _countof(markerPositions); ++i)
            {
                // Initialize as unused to hide the marker.
                markerPositions[i].x = markerPositions[i].y = -1;
            }
            // Create marker color pens.
            for (i = 0; i < _countof(markerColors); ++i)
            {
                markerPens[i] = CreatePen(PS_SOLID, 2, markerColors[i]);
            }
        }

        UINT tmpOrgX = gBmpInfo.nWidth + 24;
        UINT tmpOrgY = gBmpInfo.nHeight + 2 * gCharHeight + 12;
        // Resize window to display all widgets.
        if (xBottomPanelRight > tmpOrgX || yRightPanelBottom > tmpOrgY)
        {
            RECT rc;
            rc.left = rc.top = 0;
            rc.right = max(xBottomPanelRight, tmpOrgX) + 25 * gCharWidth + 12;
            rc.bottom = max(yRightPanelBottom, tmpOrgY); // no extra padding
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            // Only change window width.
            MoveWindow(hwnd, pCreateStruct->x, pCreateStruct->y,
                       rc.right - rc.left, rc.bottom - rc.top, TRUE);
        }
        return 0;
    }
    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_KEYDOWN:
    {
        // Change current selected marker.
        // '0' for 10th marker.
        if (wParam == '0')
            currMarkerIdx = 9;
        // '1' ~ '9' for 1st ~ 9th markers.
        else if (wParam > '0' && wParam <= '9')
            currMarkerIdx = (UINT)wParam - '0' - 1;

        // Make sure marker index is in valid range.
        switch (nFilterType)
        {
        case IDM_TFUNC_ILPF:
        case IDM_TFUNC_GLPF:
        case IDM_TFUNC_BLPF:
        case IDM_TFUNC_IHPF:
        case IDM_TFUNC_GHPF:
        case IDM_TFUNC_BHPF:
            currMarkerIdx = min(4, currMarkerIdx);
            break;
        case IDM_TFUNC_HOMO:
            currMarkerIdx = min(3, currMarkerIdx);
            break;
        default:
            break;
        }

        // There's no need to erase since double-buffer is enabled.
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        UINT clntWidth = rc.right - rc.left;
        UINT clntHeight = rc.bottom - rc.top;

        // Recalculate available image area.
        validWidth = rc.right - rc.left - 36 - 25 * gCharWidth;
        validHeight = rc.bottom - rc.top - 12 - 2 * gCharHeight;
        // Clamp to origin image size when window is larger.
        validWidth = min(validWidth, gBmpInfo.nWidth);
        validHeight = min(validHeight, gBmpInfo.nHeight);

        // Always reset image anchor when resized.
        xImgAnchor = yImgAnchor = 0;

        // Keep apply-all button stick to window bottom.
        GetWindowRect(btnApply, &rc);
        MoveWindow(btnApply, 12, clntHeight - 6 - (rc.bottom - rc.top),
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);
        // Keep save-to-main button stick to window bottom.
        GetWindowRect(btnSaveMain, &rc);
        MoveWindow(btnSaveMain, 18 + 10 * gCharWidth, clntHeight - 6 - (rc.bottom - rc.top),
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);
        // Keep border-mode button stick to window bottom.
        GetWindowRect(b3sBorderMode, &rc);
        MoveWindow(b3sBorderMode, 24 + 24 * gCharWidth, clntHeight - 6 - (rc.bottom - rc.top),
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);

        UINT iParamIndex =
            (nFilterType >= IDM_TFUNC_ILPF) ?
                ((nFilterType >= IDM_NMODEL_GAUSSIAN) ?
                (nFilterType - IDM_NMODEL_GAUSSIAN + MY_DOMAIN_IDM_COUNT + MY_TFUNC_IDM_COUNT) :
                (nFilterType - IDM_TFUNC_ILPF + MY_DOMAIN_IDM_COUNT)) :
            (nFilterType - IDM_SPAF_BOX);
        // Keep widget lines stick to right border of window.
        myMoveParamWidgets(nParamCount[iParamIndex],
                           clntWidth - 12 - 25 * gCharWidth, lblParam, edtParam);

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        RECT rcClnt;
        GetClientRect(hwnd, &rcClnt);
        //--------------------------------------------------------------------- Double Buffer Start
        HDC hMemDC = CreateCompatibleDC(hdc); // Acquire copy device context.
        HBITMAP hMemBmp = CreateCompatibleBitmap( // Draw on temporary bitmap.
            hdc, rcClnt.right - rcClnt.left, rcClnt.bottom - rcClnt.top);
        SelectObject(hMemDC, hMemBmp);
        // Erase with master window's bkgn brush.
        FillRect(hMemDC, &rcClnt, (HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND));
        //---------------------------------------------------------------------

        int tmpCX = (int)width - 36 - 25 * (int)gCharWidth;
        int tmpCY = (int)height - 12 - 2 * (int)gCharHeight;
        // We have to do this to stick the image at left top,
        // since Y-axis is flipped in origin pixel buffer.
        int tmpImgY = tmpCY - gBmpInfo.nHeight;
        // Display image from left top corner of main window.
        myDisplayImage(hMemDC, 12, 0, max(0, tmpCX), max(0, tmpCY),
                       xImgAnchor, abs(min(0, tmpImgY)) + yImgAnchor, image, &gBmpInfo);

        // Show marker for transfer-func window.
        if (nFilterType >= IDM_TFUNC_ILPF && nFilterType < IDM_NMODEL_GAUSSIAN)
        {
            int i; // Declare common index variables here.

            POINT tmpPT;
            GetCursorPos(&tmpPT);
            ScreenToClient(hwnd, &tmpPT);
            // Display next marker hint (hide in move mode).
            if (bCsorInValidArea && !(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
            {
                HPEN orgPen = SelectObject(hMemDC, markerPens[currMarkerIdx]);
                HBRUSH orgBrush = SelectObject(hMemDC, GetStockObject(NULL_BRUSH));

                myDrawTransferFuncCursor(hMemDC, nFilterType, iParamVal, dParamVal,
                                         xParamVal, yParamVal, &tmpPT, currMarkerIdx);

                SelectObject(hMemDC, orgPen);
                SelectObject(hMemDC, orgBrush);
            }

            // Display verified markers.
            for (i = 0; i < _countof(markerPositions); ++i)
            {
                HPEN orgPen = SelectObject(hMemDC, GetStockObject(NULL_PEN));
                HBRUSH orgBrush = SelectObject(hMemDC, GetStockObject(NULL_BRUSH));

                // From image coordinate to window coordinate.
                POINT relativePT = { markerPositions[i].x - abs(xImgAnchor) + 12,
                                     markerPositions[i].y - abs(yImgAnchor) };
                // Check marker whether in valid area.
                if (relativePT.x >= 12 && relativePT.x <= 12 + validWidth &&
                    relativePT.y >= 0 && relativePT.y <= validHeight)
                {
                    SelectObject(hMemDC, markerPens[i]);

                    myDrawTransferFuncCursor(hMemDC, nFilterType, iParamVal, dParamVal,
                                             xParamVal, yParamVal, &relativePT, i /* marIdx */);
                }
                SelectObject(hMemDC, orgPen);
                SelectObject(hMemDC, orgBrush);
            }
        }

        //--------------------------------------------------------------------- Double Buffer End
        BitBlt(hdc, 0, 0, rcClnt.right - rcClnt.left,
               rcClnt.bottom - rcClnt.top, hMemDC, 0, 0, SRCCOPY);
        DeleteDC(hMemDC);
        DeleteObject(hMemBmp);
        //---------------------------------------------------------------------

        EndPaint(hwnd, &ps);
        return 0;

    case WM_SETCURSOR:
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Get valid area boundary.
        int tmpWndRightMost = rc.right - 24 - 20 * gCharWidth;
        int tmpImgRightMost = 12 + gBmpInfo.nWidth;
        int tmpWndBottomMost = rc.bottom - 12 - 2 * gCharHeight;
        int tmpImgBottomMost = gBmpInfo.nHeight;

        // Check whether above the image.
        if (pt.x >= 12 && pt.x < min(tmpImgRightMost, tmpWndRightMost) &&
            pt.y >= 0 && pt.y < min(tmpImgBottomMost, tmpWndBottomMost))
        {
            if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
                // Hold down right button to start move.
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
            else
                // Show mark cursor otherwise.
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));

            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            bCsorInValidArea = TRUE; // cursor in workspace

            return 0;
        }

        // There's no need to erase since double-buffer is enabled.
        if (bCsorInValidArea) InvalidateRect(hwnd, NULL, FALSE);
        bCsorInValidArea = FALSE; // cursor out of workspace

        break; // Use default cursor in other places.
    }
    case WM_MOUSEMOVE:
    {
        static int csorCurrX, csorCurrY;
        static int csorLastX, csorLastY;

        csorCurrX = GET_X_LPARAM(lParam);
        csorCurrY = GET_Y_LPARAM(lParam);

        if (bCsorInValidArea) // in workspace
        {
            // Mark position for transfer-func window.
            if (nFilterType >= IDM_TFUNC_ILPF && nFilterType < IDM_NMODEL_GAUSSIAN && (wParam & MK_LBUTTON))
            {
                // From window coordinate to image coordinate.
                markerPositions[currMarkerIdx].x = csorCurrX + abs(xImgAnchor);
                markerPositions[currMarkerIdx].y = csorCurrY + abs(yImgAnchor);
                // Use update logic in WM_LBUTTONDOWN.
                goto start_update_marker_center_edit_text;
            }
        end_update_marker_center_edit_text:
            // Trigger move mode.
            if (wParam & MK_RBUTTON)
            {
                // Note +/- to match image coordinate.
                xImgAnchor -= (csorCurrX - csorLastX);
                yImgAnchor += (csorCurrY - csorLastY);

                // Clamp image anchor offset into valid range.
                // X offset: 0 ~ (ImgW - ValidW)
                int xOffsetMost = gBmpInfo.nWidth - validWidth;
                xImgAnchor = max(0, min(xOffsetMost, xImgAnchor));
                // Y offset: (ValidH - ImgH) ~ 0
                int yOffsetMost = validHeight - gBmpInfo.nHeight;
                yImgAnchor = max(yOffsetMost, min(0, yImgAnchor));
            }
            // We have notified to repaint in WM_SETCURSOR.
        }
        csorLastX = csorCurrX;
        csorLastY = csorCurrY;
        return 0;
    }
    case WM_RBUTTONDOWN:
        // Help kill focus of child widgets.
        SetFocus(hwnd);
        return 0;
    case WM_LBUTTONDOWN:
    {
        SetFocus(hwnd);
        // Mark position for transfer-func window.
        if (nFilterType >= IDM_TFUNC_ILPF && nFilterType < IDM_NMODEL_GAUSSIAN)
        {
            // From window coordinate to image coordinate.
            markerPositions[currMarkerIdx].x = GET_X_LPARAM(lParam) + abs(xImgAnchor);
            markerPositions[currMarkerIdx].y = GET_Y_LPARAM(lParam) + abs(yImgAnchor);

            start_update_marker_center_edit_text:
            {
                UINT tmpIdx;
                WCHAR szTmpNum[20];
                // Update edit text.
                switch (nFilterType)
                {
                case IDM_TFUNC_ILPF:
                case IDM_TFUNC_GLPF:
                case IDM_TFUNC_BLPF:
                case IDM_TFUNC_IHPF:
                case IDM_TFUNC_GHPF:
                case IDM_TFUNC_BHPF:
                case IDM_TFUNC_HOMO:
                {
                    tmpIdx = 2 * currMarkerIdx;
                    xParamVal[tmpIdx] = markerPositions[currMarkerIdx].x - 12;
                    yParamVal[tmpIdx] = markerPositions[currMarkerIdx].y;
                    break;
                }
                default:
                    break;
                }
                wsprintf(szTmpNum, L"%d, %d", xParamVal[tmpIdx], yParamVal[tmpIdx]);
                SetWindowText(edtParam[tmpIdx], szTmpNum);
                // Return to handle mouse move logic after update text.
                if (message == WM_MOUSEMOVE) goto end_update_marker_center_edit_text;
            }
            
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case DOMAIN_GENE_APPLY_BTN:
        {
            switch (nFilterType)
            {
            case IDM_SPAF_BOX: // Note the actual coefficient is different from stored [m].
                myDomainBoxFilter(image, gImage, &gBmpInfo, nBorderMode, 2 * iParamVal[0] - 1);
                break;
            case IDM_SPAF_GAUS: // Note we store [sigma] as double and [blurRadius] as int.
                myDomainGaussianFilter(image, gImage, &gBmpInfo, nBorderMode, dParamVal[0], iParamVal[1]);
                break;
            case IDM_SPAF_MEDI: // Pass through half length directly.
                myDomainMedianFilter(image, gImage, &gBmpInfo, nBorderMode, iParamVal[0]);
                break;
            case IDM_SPAF_LAPLACE: // 2nd derivative, Laplace operator.
                myDomainLaplaceFilter(image, gImage, &gBmpInfo, nBorderMode);
                break;
            case IDM_SPAF_SOBEL: // 1st derivative, Sobel operator.
                myDomainSobelFilter(image, gImage, &gBmpInfo, nBorderMode);
                break;
            case IDM_SPEF_POWER: // Fourier transform, power spectral.
                if (spectral != NULL) free(spectral);
                spectral = myPowerSpectral(image, gImage, &gBmpInfo, dParamVal[0]);
                break;
            case IDM_SPEF_PHASE: // Fourier transform, phase spectral.
                if (spectral != NULL) free(spectral);
                spectral = myPhaseSpectral(image, gImage, &gBmpInfo, dParamVal[0]);
                break;
            case IDM_SPEF_POWER_FFT: // FFT accelerated power spectral.
            {
                if (spectral != NULL) free(spectral);
                spectral = (MyComplex*)malloc(gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyComplex));
                myAssert(spectral);

                // Padding zeros for non-radix-2 FFT.
                UINT RM = myGetRadix2(gBmpInfo.nWidth);
                UINT RN = myGetRadix2(gBmpInfo.nHeight);
                MyBmpInfo tmpInfo;
                MyBGRA* tmpDstImg = myPaddingZeroImageFFT(&tmpInfo, image, &gBmpInfo);
                MyBGRA* tmpSrcImg = myPaddingZeroImageFFT(&tmpInfo, gImage, &gBmpInfo);

                MyComplex* tmpSptl = myPowerSpectralFFT(tmpDstImg, tmpSrcImg, &tmpInfo, RM, RN, dParamVal[0]);

                // Copy from padding buffers.
                for (UINT i = 0; i < gBmpInfo.nWidth; ++i)
                    for (UINT j = 0; j < gBmpInfo.nHeight; ++j)
                    {
                        UINT dstIdx = i + j * gBmpInfo.nWidth;
                        UINT srcIdx = i + j * tmpInfo.nWidth;
                        image[dstIdx] = tmpDstImg[srcIdx];
                        spectral[dstIdx] = tmpSptl[srcIdx];
                    }
                free(tmpDstImg); free(tmpSrcImg); free(tmpSptl);
                break;
            }
            case IDM_SPEF_PHASE_FFT: // FFT accelerated phase spectral.
            {
                if (spectral != NULL) free(spectral);
                spectral = (MyComplex*)malloc(gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyComplex));
                myAssert(spectral);

                // Padding zeros for non-radix-2 FFT.
                UINT RM = myGetRadix2(gBmpInfo.nWidth);
                UINT RN = myGetRadix2(gBmpInfo.nHeight);
                MyBmpInfo tmpInfo;
                MyBGRA* tmpDstImg = myPaddingZeroImageFFT(&tmpInfo, image, &gBmpInfo);
                MyBGRA* tmpSrcImg = myPaddingZeroImageFFT(&tmpInfo, gImage, &gBmpInfo);

                MyComplex* tmpSptl = myPhaseSpectralFFT(tmpDstImg, tmpSrcImg, &tmpInfo, RM, RN, dParamVal[0]);

                // Copy from padding buffers.
                for (UINT i = 0; i < gBmpInfo.nWidth; ++i)
                    for (UINT j = 0; j < gBmpInfo.nHeight; ++j)
                    {
                        UINT dstIdx = i + j * gBmpInfo.nWidth;
                        UINT srcIdx = i + j * tmpInfo.nWidth;
                        image[dstIdx] = tmpDstImg[srcIdx];
                        spectral[dstIdx] = tmpSptl[srcIdx];
                    }
                free(tmpDstImg); free(tmpSrcImg); free(tmpSptl);
                break;
            }
            case IDM_TFUNC_ILPF: // Ideal Low Pass Filter
            case IDM_TFUNC_IHPF: // Ideal High Pass Filter
                if (spectral != NULL)
                {
                    static POINT ilpfCenter[10];
                    static double ilpfRadius[10];
                    static UINT ilpfValidCount;

                    ilpfValidCount = 0;
                    for (int i = 0; i < _countof(ilpfCenter); ++i)
                    {
                        POINT tmpPT = { markerPositions[i].x,
                            // Note image Y-axis is flipped.
                            gBmpInfo.nHeight - markerPositions[i].y };
                        // Count valid ideal low/high pass filters.
                        if (tmpPT.x >= 0 && tmpPT.x < gBmpInfo.nWidth &&
                            tmpPT.y >= 0 && tmpPT.y < gBmpInfo.nHeight)
                        {
                            ilpfCenter[ilpfValidCount] = tmpPT;
                            ilpfRadius[ilpfValidCount] = dParamVal[2 * ilpfValidCount + 1];
                            ++ilpfValidCount; // Prepare for next filter checking.
                        }
                    }
                    // No need to call function for empty filter list.
                    if (ilpfValidCount != 0)
                    {
                        if (nFilterType == IDM_TFUNC_ILPF)
                            myTfuncIdealLPF(spectral, &gBmpInfo, ilpfCenter, ilpfRadius, ilpfValidCount);
                        else
                            myTfuncIdealHPF(spectral, &gBmpInfo, ilpfCenter, ilpfRadius, ilpfValidCount);

                        mycpLogarithmTrans(image, spectral, gBmpInfo.nWidth, gBmpInfo.nHeight, 1);
                    }
                }
                break;
            case IDM_TFUNC_GLPF: // Gaussian Low Pass Filter
            case IDM_TFUNC_GHPF: // Gaussian High Pass Filter
                if (spectral != NULL)
                {
                    static POINT glpfCenter[10];
                    static double glpfSigma[10];
                    static UINT glpfValidCount;

                    glpfValidCount = 0;
                    for (int i = 0; i < _countof(glpfCenter); ++i)
                    {
                        POINT tmpPT = { markerPositions[i].x,
                            // Note image Y-axis is flipped.
                            gBmpInfo.nHeight - markerPositions[i].y };
                        // Count valid ideal low/high pass filters.
                        if (tmpPT.x >= 0 && tmpPT.x < gBmpInfo.nWidth &&
                            tmpPT.y >= 0 && tmpPT.y < gBmpInfo.nHeight)
                        {
                            glpfCenter[glpfValidCount] = tmpPT;
                            glpfSigma[glpfValidCount] = dParamVal[2 * glpfValidCount + 1];
                            ++glpfValidCount; // Prepare for next filter checking.
                        }
                    }
                    // No need to call function for empty filter list.
                    if (glpfValidCount != 0)
                    {
                        if (nFilterType == IDM_TFUNC_GLPF)
                            myTfuncGaussianLPF(spectral, &gBmpInfo, glpfCenter, glpfSigma, glpfValidCount);
                        else
                            myTfuncGaussianHPF(spectral, &gBmpInfo, glpfCenter, glpfSigma, glpfValidCount);

                        mycpLogarithmTrans(image, spectral, gBmpInfo.nWidth, gBmpInfo.nHeight, 1);
                    }
                }
                break;
            case IDM_TFUNC_BLPF: // Butterworth Low Pass Filter
            case IDM_TFUNC_BHPF: // Butterworth High Pass Filter
                if (spectral != NULL)
                {
                    static POINT blpfCenter[10];
                    static double blpfCutoff[10];
                    static UINT blpfValidCount;

                    blpfValidCount = 0;
                    for (int i = 0; i < _countof(blpfCenter); ++i)
                    {
                        POINT tmpPT = { markerPositions[i].x,
                            // Note image Y-axis is flipped.
                            gBmpInfo.nHeight - markerPositions[i].y };
                        // Count valid ideal low/high pass filters.
                        if (tmpPT.x >= 0 && tmpPT.x < gBmpInfo.nWidth &&
                            tmpPT.y >= 0 && tmpPT.y < gBmpInfo.nHeight)
                        {
                            blpfCenter[blpfValidCount] = tmpPT;
                            blpfCutoff[blpfValidCount] = dParamVal[2 * blpfValidCount + 1];
                            ++blpfValidCount; // Prepare for next filter checking.
                        }
                    }
                    // No need to call function for empty filter list.
                    if (blpfValidCount != 0)
                    {
                        if (nFilterType == IDM_TFUNC_BLPF)
                            myTfuncButterworthLPF(spectral, &gBmpInfo, blpfCenter,
                                                  blpfCutoff, dParamVal[10], blpfValidCount);
                        else
                            myTfuncButterworthHPF(spectral, &gBmpInfo, blpfCenter,
                                                  blpfCutoff, dParamVal[10], blpfValidCount);

                        mycpLogarithmTrans(image, spectral, gBmpInfo.nWidth, gBmpInfo.nHeight, 1);
                    }
                }
                break;
            case IDM_TFUNC_HOMO: // Homomorphic Filter
                if (spectral != NULL)
                {
                    static POINT homoCenter[10];
                    static double homoSigma[10];
                    static UINT homoValidCount;

                    homoValidCount = 0;
                    for (int i = 0; i < _countof(homoCenter); ++i)
                    {
                        POINT tmpPT = { markerPositions[i].x,
                            // Note image Y-axis is flipped.
                            gBmpInfo.nHeight - markerPositions[i].y };
                        // Count valid homomorphic filters.
                        if (tmpPT.x >= 0 && tmpPT.x < gBmpInfo.nWidth &&
                            tmpPT.y >= 0 && tmpPT.y < gBmpInfo.nHeight)
                        {
                            homoCenter[homoValidCount] = tmpPT;
                            homoSigma[homoValidCount] = dParamVal[2 * homoValidCount + 1];
                            ++homoValidCount; // Prepare for next filter checking.
                        }
                    }
                    // No need to call function for empty filter list.
                    if (homoValidCount != 0)
                    {
                        myTfuncHomomorphicFilter(spectral, &gBmpInfo, homoCenter, homoSigma,
                                                 dParamVal[8], dParamVal[9], dParamVal[10], homoValidCount);

                        mycpLogarithmTrans(image, spectral, gBmpInfo.nWidth, gBmpInfo.nHeight, 1);
                    }
                }
                break;
            case IDM_NMODEL_GAUSSIAN:
                myApplyGaussianNoise(image, &gBmpInfo, dParamVal[0], dParamVal[1]);
                break;
            case IDM_NMODEL_RAYLEIGH:
                myApplyRayleighNoise(image, &gBmpInfo, dParamVal[0], dParamVal[1]);
                break;
            case IDM_NMODEL_ERLANG:
                myApplyErlangNoise(image, &gBmpInfo, dParamVal[0], iParamVal[1]);
                break;
            case IDM_NMODEL_EXP:
                myApplyExponentialNoise(image, &gBmpInfo, dParamVal[0]);
                break;
            case IDM_NMODEL_UNIFORM:
                myApplyUniformNoise(image, &gBmpInfo, dParamVal[0], dParamVal[1]);
                break;
            case IDM_NMODEL_SANDP:
                myApplySaltAndPepperNoise(image, &gBmpInfo, dParamVal[0], dParamVal[1]);
                break;
            default:
                break;
            }
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case DOMAIN_GENE_SAVE_MAIN_BTN:
        {
            UINT nPixelCount = gBmpInfo.nWidth * gBmpInfo.nHeight;
            memcpy(gImage, image, nPixelCount * sizeof(MyBGRA));
            // Copy spectral by the way (if has).
            if (spectral != NULL)
            {
                if (gSpectral == NULL)
                {
                    gSpectral = (MyComplex*)malloc(nPixelCount * sizeof(MyComplex));
                }
                myAssert(gSpectral);
                memcpy(gSpectral, spectral, nPixelCount * sizeof(MyComplex));
            }
            // Release spectral otherwise.
            else if (gSpectral != NULL) { free(gSpectral); gSpectral = NULL; }

            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            // Repaint main window image.
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(gMainWnd, NULL, FALSE);
            return 0;
        }
        case DOMAIN_GENE_BORDER_MODE_BTN:
        {
            // @see declaration of nBorderMode. We borrow the flags of check-button opportunely.
            switch (nBorderMode = IsDlgButtonChecked(hwnd, DOMAIN_GENE_BORDER_MODE_BTN))
            {
            case MY_SPAF_BM_BLACK:
                SetWindowText(b3sBorderMode, L"边界模式: 黑色");
                break;
            case MY_SPAF_BM_MIRROR:
                SetWindowText(b3sBorderMode, L"边界模式: 镜像");
                break;
            case MY_SPAF_BM_DUPLICATE:
                SetWindowText(b3sBorderMode, L"边界模式: 复制");
                break;
            default:
                break;
            }
            return 0;
        }
        // Max support 20 different key-value pairs.
        case DOMAIN_GENE_PARAM_EDT + 0:
        case DOMAIN_GENE_PARAM_EDT + 1:
        case DOMAIN_GENE_PARAM_EDT + 2:
        case DOMAIN_GENE_PARAM_EDT + 3:
        case DOMAIN_GENE_PARAM_EDT + 4:
        case DOMAIN_GENE_PARAM_EDT + 5:
        case DOMAIN_GENE_PARAM_EDT + 6:
        case DOMAIN_GENE_PARAM_EDT + 7:
        case DOMAIN_GENE_PARAM_EDT + 8:
        case DOMAIN_GENE_PARAM_EDT + 9:
        case DOMAIN_GENE_PARAM_EDT + 10:
        case DOMAIN_GENE_PARAM_EDT + 11:
        case DOMAIN_GENE_PARAM_EDT + 12:
        case DOMAIN_GENE_PARAM_EDT + 13:
        case DOMAIN_GENE_PARAM_EDT + 14:
        case DOMAIN_GENE_PARAM_EDT + 15:
        case DOMAIN_GENE_PARAM_EDT + 16:
        case DOMAIN_GENE_PARAM_EDT + 17:
        case DOMAIN_GENE_PARAM_EDT + 18:
        case DOMAIN_GENE_PARAM_EDT + 19:
        {
            int idxParam = (int)(LOWORD(wParam) - DOMAIN_GENE_PARAM_EDT);

            WCHAR szParam[10];
            GetWindowText(edtParam[idxParam], szParam, 10);

            int iUserInput;
            float fUserInput;
            int xUserInput, yUserInput;
            // Populate parameter with user input.
            if (swscanf(szParam, L"%d", &iUserInput) == 1)
                iParamVal[idxParam] = iUserInput;
            if (swscanf(szParam, L"%f", &fUserInput) == 1)
                dParamVal[idxParam] = (double)fUserInput;
            if (swscanf(szParam, L"%d,%d", &xUserInput, &yUserInput) == 2)
            {
                xParamVal[idxParam] = xUserInput;
                yParamVal[idxParam] = yUserInput;
            }

            switch (nFilterType)
            {
            case IDM_SPAF_BOX:
            {
                if (idxParam == 0) // norm coef, m x m
                {
                    // Make sure in valid range and is an odd number.
                    iParamVal[0] = (iParamVal[0] + 1) / 2;
                    iParamVal[0] = min(50, max(1, iParamVal[0]));
                }
                return 0;
            }
            case IDM_SPAF_GAUS:
            {
                if (idxParam == 0) // sigma
                {
                    // Make sure standard deviation not less than 1.
                    dParamVal[0] = max(1, dParamVal[0]);
                }
                else if (idxParam == 1) // blur radius
                {
                    // Make sure blur radius is not less than 1.
                    iParamVal[1] = max(1, iParamVal[1]);
                }
                return 0;
            }
            case IDM_SPAF_MEDI:
            {
                if (idxParam == 0) // half length
                {
                    // Make sure half length is not less than 1.
                    iParamVal[0] = max(1, iParamVal[0]);
                }
                return 0;
            }
            case IDM_SPEF_POWER:
            case IDM_SPEF_PHASE:
            {
                if (idxParam == 0) // clog(1 + r), c
                {
                    // Make sure dLogParam is not negative.
                    dParamVal[0] = max(0, dParamVal[0]);
                }
                return 0;
            }
            case IDM_TFUNC_ILPF:
            case IDM_TFUNC_GLPF:
            case IDM_TFUNC_BLPF:
            case IDM_TFUNC_IHPF:
            case IDM_TFUNC_GHPF:
            case IDM_TFUNC_BHPF:
            case IDM_TFUNC_HOMO:
            {
                if (idxParam % 2 == 0)
                {
                    if (idxParam == 10 && (nFilterType == IDM_TFUNC_BLPF || nFilterType == IDM_TFUNC_BHPF))
                    {
                        // Make sure Butterworth order is not less than 0.001.
                        dParamVal[idxParam] = max(1e-3, dParamVal[idxParam]);
                    }
                    else if (idxParam >= 8 && idxParam <= 10 && (nFilterType == IDM_TFUNC_HOMO))
                    {
                        if (idxParam == 8) // slope, c [not less than 0.001]
                            dParamVal[idxParam] = max(1e-3, dParamVal[idxParam]);
                        else if (idxParam == 9) // gamma low [not larger than gamma high]
                            dParamVal[idxParam] = max(0, min(dParamVal[idxParam + 1], dParamVal[idxParam]));
                        else // idxParam == 10 --> gamma high [not less than gamma low]
                            dParamVal[idxParam] = max(dParamVal[idxParam - 1], dParamVal[idxParam]);
                    }
                    else
                    {
                        UINT tmpIdx = idxParam / 2;
                        // Cursor center position.
                        markerPositions[tmpIdx].x = xParamVal[idxParam];
                        markerPositions[tmpIdx].y = yParamVal[idxParam];
                    }
                }
                else // Low/high pass core radius/sigma/cutoff.
                {
                    // Make sure LPF/HPF radius/sigma/cutoff is not less than 1.
                    dParamVal[idxParam] = max(1, dParamVal[idxParam]);
                }
                // There's no need to erase since double-buffer is enabled.
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            case IDM_NMODEL_GAUSSIAN:
            {
                return 0;
            }
            case IDM_NMODEL_RAYLEIGH:
            {
                if (idxParam == 1) // parameter b
                {
                    // Make sure b > 0
                    dParamVal[1] = max(1e-3, dParamVal[1]);
                }
                return 0;
            }
            case IDM_NMODEL_ERLANG:
            {
                if (idxParam == 0) // parameter a
                {
                    double maxA = iParamVal[1] - 1e-3;
                    // Make sure 0 < a < b.
                    dParamVal[0] = max(1e-3, min(maxA, dParamVal[0]));
                }
                else if (idxParam == 1) // parameter b
                {
                    // Make sure b is a positive integer.
                    iParamVal[1] = max(1, iParamVal[1]);
                }
                return 0;
            }
            case IDM_NMODEL_EXP:
            {
                if (idxParam == 0) // parameter a
                {
                    // Make sure 0 < a < 1.
                    dParamVal[0] = max(1e-3, min(1-1e-3, dParamVal[0]));
                }
                return 0;
            }
            case IDM_NMODEL_UNIFORM:
            {
                if (idxParam == 0) // start point
                {
                    // Make sure start <= end.
                    dParamVal[0] = min(dParamVal[1], dParamVal[0]);
                }
                else if (idxParam == 1) // end point
                {
                    // Make sure end >= start.
                    dParamVal[1] = max(dParamVal[0], dParamVal[1]);
                }
                return 0;
            } 
            case IDM_NMODEL_SANDP:
            {
                if (idxParam == 0) // white probability
                {
                    double maxWhite = 1 - dParamVal[1];
                    // Make sure total probability not greater than 1.
                    dParamVal[0] = min(maxWhite, max(0, dParamVal[0]));
                }
                else if (idxParam == 1) // black probability
                {
                    double maxBlack = 1 - dParamVal[0];
                    // Make sure total probability not greater than 1.
                    dParamVal[1] = min(maxBlack, max(0, dParamVal[1]));
                }
                return 0;
            }
            default:
                return 0;
            }
        }
        default:
            return 0;
        }
    }
    case WM_MOUSEWHEEL:
    {
        double tmpDelta = (GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA);

        switch (nFilterType)
        {
        case IDM_TFUNC_ILPF:
        case IDM_TFUNC_GLPF:
        case IDM_TFUNC_BLPF:
        case IDM_TFUNC_IHPF:
        case IDM_TFUNC_GHPF:
        case IDM_TFUNC_BHPF:
        case IDM_TFUNC_HOMO:
        {
            UINT tmpIdx = 2 * currMarkerIdx + 1;

            dParamVal[tmpIdx] += ((LOWORD(wParam) & MK_SHIFT) ? 1 : 10) * tmpDelta;
            // Make sure core radius/sigma is not less than 1.
            dParamVal[tmpIdx] = max(1, dParamVal[tmpIdx]);

            WCHAR szTmp[20];
            // Update radius edit text.
            wsprintf(szTmp, L"%d", (int)dParamVal[tmpIdx]);
            SetWindowText(edtParam[tmpIdx], szTmp);
        }
        default:
            break;
        }
        // There's no need to erase since double-buffer is enabled.
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_DESTROY:
        // Not we have created custom pens for transfer-func window.
        if (nFilterType >= IDM_TFUNC_ILPF && nFilterType < IDM_NMODEL_GAUSSIAN)
        {
            for (int i = 0; i < _countof(markerPens); ++i)
            {
                DeleteObject(markerPens[i]);
            }
        }
        // Restore config variables.
        currMarkerIdx = 0;
        nBorderMode = MY_SPAF_BM_MIRROR;
        // We might have allocated memory for spectral data.
        if (spectral != NULL) { free(spectral); spectral = NULL; }
        // Note we have allocated memory for image data.
        free(image);
        image = NULL;
        // If this is the last child window to destroy.
        if (myValidWndCount() == 2)
        {
            EnableMenuItem(gMenu, IDM_FILE, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        }
        // Make the menu selectable again since the window is closed.
        EnableMenuItem(gMenu, IDM_DOMAIN, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_TRANSFER_FUNC, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_NOISE_MODEL, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        DrawMenuBar(gMainWnd); // Force the main window to repaint the menu.
        SetFocus(gMainWnd); // Move focus to main window in case of accident hide.
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                Domain Custom Core Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

LRESULT CALLBACK DomainCustCoreWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;

    switch (message)
    {
    case WM_CREATE:
        return 0;

    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        // Paint

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                         App About Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

LRESULT CALLBACK AppAboutWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBmpInfo appIconInfo = { 32, 32 };

    static WCHAR szAppVersion[] = L"BitmapViewer 位图工具 v2.2";
    static WCHAR szAuthorCopy[] = L"Copyleft (cl) 2023 文亦尧";

    switch (message)
    {
    case WM_CREATE:
        // Distinguish app about window from main window in background.
        //SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(LTGRAY_BRUSH));
        return 0;

    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;

    case WM_PAINT:
    {
        hdc = BeginPaint(hwnd, &ps);

        // Draw popup window frame.
        Rectangle(hdc, 0, 0, 256, 10);
        Rectangle(hdc, 0, 0, 10, 256);
        Rectangle(hdc, 246, 0, 256, 256);
        Rectangle(hdc, 0, 246, 256, 256);

        /*
        * We have to re-write display codes here again (but change biHeight to a negative),
        * since the app icon is flipped to suit window creation (so we flip it again here).
        */

        BITMAPINFOHEADER bmpih;
        bmpih.biSize = sizeof(BITMAPINFOHEADER);
        bmpih.biWidth = 32;
        bmpih.biHeight = -32;
        bmpih.biPlanes = 1; // always be 1
        bmpih.biBitCount = 32; // true color
        bmpih.biCompression = BI_RGB; // no compression
        bmpih.biSizeImage = 0; // could be 0 for RGB
        // We don't care about these fields.
        bmpih.biXPelsPerMeter = bmpih.biYPelsPerMeter = 0;
        bmpih.biClrUsed = bmpih.biClrImportant = 0;

        SetDIBitsToDevice(
            hdc,
            112, // x destination coordinate
            48, // y destination coordinate
            32, // source rectangle width
            32, // source rectangle height
            0, // x source coordinate
            0, // y source coordinate
            0, // first scan line to draw
            32, // number of scan lines to draw
            gAppIconData, // pointer to DIB pixel bits
            (BITMAPINFO*)&bmpih, // pointer to DIB information
            DIB_RGB_COLORS); // color use flag

        SIZE tmpSize;
        GetTextExtentExPoint(hdc, szAppVersion, _countof(szAppVersion) - 1, INT_MAX, NULL, NULL, &tmpSize);
        TextOut(hdc, (256 - tmpSize.cx) / 2, 150, szAppVersion, _countof(szAppVersion) - 1);

        GetTextExtentExPoint(hdc, szAuthorCopy, _countof(szAuthorCopy) - 1, INT_MAX, NULL, NULL, &tmpSize);
        TextOut(hdc, (256 - tmpSize.cx) / 2, 150 + 2 * gCharHeight, szAuthorCopy, _countof(szAuthorCopy) - 1);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                         Threshold Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// THW: Threshold Window - 106
// Its child window should start from 106_01.
#define THW_THRESHOLD_LBL 10601
#define THW_THRESHOLD_EDT 10602
#define THW_THRESHOLD_BTN 10603
#define THW_SAVE_TO_MAIN_BTN 10604

LRESULT CALLBACK ThresholdWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;

    static HWND lblThreshold;
    static HWND edtThreshold;
    static HWND btnThreshold;

    static UINT8 nThresValue = 128;

    static HWND btnSaveToMainBtn;

    // Available display area for image.
    static int validWidth = 0;
    static int validHeight = 0;

    static bCsorInValidArea = FALSE;

    // Image display offset.
    static int xImgAnchor = 0, yImgAnchor = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        // We decide that user can only open one window with the same type at the same time.
        EnableMenuItem(gMenu, IDM_FILE, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_GRAY, MF_GRAYED | MF_BYPOSITION);

        // Copy image data from main window.
        size_t nByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
        image = (MyBGRA*)malloc(nByteSize);
        myAssert(image);
        memcpy(image, gImage, nByteSize);

        // Create threshold widgets.
        lblThreshold = CreateWindow(
            L"static",
            L"阈值",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            12,
            12 + gCharHeight / 2,
            5 * gCharWidth,
            gCharHeight,
            hwnd,
            (HMENU)THW_THRESHOLD_LBL,
            gInstance,
            NULL);
        edtThreshold = CreateWindow(
            L"edit",
            L"128",
            WS_CHILD | WS_VISIBLE,
            18 + 5 * gCharWidth,
            12 + gCharHeight / 2,
            3 * gCharWidth + 2,
            gCharHeight,
            hwnd,
            (HMENU)THW_THRESHOLD_EDT,
            gInstance,
            NULL);
        btnThreshold = CreateWindow(
            L"button",
            L"更新结果",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            26 + 8 * gCharWidth,
            12,
            10 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)THW_THRESHOLD_BTN,
            gInstance,
            NULL);
        // Create save-to-main button.
        btnSaveToMainBtn = CreateWindow(
            L"button",
            L"保存至主窗口",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            32 + 18 * gCharWidth,
            12,
            14 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)THW_SAVE_TO_MAIN_BTN,
            gInstance,
            NULL);

        // Initialize helper variables.
        validWidth = gBmpInfo.nWidth;
        validHeight = gBmpInfo.nHeight;

        return 0;
    }
    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int clntWidth = rc.right - rc.left;
        int clntHeight = rc.bottom - rc.top;

        // Recalculate available image area.
        validWidth = rc.right - rc.left - 24;
        validHeight = rc.bottom - rc.top - 36 - 2 * gCharHeight;
        // Clamp to origin image size when window is larger.
        validWidth = min(validWidth, gBmpInfo.nWidth);
        validHeight = min(validHeight, gBmpInfo.nHeight);

        // Always reset image anchor when resized.
        xImgAnchor = yImgAnchor = 0;

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_SETCURSOR:
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);
        RECT rc;
        GetClientRect(hwnd, &rc);

        int tmpImgTop = 24 + 2 * gCharHeight;

        // Get valid area boundary.
        int tmpWndRightMost = rc.right - 12;
        int tmpImgRightMost = 12 + gBmpInfo.nWidth;
        int tmpWndBottomMost = rc.bottom - 12;
        int tmpImgBottomMost = tmpImgTop + gBmpInfo.nHeight;

        // Check whether above the image.
        if (pt.x >= 12 && pt.x < min(tmpImgRightMost, tmpWndRightMost) &&
            pt.y >= tmpImgTop && pt.y < min(tmpImgBottomMost, tmpWndBottomMost))
        {
            if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
                // Hold down right button to start move.
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
            else
                // Show mark cursor otherwise.
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));

            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            bCsorInValidArea = TRUE; // cursor in workspace

            return 0;
        }

        // There's no need to erase since double-buffer is enabled.
        if (bCsorInValidArea) InvalidateRect(hwnd, NULL, FALSE);
        bCsorInValidArea = FALSE; // cursor out of workspace

        break; // Use default cursor in other places.
    }
    case WM_MOUSEMOVE:
    {
        static int csorCurrX, csorCurrY;
        static int csorLastX, csorLastY;

        csorCurrX = GET_X_LPARAM(lParam);
        csorCurrY = GET_Y_LPARAM(lParam);

        if (bCsorInValidArea) // in workspace
        {
            // Trigger move mode.
            if (wParam & MK_RBUTTON)
            {
                // Note +/- to match image coordinate.
                xImgAnchor -= (csorCurrX - csorLastX);
                yImgAnchor += (csorCurrY - csorLastY);

                // Clamp image anchor offset into valid range.
                // X offset: 0 ~ (ImgW - ValidW)
                int xOffsetMost = gBmpInfo.nWidth - validWidth;
                xImgAnchor = max(0, min(xOffsetMost, xImgAnchor));
                // Y offset: (ValidH - ImgH) ~ 0
                int yOffsetMost = validHeight - gBmpInfo.nHeight;
                yImgAnchor = max(yOffsetMost, min(0, yImgAnchor));
            }
            // We have notified to repaint in WM_SETCURSOR.
        }
        csorLastX = csorCurrX;
        csorLastY = csorCurrY;
        return 0;
    }
    case WM_PAINT:
    {
        hdc = BeginPaint(hwnd, &ps);

        RECT rcClnt;
        GetClientRect(hwnd, &rcClnt);
        //--------------------------------------------------------------------- Double Buffer Start
        HDC hMemDC = CreateCompatibleDC(hdc); // Acquire copy device context.
        HBITMAP hMemBmp = CreateCompatibleBitmap( // Draw on temporary bitmap.
            hdc, rcClnt.right - rcClnt.left, rcClnt.bottom - rcClnt.top);
        SelectObject(hMemDC, hMemBmp);
        // Erase with master window's bkgn brush.
        FillRect(hMemDC, &rcClnt, (HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND));
        //---------------------------------------------------------------------

        // We have to do this to stick the image at left top,
        // since Y-axis is flipped in origin pixel buffer.
        int tmpImgY = validHeight - gBmpInfo.nHeight;

        // Display image from left top corner of main window.
        myDisplayImage(hMemDC, 12, 24 + 2 * gCharHeight, max(0, validWidth), max(0, validHeight),
                       xImgAnchor, abs(min(0, tmpImgY)) + yImgAnchor, image, &gBmpInfo);

        //--------------------------------------------------------------------- Double Buffer End
        BitBlt(hdc, 0, 0, rcClnt.right - rcClnt.left,
               rcClnt.bottom - rcClnt.top, hMemDC, 0, 0, SRCCOPY);
        DeleteDC(hMemDC);
        DeleteObject(hMemBmp);
        //---------------------------------------------------------------------

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case THW_THRESHOLD_EDT:
        {
            WCHAR szThresParam[10];
            GetWindowText(edtThreshold, szThresParam, 10);
            int iUserInput = 1;
            if (swscanf(szThresParam, L"%i", &iUserInput) == 1)
                nThresValue = (UINT8)iUserInput;
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case THW_THRESHOLD_BTN:
        {
            myThresholdProcess(image, gImage, &gBmpInfo, nThresValue);
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case THW_SAVE_TO_MAIN_BTN:
        {
            // Copy image data to main window.
            memcpy(gImage, image, gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA));
            // Discard spectral after processed.
            if (gSpectral != NULL) { free(gSpectral); gSpectral = NULL; }

            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            // Repaint main window image.
            // There's no need to erase since double-buffer is enabled.
            InvalidateRect(gMainWnd, NULL, FALSE);
            return 0;
        }
        default:
            return 0;
        }
    }
    case WM_DESTROY:
        // Note we have allocated memory for image data.
        free(image);
        image = NULL;
        // If this is the last child window to destroy.
        if (myValidWndCount() == 2)
        {
            EnableMenuItem(gMenu, IDM_FILE, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        }
        // Make the menu selectable again since the window is closed.
        EnableMenuItem(gMenu, IDM_GRAY, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        DrawMenuBar(gMainWnd); // Force the main window to repaint the menu.
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
// Digital Image Processing Subroutine & Miscellaneous Function Definition
//-------------------------------------------------------------------------------------------------

// We place myLoadIcon at the end of this file since it looks too long.

HMENU myLoadMainMenu()
{
    const WORD MENU_TEMPLATE[] =
    {
        0, // versionNumber: must be zero.
        0, // offset: distance to first MENUITEMTEMPLATE.

// top level
#define OPT_TOPLEV MF_POPUP | MF_STRING
#define OPT_TOPEND OPT_TOPLEV | MF_END
// middle
#define OPT_MIDDLE MF_INSERT | MF_STRING
#define OPT_MIDEND OPT_MIDDLE | MF_END
// separator
#define OPT_______ MF_SEPARATOR, 0, 0
#define OPT____END MF_SEPARATOR | MF_END, 0, 0
// accelerator
#define MY_ACCEL_CTRL L'\t', L'C', L't', L'r', L'l', L'+'
#define MY_ACCEL_CTRL_SHIFT L'\t', L'C', L't', L'r', L'l', L'+', L'S', L'h', L'i', L'f', L't', L'+'

        // Option   ID                      String
        OPT_TOPLEV,                         L'文', L'件', 0,
        OPT_MIDDLE, IDM_FILE_NEW_256x256,   L'新', L'建', L'2', L'5', L'6', L'x', L'2', L'5', L'6', 0,
        OPT_MIDDLE, IDM_FILE_NEW_512x512,   L'新', L'建', L'5', L'1', L'2', L'x', L'5', L'1', L'2', 0,
        OPT_MIDDLE, IDM_FILE_NEW_1024x1024, L'新', L'建', L'1', L'0', L'2', L'4', L'x', L'1', L'0', L'2', L'4', 0,
        OPT_______,
        OPT_MIDDLE, IDM_FILE_OPEN,          L'打', L'开', MY_ACCEL_CTRL, L'O', 0,
        OPT_______,
        OPT_MIDDLE, IDM_FILE_SECOND,        L'第', L'二', L'窗', L'口', 0,
        OPT_______,
        OPT_MIDDLE, IDM_FILE_SAVE,          L'保', L'存', MY_ACCEL_CTRL, L'S', 0,
        OPT_MIDDLE, IDM_FILE_SAVE_AS,       L'另', L'存', L'为', MY_ACCEL_CTRL_SHIFT, L'S', 0,
        OPT_______,
        OPT_MIDEND, IDM_FILE_EXP_TXT,       L'导', L'出', L'T', L'X', L'T', 0,

        OPT_TOPLEV,                         L'灰', L'度', L'变', L'换', 0,
        OPT_MIDDLE, IDM_GRAY_EMPI,          L'经', L'验', L'公', L'式', 0,
        OPT_MIDDLE, IDM_GRAY_EVEN,          L'算', L'数', L'平', L'均', 0,
        OPT_MIDDLE, IDM_GRAY_GAMMA,         L'伽', L'马', L'校', L'正', 0,
        OPT_______,
        OPT_MIDDLE, IDM_GRAY_R,             L'R', L'通', L'道', 0,
        OPT_MIDDLE, IDM_GRAY_G,             L'G', L'通', L'道', 0,
        OPT_MIDDLE, IDM_GRAY_B,             L'B', L'通', L'道', 0,
        OPT_______,
        OPT_MIDDLE, IDM_GRAY_2xBLACK,       L'2', L'x', L'黑', L'色', L'填', L'充', 0,
        OPT_MIDDLE, IDM_GRAY_2xMIRROR,      L'2', L'x', L'镜', L'像', L'填', L'充', 0,
        OPT_MIDDLE, IDM_GRAY_2xCOPY,        L'2', L'x', L'复', L'制', L'填', L'充', 0,
        OPT_______,
        OPT_MIDEND, IDM_GRAY_THRESHOLD,     L'阈', L'值', L'处', L'理', 0,

        OPT_TOPLEV,                         L'卷', L'积', L'滤', L'波', 0,
        OPT_MIDDLE, IDM_SPAF_BOX,           L'空', L'域', L'_', L'盒', L'式', L'平', L'均', 0,
        OPT_MIDDLE, IDM_SPAF_GAUS,          L'空', L'域', L'_', L'低', L'通', L'高', L'斯', 0,
        OPT_MIDDLE, IDM_SPAF_MEDI,          L'空', L'域', L'_', L'中', L'值', L'降', L'噪', 0,
        OPT_MIDDLE, IDM_SPAF_LAPLACE,       L'空', L'域', L'_', L'拉', L'普', L'拉', L'斯', 0,
        OPT_MIDDLE, IDM_SPAF_SOBEL,         L'空', L'域', L'_', L'索', L'贝', L'尔', 0,
        OPT_______,
        OPT_MIDDLE, IDM_SPAF_CUST,          L'空', L'域', L'-', L'自', L'定', L'义', L'核', 0,
        OPT_______,
        OPT_MIDDLE, IDM_SPEF_POWER,         L'频', L'域', L'_', L'功', L'率', L'谱', 0,
        OPT_MIDDLE, IDM_SPEF_PHASE,         L'频', L'域', L'_', L'相', L'位', L'谱', 0,
        OPT_______,
        OPT_MIDDLE, IDM_SPEF_POWER_FFT,     L'频', L'域', L'_', L'功', L'率', L'谱', L'_', L'F', L'F', L'T', 0,
        OPT_MIDDLE, IDM_SPEF_PHASE_FFT,     L'频', L'域', L'_', L'相', L'位', L'谱', L'_', L'F', L'F', L'T', 0,
        OPT_______,
        OPT_MIDEND, IDM_SPAF_CUST,          L'频', L'域', L'-', L'自', L'定', L'义', L'核', 0,

        OPT_TOPLEV,                         L'传', L'递', L'函', L'数', 0,
        OPT_MIDDLE, IDM_TFUNC_ILPF,         L'理', L'想', L'_', L'低', L'通', L'/', L'陷', L'波', 0,
        OPT_MIDDLE, IDM_TFUNC_GLPF,         L'高', L'斯', L'_', L'低', L'通', L'/', L'陷', L'波', 0,
        OPT_MIDDLE, IDM_TFUNC_BLPF,         L'巴', L'特', L'沃', L'斯', L'_', L'低', L'通', L'/', L'陷', L'波', 0,
        OPT_______,
        OPT_MIDDLE, IDM_TFUNC_IHPF,         L'理', L'想', L'_', L'高', L'通', L'/', L'提', L'升', 0,
        OPT_MIDDLE, IDM_TFUNC_GHPF,         L'高', L'斯', L'_', L'高', L'通', L'/', L'提', L'升', 0,
        OPT_MIDDLE, IDM_TFUNC_BHPF,         L'巴', L'特', L'沃', L'斯', L'_', L'高', L'通', L'/', L'提', L'升', 0,
        OPT_______,
        OPT_MIDEND, IDM_TFUNC_HOMO,         L'同', L'态', L'滤', L'波', L'_', L'(', L'高', L'斯', L'拟', L'合', L')', 0,

        OPT_TOPLEV,                         L'噪', L'声', L'模', L'型', 0,
        OPT_MIDDLE, IDM_NMODEL_GAUSSIAN,    L'高', L'斯', L'分', L'布', 0,
        OPT_MIDDLE, IDM_NMODEL_RAYLEIGH,    L'瑞', L'利', L'分', L'布', 0,
        OPT_MIDDLE, IDM_NMODEL_ERLANG,      L'爱', L'尔', L'兰', L'拟', L'合', 0,
        OPT_MIDDLE, IDM_NMODEL_EXP,         L'指', L'数', L'分', L'布', 0,
        OPT_MIDDLE, IDM_NMODEL_UNIFORM,     L'均', L'匀', L'分', L'布', 0,
        OPT_MIDEND, IDM_NMODEL_SANDP,       L'椒', L'盐', L'冲', L'激', 0,

#define MY_EASTER_EGG_TEXT \
L'~', L'%', L'?', L'…', L',', L'#', L' ', L'*', L'\'', L'☆', L'&', L'℃', L'$', L'︿', L'★', L'?'

        OPT_TOPEND,                         L'帮', L'助', 0,
        OPT_MIDDLE, IDM_EASTER_EGG,         MY_EASTER_EGG_TEXT, 0,
        OPT_______,
        OPT_MIDDLE, IDM_APP_ABOUT,          L'关', L'于', L'项', L'目', 0,
        OPT_MIDEND, IDM_WEB_HOME,           L'使', L'用', L'手', L'册', 0,

#undef MY_EASTER_EGG_TEXT

#undef OPT_______
#undef OPT_MIDDLE
#undef OPT_TOPLEV
    };
    return LoadMenuIndirect(MENU_TEMPLATE);
}

HACCEL myLoadMainAccel()
{
    // Note 'O', 'S' etc. are belong to FVIRTKEY, since the value of so called VK_O, VK_S
    // are exactly the same with ASCII codes. For normal char key, FCONTROL wouldn't work.
    const ACCEL ACCEL_TEMPLATE[] =
    {
        //  BYTE: fVirt                     WORD: key           WORD: cmd
        {   FVIRTKEY | FCONTROL,            'O',                IDM_FILE_OPEN },
        {   FVIRTKEY | FCONTROL,            'S',                IDM_FILE_SAVE },
        {   FVIRTKEY | FCONTROL | FSHIFT,   'S',                IDM_FILE_SAVE_AS },
        // Easter Egg !!!
        {   FVIRTKEY,                       VK_CAPITAL,         IDM_EASTER_EGG }
    };
    return CreateAcceleratorTable((LPACCEL)&ACCEL_TEMPLATE, _countof(ACCEL_TEMPLATE));
}

UINT myValidWndCount()
{
    UINT cnt = 0;

#define MY_CNT_WND(Wnd_Handle) if (IsWindow(Wnd_Handle)) ++cnt;

    // legacy
    MY_CNT_WND(gMainWnd)
    MY_CNT_WND(gGrayTransWnd)
    MY_CNT_WND(gGtwHistDispWnd)
    MY_CNT_WND(gDomainFilterWnd)
    MY_CNT_WND(gDomainCustCoreWnd)
    // v2.0
    MY_CNT_WND(gThresholdWnd)

#undef MY_CNT_WND

    return cnt;
}

RECT myGetSecondWndInitSize()
{
    RECT rc = { 0, 0,
        ((gImage == NULL) ? 256 : gBmpInfo.nWidth) + 48 + 32 * gCharWidth,
        ((gImage == NULL) ? 256 : gBmpInfo.nHeight) + 2 * gCharHeight + 12 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

void myLogarithmTrans(MyBGRA* pData, MyBmpInfo* pInfo, double c)
{
    UINT i, j, idx; // Declare common index variables here.

    // Allocate an intermidiate buffer to normalize later.
    double* buffer = (double*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(double));
    myAssert(buffer);

    double tmpMin = 255, tmpMax = 0;
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            buffer[idx] = c * log(1 + (double)pData[idx].R);
            // Update min, max values.
            tmpMin = min(tmpMin, buffer[idx]);
            tmpMax = max(tmpMax, buffer[idx]);
        }

    tmpMax -= tmpMin;
    // Normalize to 0 ~ 255.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            buffer[idx] = 255 * ((buffer[idx] - tmpMin) / tmpMax);
            pData[idx].R = pData[idx].G = pData[idx].B = (UINT8)min(255, max(0, buffer[idx]));
        }

    // Note we have allocated memory for buffer data.
    free(buffer);
}

void myExpGammaTrans(MyBGRA* pData, MyBmpInfo* pInfo, double gamma)
{
    UINT i, j, idx; // Declare common index variables here.

    // Allocate an intermidiate buffer to normalize later.
    double* buffer = (double*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(double));
    myAssert(buffer);

    double tmpMin = 255, tmpMax = 0;
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            buffer[idx] = pow(pData[idx].R, gamma);
            // Update min, max values.
            tmpMin = min(tmpMin, buffer[idx]);
            tmpMax = max(tmpMax, buffer[idx]);
        }

    tmpMax -= tmpMin;
    // Normalize to 0 ~ 255.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            buffer[idx] = 255 * ((buffer[idx] - tmpMin) / tmpMax);
            pData[idx].R = pData[idx].G = pData[idx].B = (UINT8)min(255, max(0, buffer[idx]));
        }

    // Note we have allocated memory for buffer data.
    free(buffer);
}

void myEnableOperationMenus()
{
    // General file operations.
    EnableMenuItem(gMenu, IDM_FILE_SAVE, MF_ENABLED | MF_HILITE | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_FILE_SAVE_AS, MF_ENABLED | MF_HILITE | MF_BYCOMMAND);

    // Image process operations.
    EnableMenuItem(gMenu, IDM_GRAY, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_DOMAIN, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_TRANSFER_FUNC, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_NOISE_MODEL, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
}

void myDisableOperationMenus()
{
    // General file operations.
    EnableMenuItem(gMenu, IDM_FILE_SAVE, MF_GRAYED | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_FILE_SAVE_AS, MF_GRAYED | MF_BYCOMMAND);

    // Image process operations.
    EnableMenuItem(gMenu, IDM_GRAY, MF_GRAYED | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_DOMAIN, MF_GRAYED | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_TRANSFER_FUNC, MF_GRAYED | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_NOISE_MODEL, MF_GRAYED | MF_BYPOSITION);
}

void myInitFileDialogInfo(HWND hwnd)
{
    static WCHAR szFilter[] = L"位图文件 (*.bmp)\0*.bmp\0";
    OPENFILENAME* ofn = &gOpenFileName;
    ofn->lStructSize = sizeof(OPENFILENAME);
    ofn->hwndOwner = hwnd;
    ofn->hInstance = NULL;
    ofn->lpstrFilter = szFilter;
    ofn->lpstrCustomFilter = NULL;
    ofn->nMaxCustFilter = 0;
    ofn->nFilterIndex = 0;
    ofn->lpstrFile = NULL; // Set in Open & Close func.
    ofn->nMaxFile = MAX_PATH;
    ofn->lpstrFileTitle = NULL; // Set in Open & Close func.
    ofn->nMaxFileTitle = MAX_PATH;
    ofn->lpstrInitialDir = NULL;
    ofn->lpstrTitle = NULL;
    ofn->Flags = 0; // Set in Open & Close func.
    ofn->nFileExtension = 0;
    ofn->lpstrDefExt = L"bmp";
    ofn->lCustData = 0;
    ofn->lpfnHook = NULL;
    ofn->lpTemplateName = NULL;
}

BOOL myOpenFileDialog(HWND hwnd, PWSTR szFileName, PWSTR szTitleName)
{
    OPENFILENAME* ofn = &gOpenFileName;
    ofn->hwndOwner = hwnd;
    ofn->lpstrFile = szFileName;
    ofn->lpstrFileTitle = szTitleName;
    ofn->Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

    return GetOpenFileName(ofn);
}

BOOL mySaveFileDialog(HWND hwnd, PWSTR szFileName, PWSTR szTitleName)
{
    OPENFILENAME* ofn = &gOpenFileName;
    ofn->hwndOwner = hwnd;
    ofn->lpstrFile = szFileName;
    ofn->lpstrFileTitle = szTitleName;
    ofn->Flags = OFN_OVERWRITEPROMPT;

    return GetSaveFileName(ofn);
}

BOOL myIsImageGrayScale(MyBGRA* pData, MyBmpInfo* pInfo)
{
    for (UINT i = 0; i < pInfo->nWidth; ++i)
        for (UINT j = 0; j < pInfo->nHeight; ++j)
        {
            UINT idx = i + j * pInfo->nWidth;
            MyBGRA* pixel = pData + idx;
            if (!(pixel->R == pixel->G &&
                pixel->R == pixel->B)) return FALSE;
        }
    return TRUE;
}

void myDisplayImage(HDC hdc, int offsetX, int offsetY, UINT cxValid, UINT cyValid,
                    int imgX, int imgY, MyBGRA* pData, MyBmpInfo* pInfo)
{
    // We only handle 32-bit true color (no color table), so INFOHEADER is enough.
    BITMAPINFOHEADER bmpih;
    bmpih.biSize = sizeof(BITMAPINFOHEADER);
    bmpih.biWidth = pInfo->nWidth;
    bmpih.biHeight = pInfo->nHeight;
    bmpih.biPlanes = 1; // always be 1
    bmpih.biBitCount = 32; // true color
    bmpih.biCompression = BI_RGB; // no compression
    bmpih.biSizeImage = 0; // could be 0 for RGB
    // We don't care about these fields.
    bmpih.biXPelsPerMeter = bmpih.biYPelsPerMeter = 0;
    bmpih.biClrUsed = bmpih.biClrImportant = 0;

    SetDIBitsToDevice(
        hdc,
        offsetX, // x destination coordinate
        offsetY, // y destination coordinate
        min(cxValid, pInfo->nWidth), // source rectangle width
        min(cyValid, pInfo->nHeight), // source rectangle height
        imgX, // x source coordinate
        imgY, // y source coordinate
        0, // first scan line to draw
        pInfo->nHeight, // number of scan lines to draw
        pData, // pointer to DIB pixel bits
        (BITMAPINFO*)&bmpih, // pointer to DIB information
        DIB_RGB_COLORS); // color use flag
}

RECT myGetGrayTransWndInitSize()
{
    RECT rc = { 0, 0, gBmpInfo.nWidth + 24, gBmpInfo.nHeight + 2 * gCharHeight + 12 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                             Gray Value Extractor |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

#define MY_BEGIN_EXTRACT_GRAY \
    for (UINT i = 0; i < pInfo->nWidth; ++i) \
    for (UINT j = 0; j < pInfo->nHeight; ++j) \
    { \
        UINT idx = i + j * pInfo->nWidth;
// Add custom gray extractor here.
#define MY_END_EXTRACT_GRAY \
        pDst[idx].A = 255; \
    } // Keep opaque by default.

void myExtractGrayEmpi(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo)
{
    MY_BEGIN_EXTRACT_GRAY

        // Empirical Formula:
        // Gray = 0.299 * R + 0.587 * G + 0.114 * B
        // ==> Gray = (299 * R + 587 * G + 114 * B) / 1000
        // ==> Gray = (30 * R + 59 * G + 11 * B) / 100
        // Take binary operation into consideration:
        // ==> Gray = (19595 * R + 38469 * G + 7472 * B) / 65536
        // ==> Gray = (19595 * R + 38469 * G + 7472 * B) >> 16
        // ==> Gray = (38 * R + 75 * G + 15 * B) >> 7, (the one we use here).

        pDst[idx].R = pDst[idx].G = pDst[idx].B =
        
            (38 * pSrc[idx].R + 75 * pSrc[idx].G + 15 * pSrc[idx].B) >> 7;

    MY_END_EXTRACT_GRAY
}

void myExtractGrayEven(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo)
{
    MY_BEGIN_EXTRACT_GRAY

        pDst[idx].R = pDst[idx].G = pDst[idx].B =

            (pSrc[idx].R + pSrc[idx].G + pSrc[idx].B) / 3;

    MY_END_EXTRACT_GRAY
}

void myExtractGrayGamma(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo)
{
    MY_BEGIN_EXTRACT_GRAY

        // Take physics power into consideration (Gamma = 2.2):
        //             ___________________________________________
        //        2.2 / R^2.2  +  (1.5 * G)^2.2  +  (0.6 * B)^2.2
        // Gray =    / -------------------------------------------
        //          /          1  +  1.5^2.2  +  0.6^2.2
        //
        //        2.2 _______________________________________________________
        // Gray =    /  0.2973 * R^2.2  +  0.6274 * G^2.2  +  0.0753 * B^2.2

        pDst[idx].R = pDst[idx].G = pDst[idx].B = (UINT8)

            pow(0.2973 * pow(pSrc[idx].R, 2.2) +
                0.6274 * pow(pSrc[idx].G, 2.2) +
                0.0753 * pow(pSrc[idx].B, 2.2), 1 / 2.2);

    MY_END_EXTRACT_GRAY
}

#define MY_EXTRACT_GRAY_X(Channel) MY_BEGIN_EXTRACT_GRAY \
    pDst[idx].R = pDst[idx].G = pDst[idx].B = pSrc[idx].Channel; MY_END_EXTRACT_GRAY

void myExtractGrayR(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo)
{
    MY_EXTRACT_GRAY_X(R)
}

void myExtractGrayG(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo)
{
    MY_EXTRACT_GRAY_X(G)
}

void myExtractGrayB(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo)
{
    MY_EXTRACT_GRAY_X(B)
}

#undef MY_EXTRACT_GRAY_X
#undef MY_END_EXTRACT_GRAY
#undef MY_BEGIN_EXTRACT_GRAY

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                              Histogram Equalization & Regulation |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

void myComputeHistogramResult(MyBGRA* pData, MyBmpInfo* pInfo, double pHist[256])
{
    ZeroMemory(pHist, 256 * sizeof(double));
    // Count gray scale pixels.
    for (UINT i = 0; i < pInfo->nWidth; ++i)
        for (UINT j = 0; j < pInfo->nHeight; ++j)
        {
            UINT idx = i + j * pInfo->nWidth;
            UINT8 grayScale = min(255, max(0, pData[idx].R));
            pHist[grayScale] += 1;
        }
    UINT nPixelCount = pInfo->nWidth * pInfo->nHeight;
    // Compute each possibility.
    for (int k2 = 0; k2 < 256; ++k2)
        pHist[k2] = pHist[k2] / nPixelCount;
}

void myDisplayHistogramResult(HDC hdc, int offsetX, int offsetY, double pHist[256],
                              UINT8 nGrayScaleStep, double yHistStrench)
{
    // X-Axis (8-bit, 256, Gray Scale)
    MoveToEx(hdc, offsetX, offsetY, NULL);
    LineTo(hdc, offsetX + 256, offsetY);

    // Y-Axis (normalized Pixel Count)
    MoveToEx(hdc, offsetX, offsetY, NULL);
    LineTo(hdc, offsetX, 0);
    
    for (int i = 0; i < 256; i += nGrayScaleStep)
    {
        // Draw each vertical line.
        MoveToEx(hdc, offsetX + i, offsetY, NULL);

        double totalPossibility = 0;
        // Compute P_sum of all pixels in current scale range.
        for (int j = i - nGrayScaleStep + 1; j <= i; ++j)
            totalPossibility += pHist[j];

        totalPossibility = min(1, max(0, totalPossibility * yHistStrench));
        LineTo(hdc, offsetX + i, (int)((1 - totalPossibility) * offsetY));
    }
}

/*
* How to do histogram equalization for a given image:
* 
* r: origin gray scale value.
* s: equalized gray scale value.
* L: gray scale range (256 here).
* k: gray scale index (0 ~ 255 here, r_k, s_k).
* p(r): probability that a pixel's gray scale is [r].
* 
* s_k = (L - 1) SUM_0^k { p(r_k) }
*/
void myEqualizeHistogram(MyBGRA* pData, MyBmpInfo* pInfo, double pHist[256])
{
    double equalized[256];
    // Work out equalized result.
    for (int i = 0; i < 256; ++i)
    {
        equalized[i] = 0;
        for (int j = 0; j <= i; ++j)
            equalized[i] += pHist[j];
        equalized[i] *= 255;
    }
    // Apply gray transform for each pixel.
    for (UINT c = 0; c < pInfo->nWidth; ++c) // column
        for (UINT r = 0; r < pInfo->nHeight; ++r) // row
        {
            UINT idx = c + r * pInfo->nWidth;
            UINT8 grayScale = pData[idx].R;
            pData[idx].R = pData[idx].G = pData[idx].B = (UINT8)equalized[grayScale];
            pData[idx].A = 255; // Keep opaque by default.
        }
}

void myDisplayHistRgltFormatLines(HDC hdc, POINT aAnchors[256], UINT nPointCount)
{
    if (nPointCount < 1 || nPointCount > 256) return;

    UINT i; // Declare a common index variable here.

    HGDIOBJ originPen = SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(255, 0, 0)));
    for (i = 0; i < nPointCount - 1; ++i)
    {        
        // Draw each intermidiate line.
        MoveToEx(hdc, aAnchors[i].x, aAnchors[i].y, NULL);
        LineTo(hdc, aAnchors[i + 1].x, aAnchors[i + 1].y);
    }

    HGDIOBJ originBrush = SelectObject(hdc, CreateSolidBrush(RGB(255, 0, 0)));
    // Draw anchor endpoints.
    for (i = 0; i < nPointCount; ++i)
        Ellipse(hdc, aAnchors[i].x - 2, aAnchors[i].y - 2, aAnchors[i].x + 2, aAnchors[i].y + 2);

    // Restore origin brush and pen objects.
    DeleteObject(SelectObject(hdc, originBrush));
    DeleteObject(SelectObject(hdc, originPen));
}

void myComputeRgltHistFormat(double pTargetHist[256], POINT aAnchors[256], UINT nPointCount, double rgltStretch)
{
    if (nPointCount < 1 || nPointCount > 256) return;

    UINT i, j; // Declare common index variables here.

    POINT converted[256];
    double slopes[255];

    // We have to handle the 1st anchor point separately since we will calculate slope later.
    // @see comments around declaration of this func for more info about conversion formula.
    converted[0].x = aAnchors[0].x - 5;
    converted[0].y = 256 - aAnchors[0].y;
    // Compute converted coordinates and slopes.
    for (i = 1; i < nPointCount; ++i)
    {
        converted[i].x = aAnchors[i].x - 5;
        converted[i].y = 256 - aAnchors[i].y;
        slopes[i - 1] = (double)(converted[i].y - converted[i - 1].y)
                      / (double)(converted[i].x - converted[i - 1].x);
        // The previous coordinate has already been computed.
    }

    double sum = 0;
    // Start compute target histogram.
    for (i = 0; i < 256; ++i)
        // Decide possibility of each gray scale.
        for (j = 0; j < nPointCount - 1; ++j)
            if (converted[j].x <= (int)i && (int)i < converted[j + 1].x)
            {
                // y = y0 + k * (x - x0), powered by regulation stretch
                pTargetHist[i] = converted[j].y + slopes[j] * (i - converted[j].x);
                pTargetHist[i] = pow(pTargetHist[i], rgltStretch);
                // We will use this value to normalize later.
                sum += pTargetHist[i];
                break; // Move to next gray scale.
            }

    // Normalize target histogram.
    for (i = 0; i < 256; ++i) pTargetHist[i] = pTargetHist[i] / sum;
}

/*
* How to do histogram regulation for a given image:
* 
* @see Histogram-Equalization, we have the formula:
* 
* s_k = (L - 1) SUM_0^k { p(r_k) } = P(r)           (1)
* 
* Besides, we already know the regulated target (in histogram format),
* so it's easy to write the similar histogram-equalization formula:
* 
* s_k = (L - 1) SUM_0^k { p(z_k) } = G(r)           (2)
* 
* where [s_k] is equalized result of expected target [z_k].
* 
* The basic idea of histogram regulation is that we believe that the equalization results
* of the origin image and the target image should share the same probability distribution,
* which means that [s_k, (1)] and [s_k, (2)] is equal in the sense of probabilistic value,
* so we can simply use z = G^(P(r)) to find mapped target values where G^ is inverse of G.
*/
void myRegulateHistogram(MyBGRA* pData, MyBmpInfo* pInfo, double pHist[256], double pTargetHist[256])
{
    int i, j; // Delcare common index variables here.

    // r: origin gray scale, ranges 0 ~ 255.
    // s: target gray scale, ranges 0 ~ 255.
    // 
    // P(r): equalized gray scale, computed from pHist.
    // G(s): regulated gray scale, computed from pTargetHist.
    //
    double P_r[256], G_s[256];
    // Work out equalized and regulated results.
    for (i = 0; i < 256; ++i)
    {
        P_r[i] = 0;
        G_s[i] = 0;
        for (j = 0; j <= i; ++j)
        {
            P_r[i] += pHist[j];
            G_s[i] += pTargetHist[j];
        }
        P_r[i] *= 255;
        G_s[i] *= 255;
    }

    // z = G^(P(r)), where z is our final target.
    UINT8 mapped[256];
    // Initialize all pixels as 255, since 255 is the expected value
    // when all regulated gray scale values (G_s) are less than P_r.
    memset(mapped, 255, 256 * sizeof(UINT8));
    // Decide final target results.
    for (i = 0; i < 256; ++i)
    {
        UINT8 nLowIdx;
        BOOL bFindLow = FALSE;
        for (j = 0; j < 256; ++j)
        {
            UINT8 pr = (UINT8)P_r[i];
            UINT8 gs = (UINT8)G_s[j];
            // perfect match
            if (pr == gs)
            {
                mapped[i] = j;
                break;
            }
            // candidate found
            else if (pr > gs)
            {
                nLowIdx = j;
                bFindLow = TRUE;
            }
            // approximate match (use low by default)
            else if (pr < gs && bFindLow)
            {
                mapped[i] = nLowIdx;
                break;
            }
        }
    }

    // Apply gray transform for each pixel.
    for (UINT c = 0; c < pInfo->nWidth; ++c) // column
        for (UINT r = 0; r < pInfo->nHeight; ++r) // row
        {
            UINT idx = c + r * pInfo->nWidth;
            UINT8 grayScale = pData[idx].R;
            pData[idx].R = pData[idx].G = pData[idx].B = mapped[grayScale];
            pData[idx].A = 255; // Keep opaque by default.
        }
}

RECT myGetSpafGeneWndInitSize()
{
    RECT rc = { 0, 0, gBmpInfo.nWidth + 36 + 25 * gCharWidth, gBmpInfo.nHeight + 2 * gCharHeight + 12 };
    // Note this func wouldn't take WS_VSCROLL into consideration.
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

MyBGRA* myExpandMirrorImage(MyBGRA* pData, MyBmpInfo* pInfo, UINT xExpandSize, UINT yExpandSize)
{
    // Make sure expand-size not greater than origin size.
    assert(xExpandSize <= pInfo->nWidth);
    assert(yExpandSize <= pInfo->nHeight);

    UINT i, j, srcIdx, dstIdx; // Declare common index variables here.

    UINT nBufferWidth = pInfo->nWidth + 2 * xExpandSize;
    UINT nBufferHeight = pInfo->nHeight + 2 * yExpandSize;
    UINT nBufferPixelCount = nBufferWidth * nBufferHeight;

    // Allocate memory for expand buffer.
    MyBGRA* buffer = (MyBGRA*)malloc(nBufferPixelCount * sizeof(MyBGRA));
    myAssert(buffer);

    // The body part should be the same.
    for (i = 0 ; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            srcIdx = i + j * pInfo->nWidth;
            dstIdx = xExpandSize + i + (yExpandSize + j) * nBufferWidth;
            buffer[dstIdx] = pData[srcIdx];
        }

#define LEFT_SRC_X ( xExpandSize - i - 1 )
#define LEFT_DST_X ( i )

#define RIGHT_SRC_X ( pInfo->nWidth - i - 1 )
#define RIGHT_DST_X ( xExpandSize + pInfo->nWidth + i )

#define TOP_SRC_Y ( yExpandSize - j - 1 ) * pInfo->nWidth
#define TOP_DST_Y ( j * nBufferWidth )

#define BOTTOM_SRC_Y ( pInfo->nHeight - j - 1 ) * pInfo->nWidth
#define BOTTOM_DST_Y ( (yExpandSize + pInfo->nHeight + j) * nBufferWidth )

    for (i = 0; i < xExpandSize; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            // Left Border
            srcIdx = LEFT_SRC_X + j * pInfo->nWidth;
            dstIdx = LEFT_DST_X + (yExpandSize + j) * nBufferWidth;
            buffer[dstIdx] = pData[srcIdx];
            // Right Border
            srcIdx = RIGHT_SRC_X + j * pInfo->nWidth;
            dstIdx = RIGHT_DST_X + (yExpandSize + j) * nBufferWidth;
            buffer[dstIdx] = pData[srcIdx];
        }
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < yExpandSize; ++j)
        {
            // Top Border
            srcIdx = i + TOP_SRC_Y;
            dstIdx = (xExpandSize + i) + TOP_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
            // Bottom Border
            srcIdx = i + BOTTOM_SRC_Y;
            dstIdx = (xExpandSize + i) + BOTTOM_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
        }
    for (i = 0; i < xExpandSize; ++i)
        for (j = 0; j < yExpandSize; ++j)
        {
            // Left Top Corner
            srcIdx = LEFT_SRC_X + TOP_SRC_Y;
            dstIdx = LEFT_DST_X + TOP_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
            // Right Bottom Corner
            srcIdx = RIGHT_SRC_X + BOTTOM_SRC_Y;
            dstIdx = RIGHT_DST_X + BOTTOM_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
            // Left Bottom Corner
            srcIdx = LEFT_SRC_X + BOTTOM_SRC_Y;
            dstIdx = LEFT_DST_X + BOTTOM_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
            // Right Top Corner
            srcIdx = RIGHT_SRC_X + TOP_SRC_Y;
            dstIdx = RIGHT_DST_X + TOP_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
        }

#undef LEFT_SRC_X
#undef LEFT_DST_X
#undef RIGHT_SRC_X
#undef RIGHT_DST_X
#undef TOP_SRC_Y
#undef TOP_DST_Y
#undef BOTTOM_SRC_Y
#undef BOTTOM_DST_Y

    return buffer;
}

MyBGRA* myExpand2xBlackImage(MyBGRA* pData, MyBmpInfo* pInfo)
{
    UINT i, j, srcIdx, dstIdx; // Declare common index variables here.

    // Prepare helper variables.
    UINT _2xWidth = 2 * pInfo->nWidth,
         _2xHeight = 2 * pInfo->nHeight;

    // Allocate 2x image.
    size_t nByteSize = _2xWidth * _2xHeight * sizeof(MyBGRA);
    MyBGRA* result = (MyBGRA*)malloc(nByteSize);
    myAssert(result);
    // Populate new pixels as black.
    ZeroMemory(result, nByteSize);

    // Copy source image data.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            srcIdx = i + j * pInfo->nWidth;
            // Note the Y-axis is flipped to disply, so use [j + pInfo->nHeight].
            dstIdx = i + (j + pInfo->nHeight) * _2xWidth;
            result[dstIdx] = pData[srcIdx];
        }

    return result;
}

MyBGRA* myExpand2xMirrorImage(MyBGRA* pData, MyBmpInfo* pInfo)
{
    // Prepare helper variables.
    UINT _2xWidth = 2 * pInfo->nWidth,
         _2xHeight = 2 * pInfo->nHeight;

    UINT _3xWidth = _2xWidth + pInfo->nWidth;

    // Allocate 2x image.
    MyBGRA* result = (MyBGRA*)malloc(_2xWidth * _2xHeight * sizeof(MyBGRA));
    myAssert(result);

    // Create 3x mirrored image.
    MyBGRA* intermidiate = myExpandMirrorImage(pData, pInfo, pInfo->nWidth, pInfo->nHeight);
    myAssert(intermidiate);

    // Clip 3x mirrored image to 2x subarea image.
    for (UINT i = 0; i < _2xWidth; ++i)
        for (UINT j = 0; j < _2xHeight; ++j)
        {
            UINT _2xIdx = i + j * _2xWidth;
            // Note the Y-axis is flipped to disply, so use [j * _3xWidth].
            UINT _4xIdx = (i + pInfo->nWidth) + j * _3xWidth;
            // Copy from mirrored data.
            result[_2xIdx] = intermidiate[_4xIdx];
        }

    // Note we have allocated memory for 3x image data.
    free(intermidiate);

    return result;
}

MyBGRA* myExpand2xCopyImage(MyBGRA* pData, MyBmpInfo* pInfo)
{
    UINT i, j, srcIdx, dstIdx; // Declare common index variables here.

    // Prepare helper variables.
    UINT _2xWidth = 2 * pInfo->nWidth,
         _2xHeight = 2 * pInfo->nHeight;

    // Allocate 2x image.
    size_t nByteSize = _2xWidth * _2xHeight * sizeof(MyBGRA);
    MyBGRA* result = (MyBGRA*)malloc(nByteSize);
    myAssert(result);
    // We have to do this since the right bottom pixels woundn't be populated later.
    ZeroMemory(result, nByteSize);

    // Copy source image data.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            srcIdx = i + j * pInfo->nWidth;
            // Note the Y-axis is flipped to disply, so use [j + pInfo->nHeight].
            dstIdx = i + (j + pInfo->nHeight) * _2xWidth;
            result[dstIdx] = pData[srcIdx];
        }
    // Populate new pixels as boundary color.
    for (i = 0; i < pInfo->nWidth; ++i)
    {
        // Bottom Boundary
        srcIdx = i;
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            // @remark, Y-axis is flipped to display.
            dstIdx = i + j * _2xWidth;
            result[dstIdx] = pData[srcIdx];
        }
    }
    for (j = 0; j < pInfo->nHeight; ++j)
    {
        // Right Boundary
        srcIdx = pInfo->nWidth - 1 + j * pInfo->nWidth;
        for (i = pInfo->nWidth; i < _2xWidth; ++i)
        {
            // @remark, Y-axis is flipped to display.
            dstIdx = i + (j + pInfo->nHeight) * _2xWidth;
            result[dstIdx] = pData[srcIdx];
        }
    }

    return result;
}

/*
* We can compute horizontal and vertical pixel strides separately
* to improve performance since the convolution core is separable (rank = 1).
* 
* For example, suppose origin convolution core is [m] by [n]:
* Ordinary complexity: [m] x [n] for each core......O( n^2 )
* Separable complexity: [m] + [n] for each core.....O( n   )
* When [m] or [n] become very large, the separable approach will save a lot of time.
*/
void mySquareSeparableFilter(MyBGRA* pData, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT nBorderMode, int halfLen,
                             myHorzStrideHandler fnHorz, void* horzExtraData,
                             myVertStrideHandler fnVert, void* vertExtraData,
                             myResultHandler fnResult, void* resultExtraData)
{
    if ((UINT)(2 * halfLen + 1) > min(pInfo->nWidth, pInfo->nHeight)) return;

    UINT i, j, currIdx; // Declare common index variables here.
    int k, tmpIdx; // We must use [signed], since they might be negative.

    UINT nPixelCount = pInfo->nWidth * pInfo->nHeight;
    // Allocate a sum buffer here since we will compute horizontal & vertical separately.
    double* sumColor = (double*)malloc(nPixelCount * sizeof(double));
    myAssert(sumColor);
    ZeroMemory(sumColor, nPixelCount * sizeof(double));

    // Padding zero then we don't have to anything for border-mode_black.
    ZeroMemory(pData, nPixelCount * sizeof(MyBGRA));

    // We don't need to compute border pixels for border-mode_black.
    int offsetIdx = (nBorderMode == MY_SPAF_BM_BLACK) ? halfLen : 0;

    //--------------------------------------------------------------- Horizontal
    for (i = offsetIdx; i < pInfo->nWidth - offsetIdx; ++i)
        for (j = offsetIdx; j < pInfo->nHeight - offsetIdx; ++j)
        {
            currIdx = i + j * pInfo->nWidth;
            // Sum pixels within horizontal stride.
            for (k = -halfLen; k <= halfLen; ++k)
            {
                tmpIdx = i + k;
                // Transform out-of-border pixels.
                switch (nBorderMode)
                {
                case MY_SPAF_BM_BLACK:
                    // tmpIdx always valid for border-mode_black
                    break;
                case MY_SPAF_BM_MIRROR:
                    // Symmetry transform
                    if (tmpIdx < 0)
                        tmpIdx = (-tmpIdx - 1);
                    else if (tmpIdx >= (int)pInfo->nWidth)
                        tmpIdx = (-tmpIdx - 1 + 2 * pInfo->nWidth);
                    break;
                case MY_SPAF_BM_DUPLICATE:
                    // Clamp border color
                    tmpIdx = min((int)pInfo->nWidth - 1, max(0, tmpIdx));
                    break;
                default:
                    break;
                }
                // Call user defined horizontal stride handler.
                fnHorz(sumColor, pSrc, pInfo, currIdx, k, tmpIdx - i, horzExtraData);
            }
        }
    //--------------------------------------------------------------- Vertical
    for (i = offsetIdx; i < pInfo->nWidth - offsetIdx; ++i)
        for (j = offsetIdx; j < pInfo->nHeight - offsetIdx; ++j)
        {
            // Note the difference from horizontal!
            double result = 0;

            currIdx = i + j * pInfo->nWidth;
            // Sum pixels within vertical stride.
            for (k = -halfLen; k <= halfLen; ++k)
            {
                tmpIdx = j + k;
                // Transform out-of-border pixels.
                switch (nBorderMode)
                {
                case MY_SPAF_BM_BLACK:
                    // tmpIdx always valid for border-mode_black
                    break;
                case MY_SPAF_BM_MIRROR:
                    // Symmetry transform
                    if (tmpIdx < 0)
                        tmpIdx = (-tmpIdx - 1);
                    else if (tmpIdx >= (int)pInfo->nHeight)
                        tmpIdx = (-tmpIdx - 1 + 2 * pInfo->nHeight);
                    break;
                case MY_SPAF_BM_DUPLICATE:
                    // Clamp border color
                    tmpIdx = min((int)pInfo->nHeight - 1, max(0, tmpIdx));
                    break;
                default:
                    break;
                }
                // Call user defines vertical stride handler.
                fnVert(&result, sumColor, pInfo, currIdx, k, tmpIdx - j, vertExtraData);
            }
            // Call user defined result handler.
            fnResult(pData, pInfo, currIdx, result, resultExtraData);
        }

    // Note we have allocated memory for sum color data.
    free(sumColor);
}

/*
* Domain Box Filter
*
* Convolution Core (take 3 x 3 as example):
* [ 1/9 ][ 1/9 ][ 1/9 ]
* [ 1/9 ][ 1/9 ][ 1/9 ]
* [ 1/9 ][ 1/9 ][ 1/9 ]
*/

void myDomainBoxFilterHorzHandler(
    double* sumColor, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT currIdx, int orgOffset, int horzOffset, void* extraData)
{
    // We only need to compute one channel for each pixel.
    sumColor[currIdx] += pSrc[currIdx + horzOffset].R;
}

void myDomainBoxFilterVertHandler(
    double* result, double* sumColor, MyBmpInfo* pInfo, UINT currIdx, int orgOffset, int vertOffset, void* extraData)
{
    // Note sumColor has been handled in horizontal stride.
    (*result) += sumColor[currIdx + vertOffset * pInfo->nWidth];
}

void myDomainBoxFilterResultHandler(
    MyBGRA* pData, MyBmpInfo* pInfo, UINT currIdx, double result, void* extraData)
{
    int coef = *((int*)extraData);
    // Get final gray scale result.
    pData[currIdx].R = (UINT8)(result / coef);
    pData[currIdx].G = pData[currIdx].B = pData[currIdx].R;
    pData[currIdx].A = 255; // Keep opaque by default.
}

void myDomainBoxFilter(MyBGRA* pData, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT nBorderMode,
                       int iNormCoef_m)
{
    // Prepare config variables.
    int halfLen = iNormCoef_m / 2;
    int coef = iNormCoef_m * iNormCoef_m;

    // Process convolution indirectly.
    mySquareSeparableFilter(pData, pSrc, pInfo, nBorderMode, halfLen,
                            myDomainBoxFilterHorzHandler, NULL,
                            myDomainBoxFilterVertHandler, NULL,
                            myDomainBoxFilterResultHandler, &coef);
}

/*
* Domain Gaussian Filter
* 
* Convolution Core (take 3 x 3 as example):
* suppose K = 1, sigma = 1, blurRadius = 1:
*                [ 0.3679 ][ 0.6065 ][ 0.3679 ]
* (1 / 4.8976) * [ 0.6065 ][ 1.0000 ][ 0.6065 ]
*                [ 0.3679 ][ 0.6065 ][ 0.3679 ]
*/

void myDomainGaussianFilterHorzHandler(
    double* sumColor, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT currIdx, int orgOffset, int horzOffset, void* extraData)
{
    // We only need to compute one channel for each pixel.
    double* coef = (double*)extraData;
    sumColor[currIdx] += (pSrc[currIdx + horzOffset].R * coef[abs(orgOffset)]);
}

void myDomainGaussianFilterVertHandler(
    double* result, double* sumColor, MyBmpInfo* pInfo, UINT currIdx, int orgOffset, int vertOffset, void* extraData)
{
    // Note sumColor has been handled in horizontal stride.
    double* coef = (double*)extraData;
    (*result) += (sumColor[currIdx + vertOffset * pInfo->nWidth] * coef[abs(orgOffset)]);
}

void myDomainGaussianFilterResultHandler(
    MyBGRA* pData, MyBmpInfo* pInfo, UINT currIdx, double result, void* extraData)
{
    double K = *((double*)extraData);
    // Get final gray scale result.
    pData[currIdx].R = (UINT8)(result / K);
    pData[currIdx].G = pData[currIdx].B = pData[currIdx].R;
    pData[currIdx].A = 255; // Keep opaque by default.
}

void myDomainGaussianFilter(MY_FILTER_FUNC_PARAMS, double sigma, int blurRadius)
{
    int i, j; // Declare common index variables here.

    // Prepare config variables.
    double* coef = (double*)malloc((blurRadius + 1) * sizeof(double));
    myAssert(coef);

    // @remark
    // In fact, we only need to calculate one complete gaussian coefficient table
    // and use the existed values to get coef-array and K for better performance;
    // however, we just re-calculate these values here to improve the readability.

    // Calculate gaussian coefficient lookup table.
    double _one_2sigma2 = -1 / (2 * sigma * sigma);
    for (i = 0; i <= blurRadius; ++i) coef[i] = pow(2.71828, (i * i) * _one_2sigma2);

    // Calculate normalization coefficient [K].
    double K = 0;
    for (i = -blurRadius; i <= blurRadius; ++i)
        for (j = -blurRadius; j <= blurRadius; ++j)
            K += pow(2.71828, (i * i + j * j) * _one_2sigma2);
    

    // Process convolution indirectly.
    mySquareSeparableFilter(pData, pSrc, pInfo, nBorderMode, blurRadius,
                            myDomainGaussianFilterHorzHandler, coef,
                            myDomainGaussianFilterVertHandler, coef,
                            myDomainGaussianFilterResultHandler, &K);
}

/*
* Domain Median Filter
*
* One kind of non-linear filters (min, max, median), which is very useful for removing pepper noise.
* The filter doesn't process convolution, but will select the median value as final result directly.
*/

int myUint8CompareFunc(const void* left, const void* right)
{
    return *((UINT8*)left) - *((UINT8*)right);
}

void myDomainMedianFilter(MyBGRA* pData, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT nBorderMode, int halfLen)
{
    if ((UINT)(2 * halfLen + 1) > min(pInfo->nWidth, pInfo->nHeight)) return;

    UINT i, j, currIdx, tmpIdx; // Declare common index variables here.
    int s, t; // We must use [signed], since they might be negative.

    int nCoreSize = 2 * halfLen + 1;
    int nElemCount = nCoreSize * nCoreSize;
    int nMedianIdx = (nElemCount + 1) / 2;
    // Allocate a temporary median candidate buffer.
    UINT8* candidates = (UINT8*)malloc(nElemCount * sizeof(UINT8));
    myAssert(candidates);

    // We decide to keep border pixels the same as origin.
    for (i = halfLen; i < pInfo->nWidth - halfLen; ++i)
        for (j = halfLen; j < pInfo->nHeight - halfLen; ++j)
        {
            currIdx = i + j * pInfo->nWidth;

            tmpIdx = 0;
            // Collect candidate values.
            for (s = -halfLen; s <= halfLen; ++s)
                for (t = -halfLen; t < halfLen; ++t)
                {
                    candidates[tmpIdx++] = pSrc[currIdx + s + t * pInfo->nWidth].R;
                }
            // Find median value.
            qsort(candidates, nElemCount, sizeof(UINT8), myUint8CompareFunc);
            pData[currIdx].R = pData[currIdx].G = pData[currIdx].B = candidates[nMedianIdx];
            pData[currIdx].A = 255; // Keep opaque by default.
        }

    // Note we have allocated memory for candidate buffer.
    free(candidates);
} 

/*
* Laplace Operator, 2nd derivative.
* 
* [ -1 ][ -1 ][ -1 ]
* [ -1 ][ +8 ][ -1 ]
* [ -1 ][ -1 ][ -1 ]
* 
* There're other formats of Laplace operators, but we select this one for briefness.
*/
void myDomainLaplaceFilter(MY_FILTER_FUNC_PARAMS)
{
    if (min(pInfo->nWidth, pInfo->nHeight) < 3) return;

    UINT i, j, currIdx; // Declare common index variables here.

    // Create an intermidiate buffer.
    double* tmpImage = (double*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(double));
    myAssert(tmpImage);

    double tmpMin = 255, tmpMax = 0;
    // Calculate convolution result.
    for (i = 1; i < pInfo->nWidth - 1; ++i)
        for (j = 1; j < pInfo->nHeight - 1; ++j)
        {
            currIdx = i + j * pInfo->nWidth;

            tmpImage[currIdx] =
                (8.0 * (double)pSrc[currIdx].R
                 - pSrc[currIdx - 1                ].R - pSrc[currIdx + 1                ].R
                 - pSrc[currIdx - pInfo->nWidth    ].R - pSrc[currIdx + pInfo->nWidth    ].R
                 - pSrc[currIdx - 1 - pInfo->nWidth].R - pSrc[currIdx + 1 - pInfo->nWidth].R
                 - pSrc[currIdx - 1 + pInfo->nWidth].R - pSrc[currIdx + 1 + pInfo->nWidth].R);
            // Update min, max values.
            tmpMin = min(tmpImage[currIdx], tmpMin);
            tmpMax = max(tmpImage[currIdx], tmpMax);
        }
    tmpMax -= tmpMin;
    // Normalize to 0 ~ 255.
    for (i = 1; i < pInfo->nWidth - 1; ++i)
        for (j = 1; j < pInfo->nHeight - 1; ++j)
        {
            currIdx = i + j * pInfo->nWidth;

            pData[currIdx].R = pData[currIdx].G = pData[currIdx].B = (UINT8)
                min(255, max(0, (255.0 * ((tmpImage[currIdx] - tmpMin) / tmpMax))));
        }

    // Note we have allocated memory for temporary image data.
    free(tmpImage);
}

/*
* Sobel Operator, 1st derivative.
* 
* ( X-Axis )            ( Y-Axis )
* [ -1 ][ -2 ][ -1 ]    [ -1 ][ +0 ][ +1 ]
* [ +0 ][ +0 ][ +0 ]    [ -2 ][ +0 ][ +2 ]
* [ +1 ][ +2 ][ +1 ]    [ -1 ][ +0 ][ +1 ]
* 
* M(x,y) = { [(z7 + 2z8 + z9) - (z1 + 2z2 + z3)]^2  +
*            [(z3 + 2z6 + z9) - (z1 + 2z4 + z7)]^2 }^(1/2)
*/
void myDomainSobelFilter(MY_FILTER_FUNC_PARAMS)
{
    if (min(pInfo->nWidth, pInfo->nHeight) < 3) return;

    UINT i, j, currIdx; // Declare common index variables here.

    // Create an intermidiate buffer.
    double* tmpImage = (double*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(double));
    myAssert(tmpImage);

    double tmpMin = 255, tmpMax = 0;
    // Calculate convolution result.
    for (i = 1; i < pInfo->nWidth - 1; ++i)
        for (j = 1; j < pInfo->nHeight - 1; ++j)
        {
            currIdx = i + j * pInfo->nWidth;

            tmpImage[currIdx] = pow(
                pow(((double)pSrc[currIdx - 1 + pInfo->nWidth].R +
                             2 * pSrc[currIdx + pInfo->nWidth].R +
                             pSrc[currIdx + 1 + pInfo->nWidth].R) -
                    ((double)pSrc[currIdx - 1 - pInfo->nWidth].R +
                             2 * pSrc[currIdx - pInfo->nWidth].R +
                             pSrc[currIdx + 1 - pInfo->nWidth].R), 2) +
                pow(((double)pSrc[currIdx + 1 - pInfo->nWidth].R +
                             2 * pSrc[currIdx + 1            ].R +
                             pSrc[currIdx + 1 + pInfo->nWidth].R) -
                    ((double)pSrc[currIdx - 1 - pInfo->nWidth].R +
                             2 * pSrc[currIdx - 1            ].R +
                             pSrc[currIdx -1 + pInfo->nWidth].R), 2), 0.5);
            // Update min, max values.
            tmpMin = min(tmpImage[currIdx], tmpMin);
            tmpMax = max(tmpImage[currIdx], tmpMax);
        }
    tmpMax -= tmpMin;
    // Normalize to 0 ~ 255.
    for (i = 1; i < pInfo->nWidth - 1; ++i)
        for (j = 1; j < pInfo->nHeight - 1; ++j)
        {
            currIdx = i + j * pInfo->nWidth;

            pData[currIdx].R = pData[currIdx].G = pData[currIdx].B = (UINT8)
                min(255, max(0, (255.0 * ((tmpImage[currIdx] - tmpMin) / tmpMax))));
        }

    // Note we have allocated memory for temporary image data.
    free(tmpImage);
}

void mycpDFT(MyComplex* pDst, MyComplex* pSrc, UINT L, UINT incre)
{
    // Prepare helper variables.
    long double _2pi = -6.283185L;

    for (UINT f = 0; f < L; ++f)
    {
        UINT dstIdx = f * incre;
        // Populate zero since we will do increment later.
        pDst[dstIdx].real = pDst[dstIdx].imag = 0;
        for (UINT t = 0; t < L; ++t)
        {
            UINT srcIdx = t * incre;
            // e^-j * 2pi * (ft / L)
            MyComplex item = mycpEj(_2pi * ((double)(f * t) / L));
            // f(t) * item
            mycpMulInPlace(pSrc + srcIdx, &item, &item);
            // sum of items
            mycpAddInPlace(pDst + dstIdx, &item, pDst + dstIdx);
        }
    }
}

void mycpIDFT(MyComplex* pDst, MyComplex* pSrc, UINT L, UINT incre)
{
    // Prepare helper variables.
    long double twopi = 6.283185L;
    long double recpL = 1.0L / L;

    for (UINT t = 0; t < L; ++t)
    {
        UINT dstIdx = t * incre;
        // Populate zero since we will do increment later.
        pDst[dstIdx].real = pDst[dstIdx].imag = 0;
        for (UINT f = 0; f < L; ++f)
        {
            UINT srcIdx = f * incre;
            // e^j * 2pi * (ft / L)
            MyComplex item = mycpEj(twopi * ((long double)(f * t) / L));
            // F(f) * item
            mycpMulInPlace(pSrc + srcIdx, &item, &item);
            // sum of items
            mycpAddInPlace(pDst + dstIdx, &item, pDst + dstIdx);
        }
        // normalize, 1 / L
        mycpMulScalarInPlace(recpL, pDst + dstIdx, pDst + dstIdx);
    }
}

UINT myGetRadix2(UINT num)
{
    for (UINT i = 0; i < sizeof(UINT) * 8; ++i)
    {
        if ((0x01 << i) >= num)
        {
            return i;
        }
    }
    return UINT_MAX;
}

void mycpFFT(MyComplex* pData, UINT R, UINT incre)
{
    // We keep the twiddle factor array as static; there's no need to recalculate
    // the factors repeatedly if [N] equals [originN], which would save some time.
    static UINT originRadix = 0u;
    static UINT originLength = 0u;
    static UINT nFactorCount = 0u;
    static MyComplex* WN = NULL; // twiddle factors

    // Height of each butterfly unit.
    static UINT nBtfyDist = 0u;
    // Distance between the headers of 2 adjacent butterfly units.
    // This is always equals to [nBtfyDist * 2] or [nBtfyDist << 1].
    static UINT nBtfyInterGap = 0u;

    static UINT wnidx = 0u; // current twiddle factor index
    static MyComplex* wnptr = NULL; // points to current WN

    // Declare common index variables here.
    static UINT i, j, k, idx, idx2;

    // Prepare helper variables.
    static long double _2pi = -6.283185L;
    static MyComplex* x = NULL, * xp = NULL; // p: peer
    static MyComplex temp = {0,0}; // temp intermidiate

    UINT N = (1 << R);
    // Do nothing for empty/single array.
    if (N <= 1) return;

    // Check whether need to recalculate twiddle factors.
    if (N != originLength)
    {
        originRadix = R;
        originLength = N;
        nFactorCount = N >> 1; // N / 2

        // 2-dot FFT doesn't need twiddle factor.
        if (nFactorCount != 1)
        {
            if (WN != NULL) free(WN);
            // Allocate memory for twiddle factors.
            WN = (MyComplex*)malloc(nFactorCount * sizeof(MyComplex));
            myAssert(WN);

            long double _2pi_N = _2pi / N;
            // Calculate and store twiddle factors.
            for (i = 0; i < nFactorCount; ++i)
            {
                mycpEjInPlace(_2pi_N * i, WN + i);
            }
        }
    }

    // Calculate butterfly operation units of FFT.
    nBtfyDist = N;
    wnidx = 1; // WN_1
    // For each decimation level:
    for (i = 0; i < R; ++i)
    {
        nBtfyInterGap = nBtfyDist;
        nBtfyDist >>= 1; // nBtfyDist / 2

        // Part 1:
        // The 1st butterfly operation never needs twiddle factor,
        // since it is always multiplied by WN_0, i.e. e^(j0) = 1.
        for (j = 0; j < N; j += nBtfyInterGap)
        {
            x = pData + j * incre;
            xp = x + nBtfyDist * incre;

            // y = x + xp
            mycpAddInPlace(x, xp, &temp);
            // yp = x - xp
            mycpSubInPlace(x, xp, xp);

            (*x) = temp; // avoid overwrite
        }

        // Part 2:
        // The remained butterfly units always need twiddle factors.
        wnptr = WN + wnidx;
        // For different twiddle factor groups ......
        for (j = 1; j < nBtfyDist; ++j)
        {
            // For those with the same twiddle factors ......
            for (k = j; k < N; k += nBtfyInterGap)
            {
                x = pData + k * incre;
                xp = x + nBtfyDist * incre;

                // y = x + xp
                mycpAddInPlace(x, xp, &temp);
                // yp = (x - xp) * WN
                mycpSubInPlace(x, xp, x);
                mycpMulInPlace(x, wnptr, xp);

                (*x) = temp; // avoid overwrite
            }
            wnptr += wnidx;
        }
        wnidx <<= 1; // wnidx * 2
    }

    // Rearrange results by bit reversing.
    // For example, for 8-dot FFT of (radix 2) frequency decimation:
    // 
    // x(t)    bit reverse  X(f)
    // 0, 000   -------->   0, 000
    // 1, 001   -------->   4, 100
    // 2, 010   -------->   2, 010
    // 3, 011   -------->   6, 110
    // 4, 100   -------->   1, 001
    // 5, 101   -------->   5, 101
    // 6, 110   -------->   3, 011
    // 7, 111  --------->   7, 111
    //
    for (i = 0; i < N; ++i)
    {
        k = 0;
        idx = i * incre;
        // Get reversed number.
        for (j = 0; j < R; ++j)
        {
            k = (k << 1) | (0x01 & (i >> j));
        }
        // Swap if found.
        if (i < k)
        {
            idx2 = k * incre;
            temp = pData[idx];
            pData[idx] = pData[idx2];
            pData[idx2] = temp;
        }
    }
}

void mycpIFFT(MyComplex* pData, UINT R, UINT incre)
{
    // We keep the twiddle factor array as static; there's no need to recalculate
    // the factors repeatedly if [N] equals [originN], which would save some time.
    static UINT originRadix = 0u;
    static UINT originLength = 0u;
    static UINT nFactorCount = 0u;
    static MyComplex* WN = NULL; // twiddle factors

    // Height of each butterfly unit.
    static UINT nBtfyDist = 0u;
    // Distance between the headers of 2 adjacent butterfly units.
    // This is always equals to [nBtfyDist * 2] or [nBtfyDist << 1].
    static UINT nBtfyInterGap = 0u;

    static UINT wnidx = 0u; // current twiddle factor index
    static MyComplex* wnptr = NULL; // points to current WN

    // Declare common index variables here.
    static UINT i, j, k, idx, idx2;

    // Prepare helper variables.
    static long double twopi = 6.283185L;
    static MyComplex* x = NULL, * xp = NULL; // p: peer
    static MyComplex temp = {0,0}; // temp intermidiate
    static long double dNormCoef = 0; // set to [1 / N]

    UINT N = (1 << R);
    // Do nothing for empty/single array.
    if (N <= 1) return;

    // Check whether need to recalculate twiddle factors.
    if (N != originLength)
    {
        originRadix = R;
        originLength = N;
        nFactorCount = N >> 1; // N / 2

        // 2-dot FFT doesn't need twiddle factor.
        if (nFactorCount != 1)
        {
            if (WN != NULL) free(WN);
            // Allocate memory for twiddle factors.
            WN = (MyComplex*)malloc(nFactorCount * sizeof(MyComplex));
            myAssert(WN);

            long double twopi_N = twopi / N;
            // Calculate and store twiddle factors.
            for (i = 0; i < nFactorCount; ++i)
            {
                mycpEjInPlace(twopi_N * i, WN + i);
            }
        }
    }

    // Calculate butterfly operation units of FFT.
    nBtfyDist = N;
    wnidx = 1; // WN_1
    // For each decimation level:
    for (i = 0; i < R; ++i)
    {
        nBtfyInterGap = nBtfyDist;
        nBtfyDist >>= 1; // nBtfyDist / 2

        // Part 1:
        // The 1st butterfly operation never needs twiddle factor,
        // since it is always multiplied by WN_0, i.e. e^(j0) = 1.
        for (j = 0; j < N; j += nBtfyInterGap)
        {
            x = pData + j * incre;
            xp = x + nBtfyDist * incre;

            // y = x + xp
            mycpAddInPlace(x, xp, &temp);
            // yp = x - xp
            mycpSubInPlace(x, xp, xp);

            (*x) = temp; // avoid overwrite
        }

        // Part 2:
        // The remained butterfly units always need twiddle factors.
        wnptr = WN + wnidx;
        // For different twiddle factor groups ......
        for (j = 1; j < nBtfyDist; ++j)
        {
            // For those with the same twiddle factors ......
            for (k = j; k < N; k += nBtfyInterGap)
            {
                x = pData + k * incre;
                xp = x + nBtfyDist * incre;

                // y = x + xp
                mycpAddInPlace(x, xp, &temp);
                // yp = (x - xp) * WN
                mycpSubInPlace(x, xp, x);
                mycpMulInPlace(x, wnptr, xp);

                (*x) = temp; // avoid overwrite
            }
            wnptr += wnidx;
        }
        wnidx <<= 1; // wnidx * 2
    }

    // Rearrange results by bit reversing.
    // For example, for 8-dot FFT of (radix 2) frequency decimation:
    // 
    // x(t)    bit reverse  X(f)
    // 0, 000   -------->   0, 000
    // 1, 001   -------->   4, 100
    // 2, 010   -------->   2, 010
    // 3, 011   -------->   6, 110
    // 4, 100   -------->   1, 001
    // 5, 101   -------->   5, 101
    // 6, 110   -------->   3, 011
    // 7, 111  --------->   7, 111
    //
    for (i = 0; i < N; ++i)
    {
        k = 0;
        idx = i * incre;
        // Get reversed number.
        for (j = 0; j < R; ++j)
        {
            k = (k << 1) | (0x01 & (i >> j));
        }
        // Swap if found.
        if (i < k)
        {
            idx2 = k * incre;
            temp = pData[idx];
            pData[idx] = pData[idx2];
            pData[idx2] = temp;
        }
    }
    
    dNormCoef = 1.0L / N;
    // Normalize results with dot count.
    for (i = 0; i < N; ++i)
    {
        idx = i * incre;
        mycpMulScalarInPlace(dNormCoef, pData + idx, pData + idx);
    }
}

MyComplex* mycpDFT2(MyComplex* pData, UINT M, UINT N)
{
    size_t nByteSize = M * N * sizeof(MyComplex);
    // Allocate transformed results buffer.
    MyComplex* buffer = (MyComplex*)malloc(nByteSize);
    myAssert(buffer);
    // Allocate temporary intermidiate buffer to calculate separately.
    MyComplex* intermidiate = (MyComplex*)malloc(nByteSize);
    myAssert(intermidiate);

    //--------------------------------------------------------------- Horizontal
    for (UINT u = 0; u < M; ++u)
    {
        UINT incre = u;
        mycpDFT(intermidiate + incre, pData + incre, N, M);
    }
    //--------------------------------------------------------------- Vertical
    for (UINT v = 0; v < N; ++v)
    {
        UINT incre = v * M;
        mycpDFT(buffer + incre, intermidiate + incre, M, 1);
    }

    // Note we have allocated memory for intermidiate buffer data.
    free(intermidiate);

    return buffer;
}

MyComplex* mycpIDFT2(MyComplex* pData, UINT M, UINT N)
{
    size_t nByteSize = M * N * sizeof(MyComplex);
    // Allocate transformed results buffer.
    MyComplex* buffer = (MyComplex*)malloc(nByteSize);
    myAssert(buffer);
    // Allocate temporary intermidiate buffer to calculate separately.
    MyComplex* intermidiate = (MyComplex*)malloc(nByteSize);
    myAssert(intermidiate);

    //--------------------------------------------------------------- Horizontal
    for (UINT u = 0; u < M; ++u)
    {
        UINT incre = u;
        mycpIDFT(intermidiate + incre, pData + incre, N, M);
    }
    //--------------------------------------------------------------- Vertical
    for (UINT v = 0; v < N; ++v)
    {
        UINT incre = v * M;
        mycpIDFT(buffer + incre, intermidiate + incre, M, 1);
    }

    // Note we have allocated memory for intermidiate buffer data.
    free(intermidiate);

    return buffer;
}

MyComplex* mycpFFT2(MyComplex* pData, UINT M, UINT RM, UINT N, UINT RN)
{
    size_t nByteSize = M * N * sizeof(MyComplex);
    // Allocate transformed results buffer.
    MyComplex* buffer = (MyComplex*)malloc(nByteSize);
    myAssert(buffer);
    // We have to copy origin data to result buffer in advance
    // since FFT will calculate results in place step by step.
    memcpy(buffer, pData, nByteSize);

    //--------------------------------------------------------------- Horizontal
    for (UINT u = 0; u < M; ++u)
    {
        UINT incre = u;
        mycpFFT(buffer + incre, RN, M);
    }
    //--------------------------------------------------------------- Vertical
    for (UINT v = 0; v < N; ++v)
    {
        UINT incre = v * M;
        mycpFFT(buffer + incre, RM, 1);
    }

    return buffer;
}

MyComplex* mycpIFFT2(MyComplex* pData, UINT M, UINT RM, UINT N, UINT RN)
{
    size_t nByteSize = M * N * sizeof(MyComplex);
    // Allocate transformed results buffer.
    MyComplex* buffer = (MyComplex*)malloc(nByteSize);
    myAssert(buffer);
    // We have to copy origin data to result buffer in advance
    // since FFT will calculate results in place step by step.
    memcpy(buffer, pData, nByteSize);

    //--------------------------------------------------------------- Horizontal
    for (UINT u = 0; u < M; ++u)
    {
        UINT incre = u;
        mycpIFFT(buffer + incre, RN, M);
    }
    //--------------------------------------------------------------- Vertical
    for (UINT v = 0; v < N; ++v)
    {
        UINT incre = v * M;
        mycpIFFT(buffer + incre, RM, 1);
    }

    return buffer;
}

void mycpLogarithmTrans(MyBGRA* pDst, MyComplex* pSrc, UINT M, UINT N, long double c)
{
    UINT i, j, idx; // Declare common index variables here.

    // We must allocate an extra intermidiate buffer to keep pSrc constant.
    long double* buffer = (long double*)malloc(M * N * sizeof(long double));
    myAssert(buffer);

    long double tmpMin = LDBL_MAX, tmpMax = LDBL_MIN;
    // Find min, max values.
    for (i = 0; i < M; ++i)
        for (j = 0; j < N; ++j)
        {
            idx = i + j * M;
            // Update min, max values.
            tmpMin = min(tmpMin, pSrc[idx].real);
            tmpMax = max(tmpMax, pSrc[idx].real);
        }

    long double logMin = tmpMin;
    tmpMin = LDBL_MAX; tmpMax = LDBL_MIN;
    // Calculate logarithm transform.
    for (i = 0; i < M; ++i)
        for (j = 0; j < N; ++j)
        {
            idx = i + j * M;
            buffer[idx] = c * log(1 + (pSrc[idx].real - logMin));
            // Update min, max values.
            tmpMin = min(tmpMin, buffer[idx]);
            tmpMax = max(tmpMax, buffer[idx]);
        }

    tmpMax -= tmpMin;
    // Normalize to 0 ~ 255.
    for (i = 0; i < M; ++i)
        for (j = 0; j < N; ++j)
        {
            idx = i + j * M;
            buffer[idx] = 255 * ((buffer[idx] - tmpMin) / tmpMax);
            pDst[idx].R = pDst[idx].G = pDst[idx].B = (UINT8)min(255, max(0, buffer[idx]));
        }

    // Note we have allocated memory for intermidate buffer.
    free(buffer);
}

void mycpExpGammaTrans(MyBGRA* pDst, MyComplex* pSrc, UINT M, UINT N, long double gamma)
{
    UINT i, j, idx; // Declare common index variables here.

    // We must allocate an extra intermidiate buffer to keep pSrc constant.
    long double* buffer = (long double*)malloc(M * N * sizeof(long double));
    myAssert(buffer);

    long double tmpMin = LDBL_MAX, tmpMax = LDBL_MIN;
    // Find min, max values.
    for (i = 0; i < M; ++i)
        for (j = 0; j < N; ++j)
        {
            idx = i + j * M;
            // Update min, max values.
            tmpMin = min(tmpMin, pSrc[idx].real);
            tmpMax = max(tmpMax, pSrc[idx].real);
        }

    long double expMin = tmpMin;
    tmpMin = LDBL_MAX; tmpMax = LDBL_MIN;
    // Calculate logarithm transform.
    for (i = 0; i < M; ++i)
        for (j = 0; j < N; ++j)
        {
            idx = i + j * M;
            buffer[idx] = pow(pSrc[idx].real, gamma);
            // Update min, max values.
            tmpMin = min(tmpMin, buffer[idx]);
            tmpMax = max(tmpMax, buffer[idx]);
        }

    tmpMax -= tmpMin;
    // Normalize to 0 ~ 255.
    for (i = 0; i < M; ++i)
        for (j = 0; j < N; ++j)
        {
            idx = i + j * M;
            buffer[idx] = 255 * ((buffer[idx] - tmpMin) / tmpMax);
            pDst[idx].R = pDst[idx].G = pDst[idx].B = (UINT8)min(255, max(0, buffer[idx]));
        }

    // Note we have allocated memory for intermidate buffer.
    free(buffer);
}

MyComplex* myPowerSpectral(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, long double dLogParam)
{
    UINT i, j, idx; // Declare common index variables here.

    // Allocate memory for intermidiate complex buffer.
    MyComplex* buffer = (MyComplex*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(MyComplex));
    myAssert(buffer);
    // Initialize complex buffer.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Apply (-1)^(x+y).
            buffer[idx].real = ((i + j) % 2 == 0) ? pSrc[idx].R : (-pSrc[idx].R);
            buffer[idx].imag = 0; // real number
        }

    // Calculate DFT.
    MyComplex* result = mycpDFT2(buffer, pInfo->nWidth, pInfo->nHeight);
    // Convert to power-spectral.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Store power in real part to match the logarithm later.
            result[idx].real = mycpMagnitude2(result + idx);
        }

    // Convert complex to RGB.
    mycpLogarithmTrans(pDst, result, pInfo->nWidth, pInfo->nHeight, dLogParam);
    
    // Note we have allocated memory for complex buffer data.
    free(buffer);

    return result;
}

MyComplex* myPowerSpectralFFT(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT RM, UINT RN, long double dLogParam)
{
    UINT i, j, idx; // Declare common index variables here.

    // Allocate memory for intermidiate complex buffer.
    MyComplex* buffer = (MyComplex*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(MyComplex));
    myAssert(buffer);
    // Initialize complex buffer.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Apply (-1)^(x+y).
            buffer[idx].real = ((i + j) % 2 == 0) ? pSrc[idx].R : (-pSrc[idx].R);
            buffer[idx].imag = 0; // real number
        }

    // Calculate FFT.
    MyComplex* result = mycpFFT2(buffer, pInfo->nWidth, RM, pInfo->nHeight, RN);
    // Convert to power-spectral.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Store power in real part to match the logarithm later.
            result[idx].real = mycpMagnitude2(result + idx);
        }

    // Convert complex to RGB.
    mycpLogarithmTrans(pDst, result, pInfo->nWidth, pInfo->nHeight, dLogParam);

    // Note we have allocated memory for complex buffer data.
    free(buffer);

    return result;
}

MyComplex* myPhaseSpectral(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, long double dLogParam)
{
    UINT i, j, idx; // Declare common index variables here.

    // Allocate memory for intermidiate complex buffer.
    MyComplex* buffer = (MyComplex*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(MyComplex));
    myAssert(buffer);
    // Initialize complex buffer.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Apply (-1)^(x+y).
            buffer[idx].real = ((i + j) % 2 == 0) ? pSrc[idx].R : (-pSrc[idx].R);
            buffer[idx].imag = 0; // real number
        }

    // Calculate DFT.
    MyComplex* result = mycpDFT2(buffer, pInfo->nWidth, pInfo->nHeight);
    // Convert to phase-spectral.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Store phase in real part to match the logarithm later.
            result[idx].real = mycpPhase(result + idx);
        }

    // Convert complex to RGB.
    mycpLogarithmTrans(pDst, result, pInfo->nWidth, pInfo->nHeight, dLogParam);

    // Note we have allocated memory for complex buffer data.
    free(buffer);

    return result;
}

MyComplex* myPhaseSpectralFFT(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT RM, UINT RN, long double dLogParam)
{
    UINT i, j, idx; // Declare common index variables here.

    // Allocate memory for intermidiate complex buffer.
    MyComplex* buffer = (MyComplex*)malloc(pInfo->nWidth * pInfo->nHeight * sizeof(MyComplex));
    myAssert(buffer);
    // Initialize complex buffer.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Apply (-1)^(x+y).
            buffer[idx].real = ((i + j) % 2 == 0) ? pSrc[idx].R : (-pSrc[idx].R);
            buffer[idx].imag = 0; // real number
        }

    // Calculate FFT.
    MyComplex* result = mycpFFT2(buffer, pInfo->nWidth, RM, pInfo->nHeight, RN);
    // Convert to phase-spectral.
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            idx = i + j * pInfo->nWidth;
            // Store phase in real part to match the logarithm later.
            result[idx].real = mycpPhase(result + idx);
        }

    // Convert complex to RGB.
    mycpLogarithmTrans(pDst, result, pInfo->nWidth, pInfo->nHeight, dLogParam);

    // Note we have allocated memory for complex buffer data.
    free(buffer);

    return result;
}

void mySpectralReconstruct(MyBGRA* dst, MyBmpInfo* info, MyComplex* power, MyComplex* phase)
{
    UINT i, j, idx; // Declare common index variables here.

    MyComplex* buffer = (MyComplex*)malloc(info->nWidth * info->nHeight * sizeof(MyComplex));
    myAssert(buffer);

    long double magnitude;
    // Rebuild freq-spectral.
    for (i = 0; i < info->nWidth; ++i)
        for (j = 0; j < info->nHeight; ++j)
        {
            idx = i + j * info->nWidth;
            magnitude = sqrt(power[idx].real);
            buffer[idx].real = magnitude * cos(phase[idx].real);
            buffer[idx].imag = magnitude * sin(phase[idx].real);
        }

    // From freq-domain to space-domain.
    MyComplex* img = mycpIDFT2(buffer, info->nWidth, info->nHeight);

    // Clamp to 0 ~ 255.
    for (i = 0; i < info->nWidth; ++i)
        for (j = 0; j < info->nHeight; ++j)
        {
            idx = i + j * info->nWidth;
            dst[idx].R = dst[idx].G = dst[idx].B = (UINT8)min(255, max(0, abs((int)img[idx].real)));
        }

    free(buffer);
    // Note we have allocated memory for rebuilt image data.
    free(img);
}

void mySpectralReconstructFFT(MyBGRA* dst, MyBmpInfo* info, UINT RM, UINT RN, MyComplex* power, MyComplex* phase)
{
    UINT i, j, idx; // Declare common index variables here.

    MyComplex* buffer = (MyComplex*)malloc(info->nWidth * info->nHeight * sizeof(MyComplex));
    myAssert(buffer);

    long double magnitude;
    // Rebuild freq-spectral.
    for (i = 0; i < info->nWidth; ++i)
        for (j = 0; j < info->nHeight; ++j)
        {
            idx = i + j * info->nWidth;
            magnitude = sqrt(power[idx].real);
            buffer[idx].real = magnitude * cos(phase[idx].real);
            buffer[idx].imag = magnitude * sin(phase[idx].real);
        }

    // From freq-domain to space-domain.
    MyComplex* img = mycpIFFT2(buffer, info->nWidth, RM, info->nHeight, RN);

    // Clamp to 0 ~ 255.
    for (i = 0; i < info->nWidth; ++i)
        for (j = 0; j < info->nHeight; ++j)
        {
            idx = i + j * info->nWidth;
            dst[idx].R = dst[idx].G = dst[idx].B = (UINT8)min(255, max(0, abs((int)img[idx].real)));
        }

    free(buffer);
    // Note we have allocated memory for rebuilt image data.
    free(img);
}

MyBGRA* myPaddingZeroImageFFT(MyBmpInfo* info, MyBGRA* origin, MyBmpInfo* originInfo)
{
    // Get expanded size info.
    info->nWidth = 1 << myGetRadix2(originInfo->nWidth);
    info->nHeight = 1 << myGetRadix2(originInfo->nHeight);

    size_t nByteSize = info->nWidth * info->nHeight * sizeof(MyBGRA);
    // Allocate expanded image data.
    MyBGRA* img = (MyBGRA*)malloc(nByteSize);
    myAssert(img);
    // Padding zeros.
    ZeroMemory(img, nByteSize);

    // Copy origin image data.
    for (UINT i = 0; i < originInfo->nWidth; ++i)
        for (UINT j = 0; j < originInfo->nHeight; ++j)
            img[i + j * info->nWidth] = origin[i + j * originInfo->nWidth];

    return img;
}

MyComplex* myPaddingZeroSpectralFFT(MyComplex* origin, MyBmpInfo* originInfo)
{
    // Get expanded size info.
    UINT tmpW = 1 << myGetRadix2(originInfo->nWidth);
    UINT tmpH = 1 << myGetRadix2(originInfo->nHeight);

    size_t nByteSize = tmpW * tmpH * sizeof(MyComplex);
    // Allocate expanded spectral data.
    MyComplex* sptl = (MyComplex*)malloc(nByteSize);
    myAssert(sptl);
    // Padding zeros.
    ZeroMemory(sptl, nByteSize);

    // Copy origin spectral data.
    for (UINT i = 0; i < originInfo->nWidth; ++i)
        for (UINT j = 0; j < originInfo->nHeight; ++j)
            sptl[i + j * tmpW] = origin[i + j * originInfo->nWidth];

    return sptl;
}

void myTfuncIdealLPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* radius, UINT nFilterCount)
{
    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;

            long double tmpMag = spectral[idx].real;
            spectral[idx].real = 0; // wait for check

            for (UINT k = 0; k < nFilterCount; ++k)
            {
                if ((i - center[k].x) * (i - center[k].x) + // within low pass area
                    (j - center[k].y) * (j - center[k].y) < radius[k] * radius[k])
                {
                    spectral[idx].real = tmpMag;
                }
            }
        }
}

void myTfuncIdealHPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* radius, UINT nFilterCount)
{
    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;
            for (UINT k = 0; k < nFilterCount; ++k)
            {
                if ((i - center[k].x) * (i - center[k].x) + // out of high pass area
                    (j - center[k].y) * (j - center[k].y) < radius[k] * radius[k])
                {
                    spectral[idx].real = 0; // clear magnitude
                    break; // no need to check remaining filters
                }
            }
        }
}

void myTfuncGaussianLPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* sigma, UINT nFilterCount)
{
    // Prepare helper variables.
    const static double E = 2.71828;
    double* _twosigma2 = (double*)malloc(nFilterCount * sizeof(double));
    myAssert(_twosigma2);
    for (int s = 0; s < nFilterCount; ++s) _twosigma2[s] = -1 / (2 * sigma[s] * sigma[s]);

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;
            // Add all weight coef firstly.
            long double D2 = 0, weightCoef = 0;
            for (UINT k = 0; k < nFilterCount; ++k)
            {
                D2 = (i - center[k].x) * (i - center[k].x) +
                     (j - center[k].y) * (j - center[k].y);
                weightCoef += pow(E, D2 * _twosigma2[k]);
            }
            // Apply all gaussian coefficients at once.
            spectral[idx].real *= weightCoef;
        }

    // Note we have allocated memory for helper data.
    free(_twosigma2);
}

void myTfuncGaussianHPF(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* sigma, UINT nFilterCount)
{
    // Prepare helper variables.
    const static double E = 2.71828;
    double* _twosigma2 = (double*)malloc(nFilterCount * sizeof(double));
    myAssert(_twosigma2);
    for (int s = 0; s < nFilterCount; ++s) _twosigma2[s] = -1 / (2 * sigma[s] * sigma[s]);

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;
            // Add all weight coef firstly.
            long double D2 = 0, weightCoef = 0;
            for (UINT k = 0; k < nFilterCount; ++k)
            {
                D2 = (i - center[k].x) * (i - center[k].x) +
                     (j - center[k].y) * (j - center[k].y);
                weightCoef += (1 - pow(E, D2 * _twosigma2[k]));
            }
            // Apply all gaussian coefficients at once.
            spectral[idx].real *= weightCoef;
        }

    // Note we have allocated memory for helper data.
    free(_twosigma2);
}

void myTfuncButterworthLPF(MyComplex* spectral, MyBmpInfo* info,
                           POINT* center, double* cutoff, double nOrder, UINT nFilterCount)
{
    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;
            // Add all weight coef firstly.
            long double D2 = 0, weightCoef = 0;
            for (UINT k = 0; k < nFilterCount; ++k)
            {
                D2 = (i - center[k].x) * (i - center[k].x) +
                     (j - center[k].y) * (j - center[k].y);
                weightCoef += 1 / (1 + pow(D2 / (cutoff[k] * cutoff[k]), nOrder));
            }
            // Apply all butterworth coefficients at once.
            spectral[idx].real *= weightCoef;
        }
}

void myTfuncButterworthHPF(MyComplex* spectral, MyBmpInfo* info,
                           POINT* center, double* cutoff, double nOrder, UINT nFilterCount)
{
    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;
            // Add all weight coef firstly.
            long double D2 = 0, weightCoef = 0;
            for (UINT k = 0; k < nFilterCount; ++k)
            {
                D2 = (i - center[k].x) * (i - center[k].x) +
                    (j - center[k].y) * (j - center[k].y);
                weightCoef += 1 / (1 + pow((cutoff[k] * cutoff[k]) / D2, nOrder));

            }
            // Apply all butterworth coefficients at once.
            spectral[idx].real *= weightCoef;
        }
}

void myTfuncHomomorphicFilter(MyComplex* spectral, MyBmpInfo* info, POINT* center, double* sigma,
                              double c, double gammaLow, double gammaHigh, UINT nFilterCount)
{
    // Prepare helper variables.
    const static double E = 2.71828;
    double gammaDiff = gammaHigh - gammaLow;
    double* _csigma2 = (double*)malloc(nFilterCount * sizeof(double));
    myAssert(_csigma2);
    for (int s = 0; s < nFilterCount; ++s) _csigma2[s] = -c / (2 * sigma[s] * sigma[s]);

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            UINT idx = i + j * info->nWidth;
            // Add all weight coef firstly.
            long double D2 = 0, weightCoef = 0;
            for (UINT k = 0; k < nFilterCount; ++k)
            {
                D2 = (i - center[k].x) * (i - center[k].x) +
                     (j - center[k].y) * (j - center[k].y);
                weightCoef += (gammaDiff * (1 - pow(E, D2 * _csigma2[k])) + gammaLow);
            }
            // Apply all homomorphic filter coefficients at once.
            spectral[idx].real *= weightCoef;
        }

    // Note we have allocated memory for helper data.
    free(_csigma2);
}

RECT myGetThresholdWndInitSize()
{
    RECT rc = { 0, 0, gBmpInfo.nWidth + 24, gBmpInfo.nHeight + 36 + 2 * gCharHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

void myThresholdProcess(MyBGRA* pDst, MyBGRA* pSrc, MyBmpInfo* pInfo, UINT8 threshold)
{
    for (UINT i = 0; i < pInfo->nWidth; ++i)
        for (UINT j = 0; j < pInfo->nHeight; ++j)
        {
            UINT idx = i + j * pInfo->nWidth;
            // Low for black, high for white.
            if (pSrc[idx].R < threshold)
            {
                pDst[idx].R = pDst[idx].G = pDst[idx].B = 0;
            }
            else // There's no need to change alpha channel anyway.
            {
                pDst[idx].R = pDst[idx].G = pDst[idx].B = 255;
            }
        }
}

double myErf(double x)
{
    if (x == 0) return 0;
    double sgn = x > 0 ? 1 : -1;

    double pi = 3.14159;

    double x2 = x * x;
    double ax2 = 8 * (pi - 3) / (3 * pi * (4 - pi)) * x2;

    return sgn * sqrt(1 - exp(-x2 * (4 / pi + ax2) / (1 + ax2)));
}

double myErfInverse(double x)
{
    if (x == 0) return 0;
    double sgn = x > 0 ? 1 : -1;

    double pi = 3.14159;
    
    double a = 8 * (pi - 3) / (3 * pi * (4 - pi));
    double pia = pi * a;
    double ln1x2 = log(1 - x * x);

    return sgn * sqrt(sqrt(pow(2 / pia + ln1x2 / 2, 2) - ln1x2 / a) - (2 / pia + ln1x2 / 2));
}

void myApplyGaussianNoise(MyBGRA* data, MyBmpInfo* info, double u, double sigma)
{
    srand(time(NULL));

    // Generate gaussian distribution from uniform.
    // 
    // If X ~ N(u, sigma), then F(X) ~ U(0, 1),
    // so F_inv(U) ~ X, i.e. probit(p) ~ X.
    // 
    // Gaussian_CDF(x) = 1/2 * [1 + erf((x-u) / (sqrt(2) * sigma))]
    // Gaussian_CDF_inv(p) = sqrt(2) * sigma * erf_inv(2*p - 1) + u

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            // Step 1: get a random variable of U(0, 1).
            double W = rand() / (double)RAND_MAX;

            // Step 2: compute X with given inverse CDF.
            double X = 1.41421 * sigma * myErfInverse(2 * W - 1) + u;

            UINT idx = i + j * info->nWidth;

            double noised = data[idx].R + X;
            // Step 3: apply noise to target pixel data.
            data[idx].R = (UINT8)max(0, min(255, noised));
            data[idx].G = data[idx].B = data[idx].R;
            data[idx].A = 255; // Keep opaque by default.
        }
}

void myApplyRayleighNoise(MyBGRA* data, MyBmpInfo* info, double a, double b)
{
    srand(time(NULL));

    // Generate rayleigh distribution from uniform.
    //
    // Rayleigh_CDF(x) = 1 - e^(-(x-a)^2 / b), if x >= a.
    // Rayleigh_CDF_inv(p) = a + sqrt(-b * ln(1-p)), if 0 <= p <= 1.

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            // Step 1: get a random variable of U(0, 1).
            double W = rand() / (double)RAND_MAX;

            // Step 2: compute X with given inverse CDF.
            double X = a + sqrt(-b * log(1 - W));

            UINT idx = i + j * info->nWidth;

            double noised = data[idx].R + X;
            // Step 3: apply noise to target pixel data.
            data[idx].R = (UINT8)max(0, min(255, noised));
            data[idx].G = data[idx].B = data[idx].R;
            data[idx].A = 255; // Keep opaque by default.
        }
}

void myApplyErlangNoise(MyBGRA* data, MyBmpInfo* info, double a, int b)
{
    // TODO: support Erlang noise model.
}

void myApplyExponentialNoise(MyBGRA* data, MyBmpInfo* info, double a)
{
    srand(time(NULL));

    // Generate rayleigh distribution from uniform.
    //
    // Exponential_CDF(x) = 1 - e^(-a * x), if x >= 0.
    // Exponential_CDF_inv(p) = -ln(1 - p) / a, if 0 <= p <= 1.

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            // Step 1: get a random variable of U(0, 1).
            double W = rand() / (double)RAND_MAX;

            // Step 2: compute X with given inverse CDF.
            double X = -log(1 - W) / a;

            UINT idx = i + j * info->nWidth;

            double noised = data[idx].R + X;
            // Step 3: apply noise to target pixel data.
            data[idx].R = (UINT8)max(0, min(255, noised));
            data[idx].G = data[idx].B = data[idx].R;
            data[idx].A = 255; // Keep opaque by default.
        }
}

void myApplyUniformNoise(MyBGRA* data, MyBmpInfo* info, double start, double end)
{
    srand(time(NULL));

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            // Get a random variable of U(0, 1).
            double W = rand() / (double)RAND_MAX;

            UINT idx = i + j * info->nWidth;

            double noised = data[idx].R + (W * (end - start) + start);
            // Apply noise to target pixel data.
            data[idx].R = (UINT8)max(0, min(255, noised));
            data[idx].G = data[idx].B = data[idx].R;
            data[idx].A = 255; // Keep opaque by default.
        }
}

void myApplySaltAndPepperNoise(MyBGRA* data, MyBmpInfo* info, double salt, double pepper)
{
    srand(time(NULL));

    for (UINT i = 0; i < info->nWidth; ++i)
        for (UINT j = 0; j < info->nHeight; ++j)
        {
            // Get a random variable of U(0, 1) and remove probability of intact.
            double W = rand() / (double)RAND_MAX - (1 - salt - pepper);

            UINT idx = i + j * info->nWidth;

            if (W > 0)
            {
                if (W < salt)
                {
                    data[idx].R = 255;
                }
                else // pepper
                {
                    data[idx].R = 0;
                }
                data[idx].G = data[idx].B = data[idx].R;
                data[idx].A = 255; // Keep opaque by default.
            }
        }
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                                    App Icon Data |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

HICON myLoadAppIcon()
{
    static BYTE ICON_DATA[] =
    {
        0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, // We have removed the file header.
        0xFF, 0x00, 0xC5, 0xC5, 0xC5, 0x00, 0x77, 0x77, 0x77, 0x00, 0x65, 0x64, 0x65, 0x00, 0x65, 0x65,
        0x65, 0x00, 0x67, 0x67, 0x67, 0x00, 0x68, 0x68, 0x68, 0x00, 0x6A, 0x6A, 0x6A, 0x00, 0x6B, 0x6B,
        0x6B, 0x00, 0x6C, 0x6C, 0x6C, 0x00, 0x6D, 0x6D, 0x6D, 0x00, 0x6F, 0x6F, 0x6E, 0x00, 0x70, 0x70,
        0x70, 0x00, 0x71, 0x71, 0x71, 0x00, 0x72, 0x72, 0x72, 0x00, 0x74, 0x74, 0x74, 0x00, 0x75, 0x75,
        0x75, 0x00, 0x76, 0x76, 0x76, 0x00, 0x78, 0x78, 0x78, 0x00, 0x79, 0x79, 0x79, 0x00, 0x7A, 0x7A,
        0x7A, 0x00, 0x7C, 0x7C, 0x7C, 0x00, 0x7D, 0x7E, 0x7E, 0x00, 0x7E, 0x7E, 0x7E, 0x00, 0x7C, 0x7C,
        0x7C, 0x00, 0x85, 0x85, 0x85, 0x00, 0xC2, 0xC2, 0xC2, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x5A, 0x5A,
        0x5A, 0x00, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x2D, 0x2D, 0x00, 0x54, 0x54,
        0x54, 0x00, 0x56, 0x55, 0x55, 0x00, 0x56, 0x56, 0x56, 0x00, 0x58, 0x58, 0x58, 0x00, 0x5A, 0x5A,
        0x5A, 0x00, 0x5D, 0x5D, 0x5D, 0x00, 0x5F, 0x5F, 0x5F, 0x00, 0x61, 0x61, 0x61, 0x00, 0x64, 0x63,
        0x63, 0x00, 0x66, 0x66, 0x66, 0x00, 0x69, 0x69, 0x69, 0x00, 0x6B, 0x6B, 0x6B, 0x00, 0x6D, 0x6E,
        0x6E, 0x00, 0x70, 0x70, 0x70, 0x00, 0x73, 0x73, 0x73, 0x00, 0x75, 0x75, 0x75, 0x00, 0x78, 0x78,
        0x78, 0x00, 0x7B, 0x7A, 0x7B, 0x00, 0x7D, 0x7D, 0x7D, 0x00, 0x80, 0x80, 0x80, 0x00, 0x82, 0x82,
        0x82, 0x00, 0x86, 0x86, 0x86, 0x00, 0x88, 0x88, 0x88, 0x00, 0x7F, 0x7F, 0x7F, 0x00, 0xF9, 0xF9,
        0xF9, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x63, 0x63, 0x63, 0x00, 0x0A, 0x0A,
        0x0B, 0x00, 0x10, 0x0F, 0x0F, 0x00, 0xA2, 0xA2, 0xA2, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF7, 0xF7, 0xF6, 0x00, 0xA5, 0xA6,
        0xA6, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xC8, 0xC8, 0xC8, 0x00, 0x13, 0x13, 0x13, 0x00, 0x17, 0x17,
        0x17, 0x00, 0xD9, 0xD9, 0xD9, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xEF, 0xEF, 0xEF, 0x00, 0x84, 0x84, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0x96,
        0x96, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFB, 0xFA, 0xFA, 0x00, 0xFA, 0xFA,
        0xFA, 0x00, 0xFB, 0xFC, 0xFC, 0x00, 0x6A, 0x6A, 0x6A, 0x00, 0x26, 0x26, 0x26, 0x00, 0xEB, 0xEB,
        0xEB, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF1, 0xF1,
        0xFB, 0x00, 0xB6, 0xB6, 0xE9, 0x00, 0x88, 0x88, 0xDC, 0x00, 0x8A, 0x8A, 0xDC, 0x00, 0xD7, 0xD7,
        0xF3, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFC,
        0xFC, 0x00, 0xFB, 0xFC, 0xFB, 0x00, 0xFA, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9,
        0xF9, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0x66, 0x66, 0x66, 0x00, 0x4D, 0x4D, 0x4D, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA5, 0xA5,
        0xE5, 0x00, 0x00, 0x00, 0xAD, 0x00, 0x4B, 0x4B, 0xCA, 0x00, 0x4A, 0x4A, 0xCA, 0x00, 0x06, 0x06,
        0xB6, 0x00, 0x7A, 0x7A, 0xD8, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE,
        0xFE, 0x00, 0xFE, 0xFE, 0xFD, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFD, 0xFC, 0x00, 0xFC, 0xFB,
        0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8,
        0xF8, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0x66, 0x66, 0x66, 0x00, 0x54, 0x54, 0x54, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA1, 0xA1,
        0xE3, 0x00, 0x0F, 0x0F, 0xB9, 0x00, 0xF5, 0xF5, 0xFC, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA9, 0xA9,
        0xE6, 0x00, 0x00, 0x00, 0xAB, 0x00, 0xCF, 0xCF, 0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFE, 0xFD,
        0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFC, 0xFC, 0xFB, 0x00, 0xFB, 0xFB,
        0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7,
        0xF7, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0x67, 0x67, 0x67, 0x00, 0x56, 0x56, 0x56, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA3, 0xA3,
        0xE4, 0x00, 0x11, 0x11, 0xB9, 0x00, 0xF5, 0xF5, 0xFC, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xB9, 0xB9,
        0xEA, 0x00, 0x00, 0x00, 0xAD, 0x00, 0xD6, 0xD6, 0xF3, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xFC, 0xFD, 0xFC, 0x00, 0xFC, 0xFB, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA,
        0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF8, 0xF7, 0x00, 0xF6, 0xF6,
        0xF6, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0x68, 0x68, 0x68, 0x00, 0x58, 0x58, 0x58, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA9, 0xA9,
        0xE6, 0x00, 0x0D, 0x0D, 0xB8, 0x00, 0x97, 0x97, 0xE0, 0x00, 0x95, 0x95, 0xE0, 0x00, 0x2E, 0x2E,
        0xC1, 0x00, 0xA2, 0xA2, 0xE4, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFD, 0xFD,
        0xFC, 0x00, 0xFC, 0xFB, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9,
        0xF9, 0x00, 0xF8, 0xF8, 0xF9, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5,
        0xF5, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0x69, 0x69, 0x69, 0x00, 0x5A, 0x5A, 0x5A, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xAE, 0xAE,
        0xE7, 0x00, 0x0A, 0x0A, 0xB7, 0x00, 0x47, 0x47, 0xC9, 0x00, 0x2C, 0x2C, 0xC1, 0x00, 0x03, 0x03,
        0xB5, 0x00, 0xA9, 0xA9, 0xE5, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFF,
        0xFE, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFC, 0xFC,
        0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFB, 0x00, 0xFA, 0xF9, 0xFA, 0x00, 0xF9, 0xF9,
        0xF9, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF6, 0xF5, 0x00, 0xF4, 0xF5,
        0xF4, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0x6B, 0x6B, 0x6B, 0x00, 0x5C, 0x5C, 0x5C, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA3, 0xA3,
        0xE4, 0x00, 0x11, 0x11, 0xB9, 0x00, 0xF5, 0xF5, 0xFC, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xBA, 0xBA,
        0xEB, 0x00, 0x00, 0x00, 0xAB, 0x00, 0xAD, 0xAC, 0xE8, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xEA, 0xF4,
        0xEA, 0x00, 0xF4, 0xF9, 0xF4, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE,
        0xFE, 0x00, 0xFF, 0xFE, 0xFF, 0x00, 0xED, 0xF5, 0xED, 0x00, 0xEB, 0xF3, 0xEB, 0x00, 0xFC, 0xFB,
        0xFB, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF9, 0xF8, 0xF9, 0x00, 0xF8, 0xF8,
        0xF8, 0x00, 0xF6, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF3, 0xF3,
        0xF4, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0x6C, 0x6C, 0x6C, 0x00, 0x5F, 0x5F, 0x5F, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA3, 0xA3,
        0xE4, 0x00, 0x11, 0x11, 0xB9, 0x00, 0xF5, 0xF5, 0xFC, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE,
        0xFF, 0x00, 0x10, 0x10, 0xB8, 0x00, 0x85, 0x77, 0xEF, 0x00, 0xF2, 0xFF, 0xD5, 0x00, 0x00, 0x6F,
        0x00, 0x00, 0x4D, 0xA4, 0x4B, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xF3, 0xF8, 0xF3, 0x00, 0x11, 0x85, 0x0F, 0x00, 0x17, 0x88, 0x14, 0x00, 0xF5, 0xF8,
        0xF5, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF8, 0xF7, 0xF7, 0x00, 0xF7, 0xF7,
        0xF7, 0x00, 0xF5, 0xF6, 0xF6, 0x00, 0xF5, 0xF4, 0xF4, 0x00, 0xF4, 0xF3, 0xF4, 0x00, 0xF2, 0xF2,
        0xF3, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0x6D, 0x6D, 0x6D, 0x00, 0x61, 0x61, 0x61, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA1, 0xA1,
        0xE3, 0x00, 0x0E, 0x0E, 0xB9, 0x00, 0xF7, 0xF7, 0xFD, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xAF, 0xAF,
        0xE8, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xC9, 0xB8, 0xFF, 0x00, 0xD7, 0xFC, 0xBD, 0x00, 0x00, 0x73,
        0x00, 0x00, 0x07, 0x7E, 0x04, 0x00, 0xDC, 0xED, 0xDB, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xAA, 0xD3, 0xAA, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x15, 0x87, 0x13, 0x00, 0xE4, 0xEF,
        0xE4, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6,
        0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF2, 0xF2, 0xF3, 0x00, 0xF1, 0xF1,
        0xF1, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0x6E, 0x6E, 0x6E, 0x00, 0x63, 0x63, 0x63, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA5, 0xA5,
        0xE5, 0x00, 0x00, 0x00, 0xAD, 0x00, 0x3C, 0x3C, 0xC5, 0x00, 0x38, 0x38, 0xC4, 0x00, 0x02, 0x02,
        0xB5, 0x00, 0x75, 0x75, 0xD6, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA1, 0xD2, 0x9B, 0x00, 0x25, 0x8F,
        0x22, 0x00, 0x1A, 0x89, 0x18, 0x00, 0x86, 0xC1, 0x85, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0x54, 0xA7, 0x53, 0x00, 0x34, 0x96, 0x32, 0x00, 0x2C, 0x93, 0x29, 0x00, 0xCF, 0xE3,
        0xCE, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5,
        0xF5, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF3, 0xF2, 0xF3, 0x00, 0xF2, 0xF1, 0xF2, 0x00, 0xF0, 0xF0,
        0xF1, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0x70, 0x70, 0x70, 0x00, 0x65, 0x66, 0x65, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xEE, 0xEE,
        0xFA, 0x00, 0xB6, 0xB6, 0xE9, 0x00, 0x9B, 0x9B, 0xE1, 0x00, 0xA1, 0xA1, 0xE4, 0x00, 0xE2, 0xE2,
        0xF7, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x84, 0xC0, 0x82, 0x00, 0x41, 0x9E,
        0x3F, 0x00, 0x5F, 0xAC, 0x5E, 0x00, 0x2E, 0x94, 0x2C, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF2, 0xF7,
        0xF2, 0x00, 0x1D, 0x8A, 0x1A, 0x00, 0x94, 0xC7, 0x93, 0x00, 0x19, 0x89, 0x16, 0x00, 0xBE, 0xDB,
        0xBD, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF4,
        0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF2, 0xF2, 0xF2, 0x00, 0xF0, 0xF0, 0xF1, 0x00, 0xEF, 0xEF,
        0xF0, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0x71, 0x71, 0x71, 0x00, 0x68, 0x68, 0x68, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x71, 0xB6, 0x6F, 0x00, 0x4A, 0xA2,
        0x48, 0x00, 0xB9, 0xDA, 0xB8, 0x00, 0x00, 0x77, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xA9, 0xD1,
        0xA8, 0x00, 0x1B, 0x89, 0x1A, 0x00, 0xE5, 0xEF, 0xE4, 0x00, 0x05, 0x7F, 0x01, 0x00, 0xAD, 0xD3,
        0xAD, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF3, 0xF3,
        0xF3, 0x00, 0xF2, 0xF2, 0xF2, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xEF, 0xEF, 0xF0, 0x00, 0xEE, 0xEE,
        0xEE, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0x72, 0x72, 0x72, 0x00, 0x6A, 0x6A, 0x6A, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x5F, 0xAD, 0x5D, 0x00, 0x3E, 0x9C,
        0x3C, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x7C, 0x00, 0x00, 0xC9, 0xE2, 0xC8, 0x00, 0x67, 0xB1,
        0x66, 0x00, 0x57, 0xA8, 0x56, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x7D, 0x00, 0x00, 0x9F, 0xCB,
        0x9D, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF2, 0xF2,
        0xF2, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xEF, 0xF0, 0xF0, 0x00, 0xEE, 0xEE, 0xEF, 0x00, 0xED, 0xED,
        0xED, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0x73, 0x73, 0x73, 0x00, 0x6D, 0x6D, 0x6D, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFE, 0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x4E, 0xA4, 0x4C, 0x00, 0x3D, 0x9C,
        0x3B, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x40, 0x9D, 0x3F, 0x00, 0x53, 0xA5, 0x51, 0x00, 0x39, 0x98,
        0x37, 0x00, 0xAC, 0xD3, 0xAC, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x03, 0x7F, 0x00, 0x00, 0x8F, 0xC3,
        0x8E, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF5, 0xFB, 0xFB, 0x00, 0xF6, 0xFF, 0xFF, 0x00, 0xF4, 0xFF,
        0xFF, 0x00, 0xF3, 0xFF, 0xFF, 0x00, 0xF0, 0xFC, 0xFE, 0x00, 0xED, 0xED, 0xEE, 0x00, 0xEC, 0xEB,
        0xEB, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0x74, 0x75, 0x74, 0x00, 0x6F, 0x6F, 0x6F, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFE,
        0xFF, 0x00, 0xFF, 0xFE, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x3E, 0x9C, 0x3B, 0x00, 0x49, 0xA1,
        0x47, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x9E, 0xCC, 0x9D, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x30, 0x94,
        0x2E, 0x00, 0xFF, 0xFE, 0xFF, 0x00, 0xFF, 0xFC, 0xFF, 0x00, 0x08, 0x81, 0x05, 0x00, 0x80, 0xBC,
        0x7F, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xED, 0xD7, 0xCA, 0x00, 0xE6, 0xAE, 0x8D, 0x00, 0xE2, 0x97,
        0x6A, 0x00, 0xE4, 0xA5, 0x80, 0x00, 0xEB, 0xDD, 0xD5, 0x00, 0xEF, 0xFE, 0xFF, 0x00, 0xEB, 0xEB,
        0xEB, 0x00, 0xF1, 0xF1, 0xF2, 0x00, 0x76, 0x76, 0x76, 0x00, 0x71, 0x71, 0x71, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE,
        0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x11, 0x80, 0x11, 0x00, 0x48, 0x9E,
        0x48, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xE3, 0xF0, 0xE3, 0x00, 0x00, 0x61, 0x00, 0x00, 0x62, 0xAA,
        0x62, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFA, 0xFF, 0x00, 0x00, 0x75, 0x00, 0x00, 0x5C, 0xA8,
        0x5E, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xDF, 0x7F, 0x46, 0x00, 0xD5, 0x48, 0x00, 0x00, 0xDF, 0x8B,
        0x58, 0x00, 0xDC, 0x6C, 0x2B, 0x00, 0xD7, 0x47, 0x00, 0x00, 0xE5, 0xBD, 0xA3, 0x00, 0xED, 0xFE,
        0xFF, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0x77, 0x77, 0x77, 0x00, 0x74, 0x74, 0x74, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFE, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xFF, 0xFE, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x83, 0xBF, 0x81, 0x00, 0xAB, 0xD3,
        0xAA, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFC, 0xFE, 0x00, 0x92, 0xC5, 0x91, 0x00, 0xCA, 0xE1,
        0xCA, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFC, 0xF8, 0xFC, 0x00, 0x7F, 0xBB, 0x7D, 0x00, 0xA3, 0xCD,
        0xA4, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xDF, 0x7C, 0x43, 0x00, 0xDE, 0x80, 0x44, 0x00, 0xF4, 0xFF,
        0xFF, 0x00, 0xF1, 0xFF, 0xFF, 0x00, 0xDF, 0x89, 0x58, 0x00, 0xD8, 0x54, 0x04, 0x00, 0xE9, 0xE8,
        0xE8, 0x00, 0xF1, 0xF5, 0xF8, 0x00, 0x78, 0x78, 0x78, 0x00, 0x76, 0x76, 0x77, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFB, 0xFA,
        0xFB, 0x00, 0xF2, 0xF3, 0xF3, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFA, 0xFC,
        0xFC, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0xDE, 0x7F, 0x45, 0x00, 0xDE, 0x7C, 0x41, 0x00, 0xEF, 0xFC,
        0xFF, 0x00, 0xEF, 0xFF, 0xFF, 0x00, 0xE7, 0xCA, 0xB2, 0x00, 0xD7, 0x46, 0x00, 0x00, 0xE5, 0xD0,
        0xC7, 0x00, 0xF1, 0xF8, 0xFD, 0x00, 0x7A, 0x7A, 0x7A, 0x00, 0x79, 0x79, 0x79, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFD, 0xFD, 0xFC, 0x00, 0xFC, 0xFC,
        0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9, 0xFA, 0x00, 0xF8, 0xF8,
        0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5,
        0xF5, 0x00, 0xF4, 0xF3, 0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF1, 0xF1, 0xF2, 0x00, 0xF0, 0xF2,
        0xF3, 0x00, 0xF1, 0xFF, 0xFF, 0x00, 0xDE, 0x7C, 0x43, 0x00, 0xDE, 0x7F, 0x45, 0x00, 0xF1, 0xFF,
        0xFF, 0x00, 0xEC, 0xFF, 0xFF, 0x00, 0xDD, 0x7B, 0x40, 0x00, 0xDA, 0x5F, 0x15, 0x00, 0xE7, 0xED,
        0xF2, 0x00, 0xEF, 0xF1, 0xF4, 0x00, 0x7B, 0x7B, 0x7B, 0x00, 0x7B, 0x7B, 0x7C, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE,
        0xFE, 0x00, 0xFD, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB,
        0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xFA, 0xF9, 0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8,
        0xF8, 0x00, 0xF7, 0xF7, 0xF6, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF3,
        0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xF0, 0xF0, 0xF1, 0x00, 0xEF, 0xF1,
        0xF2, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0xE0, 0x8F, 0x5E, 0x00, 0xD9, 0x5A, 0x0E, 0x00, 0xDE, 0x7E,
        0x44, 0x00, 0xDA, 0x60, 0x17, 0x00, 0xD9, 0x50, 0x00, 0x00, 0xE5, 0xCD, 0xBF, 0x00, 0xE7, 0xF7,
        0xFF, 0x00, 0xEE, 0xEE, 0xEE, 0x00, 0x7D, 0x7D, 0x7C, 0x00, 0x7E, 0x7E, 0x7E, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFB,
        0xFA, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7,
        0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF3, 0xF4, 0x00, 0xF3, 0xF2,
        0xF3, 0x00, 0xF1, 0xF1, 0xF2, 0x00, 0xF0, 0xF1, 0xF1, 0x00, 0xEF, 0xEF, 0xEF, 0x00, 0xEE, 0xF0,
        0xF0, 0x00, 0xEF, 0xFE, 0xFF, 0x00, 0xDF, 0x89, 0x55, 0x00, 0xDA, 0x66, 0x20, 0x00, 0xE0, 0x9D,
        0x76, 0x00, 0xE1, 0xA7, 0x85, 0x00, 0xE6, 0xE2, 0xDF, 0x00, 0xE7, 0xF8, 0xFF, 0x00, 0xE4, 0xE2,
        0xE2, 0x00, 0xED, 0xEE, 0xEE, 0x00, 0x7E, 0x7D, 0x7E, 0x00, 0x80, 0x80, 0x80, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFE, 0x00, 0xFD, 0xFD,
        0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFB, 0xFA, 0xFB, 0x00, 0xF9, 0xF9,
        0xF9, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6,
        0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF3, 0xF4, 0xF4, 0x00, 0xF2, 0xF2, 0xF3, 0x00, 0xF1, 0xF2,
        0xF2, 0x00, 0xF0, 0xF0, 0xF1, 0x00, 0xF0, 0xEF, 0xEF, 0x00, 0xEE, 0xEE, 0xEE, 0x00, 0xED, 0xEE,
        0xEF, 0x00, 0xED, 0xFC, 0xFF, 0x00, 0xDD, 0x7B, 0x42, 0x00, 0xDD, 0x81, 0x45, 0x00, 0xED, 0xFF,
        0xFF, 0x00, 0xEA, 0xFF, 0xFF, 0x00, 0xE6, 0xEF, 0xF6, 0x00, 0xE4, 0xE3, 0xE2, 0x00, 0xE3, 0xE2,
        0xE3, 0x00, 0xEC, 0xEC, 0xED, 0x00, 0x90, 0x90, 0x90, 0x00, 0x83, 0x83, 0x83, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFD, 0xFE, 0xFE, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFD,
        0xFC, 0x00, 0xFC, 0xFB, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9,
        0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5,
        0xF5, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF2, 0xF2, 0xF2, 0x00, 0xF1, 0xF0,
        0xF1, 0x00, 0xEF, 0xEF, 0xF0, 0x00, 0xEE, 0xEE, 0xEE, 0x00, 0xED, 0xED, 0xEE, 0x00, 0xEC, 0xED,
        0xEE, 0x00, 0xEC, 0xFB, 0xFF, 0x00, 0xDD, 0x7B, 0x40, 0x00, 0xDC, 0x77, 0x3A, 0x00, 0xE8, 0xF6,
        0xFF, 0x00, 0xE6, 0xE7, 0xE8, 0x00, 0xE4, 0xE4, 0xE5, 0x00, 0xE3, 0xE3, 0xE3, 0x00, 0xE2, 0xE2,
        0xE2, 0x00, 0xEE, 0xEE, 0xEF, 0x00, 0xCA, 0xCA, 0xCA, 0x00, 0x82, 0x82, 0x82, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB,
        0xFB, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8,
        0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF4,
        0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF2, 0xF2, 0xF2, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xF0, 0xEF,
        0xF0, 0x00, 0xEE, 0xEE, 0xEF, 0x00, 0xED, 0xED, 0xED, 0x00, 0xEC, 0xEC, 0xEC, 0x00, 0xEB, 0xEC,
        0xED, 0x00, 0xEB, 0xFB, 0xFF, 0x00, 0xDD, 0x78, 0x3C, 0x00, 0xDC, 0x74, 0x36, 0x00, 0xE7, 0xF5,
        0xFE, 0x00, 0xE5, 0xE6, 0xE7, 0x00, 0xE3, 0xE3, 0xE3, 0x00, 0xE2, 0xE2, 0xE2, 0x00, 0xE0, 0xE0,
        0xE1, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x84, 0x84, 0x85, 0x00, 0xF0, 0xF1,
        0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFD, 0xFC, 0xFC, 0x00, 0xFC, 0xFC, 0xFB, 0x00, 0xFB, 0xFB,
        0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7,
        0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF3, 0xF3,
        0xF3, 0x00, 0xF2, 0xF2, 0xF2, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xF0, 0xF0, 0xF0, 0x00, 0xEF, 0xEF,
        0xEF, 0x00, 0xEE, 0xED, 0xEE, 0x00, 0xEC, 0xEC, 0xEC, 0x00, 0xEB, 0xEB, 0xEB, 0x00, 0xEA, 0xEA,
        0xEA, 0x00, 0xE9, 0xEB, 0xED, 0x00, 0xE5, 0xD6, 0xCD, 0x00, 0xE4, 0xD4, 0xCB, 0x00, 0xE5, 0xE7,
        0xE9, 0x00, 0xE3, 0xE3, 0xE4, 0x00, 0xE2, 0xE2, 0xE2, 0x00, 0xE0, 0xE0, 0xE1, 0x00, 0xE8, 0xE8,
        0xE8, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xAC, 0xAC,
        0xAC, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFA, 0xFA,
        0xFA, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6,
        0xF6, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF5, 0xF4, 0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF2, 0xF2,
        0xF2, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xF0, 0xF0, 0xF0, 0x00, 0xEF, 0xEE, 0xEF, 0x00, 0xED, 0xED,
        0xEE, 0x00, 0xEC, 0xEC, 0xED, 0x00, 0xEB, 0xEB, 0xEB, 0x00, 0xE9, 0xEA, 0xEA, 0x00, 0xE8, 0xE8,
        0xE9, 0x00, 0xE7, 0xE5, 0xE5, 0x00, 0xE7, 0xF1, 0xF8, 0x00, 0xE5, 0xF0, 0xF7, 0x00, 0xE3, 0xE2,
        0xE1, 0x00, 0xE2, 0xE2, 0xE2, 0x00, 0xE0, 0xE1, 0xE1, 0x00, 0xE7, 0xE7, 0xE7, 0x00, 0xFE, 0xFE,
        0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFB, 0xFB,
        0xFB, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xFA, 0xFA, 0xFA, 0x00, 0xFA, 0xF9, 0xFA, 0x00, 0xF8, 0xF8,
        0xF8, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0xF5, 0xF5,
        0xF5, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF2, 0xF2,
        0xF2, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xEF, 0xEF, 0xF0, 0x00, 0xEF, 0xEF,
        0xEF, 0x00, 0xEE, 0xEE, 0xEE, 0x00, 0xED, 0xED, 0xED, 0x00, 0xEC, 0xEC, 0xEC, 0x00, 0xEA, 0xEA,
        0xEA, 0x00, 0xEC, 0xEC, 0xEC, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00
    };
    gAppIconData = (MyBGRA*)ICON_DATA;
    // 0: origin color (opaque). 1: screen color (tranparent).
    static BYTE MASK_DATA[] =
    {
        0xFF, 0xFF, 0xFF, 0xFF,
        0xC0, 0x00, 0x00, 0x03,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0xC0, 0x00, 0x00, 0x03,
        0xFF, 0xFF, 0xFF, 0xFF,
    };
    return CreateIcon(gInstance, gCxIcon, gCyIcon, 1, 32, MASK_DATA, ICON_DATA);
}
