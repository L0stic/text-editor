#pragma once
#ifndef SCROLL_BAR_H_INCLUDED
#define SCROLL_BAR_H_INCLUDED

#include <windows.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

// relative upper limit of the scroll range
#define MAX_POS SHRT_MAX

typedef struct {
    size_t pos;     // absolute position
    size_t maxPos;  // absolute upper limit of the scroll range
} ScrollBar;

/**
 * Inits ScrollBar object.
 * IN:
 * @param pSB - pointer to ScrollBar object
 * 
 * OUT:
 * fills fields of object with zero values
 */
void InitScrollBar(ScrollBar* pSB);

/**
 * Gets absolute upper limit of new scroll range.
 * IN:
 * @param modelAreaParam - model area range
 * @param clientAreaParam - client area range
 * 
 * OUT:
 * @return absolute upper limit of new scroll range
 */
size_t GetAbsoluteMaxPos(size_t modelAreaParam, size_t clientAreaParam);

/**
 * Gets absolute position based on relative position.
 * IN:
 * @param relativePos - relative position
 * @param absoluteMaxPos - absolute upper limit of a scroll range
 * 
 * OUT:
 * @return absolute position
 */
size_t GetAbsolutePos(size_t relativePos, size_t absoluteMaxPos);

/**
 * Sets relative position
 * IN:
 * @param hwnd - a handle to a window
 * @param pSB - pointer to ScrollBar object
 * @param SB_TYPE - a scroll bar type (SB_HORZ, SB_VERT)
 */
void SetRelativePos(HWND hwnd, ScrollBar* pSB, int SB_TYPE);

/**
 * Sets relative params (scroll range and position)
 * IN:
 * @param hwnd - a handle to a window
 * @param pSB - pointer to ScrollBar object
 * @param SB_TYPE - a scroll bar type (SB_HORZ, SB_VERT)
 */
void SetRelativeParam(HWND hwnd, ScrollBar* pSB, int SB_TYPE);

// for debugging =========================================== //
    /**
     * Prints fields of ScrollBar object.
     * IN:
     * @param pSB - pointer to ScrollBar object
     */
    void PrintScrollBar(const ScrollBar* pSB);

    /**
     * Prints relative position.
     * IN:
     * @param hwnd - a handle to a window
     * @param SB_TYPE - a scroll bar type (SB_HORZ, SB_VERT)
     */
    void CheckScrollBar(const HWND hwnd, int SB_TYPE);
// ========================================================= //

#endif // SCROLL_BAR_H_INCLUDED
