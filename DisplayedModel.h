#pragma once
#ifndef DISPLAYED_MODEL_H_INCLUDED
#define DISPLAYED_MODEL_H_INCLUDED

#define CARET_ON
// #define CARET_OFF

#include <windows.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include "Error.h"
#include "Document.h"
#include "ScrollBar.h"

#ifdef CARET_ON
    #include "Caret.h"
#endif

#define DECREMENT_OF(elem) (elem - 1)

typedef enum {
    FORMAT_MODE_DEFAULT,
    FORMAT_MODE_WRAP
} FormatMode;

typedef enum {
         UP,
    LEFT,   RIGHT,
        DOWN
} Direction;

typedef struct {
    size_t x;
    size_t y;
} metric_t;

typedef struct {
    size_t x;
    size_t y;
} position_t;

typedef struct {
    size_t lines;
    size_t chars;
} area_t;

typedef struct {
    int isValid;    // TODO: delete
    size_t lines;
} WrapModel;

typedef struct {
    Block* block;       // pointer to current block (paragraph)
    position_t pos;     // current position
} ModelPos;

typedef struct {
    metric_t charMetric;    // char metric

    FormatMode mode;        // format mode
    Document* doc;          // pointer to Document object

    area_t clientArea;      // dimensions of client area
    area_t documentArea;    // dimensions of document area
    WrapModel wrapModel;    // lines count for wrap model

    struct {
        ScrollBar horizontal;   // horizontal scroll-bar
        ScrollBar vertical;     // vertical scroll-bar
        ModelPos modelPos;      // position relative to a model
    } scrollBars;           // scroll-bars

    #ifdef CARET_ON
        struct {
            struct {
                int x;      // horizontal direction
                int y;      // vertical direction
            } isHidden; // flags for display a caret

            position_t clientPos;   // position relative to client area
            ModelPos modelPos;      // position relative to a model
            size_t linePos;         // line position (for wrap model)
        } caret;    // caret
    #endif
} DisplayedModel;

/**
 * Inits a DisplayModel object.
 * IN:
 * @param dm - pointer to a DisplayModel object
 * @param tm - pointer to text metrics (gets char metric)
 * 
 * OUT:
 * fills fields of a DisplayModel object
 */
void InitDisplayedModel(DisplayedModel* dm, const TEXTMETRIC* tm);

/**
 * Updates DisplayedModel object after resizing window.
 * IN:
 * @param hwnd - a handle to a window
 * @param dm - pointer to a DisplayModel object
 * @param lParam - 
 * 
 * OUT:
 * updated some params of DisplayedModel object (scroll-bars, caret and etc)
 */
void UpdateDisplayedModel(HWND hwnd, DisplayedModel* dm, LPARAM lParam);

/**
 * Covers Document object.
 * IN:
 * @param hwnd - a handle to a window
 * @param dm - pointer to a DisplayModel object
 * @param doc - pointer to a Document object
 * 
 * OUT:
 * builds document and wrap models
 */
void CoverDocument(HWND hwnd, DisplayedModel* dm, Document* doc);

/**
 * Switchs format mode.
 * IN:
 * @param hwnd - a handle to a window
 * @param dm - pointer to a DisplayModel object
 * @param mode - format mode to be turned on
 */
void SwitchMode(HWND hwnd, DisplayedModel* dm, FormatMode mode);

/**
 * Displays the text on client area (screen).
 * IN:
 * @param hdc - a handle to a device context
 * @param dm - pointer to a DisplayModel object
 */
void DisplayModel(HDC hdc, const DisplayedModel* dm);


// Scroll-bar
/**
 * Scrolls the window to a value in a direction.
 * IN:
 * @param hwnd - a handle to a window
 * @param dm - pointer to a DisplayModel object
 * @param scrollValue - scroll value
 * @param dir - scroll direction (UP, DOWN, LEFT, RIGHT)
 * @param rectangle - pointer to rectangle (will be invalidate)
 * 
 * OUT:
 * current scroll value
 */
size_t Scroll(HWND hwnd, DisplayedModel* dm, size_t scrollValue, Direction dir, RECT* rectangle);


// Caret
#ifdef CARET_ON
    /**
     * Prints caret params.
     * IN:
     * @param dm - pointer to a DisplayModel object
     */
    void CaretPrintParams(DisplayedModel* dm);


    /**
     * Finds home position (FORMAT_MODE_DEFAULT).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void FindHome_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    
    /**
     * Finds home position (FORMAT_MODE_WRAP).
     * IN:
     * @param dm - pointer to a DisplayModel object
     */
    void FindHome_Wrap(DisplayedModel* dm);


    /**
     * Finds end position (FORMAT_MODE_DEFAULT) to the left.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void FindLeftEnd_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    /**
     * Finds home position (FORMAT_MODE_DEFAULT) to the right.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void FindRightEnd_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    
    /**
     * Finds home position (FORMAT_MODE_WRAP) to the right.
     * IN:
     * @param dm - pointer to a DisplayModel object
     */
    void FindRightEnd_Wrap(DisplayedModel* dm);


    /**
     * Moves the caret to the top (FORMAT_MODE_DEFAULT).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void CaretMoveToTop_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    /**
     * Moves the caret to the top (FORMAT_MODE_WRAP). (FORMAT_MODE_WRAP).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void CaretMoveToTop_Wrap(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    

    /**
     * Moves the caret to the bottom (FORMAT_MODE_DEFAULT).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void CaretMoveToBottom_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    
    /**
     * Moves the caret to the bottom (FORMAT_MODE_WRAP).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void CaretMoveToBottom_Wrap(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    

    /**
     * Moves the caret to the left (FORMAT_MODE_WRAP).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void CaretMoveToLeft_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    
    /**
     * Moves the caret to the left (FORMAT_MODE_WRAP).
     * IN:
     * @param dm - pointer to a DisplayModel object
     */
    void CaretMoveToLeft_Wrap(DisplayedModel* dm);


    /**
     * Moves the caret to the right (FORMAT_MODE_DEFAULT).
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void CaretMoveToRight_Default(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    /**
     * Moves the caret to the right (FORMAT_MODE_WRAP).
     * IN:
     * @param dm - pointer to a DisplayModel object
     */
    void CaretMoveToRight_Wrap(DisplayedModel* dm);


    // TODO: update
    void CaretPageUp(HWND hwnd, DisplayedModel* dm, RECT* rectangle);
    void CaretPageDown(HWND hwnd, DisplayedModel* dm, RECT* rectangle);


    /**
     * Finds the caret outside the client area.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param rectangle - pointer to rectangle (will be invalidate)
     */
    void FindCaret(HWND hwnd, DisplayedModel* dm, RECT* rectangle);

    /**
     * Sets carets on the position on display.
     * IN:
     * @param dm - pointer to a DisplayModel object
     */
    void CaretSetPos(DisplayedModel* dm);

    /**
     * Creats caret on display.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     */
    void CaretCreate(HWND hwnd, DisplayedModel* dm);

    void CaretHandleTopLeftBorder(HWND hwnd, int* p_isHidden, size_t scrollValue, size_t modelPos, size_t scrollBarPos, size_t* pClientPos, size_t clientPosMax);
    void CaretHandleBottomRightBorder(HWND hwnd, int* p_isHidden, size_t scrollValue, size_t modelPos, size_t scrollBarPos, size_t* pClientPos, size_t clientPosMax);


    // editing
    /**
     * Adds char to the text.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * @param c - char that should be added
     * 
     * OUT:
     * @return errValue - value indicating the success of the operation
     */
    int CaretAddChar(HWND hwnd, DisplayedModel* dm, char c);

    /**
     * Adds block (paragraph) to the text.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * 
     * OUT:
     * @return errValue - value indicating the success of the operation
     */
    int CaretAddBlock(HWND hwnd, DisplayedModel* dm);

    /**
     * Deletes char from the text.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     * 
     * OUT:
     * @return errValue - value indicating the success of the operation
     */
    int CaretDeleteChar(HWND hwnd, DisplayedModel* dm);

    /**
     * Deletes block (paragraph) from the text.
     * IN:
     * @param hwnd - a handle to a window
     * @param dm - pointer to a DisplayModel object
     */
    void CaretDeleteBlock(HWND hwnd, DisplayedModel* dm);
#endif

#endif // DISPLAYED_MODEL_H_INCLUDED
