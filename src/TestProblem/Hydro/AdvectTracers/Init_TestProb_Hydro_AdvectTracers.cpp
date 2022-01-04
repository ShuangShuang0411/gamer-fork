#include "GAMER.h"
#include "TestProb.h"


// problem-specific global variables
// =======================================================================================
static double Advect_Dens_Bg;       // background mass density
static double Advect_Pres_Bg;       // background pressure
       double Advect_Vel[3];        // gas velocity
       int    Advect_NPar[3];       // particles on a side

// =======================================================================================

// problem-specific function prototypes
#ifdef PARTICLE
void Par_Init_ByFunction_AdvectTracers( const long NPar_ThisRank, const long NPar_AllRank,
                                        real *ParMass, real *ParPosX, real *ParPosY, real *ParPosZ,
                                        real *ParVelX, real *ParVelY, real *ParVelZ, real *ParTime,
                                        real *ParType, real *AllAttribute[PAR_NATT_TOTAL] );
#endif

bool Flag_AdvectTracers( const int i, const int j, const int k, const int lv, 
                         const int PID, const double *Threshold );

//-------------------------------------------------------------------------------------------------------
// Function    :  Validate
// Description :  Validate the compilation flags and runtime parameters for this test problem
//
// Note        :  None
//
// Parameter   :  None
//
// Return      :  None
//-------------------------------------------------------------------------------------------------------
void Validate()
{

   if ( MPI_Rank == 0 )    Aux_Message( stdout, "   Validating test problem %d ...\n", TESTPROB_ID );


#  if ( MODEL != HYDRO )
   Aux_Error( ERROR_INFO, "MODEL != HYDRO !!\n" );
#  endif

#  ifndef TRACER
   Aux_Error( ERROR_INFO, "TRACER must be enabled !!\n" );
#  endif

#  ifdef GRAVITY
   Aux_Error( ERROR_INFO, "GRAVITY must be disabled !!\n" );
#  endif

#  ifdef COMOVING
   Aux_Error( ERROR_INFO, "COMOVING must be disabled !!\n" );
#  endif

   if ( !OPT__INIT_RESTRICT )
      Aux_Error( ERROR_INFO, "OPT__INIT_RESTRICT must be enabled !!\n" );


   if ( MPI_Rank == 0 )    Aux_Message( stdout, "   Validating test problem %d ... done\n", TESTPROB_ID );

} // FUNCTION : Validate



#if ( MODEL == HYDRO )
//-------------------------------------------------------------------------------------------------------
// Function    :  SetParameter
// Description :  Load and set the problem-specific runtime parameters
//
// Note        :  1. Filename is set to "Input__TestProb" by default
//                2. Major tasks in this function:
//                   (1) load the problem-specific runtime parameters
//                   (2) set the problem-specific derived parameters
//                   (3) reset other general-purpose parameters if necessary
//                   (4) make a note of the problem-specific parameters
//
// Parameter   :  None
//
// Return      :  None
//-------------------------------------------------------------------------------------------------------
void SetParameter()
{

   if ( MPI_Rank == 0 )    Aux_Message( stdout, "   Setting runtime parameters ...\n" );


// (1) load the problem-specific runtime parameters
   const char FileName[] = "Input__TestProb";
   ReadPara_t *ReadPara  = new ReadPara_t;

// add parameters in the following format:
// --> note that VARIABLE, DEFAULT, MIN, and MAX must have the same data type
// --> some handy constants (e.g., NoMin_int, Eps_float, ...) are defined in "include/ReadPara.h"
// ********************************************************************************************************************************
// ReadPara->Add( "KEY_IN_THE_FILE",   &VARIABLE_ADDRESS,      DEFAULT,      MIN,              MAX               );
// ********************************************************************************************************************************
   ReadPara->Add( "Advect_Dens_Bg",     &Advect_Dens_Bg,      -1.0,          Eps_double,       NoMax_double      );
   ReadPara->Add( "Advect_Pres_Bg",     &Advect_Pres_Bg,      -1.0,          Eps_double,       NoMax_double      );
   ReadPara->Add( "Advect_VelX",        &Advect_Vel[0],       -1.0,          NoMin_double,     NoMax_double      );
   ReadPara->Add( "Advect_VelY",        &Advect_Vel[1],       -1.0,          NoMin_double,     NoMax_double      );
   ReadPara->Add( "Advect_VelZ",        &Advect_Vel[2],       -1.0,          NoMin_double,     NoMax_double      );
   ReadPara->Add( "Advect_NparX",       &Advect_NPar[0],       32,           8,                NoMax_int         );
   ReadPara->Add( "Advect_NparY",       &Advect_NPar[1],       32,           8,                NoMax_int         );
   ReadPara->Add( "Advect_NparZ",       &Advect_NPar[2],       32,           8,                NoMax_int         );

   ReadPara->Read( FileName );

   delete ReadPara;


// (2) reset other general-purpose parameters
//     --> a helper macro PRINT_WARNING is defined in TestProb.h
   const double End_T_Default    = 5.0e-3;
   const long   End_Step_Default = __INT_MAX__;

   if ( END_STEP < 0 ) {
      END_STEP = End_Step_Default;
      PRINT_WARNING( "END_STEP", END_STEP, FORMAT_LONG );
   }

   if ( END_T < 0.0 ) {
      END_T = End_T_Default;
      PRINT_WARNING( "END_T", END_T, FORMAT_REAL );
   }


// (4) make a note
   if ( MPI_Rank == 0 )
   {
      Aux_Message( stdout, "=============================================================================\n" );
      Aux_Message( stdout, "  test problem ID           = %d\n",     TESTPROB_ID );
      Aux_Message( stdout, "  background mass density   = %13.7e\n", Advect_Dens_Bg );
      Aux_Message( stdout, "  background pressure       = %13.7e\n", Advect_Pres_Bg );
      Aux_Message( stdout, "=============================================================================\n" );
   }


   if ( MPI_Rank == 0 )    Aux_Message( stdout, "   Setting runtime parameters ... done\n" );

} // FUNCTION : SetParameter



//-------------------------------------------------------------------------------------------------------
// Function    :  SetGridIC
// Description :  Set the problem-specific initial condition on grids
//
// Note        :  1. This function may also be used to estimate the numerical errors when OPT__OUTPUT_USER is enabled
//                   --> In this case, it should provide the analytical solution at the given "Time"
//                2. This function will be invoked by multiple OpenMP threads when OPENMP is enabled
//                   --> Please ensure that everything here is thread-safe
//                3. Even when DUAL_ENERGY is adopted for HYDRO, one does NOT need to set the dual-energy variable here
//                   --> It will be calculated automatically
//
// Parameter   :  fluid    : Fluid field to be initialized
//                x/y/z    : Physical coordinates
//                Time     : Physical time
//                lv       : Target refinement level
//                AuxArray : Auxiliary array
//
// Return      :  fluid
//-------------------------------------------------------------------------------------------------------
void SetGridIC( real fluid[], const double x, const double y, const double z, const double Time,
                const int lv, double AuxArray[] )
{

   double Eint, Etot;

   const double MomX = Advect_Dens_Bg*Advect_Vel[0];
   const double MomY = Advect_Dens_Bg*Advect_Vel[1];
   const double MomZ = Advect_Dens_Bg*Advect_Vel[2];

   // compute the total gas energy
   Eint = EoS_DensPres2Eint_CPUPtr( Advect_Dens_Bg, Advect_Pres_Bg, NULL, EoS_AuxArray_Flt,
                                    EoS_AuxArray_Int, h_EoS_Table );    // assuming EoS requires no passive scalars

   Etot = Hydro_ConEint2Etot( Advect_Dens_Bg, MomX, MomY, MomZ, Eint, 0.0 );      // do NOT include magnetic energy here

   fluid[DENS] = Advect_Dens_Bg;
   fluid[MOMX] = MomX;
   fluid[MOMY] = MomY;
   fluid[MOMZ] = MomZ;
   fluid[ENGY] = Etot;

} // FUNCTION : SetGridIC

#endif // #if ( MODEL == HYDRO )



//-------------------------------------------------------------------------------------------------------
// Function    :  Init_TestProb_Hydro_AdvectTracers
// Description :  Test problem initializer
//
// Note        :  None
//
// Parameter   :  None
//
// Return      :  None
//-------------------------------------------------------------------------------------------------------
void Init_TestProb_Hydro_AdvectTracers()
{

   if ( MPI_Rank == 0 )    Aux_Message( stdout, "%s ...\n", __FUNCTION__ );


// validate the compilation flags and runtime parameters
   Validate();


#  if ( MODEL == HYDRO )
// set the problem-specific runtime parameters
   SetParameter();


// set the function pointers of various problem-specific routines
   Init_Function_User_Ptr        = SetGridIC;
   Output_User_Ptr               = NULL;
   Flag_User_Ptr                 = Flag_AdvectTracers;
   Mis_GetTimeStep_User_Ptr      = NULL;
   Aux_Record_User_Ptr           = NULL;
   BC_User_Ptr                   = NULL;
   Flu_ResetByUser_Func_Ptr      = NULL;
   End_User_Ptr                  = NULL;
#  ifdef PARTICLE
   Par_Init_ByFunction_Ptr       = Par_Init_ByFunction_AdvectTracers;
#  endif
#  endif // #if ( MODEL == HYDRO )


   if ( MPI_Rank == 0 )    Aux_Message( stdout, "%s ... done\n", __FUNCTION__ );

} // FUNCTION : Init_TestProb_Hydro_AdvectTracers

bool Flag_AdvectTracers( const int i, const int j, const int k, const int lv, 
                         const int PID, const double *Threshold )
{

   const double dh     = amr->dh[lv];                                                  // grid size
   const double Pos[3] = { amr->patch[0][lv][PID]->EdgeL[0] + (i+0.5)*dh,              // x,y,z position
                           amr->patch[0][lv][PID]->EdgeL[1] + (j+0.5)*dh,
                           amr->patch[0][lv][PID]->EdgeL[2] + (k+0.5)*dh  };

   const double Center[3] = { 0.5*amr->BoxSize[0], 0.5*amr->BoxSize[1], 0.5*amr->BoxSize[2] };
   const double dr[3]     = { Pos[0]-Center[0], Pos[1]-Center[1], Pos[2]-Center[2] };
   const double Radius    = sqrt( dr[0]*dr[0] + dr[1]*dr[1] + dr[2]*dr[2] );

   bool Flag = Radius < Threshold[0];

   return Flag;

}