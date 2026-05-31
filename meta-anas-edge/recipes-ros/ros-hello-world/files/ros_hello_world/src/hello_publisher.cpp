#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class HelloPublisher : public rclcpp::Node
{
public:
    HelloPublisher()
    : Node("hello_publisher")
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>(
            "/hello_world",
            10
        );

        timer_ = this->create_wall_timer(
            1s,
            std::bind(&HelloPublisher::publish_message, this)
        );

        RCLCPP_INFO(this->get_logger(), "Hello publisher started");
    }

private:
    void publish_message()
    {
        std_msgs::msg::String message;
        message.data = "Hello from ROS2 Jazzy on Yocto Raspberry Pi";

        publisher_->publish(message);

        RCLCPP_INFO(
            this->get_logger(),
            "Published: '%s'",
            message.data.c_str()
        );
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<HelloPublisher>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
