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

#include "arduino.h"
#include <stdio.h>
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
	insertedState->SetPrevious(this);
	insertedState->SetNext(this->_next);
	_next->SetPrevious(insertedState);
	this->SetNext(insertedState);
};

void SwitchState::SetNext(SwitchState* insertedState)
{
	_next = insertedState;
};

void SwitchState::SetPrevious(SwitchState* insertedState)
{
	_previous = insertedState;
};

SwitchState* SwitchState::GetNext()
{
	return _next;
};

SwitchState* SwitchState::GetPrevious()
{
	return _previous;
};

int SwitchState::GetNumberOfSwitches()
{
	return _nSwitches;
};

PhysicalSwitch** SwitchState::GetSwitches()
{
	return _activeSwitches;
};

//////////////////////////////////////////
//				Bridge					//
//////////////////////////////////////////

Bridge::Bridge(int numberOfSwitches, PhysicalSwitch** switches)
{
	_nSwitches = numberOfSwitches;
	_switches = switches;
};

void Bridge::TurnOff()
{
	int i;
	
	for(i = 0 ; i < _nSwitches ; i++)
	{
		_switches[i]->Deactivate();
	};
};

void Bridge::ActivateState(SwitchState* activatedState)
{
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
	// Init with the top state.
	this->_startState = topState;
	this->_currentState = topState;
	this->_bridge = theBridge;
	this->_encoder = theEncoder;
};

void Controller::ActivateNextState()
{
	this->_currentState = this->_currentState->GetNext();
	this->_bridge->ActivateState(this->_currentState);
};

void Controller::ActivatePreviousState()
{
	this->_currentState = this->_currentState->GetPrevious();
	this->_bridge->ActivateState(this->_currentState);
};

void Controller::CalculateOffset()
{
	// Calculation of the offset (rethink how this is integrated with the system)
	return;
};

void Controller::CalculateTransitions(int numberOfElectricRevPerMechRev, int nStates)
{
	// Calculation of the transition points
	return;
};

void Controller::Logic()
{
	// The controller logic. Needs to be implemented/thought out.
	this->ActivatePreviousState();
	return;
};
