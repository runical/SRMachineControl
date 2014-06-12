/*
  SRM.h - Library for controlling a SR machine with an Arduino Due.
  Requires the Encoder Library
 
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

#ifndef SRM_h
#define SRM_h

#include "Arduino.h"

class Bridge
{
  /* Bridge:
   * The abstract layer of the (a)symmetric bridge circuit used to drive the SRM.
   * It contains the switches and all of the possible states of the drive.
   * 
   * The Bridge changes the state and resets all of the switches when needed.
   */

  public:
    Bridge(int numberOfSwitches, Switch* switches);
    void ActivateState(SwitchState activatedState);
    void TurnOff();
  private:
    int _nSwitches;
    Switch* _switches;
};

class Switch
{
   /* Switch:
    * The abstraction of a switch in the bridge circuit.
    * Can be activated, deactivated and the associated pin can be extracted.
    */

   public:
     Switch(int pin);
     void Activate();
     void Deactivate();
   private:
     int _pin;
     boolean _state;
};

class SwitchState
{
   /* SwitchState:
    * The abstract notion of state for the switches/bridge.
    * Keeps track of which switches should be activated to obtain the state.
    *
    * This is organized in a double looped ring construct which carries sequence information.
    */

   public:
     SwitchState(Switch** activeSwitches, int nSwitches);
     SwitchState* GetNext();
     SwitchState* GetPrevious();
     void InsertSwitchState(SwitchState* insertedState);
     void SetNext(SwitchState* inserterdState);
     void SetPrevious(SwitchState* insertedState);
     int GetNumberOfSwitches();
     Switch* GetSwitch(int number);
   private:
     Switch** _activeSwitches;
     int _nSwitches;
     SwitchState* _next;
     SwitchState* _previous;
};

class Controller
{
   /* Controller:
    * Contains the switching logic and all of the components.
    */

   public:
     Controller(int analogPin);
     void Logic();
     void AddState(SwitchState State);
     void CalculateOffset();
     void UpdateTransition();
   private:
     int _analogPin;
     int _transitionPosition;
     Encoder _encoder;
     Bridge _bridge;
     SwitchState _startState;
     SwitchState _currentState;
};
