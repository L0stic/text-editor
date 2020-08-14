#include "ScrollBar.h"

#define CAMERA_MODE MANY_LINES

typedef enum {
    ONE_LINE,
    MANY_LINES
} CameraMode;

void InitScrollBar(ScrollBar* pSB) {
    assert(pSB);

    pSB->pos = 0;
    pSB->maxPos = 0;
}

static ScrollBar GetRelativeSB(ScrollBar* pSB) {
    assert(pSB && pSB->pos <= pSB->maxPos);
    assert(CAMERA_MODE == ONE_LINE || CAMERA_MODE == MANY_LINES);

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

    if (CAMERA_MODE == MANY_LINES) {
        shift = modelAreaParam < clientAreaParam ? modelAreaParam : clientAreaParam;
    }
    return modelAreaParam - shift;
}

size_t GetAbsolutePos(size_t relativePos, size_t absoluteMaxPos) {
    size_t absolutePos = relativePos;

    if (absoluteMaxPos > MAX_POS) {
        absolutePos = (size_t) roundl(((long double) relativePos / MAX_POS) * absoluteMaxPos);
    }

    return absolutePos;
}
