#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <ostream>
#include <thread>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>

bool parseSensorMessage(const char* msg, int& temperature, int& humidity);
int main() {
    std::ofstream log("/var/log/edge-gateway.log", std::ios::app);

    const char* uart_device = "/dev/ttyAMA0";

    int fd = open(uart_device, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        std::cerr << "Failed to open UART: "
                  << uart_device << std::endl;
        return 1;
    }


    termios tty{};

    if (tcgetattr(fd, &tty) != 0)
    {
        std::cerr << "tcgetattr failed" << std::endl;
        close(fd);
        return 1;
    }

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);

    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag = 0;
    tty.c_iflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    tcsetattr(fd, TCSANOW, &tty);

    char buffer[256];
    char msg_to_send[30] ;
    // strcpy(msg_to_send, "LED OFF");

    bool led_on=1;

    strcpy(msg_to_send, "PING\n");
    int num_bytes_written = write(fd,msg_to_send, strlen(msg_to_send));
    int counter=0;
    
    while (true)
    {
        counter++;
        char* cmd = (char*)malloc(sizeof(char)*21);
        if(counter%2==0){
           strcpy(cmd,"GET_STATUS\n");
        }else{
            // const char* cmd = led_on ? "LED_OFF\n" : "LED_ON\n";
            strcpy(cmd,led_on ? "LED_OFF\n" : "LED_ON\n");
            led_on = !led_on;
        }

        int written = write(fd, cmd, strlen(cmd));
        if (written < 0) {
            log << "write failed\n";
        } else {
            log << "Pi sends command line: \n" << cmd<<std::endl<<std::flush;
        }
        free(cmd);

        memset(buffer, 0, sizeof(buffer));
        int num_bytes = read(fd, buffer, sizeof(buffer) - 1);

        if (num_bytes > 0) {
            log << "STM32 replies: \n" << buffer <<std::endl<< std::flush;
        }

        sleep(1);
    }
    close(fd);
    return 0;
}

bool parseSensorMessage(const char* msg, int& temperature, int& humidity)
{
    int parsed = std::sscanf(
        msg,
        "Temp : %d : Humidity : %d",
        &temperature,
        &humidity
    );

    return parsed == 2;
}
