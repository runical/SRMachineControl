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
	
	static int number = 0;
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

void SwitchState::SetInterval(int start, int stop)
{
	this->_interval = {start, stop};
	if( start > stop )
	{
		this->_exception = true;
	}
	else
	{
		this->_exception = false;
	}
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

int* SwitchState::GetInterval()
{
	return this->_interval;
}

SwitchState* SwitchState::Clone()
{
	// Create a state with the same switches
	SwitchState* clonedState = new SwitchState(this->_activeSwitches, this->_nSwitches);
	return clonedState;
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

SwitchState* InverterStage::GetCurrentState()
{
	return this->_currentState;
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

void InverterStage::Expand(int eRevPerMRev, int nStates)
{
	// Expands the current amount of states to the amount of states given by
	// the number of states times the number of electrical revolutions per
	// mechanical revolution
	
	int i;
	
	for(i = 1 ; i < eRevPerMRev ; i++)
	{
		SwitchState* StartState = this->_startState;
		SwitchState* CurrentState = StartState;
		SwitchState* ClonedState;
		
		int n ;
		for( n = 0 ; n < nStates ; n++ )
		{
			ClonedState = CurrentState->Clone();
			StartState->GetPrevious()->InsertSwitchState(ClonedState);
			CurrentState = CurrentState->GetNext();
		}
	}
}

//////////////////////////////////////////
// 				Controller				//
//////////////////////////////////////////

// The controller implements the logic and setup.

Controller::Controller(SwitchState* topState, Encoder* theEncoder, int pulsesPerRev, int eRevPerMRev, int nStates, int offset, int direction, int switchingdelay)
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
	
	// Calculate error and error correction
	/* 
	 * This is done by finding the error on a complete mechanical rev.
	 * The  number of states the system goes through per mechanical rev is also derived.
	 * Then, we find the greatest common divider (with the gcd algorithm), divide 
	 * both of them by this gcd and save the result. Then repeating this until the gcd
	 * is 1. This gives us the smallest possible integer amount of switching states with 
	 * the smallest error to correct.
	 */
	
	Serial.println("Finding the index pulse in 10 seconds");
	int i;
	
	for( i = 10 ; i > 0 ; i--)
	{
		Serial.println(i);
		this->_inverterStage->ActivateCurrentState();
		delay(1000);
	}
	
	int oldPosition = this->_encoder->read();
	
	do {
		this->Step();
		delay(1000);
		Serial.println(this->_encoder->read());
	} while(((oldPosition > this->_encoder->read()) && (this->_direction == 1)) || ((oldPosition < this->_encoder->read()) && (this->_direction == -1)));

	Serial.print("Found, current Encoder position: ");
	Serial.println(this->_encoder->read());
	
	Serial.println("Starting the calculations");
	
	// Calculates the standard interval size.
	
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
	
	// Expand the number of states
	Serial.println("Expanding");
	this->_inverterStage->Expand(eRevPerMRev, nStates);
	Serial.println("Done expanding");
	
	// Calculate all of the intervals
	Serial.println("Start setting up the intervals");
	
	SwitchState*  StartState;
	if(this->_direction == 1)
	{
		StartState = this->_inverterStage->GetCurrentState()->GetNext();
	}
	else
	{
		StartState = this->_inverterStage->GetCurrentState()->GetPrevious();
	}
	SwitchState* CurrentState = StartState;
	
	int StartPosition = this->_encoder->read() + (this->_direction * offset);
	if(this->_direction == -1)
	{
		if(StartPosition > 0)
		{
			StartPosition = -pulsesPerRev + StartPosition;
		}
		if( StartPosition < -pulsesPerRev)
		{
			StartPosition = StartPosition + pulsesPerRev;
		}
	}
	else
	{
		if(StartPosition < 0)
		{
			StartPosition = pulsesPerRev + StartPosition;
		}
		if( StartPosition > pulsesPerRev)
		{
			StartPosition = StartPosition % pulsesPerRev;
		}
	}
	
	// Loop through all States, starting with the next state
	do {
		int NextPosition = (StartPosition + this->_direction * Change());		
		
		
		Serial.print("State ");
		Serial.print(CurrentState->_statenumber);
		Serial.print(" interval: ");
		Serial.print(StartPosition);
		Serial.print(" - ");
		
		if(this->_direction == -1)
		{
			if(NextPosition > 0)
			{
				NextPosition = -pulsesPerRev + NextPosition;
			}
			if( NextPosition < -pulsesPerRev)
			{
				NextPosition = NextPosition + pulsesPerRev;
			}
			CurrentState->SetInterval(NextPosition, StartPosition);
			CurrentState = CurrentState->GetPrevious();
		}
		else
		{
			if(NextPosition < 0)
			{
				NextPosition = pulsesPerRev + NextPosition;
			}
			if( NextPosition > pulsesPerRev)
			{
				NextPosition = NextPosition % pulsesPerRev;
			}
			CurrentState->SetInterval(StartPosition, NextPosition);
			CurrentState = CurrentState->GetNext();
		}
		Serial.println(NextPosition);
		StartPosition = NextPosition;
	} while( CurrentState != StartState );
	
	this->_interval = StartState->GetInterval();
	
	Serial.println("Starting");
	Serial.print("Encoder position: ");
	Serial.println(this->_encoder->read());
	Serial.print("Current State: ");
	Serial.println(this->_inverterStage->GetCurrentState()->_statenumber);
	Serial.print("Interval: ");
	Serial.print(this->_interval[0]);
	Serial.print(" - ");
	Serial.println(this->_interval[1]);
}

void Controller::PositionLogic()
{
	// Predefined interval algorithm
	int position = this->_encoder->read();
	static int scase = -1;

	/* 
	 * This algorithm works using predefined intervals.
	 * These intervals are calculated and calibrated on the creation of the controller.
	 * 
	 * The algorithm works for both directions because we define the interval to always
	 * have the lowest number in the first place and the highest number in the second place.
	 */
	 
	if(this->_inverterStage->GetCurrentState()->_exception == true)
	{
		if(( this->_interval[0] <= position )|| ( this->_interval[1] >= position ))
		{
			/*if( scase != 1 )
			{
				Serial.println("First Case");
				Serial.print("Encoder: ");
				Serial.println(position);
				Serial.print("Interval: ");
				Serial.print(this->_interval[0]);
				Serial.print(" - ");
				Serial.println(this->_interval[1]);
				Serial.print("Current State: ");
				Serial.println(this->_inverterStage->GetCurrentState()->_statenumber);
				Serial.print("Interval: ");
				Serial.print(this->_inverterStage->GetCurrentState()->GetInterval()[0]);
				Serial.print(" - ");
				Serial.println(this->_inverterStage->GetCurrentState()->GetInterval()[1]);
				scase = 1;
			}*/
			return;
		}
	}
	else if(( this->_interval[0] <= position ) && ( this->_interval[1] >= position ))
	{
		/*if( scase != 2 )
		{
			Serial.println("Second Case");
			Serial.print("Encoder: ");
			Serial.println(position);
			Serial.print("Interval: ");
			Serial.print(this->_interval[0]);
			Serial.print(" - ");
			Serial.println(this->_interval[1]);
			Serial.print("Current State: ");
			Serial.println(this->_inverterStage->GetCurrentState()->_statenumber);
			Serial.print("Interval: ");
			Serial.print(this->_inverterStage->GetCurrentState()->GetInterval()[0]);
			Serial.print(" - ");
			Serial.println(this->_inverterStage->GetCurrentState()->GetInterval()[1]);
			scase = 2;
		}*/
		return;
	}
	
	/*
	Serial.println("Switch!");
	if(this->_inverterStage->GetCurrentState()->_exception) { Serial.println("Exception");}
	*/
	
	this->Step();
	this->_interval = this->_inverterStage->GetCurrentState()->GetInterval();
}

int Controller::Change()
{
	this->_switchingCounter++;
	if(this->_switchingCounter == this->_correctionCondition)
	{
		this->_switchingCounter = 0;
		return (this->_error + this->_increase);
	}
	return this->_increase;
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



