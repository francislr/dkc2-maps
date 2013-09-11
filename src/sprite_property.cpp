
#include "stdafx.h"

#include "rom_file.h"
#include "sprite_property.h"

SpriteProperty::SpriteProperty()
{
  root_prop_ = false;
}

SpriteProperty::~SpriteProperty()
{
}

bool SpriteProperty::Load(CartFile& cart,
  SpritePropertyPool& props, DWORD addr)
{
  root_prop_ = true;
  CartFile::Scope scope(cart);
  if (!LoadSubProp(0, cart, props, addr)) {
    return false;
  }
  return true;
}

bool SpriteProperty::IsRoot() const
{
  return root_prop_;
}

bool SpriteProperty::GetGraphics(Buffer& src_buf, SpriteGraphicDef& gfx)
{
  CartFile::Scope scope(src_buf);
  gfx.palette_id = gfx.anim_id = gfx.sprite_id = 0;
  gfx.flip_horz = false;
  gfx.flip_vert = false;
  SpriteProperty prop;
  // 0x001C prop = Invisible
  if (GetSubProperty(0x8D00, prop)) {
    gfx.palette_id = prop.GetWord(0);
    if (GetSubProperty(0x8100, prop)) {
      // Load an animation
      gfx.anim_id = prop.GetWord(0);
      return true;
    }
    if (GetSubProperty(0x001A, prop)) {
      // Load a static graphic
      gfx.sprite_id = prop.GetWord(0);
      SpriteProperty sub_prop;
      if (GetSubProperty(0x8A00, sub_prop)) {
        WORD barrel_type = sub_prop.GetWord(0);
        BYTE initial_dir = sub_prop.GetByte(2),
             throw_dir = sub_prop.GetByte(3);
        WORD barrel_index = (initial_dir + 0x0008) & 0x00F0;
        barrel_index >>= 3;
        DWORD gfx_table = 0;
        if ((barrel_type & 0xA007) == 0 || (barrel_type & 0xA003) != 0) {
          gfx_table = 0x39F082; // Default look
        }
        else { // Auto-shoot barrel
          gfx_table = 0x39F0A2;
          if (throw_dir == initial_dir) {
            gfx_table = 0x33F26E; // Arrow barrel
            WORD flip;
            if (!src_buf.Read(0x33F28E + barrel_index, &flip, 2)) {
              return false;
            }
            gfx.flip_horz = (flip & 0x4000) != 0;
            gfx.flip_vert = (flip & 0x8000) != 0;
          }
        }
        if (gfx_table != 0) {
          /*
          gfx_table = 0x33F26E; // Arrow barrel
          gfx_table = 0x39F082; // Default look
          gfx_table = 0x39F0A2; // Auto-shoot barrel
          */

          if (!src_buf.Read(gfx_table + barrel_index, &gfx.sprite_id, 2)) {
            return false;
          }
        }
      }
      return true;
    }
  }
  return false;
}

BYTE SpriteProperty::GetByte(UINT index) const
{
  if (index >= 32) {
    return 0;
  }
  return data_.bytes[index];
}

WORD SpriteProperty::GetWord(UINT index) const
{
  if (index >= 16) {
    return 0;
  }
  return data_.words[index];
}

DWORD SpriteProperty::GetDword(UINT index) const
{
  if (index >= 8) {
    return 0;
  }
  return data_.dwords[index];
}

bool SpriteProperty::GetSubProperty(WORD op,
  SpriteProperty& prop) const
{
  std::vector<Ref<SpriteProperty> >::const_iterator i;
  for (i = properties_.begin(); i != properties_.end(); ++i) {
    Ref<SpriteProperty> p = *i;
    if (p->op_ == op) {
      prop = *(*p);
      prop.addr_ = p->addr_;
      return true;
    }
    if (p->GetSubProperty(op, prop)) {
      return true;
    }
  }
  return false;
}

SpriteProperty::SubPropList& SpriteProperty::GetSubPropList()
{
  return properties_;
}

void SpriteProperty::GetPropName(std::string& name)
{
  name.clear();
  char str[32];
  if (sprintf_s(str, 32, "%04X", op_) <= 0) {
    return ;
  }
  switch (op_) {
  case 0x8300:
    name = "Inherit property";
    break ;
  case 0x8000:
    name = "End";
    break ;
  case 0x8D00:
    name = "Palette";
    break ;
  case 0x8100:
    name = "Animation";
    break ;
  case 0x001A:
    name = "Static graphic";
    break ;
  case 0x001C:
    name = "Invisible";
    break ;
  case 0x8A00:
    name = "Barrel";
    break ;
  case 0x0000:
    name = "Routine";
    break ;
  case 0x0042:
    name = "Quantity";
    break ;
  case 0x002E:
    // Routine 2 $BE/B846 7C 00 00 JMP ($0000,x)[$BE:B9D9]
  case 0x0048:
    // Used by coin counter
  default:
    name.append(str);
  }
}

bool SpriteProperty::GetPropData(BYTE **prop_data, int& prop_size)
{
  prop_size = GetPropSize(op_);
  if (prop_size == -1) {
    return false;
  }
  *prop_data = data_.bytes;
  return true;
}

bool SpriteProperty::LoadSubProp(int recursion, CartFile& cart,
  SpritePropertyPool& props, DWORD addr)
{
  CartFile::Scope scope(cart);
  addr |= 0x003F0000;
  addr_ = addr;
  cart.Seek(addr);
  return LoadSubProp(recursion, cart, props);
}

bool SpriteProperty::LoadSubProp(int recursion, CartFile& cart,
  SpritePropertyPool& props)
{
  recursion++;
  bool end = false;
  WORD op;
  int prop_size;
  while (!end) {
    DWORD addr = cart.GetPosition();
    if (!cart.ReadWord(op)) {
      return false;
    }
    union {
      BYTE bytes[32];
      WORD words[16];
    } data;
    Ref<SpriteProperty> prop = new SpriteProperty();
    if (!prop) {
      return false;
    }
    prop_size = GetPropSize(op);
    prop->op_ = op;
    prop->addr_ = addr;
    if (prop_size == -1) {
      return false;
    }
    if (!cart.Read(data.bytes, prop_size)) {
      return false;
    }
    memcpy(prop->data_.bytes, data.bytes, prop_size);
    switch (op) {
    case 0x8000:
      end = true;
      break ;
    case 0x8300:
      if (!prop->LoadSubProp(recursion,
        cart, props, data.words[0]))
      {
        return false;
      }
      break ;
    }
    properties_.push_back(prop);
  }
  return true;
}

int SpriteProperty::GetPropSize(WORD op)
{
  if (op < 0x8000) {
    return 2;
  }
  switch (op) {
  case 0x8000:
    return 0;
  case 0x8100:
  case 0x8300:
  case 0x8400:
  case 0x8500:
  case 0x8D00:
    return 2;
  case 0x8800:
    return 0;
  case 0x8A00:
    return 26;
  }
  return -1;
}

SpritePropertyPool::SpritePropertyPool(CartFile& cart) :
  cart_(cart)
{
}

bool SpritePropertyPool::OnResourceAlloc(DWORD id,
  SpriteProperty* prop)
{
  return prop->Load(cart_, *this, id);
}

Ref<SpriteProperty> SpritePropertyPool::NewFromID(DWORD prop_id)
{
  DWORD addr = 0;
  if (!cart_.Read(0x3BE800 + prop_id, &addr, 2)) {
    return Ref<SpriteProperty>();
  }
  return New(addr);
}

Ref<SpriteProperty> SpritePropertyPool::NewFromID2(DWORD prop_id)
{
  DWORD addr = 0;
  if (!cart_.Read(0x3F047E + prop_id, &addr, 2)) {
    return Ref<SpriteProperty>();
  }
  return New(addr);
}

EditPropWindow::EditPropWindow()
{
}

EditPropWindow::~EditPropWindow()
{
}

bool EditPropWindow::Create(LPCSTR lpWindowName, int x, int y,
  int width, int height, Window& parent)
{
  DWORD dwExStyles = WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW;
  DWORD dwStyles = WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_CLIPCHILDREN;
  if (!Window::Create(ClassName, lpWindowName, dwExStyles, dwStyles,
    x, y, width, height, NULL, &parent)) {
    return false;
  }
  RECT rcClient;
  GetRect(rcClient);
  properties_.Create(ID_PROPLIST, WS_EX_CLIENTEDGE, WS_TABSTOP | WS_VISIBLE | TVS_HASLINES |
    TVS_HASBUTTONS | TVS_LINESATROOT | WS_VSCROLL | WS_HSCROLL, 5, 5,
    140, rcClient.bottom - 10, *this);
  prop_name_.Create(_T("Static"), _T("Binary"),
    NULL, WS_CHILD | WS_VISIBLE, 150, 5, rcClient.right - 10 - 145, 24,
    0, this);
  hex_edit_.Create(WS_CHILD | WS_TABSTOP,
    150, 29, rcClient.right - 10 - 145, 53,
    this
  );
  hex_edit_.SetDisplay(false, true, false);
  hex_edit_.Show();

  Show();
  return true;
}
bool EditPropWindow::SetProperty(Ref<SpriteProperty> prop)
{
  properties_.DeleteAll();
  prop_ = prop;
  if (!prop_) {
    return false;
  }
  InsertTreeProp(TVI_ROOT, **prop_);
  return true;
}

bool EditPropWindow::InsertTreeProp(HTREEITEM hTreeItem, SpriteProperty& prop)
{
  std::string prop_name;
  prop.GetPropName(prop_name);
  SpriteProperty::SubPropList& sub_props = prop.GetSubPropList();
  SpriteProperty::SubPropList::iterator i;
  TVINSERTSTRUCT tvinsert = {};
  tvinsert.hParent = hTreeItem;
  tvinsert.hInsertAfter = TVI_ROOT;
  tvinsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
  tvinsert.item.pszText = (LPSTR) prop_name.data();
  tvinsert.item.cchTextMax = prop_name.length();
  tvinsert.item.cChildren = sub_props.size() > 0 ? 1 : 0;
  tvinsert.item.lParam = (LPARAM) &prop;
  if (!prop.IsRoot()) {
   if (tvinsert.item.cChildren == 1) {
      tvinsert.item.mask |= TVIF_STATE;
      tvinsert.item.state = TVIS_EXPANDED;
      tvinsert.item.stateMask = TVIS_EXPANDED;
    }
   hTreeItem = properties_.InsertItem(&tvinsert);
  }
  for (i = sub_props.begin(); i != sub_props.end(); ++i) {
    InsertTreeProp(hTreeItem, ***i);
  }
  return true;
}

void EditPropWindow::OnCreate()
{
}

void EditPropWindow::OnSelectProperty()
{
  if (!selected_prop_) {
    return ;
  }
  int prop_size;
  static BYTE *prop_data;
  if (!selected_prop_->GetPropData(&prop_data, prop_size)) {
    return ;
  }
  hex_edit_.SetData(&prop_data, prop_size);
}

void EditPropWindow::OnSize(WORD width, WORD height)
{
  properties_.SetSize(140, height - 10);
  hex_edit_.SetSize(width - 10 - 145, 53);

}

LRESULT EditPropWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_NOTIFY:
    switch (((LPNMHDR) lParam)->code) {
    case TVN_SELCHANGED:
      if (((LPNMHDR) lParam)->idFrom == ID_PROPLIST) {
        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW) lParam;
        if ((lpnmtv->itemNew.mask & TVIF_PARAM) != 0) {
          selected_prop_ = (SpriteProperty*) lpnmtv->itemNew.lParam;
          OnSelectProperty();
        }
      }
      break ;
    }
    break ;
  }
  return Window::OnMessage(msg, wParam, lParam);
}

bool EditPropWindow::Register()
{
  return Window::Register(ClassName, (HBRUSH) GetSysColorBrush(COLOR_3DFACE));
}

const TCHAR *EditPropWindow::ClassName = TEXT("PropWnd");
