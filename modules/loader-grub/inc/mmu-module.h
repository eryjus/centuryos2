
//
// -- function prototypes
//    -------------------
extern "C" {
    Frame_t MmuGetTable(void);
    Return_t MmuIsMapped(Addr_t a);
    void ldr_MmuUnmapPage(Addr_t a);
    void ldr_MmuMapPage(Addr_t a, Frame_t f, int flags);
    void MmuEmptyPdpt(int index);
}


/****************************************************************************************************************//**
*   @fn                 static inline Frame_t PmmGetFrame(void) { return MmuGetTable(); }
*   @brief              Allocate a new frame and make sure it is cleared
*
*   @returns            The next allocatable frame
*
*   No need to duplicate a function -- just call the MmuGetTable() function since it is exactly the same thing
*///-----------------------------------------------------------------------------------------------------------------
static inline Frame_t PmmAlloc(void) { return MmuGetTable(); }
static inline void MmuUnmapPage(Addr_t a) { ldr_MmuUnmapPage(a); }
static inline void MmuMapPage(Addr_t a, Frame_t f, int flags) { ldr_MmuMapPage(a, f, flags); }



