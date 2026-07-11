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

  stbi_write_png(argv[1], FB_WIDTH, FB_HEIGHT, sizeof(uint32_t), fb, FB_WIDTH * sizeof(uint32_t));
  free(fb);
}
