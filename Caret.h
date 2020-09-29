#pragma once
#ifndef CARET_H_INCLUDED
#define CARET_H_INCLUDED

#include <windows.h>
#include <assert.h>
#include <stdio.h>

/**
 * Destoys caret.
 * 
 * IN:
 * @param hwnd - a handle to a window
 */
void CaretDestroy(HWND hwnd);

/**
 * Show caret in some direction.
 * 
 * IN:
 * @param hwnd - a handle to a window
 * @param p_isHidden - pointer to flag of direction
 * 
 * OUT:
 * isHidden - becomes false
 */
void CaretShow(HWND hwnd, int* p_isHidden);

/**
 * Hide caret in some direction.
 * 
 * IN:
 * @param hwnd - a handle to a window
 * @param p_isHidden - pointer to flag of direction
 * 
 * OUT:
 * isHidden - becomes true
 */
void CaretHide(HWND hwnd, int* p_isHidden);

#endif // CARET_H_INCLUDED
