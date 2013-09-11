
#pragma once

#include <vector>

#include "ref_count.h"
#include "hexedit.h"

class CartFile;
class Buffer;
class SpritePropertyPool;

struct SpriteGraphicDef
{
  WORD palette_id;
  WORD anim_id;
  WORD sprite_id;
  bool flip_horz;
  bool flip_vert;
};

class SpriteProperty :
  public RefCount
{
public:
  SpriteProperty();
  ~SpriteProperty();
  typedef std::vector<Ref<SpriteProperty> > SubPropList;

  bool Load(CartFile& cart, SpritePropertyPool& props,
    DWORD addr);
  bool IsRoot() const;
  bool GetGraphics(Buffer& src_buf, SpriteGraphicDef& gfx);
  BYTE GetByte(UINT index) const;
  WORD GetWord(UINT index) const;
  DWORD GetDword(UINT index) const;
  bool GetSubProperty(WORD op, SpriteProperty& prop) const;
  SubPropList& GetSubPropList();
  void GetPropName(std::string& name);
  bool GetPropData(BYTE **prop_data, int& prop_size);
  WORD GetOp() const { return op_; }

private:
  bool LoadSubProp(int recursion, CartFile& cart,
    SpritePropertyPool& props, DWORD addr);
  bool LoadSubProp(int recursion, CartFile& cart,
    SpritePropertyPool& props);
  static int GetPropSize(WORD op);

  SubPropList properties_;

  DWORD addr_;
  WORD op_;
  bool root_prop_;

  union {
    BYTE bytes[32];
    WORD words[16];
    DWORD dwords[8];
  } data_;
};

class SpritePropertyPool :
  public ResourcePool<SpriteProperty>
{
public:
  SpritePropertyPool(CartFile& cart);

  /* Called when a sprite is allocated */
  virtual bool OnResourceAlloc(DWORD id, SpriteProperty* prop);
  Ref<SpriteProperty> NewFromID(DWORD id);
  Ref<SpriteProperty> NewFromID2(DWORD id);

private:
  CartFile& cart_;
};

class SpritePropertyList :
  public std::vector<Ref<SpriteProperty> >
{
public:
  SpritePropertyList();
  ~SpritePropertyList();

  bool Load(CartFile& cart, SpritePropertyPool& props);
};

class EditPropWindow :
  public Window
{
public:
  EditPropWindow();
  ~EditPropWindow();

  bool Create(LPCSTR lpWindowName, int x, int y,
    int width, int height, Window& parent);
  bool SetProperty(Ref<SpriteProperty> prop);
  bool InsertTreeProp(HTREEITEM hTreeItem, SpriteProperty& prop);
  virtual void OnCreate();
  void OnSelectProperty();
  void OnSize(WORD width, WORD height);

  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  static bool Register();

private:
  Window prop_name_;
  HexEdit hex_edit_;
  Ref<SpriteProperty> prop_;
  Ref<SpriteProperty> selected_prop_;
  TreeView properties_;
  static const TCHAR *ClassName;
};
