#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char *s, ...) {
  va_list args;
  va_start(args, s);
  vfprintf(stderr, s, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}

void read_png_file(const char *file_name) {}

class PngError {};

class PngImage {
public:
  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  int number_of_passes;
  png_bytep *row_pointers;

  PngImage(const char *filename) : row_pointers(0) {
    png_byte header[8];

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
      throw PngError();
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
      throw PngError();
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
      throw PngError();
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      throw PngError();
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      throw PngError();
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
      throw PngError();
    }

    row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
      row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    png_read_image(png_ptr, row_pointers);

    fclose(fp);
  }

  bool sampleBool(int x, int y) {
    // assume 4-byte colors
    png_bytep row = row_pointers[y];
    char r = row[x * 4 + 0];
    char g = row[x * 4 + 1];
    char b = row[x * 4 + 2];
    char a = row[x * 4 + 3];

    return a != 0;
  }
};

class BoolMap {
public:
  bool *data;
  int width;
  int height;

  BoolMap(const char *filename, int width, int height)
      : data(new bool[width * height]), width(width), height(height) {
    PngImage image(filename);

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        at(x, y) =
            image.sampleBool(x * image.width / width, y * image.height / height);
      }
    }
  }

  bool &at(int x, int y) { return data[x + y * width]; }
};

void usageAndExit(const char* exe) {
  fprintf(stderr, "Usage: %s pi.png [width [height]]\n", exe);
  exit(1);
}
const char* colorCodes[] = {
  "1;30",
  "0;31",
  "0;32",
  "0;33",
  "0;34",
  "0;35",
  "0;36",
  "0;37",
  "1;32",
  "1;34",
  "1;35",
  "1;36",
};

void beginColor(int color) {
  if(color >= sizeof(colorCodes) / sizeof(const char*)) {
    fprintf(stderr, "bad color: %d\n", color);
    exit(1);
  }
  printf("\x1b[%sm", colorCodes[color]);
}

void fixTerminal() {
  printf("\x1b[7h\n");
}

int main(int argc, char **argv) {
  setvbuf(stdin, NULL, _IONBF, 0);
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  int width = w.ws_col;
  int height = w.ws_row;



  if(width <= 0) {
    if(const char* p = getenv("COLUMNS")) {
      width = atoi(p);
    }
  }

  if(height <= 0) {
    if(const char* p = getenv("ROWS")) {
      height = atoi(p);
    }
  }

  if(argc < 2) {
    fprintf(stderr, "Please provide image\n");
    usageAndExit(argv[0]);
  }

  if(argc > 2) {
    width = atoi(argv[2]);
  }

  if(argc > 3) {
    height = atoi(argv[3]);
  }

  if(width < 1 || height < 1) {
    fprintf(stderr, "Please provide width and/or height for terminal output\n");
    usageAndExit(argv[0]);
  }

  int colors[256];
  for(int i = 0; i < 256; i++) {
    colors[i] = 0;
  }

  const int mode = 1;
  switch(mode) {
  case 0:
    for(int i = '0'; i <= '9'; i++) {
      colors[i] = i - '0' + 1;
    }
    break;
  case 1:
    colors['0'] = 7;
    for(int i = '1'; i <= '9'; i++) {
      colors[i] = 1;
    }
    break;
  }

  try {
    BoolMap map(argv[1], width, height);
    int c = 0;
    int x = 0;
    int y = 0;
    int lastColor = -1;

    printf("\x1b[2J\x1b[0;0H\x1b[71");
    atexit(fixTerminal);

    while ((c = fgetc(stdin)) != EOF) {
      if (c == '\n') {
        x = 0;
        y++;
      }
      if (x >= map.width) {
        if(y < map.height) {
          printf("\n");
        }
        x = 0;
        y++;
      }
      if (x == map.width - 1 && y == map.height - 1) {
        y = 0;
        x = 0;
        printf("\x1b[1;1H");
      }
      int color;
      if(map.at(x, y)) {
        color = colors[c] + 1;
      } else {
        color = 0;
      }
      if(lastColor != color) {
        beginColor(color);
      }
      printf("%c", c);
      lastColor = color;
      x++;
    }
  } catch(PngError& e) {
    fprintf(stderr, "error reading png %s\n", argv[1]);
  }

  return 0;
}
