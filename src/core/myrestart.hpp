#include <assert.h>
#include <utils/utils.hpp>


class Restart {
public:
  Restart(): context_offset(0), 
	     memoryMaps_offset(0),
	     fdMetadata_offset(0)
  {
    memset(ckptImage_fileName, 0, sizeof(ckptImage_fileName));
    assert(strlen(ckptImage_fileName) == 0);
  }
 
  void move_stack();                     // to avoid address conflict
  void restore_memoryMaps();             // remap the memory regions
  void restore_fds();                    // restore file descriptors
  void restore_context();                // restore the context of execution

  void foo(){return;}

public:
  static Restart        theInstance;
  static ckptImg_header ckpt_h;
  char   ckptImage_fileName[1024];
  // int    ckptImage_fd;
  off_t  memoryMaps_offset;
  off_t  fdMetadata_offset;
  off_t  context_offset;
};

Restart Restart::theInstance;
