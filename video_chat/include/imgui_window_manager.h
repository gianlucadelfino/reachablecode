#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>

class imgui_window_manager
{
public:
  imgui_window_manager(const std::string& window_name, size_t win_width, size_t win_height)
      : _window_name(window_name)
  {
    _window = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>(
        glfwCreateWindow(win_width, win_height, window_name.c_str(), nullptr, nullptr),
        &glfwDestroyWindow);
    if (!_window)
    {
      throw std::runtime_error("Could not create window");
    }

    glfwMakeContextCurrent(_window.get());
    glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK)
    {
      throw std::runtime_error("Could not init glew.");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(_window.get(), true);
    ImGui_ImplOpenGL3_Init();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    glViewport(0, 0, win_height, win_height);
  }

  bool is_running() { return !glfwWindowShouldClose(_window.get()); }

  void render()
  {
    glfwPollEvents();
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      ImGui::Begin(_window_name.c_str());

      for (auto& ui_elem : _ui_elements)
      {
        ui_elem();
      }

      ImGui::End();
    }

    ImGui::Render();
    // This can cause leaks on some platforms https://github.com/ocornut/imgui/issues/4468
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    int display_w{};
    int display_h{};
    glfwGetFramebufferSize(_window.get(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glfwSwapBuffers(_window.get());
    ImGui::EndFrame();
  }

  ~imgui_window_manager()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
  }

  void add_ui_elem(std::function<void()>&& imgui_element)
  {
    _ui_elements.emplace_back(std::move(imgui_element));
  }

private:
  struct glfw_resurce_handler
  {
    glfw_resurce_handler()
    {
      glfwSetErrorCallback(glfw_error_callback);
      if (glfwInit() == GLFW_FALSE)
      {
        throw std::runtime_error("Could not init glfw.");
      }
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    }

    static void glfw_error_callback(int error, const char* description)
    {
      std::cerr << "Glfw Error :" << error << ", " << description << std::endl;
    }

    ~glfw_resurce_handler()
    {
      // This is not needed if we cleaned up all the windows already. Calling
      // it might at worst produce a warning like
      // "Glfw Error :65537, The GLFW library is not initialized" if it wasn't
      // required. Not a big deal.
      glfwTerminate();
    }
  };

  const std::string _window_name;
  std::vector<std::function<void()>> _ui_elements;

  std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> _window{nullptr, glfwDestroyWindow};
  glfw_resurce_handler _glfw_resource_handler;
};