#include <ostream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "manager/uartManager.hpp"
#include "manager/protocolParser.hpp"
#include "utils/Result.hpp"
#include <unistd.h>
#include <cstring>

int main(){
    std::ofstream log("/var/log/edge-gateway.log", std::ios::app);
    cc::manager::UartManager uartManager("/dev/ttyAMA0");
    if(!uartManager.openUartDevice()){
        return -1;
    }
    if(!uartManager.configureUartDevice()){
        return -1;
    }
    sleep(3);
    cc::manager::ProtocolParser protocolParser;
    // handschake
    while(true){
        if(!uartManager.writeLine("PING")){
            log<<"PI can' write"<<std::endl<<std::flush;
            continue;
        }
        log <<"PI sent PING"<<std::endl<<std::flush; 
        cc::utils::Result<std::string> response= uartManager.readLine();
        if(!response){
            log<<"PI can' read"<<std::endl<<std::flush;
            continue;
        }
        if(protocolParser.isAckandPing(response.unwrap())){
            log<<"PING received : "<<std::endl;
            break;
        }
        sleep(5);
    }

    int counter=0;
    bool led_on=1;
    while (true) {

        counter++;
        std::string cmd;
        if(counter%2==0){
           cmd = "GET_STATUS";
        }else{
            cmd = led_on ? "LED_OFF" : "LED_ON";
            led_on = !led_on;
        }

        if(!uartManager.writeLine(cmd)){
            log<<"writing fail "<<std::endl<<std::flush;
        }
        log<<"PI Sent : "<<cmd<<std::endl<<std::flush;
        cc::utils::Result<std::string> response = uartManager.readLine();

        if(response){
            log<<"STM32 REPLIES : "<<response.unwrap()<<std::endl<<std::flush;
            if(protocolParser.isAck(response.unwrap()).unwrap()){
                log<<"ack response"<<std::endl<<std::flush;
            }else if(protocolParser.isNAck(response.unwrap()).unwrap()){
                log<<"Nack response"<<std::endl<<std::flush;
            }else if(protocolParser.isStatus(response.unwrap()).unwrap()){
                cc::utils::Result<cc::manager::Telemetry> data = protocolParser.parseStatus(response.unwrap());
                if(data){
                    log<<"temperature : "<<data.unwrap().temperature<<" humidity : "<<data.unwrap().humidity<<"load : "<<data.unwrap().load<<"%"<<std::endl<<std::flush;
                }else{
                    log<<"wlah ma3reft n parsi data"<<std::endl<<std::flush;
                }
            }
        }
        sleep(10);
    }
}
