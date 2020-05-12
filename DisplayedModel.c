#include "DisplayedModel.h"

void InitDisplayedModel(DisplayedModel* dm, TEXTMETRIC* tm) {
    assert(dm && tm);

    dm->charMetric.x = tm->tmAveCharWidth;
    dm->charMetric.y = tm->tmHeight + tm->tmExternalLeading;

    dm->clientArea.chars = 0;
    dm->clientArea.lines = 0;

    dm->documentArea.chars = 0;
    dm->documentArea.lines = 0;

    dm->shift.x = 0;
    dm->shift.y = 0;

    dm->doc = NULL;

    dm->mode = FORMAT_MODE_DEFAULT;

    dm->wrapModel.isValid = 0;
    dm->wrapModel.lines = 0;
}

static int GetMaxPos(size_t requiredMax) {
    return requiredMax <= MAX_POS ? (int)requiredMax : MAX_POS;
}

static int GetPos(size_t currentPos, size_t requiredMax) {
    if (requiredMax > MAX_POS) {
        long double part = (long double) MAX_POS / requiredMax;
        return (int) roundl(part * currentPos);
    }
    return (int)currentPos;
}

size_t GetCurrentPos(int pos, size_t requiredMax) {
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

static size_t BuildWrapModel(Document* doc, size_t maxLen) {
    assert(doc);

    size_t linesModel = 0;

    for (Block* block = doc->blocks->nodes; block; block = block->next) {
        if (block->data.len > 0) {
            linesModel += DIV_WITH_ROUND_UP(block->data.len, maxLen);
        } else {
            ++linesModel;
        }
    }

    return linesModel;
}

void UpdateDisplayedModel(HWND hwnd, DisplayedModel* dm, LPARAM lParam) {
    assert(dm);
    dm->wrapModel.isValid = 0;

    dm->clientArea.chars = LOWORD(lParam) / dm->charMetric.x;
    dm->clientArea.lines = HIWORD(lParam) / dm->charMetric.y;

    // redo for remainig block
    dm->shift.x = 0;
    dm->shift.y = 0;

    // TODO: check handle owerflow of ScrollRange
    if (dm->mode == FORMAT_MODE_DEFAULT) {
        SetRange(hwnd, dm->documentArea.chars, dm->clientArea.chars, SB_HORZ);
        SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

        SetRange(hwnd, dm->documentArea.lines, dm->clientArea.lines, SB_VERT);
        SetScrollPos(hwnd, SB_VERT, 0, TRUE);
    } else { // mode == FORMAT_MODE_WRAP
        size_t linesModel = BuildWrapModel(dm->doc, dm->clientArea.chars);

        dm->wrapModel.isValid = 1;
        if (linesModel != dm->wrapModel.lines) {
            dm->wrapModel.lines = linesModel;
            SetRange(hwnd, dm->wrapModel.lines, dm->clientArea.lines, SB_VERT);
            SetScrollPos(hwnd, SB_VERT, 0, TRUE);
        }
    }
}

void CoverDocument(HWND hwnd, DisplayedModel* dm, Document* doc) {
    assert(dm && doc);

    dm->doc = doc;
    dm->documentArea.lines = doc->blocks->len;
    dm->documentArea.chars = GetMaxBlockLen(doc->blocks);

    dm->delta = 0;
    dm->firstBlock.pos = 0;
    dm->firstBlock.start = doc->blocks->nodes;

    // redo for remainig block
    dm->shift.x = 0;
    dm->shift.y = 0;

    dm->wrapModel.isValid = 0;

    if (dm->mode == FORMAT_MODE_DEFAULT) {
        SetRange(hwnd, dm->documentArea.chars, dm->clientArea.chars, SB_HORZ);
        SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

        SetRange(hwnd, dm->documentArea.lines, dm->clientArea.lines, SB_VERT);
        SetScrollPos(hwnd, SB_VERT, 0, TRUE);
    } else { // mode == FORMAT_MODE_WRAP
        size_t linesModel = BuildWrapModel(dm->doc, dm->clientArea.chars);
        
        dm->wrapModel.isValid = 1;
        if (linesModel != dm->wrapModel.lines) {
            dm->wrapModel.lines = linesModel;
            SetRange(hwnd, dm->wrapModel.lines, dm->clientArea.lines, SB_VERT);
            SetScrollPos(hwnd, SB_VERT, 0, TRUE);
        }
    }
}

void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode) {
    assert(dm);

    // redo for remainig block
    dm->shift.x = 0;
    dm->shift.y = 0;

    switch(mode) {
    case FORMAT_MODE_DEFAULT:
        printf("Default mode is activated\n");
        dm->mode = FORMAT_MODE_DEFAULT;

        SetRange(hwnd, dm->documentArea.chars, dm->clientArea.chars, SB_HORZ);
        SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

        if (dm->documentArea.lines != dm->wrapModel.lines) {
            SetRange(hwnd, dm->documentArea.lines, dm->clientArea.lines, SB_VERT);
            SetScrollPos(hwnd, SB_VERT, 0, TRUE);
        }
        break;

    case FORMAT_MODE_WRAP:
        printf("Wrap mode is activated\n");
        dm->mode = FORMAT_MODE_WRAP;

        SetScrollRange(hwnd, SB_HORZ, 0, 0, FALSE);
        SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

        if (!dm->wrapModel.isValid) {
            size_t linesModel = BuildWrapModel(dm->doc, dm->clientArea.chars);
            dm->wrapModel.isValid = 1;
            dm->wrapModel.lines = linesModel;
        }
        
        // delta = 0;
        // linesDoc = 0;
        // for (Block* block = doc->blocks->nodes; block != firstBlock; block = block->next) {
        //     if (block->data.len > 0) {
        //         linesDoc += DIV_WITH_ROUND_UP(block->data.len, charsClient);
        //     } else {
        //         ++linesDoc;
        //     }
        // }
        // iVscrollPos = linesDoc;
        // for (Block* block = firstBlock; block; block = block->next) {
        //     if (block->data.len > 0) {
        //         linesDoc += DIV_WITH_ROUND_UP(block->data.len, charsClient);
        //     } else {
        //         ++linesDoc;
        //     }
        // }

        if (dm->wrapModel.lines != dm->documentArea.lines) {
            SetRange(hwnd, dm->wrapModel.lines, dm->clientArea.lines, SB_VERT);
            SetScrollPos(hwnd, SB_VERT, 0, TRUE);
        }

        break;
    default:
        break;
    }
}

void DisplayModel(HDC hdc, DisplayedModel* dm) {
    if (dm->doc && dm->doc->text) {
        Block* block;
        size_t displayedLines;

        // redo:
        size_t iVscrollPos = 0;

        if (dm->mode == FORMAT_MODE_DEFAULT) {
            size_t displayedChars;

            block = dm->firstBlock.start;
            displayedLines = min(dm->clientArea.lines, dm->documentArea.lines - dm->shift.y);

            // print
            for (size_t i = 0; i < displayedLines; ++i) {
                if (dm->shift.x < block->data.len) {
                    displayedChars = min(dm->clientArea.chars, block->data.len - dm->shift.x);
                    TextOut(hdc,
                            0,
                            i * dm->charMetric.y,
                            dm->doc->text->data + block->data.pos + dm->shift.x,
                            displayedChars);
                }

                block = block->next;
            }
        } else { // mode == WRAP
            size_t linesBlock;
            size_t nextLine = 0;
            displayedLines = min(dm->clientArea.lines, dm->wrapModel.lines - dm->shift.y);

            block = dm->doc->blocks->nodes;
            // pass
            if (iVscrollPos > 0) {
                for (size_t i = 0; block;) {
                    if (block->data.len > 0) {
                        linesBlock = DIV_WITH_ROUND_UP(block->data.len, dm->clientArea.chars);
                    } else {
                        linesBlock = 1;
                    }

                    i += linesBlock;

                    if (i > iVscrollPos) {
                        nextLine = linesBlock - (i - iVscrollPos);
                        break;
                    }

                    block = block->next;
                }
            }


            //firstBlock = block;

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
                    for (; (displayedLines - i) > 0; ++i, ++nextLine) {
                        TextOut(hdc,
                                0,
                                i * dm->charMetric.y,
                                dm->doc->text->data + block->data.pos + nextLine * dm->clientArea.chars,
                                dm->clientArea.chars);
                    }
                }

                block = block->next;
            }
        }
    }
}

void Scroll(HWND hwnd, DisplayedModel* dm, size_t count, Direction direction) {
    int scrollPos;
    size_t delta;

    switch (direction)
    {
    case UP:
        if (dm->mode == FORMAT_MODE_DEFAULT) {
            if (dm->documentArea.lines > dm->clientArea.lines) {
                count = min(count, dm->shift.y);
                if (count == 0) { break; }

                dm->shift.y -= count;
                // for remaining
                while (dm->firstBlock.pos != dm->shift.y) {
                    dm->firstBlock.start = dm->firstBlock.start->prev;
                    --(dm->firstBlock.pos);
                }

                scrollPos = GetPos(dm->shift.y, dm->documentArea.lines - dm->clientArea.lines);

                if (scrollPos != GetScrollPos(hwnd, SB_VERT)) {
                    SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        break;

    case DOWN:
        if (dm->mode == FORMAT_MODE_DEFAULT) {
            if (dm->documentArea.lines > dm->clientArea.lines) {
                delta = dm->documentArea.lines - dm->clientArea.lines;
                count = min(count, delta - dm->shift.y);

                if (count == 0) { break; }

                dm->shift.y += count;
                // for remaining
                while (dm->firstBlock.pos != dm->shift.y) {
                    dm->firstBlock.start = dm->firstBlock.start->next;
                    ++(dm->firstBlock.pos);
                }

                scrollPos = GetPos(dm->shift.y, dm->documentArea.lines - dm->clientArea.lines);

                if (scrollPos != GetScrollPos(hwnd, SB_VERT)) {
                    SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        break;

    case LEFT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        if (dm->documentArea.chars > dm->clientArea.chars) {
            count = min(count, dm->shift.x);
            if (count == 0) { break; }

            dm->shift.x -= count;

            scrollPos = GetPos(dm->shift.x, dm->documentArea.chars - dm->clientArea.chars);

            if (scrollPos != GetScrollPos(hwnd, SB_HORZ)) {
                SetScrollPos(hwnd, SB_HORZ, scrollPos, TRUE);
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    
    case RIGHT:
        assert(dm->mode == FORMAT_MODE_DEFAULT);

        if (dm->documentArea.chars > dm->clientArea.chars) {
            delta = dm->documentArea.chars - dm->clientArea.chars;
            count = min(count, delta - dm->shift.x);
            if (count == 0) { break; }

            dm->shift.x += count;

            scrollPos = GetPos(dm->shift.x, dm->documentArea.chars - dm->clientArea.chars);

            if (scrollPos != GetScrollPos(hwnd, SB_HORZ)) {
                SetScrollPos(hwnd, SB_HORZ, scrollPos, TRUE);
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    
    default:
        break;
    }
}
