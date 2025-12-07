/*
 * SMZ CSR Interface Header
 * 
 * This header provides convenient macros and functions for accessing
 * the Secure Memory Zone (SMZ) Control Status Registers (CSRs) from
 * software running on PicoRV32.
 */
#include <stdio.h>
#ifndef _SMZ_CSR_H_
#define _SMZ_CSR_H_

#include <stdint.h>

/* ===================================================================
 * CSR Address Definitions
 * =================================================================== */

#define CSR_SMZ_BASE    0x200   /**< SMZ base address CSR */
#define CSR_SMZ_SIZE    0x201   /**< SMZ region size CSR */
#define CSR_SMZ_ENABLE  0x202   /**< SMZ enable flag CSR */

/* ===================================================================
 * CSR Read/Write Macros
 * =================================================================== */

/**
 * Read a CSR using CSRRS instruction
 * @param csr  CSR address (12-bit immediate)
 * @return     Current CSR value
 */
#define read_csr(csr) ({                                    \
    register uint32_t __tmp;                                \
    asm volatile ("csrrs %0, " #csr ", x0"                  \
        : "=r"(__tmp)                                       \
        :                                                    \
        : );                                                 \
    __tmp;                                                  \
})

/**
 * Write a CSR using CSRRW instruction
 * @param csr  CSR address (12-bit immediate)
 * @param val  Value to write to CSR
 */
#define write_csr(csr, val) ({                              \
    register uint32_t __tmp = (val);                        \
    asm volatile ("csrrw x0, " #csr ", %0"                  \
        :                                                    \
        : "r"(__tmp)                                         \
        : );                                                 \
})

/**
 * Atomic set bits in a CSR
 * @param csr  CSR address (12-bit immediate)
 * @param val  Bit mask to set
 */
#define set_csr_bits(csr, val) ({                           \
    register uint32_t __tmp = (val);                        \
    asm volatile ("csrrs x0, " #csr ", %0"                  \
        :                                                    \
        : "r"(__tmp)                                         \
        : );                                                 \
})

/**
 * Atomic clear bits in a CSR
 * @param csr  CSR address (12-bit immediate)
 * @param val  Bit mask to clear
 */
#define clear_csr_bits(csr, val) ({                         \
    register uint32_t __tmp = (val);                        \
    asm volatile ("csrrc x0, " #csr ", %0"                  \
        :                                                    \
        : "r"(__tmp)                                         \
        : );                                                 \
})

/**
 * Read and clear CSR bits
 * @param csr  CSR address (12-bit immediate)
 * @param val  Bit mask to clear
 * @return     Previous CSR value
 */
#define read_and_clear_csr(csr, val) ({                     \
    register uint32_t __tmp = (val);                        \
    register uint32_t __result;                             \
    asm volatile ("csrrc %0, " #csr ", %1"                  \
        : "=r"(__result)                                     \
        : "r"(__tmp)                                         \
        : );                                                 \
    __result;                                                \
})

/* ===================================================================
 * SMZ-specific Read/Write Macros
 * =================================================================== */

/**
 * Read the SMZ base address register
 */
#define smz_read_base() read_csr(CSR_SMZ_BASE)

/**
 * Write the SMZ base address register
 * @param base New base address
 */
#define smz_write_base(base) write_csr(CSR_SMZ_BASE, (base))

/**
 * Read the SMZ size register
 */
#define smz_read_size() read_csr(CSR_SMZ_SIZE)

/**
 * Write the SMZ size register
 * @param size New size value
 */
#define smz_write_size(size) write_csr(CSR_SMZ_SIZE, (size))

/**
 * Read the SMZ enable register
 * @return 1 if enabled, 0 if disabled
 */
#define smz_read_enable() read_csr(CSR_SMZ_ENABLE)

/**
 * Write the SMZ enable register
 * @param enable 1 to enable SMZ, 0 to disable
 */
#define smz_write_enable(enable) write_csr(CSR_SMZ_ENABLE, ((enable) ? 1 : 0))

/**
 * Enable SMZ
 */
#define smz_enable() write_csr(CSR_SMZ_ENABLE, 1)

/**
 * Disable SMZ
 */
#define smz_disable() write_csr(CSR_SMZ_ENABLE, 0)

/**
 * Check if SMZ is enabled
 * @return non-zero if enabled, 0 if disabled
 */
#define smz_is_enabled() (read_csr(CSR_SMZ_ENABLE) & 1)

/* ===================================================================
 * Utility Functions
 * =================================================================== */

/**
 * Initialize SMZ with specified configuration
 * @param base  Base address of secure region
 * @param size  Size of secure region
 * @param enable 1 to enable, 0 to disable
 * @return      0 on success, -1 on validation error
 */
static inline int smz_init(uint32_t base, uint32_t size, int enable) {
    // Validate base address is word-aligned
    if ((base & 0x3) != 0) {
        return -1;
    }
    
    // Validate size is power of 2
    if ((size & (size - 1)) != 0 || size == 0) {
        return -1;
    }
    
    // Disable SMZ during configuration
    smz_disable();
    
    // Wait for disable to take effect
    volatile int delay = 10;
    while (delay--);
    
    // Configure base address
    smz_write_base(base);
    
    // Configure size
    smz_write_size(size);
    
    // Enable if requested
    if (enable) {
        smz_enable();
    }
    
    return 0;
}

/**
 * Reconfigure SMZ secure region
 * @param base  New base address
 * @param size  New region size
 * @return      0 on success, -1 on validation error
 */
static inline int smz_reconfigure(uint32_t base, uint32_t size) {
    // Validate inputs
    if ((base & 0x3) != 0 || (size & (size - 1)) != 0 || size == 0) {
        return -1;
    }
    
    // Temporarily disable SMZ
    int was_enabled = smz_is_enabled();
    smz_disable();
    
    // Wait for disable to take effect
    volatile int delay = 10;
    while (delay--);
    
    // Update configuration
    smz_write_base(base);
    smz_write_size(size);
    
    // Restore enable state
    if (was_enabled) {
        smz_enable();
    }
    
    return 0;
}

/**
 * Get current SMZ configuration
 * @param[out] base    Pointer to store base address
 * @param[out] size    Pointer to store region size
 * @param[out] enable  Pointer to store enable flag
 */
static inline void smz_get_config(uint32_t *base, uint32_t *size, int *enable) {
    if (base) {
        *base = smz_read_base();
    }
    if (size) {
        *size = smz_read_size();
    }
    if (enable) {
        *enable = smz_is_enabled();
    }
}

/**
 * Print SMZ configuration to console (requires printf)
 */
static inline void smz_print_config(void) {
    uint32_t base = smz_read_base();
    uint32_t size = smz_read_size();
    int enable = smz_is_enabled();
    
    printf("SMZ Configuration:\n");
    printf("  Base Address: 0x%08x\n", base);
    printf("  Region Size:  0x%08x (%u bytes)\n", size, size);
    printf("  Status:       %s\n", enable ? "ENABLED" : "DISABLED");
}

#endif  /* _SMZ_CSR_H_ */
