﻿#include <iostream>
#include <aris.h>

#ifdef UNIX
#include "rtdk.h"
#include "unistd.h"
#endif

#define Motion_Num 18

auto basicParse(const std::string &cmd, const std::map<std::string, std::string> &params, aris::core::Msg &msg_out)->void
{
    aris::server::BasicFunctionParam param;

    for (auto &i : params)
    {
        if (i.first == "all")
        {
            std::fill_n(param.active_motor, 18, true);
        }
        else if (i.first == "first")
        {
            std::fill_n(param.active_motor, 18, false);
            std::fill_n(param.active_motor + 0, 3, true);
            std::fill_n(param.active_motor + 6, 3, true);
            std::fill_n(param.active_motor + 12, 3, true);
        }
        else if (i.first == "second")
        {
            std::fill_n(param.active_motor, 18, false);
            std::fill_n(param.active_motor + 3, 3, true);
            std::fill_n(param.active_motor + 9, 3, true);
            std::fill_n(param.active_motor + 15, 3, true);
        }
        else if (i.first == "motor")
        {
            int id = { stoi(i.second) };
            if (id<0 || id>17)throw std::runtime_error("invalid param in basicParse func");

            std::fill_n(param.active_motor, 18, false);
            param.active_motor[id] = true;
        }
        else if (i.first == "physical_motor")
        {
            int id = { stoi(i.second) };
            if (id<0 || id>17)throw std::runtime_error("invalid param in basicParse func");

            std::fill_n(param.active_motor, 18, false);
            param.active_motor[id] = true;
        }
        else if (i.first == "leg")
        {
            auto leg_id = std::stoi(i.second);
            if (leg_id<0 || leg_id>5)throw std::runtime_error("invalid param in parseRecover func");

            std::fill_n(param.active_motor, 18, false);
            std::fill_n(param.active_motor + leg_id * 3, 3, true);
        }
    }

    msg_out.copyStruct(param);
}

auto testParse(const std::string &cmd, const std::map<std::string, std::string> &params, aris::core::Msg &msg)->void
{
    aris::server::GaitParamBase param;

    for (auto &i : params)
    {
        if (i.first == "all")
        {
            std::fill_n(param.active_motor, 18, true);
        }
        else if (i.first == "motor")
        {
            int id = { stoi(i.second) };
            if (id<0 || id>17)throw std::runtime_error("invalid param in basicParse func");

            std::fill_n(param.active_motor, 18, false);
            param.active_motor[id] = true;
        }
        else if (i.first == "physical_motor")
        {
            int id = { stoi(i.second) };
            if (id<0 || id>17)throw std::runtime_error("invalid param in basicParse func");

            std::fill_n(param.active_motor, 18, false);
            param.active_motor[id] = true;
        }
    }
    msg.copyStruct(param);
}
auto testGait(aris::dynamic::Model &model, const aris::dynamic::PlanParamBase &param_in)->int
{
    auto &param = static_cast<const aris::server::GaitParamBase&>(param_in);

    static double begin_pin[Motion_Num];
    if(param.count==0)
     {
        for(int i=0;i<Motion_Num;i++)
            begin_pin[i] = static_cast<aris::control::RxMotionData&>(param.controller->slavePool().at(i).rxData()).feedback_pos;
    }

    double pin[Motion_Num];
    std::copy(begin_pin,begin_pin+Motion_Num,pin);

    //pin[1] = 0.002*std::sin(param.count/1000.0*2*PI)+begin_pin[1];
    for(int i=0;i<Motion_Num;i++)
    {
        pin[i] = 0.002*param.count/1000.0+begin_pin[i];
        model.motionPool().at(i).setMotPos(pin[i]);
    }


    return 10000 - param.count -1;
}



int main(int argc, char *argv[])
{

    auto &cs = aris::server::ControlServer::instance();

    cs.createModel<aris::dynamic::Model>();
    cs.createController<aris::control::Controller>();
    cs.createSensorRoot<aris::sensor::SensorRoot>();

    cs.sensorRoot().registerChildType<aris::sensor::Imu,false,false,false,false>();

    cs.loadXml("/usr/aris/resource/Robot_III.xml");

    cs.addCmd("en", basicParse, nullptr);
    cs.addCmd("ds", basicParse, nullptr);
    cs.addCmd("hm", basicParse, nullptr);
    cs.addCmd("test",testParse,testGait);

    cs.open();

    cs.setOnExit([&]()
    {
        aris::core::XmlDocument xml_doc;
        xml_doc.LoadFile("/home/vincent/qt_workspace/build/build_new_aris/Robot_III.xml");
        auto model_xml_ele = xml_doc.RootElement()->FirstChildElement("Model");
        if (!model_xml_ele)throw std::runtime_error("can't find Model element in xml file");
        cs.model().saveXml(*model_xml_ele);

        aris::core::stopMsgLoop();
    });
    aris::core::runMsgLoop();



    return 0;
}
