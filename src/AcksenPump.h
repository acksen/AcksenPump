/*!
@file AcksenPump.h
 
*/
 
/***********************************************************
This source file is licenced using the 3-Clause BSD License.

Copyright (c) 2022 Acksen Ltd, All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************/

// Acksen Pump Library v1.8.0
// (c) Acksen Ltd 2022
//
// Collection of function libraries for Acksen Pump Control.
// 
// v1.8.0	26 Jul 2022
// - Add licence, other cosmetic/comments changes for preparation for open source release
//
// v1.7.4	13 Mar 2020
// - Add ability to set negative/positive logic for Pump Control
//
// v1.7.3	14 Jan 2020
// - Allow alteration of Pump Relay Switching Delay in individual instances of library/pump
//
// v1.7.2	01 May 2019
// - Modify library to enable/inhibit Grain Rests to only occur when actively mashing
// - Changes to Grain Rest Prevention system when close to Set Point
//
// v1.7.1	24 Apr 2019
// - Max Pump Temp default reduced to 93C
//
// v1.7.0	03 Apr 2019
// - Add support for external voltage phase input which governs pump relay state changes.
//
// v1.6.0	16 Jan 2019
// - Add Relay Switching Delay of PUMP_RELAY_SWITCHING_DELAY after switching the associated pump relay on or off
//
// v1.5.0	27 Sep 2018
// - Modify Grain Rests to be stored in Minutes rather then Seconds.
//
// v1.4.0	04 May 2018
// - Change Enable Max Pump Temp default to OFF.
// - Add option for Temp Probe Offset.
//
// v1.3.0	15 Mar 2018
// - Change to AcksenPump library
//
// v1.2.0	22 Dec 2017
// - Add function to explicitly and immediately turn OFF the pump.
//
// v1.1.0	30 Nov 2017
// - Add Inhibit function for Grain Rests to delay activation at request of Host.
// - Enable Max Pump Temp by default
// - Set Grain Rests to activate every 10 minutes
//
// v1.0.0	27 Oct 2017
// - Initial Version
//

#ifndef AcksenPump_h
#define AcksenPump_h

#define AcksenPump_ver   180	///< Constant used to set the present library version. Can be used to ensure any code using this library, is correctly updated with necessary changes in subsequent versions, before compilation.

#include <Time.h>
#include <TimeLib.h>
#include <Arduino.h>

// *** PUMP CONSTANTS ***
#define PUMP_POSITIVE_LOGIC_ON	    1	///< Output state for Pump in ON state, when positive logic is used.
#define PUMP_POSITIVE_LOGIC_OFF		0	///< Output state for Pump in OFF state, when positive logic is used.

#define PUMP_NEGATIVE_LOGIC_ON	    0	///< Output state for Pump in ON state, when negative logic is used.
#define PUMP_NEGATIVE_LOGIC_OFF		1	///< Output state for Pump in OFF state, when negative logic is used.

#define PUMP_VENTILATION_CYCLE_ON_TIME_DEFAULT		5 ///< During Pump Ventilation Sequence, number of seconds the pump will be held ON for.
#define PUMP_VENTILATION_CYCLE_OFF_TIME_DEFAULT		2 ///< During Pump Ventilation Sequence, number of seconds the pump will be held OFF for.
#define PUMP_VENTILATION_CYCLE_COUNT_DEFAULT		3 ///< During Pump Ventilation Sequence, the number of ON/OFF cycles the pump will be taken through for initial venting of trapped air.
#define PUMP_VENTILATION_ENABLED_DEFAULT			true	///< Enable Pump Ventilation Sequence, which takes the pump through a number of ON/OFF cycles when turned on, to vent any trapped air in the pump.  This improves pump life and reduces operating noise.

#define PUMP_VENT_CYCLES_MIN		1	///< Minimum number of Pump Ventilation Cycles that can be set. To be used in configuration settings/menus for accompanying code, not directly utilised in library.
#define PUMP_VENT_CYCLES_MAX		5	///< Maximum number of Pump Ventilation Cycles that can be set. To be used in configuration settings/menus for accompanying code, not directly utilised in library.

#define PUMP_VENT_TIME_MIN			1	///< Minimum cycle time (ON or OFF) for pump venting sequence. To be used in configuration settings/menus for accompanying code, not directly utilised in library.
#define PUMP_VENT_TIME_MAX			10	///< Maximum cycle time (ON or OFF) for pump venting sequence. To be used in configuration settings/menus for accompanying code, not directly utilised in library.

// Pump Operating Modes
#define PUMP_OPERATING_MODE_OFF						0	///< Set the Pump Operating Mode to ON.
#define PUMP_OPERATING_MODE_ON						1	///< Set the Pump Operating Mode to OFF.

// Pump State Machine
#define PUMP_CONTROL_STOP							0	///< Pump State is presently OFF/Stopped.
#define PUMP_CONTROL_VENT							1	///< Pump State is presently Venting.
#define PUMP_CONTROL_ON								2	///< Pump State is presently ON/Running.
#define PUMP_CONTROL_GRAIN_REST						3	///< Pump State is presently in Grain Rest timeout.

// Pump Output States
#define PUMP_OUTPUT_STATE_OFF					5	///< Set Pump Output to OFF.
#define PUMP_OUTPUT_STATE_ON					6	///< Set Pump Output to ON.

// Grain Rest
#define GRAIN_REST_LENGTH_DEFAULT   			1  	///< Default Length of Grain Rests, in Minutes.
#define MIN_GRAIN_REST_LENGTH					1	///< Minimum Length of Grain Rests, in Minutes. To be used in configuration settings/menus for accompanying code, not directly utilised in library.
#define MAX_GRAIN_REST_LENGTH					10	///< Maximum Length of Grain Rests, in Minutes. To be used in configuration settings/menus for accompanying code, not directly utilised in library.

#define GRAIN_REST_PERIOD_DEFAULT   			5 	///< Default Periodic Interval between Grain Rests, in Minutes.
#define MIN_GRAIN_REST_PERIOD					1	// Minimum Period between Grain Rests, in Minutes. To be used in configuration settings/menus for accompanying code, not directly utilised in library.
#define MAX_GRAIN_REST_PERIOD					20	// Maximum Period between Grain Rests, in Minutes. To be used in configuration settings/menus for accompanying code, not directly utilised in library.

#define ENABLE_GRAIN_REST_DEFAULT								true	///< Enable the Grain Rest system by default.
#define TEMPORARY_INHIBIT_GRAIN_REST_AROUND_SET_POINT_DEFAULT	true	///< Do not allow Grain Rests to commence when close to the system Set Point, for heating/cooling systems. To be used by accompanying code, not directly utilised in library.

// Pump Operating Temperature
#define MAX_PUMP_TEMP_DEFAULT					93		///< Maximum Pump Operating Temperature, in Celsius.  Pump will cease to function above this limit to prevent damage.
#define ENABLE_MAX_PUMP_TEMP_DEFAULT			true	///< Enable checking of Pump Temperature (set by calling updatePumpTemperature()) against Maximum Pump Operating Temperature. 

#define PUMP_RELAY_SWITCHING_DELAY					200	///< Add a delay after switching the Pump output state, to allow for relay settling, in Milliseconds.

#define PHASE_SYNC_PRE_ACTIVATION_DELAY_DEFAULT		0	// Default delay after detecting Voltage Zero Crossing and switching Relay ON/OFF State, in Milliseconds.
#define PHASE_SYNC_PRE_ACTIVATION_DELAY_MAX			9	// Minimum delay after detecting Voltage Zero Crossing and switching Relay ON/OFF State, in Milliseconds. To be used in configuration settings/menus for accompanying code, not directly utilised in library.
#define PHASE_SYNC_PRE_ACTIVATION_DELAY_MIN			0	// Maximum delay after detecting Voltage Zero Crossing and switching Relay ON/OFF State, in Milliseconds. To be used in configuration settings/menus for accompanying code, not directly utilised in library.
#define PHASE_SYNC_ENABLED_DEFAULT					false	///< Allow the Pump ON/OFF Switching to be synchronised with a Voltage Zero Crossing detector input, to minimise electrical issues when switching an SSR or Relay for an AC Pump.

/**************************************************************************/
/*! 
    @brief  Class that defines the AcksenPump state and functions
*/
/**************************************************************************/
class AcksenPump
{

public:

	// Variables
	int iPumpOnState = PUMP_POSITIVE_LOGIC_ON;		///< Define the Output State that is set when Pump is ON.  Positive Logic (0=OFF, 1=ON) by default, Can be overrided for Negative logic.
	int iPumpOffState = PUMP_POSITIVE_LOGIC_OFF;	///< Define the Output State that is set when Pump is ON.  Positive Logic (0=OFF, 1=ON) by default, Can be overrided for Negative logic.

	bool bEnablePumpVentilation = PUMP_VENTILATION_ENABLED_DEFAULT;	///< Enable/Disable Pump Ventilation system on pump startup
	int iPumpVentilationCycles = PUMP_VENTILATION_CYCLE_COUNT_DEFAULT;	///< Number of Pump Ventilation ON/OFF cycles on startup
	int iPumpVentilationOnLength = PUMP_VENTILATION_CYCLE_ON_TIME_DEFAULT;	///< Length of Pump being set to ON during Ventilation Cycle, in Seconds.
	int iPumpVentilationOffLength = PUMP_VENTILATION_CYCLE_OFF_TIME_DEFAULT;///< Length of Pump being set to OFF during Ventilation Cycle, in Seconds.

	int iOperatingMode = PUMP_OPERATING_MODE_OFF;		///< Pump Operating Mode
	int iControlState = PUMP_CONTROL_STOP;				///< Pump Control State
	int iOutputStateRequested = PUMP_OUTPUT_STATE_OFF;	///< Pump Output State that has been Requested
	int iOutputStateActual = PUMP_OUTPUT_STATE_OFF;		///< Actual Pump Output State presently
	int iVentilationCycleRuntimeCount;					///< Number of Pump Ventilation Cycles that have been executed in present Ventilation phase

	time_t dtVentEndTime, dtVentStartTime;				///< Start/End Time for present Pump Ventilation Phase
	time_t dtGrainRestEndTime;							///< Time that next Grain Rest will execute
	time_t dtGrainRestPeriodStartTime;					///> Start Time for the present Grain Rest phase

	int iGrainRestLength = GRAIN_REST_LENGTH_DEFAULT;	///< Length of Grain Rest (how long pump will be OFF for, before restarting)
	int iGrainRestPeriod = GRAIN_REST_PERIOD_DEFAULT;	///< Interval Period between Grain Rests
	
	bool bEnableMaxPumpTemperature = ENABLE_MAX_PUMP_TEMP_DEFAULT;	///< Enable the Maximum Pump Temperature monitoring system
	int iMaxPumpTemperature = MAX_PUMP_TEMP_DEFAULT;	///< Maximum Pump Operating Temperature.  Pump will be disabled above this level.
	
	float fPumpTemperature;								///< The present operating temperature of the Pump.  Updated using updatePumpTemperature() with the reading from a measurement probe.

	bool bTempFlagForInhibitGrainRestAsAroundPreheatSetPoint = true;	///< Flag used to let calling code know if the Pump is presently inhibiting a Grain Rest due to being close to Set Point for Temp Control.
	
	bool bEnableGrainRest = ENABLE_GRAIN_REST_DEFAULT;	///< Enable the Grain Rest system.
	bool bEnableInhibitGrainRestAroundSetPoint = TEMPORARY_INHIBIT_GRAIN_REST_AROUND_SET_POINT_DEFAULT;///< Inhibit the operation of the Grain Rest System around the Set Point for Temperature Control.
	
	bool bEnablePhaseSync = PHASE_SYNC_ENABLED_DEFAULT;	///> Enable the Voltage Phase Sync system for Zero Crossing Detection when switching Pump Output State ON/OFF.
	int iPhaseSyncPreActivationDelay = PHASE_SYNC_PRE_ACTIVATION_DELAY_DEFAULT;	///< Delay between detecting Zero Crossing, and changing Pump Output State.

	bool bCurrentlyControllingMashing = false;	///< Set when the Pump is being used for controlling Grain Mashing for Brewing.
	
	int iPumpRelaySwitchingDelay = PUMP_RELAY_SWITCHING_DELAY;	///< Delay added after switching Pump Output ON/OFF, to allow for relay settling.

/**************************************************************************/
/*!
    @brief  Class initialisation.
            Sets up the Pump using starting defined parameters.
    @param  iPumpOutputPin
            The Arduino I/O pin assigned to the Pump Output.
    @param  iPhaseSyncInputPin
            The Arduino I/O pin assigned to the Voltage Phase Sync Output.  Set to -1 if the system is not being used.
    @return No return value.
*/
/**************************************************************************/
	AcksenPump(int iPumpOutputPin, int iPhaseSyncInputPin);
	
/**************************************************************************/
/*!
    @brief 	Toggle the Pump State (from ON to OFF, or OFF to ON)
    @return No return value.
*/
/**************************************************************************/
	void ToggleState();
	
/**************************************************************************/
/*!
    @brief  Reset the start time of the next Grain Rest, based on the Grain Rest Period that has been set
    @return No return value.
*/
/**************************************************************************/
	void resetGrainRest();
	
/**************************************************************************/
/*!
    @brief  Process any updates to automatic Pump operations, including Max Temperature check, running Grain Rests and Pump Ventilation.  This should be called regularly.
    @return No return value.
*/
/**************************************************************************/
	void process();
	
/**************************************************************************/
/*!
    @brief  Set the Pump Temperature, using an external temperature reading.  This is used by the Maximum Pump Temperature supervisory system.
    @return No return value.
*/
/**************************************************************************/
	void updatePumpTemperature(float fNewPumpTemperature);


/**************************************************************************/
/*!
    @brief  Used to determine if the Actual Output State of the Pump, doesn't match the Requested Output State.
    @return Returns true if the state changed.
			Returns false if the state did not change.
*/
/**************************************************************************/
	bool stateChangeOccurred();

/**************************************************************************/
/*!
    @brief  Call when using the Pump in a Control System, and you wish to inhibit Grain Rests triggering as normal due to being near the System Set Temperature.
    @return No return value.
*/
/**************************************************************************/
	void temporaryInhibitGrainRestAsAroundPreheatSetPoint();

/**************************************************************************/
/*!
    @brief  Call when using the Pump in a Control System, and you wish to re-enable Grain Rests triggering as normal, after moving away from being near to the System Set Temperature.
    @return No return value.
*/
/**************************************************************************/
	void temporaryPermitGrainRestAsAroundPreheatSetPoint();

/**************************************************************************/
/*!
    @brief  Call when using the Pump in a Brewing Control System for Grain Mashing.
			This sets bCurrentlyControllingMashing to True, which can be used externally to treat this differently from plain water heating control.
    @return No return value.
*/
/**************************************************************************/
	void beginMashingControl();

/**************************************************************************/
/*!
    @brief  Call when a Grain Mashing sequence in a Brewing Control System has ended.
			This sets bCurrentlyControllingMashing to True, which can be used externally to treat this differently from plain water heating control.
    @return No return value.
*/
/**************************************************************************/
	void endMashingControl();
	
/**************************************************************************/
/*!
    @brief  Turn the Pump OFF.
    @return No return value.
*/
/**************************************************************************/
	void turnOff();
	
/**************************************************************************/
/*!
    @brief  Set the Pump output to use Negative Logic (0=ON, 1=OFF) for control.
    @return No return value.
*/
/**************************************************************************/
	void switchPumnpNegativeLogic(void);

protected: 
	
	int _iPumpOutputPin;
	int _iPhaseSyncInputPin;
	
	bool _bStateChangeOccurred = false;
	
	void waitForPhaseSync();
	bool waitForPin(uint8_t pin, uint8_t value, uint16_t timeout);
	
	void setNegativeSwichingLogic(bool bPositiveSwitchingState);

};

#endif


