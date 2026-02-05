#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string/string.h>
#include <ui/console.h>

#define MAX_FD 32

static vfs_node_t* vfs_root = NULL;
static vfs_node_t* current_dir = NULL;
static file_descriptor_t file_descriptors[MAX_FD];

void vfs_init(void) {
    // Initialize file descriptors
    for (int i = 0; i < MAX_FD; i++) {
        file_descriptors[i].used = 0;
        file_descriptors[i].node = NULL;
        file_descriptors[i].position = 0;
        file_descriptors[i].flags = 0;
    }
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[VFS] Virtual File System initialized\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void vfs_mount_root(vfs_node_t* root) {
    vfs_root = root;
    current_dir = root;
    
    console_write("[VFS] Root filesystem mounted\n");
}

vfs_node_t* vfs_get_root(void) {
    return vfs_root;
}

vfs_node_t* vfs_get_cwd(void) {
    return current_dir;
}

void vfs_set_cwd(vfs_node_t* node) {
    if (node && node->type == VFS_DIRECTORY) {
        current_dir = node;
    }
}

void vfs_get_cwd_path(char* buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    if (current_dir == vfs_root) {
        buffer[0] = '/';
        buffer[1] = '\0';
    } else {
        // Simple implementation - just show name
        strncpy(buffer, current_dir->name, size - 1);
        buffer[size - 1] = '\0';
    }
}

// Helper: Split path into components
static int split_path(const char* path, char components[][256], int max_components) {
    int count = 0;
    int comp_pos = 0;
    
    for (int i = 0; path[i] != '\0' && count < max_components; i++) {
        if (path[i] == '/') {
            if (comp_pos > 0) {
                components[count][comp_pos] = '\0';
                count++;
                comp_pos = 0;
            }
        } else {
            if (comp_pos < 255) {
                components[count][comp_pos++] = path[i];
            }
        }
    }
    
    if (comp_pos > 0 && count < max_components) {
        components[count][comp_pos] = '\0';
        count++;
    }
    
    return count;
}

vfs_node_t* vfs_get_node(const char* path) {
    if (!path || !vfs_root) return NULL;
    
    // Handle root
    if (strcmp(path, "/") == 0) {
        return vfs_root;
    }
    
    // Start from root or current directory
    vfs_node_t* current = (path[0] == '/') ? vfs_root : current_dir;
    
    // Split path
    char components[16][256];
    int count = split_path(path, components, 16);
    
    // Navigate through path
    for (int i = 0; i < count; i++) {
        if (strcmp(components[i], ".") == 0) {
            continue;  // Stay in current directory
        } else if (strcmp(components[i], "..") == 0) {
            // Go to parent (for now, just go to root)
            current = vfs_root;
        } else {
            // Find child
            if (current->finddir) {
                current = current->finddir(current, components[i]);
                if (!current) return NULL;
            } else {
                return NULL;
            }
        }
    }
    
    return current;
}

int vfs_open(const char* path, uint32_t flags) {
    vfs_node_t* node = vfs_get_node(path);
    if (!node) return -1;
    
    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FD; i++) {
        if (!file_descriptors[i].used) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) return -1;  // No free descriptors
    
    // Initialize file descriptor
    file_descriptors[fd].node = node;
    file_descriptors[fd].position = 0;
    file_descriptors[fd].flags = flags;
    file_descriptors[fd].used = 1;
    
    return fd;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_FD || !file_descriptors[fd].used) {
        return -1;
    }
    
    file_descriptors[fd].used = 0;
    file_descriptors[fd].node = NULL;
    return 0;
}

int vfs_read(int fd, void* buffer, size_t size) {
    if (fd < 0 || fd >= MAX_FD || !file_descriptors[fd].used) {
        return -1;
    }
    
    file_descriptor_t* desc = &file_descriptors[fd];
    
    if (!desc->node || !desc->node->read) {
        return -1;
    }
    
    int bytes_read = desc->node->read(desc->node, desc->position, size, (uint8_t*)buffer);
    if (bytes_read > 0) {
        desc->position += bytes_read;
    }
    
    return bytes_read;
}

int vfs_write(int fd, const void* buffer, size_t size) {
    if (fd < 0 || fd >= MAX_FD || !file_descriptors[fd].used) {
        return -1;
    }
    
    file_descriptor_t* desc = &file_descriptors[fd];
    
    if (!desc->node || !desc->node->write) {
        return -1;
    }
    
    int bytes_written = desc->node->write(desc->node, desc->position, size, (uint8_t*)buffer);
    if (bytes_written > 0) {
        desc->position += bytes_written;
    }
    
    return bytes_written;
}

int vfs_seek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= MAX_FD || !file_descriptors[fd].used) {
        return -1;
    }
    
    file_descriptor_t* desc = &file_descriptors[fd];
    
    switch (whence) {
        case SEEK_SET:
            desc->position = offset;
            break;
        case SEEK_CUR:
            desc->position += offset;
            break;
        case SEEK_END:
            desc->position = desc->node->size + offset;
            break;
        default:
            return -1;
    }
    
    return desc->position;
}

vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!node || node->type != VFS_DIRECTORY || !node->readdir) {
        return NULL;
    }
    
    return node->readdir(node, index);
}

vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if (!node || node->type != VFS_DIRECTORY || !node->finddir) {
        return NULL;
    }
    
    return node->finddir(node, name);
}
