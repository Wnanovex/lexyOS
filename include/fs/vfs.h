#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

// File types
#define VFS_FILE      0
#define VFS_DIRECTORY 1

// File modes
#define O_RDONLY  0x0001
#define O_WRONLY  0x0002
#define O_RDWR    0x0004
#define O_CREAT   0x0008

// Seek modes
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

// File structure
typedef struct vfs_node {
    char name[256];           // File name
    uint32_t type;            // File or directory
    uint32_t size;            // File size
    uint32_t inode;           // Inode number
    
    // File operations
    int (*read)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
    int (*write)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
    struct vfs_node* (*readdir)(struct vfs_node* node, uint32_t index);
    struct vfs_node* (*finddir)(struct vfs_node* node, const char* name);
    
    void* impl;               // Filesystem-specific data
} vfs_node_t;

// File descriptor
typedef struct {
    vfs_node_t* node;
    uint32_t position;
    uint32_t flags;
    int used;
} file_descriptor_t;

// Initialize VFS
void vfs_init(void);

// Mount root filesystem
void vfs_mount_root(vfs_node_t* root);

// File operations
int vfs_open(const char* path, uint32_t flags);
int vfs_close(int fd);
int vfs_read(int fd, void* buffer, size_t size);
int vfs_write(int fd, const void* buffer, size_t size);
int vfs_seek(int fd, int offset, int whence);

// Directory operations
vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index);
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name);

// Path resolution
vfs_node_t* vfs_get_node(const char* path);
vfs_node_t* vfs_get_root(void);

// Current directory
void vfs_set_cwd(vfs_node_t* node);
vfs_node_t* vfs_get_cwd(void);
void vfs_get_cwd_path(char* buffer, size_t size);

#endif // VFS_H
