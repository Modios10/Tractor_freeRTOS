/*
 * File: EngTrModel_private.h
 *
 * Code generated for Simulink model 'EngTrModel'.
 *
 * Model version                  : 1.259
 * Simulink Coder version         : 9.2 (R2019b) 18-Jul-2019
 * C/C++ source code generated on : Fri Nov 10 19:18:06 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: 32-bit Generic
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_EngTrModel_private_h_
#define RTW_HEADER_EngTrModel_private_h_
#include "rtwtypes.h"

extern real_T rt_powd_snf(real_T u0, real_T u1);
extern real_T look2_binlxpw(real_T u0, real_T u1, const real_T bp0[], const
  real_T bp1[], const real_T table[], const uint32_T maxIndex[], uint32_T stride);
extern real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);

#endif                                 /* RTW_HEADER_EngTrModel_private_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
