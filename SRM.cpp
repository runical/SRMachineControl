/*
  SRMachineControl.cpp - Library for controlling a SR machine with an Arduino Due.
 
  Copyright (c) 2014 Peter van den Hurk

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

//#include "arduino.h"
#include "SRM.h"

//////////////////////////////////////////
// 				PhysicalSwitch			//
//////////////////////////////////////////

PhysicalSwitch::PhysicalSwitch(int switchnumber, int pin)
{
	// PhysicalSwitch sets the pin mode to output, inits it to low and saves the pin number.
	
	pinMode(pin, OUTPUT);
	this->_state = true;
	this->_pin = pin;
	this->Deactivate();
	
	static int number = 1;
	this->_switchnumber = number;
	number++;
}

void PhysicalSwitch::Activate()
{	
	// Activate a switch
	if(not this->_state)
	{
		digitalWrite(this->_pin, HIGH);
		this->_state = true;
		//Serial.print("ON: ");
		//Serial.print(this->_switchnumber);
		//Serial.println(" ");
	}
}

void PhysicalSwitch::Deactivate()
{
	// Deactivate a switch
	if(this->_state)
	{
		digitalWrite(this->_pin, LOW);
		this->_state = false;
		//Serial.print("OFF: ");
		//Serial.print(this->_switchnumber);
		//Serial.println(" ");
	}
}

//////////////////////////////////////////
// 				SwitchState				//
//////////////////////////////////////////

SwitchState::SwitchState(PhysicalSwitch** activeSwitches, int nSwitches)
{
	// Switchstates takes an array of Switches, which can be activated in groups.
	this->_activeSwitches = activeSwitches;
	this->_nSwitches = nSwitches;
	this->_next = this;
	this->_previous = this;
	
	static int number = 1;
	this->_statenumber = number;
	number++;
}

void SwitchState::InsertSwitchState(SwitchState* insertedState)
{
	// Function for the ring. This inserts a state after this state.
	insertedState->SetPrevious(this);
	insertedState->SetNext(this->_next);
	_next->SetPrevious(insertedState);
	this->SetNext(insertedState);
}

void SwitchState::SetNext(SwitchState* insertedState)
{
	// set the _next variable
	_next = insertedState;
}

void SwitchState::SetPrevious(SwitchState* insertedState)
{
	// set the _previous variable
	_previous = insertedState;
}

SwitchState* SwitchState::GetNext()
{
	// get the _next variable
	return _next;
}

SwitchState* SwitchState::GetPrevious()
{
	// get the _previous variable
	return _previous;
}

int SwitchState::GetNumberOfSwitches()
{
	// get the number of switches (in _nSwitches variable)
	return _nSwitches;
}

PhysicalSwitch** SwitchState::GetSwitches()
{
	// Get the switches associated with the variable
	return _activeSwitches;
}

//////////////////////////////////////////////
//				InverterStage				//
//////////////////////////////////////////////

InverterStage::InverterStage(SwitchState* topState)
{
	// Initialize the InverterStage. Only stores the topstate.
	this->_currentState = topState;
	this->_startState = topState;
}

void InverterStage::TurnOff()
{
	// Turn off the current state
	int limit = this->_currentState->GetNumberOfSwitches();
	PhysicalSwitch** tempSwitches = this->_currentState->GetSwitches();
	
	int i;
	
	//Serial.print("Turning OFF: State ");
	//Serial.println(this->_currentState->_statenumber);
	
	for(i = 0 ; i < limit ; i++)
	{
		tempSwitches[i]->Deactivate();
	}
}

void InverterStage::ActivateNextState()
{
	this->ActivateState(this->_currentState->GetNext());
}

void InverterStage::ActivatePreviousState()
{
	this->ActivateState(this->_currentState->GetPrevious());
}

void InverterStage::ActivateCurrentState()
{
	this->ActivateState(this->_currentState);
}

// Private ActivateState, because this makes the other interface functions easier.

void InverterStage::ActivateState(SwitchState* activatedState)
{
	// Activates the given SwitchState, while also deactivating the current state (to prevent double activation)
	
	// Then, get the relevant switches and turn them ON
	int limit = activatedState->GetNumberOfSwitches();
	PhysicalSwitch** tempSwitches = activatedState->GetSwitches();
	
	int i;
	
	// First, turn OFF the current state
	this->TurnOff();
	
	// Turn the new switches ON
	//Serial.print("Turning ON: State ");
	//Serial.println(activatedState->_statenumber);
		
	for(i = 0 ; i < limit ; i++)
	{
		tempSwitches[i]->Activate();
	}
		
	// Lastly, save the activated state as the CURRENT state.
	// This way, we can never lose track of the state and rule out 
	// any possible errors related to turning 2 sets of switches on.
	this->_currentState = activatedState;
}

//////////////////////////////////////////
// 				Controller				//
//////////////////////////////////////////

// The controller implements the logic and setup.

Controller::Controller(SwitchState* topState, Encoder* theEncoder, int pulsesPerRev, int eRevPerMRev, int nStates, int offset, int direction)
{
	
	Serial.println("Initializing controller");
	
	// Init the needed objects
	this->_encoder = theEncoder;
	this->_inverterStage = new InverterStage(topState);
	
	// Init known variables (and check given variables)
	// The variables that are not here are to be calculated
	if(direction < 0)
	{
		this->_direction = -1;
	}
	else
	{
		this->_direction = 1;
	}
	
	this->_startPosition = this->_direction * (offset  - pulsesPerRev);
	this->_endPosition = this->_direction * offset;
	this->_overflow = false;
	
	// Calculate error and error correction
	/* 
	 * This is done by finding the error on a complete mechenical rev.
	 * The  number of states the system goes through per mechanical rev is also derived.
	 * Then, we find the greatest common divider (with the gcd algorithm), divide 
	 * both of them by this gcd and save the result. Then repeating this untill the gcd
	 * is 1. This gives us the smallest possible integer amount of switching states with 
	 * the smalles error to correct.
	 */
	
	Serial.println("Starting the calculations");
	
	this->_correctionCondition = eRevPerMRev*nStates;
	this->_increase = (int) ( (float) pulsesPerRev / (float) this->_correctionCondition );
	
	this->_error = pulsesPerRev - (this->_increase * this->_correctionCondition);
	int factor;
	
	do{
		if(this->_error > this->_correctionCondition)
		{
			factor = this->gcd(this->_error, this->_correctionCondition);
		}
		else
		{
			factor = this->gcd(this->_correctionCondition, this->_error);
		}
		this->_error = this->_error/factor;
		this->_correctionCondition = this->_correctionCondition/factor;
	} while(factor != 1);
	
	this->_switchingCounter = 0;
	
	Serial.print("increase = ");
	Serial.println(this->_increase);
	Serial.print("error = ");
	Serial.println(this->_error);
	Serial.print("correctionCondition = ");
	Serial.println(this->_correctionCondition);
	
	// Calibration
	Serial.println("Starting with calibration in 10 seconds");
	this->ControllerDelay(10);
	this->Calibrate();
	
	Serial.println("Initial conditions:");
	Serial.print("startPosition: ");
	Serial.println(this->_startPosition);
	Serial.print("endPosition: ");
	Serial.println(this->_endPosition);
	Serial.print("Position: ");
	Serial.println(this->_encoder->read());
	
	// Activate the next or previous state, depending on the direction.
	Serial.println("Starting the motor");
	Serial.end();
}

void Controller::Logic()
{
	// The controller logic.
	// It is the differential algorithm
	
	int position = this->_encoder->read();
	
	if(this->_direction == 1)
	{
		// Positive Direction
		if( this->_overflow )
		{
			if( position >= this->_endPosition && position < this->_startPosition )
			{
				this->_inverterStage->ActivateNextState();
				this->_overflow = this->CalculateNext();
			}
		}
		else if( position >= this->_endPosition || position < this->_startPosition )
		{
			this->_inverterStage->ActivateNextState();
			this->_overflow = this->CalculateNext();
		}
		return;
	}
	else if(this->_direction == -1)
	{
		// Negative Direction
		if( this->_overflow )
		{
			if( position <= this->_endPosition && position > this->_startPosition )
			{
				this->_inverterStage->ActivatePreviousState();
				this->_overflow = this->CalculateNext();
			}
		}
		else if( position <= this->_endPosition || position > this->_startPosition )
		{
			this->_inverterStage->ActivatePreviousState();
			this->_overflow = this->CalculateNext();
		}
		return;
	}
}

bool Controller::CalculateNext()
{
	// Calculate the next step.
	// bidirectional
	int change = this->_direction * ( this->_increase + this->Correction() );
	
	this->_startPosition = this->_startPosition + change;
	this->_endPosition = this->_endPosition + change;
	
	// Find overflow
	if( ( this->_direction * this->_endPosition ) < ( this->_direction * this->_startPosition ) )
	{
		// Overflow in the positive direction, so endPosition < startPosition
		// OR overflow in the negative direction, so endPosition > startPosition (or -endPosition < -startPosition)
		return true;
	}
	else
	{
		// Otherwise, there is no overflow
		return false;
	}
}

int Controller::Correction()
{
	this->_switchingCounter = this->_switchingCounter + 1;
	if(this->_switchingCounter == this->_correctionCondition)
	{
		this->_switchingCounter = 0;
		return this->_error;
	}
	return 0;
}

int Controller::gcd(int x, int y)
{
	// Calculates the greates common divider. Only positive numbers and y must be the smaller number (x >= y).
	if( y == 0 )
	{
		return x;
	}
	return this->gcd(y, x % y);
}

bool Controller::Calibrate()
{
	int oldPosition = this->_encoder->read();
	bool calibrate = false;
	
	this->_inverterStage->ActivateCurrentState();
	
	while( calibrate == false )
	{
		this->ControllerDelay(1);
		
		if(this->_encoder->read() == oldPosition)
		{
			this->_encoder->write(0);
			calibrate = true;
		}
		oldPosition = this->_encoder->read();
	}
	//this->_inverterStage->TurnOff();
	return true;
}

void Controller::Step()
{	
	if(this->_direction == 1)
	{
		this->_inverterStage->ActivateNextState();
	}
	else if(this->_direction == -1)
	{
		this->_inverterStage->ActivatePreviousState();
	}
}

void Controller::ControllerDelay(int secondsDelay)
{
	unsigned int startTime = millis();
	if( secondsDelay < 0 )
	{
		secondsDelay = -secondsDelay;
	}
	unsigned int endTime = startTime + 1000*secondsDelay;
	
	bool delayed = true;
	while(delayed)
	{
		unsigned int currentTime = millis();
		if(endTime < currentTime && currentTime >= endTime && currentTime < startTime)
		{
			delayed = false;
		}
		else if(currentTime >= endTime || currentTime < startTime)
		{
			delayed = false;
		}
	}
}
