#define DEBUG_TOKEN_PASTE(x) DEBUG_##x
#define DEBUG_ENABLED(f) DEBUG_TOKEN_PASTE(f)>DISABLED
#define DEBUG_DebuggerMain DISABLED
