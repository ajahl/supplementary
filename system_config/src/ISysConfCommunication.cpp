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

using namespace supplementary;

ISysConfCommunication::ISysConfCommunication(SystemConfig* sc)
{
	this->sc = sc;
}

void supplementary::ISysConfCommunication::onChangeAtr(shared_ptr<ChangeAtr> ca)
{
	DIR *dir;
	struct dirent *ent;
	const char* path = '/home/nida_bjk/testRos';
	FileSystem::createDirectory(path);
//	if ((dir = opendir(path)) != NULL)
//	{
//		while ((ent = readdir(dir)) != NULL)
//		{
//			ent->d_name = 'test';
//		}
//	}
}



