/*
 * ProcessManagerRegistry.cpp
 *
 *  Created on: Feb 13, 2015
 *      Author: Stephan Opfer
 */

#include "process_manager/ExecutableMetaData.h"
#include "process_manager/RobotMetaData.h"
#include <SystemConfig.h>
#include <iostream>
#include <process_manager/RobotExecutableRegistry.h>
#include <ConsoleCommandHelper.h>

namespace supplementary
{

	RobotExecutableRegistry* RobotExecutableRegistry::get()
	{
		static RobotExecutableRegistry instance;
		return &instance;
	}

	RobotExecutableRegistry::RobotExecutableRegistry() : sc(SystemConfig::getInstance())
	{

	}

	RobotExecutableRegistry::~RobotExecutableRegistry()
	{
		for (auto metaData : this->executableList)
		{
			delete metaData;
		}

		for (auto metaData : this->robotList)
		{
			delete metaData;
		}
	}

	const map<string, vector<pair<int, int>>>* const RobotExecutableRegistry::getBundlesMap()
	{
		if(bundlesMap.size() == 0)
		{
			// Read bundles from Processes.conf
			auto bundlesSections = (*this->sc)["ProcessManaging"]->getSections("Processes.Bundles", NULL);
			for (auto bundleName : (*bundlesSections))
			{
				vector<string> processList = (*this->sc)["ProcessManaging"]->getList<string>("Processes.Bundles",
						bundleName.c_str(),
						"processList", NULL);
				vector<string> processParamsList = (*this->sc)["ProcessManaging"]->getList<string>("Processes.Bundles",
						bundleName.c_str(),
						"processParamsList",
						NULL);
				if (processList.size() != processParamsList.size())
				{
					cerr << "RobotExecutableReg: Number of processes does not match the number of parameter sets for the bundle '"
					<< bundleName << "' in the ProcessManaging.conf!" << endl;
					continue;
				}

				for (int i = 0; i < processList.size(); i++)
				{
					this->bundlesMap[bundleName].push_back(
							pair<int, int>(stoi(processList[i]), stoi(processParamsList[i])));
				}
				cout << "RobotExecutableReg: Bundle '" << bundleName << "' has " << this->bundlesMap[bundleName].size()
				<< " processes." << endl;
			}
		}
		return &bundlesMap;
	}

	bool RobotExecutableRegistry::getRobotName(int robotId, string& robotName)
	{
		for (auto robotMetaData : this->robotList)
		{
			if (robotMetaData->id == robotId)
			{
				robotName = robotMetaData->name;
				return true;
			}
		}

		robotName = "";
		return false;
	}

	bool RobotExecutableRegistry::robotExists(int robotId)
	{
		for (auto robotMetaData : this->robotList)
		{
			if (robotMetaData->id == robotId)
			{
				return true;
			}
		}
		return false;
	}

	bool RobotExecutableRegistry::robotExists(string robotName)
	{
		for (auto robotMetaData : this->robotList)
		{
			if (robotMetaData->name == robotName)
			{
				return true;
			}
		}
		return false;
	}

	bool RobotExecutableRegistry::getRobotId(string robotName, int& robotId)
	{
		for (auto robotMetaData : this->robotList)
		{
			if (robotMetaData->name == robotName)
			{
				robotId = robotMetaData->id;
				return true;
			}
		}

		robotId = 0;
		return false;
	}

	void RobotExecutableRegistry::addRobot(string robotName, int robotId)
	{
		this->robotList.push_back(new RobotMetaData(robotName, robotId));
	}

	/**
	 * Adds a robot with its configured id, if it exists. Otherwise it generates a new unique id.
	 * This method allows testing with systems, which are not in the Globals.conf,
	 * i.d., are no official robots.
	 */
	int RobotExecutableRegistry::addRobot(string robotName)
	{
		int robotId;
		try
		{
			robotId = (*sc)["Globals"]->get<int>("Globals.Team", robotName.c_str(), "ID", NULL);
		}
		catch (runtime_error& e)
		{
			robotId = 0;
			bool idExists;

			do
			{
				idExists = false;
				robotId++;
				for (auto entry : this->robotList)
				{
					if (entry->id == robotId)
					{
						idExists = true;
						break;
					}
				}
			} while (idExists);
			cout << "PM Registry: Warning! Adding unknown robot " << robotName << " with ID " << robotId << "!" << endl;
		}

		this->addRobot(robotName, robotId);
		return robotId;
	}

	const vector<RobotMetaData*>& RobotExecutableRegistry::getRobots() const
	{
		return this->robotList;
	}

	bool RobotExecutableRegistry::getExecutableName(int execId, string& execName)
	{
		for (auto execMetaData : this->executableList)
		{
			if (execMetaData->id == execId)
			{
				execName = execMetaData->name;
				return true;
			}
		}

		execName = "";
		return false;
	}

	bool RobotExecutableRegistry::getExecutableIdByExecName(string execName, int& execId)
	{
		for (auto execMetaData : this->executableList)
		{
			if (execMetaData->execName == execName)
			{
				execId = execMetaData->id;
				return true;
			}
		}

		execId = 0;
		return false;
	}

	bool RobotExecutableRegistry::getExecutableId(vector<string>& splittedCmdLine, int& execId)
	{
		for (auto execMetaData : this->executableList)
		{
			if (execMetaData->matchSplittedCmdLine(splittedCmdLine))
			{
				execId = execMetaData->id;
				return true;
			}
		}
		execId = 0;
		return false;
	}

	bool RobotExecutableRegistry::executableExists(int execId)
	{
		for (auto execMetaData : this->executableList)
		{
			if (execMetaData->id == execId)
			{
				return true;
			}
		}

		return false;
	}

	bool RobotExecutableRegistry::executableExists(string execName)
	{
		for (auto execMetaData : this->executableList)
		{
			if (execMetaData->name == execName)
			{
				return true;
			}
		}

		return false;
	}

	/**
	 * This method registers the given executable, if it is listed in the ProcessManaging.conf file.
	 * @param execName
	 * @return -1, if the executable is not registered, due to some error. Otherwise, it returns the registered id.
	 */
	int RobotExecutableRegistry::addExecutable(string execSectionName)
	{
		if (this->executableExists(execSectionName))
		{
			cerr << "RobotExecutableRegistry: The executable '" << execSectionName << "' is already registered!"
					<< endl;
			return -1;
		}

		SystemConfig* sc = SystemConfig::getInstance();
		int execId;
		string processMode;
		string execName;
		string absExecName;
		string rosPackage = "NOT-FOUND"; // optional
		string prefixCmd = "NOT-FOUND"; // optional

		try
		{
			execId = (*sc)["ProcessManaging"]->get<int>("Processes.ProcessDescriptions", execSectionName.c_str(), "id",
														NULL);
			processMode = (*sc)["ProcessManaging"]->get<string>("Processes.ProcessDescriptions",
																execSectionName.c_str(), "mode", NULL);
			execName = (*sc)["ProcessManaging"]->get<string>("Processes.ProcessDescriptions", execSectionName.c_str(),
																"execName", NULL);
			rosPackage = (*sc)["ProcessManaging"]->tryGet<string>("NOT-FOUND", "Processes.ProcessDescriptions",
																	execSectionName.c_str(), "rosPackage", NULL);
			prefixCmd = (*sc)["ProcessManaging"]->tryGet<string>("NOT-FOUND", "Processes.ProcessDescriptions",
																	execSectionName.c_str(), "prefixCmd", NULL);
		}
		catch (runtime_error& e)
		{
			cerr << "PM-Registry: Cannot add executable '" << execSectionName
					<< "', because of faulty values in ProcessManaging.conf!" << endl;
			return -1;
		}

		// create absolute executable name, if possible
		if (rosPackage.compare("NOT-FOUND") != 0 && prefixCmd.compare("roslaunch") != 0)
		{
			string cmd = "catkin_find --first-only --libexec " + rosPackage;
			absExecName = supplementary::ConsoleCommandHelper::exec(cmd.c_str());

			if (absExecName.length() > 1)
			{
				absExecName = absExecName.substr(0, absExecName.length() - 1);
				absExecName = absExecName + "/" + execName;
			}
		}

		ExecutableMetaData* execMetaData = new ExecutableMetaData(execSectionName, execId, processMode, execName,
																	rosPackage, prefixCmd, absExecName);
		auto paramSets = (*sc)["ProcessManaging"]->tryGetNames("NONE", "Processes.ProcessDescriptions",
																execSectionName.c_str(), "paramSets", NULL);
		if (paramSets->size() > 1 || paramSets->at(0) != "NONE")
		{
			for (string paramSetKeyString : (*paramSets))
			{
				try
				{
					int paramSetKey = stoi(paramSetKeyString);
					auto paramSetValues = (*sc)["ProcessManaging"]->getList<string>("Processes.ProcessDescriptions",
																					execSectionName.c_str(),
																					"paramSets",
																					paramSetKeyString.c_str(), NULL);

					// first param is always the executable name
					vector<char*> currentParams;
					if (absExecName.length() > 1)
					{
						currentParams.push_back(strdup(absExecName.c_str()));
					}
					else
					{
						currentParams.push_back(strdup(execName.c_str()));
					}
					// transform the system config params to vector of char*, for c-compatibility.
					cout << currentParams[0] << endl;
					for (string param : paramSetValues)
					{
						char * tmp = new char[param.size() + 1];
						strcpy(tmp, param.c_str());
						tmp[param.size()] = '\0';
						currentParams.push_back(tmp);
					}
					currentParams.push_back(nullptr);

					execMetaData->addParameterSet(paramSetKey, currentParams);
				}
				catch (exception & e)
				{
					cerr << "RobotExecutableRegistry: Unable to parse parameter set \"" << paramSetKeyString
							<< "\" of process \"" << execSectionName << "\"" << endl;
					cerr << e.what() << endl;
				}
			}
		}
		else
		{
			vector<char*> currentParams;
			if (absExecName.length() > 1)
			{
				currentParams.push_back(strdup(absExecName.c_str()));
			}
			else
			{
				currentParams.push_back(strdup(execName.c_str()));
			}
			currentParams.push_back(nullptr);
			execMetaData->addParameterSet(0, currentParams);
		}

		//cout << (*execMetaData) << endl;
		this->executableList.push_back(execMetaData);
		return execId;
	}

	/**
	 * This method is for copying meta data from an entry into a real managed executable.
	 * Don't change anything in the returns object.
	 * @param execName is the name of the demanded entry.
	 * @return The demanded entry, if it exists. nullptr, otherwise.
	 */
	ExecutableMetaData
	const * const RobotExecutableRegistry::getExecutable(string execName) const
	{
		for (auto execEntry : this->executableList)
		{
			if (execEntry->name == execName)
			{
				return execEntry;
			}
		}
		return nullptr;
	}

	/**
	 * This method is for copying meta data from an entry into a real managed executable.
	 * Don't change anything in the returns object.
	 * @param execId is the id of the demanded entry.
	 * @return The demanded entry, if it exists. nullptr, otherwise.
	 */
	ExecutableMetaData
	const * const RobotExecutableRegistry::getExecutable(int execId) const
	{
		for (auto execEntry : this->executableList)
		{
			if (execEntry->id == execId)
			{
				return execEntry;
			}
		}
		return nullptr;
	}

	/**
	 * For accessing the internal data structure of executable meta data entries.
	 * @return The internal data structure of executable meta data entries.
	 */
	const vector<ExecutableMetaData*>& RobotExecutableRegistry::getExecutables() const
	{
		return this->executableList;
	}

	void RobotExecutableRegistry::setInterpreters(vector<string> interpreter)
	{
		this->interpreter = interpreter;
	}

} /* namespace supplementary */

