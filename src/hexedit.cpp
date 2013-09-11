
#include "stdafx.h"
#include "hexedit.h"

const TCHAR *HexEdit::ClassName = TEXT("HexEdit");

BOOL SetScrollInfo64(HWND hwnd, int nBar, int fMask,
                     UINT64 nMax64, UINT64 nPos64,
                     int nPage, BOOL fRedraw)
{
  SCROLLINFO si = { sizeof(si), fMask };
  if(nMax64 <= 0x7fffffff)
  {
    si.nMin = 0;
    si.nMax = (int) nMax64;
    si.nPage = (int) nPage;
    si.nPos = (int) nPos64;
  }
  else {
    si.nMin = 0;
    si.nMax = (int) 0x7fffffff;
    si.nPage = (int) nPage;
    si.nPos = (int) (nPos64 / (nMax64 / 0x7fffffff));
  }
  return SetScrollInfo(hwnd, nBar, &si, fRedraw);
}

UINT64 GetScrollPos64(HWND hwnd, int nBar,
                      int fMask, UINT64 nMax64)
{
  SCROLLINFO si = { sizeof(si), fMask | SIF_PAGE};
  UINT64 nPos32;
  if(!GetScrollInfo(hwnd, nBar, &si)) {
    return 0;
  }
  nPos32 = (fMask & SIF_TRACKPOS) ? si.nTrackPos : si.nPos;
  if(nPos32 == 0x7fffffff - si.nPage + 1) {
    return nMax64 - si.nPage + 1;
  }
  else if(nMax64 <= 0x7fffffff) {
    return nPos32;
  }
  else {
    return nPos32 * (nMax64 / 0x7fffffff);
  }
}

HexEdit::HexEdit()
{
  static BYTE *dummy = NULL;
  data_ = &dummy;
  hFont_ = NULL;
  data_size_ = 0;
  caret_offset_ = 0;
  caret_hiByte_ = false;
  display_address_ = true;
  display_hex_ = true;
  display_ascii_ = true;
  scroll_position_x_ = 0;
  scroll_position_y_ = 0;
  bytes_per_row_ = 16;
  window_height_ = 256;
  hex_left_ = 0;
  hex_right_ = 0;
  digit_width_ = 0;
  digit_height_ = 14;
  margin_address_x_ = 4;
  margin_hex_x_ = 10;
  margin_digit_ = 8;
  margin_ascii_x_ = 10;
  character_width_ = 12;
  focus_ = false;
}

HexEdit::~HexEdit()
{
  SetFont(NULL);
}

bool HexEdit::Create(DWORD dwStyles, int x, int y,
                     int width, int height, Window *parent)
{
  dwStyles |= WS_HSCROLL | WS_VSCROLL;
  window_width_ = width;
  window_height_ = height;
  HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL,
    FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH,
    "Lucida Console"
  );
  SetFont(hFont);
  bool result = Window::Create(ClassName, NULL, WS_EX_CLIENTEDGE,
    dwStyles, x, y, width, height, NULL, parent);
  if (result) {
    EnableWindow(hwnd_, TRUE);
  }
  return result;
}

void HexEdit::SetData(BYTE **data, UINT data_size)
{
  data_ = data;
  data_size_ = data_size;
  UpdateScroll();
  // TODO: Redraw control
  RedrawRegion redraw(*this);
  redraw.Invalidate(NULL);
}

void HexEdit::SetFont(HFONT hFont)
{
  if (hFont_) {
    DeleteObject((HGDIOBJ) hFont_);
    hFont_ = NULL;
  }
  character_width_ = -1;
  hFont_ = hFont;
}

void HexEdit::SetDisplay(bool address, bool hex, bool ascii)
{
  display_address_ = address;
  display_hex_ = hex;
  display_ascii_ = ascii;
}

void HexEdit::Scroll(int x, int y)
{
  int scroll_x = x,
      scroll_y = y;
  UINT surface_width = GetWidth(),
       surface_height = GetHeight();

  // Apply vertical scroll limits
  if (surface_height < (int) scroll_position_y_ + scroll_y + window_height_) {
    scroll_y = surface_height - scroll_position_y_ - window_height_ + 1;
    if (scroll_y + scroll_position_y_ < 0) {
      // stick to the top
      scroll_y = -(scroll_position_y_);
    }
  }
  if (scroll_position_y_ + scroll_y < 0) {
    scroll_y = -scroll_position_y_;
  }

  // Apply horizontal scroll limits
  if (surface_width < scroll_position_x_ + scroll_x + window_width_) {
    scroll_x = surface_width - scroll_position_x_ - window_width_ + 1;
    if (scroll_x + scroll_position_x_ < 0) {
      // stick to the right
      scroll_x = -scroll_position_x_;
    }
  }
  if (scroll_position_x_ + scroll_x < 0) {
    scroll_x = -scroll_position_x_;
  }

  if (scroll_x || scroll_y) {
    ScrollWindow(hwnd_, -scroll_x, -scroll_y, NULL, NULL);
    scroll_position_x_ += scroll_x;
    scroll_position_y_ += scroll_y;
    scroll_x = 0,
    scroll_y = 0;
  }
  UpdateScroll();
}

void HexEdit::OnPaint(HDC hdc, const RECT& rcPaint)
{
  UpdateCharacterWidth(hdc);
  HGDIOBJ hOldFont = SelectObject(hdc, (HGDIOBJ) hFont_);
  int x = -scroll_position_x_;
  COLORREF b, f;

  if (display_address_) {
    b = SetBkColor(hdc, RGB(255, 255, 230));
    f = SetTextColor(hdc, RGB(70, 70, 0));
    PaintContent(hdc, rcPaint, x, &HexEdit::PaintAddress);
    SetBkColor(hdc, b);
    SetTextColor(hdc, f);
  }

  if (display_hex_) {
    b = SetBkColor(hdc, RGB(255, 230, 255));
    f = SetTextColor(hdc, RGB(70, 0, 70));
    PaintContent(hdc, rcPaint, x, &HexEdit::PaintHex);
    SetBkColor(hdc, b);
    SetTextColor(hdc, f);
  }

  if (display_ascii_) {
    b = SetBkColor(hdc, RGB(240, 240, 255));
    f = SetTextColor(hdc, RGB(0, 0, 72));
    PaintContent(hdc, rcPaint, x, &HexEdit::PaintAscii);
    SetBkColor(hdc, b);
    SetTextColor(hdc, f);
  }

  SelectObject(hdc, hOldFont);
  SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
  ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPaint, "", 0, NULL);
}

bool HexEdit::OnEraseBackground(HDC hdc)
{
  return true;
}

void HexEdit::OnSize(WORD width, WORD height)
{
  window_width_ = width;
  window_height_ = height;
  Scroll(0, 0);
}

void HexEdit::OnMouseDown(DWORD key, int x, int y)
{
  x += scroll_position_x_;
  UINT offset;
  bool hiByte;
  if (GetOffsetFromPosition(x, y, offset, hiByte)) {
    SetCaretOffset(offset, hiByte);
  }
  SetFocus();
}

void HexEdit::OnMouseUp(DWORD key, int x, int y)
{
}

void HexEdit::OnMouseMove(int x, int y)
{
}

bool HexEdit::OnKeyDown(DWORD key_code)
{
  int off_x = 0,
      off_y = 0;
  bool ctrl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

  switch (key_code) {
  case VK_LEFT:
    off_x = -1;
    if (ctrl) {
      off_x = -2;
    }
    break ;
  case VK_UP:
    off_y = -1;
    if (ctrl) {
      off_y = -2;
    }
    break ;
  case VK_RIGHT:
    off_x = 1;
    if (ctrl) {
      off_x = 2;
    }
    break ;
  case VK_DOWN:
    off_y = 1;
    if (ctrl) {
      off_y = 2;
    }
    break ;
  case VK_HOME:
    SetCaretLineStart();
    return true;
  case VK_END:
    SetCaretLineEnd();
    return true;
  case VK_PRIOR:
    off_y = - (int)(window_height_ / GetLineHeight()) + 1;
    break ;
  case VK_NEXT:
    off_y = window_height_ / GetLineHeight() - 1;
    break ;
  }
  if (off_x || off_y) {
    AddCaretOffset(off_x, off_y);
  }
  return false;
}

bool HexEdit::OnQueryCursor(DWORD dwHitTest, const POINT& pt)
{
  if (dwHitTest != HTCLIENT) {
    return false;
  }
  if (HexDigitHitTest(pt)) {
    SetCursor(LoadCursor(NULL, IDC_IBEAM));
  }
  else {
    SetCursor(LoadCursor(NULL, IDC_ARROW));
  }
  return true;
}

LRESULT HexEdit::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  int nScrollCode;
  UINT64 pos;
  switch (msg) {
  case WM_HSCROLL:
    nScrollCode = LOWORD(wParam);
		switch (nScrollCode) {
		case SB_THUMBTRACK:
      Scroll(((int) HIWORD(wParam)) - scroll_position_x_, 0);
			break ;
		case SB_LINERIGHT:
      Scroll(1, 0);
			break ;
		case SB_LINELEFT:
      Scroll(-1, 0);
			break ;
    case SB_RIGHT:
      //Scroll(width_ - x_, 0);
      break ;
    case SB_LEFT:
      //Scroll(-x_, 0);
      break ;
    case SB_PAGERIGHT:
      Scroll(10, 0);
      break ;
    case SB_PAGELEFT:
      Scroll(-10, 0);
      break ;
		}
    return 0;
	case WM_VSCROLL:
    nScrollCode = LOWORD(wParam);
		switch (nScrollCode) {
		case SB_THUMBTRACK:
      pos = GetScrollPos64(hwnd_, SB_VERT, SIF_TRACKPOS, GetHeight());
      Scroll(0, (int) pos - scroll_position_y_);
			break ;
		case SB_LINEDOWN:
      Scroll(0, 1);
			break ;
		case SB_LINEUP:
      Scroll(0, -1);
			break ;
    case SB_PAGEDOWN:
      Scroll(0, window_height_);
      break ;
    case SB_PAGEUP:
      Scroll(0, - (int) window_height_);
      break ;
    case SB_TOP:
      Scroll(0, -scroll_position_y_);
      break ;
    case SB_BOTTOM:
      Scroll(0, GetHeight() - scroll_position_y_);
      break ;
		}
    return 0;
  case WM_MOUSEWHEEL:
    Scroll(0, - ((short) HIWORD(wParam)));
    return 0;
  case WM_PRINTCLIENT:
    {
      RECT rcClient;
      GetRect(rcClient);
      OnPaint((HDC) wParam, rcClient);
      return 0;
    }
  case WM_SETFOCUS:
    focus_ = true;
    CreateCaret(hwnd_, NULL, (digit_width_ / 2), GetLineHeight());
    UpdateCaret();
    ShowCaret(hwnd_);
    return 0;
  case WM_KILLFOCUS:
    focus_ = false;
    DestroyCaret();
    break ;
  case WM_CHAR:
    if ((wParam >= '0' && wParam <= '9') ||
        (wParam >= 'A' && wParam <= 'F') ||
        (wParam >= 'a' && wParam <= 'f'))
    {
      const UINT& chr = wParam;
      BYTE value = 0;
      if (chr >= '0' && chr <= '9') {
        value = chr - '0';
      }
      else if (chr >= 'A' && chr <= 'F') {
        value = chr - 'A' + 10;
      }
      else if (chr >= 'a' && chr <= 'f') {
        value = chr - 'a' + 10;
      }
      SetByte(caret_offset_, caret_hiByte_, value);
      AddCaretOffset(1, 0);
    }
    break ;
  case WM_GETDLGCODE:
    return DLGC_WANTALLKEYS;
  }
  return Window::OnMessage(msg, wParam, lParam);
}

UINT HexEdit::GetLineHeight() const
{
  return digit_height_;
}

UINT HexEdit::GetWidth() const
{
  return hex_right_;
}

UINT HexEdit::GetHeight() const
{
  UINT lines = data_size_ / bytes_per_row_;
  if (data_size_ % bytes_per_row_) {
    lines += 1;
  }
  return lines * GetLineHeight();
}

UINT HexEdit::GetCharacterWidth() const
{
  if (character_width_ == -1) {
    return 12;
  }
  return character_width_;
}

void HexEdit::UpdateCharacterWidth(HDC hdc)
{
  if (character_width_ == -1) {
    SIZE char_size;
    if (GetTextExtentPoint32(hdc, " ", 1, &char_size)) {
      character_width_ = char_size.cx;
    }
  }
}

void HexEdit::PaintContent(HDC hdc, const RECT& clip,
                           int& x, PaintLine paint_line_func)
{
  int rx = x;
  UINT line_height = GetLineHeight();
  UINT start = GetLineNumber(clip.top),
       end = GetLineNumber(clip.bottom);
  UINT offset = scroll_position_y_;
  for (UINT i = start; i <= end; i++) {
    int pos_y = i * line_height - offset,
        pos_x = -scroll_position_x_;
    RECT rcText = {
      pos_x,  // left
      pos_y,  // top
      pos_x + window_width_, // right
      pos_y + line_height, // bottom
    };
    rx = (this->*paint_line_func)(hdc, x, i, rcText);
  }
  x = rx;
}

int HexEdit::PaintAddress(HDC hdc, int x, UINT line, RECT& zone)
{
  if (!*data_) {
    return x;
  }
  RECT rcText;
  rcText.top = zone.top,
  rcText.bottom = zone.bottom;
  x += margin_address_x_;
  UINT addr = line * bytes_per_row_;
  TCHAR hex_addr[32];
  hex_addr[0] = '\0';
  int addr_len = sprintf_s(hex_addr, 32,
    "$%02X:%04X", (addr >> 16) & 0xff, addr & 0xffff);
  SIZE size = {};
  GetTextExtentPoint32(hdc, hex_addr, addr_len, &size);
  rcText.left = x,
  rcText.right = x + size.cx;
  ExtTextOut(hdc, x, zone.top, ETO_OPAQUE,
    &rcText, hex_addr, addr_len, NULL);
  ExcludeClipRect(hdc, x, zone.top, x + size.cx, zone.bottom);
  x += size.cx;
  return x;
}

void HexEdit::PaintHexText(HDC hdc, int& x, int y, UINT offset,
                          UINT len, const RECT& zone)
{
  if (!*data_) {
    return ;
  }
  RECT rcText;
  rcText.top = zone.top,
  rcText.bottom = zone.bottom;
  TCHAR hex_digit[128];
  hex_digit[0] = '\0';
  TCHAR *c = hex_digit;
  UINT local_x = x;
  for (UINT i = 0; i < len; i++) {
    UINT chr_offset = offset + i;
    if (chr_offset >= data_size_) {
      break ;
    }
    int digit_len = sprintf_s(hex_digit, 4,
      "%02X", (*data_)[chr_offset]);
    SIZE size = {};
    GetTextExtentPoint32(hdc, hex_digit, digit_len, &size);
    rcText.left = local_x,
    rcText.right = local_x + size.cx;
    ExtTextOut(hdc, local_x, y, ETO_OPAQUE,
      &rcText, hex_digit, digit_len, NULL);
    ExcludeClipRect(hdc, local_x, zone.top, local_x + size.cx, zone.bottom);
    digit_width_ = size.cx;
    digit_height_ = size.cy;
    local_x += digit_width_ + margin_digit_;
  }
  x += (digit_width_ + margin_digit_) * len;
}

void HexEdit::PaintAsciiText(HDC hdc, int& x, int y, UINT offset,
                            UINT len, const RECT& zone)
{
  if (!*data_) {
    return ;
  }
  RECT rcText;
  rcText.top = zone.top,
  rcText.bottom = zone.bottom;
  TCHAR hex_digit[128];
  hex_digit[0] = '\0';
  TCHAR *c = hex_digit;
  for (UINT i = 0; i < len; i++) {
    UINT chr_offset = offset + i;
    if (chr_offset >= data_size_) {
      break ;
    }
    TCHAR chr = (*data_)[chr_offset];
    if (chr < 0x20 || chr >= 0x7F) {
      chr = '.';
    }
    *c = chr;
    c++;
  }
  UINT text_len = c - hex_digit;
  SIZE size = {};
  GetTextExtentPoint32(hdc, hex_digit, text_len, &size);
  rcText.left = x;
  rcText.right = x + size.cx;
  ExtTextOut(hdc, x, y, ETO_OPAQUE,
    &rcText, hex_digit, text_len, NULL);
  ExcludeClipRect(hdc, x, zone.top, x + size.cx, zone.bottom);
  x += size.cx;
}

int HexEdit::PaintHex(HDC hdc, int x, UINT line, RECT& zone)
{
  if (!*data_) {
    return x;
  }
  x += margin_hex_x_;
  if (hex_left_ == 0) {
    hex_left_ = x + scroll_position_x_;
  }
  UINT offset = line * bytes_per_row_;
  PaintHexText(hdc, x, zone.top, offset, bytes_per_row_, zone);
  if (hex_right_ == 0) {
    hex_right_ = x + scroll_position_x_;
  }
  return x;
}

int HexEdit::PaintAscii(HDC hdc, int x, UINT line, RECT& zone)
{
  if (!*data_) {
    return x;
  }
  x += margin_ascii_x_;
  UINT offset = line * bytes_per_row_;

  PaintAsciiText(hdc, x, zone.top, offset, bytes_per_row_, zone);
  return x;
}

bool HexEdit::HexDigitHitTest(const POINT& pt)
{
  POINT pos = pt;
  pos.x += scroll_position_x_,
  pos.y += scroll_position_y_;
  if (pos.x < hex_left_ || pos.x > hex_right_) {
    return false;
  }
  //hex_position_
  return true;
}

UINT HexEdit::GetLineNumber(int y)
{
  UINT offset = (scroll_position_y_ + y) / GetLineHeight();
  if (offset * bytes_per_row_ > data_size_) {
    offset = data_size_ / bytes_per_row_;
  }
  return offset;
}

bool HexEdit::GetOffsetFromPosition(int x, int y,
                                    UINT& offset, bool& hiByte)
{
  if (x < hex_left_ || x > hex_right_) {
    return false;
  }
  UINT line = GetLineNumber(y);
  x -= hex_left_;
  UINT row_width = (digit_width_ + margin_digit_);
  UINT row_x = x - (x % row_width);
  offset = x / row_width;
  if ((x - row_x) < (digit_width_ / 2)) {
    hiByte = true;
  }
  else {
    hiByte = false;
  }
  offset += line * bytes_per_row_;
  return true;
}

bool HexEdit::GetDigitPos(UINT offset, bool hiByte, int& x, int& y)
{
  if (offset >= data_size_) {
    return false;
  }
  y = (offset / bytes_per_row_) * GetLineHeight();
  y -= scroll_position_y_;
  x = (offset % bytes_per_row_) * (digit_width_ + margin_digit_);
  x += hex_left_;
  x -= scroll_position_x_;
  if (!hiByte) {
    x += (digit_width_ / 2);
  }
  return true;
}

void HexEdit::SetCaretOffset(int offset, bool hiByte)
{
  if (data_size_ == 0) {
    return ;
  }
  if (offset < 0) {
    offset = 0;
    hiByte = true;
  }
  if (offset >= data_size_) {
    offset = data_size_ - 1;
    hiByte = false;
  }
  caret_offset_ = offset;
  caret_hiByte_ = hiByte;
  UpdateCaret();
  AdjustCaretScreen();
}

void HexEdit::SetCaretLineStart()
{
  int delta = caret_offset_ % bytes_per_row_;
  SetCaretOffset(caret_offset_ - delta, true);
}

void HexEdit::SetCaretLineEnd()
{
  int delta = (caret_offset_) % bytes_per_row_;
  SetCaretOffset(caret_offset_ - delta + bytes_per_row_ - 1, true);
}

void HexEdit::AddCaretOffset(int offset_x, int offset_y)
{
  bool hiByte = caret_hiByte_;
  if (offset_x != 0) {
    if (offset_x % 2) {
      if (!hiByte && offset_x > 0) {
        caret_offset_ += 1;
      }
      else if (hiByte && offset_x < 0) {
        caret_offset_ += -1;
      }
      hiByte = !hiByte;
    }
    SetCaretOffset(caret_offset_ + offset_x / 2, hiByte);
  }
  if (offset_y != 0) {
    int offset = offset_y * bytes_per_row_;
    offset = caret_offset_ + offset;
    if (offset < 0) {
      offset = 0;
      hiByte = true;
    }
    else if (offset >= data_size_) {
      offset = data_size_ - 1;
      hiByte = false;
    }
    SetCaretOffset(offset, hiByte);
  }
}

void HexEdit::SetByte(UINT offset, bool hiByte, BYTE value)
{
  if (offset >= data_size_ || !*data_) {
    return ;
  }
  BYTE& c = (*data_)[offset];
  if (hiByte) {
    c = (c & 0x0F) | ((value & 0x0F) << 4);
  }
  else {
    c = (c & 0xF0) | (value & 0x0F);
  }
  RedrawRegion redraw(*this);
  InvalidateDigit(offset, redraw);
}

void HexEdit::InvalidateDigit(UINT offset, RedrawRegion& redraw)
{
  int x, y;
  RECT zone;
  if (!GetDigitPos(offset, true, x, y)) {
    return ;
  }
  zone.left = x,
  zone.top = y,
  zone.right = x + digit_width_,
  zone.bottom = y + GetLineHeight();
  redraw.Invalidate(&zone);
}

void HexEdit::AdjustCaretScreen()
{
  int x, y;
  int scroll_x = 0,
      scroll_y = 0;
  if (!GetDigitPos(caret_offset_, true, x, y)) {
    return ;
  }
  // Vertical
  if (y < 0) {
    // Up
    scroll_y = y;
  }
  else if (y + GetLineHeight() > window_height_) {
    // Down
    scroll_y = (y + GetLineHeight()) - window_height_;
  }
  // Horizontal
  if (x < 0) {
    // Left
    scroll_x = x;
  }
  else if (x + digit_width_ > window_width_) {
    // Right
    scroll_x = (x + digit_width_) - window_width_;
  }
  Scroll(scroll_x, scroll_y);
}

void HexEdit::UpdateScroll()
{
  if (!hwnd_) {
    return ;
  }
  static bool locked = false;
  if (locked) {
    return ;
  }
  locked = true;

  SetScrollInfo64(hwnd_, SB_VERT,
    SIF_RANGE | SIF_PAGE | SIF_POS,
    GetHeight(), scroll_position_y_,
    window_height_, TRUE);

  /*
  si.nPos = scroll_position_y_;
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMax = GetHeight();
  si.nMin = 0;
  si.nPage = window_height_;
  SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);
  */
  SCROLLINFO si;
  si.cbSize = sizeof(si);

  si.nPos = scroll_position_x_;
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMax = GetWidth();
  si.nMin = 0;
  si.nPage = window_width_;
  SetScrollInfo(hwnd_, SB_HORZ, &si, TRUE);

  locked = false;
}

void HexEdit::UpdateCaret()
{
  int x, y;
  if (GetDigitPos(caret_offset_, caret_hiByte_, x, y)) {
    SetCaretPos(x, y);
  }
}

bool HexEdit::Register()
{
  return Window::Register(ClassName, (HBRUSH) (COLOR_WINDOW + 1));
}
