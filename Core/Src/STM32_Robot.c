/*
 * STM32_Robot.c
 *
 *  Created on: Dec 23, 2020
 *      Author: pawel
 */
#include "STM32_Robot.h"

void robotDefaultValuesInit(Robot* rob, TIM_HandleTypeDef* timEngLeft, TIM_HandleTypeDef* timEngRight, UART_HandleTypeDef* uartUSB, UART_HandleTypeDef* uartBlth, UART_HandleTypeDef* uartSelected, ADC_HandleTypeDef* adc)
{
	rob->engLeft.tim = timEngLeft;
	rob->engLeft.timChl = TIM_CHANNEL_1;//3
	rob->engLeft.dirPinPort = GPIOB;
	rob->engLeft.dirPin = GPIO_PIN_5;
	rob->engLeft.pwmVal = 0;

	rob->engRight.tim = timEngRight;
	rob->engRight.timChl = TIM_CHANNEL_3;//1
	rob->engRight.dirPinPort = GPIOC;
	rob->engRight.dirPin = GPIO_PIN_7;
	rob->engRight.pwmVal = 0;

	rob->uartUSB = uartUSB;
	rob->uartBlth = uartBlth;
	rob->blthPinPort = GPIOA;		//to trzeba przerzucic na i2c
	rob->blthPin = GPIO_PIN_15;		//to trzeba przerzucic na i2c
	rob->uartSelected = uartSelected;

	strcpy(rob->snr[0].snrName, "RgtCnct");
	rob->snr[0].snrPinPort = DIGIAL_SNR;
	rob->snr[0].snrPinPort = GPIOA;
	rob->snr[0].snrPin = GPIO_PIN_0;
	rob->snr[0].snrVal = 0;

	strcpy(rob->snr[1].snrName, "LftCnct");
	rob->snr[1].snrType = DIGIAL_SNR;
	rob->snr[1].snrPinPort = GPIOA;
	rob->snr[1].snrPin = GPIO_PIN_1;
	rob->snr[1].snrVal = 0;

	strcpy(rob->snr[2].snrName, "RgtLight");
	rob->snr[2].snrType = ANALOG_SNR;
	rob->snr[2].snrPinPort = GPIOC;
	rob->snr[2].snrPin = GPIO_PIN_5;
	rob->snr[2].snrVal = 0;
	rob->snr[2].snrAdc = adc;
	rob->snr[2].snrAdcChannel.Channel = ADC_CHANNEL_15;
	rob->snr[2].snrAdcChannel.Rank = ADC_REGULAR_RANK_1;
	rob->snr[2].snrAdcChannel.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

	strcpy(rob->snr[3].snrName, "LftLight");
	rob->snr[3].snrType = ANALOG_SNR;
	rob->snr[3].snrPinPort = GPIOC;
	rob->snr[3].snrPin = GPIO_PIN_0;
	rob->snr[3].snrVal = 0;
	rob->snr[3].snrAdc = adc;
	rob->snr[3].snrAdcChannel.Channel = ADC_CHANNEL_10;
	rob->snr[3].snrAdcChannel.Rank = ADC_REGULAR_RANK_1;
	rob->snr[3].snrAdcChannel.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);


}

void robotSetEnginePwm(Robot* rob, int8_t pwmValLeft, int8_t pwmValRight)
{
	if(pwmValLeft >= -100 && pwmValLeft <= 100 && pwmValRight >= -100 && pwmValRight <= 100)
	{
		rob->engLeft.pwmVal = pwmValLeft;
		rob->engRight.pwmVal = pwmValRight;

		if(rob->engLeft.pwmVal >= 0)
			HAL_GPIO_WritePin(rob->engLeft.dirPinPort, rob->engLeft.dirPin, FOWARD);
		else
			HAL_GPIO_WritePin(rob->engLeft.dirPinPort, rob->engLeft.dirPin, BACKWARD);

		if(rob->engRight.pwmVal >= 0)
			HAL_GPIO_WritePin(rob->engRight.dirPinPort, rob->engRight.dirPin, FOWARD);
		else
			HAL_GPIO_WritePin(rob->engRight.dirPinPort, rob->engRight.dirPin, BACKWARD);

		uint32_t pwmVal = abs((rob->engLeft.pwmVal * PWM_MAX) / 100);
		__HAL_TIM_SET_COMPARE(rob->engLeft.tim, rob->engLeft.timChl, pwmVal);

		pwmVal = abs((rob->engRight.pwmVal * PWM_MAX) / 100)
				;
		__HAL_TIM_SET_COMPARE(rob->engRight.tim, rob->engRight.timChl, pwmVal);
		HAL_TIM_PWM_Start(rob->engRight.tim, rob->engRight.timChl);
		HAL_TIM_PWM_Start(rob->engLeft.tim, rob->engLeft.timChl);
	}
}

void robotTelemetry(Robot *rob)
{
	// array with data to be send
	char data[400];

	// JSON serialisation structure for data from robot
	char json[] = "{\"blthState\":\"%i\",\"StartProcedureFinished\":\"%i\",\"StartProcedureCountNum\":\"%i\",\""
			"engLeft\":{\"dirPin\":\"%i\",\"pwmVal\":\"%i\"},\""
			"engRight\":{\"dirPin\":\"%i\",\"pwmVal\":\"%i\"},\""
			"snr[0]\":{\"snrName\":\"%s\",\"snrType\":\"%i\",\"snrVal\":\"%i\"},\""
			"snr[1]\":{\"snrName\":\"%s\",\"snrType\":\"%i\",\"snrVal\":\"%i\"},\""
			"snr[2]\":{\"snrName\":\"%s\",\"snrType\":\"%i\",\"snrVal\":\"%i\"},\""
			"snr[3]\":{\"snrName\":\"%s\",\"snrType\":\"%i\",\"snrVal\":\"%i\"}}";

	// JSON structure connection with robot variable into data array
	sprintf(data, json, rob->blthState, rob->StartProcedureFinished, rob->StartProcedureCountNum,
			rob->engLeft.dirPin, rob->engLeft.pwmVal,
			rob->engRight.dirPin, rob->engRight.pwmVal,
			rob->snr[0].snrName, rob->snr[0].snrType, rob->snr[0].snrVal,
			rob->snr[1].snrName, rob->snr[1].snrType, rob->snr[1].snrVal,
			rob->snr[2].snrName, rob->snr[2].snrType, rob->snr[2].snrVal,
			rob->snr[3].snrName, rob->snr[3].snrType, rob->snr[3].snrVal);


	// Check if UART through bluetooth is selected, if no then switch off bluetooth module
	if(rob->uartSelected == rob->uartUSB)
		HAL_GPIO_WritePin(rob->blthPinPort, rob->blthPin, GPIO_PIN_RESET);

	else if(rob->uartSelected == rob->uartBlth)
		HAL_GPIO_WritePin(rob->blthPinPort, rob->blthPin, GPIO_PIN_SET);

	// Transmit the JSON data via UART
	HAL_UART_Transmit(rob->uartSelected, (uint8_t*)data, strlen(data), 1000);


}

void robotManualSteering(Robot* rob)
{
	while(__HAL_UART_GET_FLAG(rob->uartSelected, UART_FLAG_RXNE) == RESET) {}
	uint8_t key;
	HAL_UART_Receive(rob->uartSelected, &key, 1, 100);
	switch(key){
	case 'w':
		robotSetEnginePwm(rob, 100, 100);
		break;
	case 's':
		robotSetEnginePwm(rob, -100, -100);
		break;
	case 'a':
		robotSetEnginePwm(rob, -100, 100);
		break;
	case 'd':
		robotSetEnginePwm(rob, 100, -100);
		break;
	case 'e':
		robotSetEnginePwm(rob, 0, 0);
	}
}

void robotReadSensors(Robot* rob)
{
	for(uint8_t i = 0; i < NUMBER_SNR; i++){
		if(rob->snr[i].snrType == DIGIAL_SNR)
			rob->snr[i].snrVal = HAL_GPIO_ReadPin(rob->snr[i].snrPinPort, rob->snr[i].snrPin);

		else if(rob->snr[i].snrType == ANALOG_SNR){
			HAL_ADC_ConfigChannel(rob->snr[i].snrAdc, &rob->snr[i].snrAdcChannel);
			HAL_ADC_Start(rob->snr[i].snrAdc);
			HAL_ADC_PollForConversion(rob->snr[i].snrAdc, 1000);
			rob->snr[i].snrVal = HAL_ADC_GetValue(rob->snr[i].snrAdc);
		}
	}
}

void robotMove(Robot *rob)
{

	robotSetEnginePwm(rob, -40, -40);
}

void robotStartProcedure(Robot* rob)
{
	// loop until whole procedure is finished
	while(rob->StartProcedureFinished == 0){

		// read data from sensors
		robotReadSensors(rob);

		// loop until both sensors are switched on
		do{
			robotReadSensors(rob);
		}
		while(rob->snr[0].snrVal == 1 || rob->snr[1].snrVal == 1);


		// switch on LED to inform that start procedure is on
		robotSwitchLED(rob, 1);

		// counter initialization
		rob->StartProcedureCountNum = 0;

		// counter is set for around 4 seconds
		while(rob->StartProcedureCountNum < START_CNT_NUM){
			robotReadSensors(rob);

			// if one of sensor is not switched on, stop the counting
			if(rob->snr[0].snrVal == 1 || rob->snr[1].snrVal == 1){
				break;
			}
			rob->StartProcedureCountNum++;
		}

		// check if counter finished properly
		if(rob->StartProcedureCountNum == START_CNT_NUM){

			// mark that start procedure finished
			rob->StartProcedureFinished = 1;

			// switch on buzzer for 0.5 second to announce that start procedure finished
			robotSwitchBuzzer(rob, 1);
			HAL_Delay(1000);
			robotSwitchLED(rob, 0);

			// switch off LED
			robotSwitchBuzzer(rob, 0);
		}

		//if counter does not finished properly, just switch off LED
		else{
			rob->StartProcedureCountNum = 0;
			robotSwitchLED(rob,0);
		}
	}
}

void robotWallBouncer(Robot* rob)
{
	// create seed
	srand(time(0));
	while(1){
		// read data from sensors
		robotReadSensors(rob);

		// go straight if nothing is in front
		robotSetEnginePwm(rob, 40, 40);

		// check if robot touch the wall from left side
		if(rob->snr[0].snrVal == 0){
			HAL_Delay(100);

			// go backward
			robotSetEnginePwm(rob, -40,-40);
			HAL_Delay(500);

			// turn right for random period of time
			robotSetEnginePwm(rob, 40, -40);
			HAL_Delay(rand()%1000 + 200);

			// quick stop for better robot stability
			robotSetEnginePwm(rob, 0, 0);
			HAL_Delay(200);
		}

		// check if robot touch the wall from right side
		else if(rob->snr[1].snrVal == 0){
			HAL_Delay(100);

			// go backward
			robotSetEnginePwm(rob, -40,-40);
			HAL_Delay(500);

			// turn left for random period of time
			robotSetEnginePwm(rob, -40, 40);
			HAL_Delay(rand()%1000 + 200);

			// quick stop for better robot stability
			robotSetEnginePwm(rob, 0, 0);
			HAL_Delay(200);
		}
	}
}

void robotLightFollower(Robot* rob)
{
	  robotReadSensors(rob);
	  robotTelemetry(rob);
	  if(rob->StartProcedureFinished == 0){
		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		  HAL_Delay(1000);
		  if(rob->snr[2].snrVal < 700 || rob->snr[3].snrVal < 700){
			  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
			  HAL_Delay(1000);
			  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
			  rob->StartProcedureFinished = 1;
		  }
	  }
	  else{
		  int32_t diff = rob->snr[2].snrVal - rob->snr[3].snrVal;
		  if(diff > 300) diff = 300;
		  else if(diff < -300) diff = -300;
		  int8_t pwmSpd = (diff *20)/300;
		  if(pwmSpd > 20) pwmSpd = 20;
		  else if(pwmSpd < -20) pwmSpd = -20;

		  robotSetEnginePwm(rob, 50+pwmSpd, 50-pwmSpd);
	  }
}

void robotLineFollowerV1(Robot* rob)
{
	robotReadSensors(rob);
	robotTelemetry(rob);
	int32_t diff = rob->snr[2].snrVal - rob->snr[3].snrVal;
	if(diff > 100) diff = 100;
	else if(diff < -100) diff = -100;
	diff = (diff * 30)/100;
	if(diff > 0) {
	  diff = abs(diff);
	  robotSetEnginePwm(rob, 40, 40-diff);
	}
	else if(diff < 0) {
	  diff = abs(diff);
	  robotSetEnginePwm(rob, 40-diff, 40);
	}
}

void robotSwitchBuzzer(Robot* rob, uint8_t state)
{
	if(state == 0 || state == 1)
		HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, state);
}

void robotSwitchLED(Robot* rob, uint8_t state)
{
	if(state == 0 || state == 1)
		HAL_GPIO_WritePin(LED_PORT, LED_PIN, state);
}
