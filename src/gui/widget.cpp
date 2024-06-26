#include "gui/widget.h"

using namespace myos::common;
using namespace myos::gui;

Widget::Widget(Widget* parent, int32_t x, int32_t y,
        int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b) : KeyboardEventHandler(), parent(parent), 
        x(x), y(y), w(w), h(h), r(r), g(g), b(b), focussable(true) {}
Widget::~Widget() {}

void Widget::GetFocus(Widget* widget) {
  if (parent != 0) parent->GetFocus(widget);
}

void Widget::ModelToScreen(int32_t &x, int32_t &y) {
  if (parent != 0) parent->ModelToScreen(x, y);
  x += this->x;
  y += this->y;
}

void Widget::Draw(GraphicsContext* gc) {
  int32_t X = 0, Y = 0;
  ModelToScreen(X, Y);
  gc->FillRectangle(X, Y, w, h, r, g, b);
}

bool Widget::ContainsCoordinate(int32_t x, int32_t y) {
  return (this->x <= x && this->y <= y && this->x + this->w > x && this->y + this->h > y);
}

void Widget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
  if (focussable) GetFocus(this);
}
void Widget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {}
void Widget::OnMouseMove(int32_t x, int32_t y, int32_t nx, int32_t ny) {}



CompositeWidget::CompositeWidget(Widget* parent, int32_t x, int32_t y,
        int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b)
        : Widget(parent, x, y, w, h, r, g, b), numChildren(0), focussedChild(0) {}

CompositeWidget::~CompositeWidget() {}

void CompositeWidget::GetFocus(Widget* widget) {
  focussedChild = widget;
  if (parent != 0) parent->GetFocus(this);
}

void CompositeWidget::Draw(GraphicsContext* gc) {
  Widget::Draw(gc);
  for (int32_t i = numChildren - 1; ~i; i--) {
    children[i]->Draw(gc);
  }
}

bool CompositeWidget::AddChild(Widget* child) {
  if (numChildren >= 100) return false;
  children[numChildren++] = child;
  return true;
}


void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
  for (int32_t i = 0; i < numChildren; i++) {
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
      children[i]->OnMouseDown(x - this->x, y - this->y, button);
      break;
    }
  }
}
void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
  for (int32_t i = 0; i < numChildren; i++) {
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
      children[i]->OnMouseUp(x - this->x, y - this->y, button);
      break;
    }
  }
}

void CompositeWidget::OnMouseMove(int32_t x, int32_t y, int32_t nx, int32_t ny) {
  int32_t firstChild = -1;
  for (int32_t i = 0; i < numChildren; i++) {
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
      children[i]->OnMouseMove(x - this->x, y - this->y, nx - this->x, ny - this->y);
      firstChild = i;
      break;
    }
  }

  for (int32_t i = 0; i < numChildren; i++) {
    if (children[i]->ContainsCoordinate(nx - this->x, ny - this->y)) {
      if (firstChild != i) {
        children[i]->OnMouseMove(x - this->x, y - this->y, nx - this->x, ny - this->y);
      }
      break;
    }
  }
}

void CompositeWidget::OnKeyDown(char x) {
  if (focussedChild != 0) {
    focussedChild->OnKeyDown(x);
  }
}
void CompositeWidget::OnKeyUp(char x) {
  if (focussedChild != 0) {
    focussedChild->OnKeyUp(x);
  }
}