#include "CUFLU.h"



// external functions and GPU-related set-up
#ifdef __CUDACC__

#include "CUAPI.h"
#include "CUFLU_Shared_FluUtility.cu"
#include "CUDA_ConstMemory.h"

#endif // #ifdef __CUDACC__



/********************************************************
1. Deleptonization source term
   --> Enabled by the runtime option "SRC_DELEPTONIZATION"

2. This file is shared by both CPU and GPU

   CUSRC_Src_Deleptonization.cu -> CPU_Src_Deleptonization.cpp

3. Four steps are required to implement a source term

   I.   Set auxiliary arrays
   II.  Implement the source-term function
   III. [Optional] Add the work to be done every time
        before calling the major source-term function
   IV.  Set initialization functions

4. The source-term function must be thread-safe and
   not use any global variable
********************************************************/



// =======================
// I. Set auxiliary arrays
// =======================

//-------------------------------------------------------------------------------------------------------
// Function    :  Src_SetAuxArray_Deleptonization
// Description :  Set the auxiliary arrays AuxArray_Flt/Int[]
//
// Note        :  1. Invoked by Src_Init_Deleptonization()
//                2. AuxArray_Flt/Int[] have the size of SRC_NAUX_DLEP defined in Macro.h (default = 5)
//                3. Add "#ifndef __CUDACC__" since this routine is only useful on CPU
//
// Parameter   :  AuxArray_Flt/Int : Floating-point/Integer arrays to be filled up
//
// Return      :  AuxArray_Flt/Int[]
//-------------------------------------------------------------------------------------------------------
#ifndef __CUDACC__
void Src_SetAuxArray_Deleptonization( double AuxArray_Flt[], int AuxArray_Int[] )
{

// TBF

} // FUNCTION : Src_SetAuxArray_Deleptonization
#endif // #ifndef __CUDACC__



// ======================================
// II. Implement the source-term function
// ======================================

//-------------------------------------------------------------------------------------------------------
// Function    :  Src_Deleptonization
// Description :  Major source-term function
//
// Note        :  1. Invoked by CPU/GPU_SrcSolver_IterateAllCells()
//                2. See Src_SetAuxArray_Deleptonization() for the values stored in AuxArray_Flt/Int[]
//                3. Shared by both CPU and GPU
//
// Parameter   :  fluid             : Fluid array storing both the input and updated values
//                                    --> Including both active and passive variables
//                B                 : Cell-centered magnetic field
//                SrcTerms          : Structure storing all source-term variables
//                dt                : Time interval to advance solution
//                dh                : Grid size
//                x/y/z             : Target physical coordinates
//                TimeNew           : Target physical time to reach
//                TimeOld           : Physical time before update
//                                    --> This function updates physical time from TimeOld to TimeNew
//                MinDens/Pres/Eint : Density, pressure, and internal energy floors
//                EoS_DensEint2Pres : EoS routine to compute the gas pressure
//                EoS_DensPres2Eint : EoS routine to compute the gas internal energy
//                EoS_DensPres2CSqr : EoS routine to compute the sound speed square
//                EoS_AuxArray_*    : Auxiliary arrays for the EoS routines
//                EoS_Table         : EoS tables
//                AuxArray_*        : Auxiliary arrays (see the Note above)
//
// Return      :  fluid[]
//-----------------------------------------------------------------------------------------
GPU_DEVICE_NOINLINE
static void Src_Deleptonization( real fluid[], const real B[],
                                 const SrcTerms_t SrcTerms, const real dt, const real dh,
                                 const double x, const double y, const double z,
                                 const double TimeNew, const double TimeOld,
                                 const real MinDens, const real MinPres, const real MinEint,
                                 const EoS_DE2P_t EoS_DensEint2Pres,
                                 const EoS_DP2E_t EoS_DensPres2Eint,
                                 const EoS_DP2C_t EoS_DensPres2CSqr,
                                 const double EoS_AuxArray_Flt[],
                                 const int    EoS_AuxArray_Int[],
                                 const real *const EoS_Table[EOS_NTABLE_MAX],
                                 const double AuxArray_Flt[], const int AuxArray_Int[] )
{

// check
#  ifdef GAMER_DEBUG
   if ( AuxArray_Flt == NULL )   printf( "ERROR : AuxArray_Flt == NULL in %s !!\n", __FUNCTION__ );
   if ( AuxArray_Int == NULL )   printf( "ERROR : AuxArray_Int == NULL in %s !!\n", __FUNCTION__ );
#  endif

// TBF

} // FUNCTION : Src_Deleptonization



// ==================================================
// III. [Optional] Add the work to be done every time
//      before calling the major source-term function
// ==================================================

//-------------------------------------------------------------------------------------------------------
// Function    :  Src_WorkBeforeMajorFunc_Deleptonization
// Description :  Specify work to be done every time before calling the major source-term function
//
// Note        :  1. Invoked by Src_WorkBeforeMajorFunc()
//                2. Add "#ifndef __CUDACC__" since this routine is only useful on CPU
//
// Parameter   :  lv               : Target refinement level
//                TimeNew          : Target physical time to reach
//                TimeOld          : Physical time before update
//                                   --> The major source-term function will update the system from TimeOld to TimeNew
//                dt               : Time interval to advance solution
//                                   --> Physical coordinates : TimeNew - TimeOld == dt
//                                       Comoving coordinates : TimeNew - TimeOld == delta(scale factor) != dt
//                AuxArray_Flt/Int : Auxiliary arrays
//                                   --> Can be used and/or modified here
//
// Return      :  None
//-------------------------------------------------------------------------------------------------------
#ifndef __CUDACC__
void Src_WorkBeforeMajorFunc_Deleptonization( const int lv, const double TimeNew, const double TimeOld, const double dt,
                                              double AuxArray_Flt[], int AuxArray_Int[] )
{

// TBF

} // FUNCTION : Src_WorkBeforeMajorFunc_Deleptonization
#endif



// ================================
// IV. Set initialization functions
// ================================

#ifdef __CUDACC__
#  define FUNC_SPACE __device__ static
#else
#  define FUNC_SPACE            static
#endif

FUNC_SPACE SrcFunc_t SrcFunc_Ptr = Src_Deleptonization;

//-----------------------------------------------------------------------------------------
// Function    :  Src_SetFunc_Deleptonization
// Description :  Return the function pointer of the CPU/GPU source-term function
//
// Note        :  1. Invoked by Src_Init_Deleptonization()
//                2. Call-by-reference
//                3. Use either CPU or GPU but not both of them
//
// Parameter   :  SrcFunc_CPU/GPUPtr : CPU/GPU function pointer to be set
//
// Return      :  SrcFunc_CPU/GPUPtr
//-----------------------------------------------------------------------------------------
#ifdef __CUDACC__
__host__
void Src_SetFunc_Deleptonization( SrcFunc_t &SrcFunc_GPUPtr )
{
   CUDA_CHECK_ERROR(  cudaMemcpyFromSymbol( &SrcFunc_GPUPtr, SrcFunc_Ptr, sizeof(SrcFunc_t) )  );
}

#elif ( !defined GPU )

void Src_SetFunc_Deleptonization( SrcFunc_t &SrcFunc_CPUPtr )
{
   SrcFunc_CPUPtr = SrcFunc_Ptr;
}

#endif // #ifdef __CUDACC__ ... elif ...



#ifndef __CUDACC__

// local function prototypes
void Src_SetAuxArray_Deleptonization( double [], int [] );
void Src_SetFunc_Deleptonization( SrcFunc_t & );

//-----------------------------------------------------------------------------------------
// Function    :  Src_Init_Deleptonization
// Description :  Initialize the deleptonization source term
//
// Note        :  1. Set auxiliary arrays by invoking Src_SetAuxArray_*()
//                2. Set the source-term function by invoking Src_SetFunc_*()
//                   --> Unlike other modules (e.g., EoS), here we use either CPU or GPU but not
//                       both of them
//                3. Invoked by Src_Init()
//                4. Add "#ifndef __CUDACC__" since this routine is only useful on CPU
//
// Parameter   :  None
//
// Return      :  None
//-----------------------------------------------------------------------------------------
void Src_Init_Deleptonization()
{

   Src_SetAuxArray_Deleptonization( SRC_TERMS.Dlep_AuxArray_Flt, SRC_TERMS.Dlep_AuxArray_Int );

   Src_SetFunc_Deleptonization( SRC_TERMS.Dlep_FuncPtr );

} // FUNCTION : Src_Init_Deleptonization



//-----------------------------------------------------------------------------------------
// Function    :  Src_End_Deleptonization
// Description :  Release the resources used by the deleptonization source term
//
// Note        :  1. Invoked by Src_End()
//                2. Add "#ifndef __CUDACC__" since this routine is only useful on CPU
//
// Parameter   :  None
//
// Return      :  None
//-----------------------------------------------------------------------------------------
void Src_End_Deleptonization()
{

// TBF

} // FUNCTION : Src_End_Deleptonization

#endif // #ifndef __CUDACC__
