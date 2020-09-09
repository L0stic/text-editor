#pragma once
#ifndef DISPLAYED_MODEL_H_INCLUDED
#define DISPLAYED_MODEL_H_INCLUDED

#define CARET_ON
// #define CARET_OFF

#include <windows.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include "Document.h"
#include "ScrollBar.h"

#ifdef CARET_ON
    #include "Caret.h"
#endif

#define DECREMENT_OF(elem) (elem - 1)

typedef enum {
    FORMAT_MODE_DEFAULT,
    FORMAT_MODE_WRAP
} FormatMode;

typedef enum {
         UP,
    LEFT,   RIGHT,
        DOWN
} Direction;

typedef struct {
    size_t x;
    size_t y;
} metric_t;

typedef struct {
    size_t x;
    size_t y;
} position_t;

typedef struct {
    size_t lines;
    size_t chars;
} area_t;

typedef struct {
    int isValid;
    size_t lines;
} WrapModel;

typedef struct {
    Block* block;
    position_t pos;
} ModelPos;

typedef struct {
    metric_t charMetric;

    FormatMode mode;
    Document* doc;

    area_t clientArea;
    area_t documentArea;
    WrapModel wrapModel;

    struct {
        ScrollBar horizontal;
        ScrollBar vertical;
        ModelPos modelPos;
    } scrollBars;

    #ifdef CARET_ON
        struct {
            struct {
                int x;
                int y;
            } isHidden;
            position_t clientPos;
            ModelPos modelPos;
        } caret;
    #endif
} DisplayedModel;

void InitDisplayedModel(DisplayedModel* dm, TEXTMETRIC* tm);
void UpdateDisplayedModel(HWND hwnd, DisplayedModel* dm, LPARAM lParam);
void CoverDocument(HWND hwnd, DisplayedModel* dm, Document* doc);

void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode);
void DisplayModel(HDC hdc, const DisplayedModel* dm);

// Scroll-bar
size_t Scroll(HWND hwnd, DisplayedModel* dm, size_t count, Direction dir, RECT* rectangle);

// Caret
#ifdef CARET_ON
    void CaretPrintParams(DisplayedModel* dm);

    void FindEnd_Left(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void FindEnd_Right(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void FindHome(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    void CaretMoveToTop(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void CaretMoveToBottom(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void CaretMoveToRight(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void CaretMoveToLeft(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    void CaretPageUp(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void CaretPageDown(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    void FindCaret(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    void CaretSetPos(DisplayedModel* dm);
    void CaretCreate(HWND hwnd, DisplayedModel* dm);
    void CaretDestroy(HWND hwnd);
    void CaretShow(HWND hwnd, int* p_isHidden);
    void CaretHide(HWND hwnd, int* p_isHidden);

    void CaretTopLeftBorder(HWND hwnd, int* p_isHidden, size_t scrollValue, size_t modelPos, size_t scrollBarPos, size_t* pClientPos, size_t clientPosMax);
    void CaretBottomRightBorder(HWND hwnd, int* p_isHidden, size_t scrollValue, size_t modelPos, size_t scrollBarPos, size_t* pClientPos, size_t clientPosMax);
#endif

#endif // DISPLAYED_MODEL_H_INCLUDED
