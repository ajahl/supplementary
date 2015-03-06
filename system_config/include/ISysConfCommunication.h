/*
 * ISysConfCommunication.h
 */

#ifndef ISYSCONFCOMMUNICATION_H_
#define ISYSCONFCOMMUNICATION_H_

#include "SystemConfig.h"


namespace supplementary {

	class ISysConfCommunication
	{
	public:
		ISysConfCommunication(SystemConfig* sc);
		virtual ~ISysConfCommunication(){}

		virtual void sendValue(ChangeAtr& ca);

		virtual void handleSendValue();

		virtual void tick() {};
		virtual void startCommunication() = 0;
		virtual void stopCommunication() = 0;
	};

}  // namespace ISysConfCommunication



#endif /* ISYSCONFCOMMUNICATION_H_ */
