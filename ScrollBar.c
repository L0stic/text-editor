#include "ScrollBar.h"

/*  CAMERA_MODE params:
*     * ONE_LINE    - expanded area (you can highlight one character);
*     * MANY_LINES  - text fills all the free space.
*/
// #define ONE_LINE
#define MANY_LINES

void InitScrollBar(ScrollBar* pSB) {
    assert(pSB);

    pSB->pos = 0;
    pSB->maxPos = 0;
}

static ScrollBar GetRelativeSB(ScrollBar* pSB) {
    assert(pSB && pSB->pos <= pSB->maxPos);

    ScrollBar relativeSB;

    if (pSB->maxPos > MAX_POS) {
        relativeSB.maxPos = MAX_POS;
        relativeSB.pos = (size_t) roundl(((long double)pSB->pos / pSB->maxPos) * relativeSB.maxPos);
    } else {
        relativeSB.maxPos = pSB->maxPos;
        relativeSB.pos = pSB->pos;
    }

    return relativeSB;
}

void SetRelativePos(HWND hwnd, ScrollBar* pSB, int SB_TYPE) {
    assert(pSB && pSB->pos <= pSB->maxPos);
    assert(SB_TYPE == SB_VERT || SB_TYPE == SB_HORZ);

    ScrollBar relativeSB = GetRelativeSB(pSB);

    if (relativeSB.pos != GetScrollPos(hwnd, SB_TYPE)) {
        SetScrollPos(hwnd, SB_TYPE, relativeSB.pos, TRUE);
    }
}

void SetRelativeParam(HWND hwnd, ScrollBar* pSB, int SB_TYPE) {
    assert(pSB);
    assert(pSB->pos <= pSB->maxPos);
    assert(SB_TYPE == SB_VERT || SB_TYPE == SB_HORZ);

    ScrollBar relativeSB = GetRelativeSB(pSB);
    
    SetScrollRange(hwnd, SB_TYPE, 0, relativeSB.maxPos, FALSE);
    SetScrollPos(hwnd, SB_TYPE, relativeSB.pos, TRUE);
}

size_t GetAbsoluteMaxPos(size_t modelAreaParam, size_t clientAreaParam) {
    size_t shift = 1;

    if (!modelAreaParam) { return 0; }

    if (clientAreaParam == 1) { return modelAreaParam - 1; }

    #ifdef MANY_LINES
        shift = modelAreaParam < clientAreaParam ? modelAreaParam : (clientAreaParam - 1);
    #endif

    return modelAreaParam - shift;
}

size_t GetAbsolutePos(size_t relativePos, size_t absoluteMaxPos) {
    size_t absolutePos = relativePos;

    if (absoluteMaxPos > MAX_POS) {
        absolutePos = (size_t) roundl(((long double) relativePos / MAX_POS) * absoluteMaxPos);
    }

    return absolutePos;
}

// for debugging
void PrintScrollBar(const ScrollBar* pSB) {
    printf("Absolute: pos = %i of [0; %i]\n", pSB->pos, pSB->maxPos);
}

void CheckScrollBar(const HWND hwnd, int SB_TYPE) {
    assert(SB_TYPE == SB_HORZ || SB_TYPE == SB_VERT);
    int pos, bottom, top;

    pos = GetScrollPos(hwnd, SB_TYPE);
    GetScrollRange(hwnd, SB_TYPE, &bottom, &top);
    printf("Relative: pos = %i of [%i; %i]\n", pos, bottom, top);
}
