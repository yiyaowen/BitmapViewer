#include "BitmapIO.h"

// common dialog
#include <commdlg.h>
#include <math.h> // for pow
#include <stdio.h> // for scanf

// Disable type casting warning.
#pragma warning(disable : 4311)
// Disable security func warnings.
#pragma warning(disable : 4996)

#define myAssert(Expression) \
    if ((Expression) == NULL) \
    { MessageBox(NULL, L"发生了一个未知错误, 应用程序即将退出.", L"错误", MB_OK | MB_ICONERROR); exit(1); }

//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------

WCHAR gAppName[] = L"BitmapViewer";

// Window Class Identifier
WCHAR* gMainWndName = gAppName;
WCHAR gGrayTransWndName[] = L"GrayTrans";
// Gtw: Gray transform window
WCHAR gGtwHistDispWndName[] = L"GtwHistDisp";
WCHAR gSpatialFilterWndName[] = L"SpatialFilter";
// Spaf: Spatial filter (window)
WCHAR gSpafCustomCoreWndName[] = L"SpafCustomCore";
WCHAR gSpectralFilterWndName[] = L"SpectralFilter";

// Application Handle
HINSTANCE gInstance;

// Main window & Child windows
HWND gMainWnd,
     gGrayTransWnd,
     gGtwHistDispWnd,
     gSpatialFilterWnd,
     gSpafCustomCoreWnd,
     gSpectralFilterWnd;

// Image data of main window
MyBGRA* gImage = NULL;
MyBmpInfo gBmpInfo;

// System metrics info
UINT gCaptionHeight = 0;
UINT gCharWidth = 0, gCharHeight = 0;
UINT gCxVertScr = 0, gCyVertScr = 0;
UINT gCxHorzScr = 0, gCyHorzScr = 0;

// Main window menu
HMENU gMenu;

// Whether the image of main window is connected to an external file.
BOOL gHasExternalFile = FALSE;
// Used for common dialog. gFileName (full path), gTitleName(body name).
WCHAR gFileName[MAX_PATH], gTitleName[MAX_PATH];

// Child Window ID
#define GRAY_TRANS_WND 101
#define GTW_HIST_DISP_WND 102
#define SPATIAL_FILTER_WND 103
#define SPAF_CUST_CORE_WND 104
#define SPECTRAL_FILTER_WND 105

// Main Menu ID
//-------------------------------------
#define IDM_FILE 0
#define IDM_FILE_NEW 1001
#define IDM_FILE_OPEN 1002
#define IDM_FILE_SAVE 1003
#define IDM_FILE_SAVE_AS 1004
#define IDM_FILE_EXP_TXT 1005
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
// SPAF: SPAtial Filter
#define IDM_SPAF 2
#define IDM_SPAF_GENE 1021
#define IDM_SPAF_CUST 1022
//-------------------------------------
// SPEF: SPEctral Filter
#define IDM_SPEF 3
//-------------------------------------
#define IDM_HELP 4
#define IDM_APP_ABOUT 1041

//-------------------------------------------------------------------------------------------------
// Helper Function Declaration
//-------------------------------------------------------------------------------------------------

// Window Process Function
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GrayTransWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GtwHistDispWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SpatialFilterWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SpafCustomCoreWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SpectralFilterWndProc(HWND, UINT, WPARAM, LPARAM);

HMENU myLoadMainMenu();

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

// Process the image with a spatial box filter.
//
// @param ppDst: destination image buffer to write.
// @param pSrc: source image buffer to read & process.
// @param pInfo: points to image pixel info.
// @param iNormCoef_m: also known as box size (square core, m x m).
//
void mySpatialBoxFilter(MyBGRA** ppDst, MyBGRA* pSrc, MyBmpInfo* pInfo, int iNormCoef_m);

//-------------------------------------------------------------------------------------------------
// Main Function
//-------------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
{
    MSG msg;
    WNDCLASS wndclass;

    gInstance = hInstance;

    UINT originStyle;
    // Main Window
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = MainWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = gMainWndName;
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

    // Spatial Filter Window
    wndclass.lpfnWndProc = SpatialFilterWndProc;
    wndclass.lpszClassName = gSpatialFilterWndName;
    RegisterClass(&wndclass);

    // SPAF Custom Core Window
    wndclass.lpfnWndProc = SpafCustomCoreWndProc;
    wndclass.lpszClassName = gSpafCustomCoreWndName;
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
            // Display image in the center of main window.
            x = ((int)width - (int)gBmpInfo.nWidth) / 2;
            y = ((int)height - (int)gBmpInfo.nHeight) / 2;
            myDisplayImage(hdc, x, y, gImage, &gBmpInfo);
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
                RECT rc = { 0, 0, gBmpInfo.nWidth + 36, gBmpInfo.nHeight + 36 };
                AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
                MoveWindow(hwnd, left, top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

                // Have fun with operations.
                myEnableOperationMenus(myIsImageGrayScale(gImage, &gBmpInfo));
                DrawMenuBar(hwnd);
            }
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
        case IDM_SPAF_GENE:
        {
            RECT rc = myGetSpafGeneWndInitSize();
            // Create window with quried size.
            gSpatialFilterWnd = CreateWindow(
                gSpatialFilterWndName,
                NULL,
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL,
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
        case IDM_SPAF_CUST:
        {
            gSpafCustomCoreWnd = CreateWindow(
                gSpafCustomCoreWndName,
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
        // Make the menu selectable again since the window is closed.
        EnableMenuItem(gMenu, IDM_FILE, MF_ENABLED | MF_HILITE | MF_BYPOSITION);
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
|                                                                    Spatial Filter Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

// SPAF: Spatial Filter Window - 103
// Its child window should start from 103_01.
#define SPAF_GENE_APPLY_BTN 10301
// Label range: 10310 ~ 10319
#define SPAF_GENE_PARAM_LBL 10310
// Scrll bar range: 10320 ~ 10329
#define SPAF_GENE_PARAM_SCR 10320
// Edit line range: 10330 ~ 10339
#define SPAF_GENE_PARAM_EDT 10330

LRESULT CALLBACK SpatialFilterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static PAINTSTRUCT ps;

    static UINT left, top;
    static UINT width, height;

    static MyBGRA* image = NULL;

    // Left top position of filter panel.
    static int xCurrHorzPos;
    static int yCurrVertPos;
    
    static HWND btnApply;

    static HWND lblParam[10];
    static HWND scrParam[10];
    static HWND edtParam[10];
    static int iParamVal[10];
    static double dParamVal[10];

    switch (message)
    {
    case WM_CREATE:
        // Copy image data from main window.
        size_t nImageByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
        image = (MyBGRA*)malloc(nImageByteSize);
        myAssert(image);
        memcpy(image, gImage, nImageByteSize);

        // Create apply-all button.
        btnApply = CreateWindow(
            L"button",
            L"计算结果",
            WS_CHILD | WS_VISIBLE,
            12,
            gBmpInfo.nHeight + 6,
            16 * gCharWidth,
            2 * gCharHeight,
            hwnd,
            (HMENU)SPAF_GENE_APPLY_BTN,
            gInstance,
            NULL);

        RECT rc;
        int cxTmpWidth = 388 - gCxVertScr;
        xCurrHorzPos = gBmpInfo.nWidth + 24;
        yCurrVertPos = 12; // top padding

        int tmpHorz = xCurrHorzPos;
        int tmpVert = yCurrVertPos;
        // Create box-filter text label.
        //lblBoxFilter = CreateWindow(
        //    L"static",
        //    L"归一系数 m",
        //    WS_CHILD | WS_VISIBLE | SS_CENTER,
        //    tmpHorz + gCharWidth,
        //    tmpVert + 2 * gCharHeight,
        //    10 * gCharWidth,
        //    2 * gCharHeight,
        //    hwnd,
        //    (HMENU)SPAF_GENE_BOX_LBL,
        //    gInstance,
        //    NULL);
        //// Create box-filter scroll bar.
        //scrBoxFilter = CreateWindow(
        //    L"scrollbar",
        //    NULL,
        //    WS_CHILD | WS_VISIBLE | SBS_HORZ,
        //    tmpHorz + 12 * gCharWidth,
        //    tmpVert + 2 * gCharHeight,
        //    200,
        //    2 * gCharHeight,
        //    hwnd,
        //    (HMENU)SPAF_GENE_BOX_SCR,
        //    gInstance,
        //    NULL);
        //SetScrollRange(scrBoxFilter, SB_CTL, 1, 50, FALSE);
        //SetScrollPos(scrBoxFilter, SB_CTL, 1, TRUE);
        //// Create box-filter edit line.
        //edtBoxFilter = CreateWindow(
        //    L"edit",
        //    L"1",
        //    WS_CHILD | WS_VISIBLE,
        //    tmpHorz + 200 + 13 * gCharWidth,
        //    tmpVert + 2 * gCharHeight,
        //    rc.right - rc.left - (200 + 14 * gCharWidth),
        //    2 * gCharHeight,
        //    hwnd,
        //    (HMENU)SPAF_GENE_BOX_EDT,
        //    gInstance,
        //    NULL);
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

        GetWindowRect(btnApply, &rc);
        // Keep apply-all button stick to window bottom.
        MoveWindow(btnApply, 12, clntHeight - 6 - (rc.bottom - rc.top),
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);

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
        case SPAF_GENE_APPLY_BTN:
        {
            // Clone origin image firstly.
            size_t nImageByteSize = gBmpInfo.nWidth * gBmpInfo.nHeight * sizeof(MyBGRA);
            MyBGRA* clone = (MyBGRA*)malloc(nImageByteSize);
            myAssert(clone);
            memcpy(clone, gImage, nImageByteSize);

            // Note the actual coefficient is different from stored [m].
            //mySpatialBoxFilter(&image, clone, &gBmpInfo, 2 * iBoxFilterPos_m - 1);

            // Note we have allocated memory for clone data.
            free(clone);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        //case SPAF_GENE_BOX_EDT:
        //{
        //    WCHAR szCoef_m[10];
        //    GetWindowText(edtBoxFilter, szCoef_m, 10);
        //    int iUserInput = 1;
        //    if (swscanf(szCoef_m, L"%d", &iUserInput) == 1)
        //        iBoxFilterPos_m = (int)iUserInput;

        //    // Make sure in valid range and is an odd number.
        //    iBoxFilterPos_m = (iBoxFilterPos_m + 1) / 2;
        //    iBoxFilterPos_m = min(50, max(1, iBoxFilterPos_m));

        //    // Update related scroll bar.
        //    SetScrollPos(scrBoxFilter, SB_CTL, iBoxFilterPos_m, TRUE);
        //    return 0;
        //}
        default:
            return 0;
        }
    }
    case WM_MOUSEWHEEL:
    {
        return 0;
    }
    //case WM_HSCROLL:
    //{
    //    UINT id = GetWindowLong((HWND)lParam, GWL_ID);
    //    switch (id)
    //    {
    //    case SPAF_GENE_BOX_SCR: // Box Filter, Normalize Coefficient [m]
    //    {
    //        // Kill scroll bar focus.
    //        SetFocus(hwnd);
    //        switch (LOWORD(wParam))
    //        {
    //        case SB_PAGEUP:
    //            iBoxFilterPos_m -= 9;
    //            // fall through
    //        case SB_LINEUP:
    //            iBoxFilterPos_m = max(1, iBoxFilterPos_m - 1);
    //            break;
    //        case SB_PAGEDOWN:
    //            iBoxFilterPos_m += 9;
    //            // fall through
    //        case SB_LINEDOWN:
    //            iBoxFilterPos_m = min(50, iBoxFilterPos_m + 1);
    //            break;
    //        // no focus, so no top & bottom
    //        case SB_THUMBPOSITION:
    //        case SB_THUMBTRACK:
    //            iBoxFilterPos_m = HIWORD(wParam);
    //            break;
    //        default:
    //            return 0;
    //        }
    //        SetScrollPos(scrBoxFilter, SB_CTL, iBoxFilterPos_m, TRUE);

    //        // Update related edit line.
    //        WCHAR szBuffer[10];
    //        wsprintf(szBuffer, L"%d", 2 * iBoxFilterPos_m - 1);
    //        SetWindowText(edtBoxFilter, szBuffer);
    //        return 0;
    //    }
    //    default:
    //        return 0;
    //    }
    //}
    case WM_VSCROLL:
    {
        switch (LOWORD(wParam))
        {
        case SB_PAGEUP:
        case SB_LINEUP:
            break;
        case SB_PAGEDOWN:
        case SB_LINEDOWN:
            break;
        case SB_TOP:
            break;
        case SB_BOTTOM:
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            break;
        default:
            return 0;
        }
        return 0;
    }
    case WM_DESTROY:
        // Note we have allocated memory for image data.
        free(image);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//#################################################################################################
/**************************************************************************************************\
|                                                                                                  |
|                                                                  Spaf Custom Core Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

LRESULT CALLBACK SpafCustomCoreWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
|                                                                   Spectrel Filter Window Process |
|                                                                                                  |
\**************************************************************************************************/
//#################################################################################################

LRESULT CALLBACK SpectralFilterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

//-------------------------------------------------------------------------------------------------
// Digital Image Processing Subroutine & Miscellaneous Function Definition
//-------------------------------------------------------------------------------------------------

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

        OPT_TOPLEV,                     L'空', L'间', L'滤', L'波', 0,
        OPT_MIDDLE, IDM_SPAF_GENE,      L'常', L'用', L'模', L'型', 0,
        OPT_______,
        OPT_MIDEND, IDM_SPAF_CUST,      L'自', L'定', L'义', L'核', 0,

        OPT_TOPLEV,                     L'频', L'域', L'滤', L'波', 0,
        OPT____END,

        OPT_TOPEND,                     L'帮', L'助', 0,
        OPT_MIDEND, IDM_APP_ABOUT,      L'关', L'于', 0

#undef OPT_______
#undef OPT_MIDDLE
#undef OPT_TOPLEV
    };
    return LoadMenuIndirect(MENU_TEMPLATE);
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
    EnableMenuItem(gMenu, IDM_SPAF, nEnable | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_SPEF, nEnable | MF_BYPOSITION);
}

void myDisableOperationMenus()
{
    EnableMenuItem(gMenu, IDM_FILE_SAVE, MF_GRAYED | MF_BYCOMMAND);
    EnableMenuItem(gMenu, IDM_FILE_SAVE_AS, MF_GRAYED | MF_BYCOMMAND);

    EnableMenuItem(gMenu, IDM_GRAY, MF_GRAYED | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_SPAF, MF_GRAYED | MF_BYPOSITION);
    EnableMenuItem(gMenu, IDM_SPEF, MF_GRAYED | MF_BYPOSITION);
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
            if (converted[j].x <= (int)i && (int)i <= converted[j + 1].x)
            {
                // y = y0 + k * (x - x0), powered by regulation stretch
                pTargetHist[i] = converted[j].y + slopes[j] * (i - converted[j].x);
                pTargetHist[i] = pow(pTargetHist[i], rgltStretch);
                // We will use this value to normalize later.
                sum += pTargetHist[i];
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
    RECT rc = { 0, 0, gBmpInfo.nWidth + 24 + 400, gBmpInfo.nHeight + 2 * gCharHeight + 12 };
    // Note this func wouldn't take WS_VSCROLL into consideration.
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    return rc;
}

/*
* Spatial Box Filter:
* 
* Convolution Core (take 3 x 3 as example):
* [ 1/9 ][ 1/9 ][ 1/9 ]
* [ 1/9 ][ 1/9 ][ 1/9 ]
* [ 1/9 ][ 1/9 ][ 1/9 ]
* 
* In fact, the elements of box filter core could be different,
* but we just use the simplest unified format in this function.
*/
void mySpatialBoxFilter(MyBGRA** ppDst, MyBGRA* pSrc, MyBmpInfo* pInfo, int iNormCoef_m)
{
    if ((UINT)iNormCoef_m > min(pInfo->nWidth, pInfo->nHeight)) return;

    // Declare common index variables here.
    UINT i, j, currIdx;
    int k, s, sumColor;

    MyBGRA* pData = *ppDst;
    // Padding zero. We simply populate 0 for border pixels.
    ZeroMemory(pData, pInfo->nWidth * pInfo->nHeight * sizeof(MyBGRA));

    int halfLen = iNormCoef_m / 2;
    int coef = iNormCoef_m * iNormCoef_m;

    for (i = halfLen; i < pInfo->nWidth - halfLen; ++i)
        for (j = halfLen; j < pInfo->nHeight - halfLen; ++j)
        {
            // Horizontal & Vertical
            currIdx = i + j * pInfo->nWidth;

            sumColor = 0;
            // Sum pixels within box.
            for (k = -halfLen; k <= halfLen; ++k)
                for (s = -halfLen; s <= halfLen; ++s)
                {
                    // We only need to compute one channel for each pixel.
                    sumColor += pSrc[currIdx + k + s * pInfo->nWidth].R;
                }

            // Normalize && Copy Channel
            pData[currIdx].R = (UINT8)(sumColor / coef);
            pData[currIdx].G = pData[currIdx].B = pData[currIdx].R;
            pData[currIdx].A = 255; // Keep opaque by default.
        }
}
