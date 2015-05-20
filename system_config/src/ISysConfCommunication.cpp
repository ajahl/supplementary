/*
 * ISysConfCommunication.cpp
 *
 *  Created on: 02.03.2015
 *      Author: nida_bjk
 */

#include "ISysConfCommunication.h"
#include "ChangeAtr.h"
#include <iostream>
#include "SystemConfig.h"

using namespace std;

namespace supplementary {

ISysConfCommunication::ISysConfCommunication(SystemConfig* sc)
{
	this->sc = sc;
}

void ISysConfCommunication::onChangeAtrReceived(shared_ptr<ChangeAtr> scca)
{
	cout << "ISYSCONF: Attribute: " << scca->attribute << " Value: " << scca->value << endl;
	sc->onChangeAtr(scca);
}

}


