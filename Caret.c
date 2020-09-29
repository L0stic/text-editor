#include "Caret.h"

void CaretDestroy(HWND hwnd) {
    HideCaret(hwnd);
    DestroyCaret();
}

void CaretShow(HWND hwnd, int* p_isHidden) {
    assert(p_isHidden);
    assert(*p_isHidden);

    #ifndef DEBUG // ===== /
        printf("Show\n");
    #endif // ============ /

    ShowCaret(hwnd);
    *p_isHidden = 0;
}

void CaretHide(HWND hwnd, int* p_isHidden) {
    assert(p_isHidden);
    assert(!*p_isHidden);

     #ifndef DEBUG // ===== /
        printf("Hide\n");
    #endif // ============ /

    HideCaret(hwnd);
    *p_isHidden = 1;
}
