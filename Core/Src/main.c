#include <stdint.h>
#include "main.h"
#include "main2.h"
#include "lcd.h"
#include "user_i2c.h"
#include "EngTrModel.h" 
#include <stdio.h>
#include "user_uart.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define SERVO_TIMER_PSC         63U     /* 64 MHz / (63 + 1) = 1 MHz */
#define SERVO_TIMER_ARR         19999U  /* 20 ms period = 50 Hz */
#define SERVO_MIN_US            1000U
#define SERVO_NEUTRAL_US        1500U
#define SERVO_MAX_US            2275U

#define SERVO2_MIN_US           700U
#define SERVO2_NEUTRAL_US       1500U
#define SERVO2_MAX_US           2000U

#define VEH_SPEED_MAX           120U
#define VEH_SPEED_GAIN_PCT      250U

#define RX_BUF_SIZE             32U

/*dividers */
static volatile uint8_t g_lcd_divider = 0U;
static volatile uint8_t g_uart_divider = 0U;


/* --- Variables de Control de Estado (Dashboard Override) --- */
static volatile uint8_t  g_control_mode = 0U;          /* 0 = Manual, 1 = Remoto */
static volatile uint32_t g_dashboard_throttle_pct = 0U; 
static volatile uint32_t g_dashboard_brake = 0U;        
static uint32_t          g_current_physical_pct = 0U;  /* Lectura en tiempo real */
static uint32_t          g_locked_pot_pct = 0U;        /* Foto de posición bloqueada */

/* Buffer de recepción para comandos USART1 */
static char    g_rx_buffer[RX_BUF_SIZE];
static uint8_t g_rx_index = 0U;

/* Shared data between tasks */
typedef struct {
    uint32_t throttle_pct;
    uint32_t brake_torque;
    uint32_t engine_rpm;
    uint32_t vehicle_speed;
    uint32_t gear;
} VehicleData_t;

VehicleData_t g_state;


TickType_t t0;
TaskHandle_t TaskCHandle; //Control handle
TaskHandle_t TaskOHandle; //Output handle
TaskHandle_t TaskRXHandle; //RX parsing handle
QueueHandle_t xQueueRX; //Queue for USART1 received characters
SemaphoreHandle_t xMutexHandle; //mutex for shared state protection

//Task initialization function
void Task_control(void *pvParameters);
void Task_output(void *pvParameters);
void Task_rx_parse(void *pvParameters);

static void Inputs_Init(void);
static uint8_t BrakeButton_IsPressed(void);

//Function prototypes for ADC reading and conversion
static void ADC1_LocalInit(void);
static uint32_t ADC1_ReadRaw(void);
static uint32_t ToPercentFromAdc(uint32_t adc);

static void ServoOutputs_Init(void);

static void BuildLcdLine1(char *line, uint32_t throttle_pct, uint32_t gear, uint32_t vehicle_speed);

static void BuildLcdLine2(char *line, uint32_t engine_rpm);

static void ServoSetPulse1(uint32_t pulse_us);
static void ServoSetPulse2(uint32_t pulse_us);

static uint32_t VehicleSpeedToPercent(uint32_t vehicle_speed);
static uint32_t Servo1PulseFromVehicleSpeed(uint32_t vehicle_speed);
static uint32_t Servo2PulseFromVehicleSpeed(uint32_t vehicle_speed);

static void Parse_ESP_Command(const char *str);



//Main FreeRTOS Task function
int main(void){
    USER_SystemClock_Config();
    Inputs_Init();
    ADC1_LocalInit();
    GPIOA_init();
    USART1_Init();
    ServoOutputs_Init();
    USER_I2C1_Init();
    LCD_Init();
    LCD_Clear();

    EngTrModel_initialize();

    xTaskCreate(Task_control, "Control", 128, NULL, 3, &TaskCHandle); //Control task creation
    xTaskCreate(Task_output, "Output", 128, NULL, 1, &TaskOHandle); //Output task creation
    xTaskCreate(Task_rx_parse, "RX_Parse", 128, NULL, 2, &TaskRXHandle); //RX parsing task creation
    // Change this line in main():
    xQueueRX = xQueueCreate(RX_BUF_SIZE, sizeof(char)); // Create a queue for Usart1 commands
    xMutexHandle = xSemaphoreCreateMutex(); // Create a mutex for shared resource protection

    // Inicializar variables para arrancar limpiamente en modo manual
    g_current_physical_pct = ToPercentFromAdc(ADC1_ReadRaw());
    g_locked_pot_pct = g_current_physical_pct;

    vTaskStartScheduler();

    for(;;)
    {}
        
}


//Function to clear RX buffer and set brack and acceleration from dashboard
void Task_rx_parse(void *pvParameters)
{
    char c;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for(;;)
    {
        // Block until a character arrives in the queue
        if(xQueueReceive(xQueueRX, &c, portMAX_DELAY) == pdTRUE)
        {
            if(c != '\r') 
            {
                if(c == '\n')
                {
                    g_rx_buffer[g_rx_index] = '\0';
                    if(g_rx_index >= 2U)
                    {
                        Parse_ESP_Command(g_rx_buffer); 
                    }
                    g_rx_index = 0U;
                }
                else
                {
                    if(g_rx_index < (RX_BUF_SIZE - 1U)) g_rx_buffer[g_rx_index++] = c;
                    else g_rx_index = 0U; 
                }
            }
        }
        vTaskDelayUntil(&xLastWakeTime, 50U); // Small delay to prevent task hogging CPU (adjust as needed)
    }
}


void Task_control(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    // Local variables to hold the input state before saving it to the Mutex
    uint32_t current_throttle = 0U;
    uint32_t current_brake = 0U;

    for(;;)
    {
        //STEP 1: READ INPUTS (Formerly Task_input)
        uint32_t adc_raw = ADC1_ReadRaw();
        g_current_physical_pct = ToPercentFromAdc(adc_raw);

        // Check if user is physically overriding the remote control
        if(g_control_mode == 1U)
        {
            int32_t delta = (int32_t)g_current_physical_pct - (int32_t)g_locked_pot_pct;
            if(delta < 0) delta = -delta;
            if(delta > 6) g_control_mode = 0U; 
        }

        // Determine priority: Hardware Brake > Remote > Manual
        if(BrakeButton_IsPressed())
        {
            g_control_mode = 0U; 
            current_throttle = 0U;
            current_brake = 4500U;
            EngTrModel_U.Throttle = 1.5;
            EngTrModel_U.BrakeTorque = 4500.0;
        }
        else
        {
            if(g_control_mode == 0U)
            {
                /* MANUAL */
                current_throttle = g_current_physical_pct;
                current_brake = 0U;
                EngTrModel_U.Throttle = 1.5 + ((double)current_throttle * 98.5 / 100.0);
                EngTrModel_U.BrakeTorque = 0.0;
            }
            else
            {
                /* REMOTO */
                current_throttle = g_dashboard_throttle_pct;
                current_brake = g_dashboard_brake;
                EngTrModel_U.Throttle = 1.5 + ((double)current_throttle * 98.5 / 100.0);
                EngTrModel_U.BrakeTorque = (current_brake > 0U) ? (double)current_brake : 0.0;
            }
        }

        //STEP 2: COMPUTE PHYSICS
        EngTrModel_step();

        //STEP 3: UPDATE SHARED STATE (Mutex Protected)
  
        if(xSemaphoreTake(xMutexHandle, portMAX_DELAY) == pdTRUE)
        {
            g_state.engine_rpm = (uint32_t)EngTrModel_Y.EngineSpeed;
            g_state.vehicle_speed = (uint32_t)EngTrModel_Y.VehicleSpeed;
            g_state.gear = (uint32_t)EngTrModel_Y.Gear;
            g_state.throttle_pct = current_throttle;
            g_state.brake_torque = current_brake;
            xSemaphoreGive(xMutexHandle);
        }

        // Local limits for servo calculations
        uint32_t safe_vehicle_speed = (uint32_t)EngTrModel_Y.VehicleSpeed;
        
        //STEP 4: OUTPUT TO HARDWARE ACTUATORS
        ServoSetPulse1(Servo1PulseFromVehicleSpeed(safe_vehicle_speed));
        ServoSetPulse2(Servo2PulseFromVehicleSpeed(safe_vehicle_speed));

        /* Wait strictly until the next 40ms cycle */
        vTaskDelayUntil(&xLastWakeTime, 40U); 
    }
}

void Task_output(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    VehicleData_t local_state; // Create a local copy
   
    for(;;)
    {
        // 1. Lock the Mutex and copy the data instantly
        if(xSemaphoreTake(xMutexHandle, portMAX_DELAY) == pdTRUE)
        {
            local_state = g_state; // Copy the whole struct
            xSemaphoreGive(xMutexHandle); // Unlock!
        }
        


        g_lcd_divider++;
        if(g_lcd_divider >= 10U)
        {
            g_lcd_divider = 0U;
            char line1[17];
            char line2[17];
            // 2. Use local_state to print!
            BuildLcdLine1(line1, local_state.throttle_pct, local_state.gear, local_state.vehicle_speed);
            BuildLcdLine2(line2, local_state.engine_rpm);
            LCD_Clear();
            LCD_Print(line1, line2);
        }

        g_uart_divider++;
        if(g_uart_divider >= 5U)
        {
            g_uart_divider = 0U;
            // Use local_state here too!
            USART1_print("ENG="); USART1_printNumber(local_state.engine_rpm);
            USART1_print(" VEH="); USART1_printNumber(local_state.vehicle_speed);
            USART1_print(" G="); USART1_printNumber(local_state.gear);
            USART1_print(" TH="); USART1_printNumber(local_state.throttle_pct);
            USART1_print(" BR="); USART1_printNumber(local_state.brake_torque);
            USART1_print("\r\n");
        }
        vTaskDelayUntil(&xLastWakeTime, 500U); 
    }
}


/* --- Hardware init functions --- */
static void Inputs_Init(void)
{
    RCC->APB2ENR |= (1U << 2U); /* GPIOA */
    RCC->APB2ENR |= (1U << 4U); /* GPIOC */

    /* PA1 analog input */
    GPIOA->CRL &= ~(0xFU << 4U);

    /* PC13 input pull-up */
    GPIOC->CRH &= ~(0xFU << 20U);
    GPIOC->CRH |=  (0x8U << 20U);
    GPIOC->BSRR = (1U << 13U);
}

// Returns 1 if brake button is pressed, 0 otherwise
static uint8_t BrakeButton_IsPressed(void)
{
    return ((GPIOC->IDR & (1U << 13U)) == 0U);
}




// Función para inicializar el ADC1 y calibrarlo
static void ADC1_LocalInit(void)
{
    RCC->APB2ENR |= (1U << 9U);

    ADC1->SQR1 &= ~(0xFUL << 20U);
    ADC1->SQR3 &= ~(0x1FUL << 0U);
    ADC1->SQR3 |=  (0x1UL << 0U);
    ADC1->SMPR2 &= ~(0x7UL << 3U);
    ADC1->SMPR2 |=  (0x7UL << 3U);

    ADC1->CR2 &= ~(1U << 11U); 
    ADC1->CR2 |= (1U << 0U);   

    for(volatile int i = 0; i < 10000; i++);

    ADC1->CR2 |= (1U << 3U);
    while(ADC1->CR2 & (1U << 3U));

    ADC1->CR2 |= (1U << 2U);
    while(ADC1->CR2 & (1U << 2U));
}

// Función para leer el valor bruto del ADC1 y convertirlo a porcentaje
static uint32_t ADC1_ReadRaw(void)
{
    ADC1->CR2 |= (1U << 0U);

    for(volatile uint32_t timeout = 0U; timeout < 200000U; timeout++)
    {
        if(ADC1->SR & (1U << 1U))
        {
            return ADC1->DR & 0x0FFFU;
        }
    }
    return 0U;
}

static uint32_t ToPercentFromAdc(uint32_t adc)
{
    if(adc > 4095U) adc = 4095U;
    return (adc * 100U) / 4095U;
}



// Servo outputs initialization (TIM2_CH1 on PA0 and TIM3_CH3 on PB0)
static void ServoOutputs_Init(void)
{
    RCC->APB2ENR |= (1U << 3U); // GPIOB clock
    RCC->APB1ENR |= (1U << 0U); // TIM2 clock
    RCC->APB1ENR |= (1U << 1U); //` TIM3 clock

    // PA0 = TIM2_CH1
    GPIOA->CRL &= ~(0xFU << 0U); // PA0 = TIM2_CH1
    GPIOA->CRL |=  (0xBU << 0U); // Alternate function push-pull, 50 MHz on PA0

    // TIM2 configuration for Servo 1
    TIM2->CR1 = 0U;
    TIM2->PSC = SERVO_TIMER_PSC;
    TIM2->ARR = SERVO_TIMER_ARR;
    TIM2->CNT = 0U;
    TIM2->CCR1 = SERVO_NEUTRAL_US;
    TIM2->CCMR1 &= ~((0xFFU << 0U));
    TIM2->CCMR1 |=  ((0x6U << 4U) | (1U << 3U));
    TIM2->CCER |= (1U << 0U);
    TIM2->CR1 |= (1U << 7U);
    TIM2->EGR = 1U;
    TIM2->CR1 |= (1U << 0U);

    // PB0 = TIM3_CH3
    GPIOB->CRL &= ~(0xFU << 0U);
    GPIOB->CRL |=  (0xBU << 0U);

    // TIM3 configuration for Servo 2
    TIM3->CR1 = 0U;
    TIM3->PSC = SERVO_TIMER_PSC;
    TIM3->ARR = SERVO_TIMER_ARR;
    TIM3->CNT = 0U;
    TIM3->CCR3 = SERVO2_NEUTRAL_US;
    TIM3->CCMR2 &= ~((0xFFU << 0U));
    TIM3->CCMR2 |=  ((0x6U << 4U) | (1U << 3U));
    TIM3->CCER |= (1U << 8U);
    TIM3->CR1 |= (1U << 7U);
    TIM3->EGR = 1U;
    TIM3->CR1 |= (1U << 0U);
}

static void FillSpaces(char *line, uint32_t length)
{
    for(uint32_t i = 0U; i < length; i++) line[i] = ' ';
}

static void BuildLcdLine1(char *line, uint32_t throttle_pct, uint32_t gear, uint32_t vehicle_speed)
{
    FillSpaces(line, 16U);
    line[16] = '\0';
    line[0] = 'T'; line[1] = 'H'; line[2] = ':';
    line[6] = ' '; line[7] = 'G'; line[8] = ':';
    line[10] = ' '; line[11] = 'V'; line[12] = ':';
    line[3] = (char)('0' + ((throttle_pct / 100U) % 10U));
    line[4] = (char)('0' + ((throttle_pct / 10U) % 10U));
    line[5] = (char)('0' + (throttle_pct % 10U));
    line[9] = (char)('0' + (gear % 10U));
    line[13] = (char)('0' + ((vehicle_speed / 100U) % 10U));
    line[14] = (char)('0' + ((vehicle_speed / 10U) % 10U));
    line[15] = (char)('0' + (vehicle_speed % 10U));
}

static void BuildLcdLine2(char *line, uint32_t engine_rpm)
{
    FillSpaces(line, 16U);
    line[16] = '\0';
    if(engine_rpm > 99999U) engine_rpm = 99999U;
    line[0] = 'E'; line[1] = 'N'; line[2] = 'G'; line[3] = ':';
    line[4] = (char)('0' + ((engine_rpm / 10000U) % 10U));
    line[5] = (char)('0' + ((engine_rpm / 1000U) % 10U));
    line[6] = (char)('0' + ((engine_rpm / 100U) % 10U));
    line[7] = (char)('0' + ((engine_rpm / 10U) % 10U));
    line[8] = (char)('0' + (engine_rpm % 10U));
}

static void ServoSetPulse1(uint32_t pulse_us)
{
    if(pulse_us < SERVO_NEUTRAL_US) pulse_us = SERVO_NEUTRAL_US;
    if(pulse_us > SERVO_MAX_US) pulse_us = SERVO_MAX_US;
    TIM2->CCR1 = pulse_us;
}

static void ServoSetPulse2(uint32_t pulse_us)
{
    if(pulse_us < SERVO2_MIN_US) pulse_us = SERVO2_MIN_US;
    if(pulse_us > SERVO2_MAX_US) pulse_us = SERVO2_MAX_US;
    TIM3->CCR3 = pulse_us;
}

static uint32_t VehicleSpeedToPercent(uint32_t vehicle_speed)
{
    uint32_t speed_pct;
    if(vehicle_speed > VEH_SPEED_MAX) vehicle_speed = VEH_SPEED_MAX;
    speed_pct = (vehicle_speed * 100U) / VEH_SPEED_MAX;
    speed_pct = (speed_pct * VEH_SPEED_GAIN_PCT) / 100U;
    if(speed_pct > 100U) speed_pct = 100U;
    return speed_pct;
}


// Funciones para mapear la velocidad del vehículo a pulsos de servo
static uint32_t Servo1PulseFromVehicleSpeed(uint32_t vehicle_speed)
{
    uint32_t speed_pct = VehicleSpeedToPercent(vehicle_speed);
    return SERVO_NEUTRAL_US + ((speed_pct * (SERVO_MAX_US - SERVO_NEUTRAL_US)) / 100U);
}

static uint32_t Servo2PulseFromVehicleSpeed(uint32_t vehicle_speed)
{
    uint32_t speed_pct = VehicleSpeedToPercent(vehicle_speed);
    return SERVO2_NEUTRAL_US - ((speed_pct * (SERVO2_NEUTRAL_US - SERVO2_MIN_US)) / 100U);
}





/* --- Parseador de comandos enviados por el ESP8266 --- */
static void Parse_ESP_Command(const char *str)
{
    uint32_t val = 0U;
    uint8_t i = 2U;

    while(str[i] >= '0' && str[i] <= '9')
    {
        val = (val * 10U) + (uint32_t)(str[i] - '0');
        i++;
    }

    if(str[0] == 'A' && str[1] == ':')
    {
        g_dashboard_throttle_pct = val;
        /* Tomar "foto" del potenciómetro si venimos de modo manual */
        if(g_control_mode == 0U) {
            g_locked_pot_pct = g_current_physical_pct; 
        }
        g_control_mode = 1U; 
    }
    else if(str[0] == 'B' && str[1] == ':')
    {
        if(val == 1U)
        {
            g_dashboard_brake = 4500U;
        }
        else
        {
            g_dashboard_brake = 0U;
        }

        if(g_control_mode == 0U) {
            g_locked_pot_pct = g_current_physical_pct; 
        }
        g_control_mode = 1U; 
    }
}

/* --- Manejador Automático de Interrupciones del USART1 --- */
void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Si la interrupción fue porque llegó un dato nuevo (RXNE) */
    if(USART1->SR & (1U << 5U))
    {
        char c = (char)(USART1->DR & 0xFFU); // Leer el dato recibido
        xQueueSendFromISR(xQueueRX, &c, &xHigherPriorityTaskWoken); // Enviar el carácter a la cola para que lo procese la tarea de control

       
    }
    /* Si ocurrió un error de Overrun, limpiarlo leyendo el registro de datos */
    if(USART1->SR & (1U << 3U))
    {
        volatile uint32_t dummy = USART1->DR;
        (void)dummy;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



