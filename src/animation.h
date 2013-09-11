
#pragma once

#include <vector>

#include "ref_count.h"
#include "bitmap.h"
#include "sprite.h"

class SpritePool;
class SpriteResource;
class SpriteProperty;
class CartFile;
class Buffer;

class Animation
{
public:
  Animation();
  ~Animation();

  /* Draw the current sprite frame on a surface */
  bool Draw(BitmapSurface& surface, int to_x,
    int to_y, int from_x, int from_y);
  bool DrawDC(HDC hdc, int to_x,
    int to_y, int from_x, int from_y);

  bool DrawSelection(BitmapSurface& surface, int x, int y);

  void SetFrame(UINT nFrame);
  bool NextFrame();

  bool Load(Buffer& src_buf, SpritePool& sprites,
    UINT anim_id, UINT pal_id);
  bool LoadStatic(Buffer& src_buf, SpritePool& sprites,
    UINT sprite_id, UINT pal_id);

  /* Load an animation from a sprite graphics */
  bool LoadFromProperty(Buffer& src_buf, SpritePool& sprites,
    SpriteProperty& prop);

  Ref<SpriteResource> GetCurrentSprite();
  virtual bool IntersectBox(RECT& box);
  virtual bool IntersectPoint(POINT& pt);
  const RECT& GetBoundingBox() const;

private:
  bool flip_horz_;
  bool flip_vert_;
  /* Calculates the frame count */
  void UpdateFrameCount();

  void IncreaseFrame();

  UINT anim_id_;

  /* Current sprite index to display */
  UINT frame_index_;

  UINT counter_;

  /* Count of frames */
  UINT frame_count_;

  typedef struct {
    UINT duration; // Duration of this frame
    Ref<SpriteResource> sprite; // Sprite to display
  } Frame;

  RECT base_;
  Frame& GetCurrentFrame();
  std::vector<Frame> frames_;
};

class AnimationRef :
  public Animation, public RefCount
{
public:
};

class AnimationPool :
  ResourcePool<AnimationRef>
{
public:
  AnimationPool(CartFile& cart, SpritePool& sprites);

  virtual bool OnResourceAlloc(DWORD id, AnimationRef* resource);

private:
  CartFile& cart_;
  SpritePool& sprites_;
};
