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

PhysicalSwitch::PhysicalSwitch(int pin)
{
	// PhysicalSwitch sets the pin mode to output, inits it to low and saves the pin number.
	
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	this->_state = false;
	this->_pin = pin;
};

void PhysicalSwitch::Activate()
{	
	// Activate a switch
	if(not this->_state)
	{
		digitalWrite(_pin, HIGH);
		this->_state = true;
	};
};

void PhysicalSwitch::Deactivate()
{
	// Deactivate a switch
	if(this->_state)
	{
		digitalWrite(_pin, LOW);
		this->_state = false;
	};
};

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

Controller::Controller(SwitchState* topState, Bridge* theBridge, Encoder* theEncoder, int nStates, int eRevPerMRev, int pulsesPerRev)
{
	// Init with all of the variables
	this->_startState = topState;
	this->_currentState = topState;
	this->_bridge = theBridge;
	this->_encoder = theEncoder;
	
	this->_transitionposition = {0, 0};
	this->_pulsesPerRev = pulsesPerRev;
    this->_eRevPerMRev = eRevPerMRev;
    this->_nStates = nStates;
    
    // The systems starts paused
    this->_paused = 1;
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

void Controller::CalculateOffset()
{
	// Calculation of the offset (What the hell is this supposed to be?)
	return;
};

void Controller::CalculateTransition()
{
	// Calculation of the new transition point, given by the pulses per revolution, number of states and the difference in electrical and mechanical speeds.
	// First, save the old number to make transitioning past 0 possible.
	this->_transitionPosition[0] = this->_transitionPosition[1];
	
	// Then calculate and make a transition past 0
	this->_transitionPosition[1] = this->_transitionPosition[1] + ((float) this->_pulsesPerRev/(this->_eRevPerMRev*this->_nStates));
	if this->_transitionPosition[1] >= this->pulsesPerRev
		this->_transitionPosition[1] = this->_transitionPosition[1] - this->pulsesPerRev;
	return;
};

void Controller::Logic()
{
	// The controller logic. Needs to be implemented/thought out.
	//int offset = this->CalculateOffset();
	
	// First, define the virtual position, which is encoder position + offset, modulo number of pulses.
	int virtualposition = this->_encoder->read(); // Plus offset?
	int step;
	
	// The decision tree for the transition. Setp = 1 -> transition.
	if this->_transitionPosition[0] < this->_transitionPosition[1]
	{
		if virtualposition >= this->_transitionPosition[1]
		{
			step = 1;
		}
		else
		{
			step = 0;
		}
	}
	else
	{
		if virtualposition >= this->_transitionPosition[1] && virtualposition < this->_transitionPosition[0]
		{
			step = 1;
		}
		else
		{
			step = 0;
		}
	}
	
	// The actual transition
	if(step == 1)
	{
		switch (this->_paused)
		{
			case 1:
				this->currentState = this->_currentState->getNextState();
				break;
			case 0:
				this->ActivateNextState();
				break;
		}
		this->CalculateTransition();
	}
	
	return;
};

void Controller::Pause()
{
	// The pause function, which behaves more like a start/stop.
	if this->_paused == 0
	{
		this->_bridge->TurnOff();
		this->_paused = 1;
	}
	else
	{
		this->_bridge->ActivateState(this->_currentState)
		this->_paused = 0;
	}
}
