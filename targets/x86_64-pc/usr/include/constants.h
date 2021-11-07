#define ENABLED 1
#define DISABLED 0
#define IS_ENABLED(func) func>DISABLED
#define KERNEL_DEBUGGER ENABLED
#define DEBUGGER_SLEEP_START 0
#define DBG_CMD_LEN 16
#define DBG_ALIAS_LEN 6
#define DBG_NAME_LEN MOD_NAME_LEN
#define DBG_MAX_CMD_LEN 256
#define MOD_NAME_LEN 16
#define IPI_PAUSE_CORES 0x20
#define INT_TIMER 0x30
#define INT_SPURIOUS 0xff
#define MAGIC1 0x1badb002
#define MAGIC2 0xe85250d6
#define MB1SIG 0x2badb002
#define MB2SIG 0x36d76289
#define MBFLAGS ((1<<1)|(1<<2))
#define TRAMP_OFF 0x3000
#define PAGE_SIZE 0x1000
#define PML4_ENTRY_ADDRESS ((Addr_t)0xfffffffffffff000)
#define PDPT_ENTRY_ADDRESS ((Addr_t)0xffffffffffe00000)
#define PD_ENTRY_ADDRESS ((Addr_t)0xffffffffc0000000)
#define PT_ENTRY_ADDRESS ((Addr_t)0xffffff8000000000)
#define TEMP_MAP ((Addr_t)0xffffff0000000000)
#define LOW_STACK ((Addr_t)0xffffff0000001000)
#define NORM_STACK ((Addr_t)0xffffff0000002000)
#define SCRUB_STACK ((Addr_t)0xffffff0000003000)
#define TEMP_INSERT ((Addr_t)0xffffff0000004000)
#define CLEAR_ADDR ((Addr_t)0xffffff0000005000)
#define INTERFACE_LOCATION ((Addr_t)0xffff9ffffffff000)
#define KERNEL_STACK ((Addr_t)0xfffff80000000000)
#define MODE_TYPE 0
#define WIDTH 1024
#define HEIGHT 768
#define DEPTH 16
#define EBDA 0x80000
#define BIOS 0xe0000
#define BIOS_END 0xfffff
#define COM1_BASE 0x3f8
