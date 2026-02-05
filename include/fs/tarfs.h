#ifndef TARFS_H
#define TARFS_H

#include <stdint.h>
#include <fs/vfs.h>

// Initialize TARFS from initrd
vfs_node_t* tarfs_init(void* tar_data, size_t tar_size);

#endif // TARFS_H
