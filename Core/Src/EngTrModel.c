/*
 * File: EngTrModel.c
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

#include "EngTrModel.h"
#include "EngTrModel_private.h"

/* Named constants for Chart: '<Root>/ShiftLogic' */
#define EngTrModel_CALL_EVENT          (-1)
#define EngTrModel_IN_downshifting     ((uint8_T)1U)
#define EngTrModel_IN_first            ((uint8_T)1U)
#define EngTrModel_IN_fourth           ((uint8_T)2U)
#define EngTrModel_IN_second           ((uint8_T)3U)
#define EngTrModel_IN_steady_state     ((uint8_T)2U)
#define EngTrModel_IN_third            ((uint8_T)4U)
#define EngTrModel_IN_upshifting       ((uint8_T)3U)
#define EngTrModel_event_DOWN          (0)
#define EngTrModel_event_UP            (1)

/* Block signals (default storage) */
B_EngTrModel_T EngTrModel_B;

/* Block states (default storage) */
DW_EngTrModel_T EngTrModel_DW;

/* External inputs (root inport signals with default storage) */
ExtU_EngTrModel_T EngTrModel_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_EngTrModel_T EngTrModel_Y;

/* Real-time model */
RT_MODEL_EngTrModel_T EngTrModel_M_;
RT_MODEL_EngTrModel_T *const EngTrModel_M = &EngTrModel_M_;

/* Forward declaration for local functions */
static void EngTrModel_gear_state(const int32_T *sfEvent);
real_T look2_binlxpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[],
                     const real_T table[], const uint32_T maxIndex[], uint32_T
                     stride)
{
  real_T frac;
  uint32_T bpIndices[2];
  real_T fractions[2];
  real_T yL_1d;
  uint32_T iRght;
  uint32_T bpIdx;
  uint32_T iLeft;

  /* Column-major Lookup 2-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search */
    bpIdx = maxIndex[0U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[0U];
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex[0U] - 1U;
    frac = (u0 - bp0[maxIndex[0U] - 1U]) / (bp0[maxIndex[0U]] - bp0[maxIndex[0U]
      - 1U]);
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = (u1 - bp1[0U]) / (bp1[1U] - bp1[0U]);
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search */
    bpIdx = maxIndex[1U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[1U];
    while (iRght - iLeft > 1U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u1 - bp1[iLeft]) / (bp1[iLeft + 1U] - bp1[iLeft]);
  } else {
    iLeft = maxIndex[1U] - 1U;
    frac = (u1 - bp1[maxIndex[1U] - 1U]) / (bp1[maxIndex[1U]] - bp1[maxIndex[1U]
      - 1U]);
  }

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  yL_1d = (table[bpIdx + 1U] - table[bpIdx]) * fractions[0U] + table[bpIdx];
  bpIdx += stride;
  return (((table[bpIdx + 1U] - table[bpIdx]) * fractions[0U] + table[bpIdx]) -
          yL_1d) * frac + yL_1d;
}

real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
                     uint32_T maxIndex)
{
  real_T frac;
  uint32_T iRght;
  uint32_T iLeft;
  uint32_T bpIdx;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex]) {
    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  return (table[iLeft + 1U] - table[iLeft]) * frac + table[iLeft];
}

/* Function for Chart: '<Root>/ShiftLogic' */
static void EngTrModel_gear_state(const int32_T *sfEvent)
{
  switch (EngTrModel_DW.is_gear_state) {
   case EngTrModel_IN_first:
    if (*sfEvent == EngTrModel_event_UP) {
      EngTrModel_DW.is_gear_state = EngTrModel_IN_second;
      EngTrModel_B.Gear = 2.0;
    }
    break;

   case EngTrModel_IN_fourth:
    if (*sfEvent == EngTrModel_event_DOWN) {
      EngTrModel_DW.is_gear_state = EngTrModel_IN_third;
      EngTrModel_B.Gear = 3.0;
    }
    break;

   case EngTrModel_IN_second:
    switch (*sfEvent) {
     case EngTrModel_event_UP:
      EngTrModel_DW.is_gear_state = EngTrModel_IN_third;
      EngTrModel_B.Gear = 3.0;
      break;

     case EngTrModel_event_DOWN:
      EngTrModel_DW.is_gear_state = EngTrModel_IN_first;
      EngTrModel_B.Gear = 1.0;
      break;
    }
    break;

   case EngTrModel_IN_third:
    switch (*sfEvent) {
     case EngTrModel_event_UP:
      EngTrModel_DW.is_gear_state = EngTrModel_IN_fourth;
      EngTrModel_B.Gear = 4.0;
      break;

     case EngTrModel_event_DOWN:
      EngTrModel_DW.is_gear_state = EngTrModel_IN_second;
      EngTrModel_B.Gear = 2.0;
      break;
    }
    break;
  }
}

real_T rt_powd_snf(real_T u0, real_T u1)
{
  real_T y;
  real_T tmp;
  real_T tmp_0;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = (rtNaN);
  } else {
    tmp = fabs(u0);
    tmp_0 = fabs(u1);
    if (rtIsInf(u1)) {
      if (tmp == 1.0) {
        y = 1.0;
      } else if (tmp > 1.0) {
        if (u1 > 0.0) {
          y = (rtInf);
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = (rtInf);
      }
    } else if (tmp_0 == 0.0) {
      y = 1.0;
    } else if (tmp_0 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > floor(u1))) {
      y = (rtNaN);
    } else {
      y = pow(u0, u1);
    }
  }

  return y;
}

/* Model step function */
void EngTrModel_step(void)
{
  int32_T sfEvent;
  real_T InterpDown;
  real_T InterpUp;

  /* DiscreteIntegrator: '<S1>/Discrete-Time Integrator' */
  EngTrModel_B.EngineRPM = EngTrModel_DW.DiscreteTimeIntegrator_DSTATE;

  /* Outport: '<Root>/EngineSpeed' */
  EngTrModel_Y.EngineSpeed = EngTrModel_B.EngineRPM;

  /* UnitConversion: '<S4>/Unit Conversion' incorporates:
   *  DiscreteIntegrator: '<S4>/WheelSpeed'
   *  Gain: '<S8>/ToLinearSpeed'
   */
  /* Unit Conversion - from: ft/min to: mph
     Expression: output = (0.0113636*input) + (0) */
  EngTrModel_B.VehicleSpeed = 6.2831853071795862 *
    EngTrModel_DW.WheelSpeed_DSTATE * 0.011363636363636364;

  /* Chart: '<Root>/ShiftLogic' */
  sfEvent = EngTrModel_CALL_EVENT;
  if (EngTrModel_DW.temporalCounter_i1 < MAX_uint32_T) {
    EngTrModel_DW.temporalCounter_i1++;
  }

  if (EngTrModel_DW.is_active_c1_EngTrModel == 0U) {
    EngTrModel_DW.is_active_c1_EngTrModel = 1U;
    EngTrModel_DW.is_active_gear_state = 1U;
    EngTrModel_DW.is_gear_state = EngTrModel_IN_first;
    EngTrModel_B.Gear = 1.0;
    EngTrModel_DW.is_active_selection_state = 1U;
    EngTrModel_DW.is_selection_state = EngTrModel_IN_steady_state;
  } else {
    if (EngTrModel_DW.is_active_gear_state != 0U) {
      EngTrModel_gear_state(&sfEvent);
    }

    if (EngTrModel_DW.is_active_selection_state != 0U) {
      /* Outputs for Function Call SubSystem: '<S2>/ComputeThreshold' */
      /* Lookup_n-D: '<S5>/InterpDown' incorporates:
       *  Inport: '<Root>/Throttle'
       */
      InterpDown = look2_binlxpw(EngTrModel_U.Throttle, EngTrModel_B.Gear,
        EngTrModel_ConstP.InterpDown_bp01Data, EngTrModel_ConstP.pooled2,
        EngTrModel_ConstP.InterpDown_tableData, EngTrModel_ConstP.pooled6, 6U);

      /* Lookup_n-D: '<S5>/InterpUp' incorporates:
       *  Inport: '<Root>/Throttle'
       */
      InterpUp = look2_binlxpw(EngTrModel_U.Throttle, EngTrModel_B.Gear,
        EngTrModel_ConstP.InterpUp_bp01Data, EngTrModel_ConstP.pooled2,
        EngTrModel_ConstP.InterpUp_tableData, EngTrModel_ConstP.pooled6, 6U);

      /* End of Outputs for SubSystem: '<S2>/ComputeThreshold' */
      switch (EngTrModel_DW.is_selection_state) {
       case EngTrModel_IN_downshifting:
        if ((EngTrModel_DW.temporalCounter_i1 >= (uint32_T)2.0) &&
            (EngTrModel_B.VehicleSpeed <= InterpDown)) {
          sfEvent = EngTrModel_event_DOWN;
          if (EngTrModel_DW.is_active_gear_state != 0U) {
            EngTrModel_gear_state(&sfEvent);
          }

          EngTrModel_DW.is_selection_state = EngTrModel_IN_steady_state;
        } else {
          if (EngTrModel_B.VehicleSpeed > InterpDown) {
            EngTrModel_DW.is_selection_state = EngTrModel_IN_steady_state;
          }
        }
        break;

       case EngTrModel_IN_steady_state:
        if (EngTrModel_B.VehicleSpeed > InterpUp) {
          EngTrModel_DW.is_selection_state = EngTrModel_IN_upshifting;
          EngTrModel_DW.temporalCounter_i1 = 0U;
        } else {
          if (EngTrModel_B.VehicleSpeed < InterpDown) {
            EngTrModel_DW.is_selection_state = EngTrModel_IN_downshifting;
            EngTrModel_DW.temporalCounter_i1 = 0U;
          }
        }
        break;

       case EngTrModel_IN_upshifting:
        if ((EngTrModel_DW.temporalCounter_i1 >= (uint32_T)2.0) &&
            (EngTrModel_B.VehicleSpeed >= InterpUp)) {
          sfEvent = EngTrModel_event_UP;
          if (EngTrModel_DW.is_active_gear_state != 0U) {
            EngTrModel_gear_state(&sfEvent);
          }

          EngTrModel_DW.is_selection_state = EngTrModel_IN_steady_state;
        } else {
          if (EngTrModel_B.VehicleSpeed < InterpUp) {
            EngTrModel_DW.is_selection_state = EngTrModel_IN_steady_state;
          }
        }
        break;
      }
    }
  }

  /* End of Chart: '<Root>/ShiftLogic' */

  /* Lookup_n-D: '<S7>/Table' */
  InterpDown = look1_binlxpw(EngTrModel_B.Gear, EngTrModel_ConstP.pooled2,
    EngTrModel_ConstP.Table_tableData, 3U);

  /* Gain: '<S4>/FinalDriveRatio2' incorporates:
   *  DiscreteIntegrator: '<S4>/WheelSpeed'
   */
  EngTrModel_B.TransmissionRPM = 3.23 * EngTrModel_DW.WheelSpeed_DSTATE;

  /* Product: '<S6>/SpeedRatio' incorporates:
   *  Product: '<S7>/Product1'
   */
  if( EngTrModel_B.EngineRPM != 0)
    InterpUp = InterpDown * EngTrModel_B.TransmissionRPM / EngTrModel_B.EngineRPM;
  else
    InterpUp = 0.0;

  /* Fcn: '<S6>/Impeller' incorporates:
   *  Lookup_n-D: '<S6>/FactorK'
   *  Product: '<S6>/Quotient'
   */
  EngTrModel_B.ImpellerTorque = rt_powd_snf(EngTrModel_B.EngineRPM /
    look1_binlxpw(InterpUp, EngTrModel_ConstP.pooled5,
                  EngTrModel_ConstP.FactorK_tableData, 20U), 2.0);

  /* Lookup_n-D: '<S6>/TorqueRatio' */
  InterpUp = look1_binlxpw(InterpUp, EngTrModel_ConstP.pooled5,
    EngTrModel_ConstP.TorqueRatio_tableData, 20U);

  /* Product: '<S7>/Product' incorporates:
   *  Product: '<S6>/Turbine'
   */
  EngTrModel_B.OutputTorque = EngTrModel_B.ImpellerTorque * InterpUp *
    InterpDown;

  /* Outport: '<Root>/Gear' */
  EngTrModel_Y.Gear = EngTrModel_B.Gear;

  /* Outport: '<Root>/VehicleSpeed' */
  EngTrModel_Y.VehicleSpeed = EngTrModel_B.VehicleSpeed;

  /* Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator' incorporates:
   *  Gain: '<S1>/EnginePlusImpellerInertia'
   *  Inport: '<Root>/Throttle'
   *  Lookup_n-D: '<S1>/EngineTorque'
   *  Sum: '<S1>/Sum'
   */
  EngTrModel_DW.DiscreteTimeIntegrator_DSTATE += (look2_binlxpw
    (EngTrModel_U.Throttle, EngTrModel_B.EngineRPM,
     EngTrModel_ConstP.EngineTorque_bp01Data,
     EngTrModel_ConstP.EngineTorque_bp02Data,
     EngTrModel_ConstP.EngineTorque_tableData,
     EngTrModel_ConstP.EngineTorque_maxIndex, 10U) - EngTrModel_B.ImpellerTorque)
    * 45.472138452209627 * 0.04;

  /* Signum: '<S4>/Sign' */
  if (EngTrModel_B.VehicleSpeed < 0.0) {
    InterpDown = -1.0;
  } else if (EngTrModel_B.VehicleSpeed > 0.0) {
    InterpDown = 1.0;
  } else if (EngTrModel_B.VehicleSpeed == 0.0) {
    InterpDown = 0.0;
  } else {
    InterpDown = (rtNaN);
  }

  /* End of Signum: '<S4>/Sign' */

  /* Update for DiscreteIntegrator: '<S4>/WheelSpeed' incorporates:
   *  Fcn: '<S4>/RoadLoad'
   *  Gain: '<S4>/FinalDriveRatio1'
   *  Gain: '<S4>/VehicleInertia'
   *  Inport: '<Root>/Brake'
   *  Product: '<S4>/SignedLoad'
   *  Sum: '<S4>/Sum'
   *  Sum: '<S4>/Sum1'
   */
  EngTrModel_DW.WheelSpeed_DSTATE += (3.23 * EngTrModel_B.OutputTorque - ((0.02 *
    rt_powd_snf(EngTrModel_B.VehicleSpeed, 2.0) + 40.0) +
    EngTrModel_U.BrakeTorque) * InterpDown) * 0.082684618362373577 * 0.04;
}

/* Model initialize function */
void EngTrModel_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));
}

/* Model terminate function */
void EngTrModel_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
