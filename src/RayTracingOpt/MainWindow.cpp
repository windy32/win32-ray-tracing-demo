#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <omp.h> // OpenMP

#include "GeometrySet.h"
#include "RenderSetting.h"
#include "Scripts.h"
#include "Utils.h"
#include "erand48.h"
#include "resource.h"

// min & max settings
static const int MIN_TUNNEL_TESSELLATION = 6;
static const int MAX_TUNNEL_TESSELLATION = 1200;
static const int MIN_WIDTH = 160;
static const int MAX_WIDTH = 1280;
static const int MIN_REPEAT = 1;
static const int MAX_REPEAT = 100;

// initial settings
static const int INIT_TUNNEL_TESSELLATION = 150;
static const int INIT_WIDTH = 400;
static const int INIT_REPEAT = 75;

// settings (updated when the "Render" button is pressed)
static int width;
static int height;
static int segments;
static int samples;
static int repeat;
static int algorithm;

// GDI objects
static HDC hdcBuffer = 0;
static HBITMAP hBitmap = 0;

// handle to the dialog
static HWND hDialog = 0;

// Critical sections
static CRITICAL_SECTION csLog;
static CRITICAL_SECTION csProgress;

// Font
static HFONT hLogFont;

// The algorithm list
const char *algorithms[] = 
{
    "Linear",
    "Regular Grid",
    "Flat Grid",
    "k-d Tree",
    "k-d Tree (SAH)",
    "Convex",
    "Convex Simple"
};

// user defined messages
#define WM_RENDER_FINISH    (WM_USER + 1)

Color trace(GeometrySet &scene, Ray &r, int depth, unsigned short *Xi, RenderSetting &setting)
{
    IntersectResult result = scene.intersect(r);
    if (!result.hit)
    {
        return Color::Black();
    }

    Geometry *obj = result.geometry;
    Point &p = result.position;
    Vector &n = result.normal; // points to the outside
    Vector nl = (n.dot(r.direction) < 0) ? n : n * -1; // points to the ray
    Color local = obj->material->local(r, p, result.normal);

    if (++depth > setting.maxDepth)
        return Color::Black();

    if (depth > 100) // add a hard limit and avoid stack overflow
        return Color::Black();

    float diffusiveness = obj->material->diffusiveness;
    float reflectiveness = obj->material->reflectiveness;
    float refractiveness = obj->material->refractiveness;
    Color diffusive;
    Color reflective;
    Color refractive;

    if (diffusiveness > 0)
    {
        diffusive = local;
    }
    
    if (reflectiveness > 0)
    {
        Vector v = r.direction - nl * 2 * nl.dot(r.direction);
        Ray newRay(p, v);
        newRay.context = r.context;
        reflective = trace(scene, newRay, depth, Xi, setting);
    }

    if (refractiveness > 0)
    {
        Ray reflRay(p, r.direction - n * 2 * n.dot(r.direction));
        bool into = n.dot(nl) > 0;
        float nc = 1;
        float nt = obj->material->refractive_index;
        float nnt = into ? nc / nt : nt / nc;
        float ddn = r.direction.dot(nl);
        float cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
        if (cos2t < 0) // total internal reflection
        {
            refractive = trace(scene, reflRay, depth, Xi, setting);
        }
        else
        {
            Vector tdir = (r.direction * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
            float a = nt - nc;
            float b = nt + nc;
            float R0 = a * a / (b * b);
            float c = 1 - (into ? -ddn : tdir.dot(n));
            float Re = R0 + (1 - R0) * c * c * c * c * c;
            float Tr = 1 - Re;
            float P = 0.25f + 0.5f * Re;
            float RP = Re / P;
            float TP = Tr / (1 - P);

            refractive = trace(scene, reflRay, depth, Xi, setting) * Re + 
                trace(scene, Ray(p, tdir), depth, Xi, setting)*Tr;
        }
    }

    return diffusive * diffusiveness + 
        reflective * reflectiveness +
        refractive * refractiveness;
}

Color radiance(GeometrySet &scene, Ray &r, int depth, unsigned short *Xi, RenderSetting &setting)
{
    IntersectResult result = scene.intersect(r);
    if (!result.hit)
    {
        return Color(0, 0, 0);
    }

    Geometry *obj = result.geometry;
    Point &p = result.position;
    Vector &n = result.normal; // points to the outside
    Vector nl = (n.dot(r.direction) < 0) ? n : n * -1; // points to the ray
    Color local = obj->material->local(r, p, result.normal);
    Color emission = obj->material->emission(p);

    float maxColor = (local.r + local.g + local.b) * 0.333333f;

    if (++depth > setting.maxDepth)
        return emission;

    if (depth > setting.terminationDepth)
    { 
        if (erand48(Xi) < maxColor) 
            local = local * (1 / maxColor); 
        else
            return emission;
    }

    if (depth > 100) // avoid stack overflow
        return emission;

    float diffusiveness = obj->material->diffusiveness;
    float reflectiveness = obj->material->reflectiveness;
    float refractiveness = obj->material->refractiveness;

    // A material may have multiple reflection types, 
    // e.g., 10% diffuse, 5% specular and 85% refractive.
    // Only one type is used for each sample.
    float p_type = (float)erand48(Xi);

    if (diffusiveness > 0 && p_type < diffusiveness)
    {
        // Pick a random point on the surface of a unit sphere
        // http://mathworld.wolfram.com/SpherePointPicking.html
        float r1 = (float)erand48(Xi);
        float r2 = (float)erand48(Xi);
        float theta = 2 * PI * r1;
        float phi = acos(r2);

        Vector w = nl;
        Vector u = (fabs(w.x) > 0.1) ? 
            Vector(0, 1, 0).cross(w).norm() : Vector(1, 0, 0).cross(w).norm();
        Vector v = w.cross(u);
        Vector dir = u * (cos(theta) * sin(phi)) + v * (sin(theta) * sin(phi)) + w * cos(phi);
        return emission + local.mult(radiance(scene, Ray(p, dir), depth, Xi, setting));
    }
    
    if (reflectiveness > 0 && 
        p_type >= diffusiveness && 
        p_type <= diffusiveness + reflectiveness)
    {
        Vector v = r.direction - nl * 2 * nl.dot(r.direction);
        return emission + local.mult(radiance(scene, Ray(p, v), depth, Xi, setting));
    }

    if (refractiveness > 0 && p_type > diffusiveness + reflectiveness)
    {
        Ray reflRay(p, r.direction - n * 2 * n.dot(r.direction));
        bool into = n.dot(nl) > 0;
        float nc = 1;
        float nt = obj->material->refractive_index;
        float nnt = into ? nc / nt : nt / nc;
        float ddn = r.direction.dot(nl);
        float cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
        if (cos2t < 0) // total internal reflection
            return emission + local.mult(radiance(scene, reflRay, depth, Xi, setting));

        Vector tdir = (r.direction * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
        float a = nt - nc;
        float b = nt + nc;
        float R0 = a * a / (b * b);
        float c = 1 - (into ? -ddn : tdir.dot(n));
        float Re = R0 + (1 - R0) * c * c * c * c * c;
        float Tr = 1 - Re;
        float P = 0.25f + 0.5f * Re;
        float RP = Re / P;
        float TP = Tr / (1 - P);

        if (depth > setting.singleTracingDepth)
        {
            if ((float)erand48(Xi) < P)
                return radiance(scene, reflRay, depth, Xi, setting) * RP;
            else
                return radiance(scene, Ray(p,tdir), depth, Xi, setting) * TP;
        }
        else
        {
            return radiance(scene, reflRay, depth, Xi, setting) * Re + 
                radiance(scene, Ray(p, tdir), depth, Xi, setting)*Tr;
        }
    }

    // Impossible to reach here
    return emission; // simply to avoid compiling warnings
}

int Render(GeometrySet &scene, PerspectiveCamera &camera, RenderSetting &setting,
           ProgressCallback progress)
{
    const float dx = 1.0f / height;
    const float dy = 1.0f / height;

    Color *colors = new Color[width * height];
    for (int i = 0; i < width * height; i++)
    {
        colors[i].r = 0;
        colors[i].g = 0;
        colors[i].b = 0;
    }

    int t1 = Utils::GetTickCount();

    #pragma omp parallel for schedule(dynamic, 1) // OpenMP

    for (int y = 0; y < height; y++)
    {
        progress(y + 1, height);

        unsigned short Xi[3] = { 0, 0, y * y * y };
        for (int x = 0; x < width; x++)
        {
            int index = x * height + y;
            if (setting.enableMonteCarlo)
            {
                Color r = Color::Black();
                for (int i = 0; i < samples; i++)
                {
                    float r1 = (float)erand48(Xi);
                    float r2 = (float)erand48(Xi);
                    float sx = (x + r1) * dx;
                    float sy = 1 - (y + r2) * dy;

                    Ray ray(camera.generateRay(sx, sy));
                    r = r + radiance(scene, ray, 0, Xi, setting) * (1.0f / samples);
                }
                colors[index] = r;
            }
            else
            {
                float sx = (x + 0.5f) * dx;
                float sy = 1 - (y + 0.5f) * dy;

                Ray ray(camera.generateRay(sx, sy));
                colors[index] = trace(scene, ray, 0, Xi, setting);
            }
        }
    }

    int t2 = Utils::GetTickCount();

    for (int i = 0; i < width * height; i++)
    {
        colors[i].saturate();
        int r = (int)(colors[i].r * 255);
        int g = (int)(colors[i].g * 255);
        int b = (int)(colors[i].b * 255);
        SetPixel(hdcBuffer, i / height, i % height, RGB(r, g, b));
    }
    delete []colors;

    return t2 - t1;
}

void EnableControls(BOOL enable)
{
    EnableWindow(GetDlgItem(hDialog, IDC_LIST), enable);
    EnableWindow(GetDlgItem(hDialog, IDC_TUNNEL_ALGORITHM), enable);
    EnableWindow(GetDlgItem(hDialog, IDC_TUNNEL_TESSELLATION), enable);
    EnableWindow(GetDlgItem(hDialog, IDC_IMAGE_WIDTH), enable);
    EnableWindow(GetDlgItem(hDialog, IDC_REPEAT), enable);

    EnableWindow(GetDlgItem(hDialog, IDC_RENDER), enable);
    EnableWindow(GetDlgItem(hDialog, IDC_SAVE_AS), enable);
}

void AddLog(const char *str)
{
    // Append
    HWND hEdit = GetDlgItem(hDialog, IDC_LOG);
    EnterCriticalSection(&csLog);
    SendMessage(hEdit, EM_SETSEL, LOWORD(-1), HIWORD(-1));
    SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)str);
    LeaveCriticalSection(&csLog);
}

void UpdateProgress(int cur, int total)
{
    HWND hProgress = GetDlgItem(hDialog, IDC_PROGRESS);
    EnterCriticalSection(&csProgress);
    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, total));
    SendMessage(hProgress, PBM_SETPOS, cur, 0);
    LeaveCriticalSection(&csProgress);
}

DWORD WINAPI RenderThread(LPVOID lpParam)
{
    Script *s = (Script *)lpParam;

    std::vector<int> prepareTimes;
    std::vector<int> executeTimes;
    int avgPrepareTime = 0;
    int avgExecuteTime = 0;

    HWND hProgress = GetDlgItem(hDialog, IDC_PROGRESS);
    HWND hOverallProgress = GetDlgItem(hDialog, IDC_OVERALL_PROGRESS);
    SendMessage(hOverallProgress, PBM_SETRANGE, 0, MAKELPARAM(0, repeat));
    SendMessage(hOverallProgress, PBM_SETPOS, 0, 0);

    for (int i = 0; i < repeat; i++)
    {
        Utils::DbgPrint("Render test %d / %d\r\n\r\n", i + 1, repeat);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);

        int prepareTime, execTime;
        s->Run(Render, algorithm, AddLog, UpdateProgress, prepareTime, execTime);
        prepareTimes.push_back(prepareTime);
        executeTimes.push_back(execTime);
        avgPrepareTime += prepareTime;
        avgExecuteTime += execTime;

        SendMessage(hOverallProgress, PBM_SETPOS, i + 1, 0);
    }

    // Print summary
    Utils::DbgPrint("==================================\r\n");
    Utils::DbgPrint("Pass | Prepare Time | Execute Time\r\n");
    Utils::DbgPrint("-----+--------------+-------------\r\n");
    for (int i = 0; i < repeat; i++)
    {
        Utils::DbgPrint("%4d | %9d ms | %9d ms\r\n", i + 1, prepareTimes[i], executeTimes[i]);
    }
    Utils::DbgPrint("==================================\r\n");
    Utils::DbgPrint(" avg | %9d ms | %9d ms\r\n", avgPrepareTime / repeat, avgExecuteTime / repeat);

    // Send a finish message to the main dialog
    SendMessage(hDialog, WM_RENDER_FINISH, 0, 0);
    return 0;
}

LRESULT CALLBACK ProcImage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg == WM_PAINT)
    {
        PAINTSTRUCT stPS;
        BeginPaint(hWnd, &stPS);

        SelectObject(stPS.hdc, GetStockObject(NULL_BRUSH));
        SelectObject(stPS.hdc, GetStockObject(BLACK_PEN));
        Rectangle(stPS.hdc, 0, 0, width + 4, height + 4);

        SelectObject(stPS.hdc, GetStockObject(WHITE_PEN));
        Rectangle(stPS.hdc, 1, 1, width + 3, height + 3);

        BitBlt(stPS.hdc, 2, 2, width, height, hdcBuffer, 0, 0, SRCCOPY);
        EndPaint(hWnd, &stPS);
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK ProcDlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        // Initialize window size
        MoveWindow(hWnd, 100, 100, 740, 400, FALSE);

        // Initialize the up-down controls
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_TUNNEL_TESSELLATION), UDM_SETRANGE, 0, 
            MAKELPARAM(MAX_TUNNEL_TESSELLATION, MIN_TUNNEL_TESSELLATION));
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_IMAGE_WIDTH), UDM_SETRANGE, 0, 
            MAKELPARAM(MAX_WIDTH, MIN_WIDTH));
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_REPEAT), UDM_SETRANGE, 0, 
            MAKELPARAM(MAX_REPEAT, MIN_REPEAT));

        SendMessage(GetDlgItem(hWnd, IDC_SPIN_TUNNEL_TESSELLATION), UDM_SETPOS, 0,
            INIT_TUNNEL_TESSELLATION);
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_IMAGE_WIDTH), UDM_SETPOS, 0,
            INIT_WIDTH);
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_REPEAT), UDM_SETPOS, 0,
            INIT_REPEAT);

        // Initialize the font of the logs
        LOGFONT stLogFont;
        RtlZeroMemory(&stLogFont, sizeof(stLogFont));

        stLogFont.lfHeight = 13;
        stLogFont.lfWidth = 0;
        stLogFont.lfEscapement = 0;
        stLogFont.lfOrientation = 0;
        stLogFont.lfWeight = FW_REGULAR;
        stLogFont.lfItalic = FALSE;
        stLogFont.lfUnderline = FALSE;
        stLogFont.lfStrikeOut = FALSE;
        strcpy_s(stLogFont.lfFaceName, sizeof(stLogFont.lfFaceName), "Consolas");

        hLogFont = CreateFontIndirect(&stLogFont); 
        SetWindowFont(GetDlgItem(hWnd, IDC_LOG), hLogFont, FALSE);

        // Initialize the scenario list
        for (int i = 0; i < _countof(scripts); i++)
        {
            ComboBox_AddString(GetDlgItem(hWnd, IDC_LIST), (LPARAM)scripts[i]->name);
        }
        ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_LIST), 4);

        // Initialize the algorithm list
        for (int i = 0; i < _countof(algorithms); i++)
        {
            ComboBox_AddString(GetDlgItem(hWnd, IDC_TUNNEL_ALGORITHM), (LPARAM)algorithms[i]);
        }
        ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_TUNNEL_ALGORITHM), 0);

        // Initialize critical sections
        InitializeCriticalSection(&csLog);
        InitializeCriticalSection(&csProgress);

        // Initialize log output target
        Utils::RegisterOutputTarget(AddLog);

        // Initialize GDI objects
        HDC hdc = GetDC(hWnd);
        hdcBuffer = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, MAX_WIDTH, MAX_WIDTH * 3 / 4);
        SelectObject(hdcBuffer, hBitmap);

        // Fill white
        SelectObject(hdcBuffer, GetStockObject(NULL_PEN));
        SelectObject(hdcBuffer, GetStockObject(WHITE_BRUSH));
        Rectangle(hdcBuffer, 0, 0, MAX_WIDTH, MAX_WIDTH * 3 / 4);

        // Initialize width & height of the image
        width = INIT_WIDTH;
        height = INIT_WIDTH * 3 / 4;

        // Initialize the static control (image)
        SetWindowLongPtr(GetDlgItem(hWnd, IDC_IMAGE), GWLP_WNDPROC, (LONG_PTR)ProcImage);

        // Display window
        ShowWindow(hWnd, SW_SHOW);
        return TRUE;
    }
    else if (uMsg == WM_CTLCOLORSTATIC) // use white background for read-only text edit controls
    {
        if ((HWND)lParam == GetDlgItem(hWnd, IDC_LOG))
        {
            return (INT_PTR)GetStockObject(WHITE_BRUSH);
        }
        return FALSE;
    }
    else if (uMsg == WM_SIZE)
    {
        // auto layout under win32 is a bit inconvenient :)
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        const int TOP_MARGIN = 12;
        const int LEFT_MARGIN = 12;
        const int RIGHT_MARGIN = 12;
        const int BOTTOM_MARGIN = 12;

        const int LABEL_WIDTH = 110;
        const int LINE_HEIGHT = 23;
        const int SETTING_WIDTH = 160;
        const int ROW_SPACING = 5;
        const int COL_SPACING = 10;

        const int SPIN_WIDTH = 18;
        const int BUTTON_WIDTH = 100;

        const int SETTING_X = LEFT_MARGIN + LABEL_WIDTH + COL_SPACING;
        const int SPIN_X = LEFT_MARGIN + LABEL_WIDTH + COL_SPACING + SETTING_WIDTH - SPIN_WIDTH;
        const int LOG_WIDTH = LABEL_WIDTH + COL_SPACING + SETTING_WIDTH;
        const int LOG_HEIGHT = height - TOP_MARGIN - BOTTOM_MARGIN - (LINE_HEIGHT + ROW_SPACING) * 8;
        const int BUTTON_Y = height - BOTTOM_MARGIN - LINE_HEIGHT;
        const int IMAGE_X = LEFT_MARGIN + LABEL_WIDTH + COL_SPACING + SETTING_WIDTH + COL_SPACING;
        const int IMAGE_WIDTH = width - IMAGE_X - RIGHT_MARGIN;
        const int IMAGE_HEIGHT = height - TOP_MARGIN - BOTTOM_MARGIN;

        // Labels on the left
        MoveWindow(GetDlgItem(hWnd, IDL_SCENARIO), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 0 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDL_TUNNEL_ALGORITHM), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 1 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDL_TUNNEL_TESSELLATION), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 2 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDL_IMAGE_WIDTH), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 3 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDL_REPEAT), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 4 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDL_PROGRESS), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 5 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDL_OVERALL_PROGRESS), 
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 6 + 3, LABEL_WIDTH, LINE_HEIGHT, FALSE);

        // Settings on the right
        MoveWindow(GetDlgItem(hWnd, IDC_LIST),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 0, SETTING_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_TUNNEL_ALGORITHM),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 1, SETTING_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_TUNNEL_TESSELLATION),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 2, SETTING_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_IMAGE_WIDTH),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 3, SETTING_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_REPEAT),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 4, SETTING_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_PROGRESS),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 5 + 2, SETTING_WIDTH, LINE_HEIGHT - 4, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_OVERALL_PROGRESS),
            SETTING_X, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 6 + 2, SETTING_WIDTH, LINE_HEIGHT - 4, FALSE);

        // Up-down controls
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_TUNNEL_TESSELLATION), UDM_SETBUDDY, 
            (WPARAM)GetDlgItem(hWnd, IDC_TUNNEL_TESSELLATION), 0);
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_IMAGE_WIDTH), UDM_SETBUDDY,
            (WPARAM)GetDlgItem(hWnd, IDC_IMAGE_WIDTH), 0);
        SendMessage(GetDlgItem(hWnd, IDC_SPIN_REPEAT), UDM_SETBUDDY,
            (WPARAM)GetDlgItem(hWnd, IDC_REPEAT), 0);

        // Log
        MoveWindow(GetDlgItem(hWnd, IDC_LOG),
            LEFT_MARGIN, TOP_MARGIN + (LINE_HEIGHT + ROW_SPACING) * 7, LOG_WIDTH, LOG_HEIGHT, FALSE);

        // Bottons
        MoveWindow(GetDlgItem(hWnd, IDC_RENDER),
            LEFT_MARGIN, BUTTON_Y, BUTTON_WIDTH, LINE_HEIGHT, FALSE);
        MoveWindow(GetDlgItem(hWnd, IDC_SAVE_AS),
            LEFT_MARGIN + BUTTON_WIDTH + COL_SPACING, BUTTON_Y, BUTTON_WIDTH, LINE_HEIGHT, FALSE);

        // Image
        MoveWindow(GetDlgItem(hWnd, IDC_IMAGE), IMAGE_X, TOP_MARGIN, IMAGE_WIDTH, IMAGE_HEIGHT, FALSE);

        // Refresh window
        InvalidateRect(hWnd, NULL, FALSE);
        return TRUE;
    }
    else if (uMsg == WM_GETMINMAXINFO) // specify min window size
    {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;

        mmi->ptMaxSize.x = GetSystemMetrics(SM_CXFULLSCREEN);
        mmi->ptMaxSize.y = GetSystemMetrics(SM_CYFULLSCREEN);

        mmi->ptMaxPosition.x = 0;
        mmi->ptMaxPosition.y = 0;

        mmi->ptMinTrackSize.x = 740;
        mmi->ptMinTrackSize.y = 400;

        mmi->ptMaxTrackSize.x = GetSystemMetrics(SM_CXFULLSCREEN);
        mmi->ptMaxTrackSize.y = GetSystemMetrics(SM_CXFULLSCREEN);    
        return TRUE;
    }
    else if (uMsg == WM_COMMAND)
    {
        if (wParam == IDC_RENDER)
        {
            width = GetDlgItemInt(hWnd, IDC_IMAGE_WIDTH, NULL, FALSE);
            height = width * 3 / 4;
            samples = 2000;
            segments = GetDlgItemInt(hWnd, IDC_TUNNEL_TESSELLATION, NULL, FALSE);
            repeat = GetDlgItemInt(hWnd, IDC_REPEAT, NULL, FALSE);
            algorithm = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_TUNNEL_ALGORITHM));

            if (width < MIN_WIDTH || width > MAX_WIDTH)
            {
                MessageBox(hWnd, "Illegal image width!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (segments < MIN_TUNNEL_TESSELLATION || segments > MAX_TUNNEL_TESSELLATION)
            {
                MessageBox(hWnd, "Illegal tunnel tessellation value!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (repeat < MIN_REPEAT || repeat > MAX_REPEAT)
            {
                MessageBox(hWnd, "Illegal repeat value!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Get selected script
            int index = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_LIST));
            Script *s = scripts[index];

            // If the script includes tunnel, update parameter segment first
            if (s->flags & Script::FLAG_TUNNEL)
            {
                s->tunnelSegments = segments;
            }

            // If monte carlo is enabled, update parameter samples first
            if (s->flags & Script::FLAG_MONTE_CARLO)
            {
                s->samples = samples;
            }

            // Disable controls
            EnableControls(FALSE);

            // Clear logs
            Edit_SetText(GetDlgItem(hWnd, IDC_LOG), "");

            // Update cursor
            SetCursor(LoadCursor(NULL, IDC_WAIT));

            // Start
            CreateThread(0, 0, RenderThread, s, 0, 0);
        }
        else if (wParam == IDC_SAVE_AS)
        {
            unsigned char *bits = new unsigned char[height * width * 3];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    COLORREF color = GetPixel(hdcBuffer, x, y);
                    unsigned char r = (unsigned char)(int)(color & 0xFF);
                    unsigned char g = (unsigned char)(int)((color & 0xFF00) >> 8);
                    unsigned char b = (unsigned char)(int)((color & 0xFF0000) >> 16);
                    int index = ((height - y - 1) * width + x) * 3;
                    bits[index] = b;
                    bits[index + 1] = g;
                    bits[index + 2] = r;
                }
            }
            
            // Get save file name
            OPENFILENAME ofn;
            char filename[MAX_PATH] = "";
            ZeroMemory(&ofn, sizeof(ofn));

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Bitmap Images (*.bmp)\0*.bmp\0\0";
            ofn.Flags = OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn))
            {
                int len = strlen(filename);
                if (len > 4 && !(
                    (filename[len - 4] == '.') &&
                    (tolower(filename[len - 3]) == 'b') &&
                    (tolower(filename[len - 2]) == 'm') &&
                    (tolower(filename[len - 1]) == 'p')))
                {
                    strcat_s(filename, MAX_PATH, ".bmp");
                }
                Utils::SaveBitmap(filename, width, height, bits);
            }
            delete []bits;
        }

        return TRUE;
    }
    else if (uMsg == WM_RENDER_FINISH)
    {
        // Disable controls
        EnableControls(TRUE);
    
        // Update cursor
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        // Update image
        InvalidateRect(GetDlgItem(hWnd, IDC_IMAGE), NULL, TRUE);

        // Display a message
        MessageBox(hWnd, "Render finished!", "Win32 Ray Tracing Demo", MB_OK | MB_ICONINFORMATION);
        return TRUE;
    }
    else if (uMsg == WM_CLOSE)
    {
        // Delete critical sections
        DeleteCriticalSection(&csLog);
        DeleteCriticalSection(&csProgress);

        // Delte GDI objects
        DeleteDC(hdcBuffer);
        DeleteObject(hBitmap);

        // Exit
        DestroyWindow(hWnd);
        PostQuitMessage(0);
    }
    
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    // Create dialog
    hDialog = CreateDialogParam(hInstance, TEXT("DLG_MAIN"), NULL, ProcDlgMain, 0);

    // Create message loop
    MSG stMsg;
    while (GetMessage(&stMsg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&stMsg);
        DispatchMessage(&stMsg);
    }

    return 0;
}
