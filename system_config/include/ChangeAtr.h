/*
 * ChangeAtr.h
 */

#ifndef CHANGEATR_H_
#define CHANGEATR_H_

#include <tuple>

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
		stdChangeAtr toStandard()
				{
					return move(make_tuple(attribute, value));
				}
	};
}

#endif /* CHANGEATR_H_ */
