
#include "stdafx.h"

#include "level_sprite.h"
#include "level.h"
#include "rom_file.h"
#include "resource.h"
#include "sprite_property.h"
#include "frame.h"

const MENUDEF kLevelSpriteMenus[] = {
//  { MAKEINTRESOURCE(IDR_MENU_SPRITE), "Sprite" },
  { NULL, NULL },
};

const UINT kLevelSpriteMenuOffset = 3;

LevelSprite::LevelSprite(SpritePropertyPool& prop_pool, SpritePool& sprite_pool) :
  prop_pool_(prop_pool), sprite_pool_(sprite_pool)
{
}

LevelSprite::~LevelSprite()
{
}

bool LevelSprite::Load(CartFile& cart)
{
  CartFile::Scope scope(cart);
  DWORD addr = 0;

  property_ = prop_pool_.NewFromID(prop_id_);
  if (!property_) {
    return false;
  }
  return LoadFromProperty(cart, sprite_pool_, *(*property_));
}

bool LevelSprite::Serialize(Buffer& dest_buf)
{
  return dest_buf.Writef("HHHH",
    &param_, &x_, &y_, &prop_id_);
}

bool LevelSprite::Deserialize(Buffer& src_buf)
{
  if (!src_buf.Readf("HHHH",
    &param_, &x_, &y_, &prop_id_))
  {
    return false;
  }
  if (param_ == 0) {
    return false;
  }
  CartFile::Scope scope(src_buf);
  property_ = prop_pool_.NewFromID(prop_id_);
  if (!property_) {
    return false;
  }
  LoadFromProperty(src_buf, sprite_pool_, *(*property_));
  return true;
}

bool LevelSprite::Draw(BitmapSurface& surface, const RECT& pos)
{
  int x, y;
  x = pos.left + x_ - 256,
  y = pos.top + y_ - 256;
  if (!Animation::Draw(surface,
    x, y,
    0, 0))
  {
    return false;
  }
  if (selected_) {
    DrawSelection(surface, x, y);
  }
  return true;
}

bool LevelSprite::IntersectBox(RECT& box)
{
  OffsetRect(&box, -x_ + 256, -y_ + 256);
  return Animation::IntersectBox(box);
}

bool LevelSprite::IntersectPoint(POINT& pt)
{
  pt.x += -x_ + 256,
  pt.y += -y_ + 256;
  return Animation::IntersectPoint(pt);
}

void LevelSprite::GetRect(RECT& rect)
{
  rect = GetBoundingBox();
  OffsetRect(&rect,
    x_ - 256,
    y_ - 256
  );
}

bool LevelSprite::Move(int x, int y)
{
  x_ += x,
  y_ += y;
  return true;
}

bool LevelSprite::NextFrame(RedrawRegion& redraw)
{
  if (Animation::NextFrame()) {
    return Invalidate(redraw);
  }
  return false;
}

Ref<SpriteProperty> LevelSprite::GetProperty()
{
  return property_;
}

// Measure the current sprite table size
UINT SpriteTableSize(CartFile& cart, DWORD level_id)
{
  CartFile::Scope scope(cart);
  if (!cart.FollowPtr2(0x3E0000, level_id * 2)) {
    return 0;
  }
  UINT lvlspr_data_size = 0;
  do {
    WORD param;
    if (!cart.ReadWord(param)) {
      return 0;
    }
    lvlspr_data_size += 2;
    if (param == 0) {
      break ;
    }
    cart.Skip(6);
    lvlspr_data_size += 6;
  } while (1);

  return lvlspr_data_size;
}

LevelSpriteManager::LevelSpriteManager()
{
  level_id_ = 0;
}

LevelSpriteManager::~LevelSpriteManager()
{
}

bool LevelSpriteManager::Load(CartFile& cart, SpritePool& sprites,
  SpritePropertyPool& properties, UINT level_id)
{
  CartFile::Scope scope(cart);
  objects_.clear();
  if (!cart.FollowPtr2(0x3E0000, level_id * 2)) {
    return false;
  }
  do {
    Ref<LevelSprite> lvl_spr;
    lvl_spr = new LevelSprite(properties, sprites);
    if (!lvl_spr->Deserialize(cart)) {
      break ;
    }
    objects_.push_back(lvl_spr);
  } while (1);
  level_id_ = level_id;
  return true;
}

bool LevelSpriteManager::Save(CartFile& cart)
{
  CartFile::Scope scope(cart);
  if (!cart.FollowPtr2(0x3E0000, level_id_ * 2)) {
    return false;
  }
  UINT i;
  UINT old_table_size = SpriteTableSize(cart, level_id_);
  UINT current_table_size = objects_.size() * 8 + 2;
  int delta = current_table_size - old_table_size;
  for (i = level_id_ + 1; i < 256; i++) {
    UINT offset = 0x3E0000 + i * 2;
    WORD spr_table_addr;
    if (!cart.Read(offset, &spr_table_addr, 2)) {
      return false;
    }
    spr_table_addr += delta;
    //if (!cart.Write(offset, &spr_table_addr, 2)) {
    //  return false;
    //}
  }
  if (!cart.FollowPtr2(0x3E0000, level_id_ * 2)) {
    return false;
  }
  ObjectList::iterator it;
  for (it = objects_.begin(); it != objects_.end(); ++it) {
    Ref<LevelSprite> lvl_spr = *it;
    if (!cart.Writef("HHHH",
      &lvl_spr->param_, &lvl_spr->x_, &lvl_spr->y_, &lvl_spr->prop_id_))
    {
      return false;
    }
  }
  cart.WriteWord(0x0000);
  return true;
}

bool LevelSpriteManager::NextFrame(const RECT& pos, RedrawRegion& redraw)
{
  bool invalidated = false;
  ObjectList::iterator i;
  for (i = objects_.begin(); i != objects_.end(); ++i) {
    Ref<LevelSprite> obj = (*i);
    if (obj->NextFrame(redraw)) {
      invalidated = true;
    }
  }
  return invalidated;
}

const TCHAR *SpritePropWindow::ClassName = TEXT("SpriteProp");

SpritePropWindow::SpritePropWindow(CartFile& cart, LevelEditor& level_editor) :
  cart_(cart), level_editor_(level_editor)
{
}

SpritePropWindow::~SpritePropWindow()
{
}

bool SpritePropWindow::Create(LPCSTR lpWindowName, int x, int y,
  int width, int height)
{
  DWORD dwExStyles = WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW;
  DWORD dwStyles = WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_CLIPCHILDREN;
  if (!Window::Create(ClassName, lpWindowName, dwExStyles, dwStyles,
    x, y, width, height, NULL, &level_editor_)) {
    return false;
  }
  sprite_list_.Create(ID_SPRITELIST,
    NULL,
    WS_TABSTOP | WS_VISIBLE | LBS_NOINTEGRALHEIGHT |
    LBS_OWNERDRAWFIXED | LBS_NODATA | WS_VSCROLL,
    3, 245, width - 6, height - 245 - 6,
    *this
  );
  sprite_list_.SetDefaultFont();
  sprite_list_.SetCount(2100);
  Show();
  return true;
}

void SpritePropWindow::OnCreate()
{
  stock_sprite_.Load(cart_);
}

void SpritePropWindow::OnSize(WORD width, WORD height)
{
  sprite_list_.SetSize(width - 6, height - 6 - 245);
}

bool SpritePropWindow::OnDrawItem(DWORD id, DRAWITEMSTRUCT* item)
{
  if (id == ID_SPRITELIST) {
    const RECT& zone = item->rcItem;

    if (item->itemState & ODS_SELECTED) {
      SetBkColor(item->hDC, GetSysColor(COLOR_HIGHLIGHT));
      SetTextColor(item->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else {
      SetBkColor(item->hDC, RGB(255, 255, 255));
      SetTextColor(item->hDC, GetSysColor(COLOR_WINDOWTEXT));
    }
    ExtTextOut(item->hDC, 0, 0, ETO_OPAQUE,
      &zone, "", 0, NULL);
    if (item->itemState & ODS_SELECTED) {
      DrawFocusRect(item->hDC, &zone);
    }
    SpritePool& sprites = level_editor_.GetSprites();
    SpritePropertyPool& props = level_editor_.GetSpriteProps();
    Ref<SpriteProperty> prop = props.NewFromID(item->itemID);
    if (!prop) {
      return true;
    }
    Animation anim;
    if (!anim.LoadFromProperty(cart_, sprites, *(*prop))) {
      return false;
    }
    Ref<SpriteResource> sprite = anim.GetCurrentSprite();
    if (!sprite) {
      return false;
    }
    int width = 42,
        height = 42;
    sprite->DrawSized(item->hDC, zone.left + 3, zone.top + 3,
      width, height, 0, 0, true);
    std::string sprite_name;
    if (stock_sprite_.GetName(sprite_name, prop)) {

    }
    TextOut(item->hDC, zone.left + 124, zone.top + 3,
      sprite_name.data(), sprite_name.length()
    );
    return true;
  }
  return false;
}

bool SpritePropWindow::OnMesureItem(DWORD id, MEASUREITEMSTRUCT* mesure)
{
  if (id == ID_SPRITELIST) {
    mesure->itemHeight = 48;
    return true;
  }
  return false;
}

bool SpritePropWindow::Register()
{
  return Window::Register(ClassName, (HBRUSH) COLOR_WINDOW + 1);
}

LevelSpriteEditor::LevelSpriteEditor(EditorObjectManager<LevelSprite>& objects,
  CartFile& cart, LevelEditor& level_editor, MainFrame& frame) :
    EditorTool(objects, level_editor),
    cart_(cart), level_editor_(level_editor),
    frame_(frame),
    sprite_window_(cart, level_editor)
{
}

LevelSpriteEditor::~LevelSpriteEditor()
{
}

void LevelSpriteEditor::OnCreate()
{
  //frame_.AddMenus(kLevelSpriteMenus, kLevelSpriteMenuOffset);
}

bool LevelSpriteEditor::OnDestroy()
{
  //frame_.RemoveMenus(kLevelSpriteMenus, kLevelSpriteMenuOffset);
  return true;
}

void LevelSpriteEditor::OnMouseDoubleClick(DWORD key, int x, int y)
{
  ShowProperty();
}

void LevelSpriteEditor::ShowProperty()
{
  if (!prop_window_.IsValid()) {
    if (!prop_window_.Create("Sprite Property",
      CW_USEDEFAULT, CW_USEDEFAULT,
      268, 450, *this))
    {
      return ;
    }
  }
  {
    Ref<LevelSprite> selected_sprite = objects_.GetSelected();
    if (selected_sprite) {
      prop_window_.SetProperty(selected_sprite->GetProperty());
    }
  }

}

void LevelSpriteEditor::OnCommand(DWORD id, DWORD code)
{
  switch (id) {
  case ID_INSERT:
    sprite_window_.Create("Insert Sprite",
      CW_USEDEFAULT, CW_USEDEFAULT,
      200, 368);
    break ;
  case ID_PROPERTY:
    ShowProperty();
    break ;
  }
}
