/*
 * SysConfRosCommunication.cpp
 *
 *  Created on: 05.03.2015
 *      Author: nida_bjk
 */

#include "SysConfRosCommunication.h"

#include <ros/node_handle.h>
#include <ros/publisher.h>
#include <ros/subscriber.h>
#include <SystemConfig.h>
#include <Configuration.h>

#include "ChangeAtr.h"

using namespace supplementary;

namespace sysConfRosProxy
{

	SysConfRosCommunication::SysConfRosCommunication(SystemConfig* sc) :
			ISysConfCommunication(sc) //, rosNode()
	{
		this->isRunning = false;
		rosNode = new ros::NodeHandle();
		spinner = new ros::AsyncSpinner(4);

		ChangeAtrPublisher = rosNode->advertise<sys_conf_ros_proxy::ChangeAtr>(
				"/SystemConfig/ChangeAtr", 2);
		ChangeAtrSubscriber = rosNode->subscribe("/SystemConfig/ChangeAtr", 10,
																		&SysConfRosCommunication::handleChangeAtrRos,
																		(SysConfRosCommunication*)this);

	}
	SysConfRosCommunication::~SysConfRosCommunication()
	{
		if (this->isRunning)
		{
			spinner->stop();
		}
		delete spinner;

		ChangeAtrPublisher.shutdown();
		rosNode->shutdown();
		delete rosNode;

	}
	void SysConfRosCommunication::tick()
	{
		if (this->isRunning)
		{
			//Use this for synchronous communication!
			//ros::spinOnce();
		}
	}
	void SysConfRosCommunication::sendChangeAtr(ChangeAtr& ca)
	{
		sys_conf_ros_proxy::ChangeAtr scca;
		scca.attribute = ca.attribute;
		scca.value 	   = ca.value;
		if (this->isRunning)
		{
			this->ChangeAtrPublisher.publish(scca);
		}
	}
	void SysConfRosCommunication::handleChangeAtrRos(sys_conf_ros_proxy::ChangeAtrPtr scca)
	{
		auto sccaPtr 		= make_shared<ChangeAtr>();
		sccaPtr->attribute  = scca->attribute;
		sccaPtr->value      = scca->value;
		if (this->isRunning)
		{
			this->onChangeAtrReceived(sccaPtr);
		}
	}
	void SysConfRosCommunication::startCommunication()
	{
		this->isRunning = true;
		spinner->start();
	}
	void SysConfRosCommunication::stopCommunication()
	{
		this->isRunning = false;
		spinner->stop();
	}
}
