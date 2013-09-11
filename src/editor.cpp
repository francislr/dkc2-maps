
#include "stdafx.h"

#include "editor.h"
#include "frame.h"

const TCHAR *EditorHost::ClassName = TEXT("EditorHost");

EditorHost::EditorHost(MainFrame& frame) :
  frame_(frame)
{
}

bool EditorHost::Create(LPCSTR lpWindowName, int x, int y,
  int width, int height, Window *parent)
{
  DWORD dwExStyles = WS_EX_WINDOWEDGE;
  DWORD dwStyles = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
  if (!Window::Create(ClassName, lpWindowName, dwExStyles, dwStyles,
    x, y, width, height, NULL, &frame_)) {
    return false;
  }
  return true;
}

void EditorHost::OnPaint(HDC hdc, const RECT& rcPaint)
{
  if (!HasPanel()) {
    SetBkColor(hdc, RGB(0, 0, 0));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPaint, "", 0, NULL);
  }
  Window::OnPaint(hdc, rcPaint);
}

bool EditorHost::OnEraseBackground(HDC hdc)
{
  return true;
}

bool EditorHost::Register()
{
  return Window::Register(ClassName, (HBRUSH) COLOR_WINDOW + 1,
    NULL, CS_DBLCLKS);
}

bool Editor::SaveFile()
{
  return true;
}

void Editor::OnCommand(DWORD id, DWORD code)
{
  switch (id) {
  case ID_EDIT_UNDO:
    Undo();
    break ;
  case ID_EDIT_REDO:
    Redo();
    break ;
  }
}

bool Editor::Undo()
{
  return UndoBuffer::Undo();
}

bool Editor::Redo()
{
  return UndoBuffer::Redo();
}

bool Editor::Do(Ref<UndoAction> action)
{
  if (!UndoBuffer::Do(action)) {
    return false;
  }

  return true;
}

ScrollSurface::ScrollSurface(MainFrame& frame, CartFile& cart)
    : Editor(frame, cart)
{
  x_ = 0,
  y_ = 0;
  scroll_x_ = 0,
  scroll_y_ = 0;
  surface_width_ = 0,
  surface_height_ = 0;
}

ScrollSurface::~ScrollSurface()
{
}

ScrollSurface::Redraw::Redraw(ScrollSurface& surface) :
  surface_(surface), RedrawRegion(surface)
{
}

ScrollSurface::Redraw::~Redraw()
{
}

void ScrollSurface::Redraw::Invalidate(const RECT *zone)
{
  if (!zone) {
    RedrawRegion::Invalidate(zone);
    return ;
  }
  RECT rcInvalid = *zone;
  OffsetRect(&rcInvalid, -surface_.X(), -surface_.Y());
  RedrawRegion::Invalidate(&rcInvalid);
}

void ScrollSurface::OnCreate()
{
  RECT rcClient;
  GetClientRect(hwnd_, &rcClient);
  viewRc_.left = 0,
  viewRc_.top = 0,
  viewRc_.right = rcClient.right;
  viewRc_.bottom = rcClient.bottom;
  ShowScrollBar(hwnd_, SB_BOTH, TRUE);
}

void ScrollSurface::OnSize(WORD width, WORD height)
{
  HDC hdc = GetDC();
  BitmapSurface::Resize(width, height, 128);
  ReleaseDC(hwnd_, hdc);
  UpdateScroll(0, 0);
  RECT rcClient;
  GetClientRect(hwnd_, &rcClient);
  viewRc_.right = x_ + rcClient.right,
  viewRc_.bottom = y_ + rcClient.bottom;
}

LRESULT ScrollSurface::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  int nScrollCode,
      n = 0;
  RECT rect;
  switch (msg) {
  case WM_HSCROLL:
    nScrollCode = LOWORD(wParam);
    Window::GetRect(rect);
		switch (nScrollCode) {
		case SB_THUMBTRACK:
      UpdateScroll(HIWORD(wParam) - x_, 0);
			break ;
		case SB_LINERIGHT:
      UpdateScroll(5, 0);
			break ;
		case SB_LINELEFT:
      UpdateScroll(-5, 0);
			break ;
    case SB_RIGHT:
      UpdateScroll(width_ - x_, 0);
      break ;
    case SB_LEFT:
      UpdateScroll(-x_, 0);
      break ;
    case SB_PAGERIGHT:
      UpdateScroll(rect.right, 0);
      break ;
    case SB_PAGELEFT:
      UpdateScroll(-rect.right, 0);
      break ;
		}
    return 0;
	case WM_VSCROLL:
    nScrollCode = LOWORD(wParam);
    Window::GetRect(rect);
		switch (nScrollCode) {
    case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
      UpdateScroll(0, HIWORD(wParam) - y_);
			break ;
		case SB_LINEDOWN: // down - right
      UpdateScroll(0, 5);
			break ;
		case SB_LINEUP: // up - left
      UpdateScroll(0, -5);
			break ;
    case SB_PAGEDOWN:
      UpdateScroll(0, rect.bottom);
      break ;
    case SB_PAGEUP:
      UpdateScroll(0, -rect.bottom);
      break ;
    case SB_TOP:
      UpdateScroll(0, -y_);
      break ;
    case SB_BOTTOM:
      UpdateScroll(0, height_ - y_);
      break ;
		}
    return 0;
  }
  return Window::OnMessage(msg, wParam, lParam);
}

void ScrollSurface::OnPaint(HDC hdc, const RECT& rcPaint)
{
  const RECT& pos = GetScrollRect();
  
  COLORREF color;
  color = SetBkColor(hdc_, RGB(0, 0, 0));
  ExtTextOut(hdc_, 0, 0, ETO_OPAQUE, &rcPaint, "", 0, NULL);
  SetBkColor(hdc_, color);
  
  OnPaintArea(pos, hdc);
  Draw(hdc,
    rcPaint.left,
    rcPaint.top,
    rcPaint.right - rcPaint.left,
    rcPaint.bottom - rcPaint.top,
    rcPaint.left,
    rcPaint.top
  );
}

bool ScrollSurface::OnDestroy()
{
  DWORD dwStyle = GetWindowLong(hwnd_, GWL_STYLE);
  //SetWindowLong(hwnd_, GWL_STYLE, dwStyle & ~(WS_VSCROLL | WS_HSCROLL));
  ShowScrollBar(hwnd_, SB_BOTH, FALSE);
  return true;
}

void ScrollSurface::OnMouseDown(DWORD key, int x, int y)
{
  x += x_,
  y += y_;
  Window::OnMouseDown(key, x, y);
}

void ScrollSurface::OnMouseUp(DWORD key, int x, int y)
{
  x += x_,
  y += y_;
  Window::OnMouseUp(key, x, y);
}

void ScrollSurface::OnMouseMove(int x, int y)
{
  x += x_,
  y += y_;
  Window::OnMouseMove(x, y);
}

bool ScrollSurface::OnQueryCursor(DWORD dwHitTest, const POINT& pt)
{
  POINT pt2 = pt;
  pt2.x += x_,
  pt2.y += y_;
  return Window::OnQueryCursor(dwHitTest, pt2);
}

void ScrollSurface::GetCursorPos(POINT& pt)
{
  DWORD cursor_pos = GetMessagePos();
  pt.x = GET_X_LPARAM(cursor_pos),
  pt.y = GET_Y_LPARAM(cursor_pos);
  ScreenToClient(hwnd_, &pt);
  pt.x += viewRc_.left,
  pt.y += viewRc_.top;
}

void ScrollSurface::ClientToScroll(RECT& rc)
{
  OffsetRect(&rc, viewRc_.left, viewRc_.top);
}

void ScrollSurface::ScrollToClient(RECT& rc)
{
  OffsetRect(&rc, -viewRc_.left, -viewRc_.top);
}

void ScrollSurface::SetDimensions(int width, int height)
{
  surface_width_ = width,
  surface_height_ = height;
  UpdateScroll(0, 0);
}

const RECT& ScrollSurface::GetScrollRect() const
{
  return scroll_rect_;
}

void ScrollSurface::GetRect(RECT& rect) const
{
  rect = scroll_rect_;
}

void ScrollSurface::GetView(RECT& rcView)
{
  rcView = viewRc_;
}

bool ScrollSurface::Resize(int width, int height)
{
  if (!BitmapSurface::Resize(width, height, 10)) {
    return false;
  }
  SetDimensions(width, height);
  return true;
}

void ScrollSurface::UpdateScroll(int x, int y)
{
  Scroll(x, y);
}

void ScrollSurface::Scroll(int& x, int& y)
{
  static bool locked = false;
  if (locked) {
    return ;
  }
  locked = true;
  scroll_x_ += x,
  scroll_y_ += y;
  SCROLLINFO si;
  RECT rect;
  Window::GetRect(rect);
  si.cbSize = sizeof(si);
  si.fMask = SIF_POS;
  GetScrollInfo(hwnd_, SB_VERT, &si);
  si.fMask = NULL;
  if (surface_height_ < y_ + scroll_y_ + rect.bottom) {
    scroll_y_ = surface_height_ - y_ - rect.bottom + 1;
    if (scroll_y_ + y_ < 0) {
      // stick to the top
      scroll_y_ = -y_;
    }
  }
  if (y_ + scroll_y_ < 0) {
    scroll_y_ = -y_;
  }

  if (scroll_y_) {
    y_ += scroll_y_;
    si.fMask |= SIF_POS;
    si.nPos = y_;
  }
  si.fMask |= SIF_RANGE | SIF_PAGE;
  si.nMax = surface_height_;
  si.nMin = 0;
  si.nPage = rect.bottom;
  SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);
  si.fMask = SIF_POS;
  GetScrollInfo(hwnd_, SB_HORZ, &si);
  si.fMask = NULL;
  if (surface_width_ < x_ + scroll_x_ + rect.right) {
    scroll_x_ = surface_width_ - x_ - rect.right + 1;
    if (scroll_x_ + x_ < 0) {
      // stick to the right
      scroll_x_ = -x_;
    }
  }
  if (x_ + scroll_x_ < 0) {
    scroll_x_ = -x_;
  }

  if (scroll_x_) {
    x_ += scroll_x_;
    si.fMask |= SIF_POS;
    si.nPos = x_;
  }
  si.fMask |= SIF_RANGE | SIF_PAGE;
  si.nMax = surface_width_;
  si.nMin = 0;
  si.nPage = rect.right;
  SetScrollInfo(hwnd_, SB_HORZ, &si, TRUE);

  x = 0,
  y = 0;
  if (scroll_x_ || scroll_y_) {
    ScrollDC(hdc_, -scroll_x_, -scroll_y_, NULL, NULL, NULL, NULL);
    ScrollWindow(hwnd_, -scroll_x_, -scroll_y_, NULL, NULL);
    x = scroll_x_,
    y = scroll_y_;
    viewRc_.left += scroll_x_,
    viewRc_.top += scroll_y_,
    viewRc_.right += scroll_x_,
    viewRc_.bottom += scroll_y_;
    scroll_x_ = 0,
    scroll_y_ = 0;
  }
  SetRect(&scroll_rect_,
    -x_, -y_,
    x_ + width_, y_ + height_
  );
  locked = false;
}

const DWORD EditTool::kTimerId = 0x2001;

EditTool::EditTool(ScrollSurface& target) : target_(target)
{
  selection_active_ = false;
  drag_active_ = false;
}

EditTool::~EditTool()
{
  StopSelection();
  StopDrag();
}

void EditTool::StartSelection()
{
  if (!IsValid()) {
    return ;
  }
  StartDrag();
  selectionRc_.left = dragPt_.x,
  selectionRc_.top = dragPt_.y;
  selectionRc_.right = dragPt_.x;
  selectionRc_.bottom = dragPt_.y;

  selection_active_ = true;
  UpdateSelection();
}

void EditTool::StopSelection()
{
  if (!selection_active_) {
    return ;
  }
  ScrollSurface::Redraw redraw(target_);
  RECT selection = selectionRc_;
  if (selectionRc_.right < selectionRc_.left) {
    selection.left = selectionRc_.right;
    selection.right = selectionRc_.left;
  }
  if (selectionRc_.bottom < selectionRc_.top) {
    selection.top = selectionRc_.bottom;
    selection.bottom = selectionRc_.top;
  }
  OnSelectionRelease(selection, redraw);
  InvalidateFocusRect(redraw);
  selection_active_ = false;
}

bool EditTool::IsSelectionActive() const
{
  return selection_active_;
}

void EditTool::OnSelectionMove(const RECT& selection)
{
}

void EditTool::OnSelectionRelease(const RECT& selection, RedrawRegion& redraw)
{
}

void EditTool::StartDrag()
{
  if (!IsValid()) {
    return ;
  }
  StopSelection();
  StopDrag();
  target_.GetCursorPos(dragPt_);
  drag_active_ = true;
  target_.CaptureMouse();
  ix_ = dragPt_.x;
  iy_ = dragPt_.y;
  SetTimer(hwnd_, kTimerId, 200, NULL);
}

void EditTool::StopDrag()
{
  if (!drag_active_) {
    return ;
  }
  if (!IsValid()) {
    return ;
  }
  target_.ReleaseMouse();
  target_.GetCursorPos(dragPt_);
  if (!selection_active_) {
    ix_ = dragPt_.x - ix_;
    iy_ = dragPt_.y - iy_;
    OnDragRelease(dragPt_, ix_, iy_);
  }
  else {
    // Selection marquee
    UpdateSelection();
    StopSelection();
  }
  KillTimer(hwnd_, kTimerId);
  drag_active_ = false;
}

bool EditTool::IsDragging() const
{
  return drag_active_ && !selection_active_;
}

void EditTool::OnDragMove(int x, int y)
{
}

void EditTool::OnDragRelease(const POINT& pt, int ix, int iy)
{
}

void EditTool::OnPaintArea(const RECT& pos, HDC hdc)
{
  DrawFocusRect(pos, hdc);
}

void EditTool::OnMouseMove(int x, int y)
{
  if (drag_active_) {
    Drag(x, y);
  }
}

void EditTool::OnMouseUp(DWORD key, int x, int y)
{
  StopDrag();
}

void EditTool::OnTimer(DWORD id)
{
  if (id == kTimerId) {
    int x = 0,
        y = 0;
    RECT rcView;
    target_.GetView(rcView);
    int mult = 0;
    if (dragPt_.x < rcView.left + 50) {
      x = (int) -(rcView.left - dragPt_.x + 50) * 0.8;
    }
    else if (dragPt_.x > rcView.right - 50) {
      x = (int) (dragPt_.x + 50 - rcView.right) * 0.8;
    }
    if (dragPt_.y < rcView.top + 50) {
      y = (int) -(rcView.top - dragPt_.y + 50) * 0.8;
    }
    else if (dragPt_.y > rcView.bottom - 50) {
      y = (dragPt_.y + 50 - rcView.bottom) * 0.8;
    }
    if (x || y) {
      if (selection_active_) {
        ScrollSurface::Redraw redraw(target_);
        InvalidateFocusRect(redraw);
      }
      target_.Scroll(x, y);
      dragPt_.x += x,
      dragPt_.y += y;
      if (selection_active_) {
        UpdateSelection();
      }
      else {
         OnDragMove(x, y);
      }
    }
  }
}

void EditTool::Drag(int x, int y)
{
  POINT new_pos;
  target_.GetCursorPos(new_pos);
  if (!selection_active_) {
    int dx = dragPt_.x - x,
        dy = dragPt_.y - y;
    dragPt_ = new_pos;
    OnDragMove(-dx, -dy);
  }
  else {
    dragPt_ = new_pos;
    UpdateSelection();
  }
}

void EditTool::UpdateSelection()
{
  if (!selection_active_) {
    return ;
  }
  int x = dragPt_.x,
      y = dragPt_.y;
  ScrollSurface::Redraw redraw(target_);
  InvalidateFocusRect(redraw);
  selectionRc_.right = x;
  selectionRc_.bottom = y;
  InvalidateFocusRect(redraw);
}

void EditTool::DrawFocusRect(const RECT& pos, HDC hdc)
{
  if (!selection_active_) {
    return ;
  }
  HPEN marqueePen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
  HPEN oldPen = (HPEN) SelectObject(hdc, (HGDIOBJ) marqueePen);
  HBRUSH oldBrush = (HBRUSH) SelectObject(hdc, (HGDIOBJ) GetStockObject(NULL_BRUSH));
  RECT zone = selectionRc_;
  if (selectionRc_.right < selectionRc_.left) {
    zone.left = selectionRc_.right;
    zone.right = selectionRc_.left;
  }
  if (selectionRc_.bottom < selectionRc_.top) {
    zone.top = selectionRc_.bottom;
    zone.bottom = selectionRc_.top;
  }

  target_.ScrollToClient(zone);
  Rectangle(hdc, zone.left, zone.top, zone.right, zone.bottom);
  SelectObject(hdc, (HGDIOBJ) oldBrush);
  SelectObject(hdc, (HGDIOBJ) oldPen);
  DeleteObject((HGDIOBJ) marqueePen);
	ExcludeClipRect(hdc, zone.left, zone.top, zone.right, zone.top + 1);
	ExcludeClipRect(hdc, zone.left, zone.top, zone.left + 1, zone.bottom);
	ExcludeClipRect(hdc, zone.right, zone.top, zone.right - 1, zone.bottom);
	ExcludeClipRect(hdc, zone.left, zone.bottom - 1, zone.right, zone.bottom);
}

void EditTool::InvalidateFocusRect(RedrawRegion& redraw)
{
  if (!selection_active_) {
    return ;
  }
  RECT zone = selectionRc_;
  target_.ScrollToClient(zone);
  redraw.Invalidate(&zone);
  zone.right = selectionRc_.left + 1;
  redraw.Invalidate(&zone); // left
  zone.right = selectionRc_.right;

  zone.bottom = zone.top + 1;
  redraw.Invalidate(&zone); // top
  zone.bottom = selectionRc_.bottom;

  zone.left = zone.right - 1;
  redraw.Invalidate(&zone); // right
  zone.left = selectionRc_.left;

  zone.top = zone.bottom - 1;
  redraw.Invalidate(&zone); // bottom
}

SelectionMarquee::SelectionMarquee(Window& target)
  : target_(target)
{
  grid_ = 0;
  active_ = false;
}

void SelectionMarquee::Activate(int x, int y)
{
  target_.CaptureMouse();
  active_ = true;
  if (grid_) {
    x -= x % grid_;
    y -= y % grid_;
  }
  zone_.left = x;
  zone_.top = y;
  MouseMove(x, y);
}

void SelectionMarquee::Desactivate(int x, int y)
{
  target_.ReleaseMouse();
  active_ = false;
  MouseMove(x, y);
  target_.Invalidate(NULL);
  target_.Update();
}

void SelectionMarquee::Draw(HDC hdc, const RECT& pos)
{
  if (!active_) {
    return ;
  }
  HPEN marqueePen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
  HPEN oldPen = (HPEN) SelectObject(hdc, (HGDIOBJ) marqueePen);
  HBRUSH oldBrush = (HBRUSH) SelectObject(hdc, (HGDIOBJ) GetStockObject(BLACK_BRUSH));
  RECT zone = zone_;
  OffsetRect(&zone, pos.left, pos.top);
  MoveToEx(hdc, zone.left, zone.bottom, NULL);
  LineTo(hdc, zone.left, zone.top);
  LineTo(hdc, zone.right, zone.top);
  LineTo(hdc, zone.right, zone.bottom);
  LineTo(hdc, zone.left, zone.bottom);
  SelectObject(hdc, (HGDIOBJ) oldPen);
  SelectObject(hdc, (HGDIOBJ) oldBrush);
  DeleteObject((HGDIOBJ) marqueePen);
	ExcludeClipRect(hdc, zone.left, zone.top, zone.right, zone.top + 1);
	ExcludeClipRect(hdc, zone.left, zone.top, zone.left + 1, zone.bottom);
	ExcludeClipRect(hdc, zone.right, zone.top, zone.right + 1, zone.bottom);
	ExcludeClipRect(hdc, zone.left, zone.bottom, zone.right, zone.bottom + 1);
}

void SelectionMarquee::MouseMove(int x, int y)
{
  if (!active_) {
    return ;
  }
  if (x < 1) {
    x = 1;
  }
  if (y < 1) {
    y = 1;
  }
  if (grid_) {
    x -= x % grid_;
    y -= y % grid_;
    if (x >= zone_.left) {
      x += grid_;
    }
    if (y >= zone_.top) {
      y += grid_;
    }
  }
  zone_.right = x;
  zone_.bottom = y;
  target_.Invalidate(NULL);
  target_.Update();
}

bool SelectionMarquee::IsActive() const
{
  return active_;
}

RECT& SelectionMarquee::GetSelectionZone()
{
  selection_zone_ = zone_;
  if (zone_.right < zone_.left) {
    selection_zone_.left = zone_.right;
    selection_zone_.right = zone_.left;
  }
  if (zone_.bottom < zone_.top) {
    selection_zone_.top = zone_.bottom;
    selection_zone_.bottom = zone_.top;
  }
  return selection_zone_;
}

DragDrop::DragDrop(Window& host) :
  host_(host)
{
  active_ = false;
}

void DragDrop::Activate(int x, int y)
{
  active_ = true;
  last_move_.x = x,
  last_move_.y = y;
  host_.CaptureMouse();
}

void DragDrop::Desactivate(int x, int y)
{
  if (!active_) {
    return ;
  }
  host_.ReleaseMouse();
  active_ = false;
}

void DragDrop::MouseMove(int& x, int& y)
{
  if (!active_) {
    return ;
  }
  int x1, y1;
  x1 = last_move_.x - x,
  y1 = last_move_.y - y;
  last_move_.x = x,
  last_move_.y = y;
  x = -x1,
  y = -y1;
}

bool DragDrop::IsActive() const
{
  return active_;
}
