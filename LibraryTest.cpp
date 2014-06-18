#include <stdio.h>
#include "SRM.h"

PhysicalSwitch Switch1(1);
PhysicalSwitch Switch2(2);
PhysicalSwitch Switch3(3);
PhysicalSwitch Switch4(4);

PhysicalSwitch Switches[] = {Switch1, Switch2, Switch3, Switch4};



int main()
{
	Switches[1].Activate();
	Switches[1].Deactivate();
	return 0;
};
