# SDRAM Driver Example

**English** | [**中文**](README_zh.md)

## Introduction

This project provides a complete SDRAM driver solution for the **Titan Board Mini** platform, based on the Renesas RA8P1 microcontroller and RT-Thread real-time operating system. This driver fully utilizes RA8P1's external memory interface to provide high-performance external memory access for embedded systems.

### Key Features

- **Large capacity**: Supports 64MB SDRAM external storage
- **High-performance access**: 32-bit data bus, high-frequency operation capability
- **Real-time system support**: Fully compatible with RT-Thread memory management framework
- **Complete functionality**: Supports initialization, testing, performance evaluation and more
- **Easy to use**: Provides command-line interface for development and debugging

### Technical Architecture

```
User Application Layer
    ↓
RT-Thread Memory Management Layer
    ↓
BSP SDRAM Driver Layer
    ↓
RA8P1 SDRAMC Hardware Controller
    ↓
Physical SDRAM Chip
```

---

## Hardware Introduction

### 2.1 RA8P1 External Memory Interface Features

The RA8P1 microcontroller integrates a powerful external memory interface specifically designed for connecting SDRAM chips. Main features include:

#### Controller Features
- **SDRAM Controller (SDRAMC)**: Dedicated hardware controller for SDRAM timing management
- **Address/Data multiplexing**: Supports time-division multiplexing of address and data lines
- **32-bit data bus**: High-throughput data transmission
- **Auto-refresh**: Built-in auto-refresh mechanism ensuring data integrity
- **Low-power mode**: Supports self-refresh mode for power reduction

#### Timing Control
- **Programmable timing**: Supports multiple CAS latency settings (CL2, CL3, CL4)
- **Flexible row/column access**: Configurable row precharge and row activation times
- **Refresh cycle**: Programmable auto-refresh interval
- **Burst length**: Supports 1, 2, 4, 8-bit burst transfers

#### Memory Mapping
```c
#define SDRAM_BASE_ADDR    (0x68000000UL)  // SDRAM base address
#define SDRAM_SIZE         (64 * 1024 * 1024UL) // 64MB size
// Access range: 0x68000000 - 0x68FFFFFF
```

### 2.2 Supported SDRAM Types

Current system supported SDRAM configuration:

#### Chip Specifications
- **Capacity**: 64MB (256Mbit)
- **Data width**: 32-bit
- **Organization**: 4banks × 4M × 32bit
- **Operating voltage**: 3.3V
- **Access time**: Standard synchronous SDRAM

#### Timing Parameters
```c
// Key timing parameters
#define BSP_CFG_SDRAM_TRAS  (6)    // Row active to precharge time
#define BSP_CFG_SDRAM_TRCD  (3)    // Row active to column access time
#define BSP_CFG_SDRAM_TRP   (3)    // Precharge time
#define BSP_CFG_SDRAM_TWR   (2)    // Write recovery time
#define BSP_CFG_SDRAM_TCL   (3)    // CAS latency
#define BSP_CFG_SDRAM_TRFC  (937)  // Refresh cycle
#define BSP_CFG_SDRAM_TREFW (8)    // Refresh wait time
```

### 2.3 Hardware Connection

SDRAM chip connects to RA8P1 via the following signal lines:

#### Data Bus
- `DQ[31:0]`: 32-bit bidirectional data bus
- `DQM[3:0]`: Data mask signals

#### Address Bus
- `A[12:0]`: Address lines (multiplexed)
- `BA[1:0]`: Bank address select

#### Control Signals
- `RAS#`: Row address strobe
- `CAS#`: Column address strobe
- `WE#`: Write enable
- `CS#`: Chip select
- `CKE`: Clock enable
- `CLK`: Clock signal

#### Power and Ground
- `VDD/VSS`: Power and ground
- `VDDQ/VSSQ`: Data power and ground

---

## Software Architecture

### 3.1 SDRAM Driver Initialization Flow

SDRAM initialization follows the standard JEDEC specification with the following main steps:

#### Initialization Sequence
```c
void R_BSP_SdramInit(bool init_memory)
{
    // 1. Check status register
    while (R_BUS->SDRAM.SDSR) {
        // Wait for status register to clear
    }

    // 2. Set initialization parameters
    R_BUS->SDRAM.SDIR = ((BSP_CFG_SDRAM_INIT_ARFI - 3U) << R_BUS_SDRAM_SDIR_ARFI_Pos) |
                        (BSP_CFG_SDRAM_INIT_ARFC << R_BUS_SDRAM_SDIR_ARFC_Pos) |
                        ((BSP_CFG_SDRAM_INIT_PRC - 3U) << R_BUS_SDRAM_SDIR_PRC_Pos);

    // 3. Set bus width
    R_BUS->SDRAM.SDCCR = (BSP_CFG_SDRAM_BUS_WIDTH << R_BUS_SDRAM_SDCCR_BSIZE_Pos);

    // 4. Enable clock output
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_CGC);
    R_SYSTEM->SDCKOCR = 1;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_CGC);

    // 5. Execute initialization sequence
    R_BUS->SDRAM.SDICR = 1U;
    while (R_BUS->SDRAM.SDSR_b.INIST) {
        // Wait for initialization to complete
    }

    // 6. Set access mode
    R_BUS->SDRAM.SDAMOD = BSP_CFG_SDRAM_ACCESS_MODE;
    R_BUS->SDRAM.SDCMOD = BSP_CFG_SDRAM_ENDIAN_MODE;

    // 7. Configure mode register
    R_BUS->SDRAM.SDMOD = (BSP_PRV_SDRAM_MR_WB_SINGLE_LOC_ACC << 9) |
                         (BSP_PRV_SDRAM_MR_OP_MODE << 7) |
                         (BSP_CFG_SDRAM_TCL << 4) |
                         (BSP_PRV_SDRAM_MR_BT_SEQUENTIAL << 3) |
                         (BSP_PRV_SDRAM_MR_BURST_LENGTH << 0);

    // 8. Set timing parameters
    R_BUS->SDRAM.SDTR = ((BSP_CFG_SDRAM_TRAS - 1U) << R_BUS_SDRAM_SDTR_RAS_Pos) |
                        ((BSP_CFG_SDRAM_TRCD - 1U) << R_BUS_SDRAM_SDTR_RCD_Pos) |
                        ((BSP_CFG_SDRAM_TRP - 1U) << R_BUS_SDRAM_SDTR_RP_Pos) |
                        ((BSP_CFG_SDRAM_TWR - 1U) << R_BUS_SDRAM_SDTR_WR_Pos) |
                        (BSP_CFG_SDRAM_TCL << R_BUS_SDRAM_SDTR_CL_Pos);

    // 9. Set address offset
    R_BUS->SDRAM.SDADR = BSP_CFG_SDRAM_MULTIPLEX_ADDR_SHIFT;

    // 10. Configure auto-refresh
    R_BUS->SDRAM.SDRFCR = ((BSP_CFG_SDRAM_TREFW - 1U) << R_BUS_SDRAM_SDRFCR_REFW_Pos) |
                          ((BSP_CFG_SDRAM_TRFC - 1U) << R_BUS_SDRAM_SDRFCR_RFC_Pos);

    // 11. Enable auto-refresh
    R_BUS->SDRAM.SDRFEN = 1U;

    // 12. Enable SDRAM access
    R_BUS->SDRAM.SDCCR = R_BUS_SDRAM_SDCCR_EXENB_Msk |
                         (BSP_CFG_SDRAM_BUS_WIDTH << R_BUS_SDRAM_SDCCR_BSIZE_POS);
}
```

#### Key Step Descriptions

1. **Status check**: Ensure SDRAM is in ready state
2. **Initialization parameters**: Configure basic parameters like row refresh and precharge
3. **Clock configuration**: Enable SDRAM clock signal
4. **Initialization execution**: Execute standard SDRAM initialization sequence
5. **Access mode**: Set sequential access mode and endianness
6. **Mode register**: Configure CAS latency, burst length and other parameters
7. **Timing configuration**: Set all critical timing parameters
8. **Address configuration**: Configure address multiplexing mode
9. **Refresh configuration**: Set auto-refresh parameters
10. **Enable refresh**: Start auto-refresh operation
11. **Enable access**: Allow CPU to access SDRAM

### 3.2 Memory Management Mechanism

#### Memory Allocation Strategy
- **Contiguous memory pool**: Uses fixed-size memory blocks
- **Alignment requirements**: 32-bit data bus requires 4-byte alignment
- **Cache optimization**: Supports DMA direct access

#### RT-Thread Integration
By default, RT-Thread's heap is located in on-chip SRAM and cannot directly `rt_malloc()` large blocks like 64MB. The recommended approach is to create an independent memheap in the SDRAM region or use static buffers with `BSP_PLACE_IN_SECTION(".sdram")` attribute. The following example demonstrates how to create a memheap named `sdram`:

```c
#define SDRAM_POOL_SIZE   (4 * 1024 * 1024)  // Adjust as needed
static uint8_t sdram_pool[SDRAM_POOL_SIZE] BSP_PLACE_IN_SECTION(".sdram");
static struct rt_memheap sdram_heap;

void sdram_heap_init(void)
{
    rt_memheap_init(&sdram_heap, "sdram", sdram_pool, sizeof(sdram_pool));
}

void example_use(void)
{
    void *sdram_buffer = rt_memheap_alloc(&sdram_heap, 512 * 1024);
    if (sdram_buffer)
    {
        rt_memcpy(sdram_buffer, src_data, 512 * 1024);
        rt_memheap_free(sdram_buffer);
    }
}
```

#### Data Integrity Protection
- **Initialization verification**: Basic testing before each use
- **Runtime monitoring**: Memory access error detection support
- **Error recovery**: Provides error handling mechanism

### 3.3 BSP Layer Abstraction

#### BSP Interface Functions
```c
// SDRAM initialization
R_BSP_SdramInit(true);

// Self-refresh mode switching
R_BSP_SdramSelfRefreshEnable();  // Enter self-refresh
R_BSP_SdramSelfRefreshDisable(); // Exit self-refresh
```

#### Hardware Abstraction Layer
- **Register mapping**: Unified register access interface
- **Timing configuration**: Configurable timing parameters
- **Status monitoring**: Real-time status query functionality

---

## Usage Examples

### 4.1 Basic Read/Write Operations

#### Memory-mapped Access
```c
// SDRAM basic access example
void basic_sdram_access(void)
{
    uint32_t *sdram_ptr = (uint32_t *)SDRAM_BASE_ADDR;
    uint32_t i;

    // Write test data
    for (i = 0; i < 1024; i++) {
        sdram_ptr[i] = 0xA5A5A5A5 + i;
    }

    // Read and verify
    for (i = 0; i < 1024; i++) {
        uint32_t read_data = sdram_ptr[i];
        uint32_t expected = 0xA5A5A5A5 + i;

        if (read_data != expected) {
            rt_kprintf("Error at offset %d: expected 0x%08X, got 0x%08X\n",
                      i, expected, read_data);
        }
    }

    rt_kprintf("Basic SDRAM access test completed.\n");
}
```

#### Bulk Data Operations
```c
// Large data block operation example
void sdram_bulk_operation(void)
{
    uint32_t *sdram_ptr = (uint32_t *)SDRAM_BASE_ADDR;
    uint32_t *local_buffer = rt_malloc(1024 * 1024); // 1MB

    if (!local_buffer) {
        rt_kprintf("Failed to allocate local buffer.\n");
        return;
    }

    // Fill local buffer
    for (uint32_t i = 0; i < 256 * 1024; i++) {
        local_buffer[i] = 0x12345678 + i;
    }

    // Bulk write to SDRAM
    rt_memcpy(sdram_ptr, local_buffer, 1024 * 1024);

    // Bulk read for verification
    rt_memcpy(local_buffer, sdram_ptr, 1024 * 1024);

    // Verify data integrity
    uint32_t errors = 0;
    for (uint32_t i = 0; i < 256 * 1024; i++) {
        if (local_buffer[i] != 0x12345678 + i) {
            errors++;
        }
    }

    rt_kprintf("Bulk operation test: %d errors found.\n", errors);
    rt_free(local_buffer);
}
```

## Run Effect

Compile and flash the program to the development board, then run `sdram_speed_test` in the terminal to see the results.

![alt text](figures/image1.png)
