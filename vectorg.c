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
  struct Command* next;
};

struct Command* command_new(enum Command_Type type, struct Command* next)
{
  struct Command* cmd = aligned_alloc(_Alignof(struct Command), sizeof(struct Command));
  if (cmd == NULL)
  {
    return NULL;
  }

  cmd->next = next;
  cmd->type = type;
  return cmd;
}

struct Command* command_move_to(struct Vec2 move_to, struct Command* next)
{
  struct Command* cmd = command_new(Command_Type_Move_To, next);
  if (cmd == NULL)
  {
    return NULL;
  }

  cmd->move_to = move_to;
  return cmd;
}

struct Command* command_line_to(struct Vec2 line_to, struct Command* next)
{
  struct Command* cmd = command_new(Command_Type_Line_To, next);
  if (cmd == NULL)
  {
    return NULL;
  }

  cmd->line_to = line_to;
  return cmd;
}

void path_drop(struct Command* curr_cmd)
{
  do
  {
    struct Command* tmp = curr_cmd->next;
    free(curr_cmd);
    curr_cmd = tmp;
  } while (curr_cmd != NULL);
}

struct Mesh
{
  struct Vec2* vertices;
  size_t vertices_len;
  size_t vertices_cap;

  uint32_t* indices;
  size_t indices_len;
  size_t indices_cap;
};

#define mesh_new() ((struct Mesh){NULL, 0, 0, NULL, 0, 0})

size_t next_cap(size_t current_cap, size_t min_cap)
{
  size_t next_cap = current_cap > 0 ? current_cap : 4;
  while (next_cap <= min_cap)
  {
    next_cap *= 2;
  }
  return next_cap;
}

bool mesh_append_vertex(struct Mesh* mesh, struct Vec2 vertex)
{
  if (mesh->vertices_len + 1 > mesh->vertices_cap)
  {
    size_t new_vertices_cap = next_cap(mesh->vertices_cap, mesh->vertices_len + 1);
    struct Vec2* new_vertices = realloc(mesh->vertices, new_vertices_cap * sizeof(struct Vec2));
    if (new_vertices == NULL)
    {
      return false;
    }
    mesh->vertices = new_vertices;
  }

  mesh->vertices[mesh->vertices_len] = vertex;
  mesh->vertices_len += 1;
  return true;
}

bool mesh_append_index(struct Mesh* mesh, uint32_t index)
{
  if (mesh->indices_len + 1 > mesh->indices_cap)
  {
    size_t new_indices_cap = next_cap(mesh->indices_cap, mesh->indices_len + 1);
    uint32_t* new_indices = realloc(mesh->indices, new_indices_cap * sizeof(uint32_t));
    if (new_indices == NULL)
    {
      return false;
    }
    mesh->indices = new_indices;
  }

  mesh->indices[mesh->indices_len] = index;
  mesh->indices_len += 1;
  return true;
}

void mesh_drop(struct Mesh* mesh)
{
  free(mesh->vertices);
  free(mesh->indices);
  *mesh = (struct Mesh){0};
}

struct Command* box_to(struct Vec2 top_left, struct Vec2 bottom_right)
{
  struct Command* cmd = command_new(Command_Type_Close, NULL);
  assert(cmd != NULL);
  cmd = command_line_to(vec2_new(top_left.x, bottom_right.y), cmd);
  assert(cmd != NULL);
  cmd = command_line_to(bottom_right, cmd);
  assert(cmd != NULL);
  cmd = command_line_to(vec2_new(bottom_right.x, top_left.y), cmd);
  assert(cmd != NULL);
  cmd = command_move_to(top_left, cmd);
  assert(cmd != NULL);

  return cmd;
}

struct Mesh tessellate(struct Command* cmds)
{
  struct Mesh mesh = mesh_new();

  for (struct Command* cmd = cmds; cmd != NULL; cmd = cmd->next)
  {
      
  }
}

void draw(uint32_t* fb, struct Mesh mesh) {}

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

  struct Command* box = box_to(vec2_new(400, 500), vec2_new(800, 900));

  struct Mesh box_mesh = tessellate(box);
  draw(fb, box_mesh);

  mesh_drop(&box_mesh);
  path_drop(box);

  stbi_write_png(argv[1], FB_WIDTH, FB_HEIGHT, sizeof(uint32_t), fb, FB_WIDTH * sizeof(uint32_t));
  free(fb);
}
