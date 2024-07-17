#include "chronos_window.hpp"
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace Chronos {

    ChronosWindow::ChronosWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
    {
        initWindow();
    }

    ChronosWindow::~ChronosWindow() 
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void ChronosWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void ChronosWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void ChronosWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto chronosWindow = reinterpret_cast<ChronosWindow *>(glfwGetWindowUserPointer(window));
        chronosWindow->framebufferResized = true;
        chronosWindow->width = width;
        chronosWindow->height = height;
    }
}
