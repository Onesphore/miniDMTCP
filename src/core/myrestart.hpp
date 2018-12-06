#include <assert.h>
#include <sys/types.h>
#include <limits.h>
#include <common/ckptImage.hpp>
#include <utils/utils.hpp>


class Restart {
public:
  static void move_stack();
  static void restore_memoryMaps(int, ckpt_header_t);
  // static void restore_fds();
  static void restore_context(int, ckpt_header_t);

  static void foo(){return;}

public:
  static ckpt_header_t ckpt_h;
  // char   ckptImage_fileName[1024];
  // int    ckptImage_fd;
  // off_t  memoryMaps_offset;
  // off_t  fdMetadata_offset;
  // off_t  context_offset;
};

ckpt_header_t Restart::ckpt_h;
