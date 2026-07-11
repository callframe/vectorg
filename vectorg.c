#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#if !defined(FB_HEIGHT)
#define FB_HEIGHT 1080
#endif

#if !defined(FB_WIDTH)
#define FB_WIDTH 1920
#endif

struct Vec2
{
  float x;
  float y;
};

#define vec2_new(x, y) ((struct Vec2){x, y})

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

struct Mesh
{
  struct Vec2_Array vertices;
  struct UInt32_Array indices;
};

#define mesh_new() ((struct Mesh){vec2_array_new(), uint32_array_new()})

void mesh_drop(struct Mesh* mesh)
{
  vec2_array_drop(&mesh->vertices);
  uint32_array_drop(&mesh->indices);
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

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "%s <output_path>.png\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint32_t* fb = aligned_alloc(_Alignof(uint32_t), sizeof(uint32_t) * (FB_WIDTH * FB_HEIGHT));
  if (fb == NULL)
  {
    fprintf(stderr, "failed to allocate framebuffer of size %dx%d\n", FB_WIDTH, FB_HEIGHT);
    return 1;
  }

  struct Command_Array box = {0};
  if(!box_to(vec2_new(400, 500), vec2_new(800, 900), &box)) {
    fprintf(stderr, "failed to construct commands for box\n");
    free(fb);
    return 1;
  }

  command_array_drop(&box);

  stbi_write_png(argv[1], FB_WIDTH, FB_HEIGHT, sizeof(uint32_t), fb, FB_WIDTH * sizeof(uint32_t));
  free(fb);
}
