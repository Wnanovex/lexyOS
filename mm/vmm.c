#include <mm/vmm.h>
#include <mm/pmm.h>
#include <ui/console.h>

// Kernel PML4 (Page Map Level 4 - top level page table)
static page_table_t* kernel_pml4 = NULL;

// Helper: Get or create page table
static page_table_t* get_or_create_table(uint64_t* entry) {
    if (*entry & PAGE_PRESENT) {
        return (page_table_t*)(*entry & ~0xFFF);
    }
    
    // Allocate new page table
    uint64_t phys_addr = pmm_alloc_page();
    if (!phys_addr) {
        return NULL;
    }
    
    // Clear the new table
    page_table_t* table = (page_table_t*)phys_addr;
    for (int i = 0; i < 512; i++) {
        table->entries[i] = 0;
    }
    
    *entry = phys_addr | PAGE_PRESENT | PAGE_WRITABLE;
    return table;
}

void vmm_init(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[VMM] Virtual Memory Manager initialized\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    
    // Allocate kernel PML4
    uint64_t pml4_phys = pmm_alloc_page();
    if (!pml4_phys) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("[VMM] ERROR: Failed to allocate PML4!\n");
        return;
    }
    
    kernel_pml4 = (page_table_t*)pml4_phys;
    
    // Clear PML4
    for (int i = 0; i < 512; i++) {
        kernel_pml4->entries[i] = 0;
    }
    
    // Identity map first 16MB (kernel space)
    for (uint64_t addr = 0; addr < 16 * 1024 * 1024; addr += PAGE_SIZE) {
        vmm_map_page(addr, addr, PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    console_write("[VMM] Identity mapped first 16MB\n");
}

int vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    if (!kernel_pml4) return -1;
    
    // Extract page table indices from virtual address
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdp_index  = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index   = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index   = (virt_addr >> 12) & 0x1FF;
    
    // Get or create PDPT (Page Directory Pointer Table)
    page_table_t* pdpt = get_or_create_table(&kernel_pml4->entries[pml4_index]);
    if (!pdpt) return -1;
    
    // Get or create PD (Page Directory)
    page_table_t* pd = get_or_create_table(&pdpt->entries[pdp_index]);
    if (!pd) return -1;
    
    // Get or create PT (Page Table)
    page_table_t* pt = get_or_create_table(&pd->entries[pd_index]);
    if (!pt) return -1;
    
    // Map the page
    pt->entries[pt_index] = (phys_addr & ~0xFFF) | flags;
    
    // Flush TLB for this page
    __asm__ volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
    
    return 0;
}

void vmm_unmap_page(uint64_t virt_addr) {
    if (!kernel_pml4) return;
    
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdp_index  = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index   = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index   = (virt_addr >> 12) & 0x1FF;
    
    // Navigate through page tables
    if (!(kernel_pml4->entries[pml4_index] & PAGE_PRESENT)) return;
    page_table_t* pdpt = (page_table_t*)(kernel_pml4->entries[pml4_index] & ~0xFFF);
    
    if (!(pdpt->entries[pdp_index] & PAGE_PRESENT)) return;
    page_table_t* pd = (page_table_t*)(pdpt->entries[pdp_index] & ~0xFFF);
    
    if (!(pd->entries[pd_index] & PAGE_PRESENT)) return;
    page_table_t* pt = (page_table_t*)(pd->entries[pd_index] & ~0xFFF);
    
    // Unmap the page
    pt->entries[pt_index] = 0;
    
    // Flush TLB
    __asm__ volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

uint64_t vmm_get_physical_address(uint64_t virt_addr) {
    if (!kernel_pml4) return 0;
    
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdp_index  = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index   = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index   = (virt_addr >> 12) & 0x1FF;
    uint64_t offset     = virt_addr & 0xFFF;
    
    if (!(kernel_pml4->entries[pml4_index] & PAGE_PRESENT)) return 0;
    page_table_t* pdpt = (page_table_t*)(kernel_pml4->entries[pml4_index] & ~0xFFF);
    
    if (!(pdpt->entries[pdp_index] & PAGE_PRESENT)) return 0;
    page_table_t* pd = (page_table_t*)(pdpt->entries[pdp_index] & ~0xFFF);
    
    if (!(pd->entries[pd_index] & PAGE_PRESENT)) return 0;
    page_table_t* pt = (page_table_t*)(pd->entries[pd_index] & ~0xFFF);
    
    if (!(pt->entries[pt_index] & PAGE_PRESENT)) return 0;
    
    return (pt->entries[pt_index] & ~0xFFF) | offset;
}

void vmm_switch_page_directory(page_table_t* pml4) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4) : "memory");
}
