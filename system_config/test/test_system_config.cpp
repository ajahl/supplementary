#include "SystemConfig.h"
#include <iostream>
#include <typeinfo>
#include <gtest/gtest.h>

using namespace supplementary;
// Declare a test
TEST(SystemConfigBasics, readValues)
{
	//	// determine the path to the test config
	//	string path = FileSystem::getSelfPath();
	//
	//	int place = path.rfind("devel");
	//	path = path.substr(0, place);
	//	path = path + "src/supplementary/system_config/test";
	//	// bring up the SystemConfig with the corresponding path
	//	SystemConfig* sc = SystemConfig::getInstance();
	//	sc->setRootPath(path);
	//	sc->setConfigPath(path + "/etc");
	//
	//	// read int
	//	int intTestValue = (*sc)["Test"]->get<int>("intTestValue", NULL);
	//	EXPECT_EQ(221, intTestValue);
	//
	//	cout << "intTestValue: " << intTestValue << endl;
	//
	//	// read double
	//	double doubleTestValue = (*sc)["Test"]->get<double>("doubleTestValue", NULL);
	//	EXPECT_DOUBLE_EQ(0.66234023823, doubleTestValue);
	//
	//	cout << "doubleTestValue: " << doubleTestValue << endl;
	//
	//	// read float
	//	float floatTestValue = (*sc)["Test"]->get<float>("floatTestValue", NULL);
	//	EXPECT_FLOAT_EQ(1.14f, floatTestValue);
	//
	//	cout << "floatTestValue: " << floatTestValue << endl;
}

TEST(SystemConfigBasics, readGlobalValues)
{
	string path = FileSystem::getSelfPath();
		string path2 = FileSystem::getSelfPath();

	//	int place = path.rfind("devel");
	//	path = path.substr(0, place);
	//	path = path + "src/supplementary/system_config/test";
		// bring up the SystemConfig with the corresponding path
		SystemConfig* sc = SystemConfig::getInstance();
	//	sc->setRootPath(path);
	//	sc->setConfigPath(path + "/etc");
		path = sc->getRootPath();
		path2 = sc->getConfigPath();

		Configuration* firstConf = sc->operator []("Globals");
		cout << "RootPath: " << path << endl;

		// read Values
		shared_ptr<vector<string>> confValues = firstConf->getNames("Globals", NULL);
		vector<string>::iterator iter1;
		ConfigNode *testNode;
		shared_ptr<vector<ConfigNode*> > confNodes;
		shared_ptr<vector<ConfigNode*> > confNodes2;
		confNodes = firstConf->getNodes("Globals", NULL);
		confNodes2 = firstConf->getNodes("Globals.FootballField", NULL);
		vector<ConfigNode*>::iterator iter2;
		vector<ConfigNodePtr> *confChildren3;
		vector<ConfigNodePtr>::iterator iter3;
		vector<ConfigNodePtr> *confChildren4;
		vector<ConfigNodePtr>::iterator iter4;
		for(iter2 = confNodes->begin(); iter2 != confNodes->end(); iter2 ++ )
		{
			testNode = iter2[0];
			confChildren3 = testNode->getChildren();
			cout << "NewSections1: " << testNode->getName()<< endl;
			for(iter3 = confChildren3->begin(); iter3 != confChildren3->end(); iter3 ++ )
			{
				if(iter3->get()->getValue().size() !=0)
				{
					cout << "Children: " << iter3->get()->getName()
					 << " - Value: " << iter3->get()->getValue() << endl;
				}
				else if(iter3->get()->getType() != 0)
				{
					cout <<"Comment: #" << iter3->get()->getName() << endl;
				}
				else if(iter3->get()->getType() == 0)
				{
					cout << "Section: " << iter3->get()->getName() << endl;
					confChildren4 = iter3->get()->getChildren();
					for(iter4 = confChildren4->begin(); iter4 != confChildren4->end(); iter4 ++ )
					{
						if(iter4->get()->getValue().size() !=0)
						{
							cout << "Children: " << iter4->get()->getName()
							 << " - Value: " << iter4->get()->getValue() << endl;
						}
						else if(iter4->get()->getType() != 0)
						{
							cout <<"Comment: #" << iter4->get()->getName() << endl;
						}

					}
				}
			}
		}
		for(iter2 = confNodes2->begin(); iter2 != confNodes2->end(); iter2 ++ )
		{
			testNode = iter2[0];
			confChildren3 = testNode->getChildren();
			cout << "NewSections2: " << testNode->getName()<< endl;
			for(iter3 = confChildren3->begin(); iter3 != confChildren3->end(); iter3 ++ )
			{
				if(iter3->get()->getValue().size() !=0)
				{
					cout << "Children: " << iter3->get()->getName()
					 << " - Value: " << iter3->get()->getValue() << endl;
				}
			}
		}
}


TEST(SystemConfigBasics, readTestValues)
{
	string path = FileSystem::getSelfPath();
	string path2 = FileSystem::getSelfPath();

	int place = path.rfind("devel");
	path = path.substr(0, place);
	path = path + "src/supplementary/system_config/test";
	// bring up the SystemConfig with the corresponding path
	SystemConfig* sc = SystemConfig::getInstance();
	sc->setRootPath(path);
	sc->setConfigPath(path + "/etc");
//	path = sc->getRootPath();
//	path2 = sc->getConfigPath();

	Configuration* firstConf = sc->operator []("Test");
	cout << "RootPath: " << path << endl;

	// read Values
//	shared_ptr<vector<string>> confValues = firstConf->getNames("TestSection", NULL);
	vector<string>::iterator iter1;
	ConfigNode *testNode;
	shared_ptr<vector<ConfigNode*> > confNodes;
	shared_ptr<vector<ConfigNode*> > confNodes2;
	confNodes = firstConf->getNodes("Test", NULL);
	cout << confNodes << endl;
//	confNodes2 = firstConf->getNodes("Globals.FootballField", NULL);
	vector<ConfigNode*>::iterator iter2;
	vector<ConfigNodePtr> *confChildren3;
	vector<ConfigNodePtr>::iterator iter3;
	vector<ConfigNodePtr> *confChildren4;
	vector<ConfigNodePtr>::iterator iter4;
	for(iter2 = confNodes->begin(); iter2 != confNodes->end(); iter2 ++ )
	{
		testNode = iter2[0];
		confChildren3 = testNode->getChildren();
		cout << "NewSections1: " << testNode->getName()<< endl;
		for(iter3 = confChildren3->begin(); iter3 != confChildren3->end(); iter3 ++ )
		{
			if(iter3->get()->getValue().size() !=0)
			{
				cout << "Children: " << iter3->get()->getName()
				 << " - Value: " << iter3->get()->getValue() << endl;
			}
			else if(iter3->get()->getType() != 0)
			{
				cout <<"Comment: #" << iter3->get()->getName() << endl;
			}
			else if(iter3->get()->getType() == 0)
			{
				cout << "Section: " << iter3->get()->getName() << endl;
				confChildren4 = iter3->get()->getChildren();
				for(iter4 = confChildren4->begin(); iter4 != confChildren4->end(); iter4 ++ )
				{
					cout << "IN Section" << endl;
					if(iter4->get()->getValue().size() !=0)
					{
						cout << "Children: " << iter4->get()->getName()
						 << " - Value: " << iter4->get()->getValue() << endl;
					}
					else if(iter4->get()->getType() != 0)
					{
						cout <<"Comment: #" << iter4->get()->getName() << endl;
					}

				}
			}
		}
	}
//	for(iter2 = confNodes2->begin(); iter2 != confNodes2->end(); iter2 ++ )
//	{
//		testNode = iter2[0];
//		confChildren3 = testNode->getChildren();
//		cout << "NewSections2: " << testNode->getName()<< endl;
//		for(iter3 = confChildren3->begin(); iter3 != confChildren3->end(); iter3 ++ )
//		{
//			if(iter3->get()->getValue().size() !=0)
//			{
//				cout << "Children: " << iter3->get()->getName()
//				 << " - Value: " << iter3->get()->getValue() << endl;
//			}
//		}
//	}

}

// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

