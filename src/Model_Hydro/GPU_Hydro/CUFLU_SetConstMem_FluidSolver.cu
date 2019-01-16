#include "Macro.h"
#include "CUFLU.h"

#if ( MODEL == HYDRO  &&  defined GPU )




#ifdef UNSPLIT_GRAVITY
#include "CUPOT.h"
__constant__ double c_ExtAcc_AuxArray[EXT_ACC_NAUX_MAX];

//-------------------------------------------------------------------------------------------------------
// Function    :  CUFLU_SetConstMem_FluidSolver_ExtAcc
// Description :  Set the constant memory of c_ExtAcc_AuxArray[] used by CUFLU_FluidSolver_CTU/MHM()
//
// Note        :  1. Adopt the suggested approach for CUDA version >= 5.0
//                2. Invoked by CUAPI_Init_ExternalAccPot()
//
// Parameter   :  None
//
// Return      :  0/-1 : successful/failed
//---------------------------------------------------------------------------------------------------
__host__
int CUFLU_SetConstMem_FluidSolver_ExtAcc( double h_ExtAcc_AuxArray[] )
{

   if (  cudaSuccess != cudaMemcpyToSymbol( c_ExtAcc_AuxArray, h_ExtAcc_AuxArray, EXT_ACC_NAUX_MAX*sizeof(double),
                                            0, cudaMemcpyHostToDevice)  )
      return -1;

   else
      return 0;

} // FUNCTION : CUFLU_SetConstMem_FluidSolver_ExtAcc
#endif // #ifdef UNSPLIT_GRAVITY



#if ( NCOMP_PASSIVE > 0 )
__constant__ int c_NormIdx[NCOMP_PASSIVE];

//-------------------------------------------------------------------------------------------------------
// Function    :  CUFLU_SetConstMem_FluidSolver_NormIdx
// Description :  Set the constant memory of c_NormIdx[] used by CUFLU_FluidSolver_CTU/MHM()
//
// Note        :  1. Adopt the suggested approach for CUDA version >= 5.0
//                2. Invoked by CUAPI_Set_Default_GPU_Parameter()
//
// Parameter   :  None
//
// Return      :  0/-1 : successful/failed
//---------------------------------------------------------------------------------------------------
__host__
int CUFLU_SetConstMem_FluidSolver_NormIdx( int h_NormIdx[] )
{

   if (  cudaSuccess != cudaMemcpyToSymbol( c_NormIdx, h_NormIdx, NCOMP_PASSIVE*sizeof(int),
                                            0, cudaMemcpyHostToDevice)  )
      return -1;

   else
      return 0;

} // FUNCTION : CUFLU_SetConstMem_FluidSolver_NormIdx

#else // #if ( NCOMP_PASSIVE > 0 )
__constant__ int *c_NormIdx = NULL;

#endif // #if ( NCOMP_PASSIVE > 0 ) ... else ...



__constant__ real c_dh_AllLv[NLEVEL][3];

//-------------------------------------------------------------------------------------------------------
// Function    :  CUFLU_SetConstMem_FluidSolver_dh
// Description :  Set the constant memory of c_dh_AllLv[] used by CUFLU_FluidSolver_CTU/MHM()
//
// Note        :  1. Adopt the suggested approach for CUDA version >= 5.0
//                2. Invoked by CUAPI_SetConstMem()
//
// Parameter   :  None
//
// Return      :  0/-1 : successful/failed
//---------------------------------------------------------------------------------------------------
__host__
int CUFLU_SetConstMem_FluidSolver_dh( real h_dh_AllLv[][3] )
{

   if (  cudaSuccess != cudaMemcpyToSymbol( c_dh_AllLv, h_dh_AllLv, NLEVEL*3*sizeof(real),
                                            0, cudaMemcpyHostToDevice)  )
      return -1;

   else
      return 0;

} // FUNCTION : CUFLU_SetConstMem_FluidSolver_dh


#endif // #if ( MODEL == HYDRO  &&  defined GPU )
