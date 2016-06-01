#include "Copyright.h"
#include "GAMER.h"




//-------------------------------------------------------------------------------------------------------
// Function    :  Flag_UserCriteria
// Description :  Check if the element (i,j,k) of the input data satisfies the user-defined flag criteria
//
// Note        :  Users can put their favorite flag criteria in this function
//
// Parameter   :  i,j,k       : Indices of the targeted element in the patch ptr[ amr->FluSg[lv] ][lv][PID]
//                lv          : Refinement level of the targeted patch
//                PID         : ID of the targeted patch
//                Threshold   : User-provided threshold for the flag operation, which is loaded from the 
//                              file "Input__Flag_User"
//
// Return      :  "true"  if the flag criteria are satisfied
//                "false" if the flag criteria are not satisfied
//-------------------------------------------------------------------------------------------------------
bool Flag_UserCriteria( const int i, const int j, const int k, const int lv, const int PID, const double Threshold )
{

   const double dh     = amr->dh[lv];                                                  // grid size
   const int    scale  = amr->scale[lv];                                               // grid scale
   const double Pos[3] = { amr->patch[0][lv][PID]->EdgeL[0] + (i+0.5)*dh,              // x,y,z position
                           amr->patch[0][lv][PID]->EdgeL[1] + (j+0.5)*dh,
                           amr->patch[0][lv][PID]->EdgeL[2] + (k+0.5)*dh  };  
   bool Flag = false;

   const double Center[3] = { 0.5*amr->BoxSize[0], 0.5*amr->BoxSize[1], 0.5*amr->BoxSize[2] };
   const double dr[3]     = { Pos[0]-Center[0], Pos[1]-Center[1], Pos[2]-Center[2] };
   const double Radius    = sqrt( dr[0]*dr[0] + dr[1]*dr[1] + dr[2]*dr[2] );

   Flag = Radius < 0.1;

   /*
   if ( amr->patch[0][lv][PID]->son == -1 )     Flag = amr->patch[0][lv][PID]->NPar > 0;
   else                                         Flag = Par_CountParticleInDescendant( lv, PID ) > 0;
   */

   return Flag;

} // FUNCTION : Flag_UserCriteria

