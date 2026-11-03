
#include "main.h"

/* ===============================
   GPIO 腳位設定
   =============================== */

#define LED1_PIN GPIO_PIN_5
#define LED2_PIN GPIO_PIN_8
#define LED3_PIN GPIO_PIN_6

#define LED_PORT GPIOC

#define BTN_PIN GPIO_PIN_9
#define BTN_PORT GPIOC

/* ===============================
   時間設定 (ms)
   =============================== */

#define DEBOUNCE_TIME 50
#define BLINK_TIME    300
#define STAGE_TIME    5000
#define TOTAL_TIME    15000

/* ===============================
   系統狀態
   =============================== */

typedef enum
{
    RUNNING,
    PAUSED,
    BLINKING
} SystemState;

SystemState state = RUNNING;

/* 計時變數 */

uint32_t startTime = 0;
uint32_t elapsedTime = 0;
uint32_t blinkTime = 0;

/* 按鈕變數 */

GPIO_PinState buttonStable = GPIO_PIN_SET;
GPIO_PinState buttonLastRead = GPIO_PIN_SET;

uint32_t debounceTimer = 0;


/* ===============================
   系統時脈設定
   使用 HSI 16MHz
   =============================== */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ===============================
   GPIO 初始化
   =============================== */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* LED 初始關閉 */

    HAL_GPIO_WritePin(LED_PORT,
                      LED1_PIN | LED2_PIN | LED3_PIN,
                      GPIO_PIN_RESET);

    /* LED 設定為輸出 */

    GPIO_InitStruct.Pin =
        LED1_PIN | LED2_PIN | LED3_PIN;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /* 按鈕設定為輸入，上拉 */

    GPIO_InitStruct.Pin = BTN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(BTN_PORT, &GPIO_InitStruct);
}


/* ===============================
   LED 控制
   =============================== */

void LED_Control(uint8_t led1,
                 uint8_t led2,
                 uint8_t led3)
{
    HAL_GPIO_WritePin(LED_PORT, LED1_PIN,
        led1 ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LED_PORT, LED2_PIN,
        led2 ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LED_PORT, LED3_PIN,
        led3 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


/* ===============================
   計時器重置
   =============================== */

void Reset_Timer(void)
{
    state = RUNNING;

    elapsedTime = 0;

    startTime = HAL_GetTick();

    blinkTime = 0;

    LED_Control(1, 0, 0);
}


/* ===============================
   按鈕防彈跳
   =============================== */

uint8_t Button_Detect(void)
{
    GPIO_PinState reading;

    reading = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);

    if (reading != buttonLastRead)
    {
        debounceTimer = HAL_GetTick();
        buttonLastRead = reading;
    }

    if ((HAL_GetTick() - debounceTimer) >= DEBOUNCE_TIME)
    {
        if (reading != buttonStable)
        {
            buttonStable = reading;

            /* 按下時為 LOW */

            if (buttonStable == GPIO_PIN_RESET)
            {
                return 1;
            }
        }
    }

    return 0;
}


/* ===============================
   按鈕狀態處理
   =============================== */

void Button_Process(void)
{
    if (Button_Detect())
    {
        if (state == RUNNING)
        {
            /* 暫停 */

            elapsedTime = HAL_GetTick() - startTime;

            if (elapsedTime >= TOTAL_TIME)
            {
                elapsedTime = TOTAL_TIME;
                state = BLINKING;
                blinkTime = HAL_GetTick();
                LED_Control(0, 0, 0);
            }
            else
            {
                state = PAUSED;
            }
        }

        else if (state == PAUSED)
        {
            /* 繼續 */

            startTime = HAL_GetTick() - elapsedTime;

            state = RUNNING;
        }

        else if (state == BLINKING)
        {
            /* 重置 */

            Reset_Timer();
        }
    }
}


/* ===============================
   計時與 LED 控制
   =============================== */

void Timer_Process(void)
{
    uint32_t now = HAL_GetTick();

    if (state == RUNNING)
    {
        elapsedTime = now - startTime;

        if (elapsedTime < STAGE_TIME)
        {
            /* 0 ~ 5 秒 */

            LED_Control(1, 0, 0);
        }

        else if (elapsedTime < STAGE_TIME * 2)
        {
            /* 5 ~ 10 秒 */

            LED_Control(1, 1, 0);
        }

        else if (elapsedTime < TOTAL_TIME)
        {
            /* 10 ~ 15 秒 */

            LED_Control(1, 1, 1);
        }

        else
        {
            /* 超過 15 秒 */

            state = BLINKING;

            blinkTime = now;

            LED_Control(0, 0, 0);
        }
    }

    else if (state == BLINKING)
    {
        if (now - blinkTime >= BLINK_TIME)
        {
            blinkTime = now;

            HAL_GPIO_TogglePin(LED_PORT, LED1_PIN);
            HAL_GPIO_TogglePin(LED_PORT, LED2_PIN);
            HAL_GPIO_TogglePin(LED_PORT, LED3_PIN);
        }
    }
}


/* ===============================
   錯誤處理
   =============================== */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}


/* ===============================
   主程式
   =============================== */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();

    Reset_Timer();

    while (1)
    {
        Button_Process();

        Timer_Process();
    }
}
