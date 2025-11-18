# Secure Memory Zone for RISC-V (PicoRV32)

This project implements a **Secure Memory Zone** for the PicoRV32 RISC-V core by adding a lightweight **on‑the‑fly memory encryption/decryption module** between the CPU and RAM. The goal is to protect sensitive data from physical and cold‑boot attacks by ensuring that any data stored in a selected memory region is encrypted automatically, without requiring software changes.

---

## Project Overview

Modern processors (Intel TME, AMD SME, ARM TrustZone) use hardware memory encryption to protect secrets stored in RAM. Teaching‑class RISC‑V cores lack these capabilities, making them vulnerable to:

- **Cold‑boot attacks**
- **Physical memory probing**
- **Device theft–based RAM extraction**

This project demonstrates a simplified version of such hardware‑level protection on the PicoRV32 core using Verilator simulation.

---

## Objectives

- Add a hardware block that encrypts **all writes** and decrypts **all reads** for a configurable memory region.
- Expose configuration via **custom CSRs** (base address, size, enable flag).
- Ensure that normal program execution remains unaffected.
- Demonstrate the encrypted RAM contents using simulation and dumps.
- Evaluate basic performance overhead (cycle count comparison).

---

## Tools Used

- **PicoRV32** (RISC-V RV32IMC core, Verilog)
- **Verilator** (cycle-accurate Verilog-to-C++ simulation)
- **GTKWave** (waveform visualization)
- **riscv64-unknown-elf GCC toolchain** (compile C/RISC-V programs)
- **Make + g++** (build generated simulator)

---

## Project Structure

```
picorv32/
 ├── picorv32.v                # Original CPU core (unchanged)
 ├── my_secure_mem_proj/
 │     ├── secure_top.v        # Top-level wrapper (CPU + encryptor + RAM)
 │     ├── encrypt_unit.v      # Lightweight XOR/LFSR-based cipher
 │     ├── ram.v               # Behavioral RAM model (ciphertext stored here)
 │     ├── sim_main.cpp        # Verilator simulation harness
 │     ├── Makefile            # Build & run automation
 │     ├── sw/
 │     │     ├── test_enc.c    # Test program for secure vs normal region
 │     │     └── linker.ld     # Linker script for bare-metal layout
 │     └── README.md           # (this file)
 └── ...
```

---

## Building the Simulator

Inside `memory-encryption/`:

```
make
```

This runs:

1. Verilator → converts Verilog → C++
2. g++ → builds the simulator (`Vsecure_top`)
3. Places output in `obj_dir/`

---

## Running a Program

Compile the RISC-V test program:

```
cd sw
make
```

Then run the simulator:

```
../obj_dir/Vsecure_top
```

---

## Inspecting Encrypted Memory

Your simulation harness (`sim_main.cpp`) will produce:

- `memory_dump.txt` → raw RAM contents (encrypted in secure region)
- `waveforms.vcd` → open using GTKWave:

```
gtkwave waveforms.vcd &
```

Look for:

- `mem_wdata` (plaintext)
- `ram_data_in` (encrypted)
- `mem_rdata` (decrypted)

---

## Performance Evaluation

Use `rdcycle()` inside your C test to measure:

```
- Time to write N words to normal region
- Time to write N words to encrypted region
```

Document cycle overhead in your report.

---

## Expected Results

- Encrypted region stores unreadable ciphertext.
- CPU always receives correct plaintext.
- Region boundaries controlled via CSRs.
- Minimal slowdown depending on cipher.

---

## Academic Value

This project demonstrates:

- Hardware-software co-design
- Bus-level memory interception
- CSR integration
- Real-world security concepts
- Cycle-accurate performance testing

Perfect for a **Computer Architecture** course project.

---

## License

This project builds on PicoRV32 (ISC license).
All added modules (encryption unit, top-level, test code) are released under MIT.

