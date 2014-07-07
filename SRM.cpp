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
	this->_switchNumber = switchnumber;
	this->Deactivate();
}

void PhysicalSwitch::Activate()
{	
	// Activate a switch
	if(not this->_state)
	{
		digitalWrite(_pin, HIGH);
		this->_state = true;
		//Serial.print("ON: ");
		//Serial.print(this->_switchNumber);
		//Serial.println(" ");
	}
}

void PhysicalSwitch::Deactivate()
{
	// Deactivate a switch
	if(this->_state)
	{
		digitalWrite(_pin, LOW);
		this->_state = false;
		//Serial.print("ON: ");
		//Serial.print(this->_switchNumber);
		//Serial.println(" ");
	}
}

int PhysicalSwitch::GetSwitchNumber()
{
	return this->_switchNumber;
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

void SwitchState::SetTransition(int transition)
{
	this->_transition = transition;
}

int SwitchState::GetTransition()
{
	return this->_transition;
}

//////////////////////////////////////////
//				Bridge					//
//////////////////////////////////////////

Bridge::Bridge(int numberOfSwitches, PhysicalSwitch** switches)
{
	// Initialize the Bridge. It takes all of the switches associated with the bridge.
	_nSwitches = numberOfSwitches;
	_switches = switches;
}

void Bridge::TurnOff()
{
	// Turn off the switches associated with this bridge.
	int i;
	
	for(i = 0 ; i < _nSwitches ; i++)
	{
		_switches[i]->Deactivate();
	}
}

void Bridge::ActivateState(SwitchState* activatedState)
{
	// Activate the switches associated with the given state
	// Next version might include matching with bridge associated switches
	int limit = activatedState->GetNumberOfSwitches();
	PhysicalSwitch** tempSwitches = activatedState->GetSwitches();
	this->TurnOff();
	
	int i;
	
	for(i = 0 ; i < limit ; i++)
	{
		tempSwitches[i]->Activate();
	}
}

//////////////////////////////////////////
// 				Controller				//
//////////////////////////////////////////

// The controller implements the logic (and most other meta functionality)

Controller::Controller(SwitchState* topState, Bridge* theBridge, Encoder* theEncoder, int pulsesPerRev, int eRevPerMRev, int nStates, int offset)
{
	// Init the needed objects
	this->_startState = topState;
	this->_currentState = topState;
	this->_bridge = theBridge;
	this->_encoder = theEncoder;
	
	// Init variables
	this->_pulsesPerRev = pulsesPerRev;
	this->_eRevPerMRev = eRevPerMRev;
	this->_nStates = nStates;
	this->_offset = offset;
}

void Controller::ActivateNextState()
{
	// Activate the state that is next in line from the current state
	this->_currentState = this->_currentState->GetNext();
	this->_bridge->ActivateState(this->_currentState);
}

void Controller::ActivatePreviousState()
{
	// Activate the state that is the previous in the ring from the current state
	this->_currentState = this->_currentState->GetPrevious();
	this->_bridge->ActivateState(this->_currentState);
}

void Controller::ActivateCurrentState()
{
	this->_bridge->ActivateState(this->_currentState);
}

void Controller::CalculateTransitions(int calibrationOffset)
{
	// Calculation of the new transition point, given by the pulses per revolution, number of states and the difference in electrical and mechanical speeds.
	float transition = (float) ((calibrationOffset + this->_offset + this->_pulsesPerRev) % this->_pulsesPerRev);
	float increment = ((float) this->_pulsesPerRev) / ( ( float ) (this->_nStates * this->_eRevPerMRev) );
	
	SwitchState* theState = this->_startState;
	
	do
	{
		// First get the next state so that the positions are exactly 1 transition out of sync.
		theState = theState->GetNext();
		
		// First add 0.5 before typecasting to make the rounding correct.
		// mod pulsesPerRev to keep within the boundaries of the transitions.
		int newTransition = ( ( int ) (transition + 0.5) ) % this->_pulsesPerRev;
		
		Serial.print(transition);
		Serial.print(" -> ");
		Serial.println(newTransition);
		
		theState->SetTransition(newTransition);
		
		// Then calculate the next transition and select the next state.
		transition = transition + increment;
	} while( theState != this->_startState);
	
}

void Controller::Logic()
{
	// The controller logic.
	// First get the next state for the relevant information.	
	SwitchState* NextState = this->_currentState->GetNext();
	SwitchState* NextNextState = NextState->GetNext();
	
	/* 
	 * In the case that the transition goes through 0, we have 2 problems:
	 * 1. Compare a number that will be smaller wo an higher number, providing to much advancement (until encoder reset)
	 * 2. Compare a huge number to a potentially small number, inhibiting the advancement, thus stalling the motor.
	 * 
	 * So, first we start with comparing the transitions of the current and next state.
	 * This gives us if there will be a zero crossing (if current > next)
	 * We can then solve that by making sure that the encoder is not only bigger than the transition number of the next state 
	 * AND smaller than the transition number of the current state. If this is correct, we advance to the next state.
	 * 
	 * Secondly, we need to catch small intervals. We do this by essentially the same thing. We compare the next two states'
	 * transition number. If the next one is bigger than the one after it, ther is a zero crossing and thus we need extra rules.
	 * The rules are that the encoder position must be bigger than the transition number of the next state OR smaller than the 
	 * transition number of the current state.
	 * 
	 * If neither of these is the case, we can just compare the normal way (next state's transition number is smaller than the position)
	*/
	    
	   
	if( NextState->GetTransition() < _currentState->GetTransition()) // First case
	{
		if( (NextState->GetTransition() <= (this->_encoder->read()) && this->_encoder->read() < this->_currentState->GetTransition()) )
		{
			this->ActivateNextState();
		}
	}
	else if( NextNextState->GetTransition() < NextState->GetTransition() ) // Second case
	{
		if( ( this->_encoder->read() >= NextState->GetTransition() ) || ( this->_encoder->read() <= this->_currentState->GetTransition() ) )
		{
			this->ActivateNextState();
		}
	}
	else if( ( this->_encoder->read() >= NextState->GetTransition() ) ) // The rest
	{
		this->ActivateNextState();
	}
	
	return;
}

void Controller::Setup()
{
	// Finding the index pulse
	static bool FirstTime = true;
	static int oldPosition = 0;
	
	if(FirstTime)
	{
		Serial.println("Turn the encoder to find the index pulse");
		FirstTime = false;
	}
	
	int newPosition = _encoder->read();
	if (newPosition != oldPosition) 
	{
		oldPosition = newPosition;
		Serial.println(oldPosition);
	}
}

void Controller::Startup(int secondsDelay)
{
	Serial.print("Please turn on the power. After a ");
	Serial.print(secondsDelay);
	Serial.println(" second delay the motor will be calibrated and the transitions will be calculated.");
	Serial.println("Please stand back.");
	
	int c;
	
	for(c = 0 ; c < secondsDelay ; c++)
	{
		Serial.println(secondsDelay - c);
		delay(1000);
	}
	
	// Calibrate
	Serial.println("Starting calibration. Please make sure that the power is on.");
	//int calibrationOffset = this->Calibrate();
	this->Calibrate();
	
	// Set up the transitions
	//this->CalculateTransitions(calibrationOffset);
	
	// Let something know
	Serial.print("Motor calibrated. Please stand back. The motor will start in ");
	Serial.print(secondsDelay);
	Serial.println(" seconds.");
	
	for(c = 0 ; c < secondsDelay ; c++)
	{
		Serial.println(secondsDelay - c);
		delay(1000);
	}
	
	Serial.print("Encoder: ");
	Serial.println(this->_encoder->read());
	Serial.println("End of messages.");
	Serial.end();
}

void Controller::Calibrate()
{
	this->ActivateCurrentState();
	
	int transition;
	
	do
	{
		delay(5000);
		transition = (this->_encoder->read() + this->_offset + this->_pulsesPerRev) % this->_pulsesPerRev;
		
		this->ActivateNextState();
		this->_currentState->SetTransition(transition);
		Serial.println(transition);
	} while(this->_currentState != this->_startState);
}

void Controller::Step(int secondsDelay)
{
	int c;
	
	this->ActivateNextState();
	
	for(c = 0 ; c < secondsDelay ; c++)
	{
		delay(1000);
	}
}
