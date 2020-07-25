#include "DisplayedModel.h"

void InitDisplayedModel(DisplayedModel* dm, TEXTMETRIC* tm) {
    assert(dm && tm);

    dm->charMetric.x = tm->tmAveCharWidth;
    dm->charMetric.y = tm->tmHeight + tm->tmExternalLeading;

    dm->clientArea.chars = 0;
    dm->clientArea.lines = 0;

    dm->documentArea.chars = 0;
    dm->documentArea.lines = 0;

    dm->wrapModel.isValid = 0;
    dm->wrapModel.lines = 0;

    InitScrollBar(&(dm->scrollBars.horizontal));
    InitScrollBar(&(dm->scrollBars.vertical));

    dm->mode = FORMAT_MODE_DEFAULT;
    dm->doc = NULL;

    dm->currentPos.block = NULL;
    dm->currentPos.blockPos = 0;
    dm->currentPos.charPos = 0;
}

// TODO: delete ===============================================
static int GetMaxPos(size_t requiredMax) {
    return requiredMax <= MAX_POS ? (int)requiredMax : MAX_POS;
}

size_t GetCurrentPos(int pos, size_t requiredMax) {
    if (requiredMax > MAX_POS) {
        long double part = (long double) requiredMax / MAX_POS;
        return (int) roundl(part * pos);
    }
    return (size_t)pos;
}

int GetPos(size_t pos, size_t requiredMax) {
    if (requiredMax > MAX_POS) {
        long double part = (long double) requiredMax / MAX_POS;
        return (int) roundl(part * pos);
    }
    return (size_t)pos;
}

static int SetRange(HWND hwnd, size_t modelAreaParam, size_t clientAreaParam, int SB_TYPE) {
    int maxPos = modelAreaParam > clientAreaParam ? GetMaxPos(modelAreaParam - clientAreaParam) : 0;
    SetScrollRange(hwnd, SB_TYPE, 0, maxPos, FALSE);
    return maxPos;
}
//=============================================================

static size_t BuildWrapModel(DisplayedModel* dm) {
    printf("BuildWrapModel\n");
    assert(dm);
    size_t relativePos;
    Block* block = dm->doc->blocks->nodes;

    dm->wrapModel.lines = 0;

    while (block != dm->currentPos.block) {
        if (block->data.len > 0) {
            dm->wrapModel.lines += DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
        } else {
            ++(dm->wrapModel.lines);
        }
        block = block->next;
    }
    relativePos = dm->wrapModel.lines;
    while (block) {
        if (block->data.len > 0) {
            dm->wrapModel.lines += DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
        } else {
            ++(dm->wrapModel.lines);
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

static void UpdateVerticalSB(DisplayedModel* dm) {
    dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->documentArea.lines, dm->clientArea.lines);

    if (dm->scrollBars.vertical.pos > dm->scrollBars.vertical.maxPos) {
        // pass
        for(size_t delta = dm->scrollBars.vertical.pos - dm->scrollBars.vertical.maxPos; delta > 0; --delta) {
            --(dm->currentPos.blockPos);
            dm->currentPos.block = dm->currentPos.block->prev;
        }
        dm->scrollBars.vertical.pos = dm->scrollBars.vertical.maxPos;
    }
}

void UpdateDisplayedModel(HWND hwnd, DisplayedModel* dm, LPARAM lParam) {
    printf("UpdateDisplayedModel\n");
    assert(dm);

    size_t chars = LOWORD(lParam) / dm->charMetric.x;
    size_t lines = HIWORD(lParam) / dm->charMetric.y;
    int isCharsChanged = 0;
    int isLinesChanged = 0;

    if (dm->clientArea.chars != chars) {
        dm->clientArea.chars = chars;
        isCharsChanged = 1;

        dm->wrapModel.isValid = 0;
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
                dm->scrollBars.horizontal.pos = dm->scrollBars.horizontal.maxPos;
            }
        }

        // Vertical scroll-bar
        if (isLinesChanged) { UpdateVerticalSB(dm); }
        break;
    // FORMAT_MODE_DEFAULT

    case FORMAT_MODE_WRAP: /*
        // Horizontal scroll-bar
        SetRange(hwnd, 0, dm->clientArea.chars, SB_HORZ);
        SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

        if (!dm->wrapModel.isValid) {
            dm->shift.y = BuildWrapModel(dm);
            dm->currentPos.charPos = 0;
        }

        // Vertical scroll-bar
        if (SetRange(hwnd, dm->wrapModel.lines, dm->clientArea.lines, SB_VERT)) {
            maxShift = dm->wrapModel.lines - dm->clientArea.lines;
            if (dm->shift.y > maxShift) {
                Block* block = dm->currentPos.block;

                for (int delta = maxShift - dm->shift.y; delta > 0;) {
                    if (dm->currentPos.charPos + 1 <= delta) {
                        delta -= dm->currentPos.charPos + 1;
                    } else {
                        dm->currentPos.charPos -= delta;
                        break;
                    }

                    // prev block
                    dm->currentPos.block = dm->currentPos.block->prev;
                    --(dm->currentPos.blockPos);
                    if (dm->currentPos.block->data.len > 0) {
                        dm->currentPos.charPos = DIV_WITH_ROUND_UP(dm->currentPos.block->data.len, dm->clientArea.chars) - 1;
                    } else {
                        dm->currentPos.charPos = 0;
                    }
                }

                dm->shift.y = maxShift;
            }
            scrollPos = GetPos(dm->shift.y, maxShift);
        } else {
            dm->currentPos.charPos = 0;
            dm->currentPos.blockPos = 0;
            dm->currentPos.block = dm->doc->blocks->nodes;

            dm->shift.y = 0;
            scrollPos = 0;
        }
        SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
        */
        break;
    // FORMAT_MODE_WRAP
    
    default:
        return;
    }
    SetRelativeParam(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
    SetRelativeParam(hwnd, &(dm->scrollBars.vertical), SB_VERT);
}

void CoverDocument(HWND hwnd, DisplayedModel* dm, Document* doc) {
    printf("Cover document\n");
    assert(dm && doc);

    dm->doc = doc;
    dm->documentArea.lines = doc->blocks->len;
    dm->documentArea.chars = GetMaxBlockLen(doc->blocks);

    dm->wrapModel.isValid = 0;

    // can be optimized
    InitScrollBar(&(dm->scrollBars.horizontal));
    InitScrollBar(&(dm->scrollBars.vertical));

    dm->currentPos.block = doc->blocks->nodes;
    dm->currentPos.blockPos = 0;
    dm->currentPos.charPos = 0;

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(dm->documentArea.chars, dm->clientArea.chars);
        dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->documentArea.lines, dm->clientArea.lines);
        break;

    case FORMAT_MODE_WRAP:
        BuildWrapModel(dm);
        dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->wrapModel.lines, dm->clientArea.lines);
        break;

    default:
        // TODO: ERROR
        return;
    }

    SetRelativeParam(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
    SetRelativeParam(hwnd, &(dm->scrollBars.vertical), SB_VERT);

    #ifndef DEBUG
        printf("\tHorizontal\n");
        PrintScrollBar(&(dm->scrollBars.horizontal));
        printf("\tVertical\n");
        PrintScrollBar(&(dm->scrollBars.vertical));
        putchar('\n');
    #endif
}

static void PrintPos(DisplayedModel* dm) {
    assert(dm);

    // printf("x = %i, ", dm->shift.x);
    // printf("y = %i, ", dm->shift.y);
    printf("blockPos = %i, ", dm->currentPos.blockPos);
    printf("charPos = %i\n", dm->currentPos.charPos);
}

void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode) {
    assert(dm);
    size_t chars = 0;
    int scrollPos = 0;

    // TODO: update remaining
    dm->scrollBars.horizontal.pos = 0;
    dm->currentPos.charPos = 0;

    switch(mode) {
    case FORMAT_MODE_DEFAULT:
        printf("Default mode is activated\n");
        dm->mode = FORMAT_MODE_DEFAULT;

        dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(dm->documentArea.chars, dm->clientArea.chars);

        dm->scrollBars.vertical.pos = dm->currentPos.blockPos;
        UpdateVerticalSB(dm);

        break;

    case FORMAT_MODE_WRAP: /*
        printf("Wrap mode is activated\n");
        dm->mode = FORMAT_MODE_WRAP;

        dm->shift.y = BuildWrapModel(dm);

        // Horizontal scroll-bar
        chars = 0;

        // Vertical scroll-bar
        if (SetRange(hwnd, dm->wrapModel.lines, dm->clientArea.lines, SB_VERT)) {
            size_t maxShift = dm->wrapModel.lines - dm->clientArea.lines;
            if (dm->shift.y > maxShift) {
                Block* block = dm->currentPos.block;

                for (int delta = dm->shift.y - maxShift; delta > 0;) {
                    if (dm->currentPos.charPos + 1 <= delta) {
                        delta -= dm->currentPos.charPos + 1;
                    } else {
                        dm->currentPos.charPos -= delta;
                        break;
                    }

                    // prev block
                    dm->currentPos.block = dm->currentPos.block->prev;
                    --(dm->currentPos.blockPos);
                    if (dm->currentPos.block->data.len > 0) {
                        dm->currentPos.charPos = DIV_WITH_ROUND_UP(dm->currentPos.block->data.len, dm->clientArea.chars) - 1;
                    } else {
                        dm->currentPos.charPos = 0;
                    }
                }

                dm->shift.y = maxShift;
            }
            scrollPos = GetPos(dm->shift.y, maxShift);
        } else {
            dm->currentPos.blockPos = 0;
            dm->currentPos.block = dm->doc->blocks->nodes;

            dm->shift.y = 0;
            scrollPos = 0;
        }
        SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
        */
        break;
    default:
        printf("Unknown mode\n");
        return;
    }

    SetRelativeParam(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
    SetRelativeParam(hwnd, &(dm->scrollBars.vertical), SB_VERT);

#ifndef DEBUG // ====/
    PrintPos(dm);
#endif // ===========/
}

void DisplayModel(HDC hdc, const DisplayedModel* dm) {
    assert(dm && dm->doc && dm->doc->text);
    Block* block = dm->currentPos.block;
    size_t displayedLines, displayedChars;
    size_t linesBlock, nextLine;

    #ifndef DEBUG
        printf("Display model:\n");
        printf("\tHorizontal\n");
        PrintScrollBar(&(dm->scrollBars.horizontal));
        printf("\tVertical\n");
        PrintScrollBar(&(dm->scrollBars.vertical));
        putchar('\n');
    #endif

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

    case FORMAT_MODE_WRAP: /*
        nextLine = dm->currentPos.charPos;
        displayedLines = min(dm->clientArea.lines, dm->wrapModel.lines);

        block = dm->currentPos.block;

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
        */
        break;
    default:
        return;
    }
}

void Scroll(HWND hwnd, DisplayedModel* dm, size_t count, Direction direction) {
    int scrollPos;
    size_t delta;

    switch (direction) {
    case UP:
        switch (dm->mode) {
        case FORMAT_MODE_DEFAULT:
            count = min(count, dm->scrollBars.vertical.pos);
            if (count == 0) { return; }

            dm->scrollBars.vertical.pos -= count;

            // for remaining
            while (dm->currentPos.blockPos != dm->scrollBars.vertical.pos) {
                dm->currentPos.block = dm->currentPos.block->prev;
                --(dm->currentPos.blockPos);
            }

            SetRelativePos(hwnd, &(dm->scrollBars.vertical), SB_VERT);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case FORMAT_MODE_WRAP: /*
            if (dm->wrapModel.lines > dm->clientArea.lines) {
                count = min(count, dm->shift.y);
                if (count == 0) { break; }

                dm->shift.y -= count;

                // for remaining
                for (; count > 0;) {
                    if (dm->currentPos.charPos + 1 <= count) {
                        count -= dm->currentPos.charPos + 1;
                    } else {
                        dm->currentPos.charPos -= count;
                        break;
                    }

                    // prev block
                    dm->currentPos.block = dm->currentPos.block->prev;
                    --(dm->currentPos.blockPos);
                    if (dm->currentPos.block->data.len > 0) {
                        dm->currentPos.charPos = DIV_WITH_ROUND_UP(dm->currentPos.block->data.len, dm->clientArea.chars) - 1;
                    } else {
                        dm->currentPos.charPos = 0;
                    }
                }

                #ifndef DEBUG // ==============================================/
                    PrintPos(dm);
                #endif // =====================================================/

                scrollPos = GetPos(dm->shift.y, dm->wrapModel.lines - dm->clientArea.lines);

                if (scrollPos != GetScrollPos(hwnd, SB_VERT)) {
                    SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            */
            break;
        }
        break;
    // UP
    
    case DOWN:
        switch (dm->mode) {
        case FORMAT_MODE_DEFAULT:
            count = min(count, dm->scrollBars.vertical.maxPos - dm->scrollBars.vertical.pos);
            if (count == 0) { return; }

            dm->scrollBars.vertical.pos += count;
            
            // for remaining
            while (dm->currentPos.blockPos != dm->scrollBars.vertical.pos) {
                dm->currentPos.block = dm->currentPos.block->next;
                ++(dm->currentPos.blockPos);
            }

            SetRelativePos(hwnd, &(dm->scrollBars.vertical), SB_VERT);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case FORMAT_MODE_WRAP: /*
            if (dm->wrapModel.lines > dm->clientArea.lines) {
                Block* block = dm->currentPos.block;
                size_t linesBlock = 0;

                delta = dm->wrapModel.lines - dm->clientArea.lines;
                count = min(count, delta - dm->shift.y);
                if (count == 0) { break; }

                dm->shift.y += count;

                // for remaining
                if (block->data.len > 0) {
                    linesBlock = DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
                } else {
                    linesBlock = 1;
                }
                delta = linesBlock - dm->currentPos.charPos;
                dm->currentPos.charPos = 0;
                for (; count > 0; linesBlock = 0) {
                    if (count >= delta) {
                        count -= delta;
                    } else {
                        dm->currentPos.charPos = linesBlock - (delta - count);
                        break;
                    }

                    block = block->next;

                    // next block
                    dm->currentPos.block = block;
                    ++(dm->currentPos.blockPos);

                    if (block->data.len > 0) {
                        linesBlock = DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
                    } else {
                        linesBlock = 1;
                    }
                    delta = linesBlock;
                }

                #ifndef DEBUG // ==============================================/
                    PrintPos(dm);
                #endif // =====================================================/

                scrollPos = GetPos(dm->shift.y, dm->wrapModel.lines - dm->clientArea.lines);

                if (scrollPos != GetScrollPos(hwnd, SB_VERT)) {
                    SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            */
            break;
        }
        break;
    // DOWN

    case LEFT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        count = min(count, dm->scrollBars.horizontal.pos);
        if (count == 0) { return; }

        dm->scrollBars.horizontal.pos -= count;
        SetRelativePos(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    // LEFT

    case RIGHT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        count = min(count, dm->scrollBars.horizontal.maxPos - dm->scrollBars.horizontal.pos);
        if (count == 0) { return; }

        dm->scrollBars.horizontal.pos += count;
        SetRelativePos(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    // RIGHT

    default:
    // TODO: error
        break;
    }

    #ifndef DEBUG
        PrintScrollBar(&(dm->scrollBars.horizontal));
    #endif
}
