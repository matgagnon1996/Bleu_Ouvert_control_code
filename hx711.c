#include "hx711.h"

#define HX711_SLOPE  0.176136363
#define HX711_OFFSET -47947.00548//-55957.00548

int setupGPIO(HX711 *hx)
{
  setup_io();
  INP_GPIO(hx->data_pin);
  INP_GPIO(hx->clock_pin);
  OUT_GPIO(hx->clock_pin);
  int err = setPinState(hx->clock_pin, false);
  if (err)
  {
    printf("\n!!! Failed to set pin %d\n", hx->clock_pin);
    return 1;
  }
  return 0;
}

void cleanGPIO(HX711 *hx)
{
  // unpull pins
  GPIO_PULL = 0;
  GPIO_PULLCLK0 = 1 << hx->data_pin;
  GPIO_PULL = 0;
  GPIO_PULLCLK0 = 0;
  restore_io();
}

// set higher priority for the process
int setPriority(int priority)
{
  struct sched_param sched;

  memset(&sched, 0, sizeof(sched));

  sched.sched_priority = priority;
  if (sched_setscheduler(0, SCHED_FIFO, &sched))
  {
    return 1;
  }
  return 0;
}

void reset(HX711 *hx)
{
  setPinState(hx->clock_pin, true);
  usleep(60);
  setPinState(hx->clock_pin, false);
  usleep(60);
}

void setGain(HX711 *hx)
{
  // 1 is 128 gain channel a
  // 2 is 32  gain channel b
  // 3 is 63  gain channel a
  int x = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  if (hx->wanted_channel == 'A' && hx->gain_channel_A == 128)
  {
    x = 1;
  }
  else if (hx->wanted_channel == 'B')
  {
    x = 2;
  }
  else
  {
    x = 3;
  }

  for (i = 0; i < x; ++i)
  {
    setPinState(hx->clock_pin, true);
    for (j = 0; j < 4; ++j)
    {
      continue;
    }
    setPinState(hx->clock_pin, false);
    for (k = 0; k < 4; ++k)
    {
      continue;
    }
  }
}

int getRawData(HX711 *hx)
{
	// MODIFT BY MGA
	//unsigned int bits = 0;
	  unsigned long count = 0;
	  int i = 0;

	  // wait for high pin
	  while (getPinState(hx->data_pin))
	  {
	    usleep(10000);
	  };


	  for (i = 0; i < 24; i++)
	  {
	    setPinState(hx->clock_pin, true);

	    count = count << 1;

	    setPinState(hx->clock_pin, false);

	    if(getPinState(hx->data_pin) == 1)
	    {
	    	//printf("data is high \n");
	    	count++;
	    }else
	    {
	    	//printf("data is low \n");
	    }
	  }

	  setPinState(hx->clock_pin, true);

	  count = ~count;
	  count = count & 0x00FFFFFF;
	  //count = count ^ 0x800000;

	  setGain(hx);

	  setPinState(hx->clock_pin, false);

	  return count;
}

bool getPinState(unsigned pin_number)
{
  if (pin_number > 31)
  {
    printf("getPinState - wrong pin number: %d", pin_number);
    return false;
  }
  return (*(gpio + 13) & (1 << pin_number));
}

int setPinState(unsigned pin_number, bool state)
{
  if (pin_number > 31)
  {
    printf("setPinState - wrong pin number: %d", pin_number);
    return 1;
  }
  if (state)
  {
    *(gpio + 7) = (1 << pin_number);
  }
  else
  {
    *(gpio + 10) = (1 << pin_number);
  }
  return 0;
}

int initHX711(HX711 *hx, unsigned char clock_pin, unsigned char data_pin)
{
  if (clock_pin > 31)
  {
    printf("\nWrong clock_pin number: %d", clock_pin);
    return 1;
  }
  else if (data_pin > 31)
  {
    printf("\nWrong data_pin number: %d", data_pin);
    return 1;
  }
  hx->clock_pin = clock_pin;
  hx->data_pin = data_pin;
  hx->gain_channel_A = 128;
  hx->current_channel = 'A';
  hx->wanted_channel = 'A';
  hx->offset_A_128 = 0;
  hx->offset_A_64 = 0;
  hx->offset_B = 0;
  hx->scale_ratio_A_128 = 0.0;
  hx->scale_ratio_A_64 = 0.0;
  hx->scale_ratio_B = 0.0;
  hx->filterPtr = NULL;
  return 0;
}

int zeroScale(HX711 *hx)
{
  double result = getRawDataMean(hx, 160);
  if (hx->current_channel == 'A' && hx->gain_channel_A == 128)
  {
    hx->offset_A_128 = result;
    return 0;
  }
  else if (hx->current_channel == 'A' && hx->gain_channel_A == 64)
  {
    hx->offset_A_64 = result;
    return 0;
  }
  else if (hx->current_channel == 'B')
  {
    hx->offset_B = result;
    return 0;
  }
  printf("\n\n!!!zeroScale - current channel is wrong: %c\n", hx->current_channel);
  return 1;
}

int getRawDataMean(HX711 *hx, int samples)
{
	int *data;
	data = malloc(samples * sizeof(int));

	int i =0;
	for (i = 0; i < samples; ++i)
	{
		data[i] = getRawData(hx);
		usleep(10000);
	}

	long sum = 0;
	for (i = 0; i < samples; ++i)
	{
		sum += data[i];
	}
	free(data);
	return (int)(sum / samples);
}

int getDataMean(HX711 *hx, int samples)
{
  int result = getRawDataMean(hx, samples);
  if (hx->current_channel == 'A' && hx->gain_channel_A == 128)
  {
    return result - hx->offset_A_128;
  }
  else if (hx->current_channel == 'A' && hx->gain_channel_A == 64)
  {
    return result - hx->offset_A_64;
  }
  else if (hx->current_channel == 'B')
  {
    return result - hx->offset_B;
  }
  printf("\n\n!!!getDataMean - current channel is wrong: %c\n", hx->current_channel);
  return 1;
}

int getWeightMean(HX711 *hx, int samples)
{
  double result = getRawDataMean(hx, samples);

  printf("Raw data mean : %f \n", result);
  if (hx->current_channel == 'A' && hx->gain_channel_A == 128)
  {
    return (int)((result - hx->offset_A_128) / hx->scale_ratio_A_128);
  }
  else if (hx->current_channel == 'A' && hx->gain_channel_A == 64)
  {
    return (int)((result - hx->offset_A_64) / hx->scale_ratio_A_64);
  }
  else if (hx->current_channel == 'B')
  {
    return (int)((result - hx->offset_B) / hx->scale_ratio_B);
  }
  printf("\n\n!!!getWeightMean - current channel is wrong: %c\n", hx->current_channel);
  return 1;
}

double voltageToWeight(double weight)
{
	return ((double)(weight))*HX711_SLOPE + HX711_OFFSET;
}
