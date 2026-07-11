#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "%s <output_path>.png\n", argv[0]);
    return EXIT_FAILURE;
  }

  FILE* fp = fopen(argv[1], "wb+");
  if(fp == NULL) {
    fprintf(stderr, "failed to open file at %s\n", argv[1]);
    return 1;
  }

  fclose(fp);
}
