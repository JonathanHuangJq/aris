﻿#ifndef ROBOT_SERVER_H
#define ROBOT_SERVER_H

#include <string>
#include <sstream>
#include <map>
#include <memory>

#include <aris_core.h>
#include <aris_motion.h>
#include <aris_imu.h>
#include <aris_dyn_model.h>



namespace Aris
{
	const int MAX_MOTOR_NUM = 100;
	
	/*for enable, disable, and home*/
	struct BasicFunctionParam final :Aris::DynKer::PlanParamBase
	{
		bool isMotorActive[MAX_MOTOR_NUM]{ false };
	};

	struct GaitParamBase :DynKer::PlanParamBase
	{
		const Aris::Sensor::ImuData *pImuData;
		const std::vector<Aris::Control::EthercatForceSensor::Data> *pForceData;
	};
	
	
	typedef std::function<Aris::Core::Msg(const std::string &cmd, const std::map<std::string, std::string> &params)> ParseFunc;

	class ControlServer
	{
	public:
		static ControlServer &Instance();

		template<typename T>
		void CreateModel() { this->CreateModel(new T); };
		void CreateModel(DynKer::Model *pModel);

		void LoadXml(const char *fileName);
		void LoadXml(const Aris::Core::XmlDocument &xmlDoc);
		void AddGait(std::string cmdName, Aris::DynKer::PlanFunc gaitFunc, ParseFunc parseFunc);
		void Start();
		void Stop();

	private:
		ControlServer();
		virtual ~ControlServer();
		ControlServer(const ControlServer &) = delete;
		ControlServer &operator=(const ControlServer &) = delete;

		virtual void ParseEnableMsg(const std::string &cmd, const std::map<std::string, std::string> &params, Aris::Core::Msg &msg_out);
		virtual void ParseDisableMsg(const std::string &cmd, const std::map<std::string, std::string> &params, Aris::Core::Msg &msg_out);
		virtual void ParseHomeMsg(const std::string &cmd, const std::map<std::string, std::string> &params, Aris::Core::Msg &msg_out);
	private:
		class Imp;
		std::unique_ptr<Imp> pImp;
	};
}

#endif
