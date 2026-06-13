/*
 * File: EngTrModel.h
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

#ifndef RTW_HEADER_EngTrModel_h_
#define RTW_HEADER_EngTrModel_h_
#include <math.h>
#ifndef EngTrModel_COMMON_INCLUDES_
# define EngTrModel_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* EngTrModel_COMMON_INCLUDES_ */

#include "EngTrModel_types.h"
#include "rtGetNaN.h"
#include "rt_nonfinite.h"
#include "rtGetInf.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

/* Block signals (default storage) */
typedef struct {
  real_T EngineRPM;                    /* '<S1>/Discrete-Time Integrator' */
  real_T VehicleSpeed;                 /* '<S4>/Unit Conversion' */
  real_T TransmissionRPM;              /* '<S4>/FinalDriveRatio2' */
  real_T ImpellerTorque;               /* '<S6>/Impeller' */
  real_T OutputTorque;                 /* '<S7>/Product' */
  real_T Gear;                         /* '<Root>/ShiftLogic' */
} B_EngTrModel_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real_T DiscreteTimeIntegrator_DSTATE;/* '<S1>/Discrete-Time Integrator' */
  real_T WheelSpeed_DSTATE;            /* '<S4>/WheelSpeed' */
  struct {
    void *LoggedData[3];
  } PlotResults_PWORK;                 /* '<Root>/PlotResults' */

  uint32_T temporalCounter_i1;         /* '<Root>/ShiftLogic' */
  uint8_T is_active_c1_EngTrModel;     /* '<Root>/ShiftLogic' */
  uint8_T is_gear_state;               /* '<Root>/ShiftLogic' */
  uint8_T is_active_gear_state;        /* '<Root>/ShiftLogic' */
  uint8_T is_selection_state;          /* '<Root>/ShiftLogic' */
  uint8_T is_active_selection_state;   /* '<Root>/ShiftLogic' */
} DW_EngTrModel_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: DOWN_TABLE
   * Referenced by: '<S5>/InterpDown'
   */
  real_T InterpDown_tableData[24];

  /* Expression: DOWN_TH_BP
   * Referenced by: '<S5>/InterpDown'
   */
  real_T InterpDown_bp01Data[6];

  /* Pooled Parameter (Mixed Expressions)
   * Referenced by:
   *   '<S7>/Table'
   *   '<S5>/InterpDown'
   *   '<S5>/InterpUp'
   */
  real_T pooled2[4];

  /* Expression: UP_TABLE
   * Referenced by: '<S5>/InterpUp'
   */
  real_T InterpUp_tableData[24];

  /* Expression: UP_TH_BP
   * Referenced by: '<S5>/InterpUp'
   */
  real_T InterpUp_bp01Data[6];

  /* Expression: EMAP
   * Referenced by: '<S1>/EngineTorque'
   */
  real_T EngineTorque_tableData[110];

  /* Expression: TH_VEC
   * Referenced by: '<S1>/EngineTorque'
   */
  real_T EngineTorque_bp01Data[10];

  /* Expression: NE_VEC
   * Referenced by: '<S1>/EngineTorque'
   */
  real_T EngineTorque_bp02Data[11];

  /* Expression: [2.393 1.450 1.000 0.677]
   * Referenced by: '<S7>/Table'
   */
  real_T Table_tableData[4];

  /* Expression: Kfactor
   * Referenced by: '<S6>/FactorK'
   */
  real_T FactorK_tableData[21];

  /* Pooled Parameter (Expression: speedratio)
   * Referenced by:
   *   '<S6>/FactorK'
   *   '<S6>/TorqueRatio'
   */
  real_T pooled5[21];

  /* Expression: Torkratio
   * Referenced by: '<S6>/TorqueRatio'
   */
  real_T TorqueRatio_tableData[21];

  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<S5>/InterpDown'
   *   '<S5>/InterpUp'
   */
  uint32_T pooled6[2];

  /* Computed Parameter: EngineTorque_maxIndex
   * Referenced by: '<S1>/EngineTorque'
   */
  uint32_T EngineTorque_maxIndex[2];
} ConstP_EngTrModel_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T Throttle;                     /* '<Root>/Throttle' */
  real_T BrakeTorque;                  /* '<Root>/Brake' */
} ExtU_EngTrModel_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T EngineSpeed;                  /* '<Root>/EngineSpeed' */
  real_T VehicleSpeed;                 /* '<Root>/VehicleSpeed' */
  real_T Gear;                         /* '<Root>/Gear' */
} ExtY_EngTrModel_T;

/* Real-time Model Data Structure */
struct tag_RTM_EngTrModel_T {
  const char_T * volatile errorStatus;
};

/* Block signals (default storage) */
extern B_EngTrModel_T EngTrModel_B;

/* Block states (default storage) */
extern DW_EngTrModel_T EngTrModel_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_EngTrModel_T EngTrModel_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_EngTrModel_T EngTrModel_Y;

/* Constant parameters (default storage) */
extern const ConstP_EngTrModel_T EngTrModel_ConstP;

/* Model entry point functions */
extern void EngTrModel_initialize(void);
extern void EngTrModel_step(void);
extern void EngTrModel_terminate(void);

/* Real-time Model object */
extern RT_MODEL_EngTrModel_T *const EngTrModel_M;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'EngTrModel'
 * '<S1>'   : 'EngTrModel/Engine'
 * '<S2>'   : 'EngTrModel/ShiftLogic'
 * '<S3>'   : 'EngTrModel/Transmission'
 * '<S4>'   : 'EngTrModel/Vehicle'
 * '<S5>'   : 'EngTrModel/ShiftLogic/ComputeThreshold'
 * '<S6>'   : 'EngTrModel/Transmission/TorqueConverter'
 * '<S7>'   : 'EngTrModel/Transmission/TransmissionRatio'
 * '<S8>'   : 'EngTrModel/Vehicle/WheelSpeedToLinearSpeed'
 */
#endif                                 /* RTW_HEADER_EngTrModel_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
