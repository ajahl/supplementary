using namespace std;

#include "SystemConfig.h"

namespace supplementary
{
	// Initialize static variables
	string SystemConfig::rootPath;
	string SystemConfig::configPath;
	string SystemConfig::hostname;
	mutex SystemConfig::configsMapMutex;
	map<string, shared_ptr<Configuration> > SystemConfig::configs;
	map<string, shared_ptr<Configuration> > SystemConfig::newConfigs;
	shared_ptr<Configuration> SystemConfig::uniConf;
	string uniConfPath;
	vector<string> allPaths;
	vector<int> depth;

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

		cout << "Root:       " << rootPath << endl;
		cout << "ConfigRoot: " << configPath << endl;
		cout << "Hostname:   " << hostname << endl;

		collectConfigs();
	}

	void SystemConfig::shutdown()
	{

	}

	/**
	 * The access operator for choosing the configuration according to the given string
	 *
	 * @param s The string which determines the used configuration.
	 * @return The demanded configuration.
	 */

	//AFTER SYSTEMCONFIG 2.0 unused
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

		vector<string> files;

		string file = s + ".conf";

		// Check the local config
		files.push_back(file);

		// Check the host-specific config
		string tempConfigPath = configPath;
		tempConfigPath = FileSystem::combinePaths(tempConfigPath, hostname);
		tempConfigPath = FileSystem::combinePaths(tempConfigPath, file);
		files.push_back(tempConfigPath);

		// Check the global config
		tempConfigPath = configPath;
		tempConfigPath = FileSystem::combinePaths(tempConfigPath, file);
		files.push_back(tempConfigPath);

		for (size_t i = 0; i < files.size(); i++)
		{
			if (FileSystem::fileExists(files[i]))
			{
				lock_guard<mutex> lock(configsMapMutex);

				//shared_ptr<Configuration> result = shared_ptr<Configuration>(new Configuration(files[i]));
				shared_ptr<Configuration> result = make_shared<Configuration>(files[i]);
				configs[s] = result;
				cout << "IM operator" << endl;

				return result.get();
			}
		}

		// config-file not found, print error message
		cerr << "Configuration file " << file << " not found in either location:" << endl;
		for (size_t i = 0; i < files.size(); i++)
		{
			cerr << "- " << files[i] << endl;
		}
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

	void SystemConfig::collectConfigs()
	{
		string path = getConfigPath();
		path = path.substr(0, (getConfigPath().length()-1));
		SystemConfig::getAllConfigs(path);
		string s;
		int x;
//		cout << "AllPaths size: " << allPaths.size() << endl;
		for(int i = 0; i < allPaths.size(); i++)
		{
			cout << "ALL FILES: " << allPaths[i] << endl;
			x = allPaths[i].find(".conf");
			if(x > 0)
			{
				s = allPaths[i];
				s = allPaths[i].substr(0, s.find('.'));

			}
//			cout << "operator: " << s << endl;
		}

//		for (size_t i = 0; i < allPaths.size(); i++)
//		{
//			cout << "Füge ein in configs: " << allPaths[i] << endl;
//			shared_ptr<Configuration> result = make_shared<Configuration>(allPaths[i]);
//			configs[allPaths[i]] = result;
//		}

		for (size_t i = 0; i < allPaths.size(); i++)
		{
			cout << "Füge ein in configs: " << allPaths[i] << endl;
			if (FileSystem::fileExists(allPaths[i]))
			{
				lock_guard<mutex> lock(configsMapMutex);

				//shared_ptr<Configuration> result = shared_ptr<Configuration>(new Configuration(files[i]));
				shared_ptr<Configuration> result = make_shared<Configuration>(allPaths[i]);
				configs[allPaths[i]] = result;
				if(allPaths[i] == uniConfPath){
					uniConf = result;
				}
			}
		}


		map<string, shared_ptr<Configuration>>::iterator iter;
//		cout << "configs size: " << configs.size() << endl;
		for(iter = configs.begin() ; iter != configs.end(); iter ++ )
		{
			cout << "ALL CONFIGS: " << iter->first << endl;
//			cout << iter->second.get()->serialize() << endl;
//			cout << iter->second.get()->get<float>("Globals.Dimensions.DiameterBall", NULL) << endl;
//			cout << "Config-Filename: " << configs[iter->first]->get<float>("Globals", NULL) << endl;
			if(iter->first != uniConfPath){
				createUniConf(iter->second, iter->first);
			}
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
		int i;
		if ((dir = opendir(path.c_str())) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				name = ent->d_name;
				type = (int)ent->d_type;
				if(name != "." && name != ".." && type == 8)
				{
					if(entered) {
						cerr << "More than one conf-File in the same folder: " << path << endl;
						exit(1);
					}
					countDepth = std::count(path.begin(), path.end(), '/');
					cout << "FÜGE EIN: " << name << endl;
//					lDepth = ent->d_ino;
					cout << "OFF: " << countDepth << endl;
					if(find(depth.begin(), depth.end(), countDepth) != depth.end()){
						cerr << "More than one conf-File in the same folder-hierarchy: " << path << endl;
						exit(1);
					}
					entered = true;
					allPaths.push_back(path + "/" + name);
					depth.push_back(countDepth);
					if(name == "Globals.conf"){
						uniConfPath = path + "/" + name;
					}
				}
				if(name != "." && name != ".." && type == 4)
				{
//					cout << "OLD PATH: " << path << endl;
					path2 = path + "/" + ent->d_name;
//					cout << "NEW PATH: " << path2 << endl;
					SystemConfig::getAllConfigs(path2);
				}
			}
			closedir(dir);
		}
	}
	void SystemConfig::createUniConf(shared_ptr<Configuration> conf, string fileName){
		vector<ConfigNode*>::iterator iter;
		const char *confName;
		const char *confPath;
		int fileBegin = fileName.find_last_of("/");
		string confFilename = fileName.substr((fileBegin+1), (fileName.length()));
		confFilename = confFilename.substr(0, confFilename.length()-5);
		confName = confFilename.data();
		cout << "CONFFILENAME: " << confFilename << endl;
		shared_ptr<vector<ConfigNode*> > confNodes = conf->getNodes(confName, NULL);
//		cout << "FILENAME: " << fileName << endl;
		for(iter = confNodes->begin(); iter != confNodes->end(); iter ++ ){
			cout <<"UniCreate: " << iter[0]->getName() << endl;
			confPath = iter[0]->getName().data();
//			cout << "CREATEUNI: " << conf->getSections(confPath, NULL)->back().data() << endl;
			writeSections(iter[0], "");
//			cout << "Children: " << iter[0]->getChildren()->back()->getName() << endl;
//			cout << "Value: " << iter[0]->getChildren()->back()->getValue() << endl;
//			cout << uniConf->serialize() << endl;
//			if(uniConf->get<float>("Globals.TestSectionValue2", NULL) !=NULL){
//				cout <<"GEFUNDEN: " << uniConf->get<float>("Globals.TestSectionValue2", NULL) << endl;
//				uniConf->set(iter[0]->getChildren()->back()->getValue(), "Globals.TestSectionValue2", NULL);
//				cout <<"Neu setzen: " << uniConf->get<float>("Globals.TestSectionValue2", NULL) << endl;
//			}
//			cout << "TEST" << endl;
//			cout << "Name: " << iter[0]->getName() << endl;
//			cout <<"Neu gesetzt bei uniConf: " << uniConf->get<float>("Globals.TestSectionValue2", NULL) << endl;

		}
		cout << uniConf->serialize() << endl;
	}
	void SystemConfig::writeSections(ConfigNode* configNode, string section){
		vector<ConfigNodePtr>::iterator iter;
		vector<ConfigNodePtr>::iterator iter2;
		string newPath;
		cout << "Nochmal in WS: " << section << endl;
		if(configNode->getChildren() != NULL){
			for(iter = configNode->getChildren()->begin(); iter != configNode->getChildren()->end(); iter ++ ){
				if(iter[0]->getType() == 0){
					if(section == ""){
						newPath = iter[0]->getName();
					}
					else{
						newPath = section + "." + iter[0]->getName();
					}
					cout << "NEWPATH: " << newPath << endl;
					writeSections(iter[0].get(), newPath);
				}
				if(iter[0]->getType() == 1){
					if(section == ""){
						newPath = iter[0]->getName();
					}
					else{
						newPath = section + "." + iter[0]->getName();
					}
					cout <<"NAME: " << iter[0]->getName() << endl;
					cout << "VALUE: " << iter[0]->getValue() << endl;
					writeNewValues(iter[0]->getValue(), newPath);
				}
				if(iter[0]->getType() == 2){
//					cout << "COMMENT" << endl;
				}
			}
		}
	}
	void SystemConfig::writeNewValues(string value, string path){
		string newPath = "Globals." + path;
//		cout << uniConf->serialize() << endl;
		try{
			uniConf->get<string>(newPath.data(), NULL);
		}catch (...) {
			cout << newPath.data() << " nicht in 'Globals' vorhanden" << endl;
			exit(1);
		}
		cout << "Overwrite: " << newPath << endl;
		uniConf->set(value, newPath.data(), NULL);
	}
}
