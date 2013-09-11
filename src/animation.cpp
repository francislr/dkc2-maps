
#include "stdafx.h"

#include "animation.h"
#include "sprite.h"
#include "level_sprite.h"
#include "sprite_property.h"
#include "rom_file.h"

Animation::Animation()
{
  SetRect(&base_, 0, 0, 0, 0);
  frame_index_ = 0;
  frame_count_ = 0;
  counter_ = 0;
  flip_horz_ = false;
  flip_vert_ = false;
}


Animation::~Animation()
{
}

bool Animation::Draw(BitmapSurface& surface, int to_x,
    int to_y, int from_x, int from_y)
{
  if (frame_index_ >= frames_.size()) {
    return false;
  }
  Frame& frame = frames_[frame_index_];
  if (flip_vert_ || flip_horz_) {
    frame.sprite->DrawFlipped(surface,
      to_x,
      to_y,
      from_x, from_y,
      flip_horz_, flip_vert_
    );

  }
  else {
    frame.sprite->Draw(surface,
      to_x,
      to_y,
      from_x, from_y
    );
  }
  return true;
}

bool Animation::DrawDC(HDC hdc, int to_x,
    int to_y, int from_x, int from_y)
{
  Ref<SpriteResource> sprite = GetCurrentSprite();
  if (!sprite) {
    return false;
  }
  sprite->DrawDC(hdc,
    to_x,
    to_y,
    from_x, from_y
  );
  return true;
}

bool Animation::DrawSelection(BitmapSurface& surface, int x, int y)
{
  Ref<SpriteResource> sprite = GetCurrentSprite();
  if (!sprite) {
    return false;
  }
  RECT zone = base_;
  OffsetRect(&zone, x, y);
  surface.DrawSelection(zone,
    RGB(0, 0, 0),
    RGB(255, 255, 255),
    false
  );

  return true;
}

void Animation::SetFrame(UINT nFrame)
{
  if (frames_.size() == 0) {
    return ;
  }
  counter_ = nFrame % frames_.size();
}

bool Animation::NextFrame()
{
  if (frame_index_ >= frames_.size()) {
    return false;
  }
  Frame& frame = frames_[frame_index_];
  
  counter_++;

  if (counter_ >= frame.duration) {
    IncreaseFrame();
    return true;
  }
  return false;
}

bool Animation::Load(Buffer& src_buf, SpritePool& sprites,
  UINT anim_id, UINT pal_id)
{
  CartFile::Scope scope(src_buf);
  frames_.clear();
  frame_count_ = 0;
  anim_id_ = anim_id;
  if (!src_buf.FollowPtr2(0x390000, anim_id * 4)) {
    return false;
  }
  BYTE frame_duration;
  if (!src_buf.ReadByte(frame_duration)) {
    return false;
  }
  SetRect(&base_, 256, 256, -256, -256);
  while (frame_duration != 0x80) {
    UINT sprite_id = 0x00000000;
    if (!src_buf.Read(&sprite_id, 2)) {
      return false;
    }
    if (frame_duration > 0x80) {
      if (frame_duration == 0x8f) {
        src_buf.Skip(2);
        return true;
      }
      else if (frame_duration == 0x81) {
        //cart.Skip(2);

      }
      else if (frame_duration == 0x83) {
        //cart.Skip(2);

      }
      else if (frame_duration == 0x84) {
        //cart.Skip(2);

      }
      else if (frame_duration == 0x82) {
        //cart.Skip(2);

      }
      else {
      }
    }
    else {
      Frame frame;
      frame.duration = frame_duration;
      frame.sprite = sprites.New(MAKELONG(sprite_id, pal_id));
      if (!frame.sprite) {
        return false;
      }
      const RECT& sprite_rect = frame.sprite->GetRect();
      frame_count_ += frame.duration;
      if (base_.left > sprite_rect.left) {
        base_.left = sprite_rect.left;
      }
      if (base_.top > sprite_rect.top) {
        base_.top = sprite_rect.top;
      }
      if (base_.bottom < sprite_rect.bottom) {
        base_.bottom = sprite_rect.bottom;
      }
      if (base_.right < sprite_rect.right) {
        base_.right = sprite_rect.right;
      }
      frames_.push_back(frame);
    }
    if (!src_buf.ReadByte(frame_duration)) {
      return false;
    }
  }
  return true;
}

bool Animation::LoadStatic(Buffer& src_buf, SpritePool& sprites,
  UINT sprite_id, UINT pal_id)
{
  frames_.clear();
  Frame frame;
  frame.duration = 0;
  frame.sprite = sprites.New(MAKELONG(sprite_id, pal_id));
  if (!frame.sprite) {
    return false;
  }
  frames_.push_back(frame);
  base_ = frame.sprite->GetRect();
  return true;
}

bool Animation::LoadFromProperty(Buffer& src_buf, SpritePool& sprites,
  SpriteProperty& prop)
{
  SpriteGraphicDef gfx;
  if (prop.GetGraphics(src_buf, gfx)) {
    flip_horz_ = gfx.flip_horz;
    flip_vert_ = gfx.flip_vert;
    if (gfx.anim_id) {
      if (!Load(src_buf, sprites,
        gfx.anim_id, gfx.palette_id))
      {
        return false;
      }
      return true;
    }
    else if (gfx.sprite_id) {
      if (!LoadStatic(src_buf, sprites, gfx.sprite_id, gfx.palette_id)) {
        return false;
      }
      return true;
    }
  }
  return false;
}

bool Animation::IntersectBox(RECT& box)
{
  Ref<SpriteResource> sprite = GetCurrentSprite();
  if (!sprite) {
    return false;
  }
  RECT dstRect;
  return IntersectRect(&dstRect, &base_, &box) != FALSE;
}

bool Animation::IntersectPoint(POINT& pt)
{
  return base_.left <= pt.x && base_.right >= pt.x &&
    base_.top <= pt.y && base_.bottom >= pt.y;
}


const RECT& Animation::GetBoundingBox() const
{
  return base_;
}

Ref<SpriteResource> Animation::GetCurrentSprite()
{
  Frame& frame = GetCurrentFrame();
  if (!frame.sprite) {
    return Ref<SpriteResource>();
  }
  return frame.sprite;
}

void Animation::UpdateFrameCount()
{
  frame_count_ = 0;
  std::vector<Frame>::iterator i;
  for (i = frames_.begin(); i != frames_.end(); ++i) {
    frame_count_ += i->duration;
  }
}

void Animation::IncreaseFrame()
{
  frame_index_++;
  counter_ = 0;
  if (frame_index_ >= frames_.size()) {
    frame_index_ = 0;
  }
}

Animation::Frame& Animation::GetCurrentFrame()
{
  if (frame_index_ >= frames_.size()) {
    return Frame();
  }
  return frames_[frame_index_];
}

AnimationPool::AnimationPool(CartFile& cart,
  SpritePool& sprites) :
    cart_(cart), sprites_(sprites)
{
}

bool AnimationPool::OnResourceAlloc(DWORD id,
  AnimationRef* resource)
{
  return resource->Load(cart_, sprites_, LOWORD(id), HIWORD(id));
}
