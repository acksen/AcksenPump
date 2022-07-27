/***********************************************************
This source file is licenced using the 3-Clause BSD License.

Copyright (c) 2022 Acksen Ltd, All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************/

/*
Example: 		basic_pump_control.ino
Library:		AcksenPump
Author: 		Richard Phillips for Acksen Ltd

Created:		27 Jul 2022
Last Modified:		27 Jul 2022

Description:
Demonstrate basic pump control using the AcksenPump library.

*/

#include <AcksenPump.h>

// ***********************************
// Serial Debug
// ***********************************
#define DEBUG_BAUD_RATE			115200


// ***********************************
// I/O  
// ***********************************
#define PUMP_OUT_IO					13


// ***********************************
// Constants
// ***********************************
#define PUMP_RUNNING_TIME_MS				      60000	// Time the pump will run for, in milliseconds
#define SERIAL_DEBUG_OUTPUT_TIMER_MS			1000	// How often pump status will be output via debug serial, in milliseconds


// ***********************************
// Variables
// ***********************************
AcksenPump WaterPump(PUMP_OUT_IO, -1);

unsigned long ulStopPumpingTimer;		  // Timer used to stop Pump
unsigned long ulPumpDebugOutputTimer;	// Timer used to output pump status via debug serial periodically
unsigned long ulStartTime;             // Time program operation started


// ************************************************
// Setup 
// ************************************************
void setup()
{

	// Initialise Serial Port
	Serial.begin(DEBUG_BAUD_RATE);

	// Set I/O
	pinMode(PUMP_OUT_IO, OUTPUT);

	// Set I/O Initial State
	digitalWrite(PUMP_OUT_IO, WaterPump.iPumpOffState);

  
	// Set custom Pump Relay Switching Delay
	WaterPump.iPumpRelaySwitchingDelay = 1000;	// Millisecond

	// Enable Pump Ventilation System
	WaterPump.bEnablePumpVentilation = true;
	
	// Setup Timers
  	ulStartTime = millis();
	ulStopPumpingTimer = millis() + PUMP_RUNNING_TIME_MS;
	ulPumpDebugOutputTimer = millis() + SERIAL_DEBUG_OUTPUT_TIMER_MS;
	
	// Turn the Pump ON, at the start
	if (WaterPump.iControlState == PUMP_CONTROL_STOP)
	{
	
		// Toggle Pump State (should be from OFF to ON)
		// This will automatically execute any Pump Ventilation when starting (if feature enabled)
		WaterPump.ToggleState();
		Serial.println("*** Pump turned ON!");
		
	}
	
	Serial.println("Startup Complete!");
	
}

// ************************************************
// Main Control Loop
// ************************************************
void loop()
{

	// Run the Pump Control Loop (automatically updating pump ventilation state, max temperature checks, etc)
	WaterPump.process();
	
	// Check to see if the pump run timer has elapsed
	if (ulStopPumpingTimer <= millis())
	{
	
		// Turn the Pump OFF
		WaterPump.turnOff();
		
		Serial.println("*** Timer Elapsed - Pump Turned OFF!");	
		Serial.println("*** Program Complete.");
		
		// Halt program execution
		while(1);	
	
	}
	
	// Check if time to output debug serial status on pump
	if (ulPumpDebugOutputTimer <= millis())
	{
	
		Serial.print("Pump Control State = ");
		
		switch (WaterPump.iControlState)
		{
			case PUMP_CONTROL_STOP:
				Serial.print("STOP");
				break;
			case PUMP_CONTROL_VENT:
				Serial.print("VENTING");
				break;
			case PUMP_CONTROL_ON:	
				Serial.print("ON");
				break;
			case PUMP_CONTROL_GRAIN_REST:						
				Serial.print("GRAIN REST");
				break;
			default:
				Serial.print("Unknown");
				break;
		}

    		Serial.print(", Pump Output = ");
    
		switch (WaterPump.iOutputStateActual)
		{
			case PUMP_OUTPUT_STATE_OFF:
				Serial.print("OFF");
				break;
			case PUMP_OUTPUT_STATE_ON:
				Serial.print("ON");
				break;
			default:
				Serial.print("Unknown");
				break;
		}  

		Serial.print(", Elapsed Time = ");
		Serial.print(float(millis() - ulStartTime) / float(1000));
		Serial.println(" Seconds");
		
		// Update Timer for next execution
		ulPumpDebugOutputTimer = millis() + SERIAL_DEBUG_OUTPUT_TIMER_MS;
		
	}
	
	

	
	
}