#include "derivative.h" /* include peripheral declarations */


void Init_MCU(void);

uint8_t mediciones[100];
uint16_t indice = 0;

int main(void)
{
	
	
	
	ADC0_SC1A = (1*ADC_SC1_AIEN_MASK) | (0*ADC_SC1_DIFF_MASK) | ADC_SC1_ADCH(4); //Interrupciones habilitadas, diferencial desactivado, canal 4 (PTE21)
	
	for(;;) {	   
	 
	}
	
	return 0;
}

void ADC0_IRQHandler()
{
	mediciones[indice++] = ADC0_RA;
}

void Init_MCU(void) {
	SIM_COPC=0;
	//PLL
	SIM_CLKDIV1 = (SIM_CLKDIV1_OUTDIV1(0x01) | SIM_CLKDIV1_OUTDIV4(0x01)); //Actualizo los prescalers del core
	SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK; //Selecciona al PLL como fuente de reloj para varios periféricos
	SIM_SOPT1 &= ~SIM_SOPT1_OSC32KSEL(0x03); //System oscillator drives 32 kHz clock for various peripherals
	//Cambio a modo FBE
	MCG_C2 = (MCG_C2_RANGE0(2) | MCG_C2_EREFS0_MASK);
	OSC0_CR = OSC_CR_ERCLKEN_MASK;
	MCG_C1 = (MCG_C1_CLKS(2) | MCG_C1_FRDIV(3) | MCG_C1_IRCLKEN_MASK);
	MCG_C4 &= (uint8_t)~(uint8_t)((MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(3)));
	MCG_C5 = MCG_C5_PRDIV0(1);
	MCG_C6 = MCG_C6_VDIV0(0);
	while ((MCG_S & MCG_S_IREFST_MASK) != 0) { //Espero hasta que el clock de referencia del FLL sea el clock externo
	}
	while ((MCG_S & 0x0CU) != 0x08) { //Espero hasta que el clock externo sea la referencia del módulo MCG
	}
	//Cambio a modo PBE
	MCG_C6 = (MCG_C6_PLLS_MASK | MCG_C6_VDIV0(0));
	while ((MCG_S & 0x0C) != 0x08) { //Espero hasta que el clock externo sea la referencia del módulo MCG
	}
	while ((MCG_S & MCG_S_LOCK0_MASK) == 0x00) { //Espero hasta que se fije la frecuencia
	}
	//Cambio a modo PEE
	MCG_C1 = (MCG_C1_CLKS(0) | MCG_C1_FRDIV(3) | MCG_C1_IRCLKEN_MASK);
	while ((MCG_S & 0x0C) != 0x0C) { //Espero hasta que se seleccione la salida del PLL
	}
}

void InitADC (void)
{
	//Activo el modulo del ADC
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;
	//Registros del ADC
	ADC0_CFG1 = 0;
	
	//7 - Normal power configuration (0)
	//6-5 Clock Divide Select (00) - Divide por 1
	//4 Sample time (0) - Tiempo corto de muestra, para entradas de alta impedancia hay que ponerlo en 1
	//3-2 Mode (00) - Conversion de 8 bits para modo NO diferencial
	//0-1 Input Clock Select (00) - Selecciono clock de bus
	
	ADC0_CFG2 = 0b100;
	
	//4 - ADC Mux Sel - Selecciono canal A
	//3 - Async output disabled
	//2 - High speed configuration enabled
	//1-0 - Long time sampling select (no importa)
			
	ADC0_SC2 = 0; //Compare function disabled,DMA is disabled

	ADC0_SC3 = 0; //uncalibrated, only one conversion, hardware average disabled.
	
	
	//Habilito NVIC ADC0
	
	NVIC_ICPR |= 1 << 31;
	NVIC_ISER |= 1 << 31;
	
	//Tiempo de conversión de aproximadamente 604 nanosegundos (con el clock a 48MHz)
}
