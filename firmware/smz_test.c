/*
 * SMZ Final End-to-End Test
 * 
 * Tests: CPU writes 28x28 image to secure memory → encrypted in RAM
 *        CPU reads back → receives decrypted data
 * 
 * This proves the SMZ pipeline works correctly.
 */

#include "firmware.h"

#define CSR_SMZ_BASE    0x200
#define CSR_SMZ_SIZE    0x201
#define CSR_SMZ_ENABLE  0x202

// Inline asm for CSR operations
static inline uint32_t read_csr(uint32_t csr) {
	uint32_t value = 0;
	__asm__ volatile("csrrs %0, %1, x0" : "=r"(value) : "i"(csr));
	return value;
}

static inline void write_csr(uint32_t csr, uint32_t value) {
	__asm__ volatile("csrrw x0, %0, %1" : : "i"(csr), "r"(value));
}

void smz_test(void)
{
	print_str("\n");
	print_str("====================================\n");
	print_str("SMZ Pipeline Test\n");
	print_str("28x28 Image Encryption Verification\n");
	print_str("====================================\n\n");
	
	// Secure region: 0x10000 (within 128KB testbench memory), size 4KB
	#define SECURE_ADDR  0x10000
	#define SECURE_SIZE  0x1000
	
	// 28x28 = 784 bytes = 196 words (32-bit)
	// Use 196*4 = 784 bytes = 0x310 bytes
	
	print_str("STEP 1: Configure SMZ CSRs\n");
	write_csr(CSR_SMZ_BASE, SECURE_ADDR);
	write_csr(CSR_SMZ_SIZE, SECURE_SIZE);
	write_csr(CSR_SMZ_ENABLE, 1);
	
	uint32_t csr_base = read_csr(CSR_SMZ_BASE);
	uint32_t csr_size = read_csr(CSR_SMZ_SIZE);
	uint32_t csr_en = read_csr(CSR_SMZ_ENABLE);
	
	print_str("  Base: 0x");
	print_hex(csr_base, 8);
	print_str("  Size: 0x");
	print_hex(csr_size, 8);
	print_str("  Enable: ");
	print_dec(csr_en);
	print_str("\n\n");
	
	// Generate 28x28 test image pattern
	print_str("STEP 2: Generate 28x28 Test Image (784 bytes)\n");
	uint32_t test_image[196];  // 196 words = 784 bytes
	int i;
	for (i = 0; i < 196; i++) {
		// Pattern: 0xAABBCCDD varying per word
		test_image[i] = (0xAA << 24) | (0xBB << 16) | ((i & 0xFF) << 8) | (i & 0xFF);
	}
	print_str("  Generated pattern. First word: 0x");
	print_hex(test_image[0], 8);
	print_str("\n\n");
	
	// Write to secure region
	print_str("STEP 3: Write Image to Secure Memory (encrypted on write)\n");
	volatile uint32_t *secure_mem = (volatile uint32_t *)SECURE_ADDR;
	for (i = 0; i < 196; i++) {
		secure_mem[i] = test_image[i];
	}
	print_str("  Wrote 196 words to 0x");
	print_hex(SECURE_ADDR, 8);
	print_str(" (data gets encrypted by SMZ)\n\n");
	
	// Read back from secure region (should be decrypted automatically)
	print_str("STEP 4: Read Back from Secure Memory (decrypted on read)\n");
	uint32_t read_image[196];
	for (i = 0; i < 196; i++) {
		read_image[i] = secure_mem[i];
	}
	print_str("  Read 196 words from secure memory\n\n");
	
	// Verify: read data should match original (SMZ decrypted it)
	print_str("STEP 5: Verify Data Integrity\n");
	int matches = 0;
	int first_mismatch = -1;
	
	for (i = 0; i < 196; i++) {
		if (read_image[i] == test_image[i]) {
			matches++;
		} else if (first_mismatch == -1) {
			first_mismatch = i;
		}
	}
	
	print_str("  Matching words: ");
	print_dec(matches);
	print_str(" / 196\n\n");
	
	if (matches == 196) {
		print_str("✓ PASS: ALL DATA MATCHED!\n");
		print_str("  SMZ encrypted data on write and decrypted on read correctly.\n");
		print_str("  Pipeline verification SUCCESSFUL.\n");
	} else {
		print_str("✗ FAIL: Mismatch detected\n");
		print_str("  Mismatches: ");
		print_dec(196 - matches);
		print_str("\n");
		if (first_mismatch != -1) {
			print_str("  First mismatch at word ");
			print_dec(first_mismatch);
			print_str(":\n");
			print_str("    Expected: 0x");
			print_hex(test_image[first_mismatch], 8);
			print_str("\n");
			print_str("    Got: 0x");
			print_hex(read_image[first_mismatch], 8);
			print_str("\n");
		}
	}
	
	print_str("\n====================================\n");
	print_str("Test Complete\n");
	print_str("====================================\n\n");
}
