#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <commdlg.h>

#include <stdio.h>

#include "Error.h"
#include "Menu.h"
#include "Document.h"
#include "String.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

const TCHAR szClassName[]   = _T("TextEdit");
const TCHAR szTitle[]       = _T("FileName - TextEdit");
const char* example = "example.txt";

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszArgument, int nCmdShow) {
    HWND        hwnd;       /* This is the handle for our window */
    MSG         messages;   /* Here messages to the application are saved */
    WNDCLASSEX  wincl;      /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance     = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc   = WindowProcedure; /* This function is called by windows */
    wincl.style         = CS_HREDRAW | CS_VREDRAW;
    wincl.cbSize        = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = _T("Menu");
    wincl.cbClsExtra = 0; /* No extra bytes after the window class */
    wincl.cbWndExtra = 0; /* structure or the window instance */

    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx(&wincl)) {
        PrintError(NULL, ERR_UNKNOWN, __FILE__, __LINE__);
        return ERR_UNKNOWN;
    }

    /* The class is registered, let's create the program */
    hwnd = CreateWindowEx(
        0,              /* Extended possibilites for variation */
        szClassName,    /* Classname */
        szTitle,        /* Title Text */
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT,  /* Windows decides the position */
        CW_USEDEFAULT,  /* where the window ends up on the screen */
        CW_USEDEFAULT,  /* The programs width */
        CW_USEDEFAULT,  /* and height in pixels */
        HWND_DESKTOP,   /* The window is a child-window to desktop */
        NULL,           /* No menu */
        hThisInstance,  /* Program Instance handler */
        NULL            /* No Window Creation data */
    );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);
    // UpdateWindow(hwnd);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0)) {
        TranslateMessage(&messages); /* Translate virtual-key messages into character messages */
        DispatchMessage(&messages);  /* Send message to WindowProcedure */
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void InitOpenFilename(HWND hwnd, OPENFILENAME* ofn) {
/*
    static char szFilter[] = "Text Files(*.TXT)\0*.txt\0" \
                            "ASCII Files(*.ASC)\0*.asc\0" \
                            "All Files(*.*)\0*.*\0\0";
*/
    static char szFilter[] = "Text Files(*.TXT)\0*.txt\0";

    ofn->lStructSize        = sizeof(OPENFILENAME);
    ofn->hwndOwner          = hwnd;
    ofn->hInstance          = NULL;
    ofn->lpstrFilter        = szFilter;
    ofn->lpstrCustomFilter  = NULL;
    ofn->nMaxCustFilter     = 0;
    ofn->nFilterIndex       = 0;
    ofn->lpstrFile          = NULL; // Set in Open and Close functions
    ofn->nMaxFile           = _MAX_PATH;
    ofn->lpstrFileTitle     = NULL; // Set in Open and Close functions
    ofn->nMaxFileTitle      = _MAX_FNAME + _MAX_EXT;
    ofn->lpstrInitialDir    = NULL;
    ofn->lpstrTitle         = NULL;
    ofn->Flags              = 0;    // Set in Open and Close functions
    ofn->nFileOffset        = 0;
    ofn->nFileExtension     = 0;
    ofn->lpstrDefExt        = _T("txt");
    ofn->lCustData          = 0L;
    ofn->lpfnHook           = NULL;
    ofn->lpTemplateName     = NULL;
}

// BOOL FileOpenDlg(HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName) {
BOOL FileOpenDlg(HWND hwnd, OPENFILENAME* ofn, PSTR pstrFileName) {
    ofn->hwndOwner  = hwnd;
    ofn->lpstrFile  = pstrFileName;
    // ofn->lpstrFileTitle = pstrTitleName;
    ofn->Flags      = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

    return GetOpenFileName(ofn);
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static OPENFILENAME ofn;
    static PSTR pstrFilename;

    static size_t cxChar, cyChar;
    static size_t cxClient, cyClient;
    static size_t numRow;

    static Document* doc = NULL;

    HDC         hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC  tm;
    // RECT        rect;

    /* handle the messages */
    switch (message) {
    case WM_CREATE:
        doc = CreateDocument(example);
        if (!doc) { printf("All is so bad"); }

        // for debuging
        PrintDocumentParameters(NULL, doc);
        //PrintString(NULL, doc->text);
        //putchar('\n');
        PrintDocument(NULL, doc);

        hdc = GetDC(hwnd);

        GetTextMetrics(hdc, &tm);
        cxChar = (size_t)tm.tmAveCharWidth;
        cyChar = (size_t)tm.tmHeight + tm.tmExternalLeading;

        ReleaseDC(hwnd, hdc);
        break;
    // WM_CREATE

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        // numRow = (doc->text->len / cxChar + ((doc->text->len % cxChar) ? 1 : 0));

        if (doc->text->len > 0) {
            numRow = min(doc->blocks->len, cyClient / cyChar);
        } else {
            numRow = 0;
        }
        break;
    // WM_SIZE

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        if (doc) {
            Block* block = doc->blocks->nodes;

            for (size_t i = 0, j = 0; i < numRow; ++i, ++j) {
                TextOut(hdc, 0, i * cyChar, doc->text->data + block->data.pos, block->data.len);
                block = block->next;
            }
        }

        //GetClientRect(hwnd, &rect);
        //DrawText(hdc, "Hello, Windows 10!!!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        EndPaint(hwnd, &ps);
        break;
    // WM_PAINT

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_OPEN:
            printf("Open is activated\n");
            InitOpenFilename(hwnd, &ofn);
            pstrFilename = (PSTR)calloc(_MAX_PATH, sizeof(char));
            if (FileOpenDlg(hwnd, &ofn, pstrFilename)) {
                printf("%s\n", pstrFilename);
                //errorType = RebuildTextModel(&model, openFilename.lpstrFile);
                //if (errorType != ERR_NO) {
                    //free(pstrFilename);
                    //SendMessage(hwnd, WM_DESTROY, 0, 0);
                //}
            }
            free(pstrFilename);
            break;

        case IDM_FILE_EXIT:
            printf("Exit is activated\n");
            if (doc) { DestroyDocument(&doc); }

            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case IDM_VIEW_STANDARD:
            printf("Standard is activated\n");
            //if (model.displayed->viewMode != VIEW_MODE_STANDARD)
            //    SwitchMode(model.stored, model.displayed, VIEW_MODE_STANDARD);
            break;

        case IDM_VIEW_WRAP:
            printf("Wrap is activated\n");
            //if (model.displayed->viewMode != VIEW_MODE_WRAP)
            //    SwitchMode(model.stored, model.displayed, VIEW_MODE_WRAP);
            break;

        default:
            // PrintError(NULL, ERR_UNKNOWN, __FILE__, __LINE__);
            printf("Unknown problem\n");
            break;
        }

        // common actions for listed commands
        if (LOWORD(wParam) == IDM_FILE_OPEN ||
            LOWORD(wParam) == IDM_VIEW_STANDARD ||
            LOWORD(wParam) == IDM_VIEW_WRAP) {
                // update metrics binded with window size
                // 0 passed as a parameter to force recount of linesNumberWrap
                //UpdateModelMetrics(hWindow, model.stored, model.displayed, 0);

                // force repaint
                //InvalidateRect(hWindow, NULL, TRUE);
                //UpdateWindow(hWindow);
            }

        break;
    // WM_COMMAND

    case WM_DESTROY:
        PostQuitMessage(0); /* send a WM_QUIT to the message queue */
        break;
    // WM_DESTROY

    default: /* for messages that we don't deal with */
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
