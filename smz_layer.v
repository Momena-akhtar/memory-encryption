/*
 * SMZ (Secure Memory Zone) Layer
 * 
 * Transparent encryption/decryption layer for memory operations
 * 
 * Function:
 * - Intercepts CPU memory access requests
 * - Checks if address is in secure region (SMZ_BASE to SMZ_BASE+SMZ_SIZE)
 * - If in secure region: encrypt writes, decrypt reads
 * - If outside: pass through unchanged
 * 
 * Encryption: Simple XOR with derived keystream (expandable to stream cipher)
 */

module smz_layer (
	input wire clk,
	input wire resetn,
	
	// CPU-side interface (from CPU)
	input wire        cpu_mem_valid,
	input wire [31:0] cpu_mem_addr,
	input wire [31:0] cpu_mem_wdata,
	input wire [ 3:0] cpu_mem_wstrb,
	
	// Memory-side interface (to memory)
	output reg [31:0] mem_wdata,
	input wire [31:0] mem_rdata,
	
	// SMZ configuration (from CSRs in picorv32)
	input wire [31:0] smz_base,
	input wire [31:0] smz_size,
	input wire        smz_enable,
	
	// Output (decrypted read data to CPU)
	output wire [31:0] cpu_mem_rdata
);

	// Determine if address is in secure region
	wire in_secure_region = smz_enable && 
	                          (cpu_mem_addr >= smz_base) && 
	                          (cpu_mem_addr < (smz_base + smz_size));
	
	// Simple XOR cipher with address-derived keystream
	// For simplicity: keystream = addr XOR key_constant
	// Expandable to LFSR-based stream cipher
	wire [31:0] keystream = cpu_mem_addr ^ 32'hDEADBEEF;
	
	// On write: encrypt if in secure region
	always @(*) begin
		if (in_secure_region && cpu_mem_valid) begin
			mem_wdata = cpu_mem_wdata ^ keystream;
		end else begin
			mem_wdata = cpu_mem_wdata;
		end
	end
	
	// On read: decrypt if in secure region
	wire [31:0] decrypted_rdata = mem_rdata ^ keystream;
	
	assign cpu_mem_rdata = in_secure_region ? decrypted_rdata : mem_rdata;

endmodule
