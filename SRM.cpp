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
#include <stdio.h>
#include "SRM.h"

//////////////////////////////////////////
// 				PhysicalSwitch			//
//////////////////////////////////////////

PhysicalSwitch::PhysicalSwitch(int switchnumber, int pin)
{
	// PhysicalSwitch sets the pin mode to output, inits it to low and saves the pin number.
	
	//pinMode(pin, OUTPUT);
	//digitalWrite(pin, LOW);
	this->_state = false;
	this->_pin = pin;
	this->_switchNumber = switchnumber;
};

void PhysicalSwitch::Activate()
{	
	// Activate a switch
	if(not this->_state)
	{
		//digitalWrite(_pin, HIGH);
		this->_state = true;
		printf("ON: %i ", this->_switchNumber);
	};
};

void PhysicalSwitch::Deactivate()
{
	// Deactivate a switch
	if(this->_state)
	{
		//digitalWrite(_pin, LOW);
		this->_state = false;
		printf("OFF: %i ", this->_switchNumber);
	};
};

int PhysicalSwitch::getSwitchNumber()
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
};

void SwitchState::InsertSwitchState(SwitchState* insertedState)
{
	// Function for the ring. This inserts a state after this state.
	insertedState->SetPrevious(this);
	insertedState->SetNext(this->_next);
	_next->SetPrevious(insertedState);
	this->SetNext(insertedState);
};

void SwitchState::SetNext(SwitchState* insertedState)
{
	// set the _next variable
	_next = insertedState;
};

void SwitchState::SetPrevious(SwitchState* insertedState)
{
	// set the _previous variable
	_previous = insertedState;
};

SwitchState* SwitchState::GetNext()
{
	// get the _next variable
	return _next;
};

SwitchState* SwitchState::GetPrevious()
{
	// get the _previous variable
	return _previous;
};

int SwitchState::GetNumberOfSwitches()
{
	// get the number of switches (in _nSwitches variable)
	return _nSwitches;
};

PhysicalSwitch** SwitchState::GetSwitches()
{
	// Get the switches associated with the variable
	return _activeSwitches;
};

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
};

void Bridge::TurnOff()
{
	// Turn off the switches associated with this bridge.
	int i;
	
	for(i = 0 ; i < _nSwitches ; i++)
	{
		_switches[i]->Deactivate();
	};
};

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
	};
};

//////////////////////////////////////////
// 				Controller				//
//////////////////////////////////////////

// The controller implements the logic

Controller::Controller(SwitchState* topState, Bridge* theBridge, Encoder* theEncoder)
{
	// Init with all of the variables
	this->_startState = topState;
	this->_currentState = topState;
	this->_bridge = theBridge;
	this->_encoder = theEncoder;
};

void Controller::ActivateNextState()
{
	// Activate the state that is next in line from the current state
	this->_currentState = this->_currentState->GetNext();
	this->_bridge->ActivateState(this->_currentState);
};

void Controller::ActivatePreviousState()
{
	// Activate the state that is the previous in the ring from the current state
	this->_currentState = this->_currentState->GetPrevious();
	this->_bridge->ActivateState(this->_currentState);
};

/*float Controller::CalculateOffset()
{
	// Calculation of the offset (What the hell is this supposed to be?)
	float offset = ((float) this->_pulsesPerRev / (float) analogRead(1));
	return offset;
};*/

void Controller::CalculateTransitions(int pulsesPerRev, int eRevPerMRev, int nStates, int offset)
{
	// Calculation of the new transition point, given by the pulses per revolution, number of states and the difference in electrical and mechanical speeds.
	this->_startState->SetTransition((offset + pulsesPerRev) % pulsesPerRev);
	
	SwitchState* theState = this->_startState->GetNext();
	bool stop = false;
	float increase = (float) pulsesPerRev/( (float) (nStates * eRevPerMRev));
	float transition;
	int newTransition;
	int counter = 0;
	if (offset < 0)
	{
		transition = (float) (pulsesPerRev + offset);
	}
	else
	{
		transition = (float) offset;
	}
	this->_startState->SetTransition( (int) transition );
	printf("%f\n", transition);
	printf("%i\n", (int) (transition + 0.5));
	
	while( true )
	{
		transition = (transition + increase);
		newTransition = ((int) (transition + 0.5)) % pulsesPerRev;
		theState->SetTransition(newTransition);
		printf("%f\n", transition);
		printf("%i\n", newTransition);
		
		theState = theState->GetNext();
		if( theState == this->_startState )
		{
			return;
		}
	}		
	
};

void Controller::Logic()
{
	// The controller logic.
	// First get the next state for the relevant information.	
	SwitchState* NextState = this->_currentState->GetNext();		
	
	// In the case that the transition goes through 0, we check if the encoder did indeed reset. If it did, we can compare and eventually change state.
	if( NextState->GetTransition() < _currentState->GetTransition() )
	{
		if( (NextState->GetTransition() <= (this->_encoder->read()) && this->_encoder->read() < 10000) )
		{
			printf("pos: %i ", this->_encoder->read());
			this->ActivateNextState();
			printf("\n There should be a transition");
			printf("\n");
		}
	}
	else if( (NextState->GetTransition() <= (this->_encoder->read()) ) ) // otherwise, we can just use position >= transition
	{
		printf("pos: %i ", this->_encoder->read());
		this->ActivateNextState();
		printf("\n");
	}
	return;
};

SwitchState* Controller::GetCurrentState()
{
	return this->_currentState;
}

/*float* Controller::GetTransition()
{
	return this->_transitionPosition;
}
*/
