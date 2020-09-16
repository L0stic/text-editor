#pragma once
#ifndef CARET_H_INCLUDED
#define CARET_H_INCLUDED

#include <windows.h>
#include <assert.h>
#include <stdio.h>

void CaretDestroy(HWND hwnd);

void CaretShow(HWND hwnd, int* p_isHidden);

void CaretHide(HWND hwnd, int* p_isHidden);

#endif // CARET_H_INCLUDED
