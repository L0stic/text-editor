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

    dm->shift.x = 0;
    dm->shift.y = 0;

    dm->mode = FORMAT_MODE_DEFAULT;
    dm->doc = NULL;

    dm->currentPos.block = NULL;
    dm->currentPos.blockPos = 0;
    dm->currentPos.charPos = 0;
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

static size_t BuildWrapModel(DisplayedModel* dm) {
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

void UpdateDisplayedModel(HWND hwnd, DisplayedModel* dm, LPARAM lParam) {
    assert(dm);
    size_t chars = LOWORD(lParam) / dm->charMetric.x;
    size_t lines = HIWORD(lParam) / dm->charMetric.y;
    int isCharsChanged = 0;
    int isLinesChanged = 0;
    size_t maxShift;
    int scrollPos;

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
            if (SetRange(hwnd, dm->documentArea.chars, dm->clientArea.chars, SB_HORZ)) {
                maxShift = dm->documentArea.chars - dm->clientArea.chars;
                if (dm->shift.x > maxShift) {
                    dm->shift.x = maxShift;
                }
                scrollPos = GetPos(dm->shift.x, maxShift);
            } else {
                dm->shift.x = 0;
                scrollPos = 0;
            }
            SetScrollPos(hwnd, SB_HORZ, scrollPos, TRUE);
        } else {
            SetRange(hwnd, dm->documentArea.chars, dm->clientArea.chars, SB_HORZ);
            SetScrollPos(hwnd, SB_HORZ, GetScrollPos(hwnd, SB_HORZ), TRUE);
        }

        // Vertical scroll-bar
        if (isLinesChanged) {
            if (SetRange(hwnd, dm->documentArea.lines, dm->clientArea.lines, SB_VERT)) {
                maxShift = dm->documentArea.lines - dm->clientArea.lines;
                if (dm->shift.y > maxShift) {
                    // pass
                    for(int delta = maxShift - dm->shift.y; delta > 0; --delta) {
                        --(dm->currentPos.blockPos);
                        dm->currentPos.block = dm->currentPos.block->prev;
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
        } else {
            SetRange(hwnd, dm->documentArea.lines, dm->clientArea.lines, SB_VERT);
            SetScrollPos(hwnd, SB_HORZ, GetScrollPos(hwnd, SB_VERT), TRUE);
        }
        break;

    case FORMAT_MODE_WRAP:
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
        break;

    default:
        return;
    }
}

void CoverDocument(HWND hwnd, DisplayedModel* dm, Document* doc) {
    assert(dm && doc);
    size_t linesModel = 0;

    dm->doc = doc;
    dm->documentArea.lines = doc->blocks->len;
    dm->documentArea.chars = GetMaxBlockLen(doc->blocks);

    dm->wrapModel.isValid = 0;

    dm->shift.x = 0;
    dm->shift.y = 0;

    dm->currentPos.block = doc->blocks->nodes;
    dm->currentPos.blockPos = 0;
    dm->currentPos.charPos = 0;

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        // Horizontal scroll-bar
        SetRange(hwnd, dm->documentArea.chars, dm->clientArea.chars, SB_HORZ);
        SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

        // Vertical scroll-bar
        linesModel = dm->documentArea.lines;
        break;

    case FORMAT_MODE_WRAP:
        BuildWrapModel(dm);

        // Vertical scroll-bar
        linesModel = dm->wrapModel.lines;
        break;

    default:
        return;
    }

    // Vertical scroll-bar (common)
    SetRange(hwnd, linesModel, dm->clientArea.lines, SB_VERT);
    SetScrollPos(hwnd, SB_VERT, 0, TRUE);
}

void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode) {
    assert(dm);
    size_t chars = 0;
    int scrollPos = 0;

    dm->shift.x = 0;
    dm->currentPos.charPos = 0;

    switch(mode) {
    case FORMAT_MODE_DEFAULT:
        printf("Default mode is activated\n");
        dm->mode = FORMAT_MODE_DEFAULT;

        // Horizontal scroll-bar
        chars = dm->documentArea.chars;

        // Vertical scroll-bar
        if (SetRange(hwnd, dm->documentArea.lines, dm->clientArea.lines, SB_VERT)) {
            size_t maxShift = dm->documentArea.lines - dm->clientArea.lines;
            dm->shift.y = dm->currentPos.blockPos;
            if (dm->shift.y > maxShift) {
                // pass
                for(int delta = dm->shift.y - maxShift; delta > 0; --delta) {
                    --(dm->currentPos.blockPos);
                    dm->currentPos.block = dm->currentPos.block->prev;
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
        break;

    case FORMAT_MODE_WRAP:
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
        break;
    default:
        printf("Unknown mode\n");
        return;
    }

    // Horizontal scroll-bar (common)
    SetRange(hwnd, chars, dm->clientArea.chars, SB_HORZ);
    SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

    printf("y = %i, ", dm->shift.y);
    printf("blockPos = %i, ", dm->currentPos.blockPos);
    printf("charPos = %i\n", dm->currentPos.charPos);
}

void DisplayModel(HDC hdc, const DisplayedModel* dm) {
    assert(dm && dm->doc && dm->doc->text);
    Block* block = dm->currentPos.block;
    size_t displayedLines, displayedChars;
    size_t linesBlock, nextLine;

    switch (dm->mode) {
    case FORMAT_MODE_DEFAULT:
        displayedLines = min(dm->clientArea.lines, dm->documentArea.lines);

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
        break;

    case FORMAT_MODE_WRAP:
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
        break;
    default:
        return;
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
                while (dm->currentPos.blockPos != dm->shift.y) {
                    dm->currentPos.block = dm->currentPos.block->prev;
                    --(dm->currentPos.blockPos);
                }

                scrollPos = GetPos(dm->shift.y, dm->documentArea.lines - dm->clientArea.lines);

                if (scrollPos != GetScrollPos(hwnd, SB_VERT)) {
                    SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
        } else {
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

                printf("y = %i, ", dm->shift.y);
                printf("blockPos = %i, ", dm->currentPos.blockPos);
                printf("charPos = %i\n", dm->currentPos.charPos);

                scrollPos = GetPos(dm->shift.y, dm->wrapModel.lines - dm->clientArea.lines);

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
                while (dm->currentPos.blockPos != dm->shift.y) {
                    dm->currentPos.block = dm->currentPos.block->next;
                    ++(dm->currentPos.blockPos);
                }

                scrollPos = GetPos(dm->shift.y, dm->documentArea.lines - dm->clientArea.lines);

                if (scrollPos != GetScrollPos(hwnd, SB_VERT)) {
                    SetScrollPos(hwnd, SB_VERT, scrollPos, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
        } else {
            if (dm->wrapModel.lines > dm->clientArea.lines) {
                Block* block = dm->currentPos.block;
                size_t linesBlock = 0;

                delta = dm->wrapModel.lines - dm->clientArea.lines;
                count = min(count, delta - dm->shift.y);
                if (count == 0) { break; }

                dm->shift.y += count;
                printf("y = %i, ", dm->shift.y);

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

                printf("blockPos = %i, ", dm->currentPos.blockPos);
                printf("charPos = %i\n", dm->currentPos.charPos);

                scrollPos = GetPos(dm->shift.y, dm->wrapModel.lines - dm->clientArea.lines);

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
