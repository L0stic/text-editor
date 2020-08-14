#pragma once
#ifndef SCROLL_BAR_H_INCLUDED
#define SCROLL_BAR_H_INCLUDED

#include <windows.h>
#include <assert.h>
#include <math.h>

#define MAX_POS SHRT_MAX

typedef struct {
    size_t pos;
    size_t maxPos;
} ScrollBar;

void InitScrollBar(ScrollBar* pSB);

// absolute params
size_t GetAbsoluteMaxPos(size_t modelAreaParam, size_t clientAreaParam);
size_t GetAbsolutePos(size_t relativePos, size_t absoluteMaxPos);

// relative params
void SetRelativePos(HWND hwnd, ScrollBar* pSB, int SB_TYPE);
void SetRelativeParam(HWND hwnd, ScrollBar* pSB, int SB_TYPE);

#endif // SCROLL_BAR_H_INCLUDED
