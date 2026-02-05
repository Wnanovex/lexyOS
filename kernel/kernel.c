#include <multiboot/multiboot2.h>
#include <drivers/video/framebuffer.h>
#include <ui/console.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/pit.h>
#include <arch/x86_64/pic.h>
#include <drivers/input/keyboard.h>
#include <ui/shell/shell.h>
#include <ui/tty/tty.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <fs/vfs.h>
#include <fs/tarfs.h>

extern tty_t ttys[MAX_TTYS];

// Reserve memory for kernel heap
static uint8_t kernel_heap[16 * 1024 * 1024] __attribute__((aligned(4096)));

// Get total RAM from multiboot
static uint64_t get_memory_size(void* multiboot_info) {
    uint8_t* mb = multiboot_info;
    struct multiboot_tag* tag = (void*)(mb + 8);
    
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO) {
            struct multiboot_tag_basic_meminfo* meminfo = 
                (struct multiboot_tag_basic_meminfo*)tag;
            return (uint64_t)(meminfo->mem_upper + 1024) * 1024;
        }
        tag = (void*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    return 128 * 1024 * 1024;
}

// Get initrd module from multiboot
static void* get_initrd(void* multiboot_info, size_t* size) {
    uint8_t* mb = multiboot_info;
    struct multiboot_tag* tag = (void*)(mb + 8);
    
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
            struct multiboot_tag_module* mod = (struct multiboot_tag_module*)tag;
            *size = mod->mod_end - mod->mod_start;
            return (void*)(uint64_t)mod->mod_start;
        }
        tag = (void*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    return NULL;
}

void kernel_main(void* multiboot_info) {
    struct framebuffer fb;
    
    // Initialize framebuffer
    fb_init(&fb, multiboot_info);
    console_init(&fb);

    // Setup console appearance
    console_clear();  
    console_set_scale(2);
    console_set_fg_color(0, 255, 0);
    console_set_bg_color(0, 0, 0);
    console_write("\n\n\n");

    console_write(" /$$                                /$$$$$$   /$$$$$$ \n"); 
    console_write("| $$                               /$$__  $$ /$$__  $$\n");
    console_write("| $$  /$$$$$$  /$$   /$$ /$$   /$$| $$  \\ $$| $$  \\__/\n");
    console_write("| $$ /$$__  $$|  $$ /$$/| $$  | $$| $$  | $$|  $$$$$$ \n");
    console_write("| $$| $$$$$$$$ \\  $$$$/ | $$  | $$| $$  | $$ \\____  $$\n");
    console_write("| $$| $$_____/  >$$  $$ | $$  | $$| $$  | $$ /$$  \\ $$\n");
    console_write("| $$|  $$$$$$$ /$$/\\  $$|  $$$$$$$|  $$$$$$/|  $$$$$$/\n");
    console_write("|__/ \\_______/|__/  \\__/ \\____  $$ \\______/  \\______/ \n");
    console_write("                         /$$  | $$                    \n");
    console_write("                        |  $$$$$$/                    \n");
    console_write("                         \\______/                     \n");

    console_write("\n\n\n\n");
    console_set_fg_color(0, 0, 255);
    console_set_bg_color(0, 0, 0);
    console_write("lexyOS System\n\n\n");
    console_set_fg_color(255, 0, 0);
    console_set_bg_color(0, 0, 0);
    console_write("\n\n\n\n");

    // Initialize subsystems
    idt_init();
    pic_init();
    pit_init(1000);   // 1000 Hz = 1 ms per tick
    pit_sleep(5000);  // 5 seconds
    keyboard_init();
    
    // Initialize memory management
    uint64_t total_mem = get_memory_size(multiboot_info);
    pmm_init(total_mem);
    vmm_init();
    heap_init(kernel_heap, sizeof(kernel_heap));
    
    // Initialize filesystem
    vfs_init();
    
    // Load initrd
    size_t initrd_size = 0;
    void* initrd_data = get_initrd(multiboot_info, &initrd_size);
    
    if (initrd_data && initrd_size > 0) {
        vfs_node_t* root = tarfs_init(initrd_data, initrd_size);
        if (root) {
            vfs_mount_root(root);
        } else {
            console_write("[KERNEL] Failed to mount initrd\n");
        }
    } else {
        console_write("[KERNEL] No initrd found\n");
    }
    
    // Initialize TTY system
    tty_init();
    
    // Initialize shell
    shell_init();

    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);

    // Main loop
    while (1) {
        tty_poll_input();
        
        int current = tty_get_current();
        
        for (int i = 0; i < MAX_TTYS; i++) {
            if (ttys[i].initialized && ttys[i].update_func) {
                ttys[i].update_func();
            }
        }
        
        if (ttys[current].draw_func) {
            if (ttys[current].mode == TTY_MODE_GAME || ttys[current].needs_redraw) {
                ttys[current].draw_func();
                if (ttys[current].mode == TTY_MODE_SHELL) {
                    ttys[current].needs_redraw = 0;
                }
            }
        }
        
        asm volatile ("hlt");
    }
}
