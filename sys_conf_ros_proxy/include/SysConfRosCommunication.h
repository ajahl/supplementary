/*
 * SysConfRosCommunication.h
 *
 *  Created on: 13.03.2015
 *      Author: nida_bjk
 */

#ifndef SYSCONFROSCOMMUNICATION_H_
#define SYSCONFROSCOMMUNICATION_H_


#include "ISysConfCommunication.h"
#include "ros/ros.h"

#include "sys_conf_ros_proxy/ChangeAtr.h"

using namespace supplementary;

namespace sysConfRosProxy
{
	class SysConfRosCommunication: public supplementary::ISysConfCommunication
	{
		public:
			SysConfRosCommunication(SystemConfig* sc);
			virtual ~SysConfRosCommunication();

			virtual void sendChangeAtr(ChangeAtr& ca);
			virtual void handleChangeAtrRos(sys_conf_ros_proxy::ChangeAtrPtr scca);

			virtual void tick();

			virtual void startCommunication();
			virtual void stopCommunication();
		protected:
			ros::NodeHandle* rosNode;
			ros::AsyncSpinner* spinner;

			ros::Publisher ChangeAtrPublisher;
			ros::Subscriber ChangeAtrSubscriber;

			bool isRunning;
	};
}


#endif /* SYSCONFROSCOMMUNICATION_H_ */
