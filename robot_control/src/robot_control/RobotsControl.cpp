#include <pluginlib/class_list_macros.h>
#include <ros/master.h>

#include <robot_control/RobotsControl.h>

#include <SystemConfig.h>
#include <process_manager/RobotExecutableRegistry.h>
#include <QMenu>

namespace robot_control
{

	chrono::duration<double> RobotsControl::msgTimeOut = chrono::duration<double>(0);

	RobotsControl::RobotsControl() :
			rqt_gui_cpp::Plugin(), widget_(0), guiUpdateTimer(nullptr)
	{
		setObjectName("RobotsControl");
		rosNode = new ros::NodeHandle();

		this->sc = supplementary::SystemConfig::getInstance();
		RobotsControl::msgTimeOut = chrono::duration<double>(
				(*this->sc)["ProcessManaging"]->get<unsigned long>("PMControl.timeLastMsgReceivedTimeOut", NULL));
		this->pmRegistry = supplementary::RobotExecutableRegistry::get();

		/* Initialise the registry data structure for better performance
		 * with data from Globals.conf and Processes.conf file. */

		// Register robots from Globals.conf
		auto robotNames = (*this->sc)["Globals"]->getSections("Globals.Team", NULL);
		for (auto robotName : (*robotNames))
		{
			this->pmRegistry->addRobot(robotName);
		}

		// Register executables from ProcessManaging.conf
		auto processDescriptions = (*this->sc)["ProcessManaging"]->getSections("Processes.ProcessDescriptions", NULL);
		for (auto processSectionName : (*processDescriptions))
		{
			this->pmRegistry->addExecutable(processSectionName);
		}

		// Read bundles from Processes.conf
		auto bundlesSections = (*this->sc)["ProcessManaging"]->getSections("Processes.Bundles", NULL);
		for (auto bundleName : (*bundlesSections))
		{
			vector<string> processList = (*this->sc)["ProcessManaging"]->getList<string>("Processes.Bundles",
																							bundleName.c_str(),
																							"processList",
																							NULL);
			vector<string> processParamsList = (*this->sc)["ProcessManaging"]->getList<string>("Processes.Bundles",
																								bundleName.c_str(),
																								"processParamsList",
																								NULL);
			if (processList.size() != processParamsList.size())
			{
				cerr << "PMControl: Number of processes does not match the number of parameter sets for the bundle '"
						<< bundleName << "' in the Processes.conf!" << endl;
				continue;
			}

			for (int i = 0; i < processList.size(); i++)
			{
				this->bundlesMap[bundleName].push_back(
						pair<int, int>(stoi(processList[i]), stoi(processParamsList[i])));
			}
			cout << "PMControl: Bundle '" << bundleName << "' has " << this->bundlesMap[bundleName].size()
					<< " processes." << endl;
		}
	}

	void RobotsControl::initPlugin(qt_gui_cpp::PluginContext& context)
	{
		widget_ = new QWidget();
		widget_->setAttribute(Qt::WA_AlwaysShowToolTips, true);
		robotControlWidget_.setupUi(widget_);

		this->widget_->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		QObject::connect(this->widget_, SIGNAL(customContextMenuRequested(const QPoint&)), this,
							SLOT(showContextMenu(const QPoint&)));

		if (context.serialNumber() > 1)
		{
			widget_->setWindowTitle(widget_->windowTitle() + " (" + QString::number(context.serialNumber()) + ")");
		}
		context.addWidget(widget_);

		for (auto robot : this->pmRegistry->getRobots())
		{
			this->checkAndInit(robot->id);
		}

		// Initialise the ROS Communication
		processStateSub = rosNode->subscribe("/process_manager/ProcessStats", 10, &RobotsControl::receiveProcessStats,
												(RobotsControl*)this);
		alicaInfoSub = rosNode->subscribe("/AlicaEngine/AlicaEngineInfo", 10, &RobotsControl::receiveAlicaInfo,
											(RobotsControl*)this);
		//kickerStatInfoSub = rosNode->subscribe("/KickerStatInfo", 10, &RobotsControl::receiveKickerStatInfo,
		//										(RobotsControl*)this);

		// Initialise the GUI refresh timer
		this->guiUpdateTimer = new QTimer();
		QObject::connect(guiUpdateTimer, SIGNAL(timeout()), this, SLOT(run()));
		this->guiUpdateTimer->start(200);
	}

	void RobotsControl::showContextMenu(const QPoint& pos)
	{
		/* HINT: remember, if there are some problems that way:
		 * For QAbstractScrollArea and derived classes you would use:
		 * QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); */

		QPoint globalPos = this->widget_->mapToGlobal(pos);

		QMenu myMenu;
		for (auto robot : this->pmRegistry->getRobots())
		{
			myMenu.addAction(std::string(robot->name + " (" + std::to_string(robot->id) + ")").c_str());
		}

		QAction* selectedItem = myMenu.exec(globalPos);
		if (selectedItem)
		{
			int robotId;

			std::string name = selectedItem->iconText().toStdString().substr();
			name = name.substr(0, name.find('(') - 1);

			cout << "RC: '" << name << "'" << endl;

			if (this->pmRegistry->getRobotId(name, robotId))
			{
				//this->checkAndInit(robotId);
				this->controlledRobotsMap[robotId]->toggle();
			}
			else
			{
				cerr << "RC: Chosen robot is not known in the robot registry!" << endl;
			}
		}
		else
		{
			cout << "RC: Nothing chosen!" << endl;
		}
	}

	/**
	 * The worker method of RobotsControl. It processes the received ROS messages and afterwards updates the GUI.
	 */
	void RobotsControl::run()
	{
		processMessages();

		updateGUI();
	}

	/**
	 * Updates the GUI, after ROS process stat message have been processed.
	 */
	void RobotsControl::updateGUI()
	{
		chrono::system_clock::time_point now = chrono::system_clock::now();
		for (auto controlledRobotEntry : this->controlledRobotsMap)
		{
			controlledRobotEntry.second->updateGUI(now);
		}
	}

	void RobotsControl::receiveProcessStats(process_manager::ProcessStatsConstPtr processStats)
	{
		lock_guard<mutex> lck(processStatsMsgQueueMutex);
		this->processStatMsgQueue.emplace(chrono::system_clock::now(), processStats);
	}

	void RobotsControl::receiveAlicaInfo(alica_ros_proxy::AlicaEngineInfoConstPtr alicaInfo)
	{
		lock_guard<mutex> lck(alicaInfoMsgQueueMutex);
		this->alicaInfoMsgQueue.emplace(chrono::system_clock::now(), alicaInfo);
	}

	/*void RobotsControl::receiveKickerStatInfo(msl_actuator_msgs::KickerStatInfoPtr kickerStatInfo)
	{
		lock_guard<mutex> lck(kickerStatInfoMsgQueueMutex);
		this->kickerStatInfoMsgQueue.emplace(chrono::system_clock::now(), kickerStatInfo);
	}*/

	/**
	 * Processes all queued messages from the processStatMsgsQueue and the alicaInfoMsgQueue.
	 */
	void RobotsControl::processMessages()
	{
		{
			lock_guard<mutex> lck(processStatsMsgQueueMutex);
			while (!this->processStatMsgQueue.empty())
			{
				// unqueue the ROS process stat message
				auto timePstsPair = processStatMsgQueue.front();
				processStatMsgQueue.pop();

				for (auto processStat : (timePstsPair.second->processStats))
				{
//					cout << "RobotsControl: senderId: " << timePstsPair.second->senderId << " robotId: " << processStat.robotId << endl;
					this->controlledRobotsMap[processStat.robotId]->handleProcessStat(timePstsPair.first, processStat,
																						timePstsPair.second->senderId);
				}
			}
		}

		{
			lock_guard<mutex> lck(alicaInfoMsgQueueMutex);
			while (!this->alicaInfoMsgQueue.empty())
			{
				// unqueue the ROS alica info message
				auto timeAlicaInfoPair = alicaInfoMsgQueue.front();
				alicaInfoMsgQueue.pop();

				this->controlledRobotsMap[timeAlicaInfoPair.second->senderID]->handleAlicaInfo(timeAlicaInfoPair);
			}
		}

		/*{
			lock_guard<mutex> lck(kickerStatInfoMsgQueueMutex);
			while (!this->kickerStatInfoMsgQueue.empty())
			{
				// unqueue the ROS alica info message
				auto timeKickerStatInfoPair = kickerStatInfoMsgQueue.front();
				kickerStatInfoMsgQueue.pop();

				this->controlledRobotsMap[timeKickerStatInfoPair.second->senderID]->handleKickerStatInfo(timeKickerStatInfoPair);
			}
		}*/

	}

	/**
	 * If the given robot ID is already known, nothing is done.
	 * Otherwise a new entry in the controlled robot map is created.
	 */
	void RobotsControl::checkAndInit(int robotId)
	{
		auto pmEntry = this->controlledRobotsMap.find(robotId);
		if (pmEntry == this->controlledRobotsMap.end())
		{ // robot is not known, so create a corresponding instance
			string robotName;
			if (this->pmRegistry->getRobotName(robotId, robotName))
			{
				cout << "RC: Create new ControlledRobot with ID " << robotId << " and host name " << robotName << "!"
						<< endl;
				Robot* controlledRobot = new Robot(robotName, robotId, this);
				this->controlledRobotsMap.emplace(robotId, controlledRobot);
			}
			else
			{
				cerr << "RC: Received message from unknown robot with sender id " << robotId << endl;
			}
		}
	}

	void RobotsControl::shutdownPlugin()
	{
		this->processStateSub.shutdown();
		this->alicaInfoSub.shutdown();
	}

	void RobotsControl::saveSettings(qt_gui_cpp::Settings& plugin_settings,
										qt_gui_cpp::Settings& instance_settings) const
	{

	}

	void RobotsControl::restoreSettings(const qt_gui_cpp::Settings& plugin_settings,
										const qt_gui_cpp::Settings& instance_settings)
	{

	}

}

PLUGINLIB_EXPORT_CLASS(robot_control::RobotsControl, rqt_gui_cpp::Plugin)
