//
// MAIN.C
// Sets up ADC1 channel 15 input on PC.5
//     and DAC2 output PA.5
//
// Also configures PC.4 as a GPIO and generates a 880Hz square wave
// Connect PC.4 to PC.5 to test system
//
// Written by Ross Wolin
//


#include <stdbool.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

#include "hw.h"
#include "dac.h"
#include "adc.h"
#include "gpio_square_wave.h"
#include "tmr_sample.h"

#define LED_PERIOD_MS      250

// Update this macro NUM_COEFF with the number of coefficients being used.
#define NUM_COEFF 47
float ADC_array[NUM_COEFF];
// Update the coefficients array from MATLAB
// Make sure array length is "NUM_COEFF"
const float filter_coefficients[NUM_COEFF] = {
		  -0.004784039470875,-0.0008445600274623,-0.005995017410417,  0.01532470901036,
		   0.002540625979976,-0.001495247396832, 0.004631244912294,-0.008415522781061,
		   -0.01444585634034,  0.01542483390079,    0.029487031321,-0.008023471201025,
		   -0.02775560048802,-0.0006758846045277, 0.001108071937969,-0.007160256929624,
		    0.04523891525131,  0.05089671015453, -0.08153030852143,  -0.1232645152865,
		    0.07987269111248,   0.1935774570845, -0.03306034804442,   0.7774981428465,
		   -0.03306034804442,   0.1935774570845,  0.07987269111248,  -0.1232645152865,
		   -0.08153030852143,  0.05089671015453,  0.04523891525131,-0.007160256929624,
		   0.001108071937969,-0.0006758846045277, -0.02775560048802,-0.008023471201025,
		      0.029487031321,  0.01542483390079, -0.01444585634034,-0.008415522781061,
		   0.004631244912294,-0.001495247396832, 0.002540625979976,  0.01532470901036,
		  -0.005995017410417,-0.0008445600274623,-0.004784039470875
		};

//Local functions
float filter(register float val);

int main(void)
{
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (true);
	}  // Capture error;

   gsw_init();
   ADC_init();
   DAC2_init();
   tmr_sample_init();

   while (true);
}

//Time to load the DACs!
//This interrupt routine is called from a timer interrupt, at a rate of 44Khz,
//which is a good sampling rate for audio
//(Although the default handler has 'DAC' in the name, we are just using this
// as generic timer interrupt)
void TIM6_DAC_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM6, TIM_IT_Update)) {

      //Process the ADC and DACs ...

      // Generate a square wave of approimately 880hz
      static int ctCycles=0;

      if (++ctCycles >= SAMPLE_FREQ/880/2) {
         ctCycles = 0;
         gsw_toggle();
      }


      int n = ADC_get();
      ADC_start();         //Start a new conversion

      //Write filtered waveform to DAC
      // (Notch filter removes the DC offset in the original waveform,
      //  so we add it back in)
      DAC2_set((uint16_t)(DAC_MID + (int)filter(n)));


      TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
   }
}

float filter(register float val)
{
	val -= DAC_MID;
	float conv_sum = val*filter_coefficients[0], temp2 = 0, temp = ADC_array[0];

	for (int i = 0; i < NUM_COEFF - 1; ++i){
		conv_sum += temp*filter_coefficients[i+1];
		temp2 = temp;
		temp = ADC_array[i+1];
		ADC_array[i+1]= temp2;
	}
	ADC_array[0]= val;

	return conv_sum;
}




/*
 * Callback used by stm32f4_discovery_audio_codec.c.
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
  /* TODO, implement your code here */
  return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
  /* TODO, implement your code here */
  return -1;
}
