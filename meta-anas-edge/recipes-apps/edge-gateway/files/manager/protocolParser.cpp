#include "manager/protocolParser.hpp"
#include <cstdio> // Required for sscanf

namespace cc::manager {

    cc::utils::Result<bool> ProtocolParser::isAck(std::string line){
        if(line.starts_with("ACK:")){
            return  cc::utils::Result<bool>::ok(true);
        }
        return  cc::utils::Result<bool>::ok(false);
    }

    cc::utils::Result<bool> ProtocolParser::isAckandPing(std::string line){
     if(line.starts_with("ACK:PING")){
            return  cc::utils::Result<bool>::ok(true);
        }
        return  cc::utils::Result<bool>::ok(false);
    }
    cc::utils::Result<bool> ProtocolParser::isNAck(std::string line){
        if(line.starts_with("NACK:")){
            return  cc::utils::Result<bool>::ok(true);
        }
        return  cc::utils::Result<bool>::ok(false);
    }

    cc::utils::Result<bool> ProtocolParser::isStatus(std::string line){
        if(line.starts_with("STATUS:")){
            return  cc::utils::Result<bool>::ok(true);
        }
        return  cc::utils::Result<bool>::ok(false);
    }

    cc::utils::Result<Telemetry> ProtocolParser::parseStatus(std::string line){
        Telemetry data;
        // STATUS:TEMP=%d;HUM=%d;LOAD=%d;LOAD_STATUS=%s
        int matched = std::sscanf(line.c_str(), "STATUS:TEMP=%d;HUM=%d;LOAD=%d;DHT_STATUS=%29[^;];LOAD_STATUS=%29[^;];SYSTEM_STATUS=%29[^;]", &(data.temperature), &(data.humidity),&(data.load),data.dht_status,data.load_status,data.system_status);
        if(matched==6){
            return cc::utils::Result<Telemetry>::ok(data);
        }else{
            return cc::utils::Result<Telemetry>::fail(cc::utils::ErrorCode::ParseError,"can't parse data");

        }
    }
}
