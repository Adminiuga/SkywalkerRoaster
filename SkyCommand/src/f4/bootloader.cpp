#include <Arduino.h>
#include <usbd_if.h>
#include <f4/bootloader.h>


typedef struct {
    uint32_t Initial_SP;
    void (*ResetHandler)(void);
} t_bootVecTable;


/**
 * Function to perform jump to system memory boot from user application
 *
 * Call function when you want to jump to system memory
 */
void JumpToBootloader(void) {
    t_bootVecTable *_bootVec = (t_bootVecTable*) BOOTLOADER_ADDR;

	/**
	 * Step: Disable RCC, set it to default (after reset) settings
	 *       Internal clock, no PLL, etc.
	 */
#ifdef USBCON
    USBD_reenumerate();
#endif

	HAL_RCC_DeInit();
#if defined(USE_STDPERIPH_DRIVER)
	RCC_DeInit();
#endif /* defined(USE_STDPERIPH_DRIVER) */
	
	/**
	 * Step: Disable systick timer and reset it to default values
	 */
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
 
	/**
	 * Step: Disable all interrupts
	 */
	__disable_irq();
	
	/**
	 * Step: Remap system memory to address 0x0000 0000 in address space
	 *       For each family registers may be different. 
	 *       Check reference manual for each family.
	 *
	 *       For STM32F4xx, MEMRMP register in SYSCFG is used (bits[1:0])
	 *       For STM32F0xx, CFGR1 register in SYSCFG is used (bits[1:0])
	 *       For others, check family reference manual
	 */
	//Remap by hand... {
#if defined(STM32F4)
	SYSCFG->MEMRMP = 0x01;
#endif
	//} ...or if you use HAL drivers
	//__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();	//Call HAL macro to do this for you
	/* Clear Interrupt Enable Register & Interrupt Pending Register */
	for (uint16_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
      NVIC->ICER[i] = 0xFFFFFFFF;
      NVIC->ICPR[i] = 0xFFFFFFFF;
    }

	/* Re-enable all interrupts */
	__enable_irq();

/**
	 * Step: Set main stack pointer.
	 *       This step must be done last otherwise local variables in this function
	 *       don't have proper value since stack pointer is located on different position
	 *
	 *       Set direct address location which specifies stack pointer in SRAM location
	 */
	__set_MSP(_bootVec->Initial_SP);
	/**
	 * Step: Actually call our function to jump to set location
	 *       This will start system memory execution
	 */

	_bootVec->ResetHandler();
	
	/**
	 * Step: Connect USB<->UART converter to dedicated USART pins and test
	 *       and test with bootloader works with STM32 Flash Loader Demonstrator software
	 */
}