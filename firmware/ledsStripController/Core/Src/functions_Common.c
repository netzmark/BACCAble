/*
 * functions_Common.c
 *
 *  Created on: May 2, 2025
 *      Author: GauchoHP
 */

#include "functions_Common.h"
#include "debug.h"
#include <string.h>	// @netzmark: theoretically required for strlen
#include <math.h> 	// @netzmark: we can remove math i resign using isnan() and isinf()
					//@netzmark for "appendFloat" related to printf improvement
					//it is in globalVariables.h but better have it here independently

/*
 * @netzmark custom-printf improvement
 * floatToStr() + appendFloatRightAligned()
*/

/*
 * floatToStr()
 * Converts a floating-point value to a null-terminated ASCII string.
 *
 * - Performs manual decimal rounding according to the requested precision
 * - Suppresses negative zero and very small values (e.g. "-0.00")
 * - Outputs a fixed number of fractional digits
 * - Handles sign explicitly and writes into a bounded buffer
 *
 * Unlike printf():
 * - Does not perform automatic field width adjustment
 * - Does not apply padding or alignment rules internally
 * - Does not support format flags or scientific notation
 *
 * Note:
 * Field width, padding, and alignment are handled externally
 * (by appendFloatRightAligned()).
 */

void floatToStr(char* str, float num, uint8_t precision, uint8_t maxLen)
{
    if (maxLen == 0) return;

    // Fast lookup for precision 0-3 (max usable for standard float)
    static const float lsbTable[] = { 1.0f, 0.1f, 0.01f, 0.001f };
    if (precision > 3) precision = 3; //I assume that never need higher precision

    char* ptr = str;
    char* end = str + maxLen - 1; // Reserve space for '\0'

    float lsb = lsbTable[precision];
    float rounding = lsb / 2.0f;

    // --- Sign handling & Zero-clamping ---
    // Eliminates "-0.0" by checking if value is below displayable threshold
    if (num < -rounding) {
        if (ptr < end) *ptr++ = '-';
        num = -num + rounding;
    } else if (num < rounding) {
        num = 0.0f;
    } else {
        num += rounding;
    }

    uint32_t intPart = (uint32_t)num;
    float remainder = num - (float)intPart;

    // --- Integer part ---
    char temp[11]; //***instead of "char temp[12]" to save the stack RAM memory for about 1 byte, 11 is enough
    int intLen = 0;

    do {
        temp[intLen++] = (intPart % 10) + '0';
        intPart /= 10;
    } while (intPart && intLen < (int)sizeof(temp));

    while (intLen > 0 && ptr < end) {
        *ptr++ = temp[--intLen];
    }

    // --- Fractional part ---
    if (precision > 0 && ptr < end) {
        *ptr++ = '.';
        while (precision-- && ptr < end) {
            remainder *= 10.0f;
            int digit = (int)remainder;
            *ptr++ = (char)(digit + '0');
            remainder -= (float)digit;
        }
    }

    *ptr = '\0';
}


/*
 * appendFloatRightAligned()
 * Appends a float value into a transport buffer as a fixed-length field.
 * Total field length is predictable, matching the original Gaucho's intent (I guess).
 * Handles field width, left padding, right alignment, and visual layout.
 */

void appendFloatRightAligned(char* result,
                             uint8_t* index,
                             uint8_t resultMaxLen,
                             float value,
                             uint8_t precision,
                             uint8_t fieldWidth)
{
    /* Must always write exactly `fieldWidth` bytes
       to preserve fixed-length transport frames. */

    if (*index >= resultMaxLen)
        return;

    /* Invalid float → fill entire field */
    if (isnan(value) || isinf(value))
    {
        for (uint8_t i = 0; i < fieldWidth && *index < resultMaxLen; i++)
            result[(*index)++] = ' ';   // or '-' if visually preferred
        return;
    }

    char buf[32]; // ***instead of "char buf[32]" to save the stack RAM memory for about 16 bytes
    floatToStr(buf, value, precision, sizeof(buf));

    uint8_t len = strlen(buf);

    /* Overflow → mark entire field */
    if (len > fieldWidth)
    {
        for (uint8_t i = 0; i < fieldWidth && *index < resultMaxLen; i++)
            result[(*index)++] = '#';
        return;
    }

    uint8_t pad = fieldWidth - len;

    /* Left padding (right alignment) */
    for (uint8_t i = 0; i < pad && *index < resultMaxLen; i++)
        result[(*index)++] = ' ';

    /* Write value */
    for (uint8_t i = 0; i < len && *index < resultMaxLen; i++)
        result[(*index)++] = buf[i];

    /* Safety guard: ensure exact fieldWidth bytes written */
    for (uint8_t written = pad + len;
         written < fieldWidth && *index < resultMaxLen;
         written++)
    {
        result[(*index)++] = ' ';
    }
}


uint8_t calculateCRC(uint8_t* data, uint8_t arraySize) {
	uint8_t crc = 0xFF;
	if(arraySize>1){
		//calculate sae_j1850 CRC-8 of the array (excluded last element, that will be used to store the final CRC)
		for (uint8_t i=0;i<arraySize-1;i++){
			crc ^= data[i];
			for (int i = 0; i < 8; ++i){
				crc = (crc & 0x80) ? (crc << 1) ^ 0x1D : crc << 1;
			}
		}
		return (crc ^ 0xFF); //return calculated checksum
	}
	return 0; //nothing to calculate
}


//System Clock Configuration

/* HSI/HSE configuration added by @netzmark - BEGINING */

#if !defined(HSE_ENABLED_FOR_UCAN)
// System Clock Configuration for HSI (Gaucho's ori code)
// It means this is Baccable/Canable standard clock configuration

void SystemClock_Config(void){
  HAL_Init();
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    Error_Handler(2000);
  }

  // configure CRS to stabilize HSI48
  __HAL_RCC_CRS_CLK_ENABLE();

  RCC_CRSInitTypeDef crs = {0};
  crs.Prescaler = RCC_CRS_SYNC_DIV1;
  crs.Source = RCC_CRS_SYNC_SOURCE_USB;
  crs.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  crs.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000, 1000);
  crs.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  crs.HSI48CalibrationValue = 0x20;
  HAL_RCCEx_CRSConfig(&crs);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK){
    Error_Handler(1500);
  }

  //the following part is used by usb, used by canable

  // Set USB clock source to HSI48 (48 MHz)
  RCC_PeriphCLKInitTypeDef PeriphClkInit= {0};
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
	  Error_Handler(1000);
  }
  __HAL_RCC_GPIOA_CLK_ENABLE();

}
#endif

#if defined(HSE_ENABLED_FOR_UCAN)
// Configuration the boards for HSE (external oscillator) using - BEGIN ; by @netzmark

void SystemClock_Config(void) {
  HAL_Init();
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** 1. HSE oscillator and PLL loop configuration
  * HSI48 disabled, HSE (8MHz) enabled
  * PLL: 8MHz * 6 = 48MHz
  */

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;             // External oscillator ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE; // external oscillator is set as the source for PLL
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;         // Multiplication x6 for dla 8MHz external oscillator (8*6=48)

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler(2000);
  }

  /** 2. Bus clock initialisation (SYSCLK = 48MHz z PLL)
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // System starts with PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler(1500);
  }

  /** 3. Setting clock source for USB device
  * Changed from HSI48 onto signal from PLL
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLCLK; // USB gets stable clock from PLL(HSE)

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler(1000);
  }
   //__HAL_FLASH_PREFETCH_BUFFER_ENABLE(); // it seems to be not necessary as it is applied by HAL_init().
   __HAL_RCC_GPIOA_CLK_ENABLE();
}
#endif
//System Clock Configuration for HSE/HSI - END ; by @netzmark

void Error_Handler(uint16_t halfPeriod){
	//onboardLed_red_on();
	//LOGS("System error\r\n");
	//__disable_irq();
	//NVIC_SystemReset();



	RCC->AHBENR |=RCC_AHBENR_GPIOAEN; //ensure clock is enabled on port gpioA


	uint8_t tmpBool01=0;
	while (1){

		//now toggle leds without using HAL, to be more resilient
		if(tmpBool01){
			GPIOA->BSRR= GPIO_PIN_0 <<16; //set PA0 low (red led)

		}else{
			GPIOA->BSRR= GPIO_PIN_0 ; //set PA0 high (red led)
		}

		tmpBool01=!tmpBool01;

		for (volatile uint32_t i = 0; i < (12500*halfPeriod) ; i++){ //12500cycles=1msec
			__asm("nop");
		}
	}
}

// Disable all interrupts
void system_irq_disable(void){
	__disable_irq();
	__DSB();
	__ISB();
}


// Enable all interrupts
void system_irq_enable(void){
        __enable_irq();
}

void system_hex32(char *out, uint32_t val){
	char *p = out + 8;
	*p-- = 0;
	while (p >= out) {
		uint8_t nybble = val & 0x0F;
		if (nybble < 10)
			*p = '0' + nybble;
		else
			*p = 'A' + nybble - 10;
		val >>= 4;
		p--;
	}
}

void saveToFilesystem(void){
	#ifdef ENABLE_USB_MASS_STORAGE

		FIL fil;
		UINT bw;
		FRESULT res;

		res = f_mount(&fs, "", 1);
		if (res == FR_OK){
			res = f_open(&fil, "hello.txt", FA_WRITE|FA_OPEN_ALWAYS);
			if (res == FR_OK) {
				f_write(&fil, "Hello, World!\r\n", 15, &bw);
				f_sync(&fil);
				f_close(&fil);
				onboardLed_blue_on();
			}


		}
		f_unmount("");

	#endif
}


void storage_init(void){

	#ifdef ENABLE_USB_MASS_STORAGE
		//FATFS fs;
		FIL fil;
		UINT bw;
		FRESULT res;
		BYTE work[FF_MIN_SS];

		res = f_mount(&fs, "", 1);
		if (res != FR_OK){
			//onboardLed_blue_on();
			MKFS_PARM opt = {.fmt = FM_FAT | FM_SFD, .n_fat = 1, .align = 0, .n_root = 32, .au_size = FF_MIN_SS};
			res = f_mkfs("", &opt, work, FF_MIN_SS);
			if (res == FR_OK){
				res = f_setlabel("BACCABLE "
					#ifdef ACT_AS_CANABLE
						"Sniffer"
					#elif defined(C1baccable)
						"C1"
					#elif defined(C2baccable)
						"C2"
					#elif defined(BHbaccable)
						"BH"
					#endif
				);
				res = f_open(&fil, "Version.txt", FA_CREATE_ALWAYS | FA_WRITE);

				if (res == FR_OK){
					onboardLed_blue_on();

					f_write(&fil, _FW_VERSION, strlen(_FW_VERSION), &bw);
					f_close(&fil);
				}else{
					onboardLed_red_on();
				}
			}

		}

		res = f_unmount("");





	#endif
}
