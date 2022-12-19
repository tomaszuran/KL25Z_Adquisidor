#include "derivative.h" /* include peripheral declarations */
#include <stdlib.h>

void Init_MCU(void);
void Init_ADC(void);
void Init_Timer(void);
void Adquisidor(uint16_t mod, uint8_t prescaler, uint32_t muestras);

uint8_t * mediciones;
uint32_t indice = 0;
uint32_t max_muestras = 0;

uint8_t muestreo_terminado = 0;

int main(void)
{
	Init_MCU();
	Init_ADC();
	Init_Timer();
	
	Adquisidor(48000, 0, 1001);
	while(muestreo_terminado == 0){}
	Adquisidor(48000, 0, 1002);
	while(muestreo_terminado == 0){}
	Adquisidor(48000, 0, 1003);
	while(muestreo_terminado == 0){}
	Adquisidor(48000, 0, 1004);
	while(muestreo_terminado == 0){}
	
	
	for(;;) {	   
	 
	}
	
	return 0;
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

void Init_ADC (void)
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
			
	ADC0_SC2 = 0; //Funcion de comparación deshabilitada, DMA deshabilitado

	ADC0_SC3 = ADC_SC3_ADCO_MASK; //sin calibrar, conversión continua, promedio de hardware deshabilitado
	
	
	//Habilito NVIC ADC0
	
	//NVIC_ICPR |= 1 << 15;
	//NVIC_ISER |= 1 << 15;
	
	//Tiempo de conversión de aproximadamente 604 nanosegundos (con el clock a 48MHz)
	
	ADC0_SC1A = (0*ADC_SC1_AIEN_MASK) | (0*ADC_SC1_DIFF_MASK) | ADC_SC1_ADCH(4); //Interrupciones deshabilitadas, diferencial desactivado, canal 4 (PTE21)
}

void Init_Timer(void)
{
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;
	SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) | SIM_SOPT2_TPMSRC(1);
	TPM0_SC = TPM_SC_TOIE_MASK | TPM_SC_CMOD(0) | TPM_SC_PS(0);
	TPM0_MOD = 60000;
	
	NVIC_ICPR |= 1 << 17;
	NVIC_ISER |= 1 << 17;
}

void FTM0_IRQHandler()
{
	TPM0_SC |= TPM_SC_TOF_MASK;
	mediciones[indice++] = ADC0_RA;
	if(indice >= max_muestras)
	{
		TPM0_SC &= ~TPM_SC_CMOD_MASK;
		muestreo_terminado = 1;
	}
		
}

void Adquisidor(uint16_t mod, uint8_t prescaler, uint32_t muestras)
{
	if(mod > 65535)
		return;
	if(prescaler > 7)
		return;
	
	if(muestras > 10000)
		return;
	
	if(max_muestras != 0)
	{
		free(mediciones);
	}
	
	max_muestras = muestras;
	indice = 0;
	
	TPM0_MOD = mod;
	TPM0_SC = (TPM0_SC & ~TPM_SC_PS_MASK) | TPM_SC_PS(prescaler);
	muestreo_terminado = 0;
	
	mediciones = (uint8_t *) calloc(max_muestras, sizeof(uint8_t));
	
	if(mediciones != NULL)
	{
		TPM0_SC |= TPM_SC_CMOD(1);
	}
}
