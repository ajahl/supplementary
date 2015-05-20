using namespace std;

#include "SystemConfig.h"
#include "ISysConfCommunication.h"

namespace supplementary
{
	// Initialize static variables
	string SystemConfig::rootPath;
	string SystemConfig::logPath;
	string SystemConfig::configPath;
	string SystemConfig::hostname;
	bool SystemConfig::collectRoot;
	int SystemConfig::posCounter;
	int SystemConfig::rootCounter;
	int SystemConfig::hostCounter;
	mutex SystemConfig::configsMapMutex;
	map<string, shared_ptr<Configuration> > SystemConfig::configs;
	map<string, shared_ptr<Configuration> > SystemConfig::newConfigs;
	map<string, shared_ptr<Configuration> > SystemConfig::hostConfigs;
	map<string, string> SystemConfig::atrPaths;
	map<string, string> SystemConfig::rootPaths;
	shared_ptr<Configuration> SystemConfig::uniConf;
	string uniConfPath;
	vector<string> allPaths;
	vector<string> allConfigs;
	vector<int> depth;
	ISysConfCommunication* SystemConfig::communicator;

	/**
	 * The method for getting the singleton instance.
	 * @return A pointer to the SystemConfig object, you must not delete.
	 */
	SystemConfig* SystemConfig::getInstance()
	{
		static SystemConfig instance;
		return &instance;
	}

	/**
	 * The private constructor of the SystemConfig singleton.
	 */
	SystemConfig::SystemConfig()
	{
		// set the domain folder (1. by env-variable 2. by cwd)
		char *x = ::getenv(DOMAIN_FOLDER.c_str());
		char cwd[4096];
		::getcwd(cwd, 4096);
		cout << "CWD: " << cwd << endl;
		if (x == NULL)
		{
			char cwd[4096];
			if (::getcwd(cwd, 4096) == NULL)
			{
				cerr << "SystemConfig: Error while calling getcwd!" << endl;
			}
			rootPath = cwd;
			cout << "cwd: " << cwd << endl;
		}
		else
		{
			rootPath = x;
		}

		// set the domain config folger (1. by env-variable 2. by <domain folder>/etc
		x = ::getenv(DOMAIN_CONFIG_FOLDER.c_str());

		if (x == NULL)
		{
			configPath = (rootPath + "/etc/");
		}
		else
		{
			string temp = x;
			configPath = temp + "/";
		}

		logPath = FileSystem::combinePaths(rootPath, "/log/temp");
		if (!FileSystem::pathExists(logPath))
		{
			if (!FileSystem::createDirectory(logPath))
			{
				cerr << "SC: Could not create log directory: " << logPath << endl;
			}
		}

		// set the hostname (1. by env-variable 2. by gethostname)
		char* envname = ::getenv("ROBOT");
		if ((envname == NULL) || ((*envname) == 0x0))
		{
			char hn[1024];
			hn[1023] = '\0';
			gethostname(hn, 1023);
			SystemConfig::hostname = hn;
		}
		else
		{
			hostname = envname;
		}

		cout << "SC: Root:          " << rootPath << endl;
		cout << "SC: ConfigRoot:    " << configPath << endl;
		cout << "SC: Hostname:      " << hostname << endl;
		cout << "SC: Loggin Folder: " << logPath << endl;
		collectRoot = true;
		collectConfigs();
		collectRoot = false;
		collectConfigs();
	}

	void SystemConfig::shutdown()
	{
		SystemConfig::communicator->stopCommunication();
	}

	void SystemConfig::setCommunicator(ISysConfCommunication* communicator)
	{
		SystemConfig::communicator = communicator;
		SystemConfig::communicator->startCommunication();
	}

	void SystemConfig::onChangeAtr(shared_ptr<ChangeAtr> scca)
	{
		cout << "SC: OnChangeAtr - Attribute: " << scca->attribute << " Value: " << scca->value << endl;
	}

	/**
	 * The access operator for choosing the configuration according to the given string
	 *
	 * @param s The string which determines the used configuration.
	 * @return The demanded configuration.
	 */

	Configuration* SystemConfig::operator[](const string s)
	{
		{
			lock_guard<mutex> lock(configsMapMutex);

			map<string, shared_ptr<Configuration> >::iterator itr = configs.find(s);

			if (itr != configs.end())
			{
				return itr->second.get();
			}
		}
//
//		vector<string> files;
//
//		string file = s + ".conf";
//
//		// Check the local config
//		files.push_back(file);
//
//		// Check the host-specific config
//		string tempConfigPath = configPath;
//		tempConfigPath = FileSystem::combinePaths(tempConfigPath, hostname);
//		tempConfigPath = FileSystem::combinePaths(tempConfigPath, file);
//		files.push_back(tempConfigPath);
//
//		// Check the global config
//		tempConfigPath = configPath;
//		tempConfigPath = FileSystem::combinePaths(tempConfigPath, file);
//		files.push_back(tempConfigPath);
//
//		for (size_t i = 0; i < files.size(); i++)
//		{
//			if (FileSystem::pathExists(files[i]))
//			{
//				lock_guard<mutex> lock(configsMapMutex);
//
//				//shared_ptr<Configuration> result = shared_ptr<Configuration>(new Configuration(files[i]));
//				shared_ptr<Configuration> result = make_shared<Configuration>(files[i]);
//				configs[s] = result;
//				cout << "IM operator" << endl;
//
//				return result.get();
//			}
//		}
//
//		// config-file not found, print error message
//		cerr << "Configuration file " << file << " not found in either location:" << endl;
//		for (size_t i = 0; i < files.size(); i++)
//		{
//			cerr << "- " << files[i] << endl;
//		}
		return nullptr;
	}

	/**
	 * Looks up the own robot's ID with the system config's local hostname.
	 * @return The own robot's ID
	 */
	int SystemConfig::getOwnRobotID()
	{
		return SystemConfig::getRobotID(SystemConfig::getHostname());
	}

	/**
	 * Looks up the robot's ID with the given name.
	 * @return The robot's ID
	 */
	int SystemConfig::getRobotID(const string& name)
	{
		Configuration *tmp = (*SystemConfig::getInstance())["Globals"];
		int ownRobotID = tmp->get<int>("Globals", "Team", name.c_str(), "ID", NULL);
		return ownRobotID;
	}

	string SystemConfig::getRootPath()
	{
		return rootPath;
	}

	string SystemConfig::getConfigPath()
	{
		return configPath;
	}

	string SystemConfig::getLogPath()
	{
		return logPath;
	}

	string SystemConfig::getHostname()
	{
		return hostname;
	}

	void SystemConfig::setHostname(string newHostname)
	{
		hostname = newHostname;
		configs.clear();
	}

	void SystemConfig::setRootPath(string rootPath)
	{
		this->rootPath = rootPath;
	}

	void SystemConfig::setConfigPath(string configPath)
	{
		this->configPath = configPath;
	}

	void SystemConfig::resetHostname()
	{
		char* envname = ::getenv("ROBOT");
		if ((envname == NULL) || ((*envname) == 0x0))
		{
			char hn[1024];
			hn[1023] = '\0';
			gethostname(hn, 1023);
			SystemConfig::hostname = hn;
		}
		else
		{
			hostname = envname;
		}
		configs.clear();
	}

	string SystemConfig::robotNodeName(const string& nodeName)
	{
		return SystemConfig::getHostname() + NODE_NAME_SEPERATOR + nodeName;
	}

	string SystemConfig::getEnv(const string & var)
	{
		const char * val = ::getenv(var.c_str());
		if (val == 0)
		{
			cerr << "SC: Environment Variable " << var << " is null" << endl;
			return "";
		}
		else
		{
			cout << "SC: Environment Variable " << var << " is " << val << endl;
			return val;
		}
	}

	void SystemConfig::getAllConfigs(string path)
	{
		vector<string> allPaths2;
		DIR *dir;
		string name;
		string path2;
		int type;
		bool entered = false;
		struct dirent *ent;
		size_t countDepth;
		vector<string>::iterator iter_allPaths;
		vector<string>::iterator iter_allPaths2;

		int i;
		if ((dir = opendir(path.c_str())) != NULL)
		{
			while ((ent = readdir(dir)) != NULL)
			{
				name = ent->d_name;
				type = (int)ent->d_type;
				if(name == "." || name == "..")
				{
					continue;
				}
				cout << "IM NAME: " << name << endl;

				if(name != "." && name != ".." && type == 8)
				{
					cout << "DATEI" << endl;
					if((path + "/") != configPath || collectRoot == true)
					{
						cout << endl;
						countDepth = std::count(path.begin(), path.end(), '/');
						cout << "FÜGE EIN: " << name << endl;
//						cout << "OFF: " << countDepth << endl;
						cout << "PATH: " << path << endl;
						entered = true;
						string newPath1, newPath2, newHost;
						for(int i = 0; i< allPaths.size(); i++)
						{
							if(allPaths[i].find(name) == (allPaths[i].size() - name.size()))
							{
								cout << endl;
								if(allPaths[i].size() > path.size())
								{
									newPath1 = path + "/";
									newPath2 = allPaths[i].substr(0, path.size()+1);
								}
								else
								{
									newPath1 = path.substr(0, allPaths[i].size() - name.length());
									newPath2 = allPaths[i].substr(0, allPaths[i].size() - name.length());
								}

								newHost = "/" + hostname;
//								cout << "PATHSIZE: " << path.length() << endl;
//								cout << "PATH: " << path << endl;
								cout << "NEWPATH1 " << newPath1 << endl;
								cout << "NEWPATH2 " << newPath2 << endl;
//								cout << "CONFIGPATH: " << configPath << endl;
//								cout << "CONFIGSIZE: " << configPath.size() << endl;
//								cout << "ALLPATHS2: " << allPaths[i] << endl;
//								cout << "FIND1: " << path.find(newPath1) << endl;
//								cout << "FIND2: " << allPaths[i].find(newPath2) << endl;
								if(path != configPath)
								{
									if(newPath1.compare(newPath2))
									{
										if((newPath1.find(newHost) == configPath.size()-1&& newPath2.find(newHost) == configPath.size()-1)||
										   (newPath1.find(newHost) != configPath.size()-1&& newPath2.find(newHost) != configPath.size()-1))
										{
											cout << "PROBLEM UNGLEICH TROTZ HOST" << endl;
											exit(1);
										}
									}
								}
							}
						}
						if(collectRoot)
						{
							cout << "ROOT" << endl;
							allPaths.insert(allPaths.begin(), path + "/" + name);
//							cout << "COLLECT: allPathsBegin: " << allPaths[0].data() << endl;
//							cout << "ROOTCOUNTER: " << rootCounter <<  endl;
//							cout << "POSCOUNTER: " << posCounter << endl;
							rootCounter++;
							posCounter++;
							hostCounter++;

						}
						else if(path.find(newHost) != configPath.size()-1)
						{
							cout << "NO HOST" << endl;
							iter_allPaths = allPaths.begin();
							iter_allPaths += rootCounter;
							allPaths.insert(iter_allPaths, path + "/" + name);
//							cout << "ROOTCOUNTER: " << rootCounter <<  endl;
//							cout << "POSCOUNTER: " << posCounter << endl;
							posCounter++;
							hostCounter++;

						}
						else
						{
							cout << "HOST" << endl;
							iter_allPaths2 = allPaths.begin();
							iter_allPaths2 += hostCounter;
							allPaths.insert(iter_allPaths2, path + "/" + name);
//							cout << "ROOTCOUNTER: " << rootCounter <<  endl;
//							cout << "POSCOUNTER: " << posCounter << endl;
//							cout << "HOSTCOUNTER: " << hostCounter << endl;
						}
					}
				}
				if(name != "." && name != ".." && type == 4 && collectRoot == false)
				{
					cout << "ORDNER" << endl;
//					cout << "OLD PATH: " << path << endl;
					path2 = path + "/" + ent->d_name;
//					cout << "NEW PATH: " << path2 << endl;
					SystemConfig::getAllConfigs(path2);
				}
			}
			closedir(dir);
		}
	}
	void SystemConfig::collectConfigs()
	{
		string path = getConfigPath();
		path = path.substr(0, (getConfigPath().length()-1));
		SystemConfig::getAllConfigs(path);
		string newRootPath;
		string confName;
		int pathPos;
		int filePos;
//		cout << "AllPaths size: " << allPaths.size() << endl;
		for(int i = 0; i < allPaths.size(); i++)
		{
			cout << endl;
			cout << "ALL FILES: " << allPaths[i] << endl;
			pathPos = allPaths[i].find_last_of("/");
			filePos = allPaths[i].find(".");
			filePos = filePos -pathPos;
			if(pathPos > 0)
			{
				newRootPath = allPaths[i].substr(0, pathPos+1);
				confName = allPaths[i].substr(pathPos+1, filePos-1);
			}
			cout << "newRootPath: " << newRootPath << endl;
			cout << "confName: " << confName << endl;
//			cout << "operator: " << s << endl;

			if (FileSystem::pathExists(allPaths[i]))
			{
				lock_guard<mutex> lock(configsMapMutex);

				//shared_ptr<Configuration> result = shared_ptr<Configuration>(new Configuration(files[i]));
				shared_ptr<Configuration> result = make_shared<Configuration>(allPaths[i]);
//				if(allPaths[i] == uniConfPath){
//					uniConf = result;
//				}
				if(collectRoot)
				{
					if(newRootPath == configPath)
					{
						if(configs[confName] != NULL)
						{
							cout <<"More than one of same configs ind root"<< endl;
						}
						else
						{
							configs[confName] = result;
							cout << "Füge ein in configs: " << confName << endl;
							cout << endl;
						}
					}
				}
				else{
					if(newRootPath != configPath)
					{
						newConfigs[confName] = result;
						cout << "Füge ein in newConfigs: " << confName << endl;
						cout << endl;
					}
					createUniConf(result, allPaths[i]);
				}
			}
		}

		cout << endl;
		map<string, shared_ptr<Configuration>>::iterator iter_config;
		map<string, shared_ptr<Configuration>>::iterator iter_newConfig;
		map<string, string>::iterator iter_atrPaths;

//		cout << "configs size: " << configs.size() << endl;
		for(iter_config = configs.begin() ; iter_config != configs.end(); iter_config++ )
		{
			cout << "ALL CONFIGS: " << iter_config->first << endl;
		}
		for(iter_newConfig = newConfigs.begin() ; iter_newConfig != newConfigs.end(); iter_newConfig ++ )
		{
			cout << "ALL NEWCONFIGS: " << iter_newConfig->first << endl;
		}
//		for(iter_atrPaths = atrPaths.begin() ; iter_atrPaths != atrPaths.end(); iter_atrPaths ++ )
//		{
//			cout << "ALL atrPaths(FIRST): " << iter_atrPaths->first << endl;
//			cout << "ALL atrPaths(SECOND): " << iter_atrPaths->second << endl;
//		}
	}
	void SystemConfig::createUniConf(shared_ptr<Configuration> conf, string fileName){
		vector<ConfigNode*>::iterator iter;
		const char *confName;
		const char *confPath;
		int fileBegin = fileName.find_last_of("/");
		string confFilename = fileName.substr((fileBegin+1), (fileName.length()));
		string confFolder = fileName.substr(0, fileBegin+1);
		confFilename = confFilename.substr(0, confFilename.length()-5);
		confName = confFilename.data();
		cout << "CONFFILENAME: " << confFilename << endl;
		cout << "CONFFOLDER: " << confFolder << endl;

		allConfigs.push_back(confFilename);
		shared_ptr<vector<ConfigNode*> > confNodes = conf->getNodes(confName, NULL);
//		cout << "FILENAME: " << fileName << endl;
		for(iter = confNodes->begin(); iter != confNodes->end(); iter ++ ){
			cout <<"UniCreate: " << iter[0]->getName() << endl;
			confPath = iter[0]->getName().data();
			writeSections(iter[0], "", confFilename, confFolder);
		}
	}
	void SystemConfig::writeSections(ConfigNode* configNode, string section, string fileName, string filePath){
		vector<ConfigNodePtr>::iterator iter;
		string newPath;
		cout << "In Section: " << section << endl;
		if(configNode->getChildren() != NULL){
			for(iter = configNode->getChildren()->begin(); iter != configNode->getChildren()->end(); iter ++ ){
				if(iter[0]->getType() == 0){
					if(section == ""){
						newPath = iter[0]->getName();
					}
					else{
						newPath = section + "." + iter[0]->getName();
					}
//					cout << "NEWPATH: " << newPath << endl;
					writeSections(iter[0].get(), newPath, fileName, filePath);
				}
				if(iter[0]->getType() == 1){
					if(section == ""){
						newPath = iter[0]->getName();
					}
					else{
						newPath = section + "." + iter[0]->getName();
					}
//					cout <<"NAME: " << iter[0]->getName() << endl;
//					cout << "VALUE: " << iter[0]->getValue() << endl;
					newPath = fileName + "." + newPath;
//					cout << "NEWPATH: " << newPath << endl;
//					cout << "FIND: " << filePath.find(atrPaths[newPath]) << endl;
//					cout << "FILEPATH: " << filePath << endl;
//					cout << "ATRPATH: " << atrPaths[newPath] << endl;

					atrPaths[newPath] = filePath;
					writeNewValues(iter[0]->getValue(), newPath, fileName);
				}
				if(iter[0]->getType() == 2){
//					cout << "COMMENT" << endl;
				}
			}
		}
	}
	void SystemConfig::writeNewValues(string value, string path, string config){
//		string newPath = "Globals." + path;
//		cout << uniConf->serialize() << endl;
//		cout << "PATH: " << path.data() << endl;
//		cout << "CONFIG: " << config << endl;

		try{
//			cout << "PATH IN WNV: " << path << endl;
//			cout << "FILEPATH: " << atrPaths[path] << endl;
			configs[config]->get<string>(path.data(), NULL);
		}catch (...) {
			cout << path.data() << " nicht in der Root-Config vorhanden" << endl;
			exit(1);
		}
//		cout << "Overwrite: " << path << endl;
		configs[config]->set(value, path.data(), NULL);
	}
	bool SystemConfig::checkHierarchy()
	{

		return true;
	}
}
