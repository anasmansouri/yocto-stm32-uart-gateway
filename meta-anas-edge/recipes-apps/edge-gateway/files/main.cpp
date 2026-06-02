#include <ostream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "manager/uartManager.hpp"
#include "manager/protocolParser.hpp"
#include "manager/ipcServer.hpp"
#include "utils/Result.hpp"
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

enum class LED_STATE
{
    OFF,
    ON_YELLOW,
    ON_RED,
    ON_GREEN
};
int main()
{
    std::ofstream log("/var/log/edge-gateway.log", std::ios::app);
    cc::manager::UartManager uartManager("/dev/ttyAMA0");
    if (!uartManager.openUartDevice())
    {
        return -1;
    }
    if (!uartManager.configureUartDevice())
    {
        return -1;
    }
    sleep(3);
    cc::manager::ProtocolParser protocolParser;
    // test
    cc::manager::IpcServer ipcServer("/tmp/stm32-gateway.sock");

    if (!ipcServer.start())
    {
        return -1;
    }
    // handschake
    while (true)
    {
        if (!uartManager.writeLine("PING"))
        {
            log << "PI can' write" << std::endl
                << std::flush;
            continue;
        }
        log << "PI sent PING" << std::endl
            << std::flush;
        cc::utils::Result<std::string> response = uartManager.readLine();
        if (!response)
        {
            log << "PI can' read" << std::endl
                << std::flush;
            continue;
        }
        if (protocolParser.isAckandPing(response.unwrap()))
        {
            log << "PING received : " << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    int counter = 0;
    bool led_on = 1;
    std::string cmd;
    LED_STATE old_led_state = {LED_STATE::OFF};
    LED_STATE current_led_state{LED_STATE::OFF};
    while (true)
    {
        if (old_led_state == current_led_state)
        {
            cmd = "GET_STATUS";
        }
        else
        {
            old_led_state = current_led_state;
        }

        if (!uartManager.writeLine(cmd))
        {
            log << "writing fail " << std::endl
                << std::flush;
        }
        log << "PI Sent : " << cmd << std::endl
            << std::flush;
        cc::utils::Result<std::string> response = uartManager.readLine();

        if (response)
        {
            log << "STM32 REPLIES : " << response.unwrap() << std::endl
                << std::flush;
            if (protocolParser.isAck(response.unwrap()).unwrap())
            {
                log << "ack response" << std::endl
                    << std::flush;
            }
            else if (protocolParser.isNAck(response.unwrap()).unwrap())
            {
                log << "Nack response" << std::endl
                    << std::flush;
            }
            else if (protocolParser.isStatus(response.unwrap()).unwrap())
            {
                cc::utils::Result<cc::manager::Telemetry> data = protocolParser.parseStatus(response.unwrap());
                if (data)
                {
                    log << "temperature : " << data.unwrap().temperature << " humidity : " << data.unwrap().humidity << "load : " << data.unwrap().load << "%" << "dht status : " << data.unwrap().dht_status << "load status : " << data.unwrap().load_status << "system status : " << data.unwrap().system_status << std::endl
                        << std::flush;
                    std::string json =
                        "{\"type\":\"sensor_data\",\"temperature\":" +
                        std::to_string(data.unwrap().temperature) +
                        ",\"humidity\":" +
                        std::to_string(data.unwrap().humidity) +
                        ",\"load\":" +
                        std::to_string(data.unwrap().load) +
                        "}\n";

                    ipcServer.broadcastLine(json);
                    if (data.unwrap().load > 75)
                    {
                        log << "LED needs to be RED" << std::endl
                            << std::flush;
                        cmd = "SET_LED:RED";
                        current_led_state = LED_STATE::ON_RED;
                    }
                    else if (data.unwrap().load < 40)
                    {
                        log << "LED needs to be GREEN" << std::endl
                            << std::flush;
                        cmd = "SET_LED:GREEN";
                        current_led_state = LED_STATE::ON_GREEN;
                    }
                    else
                    {
                        log << "LED needs to be YELLOW" << std::endl
                            << std::flush;
                        cmd = "SET_LED:YELLOW";
                        current_led_state = LED_STATE::ON_YELLOW;
                    }
                }
                else
                {
                    log << "wlah ma3reft n parsi data" << std::endl
                        << std::flush;
                    // cmd="SET_LED:OFF";
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
