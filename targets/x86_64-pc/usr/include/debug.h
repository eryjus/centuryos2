#define DEBUG_TOKEN_PASTE(x) DEBUG_##x
#define DEBUG_ENABLED(f) DEBUG_TOKEN_PASTE(f)>DISABLED
#define DEBUG_DebuggerMain DISABLED
#define DEBUG_lInit DISABLED
#define DEBUG_MBootGetKernel DISABLED
#define DEBUG_MBootGetMb1Kernel DISABLED
#define DBEUG_MBootGetKernel DISABLED
#define DEBUG_GetPML4Entry DISABLED
#define DEBUG_GetPDPTEntry DISABLED
#define DEBUG_GetPDEntry DISABLED
#define DEBUG_GetPTEntry DISABLED
#define DEBUG_MmuIsMapped DISABLED
#define DEBUG_ldr_MmuMapPage DISABLED
#define DEBUG_ldr_MmuUnmapPage DISABLED
#define DEBUG_MmuEmptyPdpt DISABLED
