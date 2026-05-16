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
    std::string uart_device = "/dev/ttyAMA0";
    cc::manager::UartManager uartManager(uart_device);
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
        cc::utils::Result<std::string>msg = uartManager.readLine();
        if(!msg){
            log<<"PI can' read"<<std::endl<<std::flush;
            continue;
        }
        if(protocolParser.isAckandPing(msg.unwrap())){
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
        cc::utils::Result<std::string>msg = uartManager.readLine();

        if(msg){
            log<<"STM32 REPLIES : "<<msg.unwrap()<<std::endl<<std::flush;
            if(protocolParser.isAck(msg.unwrap()).unwrap()){
                log<<"ack msg"<<std::endl<<std::flush;
            }else if(protocolParser.isNAck(msg.unwrap()).unwrap()){
                log<<"Nack msg"<<std::endl<<std::flush;
            }else if(protocolParser.isStatus(msg.unwrap()).unwrap()){
                cc::utils::Result<cc::manager::Telemetry> data = protocolParser.parseStatus(msg.unwrap());
                if(data){
                    log<<"temp hya :"<<data.unwrap().temperature<<" hum hya : "<<data.unwrap().humidity<<std::endl<<std::flush;
                }else{

                    log<<"wlah ma3reft n parsi data"<<std::endl<<std::flush;
                }
            }
        }
        sleep(10);
    }
}
