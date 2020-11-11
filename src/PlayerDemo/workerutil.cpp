#include "workerutil.h"
#include"easylogging++.h"
#include <qt_windows.h>
#include <QPainter>
#include <QMutex>
#include <tchar.h>
#include <dwmapi.h>
#include <QDebug>
//#include "config.h"

INITIALIZE_EASYLOGGINGPP


BOOL CALLBACK find_worker(wnd_t wnd, LPARAM lp)
{
    wnd_t *pworker = (wnd_t*)lp;
    if (!FindWindowExA(wnd, nullptr, "SHELLDLL_DefView", nullptr)) {
        return TRUE;
    }
    *pworker = FindWindowExA(nullptr, wnd, "WorkerW", nullptr);
    if (*pworker) {
        return FALSE;
    }
    return TRUE;
}


WorkerWinManger::WorkerWinManger()
{
    mWallpaper = getWpId();
    mHwnd2ParentList.clear();
}



bool WorkerWinManger::isWin8OrLater()
{
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 2;

    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION,
                             dwlConditionMask);
}


int WorkerWinManger::updateWindowStyles(wnd_t wnd, long and1, long ex_and, long or1, long ex_or)
{
    unsigned gle;
    long style = 0, exstyle = 0;

    SetLastError(0);

    style = GetWindowLongA(wnd, GWL_STYLE);
    if (!style) goto errcheck;
    exstyle = GetWindowLongA(wnd, GWL_EXSTYLE);
errcheck:
    gle = GetLastError();
    if ((!style || !exstyle) && gle) {
        LOG(ERROR)<<"GetWindowLongA failed, GLE=" <<GetLastError() << endl;
        return 1;
    }

    style &= and1;
    exstyle &= ex_and;
    style |= or1;
    style |= ex_or;

    SetLastError(0);

    if (!SetWindowLongA(wnd, GWL_STYLE, style) ||
            !SetWindowLongA(wnd, GWL_EXSTYLE, exstyle)){
        gle = GetLastError();
        if (gle) {
            LOG(ERROR)<<"SetWindowLongA failed, GLE=" <<gle << endl;
            return 1;
        }
    }

    return 0;
}


wnd_t WorkerWinManger::getWpId()
{
    wnd_t progman;
    wnd_t worker;

    progman = FindWindowA("Progman", nullptr);

    if (!progman){
        LOG(ERROR)<<"failed to find Progman, GLE=" << GetLastError() << endl;
        return nullptr;
    }

    SendMessageA(progman, 0x052C, 0xD, 0);
    SendMessageA(progman, 0x052C, 0xD, 1);
    EnumWindows(find_worker, (LPARAM)&worker);

    if (!worker){
        LOG(INFO)<<"W: couldn't spawn WorkerW window, trying old method" << GetLastError() << endl;
        SendMessageA(progman, 0x052C, 0, 0);
        LOG(INFO)<<"checking for wallpaper" << GetLastError() << endl;
        EnumWindows(find_worker, (LPARAM)&worker);
    }

    /*
     * windows 7 with aero is almost the same as windows 10, except that we
     * have to hide the WorkerW window and render to Progman child windows
     * instead
     */
    if (worker && !isWin8OrLater()){
        LOG(INFO)<<"detected windows 7, hiding worker window" << endl;
        ShowWindow(worker, SW_HIDE);
        worker = progman;
    }

    if (!worker){
        LOG(ERROR)<<"W: couldnt spawn window behind icons, falling back to Progman" << endl;
        worker = progman;
    }

    return worker;
}

void WorkerWinManger::FlushDesktop()
{
    mWallpaper = getWpId();
    UpdateWindow(mWallpaper);
}

int WorkerWinManger::RemoveFormDesktop(wnd_t wnd)
{
    int nRet = 0;
    do{
        wnd_t wallpaper = mWallpaper;
        long or1;
        long ex_or;

        if (wallpaper == wnd) {
            LOG(INFO)<<"can't del this window" << endl;
            nRet = 1;
            break;
        }
        wnd_t hWndParent = GetDesktopWindow();
        if(mHwnd2ParentList.find(wnd) != mHwnd2ParentList.end()){
            hWndParent = mHwnd2ParentList.value(wnd);
        }
        else{
            LOG(INFO)<<"already removed" << endl;
            nRet = 1;
            break;
        }


        if (!SetParent(wnd, nullptr)) {
            LOG(INFO)<<"SetParent failed, GLE=" << GetLastError() << endl;
            nRet = 1;
            break;
        }
        mHwnd2ParentList.remove(wnd);
        or1 = WS_OVERLAPPEDWINDOW;
        ex_or = WS_EX_APPWINDOW;

        if (updateWindowStyles(wnd, -1, -1, or1, ex_or)) {
            nRet = 1;
            break;
        }

        SetWindowPos(wnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE |
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        UpdateWindow(wallpaper);
    }while(false);
    return 0;
}

WorkerWinManger::~WorkerWinManger()
{
    for(auto it = mHwnd2ParentList.begin();it != mHwnd2ParentList.end();++it){
        wnd_t hwnd = it.key();
        if(hwnd && IsWpWindowVisible(hwnd)){
            RemoveFormDesktop (hwnd);
        }
    }
}

int WorkerWinManger::AddToDesktop (wnd_t wnd)
{
    char wndclass[512];
     wnd_t wallpaper = mWallpaper;
    long and1, ex_and;
    rect_t r;
    if(wallpaper == nullptr){
        return 1;
    }
    *wndclass = 0;
    GetClassNameA(wnd, wndclass, sizeof(wndclass) - 1);

    if (wallpaper == wnd || !strcmp(wndclass, "Shell_TrayWnd")) {
        LOG(ERROR)<<"can't add this window\n";
        return 1;
    }

    if (IsChild(wallpaper, wnd)) {
        LOG(ERROR)<<"already added\n";
        return 0;
    }

    and1 = ~(
                WS_CAPTION |
                WS_THICKFRAME |
                WS_SYSMENU |
                WS_MAXIMIZEBOX |
                WS_MINIMIZEBOX
                );

    ex_and = ~(
                WS_EX_DLGMODALFRAME |
                WS_EX_COMPOSITED |
                WS_EX_WINDOWEDGE |
                WS_EX_CLIENTEDGE |
                WS_EX_LAYERED |
                WS_EX_STATICEDGE |
                WS_EX_TOOLWINDOW |
                WS_EX_APPWINDOW
                );

    //if (updateWindowStyles(wnd, and1, ex_and, WS_CHILD, 0)) {
    //    return 1;
    //}

    wpMapRect(wnd, &r);

    if (!SetParent(wnd, wallpaper)) {
        LOG(ERROR)<<"SetParent failed, GLE=" << GetLastError() <<endl;
        return 1;
    }
    mHwnd2ParentList.insert(wnd, GetParent(wnd));
    MoveWpWindow(wnd, r.left, r.top, r.right, r.bottom);
    FlushDesktop();
    return 0;
}

int WorkerWinManger::wpMapRect(wnd_t wnd, rect_t *mapped)
{
    if (getWpWindowRect(wnd, mapped)) {
        return 1;
    }
    MapWindowPoints(nullptr, mWallpaper, (LPPOINT)mapped, 2);
    return 0;
}


void WorkerWinManger::Setfocus(wnd_t wnd, int ensure)
{
    if (ensure)
    {
        wnd_t progman;
        progman = FindWindowA("Progman", nullptr);
        SendMessageA(progman, WM_ACTIVATE, WA_CLICKACTIVE, (LPARAM)progman);
        SendMessageA(progman, WM_SETFOCUS, (WPARAM)progman, 0);
    }

    SendMessageA(wnd, WM_ACTIVATE, WA_CLICKACTIVE, (LPARAM)wnd);
    SendMessageA(wnd, WM_SETFOCUS, (WPARAM)wnd, 0);
}

int WorkerWinManger::getWpWindowRect(wnd_t wnd, rect_t *rect)
{
    if (!GetWindowRect(wnd, (RECT*)rect)) {
        LOG(ERROR)<<"GetWindowRect failed, GLE=" << GetLastError();
        return 1;
    }
    return 0;
}

int WorkerWinManger::MoveWpWindow(wnd_t wnd, long left, long top, long right, long bottom)
{
    BOOL succ = SetWindowPos(wnd, nullptr, left, top, right - left, bottom - top, 0);
    if (!succ) {
        LOG(ERROR)<<"SetWindowPos failed, GLE=" << GetLastError();
        return 1;
    }
    else{
        UpdateWindow(WorkerWinManger::Instance()->getWpId());
    }
    return 0;
}

bool WorkerWinManger::IsWpWindowVisible(wnd_t wnd)
{
    bool bRet = false;
    if(wnd != nullptr){
        bRet = IsWindowVisible(wnd);
    }
    return  bRet;
}

void WorkerWinManger::SetWindowVisiable(wnd_t wnd, bool bShow)
{
    ShowWindow(wnd, bShow);
    UpdateWindow(WorkerWinManger::Instance()->getWpId());
}

int WorkerWinManger::FullScreenWpWindow(wnd_t wnd)
{
    HMONITOR mon;
    MONITORINFO mi;
    rect_t current_rect;

    if (getWpWindowRect(wnd, &current_rect)) {
        return 1;
    }

    mon = MonitorFromPoint(*(POINT*)&current_rect, MONITOR_DEFAULTTONEAREST);
    if (!mon) {
        LOG(ERROR)<<"MonitorFromWindow failed, GLE=" << GetLastError();
        return 1;
    }

    mi.cbSize = sizeof(mi);

    if (!GetMonitorInfoA(mon, &mi)) {
        LOG(ERROR)<<"GetMonitorInfoA failed, GLE=" << GetLastError();
        return 1;
    }

    MapWindowPoints(nullptr,mWallpaper, (LPPOINT)&mi.rcMonitor, 2);
    return MoveWpWindow(wnd, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right,
                        mi.rcMonitor.bottom);
}


bool WorkerWinManger::IsWindowCovered(wnd_t hwnd, QRect rect)
{
    if(!hwnd){
        return false;
    }
    bool bIsCovered = false;
    RECT hwndRect = {0,0,0,0};
    if(rect.isEmpty()){
        ::GetWindowRect(hwnd, &hwndRect);
    }
    else{
        hwndRect.left = rect.left();
        hwndRect.right = rect.right();
        hwndRect.top = rect.top();
        hwndRect.bottom = rect.bottom();
    }

    HRGN rgn = ::CreateRectRgn(hwndRect.left,
                               hwndRect.top,
                               hwndRect.right,
                               hwndRect.bottom);


    HWND desktopWin = GetDesktopWindow();
    HWND hParentWnd = ::GetAncestor(hwnd, GA_PARENT);
    HWND hChildWnd = hwnd;
    while(hChildWnd != nullptr){
        HWND topWnd = ::GetTopWindow(hParentWnd);
        do {
            if(topWnd == hChildWnd){ //如果hChildWnd已经在当前z序顶部后面就不用再遍历了
                break;
            }
            RECT topWndRect = {0,0,0,0};
            ::GetWindowRect(topWnd, &topWndRect);
            RECT tempRect = {0,0,0,0};
            //可见，不是最小化，不是桌面窗口，和要判断是否被遮挡的窗口有相交区域 才有可能遮住目标窗口
            if(::IsWindowVisible(topWnd)
                    && topWnd != desktopWin
                    && !::IsIconic(topWnd)
                    && IntersectRect(&tempRect, &topWndRect, &hwndRect) != 0)
            {
                BYTE alpha;
                BOOL success = GetLayeredWindowAttributes(topWnd, nullptr, &alpha, nullptr);
                if(success && alpha == 0){
                    topWnd = GetNextWindow(topWnd, GW_HWNDNEXT);
                    continue;
                }
                HWND ownedWin = GetWindow(topWnd, GW_OWNER);

                if ((ownedWin != nullptr) || (ownedWin == nullptr && hParentWnd == desktopWin)) {
                    char className[1024] = {0};
                    char winTitle[1024] = {0};
                    GetClassNameA(topWnd, className, 1024);
                    GetWindowTextA(topWnd, winTitle, 1024);

                    // work with desktop window
                    if (strcmp(className, "WorkerW")
                            && strcmp(className, "Progman")) {
                        if (strcmp(className, "Windows.UI.Core.CoreWindow")
                                || strcmp(className, "ApplicationFrameWindow")) {
                            int pvAttr = 0;
                            DwmGetWindowAttribute(topWnd, DWMWA_CLOAKED, &pvAttr, 4);
                            // equals 0 means the window is visible
                            if (pvAttr != 0) {
                                topWnd = GetNextWindow(topWnd, GW_HWNDNEXT);
                                continue;
                            }
                        }
                        if(!QString("2333").isEmpty()){
                            LOG(INFO) << " window title: " << winTitle
                                      << " window class: " << className
                                      << " HWND: "<< topWnd
                                      << "rect: ("<<topWndRect.left
                                      << ","<< topWndRect.top
                                      << ","<< topWndRect.right
                                      << "," << topWndRect.bottom
                                      <<")";
                        }
                        HRGN topWndRgn = ::CreateRectRgn(topWndRect.left, topWndRect.top, topWndRect.right, topWndRect.bottom);
                        int cbRet = ::CombineRgn(rgn, rgn, topWndRgn, RGN_DIFF);
                        DeleteObject(topWndRgn);
                    }
                }
            }
            topWnd = GetNextWindow(topWnd, GW_HWNDNEXT);
        } while(topWnd != nullptr);

        hChildWnd = hParentWnd;
        hParentWnd = ::GetAncestor(hParentWnd, GA_PARENT);
        if (hChildWnd == GetDesktopWindow()) {
            break;
        }
    }
    DWORD uRegionSize = GetRegionData(rgn, sizeof(RGNDATA), NULL);  // Send NULL request to get the storage size
    char *pRawRgnData = new char[uRegionSize];
    RGNDATA* pRgnData = (RGNDATA*)pRawRgnData;   // Allocate space for the region data
    DWORD uSizeCheck = GetRegionData(rgn, uRegionSize, pRgnData);

    QPainter painter;
    if (uSizeCheck == uRegionSize) {
        DWORD nCnt = pRgnData->rdh.nCount;
        if(nCnt == 0) {
            bIsCovered = true;
        }
    }
    DeleteObject(rgn);
    return bIsCovered;
}

bool WorkerWinManger::MoveWindowOnWorkW(wnd_t hwnd, QRect rect)
{
    RECT tmpRect;
    tmpRect.left = rect.left();
    tmpRect.top = rect.top();
    //这里多加一个像素不然会有边
    tmpRect.right = rect.right() + 1;
    tmpRect.bottom = rect.bottom() + 1;
    MapWindowPoints(HWND_DESKTOP, mWallpaper, (LPPOINT)&tmpRect, 2);
    LOG(INFO) << "SetWindowPos=" << tmpRect.left<<","<<tmpRect.top<<","<<tmpRect.right - tmpRect.left<<","<< tmpRect.bottom -tmpRect.top;
    if(SetWindowPos(hwnd,nullptr, tmpRect.left, tmpRect.top, tmpRect.right - tmpRect.left, tmpRect.bottom -tmpRect.top, SWP_NOZORDER ) == 0){
        LOG(INFO) << "SetWindowPos failed, GLE=" << GetLastError();
        return false;
    }
    return true;
}

HWND WorkerWinManger::GetDesktopParent()
{
    wnd_t wallpaper = mWallpaper;
    HWND hwnd = GetParent(wallpaper);
    return hwnd;
}


