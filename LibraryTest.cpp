#include <stdio.h>
#include "SRM.h"

PhysicalSwitch* Switch1 = new PhysicalSwitch(1);
PhysicalSwitch* Switch2 = new PhysicalSwitch(2);
PhysicalSwitch* Switch3 = new PhysicalSwitch(3);
PhysicalSwitch* Switch4 = new PhysicalSwitch(4);

PhysicalSwitch* State1Switches[] = {Switch1, Switch2};
PhysicalSwitch* State2Switches[] = {Switch3, Switch4};
PhysicalSwitch* State3Switches[] = {Switch1, Switch3};
PhysicalSwitch* AllSwitches[] = {Switch1, Switch2, Switch3, Switch4};

SwitchState* SwitchState1 = new SwitchState(State1Switches, 2);
SwitchState* SwitchState2 = new SwitchState(State2Switches, 2);
SwitchState* SwitchState3 = new SwitchState(State3Switches, 2);

Bridge* Bridge1 = new Bridge(4, AllSwitches);

Encoder* Encoder1 = new Encoder(0);

Controller* Controller1 = new Controller(SwitchState1, Bridge1, Encoder1);

int main()
{
	SwitchState1->InsertSwitchState(SwitchState2);
	SwitchState1->GetNext()->InsertSwitchState(SwitchState3);
	
	printf("\n");
	
	int i;
	for(i = 0 ; i < 4 ; i++)
	{
		printf("\n");
		Controller1->Logic();
		printf("Logic\n");
	};
	
	return 0;
};

