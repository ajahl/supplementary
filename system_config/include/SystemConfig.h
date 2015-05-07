#ifndef SYSTEMCONFIG_H_
#define SYSTEMCONFIG_H_

using namespace std;

#include <map>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>
#include <fstream>
#include <iostream>

#include <FileSystem.h>

#include "Configuration.h"

const string DOMAIN_FOLDER = "DOMAIN_FOLDER";
const string DOMAIN_CONFIG_FOLDER = "DOMAIN_CONFIG_FOLDER";

namespace supplementary
{
	class SystemConfig
	{

	protected:
		static string rootPath;
		static string logPath;
		static string configPath;
		static string hostname;
		static bool collectRoot;
		static int posCounter;
		static int rootCounter;
		static int hostCounter;
		static mutex configsMapMutex;
		static map<string, shared_ptr<Configuration> > configs;
		static map<string, shared_ptr<Configuration> > newConfigs;
		static map<string, shared_ptr<Configuration> > hostConfigs;
		static map<string, string> rootPaths;
		static map<string, string> atrPaths;
		static const char NODE_NAME_SEPERATOR = '_';
		static shared_ptr<Configuration> uniConf;

	public:
		static SystemConfig* getInstance();
		static void shutdown();
		static string robotNodeName(const string& nodeName);
		static int getOwnRobotID();
		static int getRobotID(const string& name);
		static string getHostname();
		static void setHostname(string newHostname);
		static void resetHostname();


		Configuration *operator[](const string s);
		string getRootPath();
		string getConfigPath();
		string getLogPath();
		void setRootPath(string rootPath);
		void setConfigPath(string configPath);
		static string getEnv(const string& var);
		void collectConfigs();
		bool checkHierarchy();



	private:
		SystemConfig();
		~SystemConfig(){};
		void getAllConfigs(string path);
		void createUniConf(shared_ptr<Configuration> conf, string fileName);
		void writeSections(ConfigNode* configNode, string section, string fileName, string filePath);
		void writeNewValues(string value, string path, string config);
	};
}
#endif /* SYSTEMCONFIG_H_ */
