#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#if !defined(FB_HEIGHT)
#define FB_HEIGHT 1080
#endif

#if !defined(FB_WIDTH)
#define FB_WIDTH 1920
#endif

#define PI 3.14159265358979323846f

#define TOLERANCE 0.25f

struct Vec2
{
  float x;
  float y;
};

#define vec2_new(x, y) ((struct Vec2){x, y})

struct Vec2 vec2_sub(struct Vec2 a, struct Vec2 b) { return vec2_new(a.x - b.x, a.y - b.y); }
float vec2_cross(struct Vec2 a, struct Vec2 b) { return (a.x * b.y) - (a.y * b.x); }

#define __ARRAY_NAME Vec2_Array
#define __ARRAY_FUNCS_NAME vec2_array
#define __ARRAY_T struct Vec2
#include "array.c"

#define __ARRAY_NAME UInt32_Array
#define __ARRAY_FUNCS_NAME uint32_array
#define __ARRAY_T uint32_t
#include "array.c"

enum Command_Type
{
  Command_Type_Move_To,
  Command_Type_Line_To,
  Command_Type_Close,
};

struct Command
{
  enum Command_Type type;
  union
  {
    struct Vec2 move_to;
    struct Vec2 line_to;
  };
};

#define __ARRAY_NAME Command_Array
#define __ARRAY_FUNCS_NAME command_array
#define __ARRAY_T struct Command
#include "array.c"

struct Command command_move_to(struct Vec2 move_to)
{
  return (struct Command){.type = Command_Type_Move_To, .move_to = move_to};
}

struct Command command_line_to(struct Vec2 line_to)
{
  return (struct Command){.type = Command_Type_Line_To, .line_to = line_to};
}

bool box_to(struct Vec2 top_left, struct Vec2 bottom_right, struct Command_Array* cmds)
{
  struct Command cmd = command_move_to(top_left);
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  cmd = command_line_to(vec2_new(bottom_right.x, top_left.y));
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  cmd = command_line_to(bottom_right);
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  cmd = command_line_to(vec2_new(top_left.x, bottom_right.y));

  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  cmd = (struct Command){.type = Command_Type_Close};
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  return true;
}

bool circle_to(struct Vec2 center, float radius, struct Command_Array* cmds)
{
  size_t segments;
  if (radius <= TOLERANCE)
  {
    segments = 8;
  }
  else
  {
    float arg = 1.0f - TOLERANCE / radius;
    if (arg < -1.0f)
    {
      arg = -1.0f;
    }

    segments = (size_t)ceilf(PI / acosf(arg));
    if (segments < 8)
    {
      segments = 8;
    }
  }

  float step = 2.0f * PI / (float)segments;
  for (size_t i = 0; i < segments; ++i)
  {
    float angle = step * (float)i;
    struct Vec2 point = vec2_new(center.x + radius * cosf(angle), center.y + radius * sinf(angle));

    struct Command cmd = (i == 0) ? command_move_to(point) : command_line_to(point);
    if (!command_array_push(cmds, cmd))
    {
      return false;
    }
  }

  struct Command cmd = (struct Command){.type = Command_Type_Close};
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  return true;
}

bool ellipse_to(struct Vec2 center, float rx, float ry, struct Command_Array* cmds)
{
  float radius = fmaxf(rx, ry);

  size_t segments;
  if (radius <= TOLERANCE)
  {
    segments = 8;
  }
  else
  {
    float arg = 1.0f - TOLERANCE / radius;
    if (arg < -1.0f)
    {
      arg = -1.0f;
    }

    segments = (size_t)ceilf(PI / acosf(arg));
    if (segments < 8)
    {
      segments = 8;
    }
  }

  float step = 2.0f * PI / (float)segments;
  for (size_t i = 0; i < segments; ++i)
  {
    float angle = step * (float)i;
    struct Vec2 point = vec2_new(center.x + rx * cosf(angle), center.y + ry * sinf(angle));

    struct Command cmd = (i == 0) ? command_move_to(point) : command_line_to(point);
    if (!command_array_push(cmds, cmd))
    {
      return false;
    }
  }

  struct Command cmd = (struct Command){.type = Command_Type_Close};
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  return true;
}

bool polygon_to(struct Vec2 center, float radius, size_t sides, float rotation,
                struct Command_Array* cmds)
{
  if (sides < 3)
  {
    sides = 3;
  }

  float step = 2.0f * PI / (float)sides;
  for (size_t i = 0; i < sides; ++i)
  {
    float angle = rotation + step * (float)i;
    struct Vec2 point = vec2_new(center.x + radius * cosf(angle), center.y + radius * sinf(angle));

    struct Command cmd = (i == 0) ? command_move_to(point) : command_line_to(point);
    if (!command_array_push(cmds, cmd))
    {
      return false;
    }
  }

  struct Command cmd = (struct Command){.type = Command_Type_Close};
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  return true;
}

bool star_to(struct Vec2 center, float outer_radius, float inner_radius, size_t points,
             float rotation, struct Command_Array* cmds)
{
  if (points < 2)
  {
    points = 2;
  }

  size_t vertices = points * 2;
  float step = PI / (float)points;
  for (size_t i = 0; i < vertices; ++i)
  {
    float radius = (i % 2 == 0) ? outer_radius : inner_radius;
    float angle = rotation + step * (float)i;
    struct Vec2 point = vec2_new(center.x + radius * cosf(angle), center.y + radius * sinf(angle));

    struct Command cmd = (i == 0) ? command_move_to(point) : command_line_to(point);
    if (!command_array_push(cmds, cmd))
    {
      return false;
    }
  }

  struct Command cmd = (struct Command){.type = Command_Type_Close};
  if (!command_array_push(cmds, cmd))
  {
    return false;
  }

  return true;
}

struct Contour
{
  struct Vec2_Array vertices;
  bool is_closed;
};

#define contour_new() ((struct Contour){vec2_array_new(), false})

void contour_drop(struct Contour* contour)
{
  vec2_array_drop(&contour->vertices);
  *contour = contour_new();
}

// TODO: handle sub-paths
bool contour_for(struct Command_Array cmds, struct Contour* contour)
{
  assert(contour->vertices.len == 0);
  struct Vec2 cursor = vec2_new(0, 0);

  for (size_t i = 0; i < cmds.len; ++i)
  {
    struct Command const* cmd = command_array_at(cmds, i);
    switch (cmd->type)
    {
      case Command_Type_Move_To:
        cursor = cmd->move_to;
        break;
      case Command_Type_Line_To:
        cursor = cmd->line_to;
        break;
      case Command_Type_Close:
        contour->is_closed = true;
        goto exit;
    }

    if (!vec2_array_push(&contour->vertices, cursor))
    {
      return false;
    }
  }

exit:
  return true;
}

float get_winding(struct Contour contour)
{
  float sum = 0.0f;

  size_t num_vertices = contour.vertices.len;
  for (size_t i = 0; i < num_vertices; ++i)
  {
    struct Vec2 v1 = *vec2_array_at(contour.vertices, i);
    struct Vec2 v2 = *vec2_array_at(contour.vertices, (i + 1) % num_vertices);
    sum += vec2_cross(v1, v2);
  }

  return sum;
}

float get_orientation(struct Vec2 prev, struct Vec2 curr, struct Vec2 next)
{
  struct Vec2 v1 = vec2_sub(curr, prev);
  struct Vec2 v2 = vec2_sub(next, curr);
  return vec2_cross(v1, v2);
}

bool is_convex(float winding, struct Vec2 prev, struct Vec2 curr, struct Vec2 next)
{
  float orientation = get_orientation(prev, curr, next);
  return (winding * orientation) > 0.0f;
}

bool is_in_triangle(struct Vec2 p, struct Vec2 a, struct Vec2 b, struct Vec2 c)
{
  float o1 = get_orientation(a, b, p);
  float o2 = get_orientation(b, c, p);
  float o3 = get_orientation(c, a, p);

  bool all_pos = o1 > 0.0f && o2 > 0.0f && o3 > 0.0f;
  bool all_neg = o1 < 0.0f && o2 < 0.0f && o3 < 0.0f;

  return all_pos || all_neg;
}

struct Indexed_Vec2
{
  struct Vec2 pos;
  size_t index;
};

#define indexed_vec2_new(pos, index) ((struct Indexed_Vec2){pos, index})

#define __ARRAY_NAME Indexed_Vec2_Array
#define __ARRAY_FUNCS_NAME indexed_vec2_array
#define __ARRAY_T struct Indexed_Vec2
#include "array.c"

bool indexed_vertices_from_vertices(struct Vec2_Array vertices,
                                    struct Indexed_Vec2_Array* out_indexed)
{
  if (!indexed_vec2_array_reserve(out_indexed, vertices.len))
  {
    return false;
  }

  for (size_t i = 0; i < vertices.len; ++i)
  {
    struct Vec2 pos = *vec2_array_at(vertices, i);
    indexed_vec2_array_push(out_indexed, indexed_vec2_new(pos, i));
  }

  return true;
}

// TOOD: currently uses shift-remove, instead use linked list
bool triangulate(struct Contour contour, float winding, struct UInt32_Array* indices)
{
  struct Indexed_Vec2_Array indexed_vertices = {0};
  if (!indexed_vertices_from_vertices(contour.vertices, &indexed_vertices))
  {
    return false;
  }

  size_t i = 0;
  while (indexed_vertices.len > 2)
  {
    struct Indexed_Vec2 curr = *indexed_vec2_array_at(indexed_vertices, i);

    size_t prev_pos = (i + indexed_vertices.len - 1) % indexed_vertices.len;
    struct Indexed_Vec2 prev = *indexed_vec2_array_at(indexed_vertices, prev_pos);

    size_t next_pos = (i + 1) % indexed_vertices.len;
    struct Indexed_Vec2 next = *indexed_vec2_array_at(indexed_vertices, next_pos);

    if (!is_convex(winding, prev.pos, curr.pos, next.pos))
    {
      i = (i + 1) % indexed_vertices.len;
      continue;
    }

    bool is_blocked = false;
    for (size_t j = 0; j < indexed_vertices.len; ++j)
    {
      if (j == prev_pos || j == i || j == next_pos)
      {
        continue;
      }

      struct Indexed_Vec2 point = *indexed_vec2_array_at(indexed_vertices, j);
      if (is_in_triangle(point.pos, prev.pos, curr.pos, next.pos))
      {
        is_blocked = true;
        break;
      }
    }

    if (is_blocked)
    {
      i = (i + 1) % indexed_vertices.len;
      continue;
    }

    indexed_vec2_array_remove(&indexed_vertices, i, NULL);
    if (!uint32_array_reserve(indices, 3))
    {
      indexed_vec2_array_drop(&indexed_vertices);
      return false;
    }

    uint32_array_push(indices, prev.index);
    uint32_array_push(indices, curr.index);
    uint32_array_push(indices, next.index);
  }

  indexed_vec2_array_drop(&indexed_vertices);
  return true;
}

uint32_t triangle_color(size_t i)
{
  uint32_t h = (uint32_t)(i + 1) * 2654435761u;
  uint8_t r = 0x40 | (h & 0xFF);
  uint8_t g = 0x40 | ((h >> 8) & 0xFF);
  uint8_t b = 0x40 | ((h >> 16) & 0xFF);
  return 0xFF000000u | (b << 16) | (g << 8) | r;
}

void draw_indexed(uint32_t* fb, struct Vec2_Array vertices, struct UInt32_Array indices)
{
  for (size_t r = 0; r + 2 < indices.len; r += 3)
  {
    struct Vec2 a = *vec2_array_at(vertices, *uint32_array_at(indices, r + 0));
    struct Vec2 b = *vec2_array_at(vertices, *uint32_array_at(indices, r + 1));
    struct Vec2 c = *vec2_array_at(vertices, *uint32_array_at(indices, r + 2));

    int32_t minx = (int32_t)floorf(fminf(a.x, fminf(b.x, c.x)));
    int32_t maxx = (int32_t)ceilf(fmaxf(a.x, fmaxf(b.x, c.x)));
    int32_t miny = (int32_t)floorf(fminf(a.y, fminf(b.y, c.y)));
    int32_t maxy = (int32_t)ceilf(fmaxf(a.y, fmaxf(b.y, c.y)));

    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx > FB_WIDTH - 1) maxx = FB_WIDTH - 1;
    if (maxy > FB_HEIGHT - 1) maxy = FB_HEIGHT - 1;

    uint32_t color = triangle_color(r);
    for (int32_t y = miny; y <= maxy; ++y)
    {
      for (int32_t x = minx; x <= maxx; ++x)
      {
        struct Vec2 p = vec2_new(x + 0.5f, y + 0.5f);
        if (is_in_triangle(p, a, b, c))
        {
          fb[y * FB_WIDTH + x] = color;
        }
      }
    }
  }
}

bool render_shape(uint32_t* fb, struct Command_Array cmds)
{
  struct Contour contour = contour_new();
  if (!contour_for(cmds, &contour))
  {
    fprintf(stderr, "failed to compute contour for shape\n");
    contour_drop(&contour);
    return false;
  }

  float winding = get_winding(contour);
  struct UInt32_Array indices = uint32_array_new();

  if (!triangulate(contour, winding, &indices))
  {
    fprintf(stderr, "failed to triangulate the shape\n");
    uint32_array_drop(&indices);
    contour_drop(&contour);
    return false;
  }

  draw_indexed(fb, contour.vertices, indices);

  uint32_array_drop(&indices);
  contour_drop(&contour);
  return true;
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "%s <output_path>.png\n", argv[0]);
    return EXIT_FAILURE;
  }

  size_t fb_size = sizeof(uint32_t) * (FB_WIDTH * FB_HEIGHT);
  uint32_t* fb = aligned_alloc(_Alignof(uint32_t), fb_size);
  if (fb == NULL)
  {
    fprintf(stderr, "failed to allocate framebuffer of size %dx%d\n", FB_WIDTH, FB_HEIGHT);
    return 1;
  }

  memset(fb, 0xff, fb_size);

  struct Command_Array box = {0};
  struct Command_Array circle = {0};
  struct Command_Array ellipse = {0};
  struct Command_Array hexagon = {0};
  struct Command_Array star = {0};

  bool ok = box_to(vec2_new(150, 500), vec2_new(550, 900), &box) &&
            circle_to(vec2_new(950, 700), 200, &circle) &&
            ellipse_to(vec2_new(1500, 700), 300, 150, &ellipse) &&
            polygon_to(vec2_new(400, 250), 180, 6, -PI / 2.0f, &hexagon) &&
            star_to(vec2_new(1100, 250), 200, 80, 5, -PI / 2.0f, &star);
  if (!ok)
  {
    fprintf(stderr, "failed to construct commands for a shape\n");
    command_array_drop(&star);
    command_array_drop(&hexagon);
    command_array_drop(&ellipse);
    command_array_drop(&circle);
    command_array_drop(&box);
    free(fb);
    return 1;
  }

  ok = render_shape(fb, box) && render_shape(fb, circle) && render_shape(fb, ellipse) &&
       render_shape(fb, hexagon) && render_shape(fb, star);

  command_array_drop(&star);
  command_array_drop(&hexagon);
  command_array_drop(&ellipse);
  command_array_drop(&circle);
  command_array_drop(&box);

  if (!ok)
  {
    free(fb);
    return 1;
  }

  stbi_write_png(argv[1], FB_WIDTH, FB_HEIGHT, sizeof(uint32_t), fb, FB_WIDTH * sizeof(uint32_t));
  free(fb);
}
