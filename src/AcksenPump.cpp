/***********************************************************/
/*!

@file AcksenPump.cpp

@mainpage Brewing-focused pump control I/O library for Arduino.

@section intro_sec Introduction

This is the documentation for Acksen Ltd's AcksenPump Brewing-focused pump control 
I/O library for Arduino.

This is a brewing-focused pump control library for Arduino supporting pump ventilation, 
grain rests, maximum operating temp and more.

@section dependencies Dependencies

Requires the Arduino Time Library by Michael Margolis (https://github.com/PaulStoffregen/Time)

Arduino Library rev.2.2 - requires Arduino IDE v1.8.10 or greater.

@section author Author

Written by Richard Phillips for Acksen Ltd.

@section license License

This source file is licenced using the 3-Clause BSD License.

Copyright (c) 2022, 2023 Acksen Ltd, All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/***********************************************************/

// Acksen Pump Library v1.8.1

#include "Arduino.h"
#include "AcksenPump.h"

AcksenPump::AcksenPump(int iPumpOutputPin, int iPhaseSyncInputPin)
{
	
	this->_iPumpOutputPin = iPumpOutputPin;
	this->_iPhaseSyncInputPin = iPhaseSyncInputPin;
	
	// Set as Output
	pinMode(this->_iPumpOutputPin, OUTPUT);
	
	// Set Phase Sync as Input
	if (this->_iPhaseSyncInputPin != -1)
	{
		pinMode(this->_iPhaseSyncInputPin, INPUT);
	}
	
	// Set Pump Off
	digitalWrite(this->_iPumpOutputPin, iPumpOffState);
	
}

void AcksenPump::turnOff()
{
	
	// Pump set to off, no Pump Vent;
	this->iControlState = PUMP_CONTROL_STOP;
	this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;

	// Pump Operating Mode OFF
	this->iOperatingMode = PUMP_OPERATING_MODE_OFF;
	
	int iInitialPumpState = digitalRead(this->_iPumpOutputPin);
	
	if (digitalRead(this->_iPumpOutputPin) == iPumpOnState)
	{
		// Deactivate Pump Output
		waitForPhaseSync();
		digitalWrite(this->_iPumpOutputPin, iPumpOffState);
	}
	
	this->iOutputStateActual = PUMP_OUTPUT_STATE_OFF;
	
	// If the pump was on previously, apply the relay switching delay since we've just turned it off.
	if (iInitialPumpState != iPumpOffState)
	{
		delay(iPumpRelaySwitchingDelay);

		launchCallbackInitLCDs();
	}
	
}

void AcksenPump::ToggleState()
{
	
	if (this->iControlState == PUMP_CONTROL_STOP)
	{

		// Check to see if the Pump Temperature has exceeded Maximum Levels
		if ((this->bEnableMaxPumpTemperature == true) && (this->fPumpTemperature >= this->iMaxPumpTemperature))
		{
			// Ignore Pump Activation
		}
		else
		{
		
			if (this->bEnablePumpVentilation == true)
			{
				// Pump One set to ON, Pump Vent On
				this->iControlState = PUMP_CONTROL_VENT;
				this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;

				// Start Pump Ventilation Cycle
				this->iVentilationCycleRuntimeCount = 0;
				//dtSystemCycleStartTime = now();
				//dtSystemCycleEndTime = dtSystemCycleStartTime + this->iPumpVentilationLength;

			}
			else
			{
				// Pump set to ON, no Pump Vent
				this->iControlState = PUMP_CONTROL_ON;
				this->iOutputStateRequested = PUMP_OUTPUT_STATE_ON;
			}

			// Pump One Operating Mode ON
			this->iOperatingMode = PUMP_OPERATING_MODE_ON;

			// setup next Grain Rest timing (if required!)
			this->resetGrainRest();

		}
		
	}
	else
	{

		// Pump set to off, no Pump Vent
		this->iControlState = PUMP_CONTROL_STOP;
		this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;

		// Pump Operating Mode OFF
		this->iOperatingMode = PUMP_OPERATING_MODE_OFF;

	}
	
}

void AcksenPump::resetGrainRest()
{
	// Resetting Grain Rest
	dtGrainRestPeriodStartTime = now() + (this->iGrainRestPeriod * 60);
	dtGrainRestEndTime = now();	
}

void AcksenPump::updatePumpTemperature(float fNewPumpTemperature)
{
	this->fPumpTemperature = fNewPumpTemperature;
}

bool AcksenPump::stateChangeOccurred()
{

	if (this->_bStateChangeOccurred == true)
	{

		// Reset Flag
		this->_bStateChangeOccurred = false;

		return true;
	}
	else
	{

		return false;
	}

}


void AcksenPump::process()
{
	
	// Check to see if the Pump Temperature has exceeded Maximum Levels
	if ((this->bEnableMaxPumpTemperature == true) && (this->fPumpTemperature >= this->iMaxPumpTemperature))
	{
		// Ensure that the Pump is turned off!				
		this->iControlState = PUMP_CONTROL_STOP;
		this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;
	}
	else
	{

		// Pump Ventilation Control
		if (this->iControlState == PUMP_CONTROL_VENT)
		{

			// Pump Vent Control Loop

			// Initial Setup Condition
			if ((this->iVentilationCycleRuntimeCount == 0) && (this->iOutputStateRequested == PUMP_OUTPUT_STATE_OFF))
			{

				// Set initial Pump Ventilation conditions

				// Start another Pump Ventilation Cycle
				this->dtVentStartTime = now();
				this->dtVentEndTime = this->dtVentStartTime + this->iPumpVentilationOnLength;

				this->iOutputStateRequested = PUMP_OUTPUT_STATE_ON;

			}

			// Check to see if the present condition has elapsed
			if (now() >= this->dtVentEndTime)
			{

				// Pump Vent Cycle End Check

				if (this->iOutputStateRequested == PUMP_OUTPUT_STATE_ON)
				{
					// ON cycle completed
					this->iVentilationCycleRuntimeCount++;

					// Turn Pump OFF
					this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;

					// Setup next cycle times
					this->dtVentStartTime = now();
					this->dtVentEndTime = this->dtVentStartTime + this->iPumpVentilationOffLength;

				}
				else if (this->iOutputStateRequested == PUMP_OUTPUT_STATE_OFF)
				{
					// OFF cycle completed

					// Turn Pump ON
					this->iOutputStateRequested = PUMP_OUTPUT_STATE_ON;

					// Setup next cycle times
					this->dtVentStartTime = now();
					this->dtVentEndTime = this->dtVentStartTime + this->iPumpVentilationOnLength;

				}

			}

			// Check to see if the pump ventilation phase has ended
			if (this->iVentilationCycleRuntimeCount > this->iPumpVentilationCycles)
			{
				// Ventilation Complete
				// Terminate Pump Ventilation.
				// Pump Operating Mode set to this->iOperatingMode

				// Move to next pump control stage
				if (this->iOperatingMode == PUMP_OPERATING_MODE_OFF)
				{
					// Moving to Stop Pump 
					this->iControlState = PUMP_CONTROL_STOP;
					this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;
				}
				else if (this->iOperatingMode == PUMP_OPERATING_MODE_ON)
				{
					// Moving to Start Pump
					this->iControlState = PUMP_CONTROL_ON;
					this->iOutputStateRequested = PUMP_OUTPUT_STATE_ON;
				}

			}

		}

		// Pump Grain Rest Control
		if ((this->iControlState == PUMP_CONTROL_GRAIN_REST) && (this->iOperatingMode == PUMP_OPERATING_MODE_ON) && (this->iGrainRestLength != 0))
		{

			// Grain Rest Period

			// Ensure that the Pump is temporarily turned off
			this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;
			this->iOutputStateActual = PUMP_OUTPUT_STATE_OFF;

			// Check to see if the present condition has elapsed
			if (now() >= this->dtGrainRestEndTime)
			{

				// Grain Rest Complete
				// Terminate Grain Rest.

				if (this->iOperatingMode == PUMP_OPERATING_MODE_OFF)
				{
					// Now disable pump again
					// Pump Off - no need to vent.
					this->iControlState = PUMP_CONTROL_STOP;
					this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;
				}
				else
				{
					// Activate Post-Rest Grain Bed

					// Initialise Mandatory Pump Vent
					this->iControlState = PUMP_CONTROL_VENT;
					this->iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;

					// Start Pump Ventilation Cycle
					this->iVentilationCycleRuntimeCount = 0;
				}

			}

		}
		
	}


	// I/O Control Function

	// Check to see if a change needs to be made
	if (this->iOutputStateRequested != this->iOutputStateActual)
	{
		// Set State Change Occurred flag for use by calling software
		this->_bStateChangeOccurred = true;
	}

	// Match the Demand State!
	if (this->iOutputStateRequested == PUMP_OUTPUT_STATE_ON)
	{
	
		if (digitalRead(this->_iPumpOutputPin) == iPumpOffState)
		{
			waitForPhaseSync();

			// Pump ON
			digitalWrite(this->_iPumpOutputPin, iPumpOnState);
		}
		
		// If thrfe relay state is changing, incur a delay.
		if (this->iOutputStateRequested != this->iOutputStateActual)
		{
			delay(iPumpRelaySwitchingDelay);
			launchCallbackInitLCDs();
		}
			
		this->iOutputStateActual = PUMP_OUTPUT_STATE_ON;
	}
	else
	{
		if (digitalRead(this->_iPumpOutputPin) == iPumpOnState)
		{
			waitForPhaseSync();
			
			// Pump OFF
			digitalWrite(this->_iPumpOutputPin, iPumpOffState);
		}
		
		// If the relay state is changing, incur a delay.
		if (this->iOutputStateRequested != this->iOutputStateActual)
		{
			delay(iPumpRelaySwitchingDelay);
			launchCallbackInitLCDs();
		}

		this->iOutputStateActual = PUMP_OUTPUT_STATE_OFF;
	}

}

void AcksenPump::beginMashingControl(void)
{
	bCurrentlyControllingMashing = true;
}

void AcksenPump::endMashingControl(void)
{
	bCurrentlyControllingMashing = false;
}

void AcksenPump::temporaryInhibitGrainRestAsAroundPreheatSetPoint(void)
{
	bTempFlagForInhibitGrainRestAsAroundPreheatSetPoint = true;
}

void AcksenPump::temporaryPermitGrainRestAsAroundPreheatSetPoint(void)
{
	bTempFlagForInhibitGrainRestAsAroundPreheatSetPoint = false;
}

void AcksenPump::waitForPhaseSync(void)
{
	
	if ((this->bEnablePhaseSync == false) || (this->_iPhaseSyncInputPin == -1))
	{
			// Phase Sync not setup or disabled - return immediately.
			return;
	}
	
	// Check to see if the phase input is negative before proceeding
	if (digitalRead(this->_iPhaseSyncInputPin) == true)
	{
		// Have to wait until the phase input is negative!
		waitForPin(this->_iPhaseSyncInputPin, false, 20);
	}
	
	// Check to see if the rising edge trigger has been received
	waitForPin(this->_iPhaseSyncInputPin, true, 20);
	
	// Apply additional delay before continuing to operate output relay
	delay(this->iPhaseSyncPreActivationDelay);
	
	
}

// Wait for the given pin to become the given value. Returns true when
// that happened, or false when timeout ms passed
bool AcksenPump::waitForPin(uint8_t pin, uint8_t value, uint16_t timeout)
{
	unsigned long start = millis();

	while (true)
	{
		if (digitalRead(pin) == value)
		{
			return true;
		}
		if (millis() - start > timeout)
		{
			return false;
		}
	}
}

void AcksenPump::switchPumpNegativeLogic(void)
{

	// Set Negative Logics for Pump

	// Allow the normal logic used to be inverted
	this->iPumpOnState = PUMP_NEGATIVE_LOGIC_ON;
	this->iPumpOffState = PUMP_NEGATIVE_LOGIC_OFF;

}

void AcksenPump::launchCallbackInitLCDs()
{
	(*callbackInitLCDs)();     // call the handler  
}