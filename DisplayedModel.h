#pragma once
#ifndef DISPLAYED_MODEL_H_INCLUDED
#define DISPLAYED_MODEL_H_INCLUDED

// #define CARET_ON
#define CARET_OFF

#include <windows.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include "Document.h"
#include "ScrollBar.h"

#ifdef CARET_ON
    #include "Caret.h"
#endif

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
    size_t lines;
    size_t chars;
} DefaultModel;

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
            int isHidden;
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
int Scroll(HWND hwnd, DisplayedModel* dm, size_t count, Direction dir, RECT* rectangle);

// Caret

#endif // DISPLAYED_MODEL_H_INCLUDED
