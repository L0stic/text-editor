#include "DisplayedModel.h"

// don't touch this parameters
#define STEP 1

static void InitModelPos(ModelPos* pMP, Block* block) {
    assert(pMP);

    pMP->block = block;
    pMP->pos.x = 0;
    pMP->pos.y = 0;
}

void InitDisplayedModel(DisplayedModel* dm, TEXTMETRIC* tm) {
    assert(dm);
    assert(tm);

    dm->charMetric.x = tm->tmAveCharWidth;
    dm->charMetric.y = tm->tmHeight + tm->tmExternalLeading;

    dm->mode = FORMAT_MODE_DEFAULT;
    dm->doc = NULL;

    dm->clientArea.chars = 0;
    dm->clientArea.lines = 0;

    dm->documentArea.chars = 0;
    dm->documentArea.lines = 0;

    dm->wrapModel.isValid = 0;
    dm->wrapModel.lines = 0;

    InitScrollBar(&(dm->scrollBars.horizontal));
    InitScrollBar(&(dm->scrollBars.vertical));
    InitModelPos(&(dm->scrollBars.modelPos), NULL);

    #ifdef CARET_ON
        dm->caret.isHidden.x = 0;
        dm->caret.isHidden.y = 0;
        dm->caret.clientPos.x = 0;
        dm->caret.clientPos.y = 0;
        InitModelPos(&(dm->caret.modelPos), NULL);
    #endif
}


static void PrintWrapModel(WrapModel* wrapModel) {
    if (wrapModel->isValid) {
        printf("Wrap model: %u\n", wrapModel->lines);
    } else {
        printf("Wrap model is not valid\n");
    }
}

static size_t BuildWrapModel(DisplayedModel* dm) {
    printf("BuildWrapModel\n");
    assert(dm);
    
    size_t relativePos; // absolute?
    Block* block = dm->doc->blocks->nodes;

    dm->scrollBars.modelPos.pos.x = 0;

    dm->wrapModel.lines = 0;

    while (block != dm->scrollBars.modelPos.block) {
        if (block->data.len > 0) {
            dm->wrapModel.lines += DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
        } else {
            ++dm->wrapModel.lines;
        }
        block = block->next;
    }

    relativePos = dm->wrapModel.lines;
    
    while (block) {
        if (block->data.len > 0) {
            dm->wrapModel.lines += DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
        } else {
            ++dm->wrapModel.lines;
        }
        block = block->next;
    }

    dm->wrapModel.isValid = 1;
    return relativePos;
}


static void PrintScrollBar(const ScrollBar* pSB) {
    printf("Absolute: pos = %i of [0; %i]\n", pSB->pos, pSB->maxPos);
}


static void CheckScrollBar(const HWND hwnd) {
    int pos, bottom, top;

    pos = GetScrollPos(hwnd, SB_HORZ);
    GetScrollRange(hwnd, SB_HORZ, &bottom, &top);
    printf("Relative: pos = %i of [%i; %i]\n", pos, bottom, top);
}


static void UpdateVerticalSB_Default(DisplayedModel* dm) {
    assert(dm);
    dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->documentArea.lines, dm->clientArea.lines);

    if (dm->scrollBars.vertical.pos > dm->scrollBars.vertical.maxPos) {
        // pass
        for(size_t delta = dm->scrollBars.vertical.pos - dm->scrollBars.vertical.maxPos; delta > 0; --delta) {
            --dm->scrollBars.modelPos.pos.y;
            dm->scrollBars.modelPos.block = dm->scrollBars.modelPos.block->prev;
        }
        dm->scrollBars.vertical.pos = dm->scrollBars.vertical.maxPos;
    }
}


static void UpdateVerticalSB_Wrap(DisplayedModel* dm) {
    assert(dm);
    dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->wrapModel.lines, dm->clientArea.lines);

    if (dm->scrollBars.vertical.pos > dm->scrollBars.vertical.maxPos) {
        // pass
        for (size_t delta = dm->scrollBars.vertical.pos - dm->scrollBars.vertical.maxPos; delta > 0;) {
            if (dm->scrollBars.modelPos.pos.x + 1 <= delta) {
                delta -= dm->scrollBars.modelPos.pos.x + 1;
            } else {
                dm->scrollBars.modelPos.pos.x -= delta;
                break;
            }

            // prev block
            dm->scrollBars.modelPos.block = dm->scrollBars.modelPos.block->prev;
            --dm->scrollBars.modelPos.pos.y;
            if (dm->scrollBars.modelPos.block->data.len > 0) {
                dm->scrollBars.modelPos.pos.x = DIV_WITH_ROUND_UP(dm->scrollBars.modelPos.block->data.len, dm->clientArea.chars) - 1;
            } else {
                dm->scrollBars.modelPos.pos.x = 0;
            }
        }
        dm->scrollBars.vertical.pos = dm->scrollBars.vertical.maxPos;
    }
}


void CoverDocument(HWND hwnd, DisplayedModel* dm, Document* doc) {
    printf("Cover document\n");
    assert(dm && doc);

    dm->doc = doc;
    dm->documentArea.lines = doc->blocks->len;
    dm->documentArea.chars = GetMaxBlockLen(doc->blocks);

    dm->wrapModel.isValid = 0;

    InitScrollBar(&(dm->scrollBars.horizontal));
    InitScrollBar(&(dm->scrollBars.vertical));
    InitModelPos(&(dm->scrollBars.modelPos), doc->blocks->nodes);

    #ifdef CARET_ON
        dm->caret.isHidden.x = 0;
        dm->caret.isHidden.y = 0;
        dm->caret.clientPos.x = 0;
        dm->caret.clientPos.y = 0;
        InitModelPos(&(dm->caret.modelPos), doc->blocks->nodes);
    #endif

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(dm->documentArea.chars, dm->clientArea.chars);
        dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->documentArea.lines, dm->clientArea.lines);
        break;

    case FORMAT_MODE_WRAP:
        PrintWrapModel(&(dm->wrapModel));

        BuildWrapModel(dm);
        PrintWrapModel(&(dm->wrapModel));
        dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->wrapModel.lines, dm->clientArea.lines);
        break;

    default:
        // TODO: ERROR
        return;
    }

    SetRelativeParam(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
    SetRelativeParam(hwnd, &(dm->scrollBars.vertical), SB_VERT);

    #ifndef DEBUG // ================================/
        // printf("\tHorizontal\n");
        // PrintScrollBar(&(dm->scrollBars.horizontal));
        // printf("\tVertical\n");
        // PrintScrollBar(&(dm->scrollBars.vertical));
        // putchar('\n');
    #endif // =======================================/
}


static void PrintPos(DisplayedModel* dm) {
    assert(dm);
    printf("blockPos = %u, ", dm->scrollBars.modelPos.pos.y);
    printf("charPos = %u\n", dm->scrollBars.modelPos.pos.x);
}


void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode) {
    printf("Switch mode\n");
    assert(dm);

    dm->scrollBars.horizontal.pos = 0;
    #ifndef DEBUG // ====/
        // PrintPos(dm);
    #endif // ===========/

    switch(mode) {
    case FORMAT_MODE_DEFAULT:
        printf("Default mode is activated\n");
        dm->mode = FORMAT_MODE_DEFAULT;
        ++dm->clientArea.chars;

        dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(dm->documentArea.chars, dm->clientArea.chars);

        dm->scrollBars.vertical.pos = dm->scrollBars.modelPos.pos.y;
        UpdateVerticalSB_Default(dm);
        break;

    case FORMAT_MODE_WRAP:
        printf("Wrap mode is activated\n");
        dm->mode = FORMAT_MODE_WRAP;
        --dm->clientArea.chars;

        dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(0, dm->clientArea.chars);

        dm->scrollBars.vertical.pos = BuildWrapModel(dm);
        UpdateVerticalSB_Wrap(dm);
        break;

    default:
        printf("Unknown mode\n");
        return;
    }

    SetRelativeParam(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
    SetRelativeParam(hwnd, &(dm->scrollBars.vertical), SB_VERT);

    #ifndef DEBUG // ====/
        // PrintPos(dm);
    #endif // ===========/
}

void DisplayModel(HDC hdc, const DisplayedModel* dm) {
    assert(dm && dm->doc && dm->doc->text);

    Block* block = dm->scrollBars.modelPos.block;
    size_t displayedLines, displayedChars;
    size_t linesBlock, nextLine;

    #ifndef DEBUG // ================================/
        // printf("Display model:\n");
        // printf("\tHorizontal\n");
        // PrintScrollBar(&(dm->scrollBars.horizontal));
        // printf("\tVertical\n");
        // PrintScrollBar(&(dm->scrollBars.vertical));
        // putchar('\n');
    #endif // =======================================/

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        displayedLines = min(dm->clientArea.lines, dm->documentArea.lines - dm->scrollBars.vertical.pos);

        // print
        for (size_t i = 0; i < displayedLines; ++i) {
            if (dm->scrollBars.horizontal.pos < block->data.len) {
                displayedChars = min(dm->clientArea.chars, block->data.len - dm->scrollBars.horizontal.pos);
                TextOut(hdc,
                        0,
                        i * dm->charMetric.y,
                        dm->doc->text->data + block->data.pos + dm->scrollBars.horizontal.pos,
                        displayedChars);
            }

            block = block->next;
        }
        break;

    case FORMAT_MODE_WRAP:
        nextLine = dm->scrollBars.modelPos.pos.x;
        // nextLine = DIV_WITH_ROUND_UP(dm->scrollBars.modelPos.pos.x, dm->clientArea.chars);

        displayedLines = min(dm->clientArea.lines, dm->wrapModel.lines - dm->scrollBars.vertical.pos);

        block = dm->scrollBars.modelPos.block;

        // print
        for (size_t i = 0; i < displayedLines; nextLine = 0) {
            if (block->data.len == 0) {
                ++i;
                continue;
            }

            linesBlock = DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);

            if (linesBlock - nextLine <= displayedLines - i) {
                for (; nextLine < linesBlock - 1; ++i, ++nextLine) {
                    TextOut(hdc,
                            0,
                            i * dm->charMetric.y,
                            dm->doc->text->data + block->data.pos + nextLine * dm->clientArea.chars,
                            dm->clientArea.chars);
                }

                TextOut(hdc,
                        0,
                        i * dm->charMetric.y,
                        dm->doc->text->data + block->data.pos + nextLine * dm->clientArea.chars,
                        (block->data.len % dm->clientArea.chars) ? (block->data.len % dm->clientArea.chars) : dm->clientArea.chars);
                ++i;
            } else {
                for (int delta = displayedLines - i; delta > 0; --delta) {
                    TextOut(hdc,
                            0,
                            i * dm->charMetric.y,
                            dm->doc->text->data + block->data.pos + nextLine * dm->clientArea.chars,
                            dm->clientArea.chars);
                    ++i;
                    ++nextLine;
                }
            }

            block = block->next;
        }
        break;

    default:
        return;
    }
}

static void UpdateScrollPos_Back(DisplayedModel* dm, size_t count) {
    assert(dm);

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        // for remaining
        while (dm->scrollBars.modelPos.pos.y != dm->scrollBars.vertical.pos) {
            dm->scrollBars.modelPos.block = dm->scrollBars.modelPos.block->prev;
            --dm->scrollBars.modelPos.pos.y;
        }
        break;

    case FORMAT_MODE_WRAP:
        // for remaining
        for (; count > 0;) {
            if (dm->scrollBars.modelPos.pos.x + 1 <= count) {
                count -= dm->scrollBars.modelPos.pos.x + 1;
            } else {
                dm->scrollBars.modelPos.pos.x -= count;
                break;
            }

            // prev block
            dm->scrollBars.modelPos.block = dm->scrollBars.modelPos.block->prev;
            --(dm->scrollBars.modelPos.pos.y);
            if (dm->scrollBars.modelPos.block->data.len > 0) {
                dm->scrollBars.modelPos.pos.x = DIV_WITH_ROUND_UP(dm->scrollBars.modelPos.block->data.len, dm->clientArea.chars) - 1;
            } else {
                dm->scrollBars.modelPos.pos.x = 0;
            }
        }
        break;

    default:
        break;
    }
}

static void UpdateScrollPos_Forward(DisplayedModel* dm, size_t count) {
    assert(dm);

    size_t delta;
    size_t linesBlock = 0;
    Block* block = dm->scrollBars.modelPos.block;    

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        // for remaining
        while (dm->scrollBars.modelPos.pos.y != dm->scrollBars.vertical.pos) {
            dm->scrollBars.modelPos.block = dm->scrollBars.modelPos.block->next;
            ++dm->scrollBars.modelPos.pos.y;
        }
        break;

    case FORMAT_MODE_WRAP:
        // for remaining
        if (block->data.len > 0) {
            linesBlock = DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
        } else {
            linesBlock = 1;
        }

        delta = linesBlock - dm->scrollBars.modelPos.pos.x;
        dm->scrollBars.modelPos.pos.x = 0;

        for (; count > 0; linesBlock = 0) {
            if (count >= delta) {
                count -= delta;
            } else {
                dm->scrollBars.modelPos.pos.x = linesBlock - (delta - count);
                break;
            }

            block = block->next;

            // next block
            dm->scrollBars.modelPos.block = block;
            ++dm->scrollBars.modelPos.pos.y;

            if (block->data.len > 0) {
                linesBlock = DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
            } else {
                linesBlock = 1;
            }
            delta = linesBlock;
        }
        break;

    default:
        break;
    }
}

size_t Scroll(HWND hwnd, DisplayedModel* dm, size_t count, Direction direction, RECT* rectangle) {
    assert(dm);
    assert(rectangle);

    int xScroll = 0;
    int yScroll = 0;

    // Init rectangle
    rectangle->left = 0;
    rectangle->right = (long) (dm->charMetric.x * dm->clientArea.chars);
    rectangle->top = 0;
    rectangle->bottom = (long) (dm->charMetric.y * dm->clientArea.lines);

    switch (direction) {
    case UP:
        count = min(count, dm->scrollBars.vertical.pos);
        if (count) {
            dm->scrollBars.vertical.pos -= count;

            UpdateScrollPos_Back(dm, count);
            SetRelativePos(hwnd, &(dm->scrollBars.vertical), SB_VERT);

            yScroll = (int) (count * dm->charMetric.y);
            rectangle->bottom = dm->charMetric.y * min(count, dm->clientArea.lines);
        }
        break;
    // UP
    
    case DOWN:
        count = min(count, dm->scrollBars.vertical.maxPos - dm->scrollBars.vertical.pos);
        if (count) {
            dm->scrollBars.vertical.pos += count;

            UpdateScrollPos_Forward(dm, count);
            SetRelativePos(hwnd, &(dm->scrollBars.vertical), SB_VERT);

            yScroll = - (int) (count * dm->charMetric.y);
            rectangle->top = dm->charMetric.y * (dm->clientArea.lines - min(count, dm->clientArea.lines));
        }
        break;
    // DOWN

    case LEFT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        count = min(count, dm->scrollBars.horizontal.pos);
        if (count) {
            dm->scrollBars.horizontal.pos -= count;

            SetRelativePos(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);

            xScroll = (int) (count * dm->charMetric.x);
            rectangle->left = dm->charMetric.x * (dm->clientArea.chars - min(count, dm->clientArea.chars));
        }
        break;
    // LEFT

    case RIGHT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        count = min(count, dm->scrollBars.horizontal.maxPos - dm->scrollBars.horizontal.pos);
        if (count) {
            dm->scrollBars.horizontal.pos += count;
            SetRelativePos(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);

            xScroll = - (int) (count * dm->charMetric.x);
            rectangle->right = dm->charMetric.x * min(count, dm->clientArea.chars);
        }
        break;
    // RIGHT

    default:
    // TODO: error
        return 0;
    }

    ScrollWindow(hwnd, xScroll, yScroll, NULL, NULL);

    // Repaint rectangle
    InvalidateRect(hwnd, rectangle, TRUE);
    UpdateWindow(hwnd);

    #ifndef DEBUG // ==============================================/
        // PrintPos(dm);
        // PrintScrollBar(&(dm->scrollBars.vertical));
        // PrintScrollBar(&(dm->scrollBars.horizontal));
    #endif // =====================================================/

    return count;
}

static void PassPrev(ModelPos* modelPos, size_t delta) {
    assert(modelPos);

    modelPos->pos.y -= delta;
    for(; delta > 0; --delta) {
        assert(modelPos->block);
        modelPos->block = modelPos->block->prev;
    }
}

static void PassNext(ModelPos* modelPos, size_t delta) {
    assert(modelPos);

    modelPos->pos.y += delta;
    for(; delta > 0; --delta) {
        assert(modelPos->block);
        modelPos->block = modelPos->block->next;
    }
}

// Caret
#ifdef CARET_ON

void CaretPrintParams(DisplayedModel* dm) {
    assert(dm);

    printf("Caret params:\n");
    printf("\tIs hidden: [%u; %u]\n", dm->caret.isHidden.x, dm->caret.isHidden.y);
    printf("\tClient pos: [%u; %u]\n", dm->caret.clientPos.x, dm->caret.clientPos.y);
    printf("\tModel pos: [%u; %u]\n", dm->caret.modelPos.pos.x, dm->caret.modelPos.pos.y);
}

void FindEnd_Left(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    if (dm->caret.modelPos.pos.x > dm->caret.modelPos.block->data.len) {
        size_t deltaModel = dm->caret.modelPos.pos.x - dm->caret.modelPos.block->data.len;

        if (dm->caret.clientPos.x < deltaModel) {
            Scroll(hwnd, dm, deltaModel - dm->caret.clientPos.x, LEFT, rectangle);
            dm->caret.clientPos.x = 0;
        } else {
            dm->caret.clientPos.x -= deltaModel;
        }

        dm->caret.modelPos.pos.x = dm->caret.modelPos.block->data.len;
    }
}

void FindEnd_Right(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    if (dm->caret.modelPos.pos.x < dm->caret.modelPos.block->data.len) {
        size_t deltaModel = dm->caret.modelPos.block->data.len - dm->caret.modelPos.pos.x;
        size_t deltaClient = DECREMENT_OF(dm->clientArea.chars) - dm->caret.clientPos.x;

        if (deltaModel > deltaClient) {
            Scroll(hwnd, dm, deltaModel - deltaClient, RIGHT, rectangle);
            dm->caret.clientPos.x = DECREMENT_OF(dm->clientArea.chars);
        } else {
            dm->caret.clientPos.x += deltaModel;
        }

        dm->caret.modelPos.pos.x = dm->caret.modelPos.block->data.len;
    }
}

void FindHome(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    if (dm->caret.modelPos.pos.x > 0) {
        size_t deltaModel = dm->caret.modelPos.pos.x;
        size_t deltaClient = dm->caret.clientPos.x;

        if (deltaModel > deltaClient) {
            Scroll(hwnd, dm, deltaModel - deltaClient, LEFT, rectangle);
        }
        dm->caret.clientPos.x = 0;

        dm->caret.modelPos.pos.x = 0;
    }
}

void CaretMoveToTop(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    --dm->caret.modelPos.pos.y;
    dm->caret.modelPos.block = dm->caret.modelPos.block->prev;
    
    if (dm->caret.clientPos.y > 0) {
        --dm->caret.clientPos.y;
    } else {
        Scroll(hwnd, dm, STEP, UP, rectangle);
    }
}

void CaretMoveToBottom(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    ++dm->caret.modelPos.pos.y;
    dm->caret.modelPos.block = dm->caret.modelPos.block->next;
    
    if (dm->caret.clientPos.y < DECREMENT_OF(dm->clientArea.lines - 1)) {
        ++dm->caret.clientPos.y;
    } else {
        Scroll(hwnd, dm, STEP, DOWN, rectangle);
    }
}

void CaretMoveToRight(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    ++dm->caret.modelPos.pos.x;
    if (dm->caret.clientPos.x <  DECREMENT_OF(dm->clientArea.chars)) {
        ++dm->caret.clientPos.x;
    } else {
        Scroll(hwnd, dm, STEP, RIGHT, rectangle);
    }
}

void CaretMoveToLeft(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    --dm->caret.modelPos.pos.x;

    if (dm->caret.clientPos.x > 0) {
        --dm->caret.clientPos.x;
    } else {
        Scroll(hwnd, dm, STEP, LEFT, rectangle);
    }
}

void CaretPageUp(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    size_t delta = min(DECREMENT_OF(dm->clientArea.lines - 1), dm->scrollBars.vertical.pos);
    printf("%u\n", delta);

    if (dm->caret.modelPos.pos.y > 0) {
        PassPrev(&(dm->caret.modelPos), delta);
        Scroll(hwnd, dm, delta, UP, rectangle);
    }
}

void CaretPageDown(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm && rectangle);

    size_t delta = min(DECREMENT_OF(dm->clientArea.lines - 1), dm->scrollBars.vertical.maxPos - dm->scrollBars.vertical.pos);
    printf("%u\n", delta);

    if (dm->caret.modelPos.pos.y - dm->caret.clientPos.y + delta <= dm->scrollBars.vertical.maxPos) {
        PassNext(&(dm->caret.modelPos), delta);
        Scroll(hwnd, dm, delta, DOWN, rectangle);
    }
}

void CaretSetPos(DisplayedModel* dm) {
    assert(dm);

    if (!(dm->caret.isHidden.y | dm->caret.isHidden.x)) {
        SetCaretPos(dm->caret.clientPos.x * dm->charMetric.x, dm->caret.clientPos.y * dm->charMetric.y);
    }
}

void CaretCreate(HWND hwnd, DisplayedModel* dm) {
    assert(dm);

    CreateCaret(hwnd, NULL, dm->charMetric.x, dm->charMetric.y);
    CaretSetPos(dm);
    ShowCaret(hwnd);
}

void CaretDestroy(HWND hwnd) {
    HideCaret(hwnd);
    DestroyCaret();
}

void CaretShow(HWND hwnd, int* p_isHidden) {
    assert(p_isHidden);

    printf("Show\n");
    ShowCaret(hwnd);
    *p_isHidden = 0;
}

void CaretHide(HWND hwnd, int* p_isHidden) {
    assert(p_isHidden);

    printf("Hide\n");
    HideCaret(hwnd);
    *p_isHidden = 1;
}

void CaretTopLeftBorder(HWND hwnd, int* p_isHidden, size_t scrollValue, size_t modelPos, size_t scrollBarPos, size_t* pClientPos, size_t clientPosMax) {
    assert(p_isHidden && pClientPos);

    // caret is over the top/left border
    if (*p_isHidden && *pClientPos == 0) {
        if (scrollBarPos <= modelPos) {
            // caret is in the client area
            *pClientPos = modelPos - scrollBarPos;
            CaretShow(hwnd, p_isHidden);
        }
    } else if (!*p_isHidden) {                        
        if (clientPosMax >= *pClientPos + scrollValue) {
            // caret is in the client area
            *pClientPos += scrollValue;

        } else {
            // caret is over the bottom/right border
            *pClientPos = clientPosMax;
            CaretHide(hwnd, p_isHidden);
        }
    }
}

void CaretBottomRightBorder(HWND hwnd, int* p_isHidden, size_t scrollValue, size_t modelPos, size_t scrollBarPos, size_t* pClientPos, size_t clientPosMax) {
    assert(p_isHidden && pClientPos);

    if (*p_isHidden && *pClientPos != 0) {
        if (scrollBarPos + clientPosMax >= modelPos) {
            // caret is in the client area
            *pClientPos = modelPos - scrollBarPos;
            CaretShow(hwnd, p_isHidden);
            
        } else {
            // caret is over the bottom/right border
            *pClientPos = clientPosMax;
        }
    } else if (!*p_isHidden) {
        if (*pClientPos > clientPosMax) {
            // caret is over the bottom/right border
            *pClientPos = clientPosMax;
            CaretHide(hwnd, p_isHidden);

        } else if (*pClientPos >= scrollValue) {
            *pClientPos -= scrollValue;

        } else {
            // caret is over the top/left border
            *pClientPos = 0;
            CaretHide(hwnd, p_isHidden);
        }
    }
}

void FindCaret(HWND hwnd, DisplayedModel* dm, RECT* rectangle) {
    assert(dm);
    size_t scrollValue;

    if (dm->caret.isHidden.x) {
        // left border
        if (dm->caret.modelPos.pos.x < dm->scrollBars.horizontal.pos) {
            scrollValue = dm->scrollBars.horizontal.pos - dm->caret.modelPos.pos.x;
            scrollValue = Scroll(hwnd, dm, scrollValue, LEFT, rectangle);
            CaretTopLeftBorder(hwnd, &(dm->caret.isHidden.x), scrollValue,
                                        dm->caret.modelPos.pos.x, dm->scrollBars.horizontal.pos,
                                        &(dm->caret.clientPos.x), DECREMENT_OF(dm->clientArea.chars));
        }

        // right border
        if (dm->caret.modelPos.pos.x > dm->scrollBars.horizontal.pos + DECREMENT_OF(dm->clientArea.chars)) {
            scrollValue = dm->caret.modelPos.pos.x - (dm->scrollBars.horizontal.pos + DECREMENT_OF(dm->clientArea.chars));
            scrollValue = Scroll(hwnd, dm, scrollValue, RIGHT, rectangle);
            CaretBottomRightBorder(hwnd, &(dm->caret.isHidden.x), scrollValue,
                                        dm->caret.modelPos.pos.x, dm->scrollBars.horizontal.pos,
                                        &(dm->caret.clientPos.x), DECREMENT_OF(dm->clientArea.chars));
        }
    }

    if (dm->caret.isHidden.y) {
        // top
        if (dm->caret.modelPos.pos.y < dm->scrollBars.vertical.pos) {
            scrollValue = dm->scrollBars.vertical.pos - dm->caret.modelPos.pos.y;
            scrollValue = Scroll(hwnd, dm, scrollValue, UP, rectangle);
            CaretTopLeftBorder(hwnd, &(dm->caret.isHidden.y), scrollValue,
                                        dm->caret.modelPos.pos.y, dm->scrollBars.vertical.pos,
                                        &(dm->caret.clientPos.y), DECREMENT_OF(dm->clientArea.lines - 1));
        }

        // bottom
        if (dm->caret.modelPos.pos.y > dm->scrollBars.vertical.pos + DECREMENT_OF(dm->clientArea.lines - 1)) {
            scrollValue = dm->caret.modelPos.pos.y - (dm->scrollBars.vertical.pos + DECREMENT_OF(dm->clientArea.lines - 1));
            scrollValue = Scroll(hwnd, dm, scrollValue, DOWN, rectangle);
            CaretBottomRightBorder(hwnd, &(dm->caret.isHidden.y), scrollValue,
                                        dm->caret.modelPos.pos.y, dm->scrollBars.vertical.pos,
                                        &(dm->caret.clientPos.y), DECREMENT_OF(dm->clientArea.lines - 1));
        }
    }

    if (dm->clientArea.lines > STEP && dm->caret.clientPos.y == DECREMENT_OF(dm->clientArea.lines)) {
        --dm->caret.clientPos.y;
        Scroll(hwnd, dm, STEP, DOWN, rectangle);
    }
}

#endif

void UpdateDisplayedModel(HWND hwnd, DisplayedModel* dm, LPARAM lParam) {
    // printf("UpdateDisplayedModel\n");
    assert(dm);

    int isCharsChanged = 0;
    int isLinesChanged = 0;

    size_t chars;
    size_t lines = DIV_WITH_ROUND_UP(HIWORD(lParam), dm->charMetric.y);
    size_t scrollValue = 0;

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        chars = DIV_WITH_ROUND_UP(LOWORD(lParam), dm->charMetric.x);
        break;

    case FORMAT_MODE_WRAP:
        chars = LOWORD(lParam) / dm->charMetric.x;
        break;

    default:
        return;
    }

    if (dm->clientArea.chars != chars) {
        dm->clientArea.chars = chars;
        isCharsChanged = 1;
    }

    if (dm->clientArea.lines != lines) {
        dm->clientArea.lines = lines;
        isLinesChanged = 1;
    }

    if (!isCharsChanged && !isLinesChanged) { return; }

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        // Horizontal scroll-bar
        if (isCharsChanged) {
            dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(dm->documentArea.chars, dm->clientArea.chars);

            if (dm->scrollBars.horizontal.pos > dm->scrollBars.horizontal.maxPos) {
                #ifdef CARET_ON
                    scrollValue = dm->scrollBars.horizontal.pos - dm->scrollBars.horizontal.maxPos;
                #endif

                dm->scrollBars.horizontal.pos = dm->scrollBars.horizontal.maxPos;

                #ifdef CARET_ON
                    // left border
                    CaretTopLeftBorder(hwnd, &(dm->caret.isHidden.x), scrollValue,
                                        dm->caret.modelPos.pos.x, dm->scrollBars.horizontal.pos,
                                        &(dm->caret.clientPos.x), DECREMENT_OF(dm->clientArea.chars));
                #endif
            }
            #ifdef CARET_ON 
                else {
                    // right border
                    CaretBottomRightBorder(hwnd, &(dm->caret.isHidden.x), 0,
                                        dm->caret.modelPos.pos.x, dm->scrollBars.horizontal.pos,
                                        &(dm->caret.clientPos.x), DECREMENT_OF(dm->clientArea.chars));
                }
            #endif
        }

        // Vertical scroll-bar
        if (isLinesChanged) {
            dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->documentArea.lines, dm->clientArea.lines);

            if (dm->scrollBars.vertical.pos > dm->scrollBars.vertical.maxPos) {
                scrollValue = dm->scrollBars.vertical.pos - dm->scrollBars.vertical.maxPos;

                // pass
                for(size_t i = 0; i < scrollValue; ++i) {
                    --dm->scrollBars.modelPos.pos.y;
                    dm->scrollBars.modelPos.block = dm->scrollBars.modelPos.block->prev;
                }
                dm->scrollBars.vertical.pos = dm->scrollBars.vertical.maxPos;

                #ifdef CARET_ON
                    // top
                    CaretTopLeftBorder(hwnd, &(dm->caret.isHidden.y), scrollValue,
                                        dm->caret.modelPos.pos.y, dm->scrollBars.vertical.pos,
                                        &(dm->caret.clientPos.y), DECREMENT_OF(dm->clientArea.lines));
                #endif
            }
            #ifdef CARET_ON 
                else {
                    // bottom
                    CaretBottomRightBorder(hwnd, &(dm->caret.isHidden.y), 0,
                                        dm->caret.modelPos.pos.y, dm->scrollBars.vertical.pos,
                                        &(dm->caret.clientPos.y), DECREMENT_OF(dm->clientArea.lines));
                }
            #endif
        }
        break;
    // FORMAT_MODE_DEFAULT

    case FORMAT_MODE_WRAP:
        PrintWrapModel(&(dm->wrapModel));

        // Horizontal scroll-bar
        if (dm->documentArea.chars >= dm->clientArea.chars && isCharsChanged) {
            dm->scrollBars.horizontal.maxPos = 0;
            dm->scrollBars.horizontal.pos = 0;

            dm->wrapModel.isValid = 0;
            dm->scrollBars.vertical.pos = BuildWrapModel(dm);
            PrintWrapModel(&(dm->wrapModel));
        }

        // Vertical scroll-bar
        UpdateVerticalSB_Wrap(dm);
        PrintWrapModel(&(dm->wrapModel));
        break;
    // FORMAT_MODE_WRAP
    
    default:
        return;
    }

    SetRelativeParam(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
    SetRelativeParam(hwnd, &(dm->scrollBars.vertical), SB_VERT);
}
