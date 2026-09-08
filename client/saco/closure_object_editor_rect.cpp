// R5 editor rectangle positioning72040/63 and reset72080/20.
#include "main.h"
class R5ObjectEditorRectView {
 int width; int height;
public: void CenterRect(RECT *rect,POINT *point); void ResetRect(RECT *rect);
};
void R5ObjectEditorRectView::CenterRect(RECT *rect,POINT *point) {
 rect->left=point->x-width/2;
 rect->right=rect->left+width;
 rect->top=point->y-height/2;
 rect->bottom=rect->top+height;
}
void R5ObjectEditorRectView::ResetRect(RECT *rect) {
 rect->left=0;
 rect->right=0;
 rect->top=0;
 rect->bottom=0;
}
