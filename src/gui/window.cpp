#include "gui/window.h"

using namespace myos::gui;
using namespace myos::common;


Window::Window(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h,
             uint8_t r, uint8_t g, uint8_t b) 
             : CompositeWidget(parent, x, y, w, h, r, g, b), dragging(false) {}



void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
  dragging = (button == 1);
  CompositeWidget::OnMouseDown(x, y, button);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
  dragging = false;
  CompositeWidget::OnMouseUp(x, y, button);
}

void Window::OnMouseMove(int32_t x, int32_t y, int32_t nx, int32_t ny) {
  if (dragging) {
    this->x = nx - x;
    this->y = ny - y;
  }
  CompositeWidget::OnMouseMove(x, y, nx, ny);
}