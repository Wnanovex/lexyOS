#include <fs/tarfs.h>
#include <mm/heap.h>
#include <lib/string/string.h>
#include <ui/console.h>

// TAR header structure (USTAR format)
typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} __attribute__((packed)) tar_header_t;

typedef struct {
    void* tar_data;
    size_t tar_size;
    uint32_t num_files;
    vfs_node_t** file_nodes;
} tarfs_data_t;

// Helper: Parse octal string
static uint32_t parse_octal(const char* str, size_t size) {
    uint32_t result = 0;
    for (size_t i = 0; i < size && str[i] >= '0' && str[i] <= '7'; i++) {
        result = result * 8 + (str[i] - '0');
    }
    return result;
}

// TAR read operation
static int tarfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node || !node->impl || !buffer) return -1;
    
    uint8_t* file_data = (uint8_t*)node->impl;
    
    // Check bounds
    if (offset >= node->size) return 0;
    
    uint32_t bytes_to_read = size;
    if (offset + bytes_to_read > node->size) {
        bytes_to_read = node->size - offset;
    }
    
    // Copy data
    memcpy(buffer, file_data + offset, bytes_to_read);
    
    return bytes_to_read;
}

// TAR readdir operation
static vfs_node_t* tarfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!node || !node->impl) return NULL;
    
    tarfs_data_t* fs_data = (tarfs_data_t*)node->impl;
    
    if (index >= fs_data->num_files) return NULL;
    
    return fs_data->file_nodes[index];
}

// TAR finddir operation
static vfs_node_t* tarfs_finddir(vfs_node_t* node, const char* name) {
    if (!node || !node->impl || !name) return NULL;
    
    tarfs_data_t* fs_data = (tarfs_data_t*)node->impl;
    
    for (uint32_t i = 0; i < fs_data->num_files; i++) {
        if (strcmp(fs_data->file_nodes[i]->name, name) == 0) {
            return fs_data->file_nodes[i];
        }
    }
    
    return NULL;
}

vfs_node_t* tarfs_init(void* tar_data, size_t tar_size) {
    if (!tar_data || tar_size == 0) return NULL;
    
    console_write("[TARFS] Initializing TAR filesystem...\n");
    
    // Count files in TAR
    uint32_t num_files = 0;
    uint8_t* ptr = (uint8_t*)tar_data;
    
    while (ptr < (uint8_t*)tar_data + tar_size) {
        tar_header_t* header = (tar_header_t*)ptr;
        
        // Check for end of archive (empty block)
        if (header->name[0] == '\0') break;
        
        // Check magic
        if (strncmp(header->magic, "ustar", 5) != 0) {
            console_write("[TARFS] WARNING: Invalid TAR magic\n");
            break;
        }
        
        uint32_t file_size = parse_octal(header->size, 11);
        
        // Only count regular files and directories
        if (header->type == '0' || header->type == '\0' || header->type == '5') {
            num_files++;
        }
        
        // Move to next header (512-byte aligned)
        uint32_t blocks = (file_size + 511) / 512;
        ptr += 512 + blocks * 512;
    }
    
    console_write("[TARFS] Found ");
    console_putchar('0' + (num_files / 10));
    console_putchar('0' + (num_files % 10));
    console_write(" files\n");
    
    if (num_files == 0) return NULL;
    
    // Allocate filesystem data
    tarfs_data_t* fs_data = (tarfs_data_t*)kmalloc(sizeof(tarfs_data_t));
    if (!fs_data) return NULL;
    
    fs_data->tar_data = tar_data;
    fs_data->tar_size = tar_size;
    fs_data->num_files = num_files;
    fs_data->file_nodes = (vfs_node_t**)kmalloc(sizeof(vfs_node_t*) * num_files);
    
    if (!fs_data->file_nodes) {
        kfree(fs_data);
        return NULL;
    }
    
    // Parse files and create nodes
    ptr = (uint8_t*)tar_data;
    uint32_t file_idx = 0;
    
    while (ptr < (uint8_t*)tar_data + tar_size && file_idx < num_files) {
        tar_header_t* header = (tar_header_t*)ptr;
        
        if (header->name[0] == '\0') break;
        
        uint32_t file_size = parse_octal(header->size, 11);
        uint8_t* file_data = ptr + 512;
        
        // Only process regular files and directories
        if (header->type == '0' || header->type == '\0' || header->type == '5') {
            vfs_node_t* node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
            if (node) {
                memset(node, 0, sizeof(vfs_node_t));
                
                // Copy name
                strncpy(node->name, header->name, 255);
                node->name[255] = '\0';
                
                // Set type
                node->type = (header->type == '5') ? VFS_DIRECTORY : VFS_FILE;
                node->size = file_size;
                node->inode = file_idx;
                
                // Set operations
                node->read = tarfs_read;
                node->write = NULL;  // Read-only
                node->readdir = NULL;
                node->finddir = NULL;
                
                // Store pointer to file data
                node->impl = (void*)file_data;
                
                fs_data->file_nodes[file_idx++] = node;
            }
        }
        
        // Move to next header
        uint32_t blocks = (file_size + 511) / 512;
        ptr += 512 + blocks * 512;
    }
    
    // Create root node
    vfs_node_t* root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!root) {
        kfree(fs_data->file_nodes);
        kfree(fs_data);
        return NULL;
    }
    
    memset(root, 0, sizeof(vfs_node_t));
    strcpy(root->name, "/");
    root->type = VFS_DIRECTORY;
    root->size = 0;
    root->inode = 0;
    root->read = NULL;
    root->write = NULL;
    root->readdir = tarfs_readdir;
    root->finddir = tarfs_finddir;
    root->impl = fs_data;
    
    console_write("[TARFS] TAR filesystem ready\n");
    
    return root;
}
