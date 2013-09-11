
#pragma once

BOOL SetScrollInfo64(HWND hwnd, int nBar, int fMask,
                     UINT64 nMax64, UINT64 nPos64,
                     int nPage, BOOL fRedraw);

class HexEdit : public Window
{
public:
  HexEdit();
  ~HexEdit();

  bool Create(DWORD dwStyles, int x, int y,
    int width, int height, Window *parent);
  void SetData(BYTE **data, UINT data_size);
  void SetFont(HFONT hFont);
  void SetDisplay(bool address, bool hex, bool ascii);
  void Scroll(int x, int y);

  virtual void OnPaint(HDC hdc, const RECT& rcPaint);
  virtual bool OnEraseBackground(HDC hdc);
  virtual void OnSize(WORD width, WORD height);
  virtual void OnMouseDown(DWORD key, int x, int y);
  virtual void OnMouseUp(DWORD key, int x, int y);
  virtual void OnMouseMove(int x, int y);
  virtual bool OnKeyDown(DWORD key_code);
  virtual bool OnQueryCursor(DWORD dwHitTest, const POINT& pt);
  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  static bool Register();

private:
  UINT GetLineHeight() const;
  UINT GetWidth() const;
  UINT GetHeight() const;
  UINT GetCharacterWidth() const;
  void UpdateCharacterWidth(HDC hdc);

  typedef int (HexEdit::*PaintLine)(HDC hdc, int x, UINT line, RECT& zone);

  void PaintContent(HDC hdc, const RECT& clip, int& x, PaintLine paint_line_func);
  int PaintAddress(HDC hdc, int x, UINT line, RECT& zone);
  void PaintHexText(HDC hdc, int& x, int y,
    UINT offset, UINT len, const RECT& zone);
  void PaintAsciiText(HDC hdc, int& x, int y,
    UINT offset, UINT len, const RECT& zone);
  int PaintHex(HDC hdc, int x, UINT line, RECT& zone);
  int PaintAscii(HDC hdc, int x, UINT line, RECT& zone);
  bool HexDigitHitTest(const POINT& pt);
  UINT GetLineNumber(int y);
  bool GetOffsetFromPosition(int x, int y, UINT& offset, bool& hiByte);
  bool GetDigitPos(UINT offset, bool hiByte, int& x, int& y);
  void SetCaretOffset(int offset, bool hiByte);
  void SetCaretLineStart();
  void SetCaretLineEnd();
  void AddCaretOffset(int offset_x, int offset_y);
  void SetByte(UINT offset, bool hiByte, BYTE value);
  void InvalidateDigit(UINT offset, RedrawRegion& redraw);

  /* Keeps the caret in the screen */
  void AdjustCaretScreen();

  void UpdateScroll();
  void UpdateCaret();

  HFONT hFont_;

  /* Data to display */
  BYTE **data_;
  UINT data_size_;

  /* Caret */
  UINT caret_offset_;
  bool caret_hiByte_;

  /* Scrolling */
	int scroll_position_x_;	
	int scroll_range_x_;
	int scroll_position_y_;	
	int scroll_range_y_;

  UINT bytes_per_row_;
  UINT window_width_;
  UINT window_height_;

  int hex_left_;
  int hex_right_;
  int digit_width_;
  int digit_height_;

  UINT margin_address_x_;
  UINT margin_hex_x_;
  UINT margin_digit_;
  UINT margin_ascii_x_;

  int character_width_;

  bool display_address_;
  bool display_hex_;
  bool display_ascii_;
  bool focus_;

  static const TCHAR *ClassName;
};
