#include <isa.h>
#include <memory/paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
//  0x800002b7,  // lui t0,0x80000
//  0x0002a023,  // sw  zero,0(t0)
//  0x0002a503,  // lw  a0,0(t0)
//  0x0000006b,  // nemu_trap
        0x00000413,
        0x00009117,
        0xffc10113,
        0x0bc000ef,
        0x00050463,
        0x00008067,
        0xff010113,
        0x00100513,
        0x00112623,
        0x098000ef,
        0x4035d793,
        0xff010113,
        0x00f50533,
        0x00a12623,
        0x0075f793,
        0x00100593,
        0x00f595b3,
        0x00c12783,
        0x02061863,
        0x0007c783,
        0xfff5c593,
        0x00f5f5b3,
        0x00c12703,
        0x00c12783,
        0x00b70023,
        0x0007c503,
        0x01010113,
        0xfff50513,
        0x00153513,
        0xf9dff06f,
        0x0007c783,
        0x00f5e5b3,
        0x0ff5f593,
        0xfd5ff06f,
        0xfe010113,
        0x00c10513,
        0xfaa00793,
        0x00100613,
        0x00800593,
        0x00112e23,
        0x00f10623,
        0x000106a3,
        0xf81ff0ef,
        0x01c12083,
        0x00000513,
        0x02010113,
        0x00008067,
        0x00050513,
        0x0000006b,
        0x0000006f,
        0x80000537,
        0xff010113,
        0x0e850513,
        0x00112623,
        0xfb1ff0ef,
        0x00050513,
        0x0000006b,
        0x0000006f
};

static void restart() {
  /* Set the initial program counter. */
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  cpu.gpr[0]._32 = 0;
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}
