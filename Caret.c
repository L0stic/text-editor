#include "Caret.h"

void CaretDestroy(HWND hwnd) {
    HideCaret(hwnd);
    DestroyCaret();
}

void CaretShow(HWND hwnd, int* p_isHidden) {
    assert(p_isHidden);
    assert(*p_isHidden);

    printf("Show\n");
    ShowCaret(hwnd);
    *p_isHidden = 0;
}

void CaretHide(HWND hwnd, int* p_isHidden) {
    assert(p_isHidden);
    assert(!*p_isHidden);

    printf("Hide\n");
    HideCaret(hwnd);
    *p_isHidden = 1;
}
