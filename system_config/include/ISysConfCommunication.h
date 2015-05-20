/*
 * ISysConfCommunication.h
 */

#ifndef ISYSCONFCOMMUNICATION_H_
#define ISYSCONFCOMMUNICATION_H_

#include "SystemConfig.h"


namespace supplementary {

	struct ChangeAtr;

	class ISysConfCommunication
	{
		public:
			ISysConfCommunication(SystemConfig* sc);
			virtual ~ISysConfCommunication(){}

			virtual void sendChangeAtr(ChangeAtr& ca) = 0;

			virtual void tick() {};

			void onChangeAtrReceived(shared_ptr<ChangeAtr> scca);


			virtual void startCommunication() = 0;
			virtual void stopCommunication() = 0;
		protected:
			SystemConfig* sc;
	};

}  // namespace ISysConfCommunication



#endif /* ISYSCONFCOMMUNICATION_H_ */
