#include "BitmapIO.h"

#include <assert.h>
// common dialog
#include <commdlg.h>
#include <math.h> // for pow
#include <stdlib.h> // for rand
#include <stdio.h> // for scanf
#include <time.h> // for time

// Disable type casting warnings.
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
// Disable security func warnings.
#pragma warning(disable : 4996)
// Disable memory uninited warnings.
#pragma warning(disable : 6001)
// Disable buffer read/write warnings.
#pragma warning(disable : 6385)
#pragma warning(disable : 6386)

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

// Application Handle
HINSTANCE gInstance;

// Main window & Child windows
HWND gMainWnd,
     gSecondWnd,
     gGrayTransWnd,
     gGtwHistDispWnd,
     gDomainFilterWnd,
     gDomainCustCoreWnd;

HWND gAppAboutWnd;

// Image data of main window
MyBGRA* gImage = NULL;
MyBmpInfo gBmpInfo;

// System metrics info
UINT gCxIcon = 0, gCyIcon = 0;
UINT gCaptionHeight = 0;
UINT gCharWidth = 0, gCharHeight = 0;
UINT gCxVertScr = 0, gCyVertScr = 0;
UINT gCxHorzScr = 0, gCyHorzScr = 0;

// App Icon
HICON gIcon;
MyBGRA* gAppIconData;

// Main window menu
HMENU gMenu;

// Whether the image of main window is connected to an external file.
BOOL gHasExternalFile = FALSE;
// Used for common dialog. gFileName (full path), gTitleName(body name).
WCHAR gFileName[MAX_PATH], gTitleName[MAX_PATH];

// Child Window ID
#define SECOND_WND 100
#define GRAY_TRANS_WND 101
#define GTW_HIST_DISP_WND 102
#define DOMAIN_FILTER_WND 103
#define DOMAIN_CUST_CORE_WND 104
#define APP_ABOUT_WND 110

// Main Menu ID
//-------------------------------------
#define IDM_FILE 0
#define IDM_FILE_NEW 1001
#define IDM_FILE_OPEN 1002
#define IDM_FILE_SECOND 1003
#define IDM_FILE_SAVE 1004
#define IDM_FILE_SAVE_AS 1005
#define IDM_FILE_EXP_TXT 1006
//-------------------------------------
#define IDM_GRAY 1
// empirical formula
#define IDM_GRAY_EMPI 1011
#define IDM_GRAY_EVEN 1012
// gamma correction
#define IDM_GRAY_GAMMA 1013
#define IDM_GRAY_R 1014
#define IDM_GRAY_G 1015
#define IDM_GRAY_B 1016
//-------------------------------------
#define IDM_DOMAIN 2
// SPAF: SPAtial Filter
#define IDM_SPAF_BOX 1021
// gaussian core
#define IDM_SPAF_GAUS 1022
// median value
#define IDM_SPAF_MEDI 1023
// 2nd derivative
#define IDM_SPAF_LAPLACE 1024
// 1st derivative
#define IDM_SPAF_SOBEL 1025
// SPEF: SPEctral Filter
// custom core
#define IDM_SPAF_CUST 1026
#define IDM_SPEF_CUST 1027
//-------------------------------------
#define IDM_HELP 3
#define IDM_EASTER_EGG 1031
#define IDM_APP_ABOUT 1032

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

HICON myLoadAppIcon();

HMENU myLoadMainMenu();
UINT myValidWndCount();

RECT myGetSecondWndInitSize();

// Operation menus are those that can export or modify the image of main window.
void myEnableOperationMenus(BOOL bIsGrayScale);
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
// @param pData: points to image pixel data.
// @param pInfo: points to image pixel info.
//
void myDisplayImage(HDC hdc, int offsetX, int offsetY, MyBGRA* pData, MyBmpInfo* pInfo);

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
//
// @param pData: the origin image to expand.
// @param pInfo: points to image pixel info.
// @param nExpandSize: how far out should expand.
// 
// @return the mirrored pixel data buffer.
//
MyBGRA* myExpandMirrorImage(MyBGRA* pData, MyBmpInfo* pInfo, UINT nExpandSize);

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

//-------------------------------------------------------------------------------------------------
// Main Function
//-------------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
{
    MSG msg;
    WNDCLASS wndclass;

    gInstance = hInstance;

    gCxIcon = GetSystemMetrics(SM_CXICON);
    gCyIcon = GetSystemMetrics(SM_CYICON);

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

    gMenu = myLoadMainMenu();
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

    ShowWindow(gMainWnd, nShowCmd);
    UpdateWindow(gMainWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    switch (message)
    {
    case WM_CREATE:
    {
        // Query system metrics info.
        gCaptionHeight = GetSystemMetrics(SM_CYCAPTION);

        gCharWidth = LOWORD(GetDialogBaseUnits());
        gCharHeight = HIWORD(GetDialogBaseUnits());

        gCxVertScr = GetSystemMetrics(SM_CXVSCROLL);
        gCyVertScr = GetSystemMetrics(SM_CYVSCROLL);
        gCxHorzScr = GetSystemMetrics(SM_CXHSCROLL);
        gCyHorzScr = GetSystemMetrics(SM_CYHSCROLL);

        // Initialize common dialog utils.
        myInitFileDialogInfo(hwnd);

        // Grayed all operations until user open an image.
        myDisableOperationMenus();
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

    case WM_SETFOCUS:
        // Close app about window if focused.
        if (IsWindow(gAppAboutWnd)) DestroyWindow(gAppAboutWnd);
        return 0;

    case WM_PAINT:
    {
        hdc = BeginPaint(hwnd, &ps);

        int x, y;
        if (gImage == NULL)
        {
            // Display hint text in the center of main window.
            x = ((int)width - (int)(18 * gCharWidth)) / 2;
            y = ((int)height - (int)(2 * gCharHeight)) / 2;
            TextOut(hdc, x, y, L"新建或打开一个图片", 9);
        }
        else
        {
            // Stick image to left top corner of main window.
            myDisplayImage(hdc, 18, 0, gImage, &gBmpInfo);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDM_FILE_NEW: // TODO: support create image.
        {
            MessageBox(hwnd, L"抱歉, 我们暂时不支持新建图片文件!", L"提示", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        case IDM_FILE_OPEN:
        {
            // Show Open-File dialog.
            if (myOpenFileDialog(hwnd, gFileName, gTitleName))
            {
                // Update main window caption.
                SetWindowText(hwnd, gFileName); 
                // Now the image has been connected with an external file.
                gHasExternalFile = TRUE;

                // Replace main window background with selected image.
                if (gImage != NULL) free(gImage);
                gImage = myReadBmp(gFileName, &gBmpInfo);
                myAssert(gImage);
                InvalidateRect(hwnd, NULL, TRUE);

                // Resize main window to suit selected image.
                RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 18 };
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                MoveWindow(hwnd, left, top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

                // Have fun with operations.
                myEnableOperationMenus(myIsImageGrayScale(gImage, &gBmpInfo));
                DrawMenuBar(hwnd);
            }
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
        case IDM_SPAF_BOX:
        case IDM_SPAF_GAUS:
        case IDM_SPAF_MEDI:
        case IDM_SPAF_LAPLACE:
        case IDM_SPAF_SOBEL:
        {
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
                // Pass core-type to child window to create different widgets.
                (LPVOID)LOWORD(wParam));
            return 0;
        }
        case IDM_SPAF_CUST:
        {
            gDomainCustCoreWnd = CreateWindow(
                gDomainCustCoreWndName,
                NULL,
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                gMainWnd,
                NULL,
                gInstance,
                NULL);
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

                UINT nExpandSize = min(gBmpInfo.nWidth, gBmpInfo.nHeight) / 2;
                MyBGRA* tmpImage = myExpandMirrorImage(gImage, &gBmpInfo, nExpandSize);
                myAssert(tmpImage);

                // Destroy the precious image of evil user!
                free(gImage);
                gImage = tmpImage;
                gBmpInfo.nWidth += 2 * nExpandSize;
                gBmpInfo.nHeight += 2 * nExpandSize;

                goto resize_and_repaint_main_window;
            }

            // Trigger final easter egg.
            if (nUserSelectCnt == 6)
            {
                SetWindowText(hwnd, L"邪恶的用户!");

                MyBGRA* tmpImage = (MyBGRA*)malloc(65536 * sizeof(MyBGRA));
                myAssert(tmpImage);

                srand((unsigned int)time(NULL));
                // Populate with random colors.
                for (UINT i = 0; i < 256; ++i)
                    for (UINT j = 0; j < 256; ++j)
                    {
                        UINT idx = i + j * 256;
                        tmpImage[idx].R = rand() % 256;
                        tmpImage[idx].G = rand() % 256;
                        tmpImage[idx].B = rand() % 256;
                        tmpImage[idx].A = rand() % 256;
                    }

                // Destroy the precious image of evil user!
                free(gImage);
                gImage = tmpImage;
                gBmpInfo.nWidth = 256;
                gBmpInfo.nHeight = 256;

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
            // Update main menu.
            myEnableOperationMenus(myIsImageGrayScale(gImage, &gBmpInfo));
            DrawMenuBar(hwnd);
            // Resize main window to suit generated image.
            RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 18 };
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
            MoveWindow(hwnd, left, top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
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
        default:
            return 0;
        }
    }
    case WM_DESTROY:
        if (gImage != NULL) free(gImage);
        gImage = NULL; // A good habit.
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

LRESULT CALLBACK SecondWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;
    static MyBmpInfo bmpInfo;

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
            size_t nPixelByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
            image = (MyBGRA*)malloc(nPixelByteSize);
            myAssert(image);
            bmpInfo = gBmpInfo;
            memcpy(image, gImage, nPixelByteSize);
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
            12,
            tmpVert,
            tmpBtnW,
            tmpBtnH,
            hwnd,
            (HMENU)SUBW_MAIN_TO_SUB_BTN,
            gInstance,
            NULL);

        int tmpLastHorz = 12 + tmpBtnW + 5;
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
            bmpInfo.nWidth + 24, \
            Btn_Idx * (tmpBtnH + 5), \
            12 * gCharWidth, \
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

        MY_STICK_BOTTOM_BTN(btnMainToSub, 12);
        MY_STICK_BOTTOM_BTN(btnSubToMain, 17 + 10 * gCharWidth);
        MY_STICK_BOTTOM_BTN(btnImportImage, 22 + 20 * gCharWidth);

#undef MY_STICK_BOTTOM_BTN

        // Keep those buttons attached to window right.
#define MY_STICK_BUTTON_RIGHT(Btn_Handle, Btn_Idx) \
        GetWindowRect(Btn_Handle, &rc); \
        MoveWindow(Btn_Handle, clntWidth - 12 - 12 * gCharWidth, \
                   Btn_Idx * (5 + 2 * gCharHeight), \
                   rc.right - rc.left, rc.bottom - rc.top, TRUE)

        MY_STICK_BUTTON_RIGHT(btnInvertPixel, 0);

        MY_STICK_BUTTON_RIGHT(btnMainAddSub, 1);

        MY_STICK_BUTTON_RIGHT(btnMainSubSub, 2);
        MY_STICK_BUTTON_RIGHT(btnSubSubMain, 3);

        MY_STICK_BUTTON_RIGHT(btnMainMulSub, 4);

        MY_STICK_BUTTON_RIGHT(btnMainDivSub, 5);
        MY_STICK_BUTTON_RIGHT(btnSubDivMain, 6);

#undef MY_STICK_BUTTON_RIGHT

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        myDisplayImage(hdc, 12, 0, image, &bmpInfo);

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

            // Sub-image must not be time at this point.
            free(image);
            size_t nPixelByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
            image = (MyBGRA*)malloc(nPixelByteSize);
            myAssert(image);
            bmpInfo = gBmpInfo;
            memcpy(image, gImage, nPixelByteSize);

            // Resize to suit copyed image.
            RECT rc = myGetSecondWndInitSize();
            int tmpW = rc.right - rc.left;
            int tmpH = rc.bottom - rc.top;
            GetWindowRect(hwnd, &rc);
            MoveWindow(hwnd, rc.left, rc.top, tmpW, tmpH, TRUE);
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

            // Main-image might be time at this point.
            if (gImage != NULL) free(gImage);
            size_t nPixelByteSize = bmpInfo.nWidth * bmpInfo.nHeight * sizeof(MyBGRA);
            gImage = (MyBGRA*)malloc(nPixelByteSize);
            myAssert(gImage);
            gBmpInfo = bmpInfo;
            memcpy(gImage, image, nPixelByteSize);

            // Resize main window to suit copyed image.
            RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 18 };
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
            int tmpW = rc.right - rc.left;
            int tmpH = rc.bottom - rc.top;
            GetWindowRect(gMainWnd, &rc);
            MoveWindow(gMainWnd, rc.left, rc.top, tmpW, tmpH, TRUE);

            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            // Update operation menu state.
            myEnableOperationMenus(TRUE);
            DrawMenuBar(gMainWnd);

            // Repaint main window image.
            InvalidateRect(gMainWnd, NULL, TRUE);
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
                myAssert(tmpImage);
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

                // Resize to suit selected image.
                RECT rc = { 0, 0,
                    bmpInfo.nWidth + 36 + 12 * gCharWidth,
                    bmpInfo.nHeight + 2 * gCharHeight + 12 };
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
                int tmpW = rc.right - rc.left;
                int tmpH = rc.bottom - rc.top;
                GetWindowRect(hwnd, &rc);
                MoveWindow(hwnd, rc.left, rc.top, tmpW, tmpH, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        case SUBW_INVERT_PIXEL_BTN: // invert gray scale
        {
            for (UINT i = 0; i < bmpInfo.nWidth; ++i)
                for (UINT j = 0; j < bmpInfo.nHeight; ++j)
                {
                    UINT idx = i + j * bmpInfo.nWidth;
                    image[idx].R = image[idx].G = image[idx].B = (255 - image[idx].R);
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
    InvalidateRect(hwnd, NULL, TRUE)

        case SUBW_MAIN_ADD_SUB_BTN: // main + sub
        {
            MY_CHECK_MAIN_SUB_IMG; UINT i, j, idx;

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
            return 0;
        }
        case SUBW_MAIN_SUB_SUB_BTN: // main - sub
        {
            MY_CHECK_MAIN_SUB_IMG; UINT i, j, idx;

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
            return 0;
        }
        case SUBW_SUB_SUB_MAIN_BTN: // sub - main
        {
            MY_CHECK_MAIN_SUB_IMG; UINT i, j, idx;

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
            return 0;
        }
        case SUBW_MAIN_MUL_SUB_BTN: // main * sub
        {
            MY_CHECK_MAIN_SUB_IMG; UINT i, j, idx;

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
            return 0;
        }
        case SUBW_MAIN_DIV_SUB_BTN: // main / sub
        {
            MY_CHECK_MAIN_SUB_IMG; UINT i, j, idx;

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
            return 0;
        }
        case SUBW_SUB_DIV_MAIN_BTN: // sub / main
        {
            MY_CHECK_MAIN_SUB_IMG; UINT i, j, idx;

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
            return 0;
        }

#undef MY_NORMALIZE_SUB_IMG
#undef MY_CHECK_MAIN_SUB_IMG

        default:
            return 0;
        }
    main_window_empty:
        MessageBox(hwnd, L"主窗口为空, 无法进行操作.", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    main_sub_size_not_matched:
        MessageBox(hwnd, L"主-副图像尺寸不匹配, 无法进行操作.", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    case WM_DESTROY:
        // Note we have allocated memory for image data.
        free(image);
        // Keep focus on main window.
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

        myDisplayImage(hdc, 12, 0, image, &gBmpInfo);

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
            myEnableOperationMenus(TRUE);
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
// Label range: 10310 ~ 10319
#define DOMAIN_GENE_PARAM_LBL 10310
// Edit line range: 10320 ~ 10329
#define DOMAIN_GENE_PARAM_EDT 10320

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
            xLeftPos + gCharWidth,
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
            5 * gCharWidth,
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

LRESULT CALLBACK DomainFilterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;
    
    static HWND btnApply;
    static UINT nFilterType;

    static HWND btnSaveMain;

    static HWND b3sBorderMode;
    // Decide the filter how to populate border pixels (BM: Border Mode).
    // We borrow the flags of auto-3-state-button opportunely.

#define MY_SPAF_BM_BLACK ( BST_UNCHECKED )
#define MY_SPAF_BM_MIRROR ( BST_CHECKED )
#define MY_SPAF_BM_DUPLICATE ( BST_INDETERMINATE )

    static UINT nBorderMode = MY_SPAF_BM_MIRROR;

    static HWND lblParam[10];
    static HWND edtParam[10];
    // We store [int] and [double] value arrays separately here,
    // as one text can be translated to different numeric types.
    static int iParamVal[10];
    static double dParamVal[10];
    
    // How many key-value pairs need for each filter type?
    static int nParamCount[] =
    {
        1, // Box Filter
        2, // Gaussian Filter
        1, // Median Filter
        0, // Laplace Filter
        0, // Sobel Filter
    };

    switch (message)
    {
    case WM_CREATE:
        LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
        // Get selected spatial filter type.
        nFilterType = (UINT)pCreateStruct->lpCreateParams;

        // We decide that user can only open one window with the same type at the same time.
        EnableMenuItem(gMenu, IDM_FILE, MF_GRAYED | MF_BYPOSITION);
        EnableMenuItem(gMenu, IDM_DOMAIN, MF_GRAYED | MF_BYPOSITION);

        // Allocate memory for image data.
        size_t nImageByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
        image = (MyBGRA*)malloc(nImageByteSize);
        myAssert(image);
        memcpy(image, gImage, nImageByteSize);

        // Acquire selected menu item text.
        WCHAR szMenuText[20] = L"卷积滤波 - ";
        GetMenuString(gMenu, nFilterType, szMenuText + 7, 13, MF_BYCOMMAND);
        // Set child window caption text.
        SetWindowText(hwnd, szMenuText);

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

        // Create related parameter widget lines for specific filter type.
        myCreateParamWidgets(hwnd, nParamCount[nFilterType - IDM_SPAF_BOX],
                             gBmpInfo.nWidth + 24, lblParam, edtParam);

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
            SetWindowText(lblParam[0], L"强度 (1-sigma)");
            SetWindowText(edtParam[0], L"1");
            dParamVal[0] = 1; // standard deviation
            SetWindowText(lblParam[1], L"半径 (3-sigma)");
            SetWindowText(edtParam[1], L"3");
            iParamVal[1] = 3; // would better be 3-sigma
            break;
        }
        case IDM_SPAF_MEDI: // Median Filter
        {
            SetWindowText(lblParam[0], L"覆盖半径");
            SetWindowText(edtParam[0], L"1");
            iParamVal[0] = 1;
            return 0;
        }
        default:
            break;
        }
        return 0;

    case WM_MOVE:
        left = LOWORD(lParam);
        top = HIWORD(lParam);
        return 0;

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        UINT clntWidth = rc.right - rc.left;
        UINT clntHeight = rc.bottom - rc.top;

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

        // Keep widget lines stick to right border of window.
        myMoveParamWidgets(nParamCount[nFilterType - IDM_SPAF_BOX],
                           clntWidth - 12 - 24 * gCharWidth, lblParam, edtParam);

        width = LOWORD(lParam);
        height = HIWORD(lParam);
        return 0;
    }
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        // Display image on left-top corner.
        myDisplayImage(hdc, 12, 0, image, &gBmpInfo);

        EndPaint(hwnd, &ps);
        return 0;

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
            default:
                break;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case DOMAIN_GENE_SAVE_MAIN_BTN:
        {
            memcpy(gImage, image, gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA));

            // Notify user to save change.
            WCHAR title[MAX_PATH + 20];
            wsprintf(title, L"%s - (未保存)", gHasExternalFile ? gFileName : L"Untitled");
            SetWindowText(gMainWnd, title);

            // Repaint main window image.
            InvalidateRect(gMainWnd, NULL, TRUE);
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
        // Max support 10 different key-value pairs.
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
        {
            int idxParam = (int)(LOWORD(wParam) - DOMAIN_GENE_PARAM_EDT);

            WCHAR szParam[10];
            GetWindowText(edtParam[idxParam], szParam, 10);

            int iUserInput;
            float fUserInput;
            // Populate parameter with user input.
            if (swscanf(szParam, L"%d", &iUserInput) == 1)
                iParamVal[idxParam] = iUserInput;
            if (swscanf(szParam, L"%f", &fUserInput) == 1)
                dParamVal[idxParam] = (double)fUserInput;

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
                    // Make sure standard deviation is positive.
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
            default:
                return 0;
            }
        }
        default:
            return 0;
        }
    }
    case WM_DESTROY:
        // Restore config variables.
        nBorderMode = MY_SPAF_BM_MIRROR;
        // Note we have allocated memory for image data.
        free(image);
        // If this is the last child window to destroy.
        if (myValidWndCount() == 2)
        {
            EnableMenuItem(gMenu, IDM_FILE, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
        }
        // Make the menu selectable again since the window is closed.
        EnableMenuItem(gMenu, IDM_DOMAIN, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
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

    static WCHAR szAppVersion[] = L"BitmapViewer 位图工具 v1.0";
    static WCHAR szAuthorCopy[] = L"Copyleft (cl) 2022 文亦尧";

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

        // Option   ID                  String
        OPT_TOPLEV,                     L'文', L'件', 0,
        OPT_MIDDLE, IDM_FILE_NEW,       L'新', L'建', 0,
        OPT_MIDDLE, IDM_FILE_OPEN,      L'打', L'开', 0,
        OPT_______,
        OPT_MIDDLE, IDM_FILE_SECOND,    L'第', L'二', L'窗', L'口', 0,
        OPT_______,
        OPT_MIDDLE, IDM_FILE_SAVE,      L'保', L'存', 0,
        OPT_MIDDLE, IDM_FILE_SAVE_AS,   L'另', L'存', L'为', 0,
        OPT_______,
        OPT_MIDEND, IDM_FILE_EXP_TXT,   L'导', L'出', L'T', L'X', L'T', 0,

        OPT_TOPLEV,                     L'灰', L'度', L'变', L'换', 0,
        OPT_MIDDLE, IDM_GRAY_EMPI,      L'经', L'验', L'公', L'式', 0,
        OPT_MIDDLE, IDM_GRAY_EVEN,      L'算', L'数', L'平', L'均', 0,
        OPT_MIDDLE, IDM_GRAY_GAMMA,     L'伽', L'马', L'校', L'正', 0,
        OPT_______,
        OPT_MIDDLE, IDM_GRAY_R,         L'R', L'通', L'道', 0,
        OPT_MIDDLE, IDM_GRAY_G,         L'G', L'通', L'道', 0,
        OPT_MIDEND, IDM_GRAY_B,         L'B', L'通', L'道', 0,

        OPT_TOPLEV,                     L'卷', L'积', L'滤', L'波', 0,
        OPT_MIDDLE, IDM_SPAF_BOX,       L'空', L'域', L'_', L'盒', L'式', L'平', L'均', 0,
        OPT_MIDDLE, IDM_SPAF_GAUS,      L'空', L'域', L'_', L'低', L'通', L'高', L'斯', 0,
        OPT_MIDDLE, IDM_SPAF_MEDI,      L'空', L'域', L'_', L'中', L'值', L'降', L'噪', 0,
        OPT_MIDDLE, IDM_SPAF_LAPLACE,   L'空', L'域', L'_', L'拉', L'普', L'拉', L'斯', 0,
        OPT_MIDDLE, IDM_SPAF_SOBEL,     L'空', L'域', L'_', L'索', L'贝', L'尔', 0,
        OPT_______,
        OPT_MIDEND, IDM_SPAF_CUST,      L'空', L'域', L'-', L'自', L'定', L'义', L'核', 0,

#define MY_EASTER_EGG_TEXT \
L'~', L'%', L'?', L'…', L',', L'#', L' ', L'*', L'\'', L'☆', L'&', L'℃', L'$', L'︿', L'★', L'?'

        OPT_TOPEND,                     L'帮', L'助', 0,
        OPT_MIDDLE, IDM_EASTER_EGG,     MY_EASTER_EGG_TEXT, 0,
        OPT_______,
        OPT_MIDEND, IDM_APP_ABOUT,      L'关', L'于', 0

#undef MY_EASTER_EGG_TEXT

#undef OPT_______
#undef OPT_MIDDLE
#undef OPT_TOPLEV
    };
    return LoadMenuIndirect(MENU_TEMPLATE);
}

UINT myValidWndCount()
{
    UINT cnt = 0;

#define MY_CNT_WND(Wnd_Handle) if (IsWindow(Wnd_Handle)) ++cnt;

    MY_CNT_WND(gMainWnd)
    MY_CNT_WND(gGrayTransWnd)
    MY_CNT_WND(gGtwHistDispWnd)
    MY_CNT_WND(gDomainFilterWnd)
    MY_CNT_WND(gDomainCustCoreWnd)

#undef MY_CNT_WND

    return cnt;
}

RECT myGetSecondWndInitSize()
{
    RECT rc = { 0, 0,
        ((gImage == NULL) ? 256 : gBmpInfo.nWidth) + 36 + 12 * gCharWidth,
        ((gImage == NULL) ? 256 : gBmpInfo.nHeight) + 2 * gCharHeight + 12 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

void myEnableOperationMenus(BOOL bIsGrayScale)
{
    EnableMenuItem(gMenu, IDM_FILE_SAVE, MF_ENABLED | MF_HILITE | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_FILE_SAVE_AS, MF_ENABLED | MF_HILITE | MF_BYCOMMAND);

    UINT nEnable = bIsGrayScale ? (MF_ENABLED | MF_HILITE) : MF_GRAYED;
    UINT nDisable = bIsGrayScale ? MF_GRAYED : (MF_ENABLED | MF_HILITE);

    EnableMenuItem(gMenu, IDM_GRAY, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
    // There's no need to compute grayscale for a grayscale image.
    EnableMenuItem(gMenu, IDM_GRAY_EMPI, nDisable | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_GRAY_EVEN, nDisable | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_GRAY_GAMMA, nDisable | MF_BYCOMMAND);
    // We don't support these operations for colorful image.
    EnableMenuItem(gMenu, IDM_DOMAIN, nEnable | MF_BYPOSITION);
}

void myDisableOperationMenus()
{
    EnableMenuItem(gMenu, IDM_FILE_SAVE, MF_GRAYED | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_FILE_SAVE_AS, MF_GRAYED | MF_BYCOMMAND);

    EnableMenuItem(gMenu, IDM_GRAY, MF_GRAYED | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_DOMAIN, MF_GRAYED | MF_BYPOSITION);
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

void myDisplayImage(HDC hdc, int offsetX, int offsetY, MyBGRA* pData, MyBmpInfo* pInfo)
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
        pInfo->nWidth, // source rectangle width
        pInfo->nHeight, // source rectangle height
        0, // x source coordinate
        0, // y source coordinate
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
    RECT rc = { 0, 0, gBmpInfo.nWidth + 36 + 24 * gCharWidth, gBmpInfo.nHeight + 2 * gCharHeight + 12 };
    // Note this func wouldn't take WS_VSCROLL into consideration.
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

MyBGRA* myExpandMirrorImage(MyBGRA* pData, MyBmpInfo* pInfo, UINT nExpandSize)
{
    // Make sure expand-size not greater than origin size.
    assert(nExpandSize <= min(pInfo->nWidth, pInfo->nHeight));

    UINT i, j, srcIdx, dstIdx; // Declare common index variables here.

    UINT nBufferWidth = pInfo->nWidth + 2 * nExpandSize;
    UINT nBufferHeight = pInfo->nHeight + 2 * nExpandSize;
    UINT nBufferPixelCount = nBufferWidth * nBufferHeight;

    // Allocate memory for expand buffer.
    MyBGRA* buffer = (MyBGRA*)malloc(nBufferPixelCount * sizeof(MyBGRA));
    myAssert(buffer);

    // The body part should be the same.
    for (i = 0 ; i < pInfo->nWidth; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            srcIdx = i + j * pInfo->nWidth;
            dstIdx = nExpandSize + i + (nExpandSize + j) * nBufferWidth;
            buffer[dstIdx] = pData[srcIdx];
        }

#define LEFT_SRC_X ( nExpandSize - i - 1 )
#define LEFT_DST_X ( i )

#define RIGHT_SRC_X ( pInfo->nWidth - i - 1 )
#define RIGHT_DST_X ( nExpandSize + pInfo->nWidth + i )

#define TOP_SRC_Y ( nExpandSize - j - 1 ) * pInfo->nWidth
#define TOP_DST_Y ( j * nBufferWidth )

#define BOTTOM_SRC_Y ( pInfo->nHeight - j - 1 ) * pInfo->nWidth
#define BOTTOM_DST_Y ( (nExpandSize + pInfo->nHeight + j) * nBufferWidth )

    for (i = 0; i < nExpandSize; ++i)
        for (j = 0; j < pInfo->nHeight; ++j)
        {
            // Left Border
            srcIdx = LEFT_SRC_X + j * pInfo->nWidth;
            dstIdx = LEFT_DST_X + (nExpandSize + j) * nBufferWidth;
            buffer[dstIdx] = pData[srcIdx];
            // Right Border
            srcIdx = RIGHT_SRC_X + j * pInfo->nWidth;
            dstIdx = RIGHT_DST_X + (nExpandSize + j) * nBufferWidth;
            buffer[dstIdx] = pData[srcIdx];
        }
    for (i = 0; i < pInfo->nWidth; ++i)
        for (j = 0; j < nExpandSize; ++j)
        {
            // Top Border
            srcIdx = i + TOP_SRC_Y;
            dstIdx = (nExpandSize + i) + TOP_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
            // Bottom Border
            srcIdx = i + BOTTOM_SRC_Y;
            dstIdx = (nExpandSize + i) + BOTTOM_DST_Y;
            buffer[dstIdx] = pData[srcIdx];
        }
    for (i = 0; i < nExpandSize; ++i)
        for (j = 0; j < nExpandSize; ++j)
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