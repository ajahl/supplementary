/*
 * ControlledRobot.h
 *
 *  Created on: Feb 27, 2015
 *      Author: Stephan Opfer
 */

#ifndef SUPPLEMENTARY_PM_CONTROL_SRC_PM_WIDGET_CONTROLLEDROBOT_H_
#define SUPPLEMENTARY_PM_CONTROL_SRC_PM_WIDGET_CONTROLLEDROBOT_H_

#include <process_manager/RobotMetaData.h>
#include <process_manager/ProcessStats.h>
#include <process_manager/ProcessStat.h>
#include <ros/ros.h>
#include <QObject>
#include <QHBoxLayout>
#include <QFrame>
#include <chrono>

namespace Ui {
	class RobotProcessesWidget;
}

namespace supplementary{
	class RobotExecutableRegistry;
}

namespace ros{
	class Publisher;
}

namespace pm_widget
{
	class ControlledExecutable;

	class ControlledRobot : public QObject, public supplementary::RobotMetaData
	{
		Q_OBJECT

	public:
		ControlledRobot(string robotName, int robotId, int parentPMid); /*<for robot_control*/
		virtual ~ControlledRobot();

		void handleProcessStat(chrono::system_clock::time_point timeMsgReceived,process_manager::ProcessStat ps, int parentPMid);
		void sendProcessCommand(vector<int> execIds, vector<int> paramSets, int cmd);
		void updateGUI(chrono::system_clock::time_point now);
		void addExec(QWidget* exec);
		void removeExec(QWidget* exec);

		chrono::system_clock::time_point timeLastMsgReceived; /* < Time point, when the last message have been received */
		QFrame* robotProcessesQFrame; /**< The widget, used to initialise the RobotProcessesWidget */
		//ControlledProcessManager* parentProcessManager;

	public Q_SLOTS:
		void updateBundles(QString text);

	private:
		chrono::duration<double> msgTimeOut;
		bool inRobotControl;
		string selectedBundle;
		Ui::RobotProcessesWidget* _robotProcessesWidget;
		map<int, ControlledExecutable*> controlledExecMap;
		ros::Publisher processCommandPub;
		int parentPMid;
	};

} /* namespace pm_widget */

#endif /* SUPPLEMENTARY_PM_CONTROL_SRC_PM_WIDGET_CONTROLLEDROBOT_H_ */
