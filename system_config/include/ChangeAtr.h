/*
 * ChangeAtr.h
 */

#ifndef CHANGEATR_H_
#define CHANGEATR_H_

using namespace std;

namespace supplementary
{
	typedef tuple<string, string> stdChangeAtr;
	struct ChangeAtr
	{
		ChangeAtr()
		{
		}

		string attribute;
		string value;

		ChangeAtr(stdChangeAtr &s)
		{
			this->attribute = get<0>(s);
			this->value 	= get<1>(s);
		}

//		stdChangeAtr toStandard()
//		{
//			vector<stdEntryPointRobot> r;
//			for (EntryPointRobots& e : entryPointRobots)
//			{
//				r.push_back(move(e.toStandard()));
//			}
//			return move(make_tuple(senderID, planId, parentState, planType, authority, move(r)));
//		}
}

#endif /* CHANGEATR_H_ */
