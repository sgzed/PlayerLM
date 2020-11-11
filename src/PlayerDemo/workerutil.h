#ifndef WORKERWINMANGER_H
#define WORKERWINMANGER_H


#include <Windows.h>
#include <QMap>
#include<QRect>
#include <stdio.h>
#include"singleton.h"

#define WEEBAPI

#define WP_VERSION_MAJOR 1 /* non-backwards-compatible changes */
#define WP_VERSION_MINOR 3 /* backwards compatible api changes */
#define WP_VERSION_PATCH 2 /* backwards-compatible changes */

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define WP_VERSION_STR \
    STRINGIFY(WP_VERSION_MAJOR) "." \
    STRINGIFY(WP_VERSION_MINOR) "." \
    STRINGIFY(WP_VERSION_PATCH)

typedef HWND wnd_t;
typedef RECT rect_t;
typedef HANDLE handle_t;


enum WPEWallpaperStartRet
{
    WPE_STARTWP_SUCCESS = 0,
    WPE_STARTWP_CREATE_PROCESS_FAILED = -1,
    WPE_STARTWP_WAIT_PROCESS_FAILED = -2,
    WPE_STARTWP_GET_HANDLE_FAILED = -3
};

class WorkerWinManger
{
    DECLARE_SINGLETON(WorkerWinManger)
public:

    ~WorkerWinManger();
    int     AddToDesktop (wnd_t wnd);
    int     RemoveFormDesktop (wnd_t wnd);
    void    SetWindowVisiable(wnd_t wnd, bool bShow);
    int     MoveWpWindow(wnd_t wnd, long left, long top, long right, long bottom);
    void    Setfocus(wnd_t wnd, int ensure);
    bool    IsWpWindowVisible(wnd_t wnd);
    int     FullScreenWpWindow(wnd_t wnd);
    void    FlushDesktop ();
    bool    IsWindowCovered(wnd_t hwnd, QRect rect= QRect());
    bool    MoveWindowOnWorkW(wnd_t hwnd, QRect rect);
    HWND    GetDesktopParent();

private:
    WorkerWinManger();
    wnd_t   getWpId();
    bool    isWin8OrLater();
    int     getWpWindowRect(wnd_t wnd, rect_t* rect);
    wnd_t   pickWpwindow(int* keys, int* cancel_keys, int poll_ms);
    int     wpMapRect(wnd_t wnd, rect_t* mapped);
    int     updateWindowStyles(wnd_t wnd, long and1, long ex_and, long or1, long ex_or);

private:
    QMap<wnd_t, wnd_t> mHwnd2ParentList;
    wnd_t mWallpaper = nullptr;


};

#endif // WORKERWINMANGER_H
