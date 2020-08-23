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

    dm->caret.currentPos.block = NULL;
    dm->caret.currentPos.modelPos.x = 0;
    dm->caret.currentPos.modelPos.y = 0;
    dm->caret.clientPos.x = 0;
    dm->caret.clientPos.y = 0;

    dm->mode = FORMAT_MODE_DEFAULT;
    dm->doc = NULL;

    dm->currentPos.block = NULL;
    dm->currentPos.blockPos = 0;
    dm->currentPos.charPos = 0;
}


static void PrintWrapModel(WrapModel* wrapModel) {
    if (wrapModel->isValid) {
        printf("Wrap model: %i\n", wrapModel->lines);
    } else {
        printf("Wrap model is not valid\n");
    }
}

static size_t BuildWrapModel(DisplayedModel* dm) {
    printf("BuildWrapModel\n");
    assert(dm);
    size_t relativePos;
    Block* block = dm->doc->blocks->nodes;

    dm->currentPos.charPos = 0;
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
    assert(dm);
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


static void UpdateVerticalSB_Wrap(DisplayedModel* dm) {
    assert(dm);
    dm->scrollBars.vertical.maxPos = GetAbsoluteMaxPos(dm->wrapModel.lines, dm->clientArea.lines);

    if (dm->scrollBars.vertical.pos > dm->scrollBars.vertical.maxPos) {
        // pass
        for (size_t delta = dm->scrollBars.vertical.pos - dm->scrollBars.vertical.maxPos; delta > 0;) {
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

    dm->caret.currentPos.block = doc->blocks->nodes;
    dm->caret.currentPos.modelPos.x = 0;
    dm->caret.currentPos.modelPos.y = 0;

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
        printf("\tHorizontal\n");
        PrintScrollBar(&(dm->scrollBars.horizontal));
        printf("\tVertical\n");
        PrintScrollBar(&(dm->scrollBars.vertical));
        putchar('\n');
    #endif // =======================================/
}


static void PrintPos(DisplayedModel* dm) {
    assert(dm);
    printf("blockPos = %i, ", dm->currentPos.blockPos);
    printf("charPos = %i\n", dm->currentPos.charPos);
}


void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode) {
    assert(dm);
    printf("Switch mode\n");

    dm->scrollBars.horizontal.pos = 0;
    #ifndef DEBUG // ====/
        PrintPos(dm);
    #endif // ===========/

    switch(mode) {
    case FORMAT_MODE_DEFAULT:
        printf("Default mode is activated\n");
        dm->mode = FORMAT_MODE_DEFAULT;

        dm->scrollBars.horizontal.maxPos = GetAbsoluteMaxPos(dm->documentArea.chars, dm->clientArea.chars);

        dm->scrollBars.vertical.pos = dm->currentPos.blockPos - 1;
        UpdateVerticalSB(dm);

        break;

    case FORMAT_MODE_WRAP:
        printf("Wrap mode is activated\n");
        dm->mode = FORMAT_MODE_WRAP
        ;
        dm->currentPos.charPos = 0;
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

    case FORMAT_MODE_WRAP:
        nextLine = dm->currentPos.charPos;
        displayedLines = min(dm->clientArea.lines, dm->wrapModel.lines - dm->scrollBars.vertical.pos);

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
        break;

    default:
        return;
    }
}


void Scroll(HWND hwnd, DisplayedModel* dm, size_t count, Direction direction, RECT* rectangle) {
    assert(dm);
    assert(rectangle);

    // TODO: update window scroll
    // rectangle->left = 0;
    // rectangle->right = dm->charMetric.x * dm->clientArea.chars;

    // rectangle->top = 0;
    // rectangle->bottom = dm->charMetric.y * dm->clientArea.lines;

    rectangle = NULL;

    switch (direction) {
    case UP:
        count = min(count, dm->scrollBars.vertical.pos);
        if (count == 0) { return; }

        dm->scrollBars.vertical.pos -= count;

        switch (dm->mode) {
        case FORMAT_MODE_DEFAULT:
            // for remaining
            while (dm->currentPos.blockPos != dm->scrollBars.vertical.pos) {
                dm->currentPos.block = dm->currentPos.block->prev;
                --(dm->currentPos.blockPos);
            }
            break;

        case FORMAT_MODE_WRAP:
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
            break;

        default:
            break;
        }

        SetRelativePos(hwnd, &(dm->scrollBars.vertical), SB_VERT);

        // TODO: update vertical window scroll
        // ScrollWindow(hwnd, 0, count * dm->charMetric.y, NULL, NULL);
        // rectangle->bottom = dm->charMetric.y * min(count, dm->clientArea.lines);
        break;
    // UP
    
    case DOWN:
        count = min(count, dm->scrollBars.vertical.maxPos - dm->scrollBars.vertical.pos);
        if (count == 0) { return; }

        dm->scrollBars.vertical.pos += count;

        switch (dm->mode) {
        case FORMAT_MODE_DEFAULT:            
            // for remaining
            while (dm->currentPos.blockPos != dm->scrollBars.vertical.pos) {
                dm->currentPos.block = dm->currentPos.block->next;
                ++(dm->currentPos.blockPos);
            }
            break;

        case FORMAT_MODE_WRAP: {
            Block* block = dm->currentPos.block;
            size_t delta;
            size_t linesBlock = 0;

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
        }
            break;
        }

        SetRelativePos(hwnd, &(dm->scrollBars.vertical), SB_VERT);

        // TODO: update vertical window scroll
        // ScrollWindow(hwnd, 0, -count * dm->charMetric.y, NULL, NULL);
        // rectangle->top = dm->charMetric.y * (dm->clientArea.lines - min(count, dm->clientArea.lines));
        break;
    // DOWN

    case LEFT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        count = min(count, dm->scrollBars.horizontal.pos);
        if (count == 0) { return; }

        dm->scrollBars.horizontal.pos -= count;

        SetRelativePos(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);

        // TODO: update horizonatl window scroll
        // ScrollWindow(hwnd, count * dm->charMetric.x, 0, NULL, NULL);
        // rectangle->left = dm->charMetric.x * (dm->clientArea.chars - min(count, dm->clientArea.chars));
        break;
    // LEFT

    case RIGHT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        count = min(count, dm->scrollBars.horizontal.maxPos - dm->scrollBars.horizontal.pos);
        if (count == 0) { return; }

        dm->scrollBars.horizontal.pos += count;
        SetRelativePos(hwnd, &(dm->scrollBars.horizontal), SB_HORZ);

        // TODO: update horizonatl window scroll
        // ScrollWindow(hwnd, -count * dm->charMetric.x, 0, NULL, NULL);
        // rectangle->right = dm->charMetric.x * min(count, dm->clientArea.chars);
        break;
    // RIGHT

    default:
    // TODO: error
        return;
    }

    #ifndef DEBUG // ==============================================/
        PrintPos(dm);
    #endif // =====================================================/

    InvalidateRect(hwnd, rectangle, TRUE);

    #ifndef DEBUG
        PrintScrollBar(&(dm->scrollBars.vertical));
        PrintScrollBar(&(dm->scrollBars.horizontal));
    #endif
}
