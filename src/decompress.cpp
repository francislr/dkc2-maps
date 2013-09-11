
#include "stdafx.h"

#include "rom_file.h"

void FillBytes(Buffer& buffer, BYTE byte, UINT times)
{
  for (UINT i = 0; i < times; i++) {
    buffer.WriteByte(byte);
  }
}

int RareDecompress(Buffer& dst, const Buffer& src)
{
  Buffer::Scope scope(src);
  dst.Seek(0);
  src.Skip(1);
  BYTE common[6];
  src.Read(common, 6);

  src.Skip(32);
  BYTE d, t, b, b1, b2;
  UINT times, diff, y, pos;
  src.ReadByte(d);
  t = (d & 0xF0) >> 2;  // $4E
  while (1)
  {
    switch (t)
    {
    case 0x10:
      times = (d & 0x0F) + 3;
      FillBytes(dst, common[0], times);
      if (!src.ReadByte(d)) {
        return -1;
      }
      t = (d & 0xF0) >> 2;
      break ;
    case 0x08:
      src.ReadByte(b);
      dst.WriteByte((b / 0x10) | ((d % 0x10) << 4));
      src.ReadByte(d);
      dst.WriteByte(((b % 0x10) << 4) | (d / 0x10));
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x4F:
      src.ReadByte(d);
      times = (d / 0x10) + 3;
      FillBytes(dst, common[0], times);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x3F:
      src.ReadByte(d);
      times = (d / 0x10);
      if (times == 0) {
        return dst.GetPosition();
      }
      for (UINT i = 0; i < times; i++) {
        src.ReadByte(b);
        dst.WriteByte( ((d % 0x10) << 4) | (b / 0x10) );
        d = b;
      }
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x43:
      src.ReadByte(d);
      dst.WriteByte(d);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x0C: /* NU */
      times = (d & 0x0F) + 3;
      src.ReadByte(d);
      FillBytes(dst, d, times);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x04:
      src.ReadByte(b);
      dst.WriteByte(((d % 0x10) << 4) | (b / 0x10));
      d = b;
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x73:
      dst.Skip(-1);
      dst.ReadByte(b);
      dst.WriteByte(b);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x4B: /* NU */
      src.ReadByte(d);
      times = d / 0x10 + 0x03;
      src.ReadByte(b);
      FillBytes(dst, ((d % 0x10) << 4) | (b / 0x10), times);
      d = b;
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x34:
      dst.Skip(-1);
      dst.ReadByte(b);
      dst.WriteByte(b);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x57:
      dst.WriteByte(common[4]);
      dst.WriteByte(common[5]);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x28:
      times = (d & 0x0F) + 0x03;
      src.ReadByte(b);
      pos = dst.GetPosition() - times - b;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(dst[pos]);
        pos++;
      }
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x47:
      src.ReadByte(d);
      dst.WriteByte(d);
      src.ReadByte(d);
      dst.WriteByte(d);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x24:
      pos = dst.GetPosition() - ((d & 0x0F) + 0x02);
      dst.WriteByte(dst[pos]);
      pos++;
      dst.WriteByte(dst[pos]);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x63:
      src.ReadByte(d);
      pos = dst.GetPosition() - ((d / 0x10) + 0x02);
      dst.WriteByte(dst[pos]);
      pos++;
      dst.WriteByte(dst[pos]);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x1C:
      dst.WriteByte(common[2]);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x3C:
      pos = ((d & 0x0F) << 1) + 0x07;
      pos += scope.GetPosition();
      dst.WriteByte(src[pos]);
      pos++;
      dst.WriteByte(src[pos]);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x20:
      dst.WriteByte(common[3]);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x7B:
      src.ReadByte(d);
      pos = ((d & 0xF0) >> 3) + 0x07;
      pos += scope.GetPosition();
      dst.WriteByte(src[pos]);
      pos++;
      dst.WriteByte(src[pos]);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x5F:
      dst.WriteByte(common[3]);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x00:
      times = d & 0x0F;
      if (times == 0) {
        return dst.GetPosition();
      }
      for (UINT i = 0; i < times; i++) {
        src.ReadByte(d);
        dst.WriteByte(d);
      }
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x67:
      src.ReadByte(b);
      times = (b / 16 + 0x03);
      src.ReadByte(d);
      diff = times + (((b % 16) << 4) | (d / 16));
      pos = dst.GetPosition() - diff;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(dst[pos]);
        pos++;
      }
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x5B:
    {
      dst.WriteByte(common[2]);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    }
    case 0x18:
      dst.WriteByte(common[4]);
      dst.WriteByte(common[5]);
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x6B:
      src.ReadByte(d);
      times = ((d / 0x10) + 0x03);
      src.ReadByte(b);
      diff = (((d & 0x0F) << 8) | b) + 0x0103;
      pos = dst.GetPosition() - diff;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(dst[pos]);
        pos++;
      }
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x2C:
      times = (d & 0x0F) + 0x03;
      src.ReadByte(b);
      y = b;
      src.ReadByte(d);
      pos = dst.GetPosition() - ((((y << 8) | d) >> 4) + 0x0103);
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(dst[pos]);
        pos++;
      }
      t = ((d & 0x0F) << 2) + 0x3F;
    break ;
    case 0x14:
      times = (d & 0x0F) + 0x03;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(common[1]);
      }
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x53: /* NU */
      src.ReadByte(d);
      times = (d / 0x10) + 0x03;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(common[1]);
      }
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x6F:
      src.ReadByte(d);
      times = (d / 0x10) + 0x03;
      src.ReadByte(b);
      y = b;
      diff = (((d << 8) | y) << 4) & 0xffff;
      src.ReadByte(d);
      diff |= (d / 0x10) & 0xff;
      pos = dst.GetPosition() - diff;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(dst[pos]);
        pos++;
      }
      t = ((d & 0x0F) << 2) + 0x3F;
      break ;
    case 0x77: /* NU */
      pos = dst.GetPosition();
      dst.WriteByte(dst[pos - 2]);
      dst.WriteByte(dst[pos - 1]);
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    case 0x38:
      pos = dst.GetPosition();
      dst.WriteByte(dst[pos - 2]);
      dst.WriteByte(dst[pos - 1]);
      t = ((d & 0x0F) << 2) + 0x3F;
    break ;
    case 0x30:
      times = (d & 0x0F) + 0x03;
      src.ReadByte(b1);
      src.ReadByte(b2);
      y = (b1 << 8) | b2;
      pos = dst.GetPosition() - y;
      for (UINT i = 0; i < times; i++) {
        dst.WriteByte(dst[pos]);
        pos++;
      }
      src.ReadByte(d);
      t = (d & 0xF0) >> 2;
      break ;
    default:
      return -1;
    }
  }
  return -1;
}