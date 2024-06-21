#include "header.h"

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h> // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE      // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h> // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE        // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL.h>
#include <vector>
#include <unistd.h>

std::vector<float> cpuUtilizationHistory;
std::vector<float> cpuTempHistory;

// systemWindow, display information for the system monitorization
void systemWindow(const char *id, ImVec2 size, ImVec2 position)
{

  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));

  ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(0.2f, 0.2f, 0.2f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(0.2f, 0.2f, 0.2f, 0.7f));

  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(position);
  ImGui::Begin(id);

  ImGui::Text("OS: %s", getOsName());
  ImGui::Text("Current Login: %s", getlogin());
  ImGui::Text("Hostname: %s", hostname());
  ImGui::Text("Total number of process: %d", getProcNum());
  ImGui::Text("CPU type: %s", CPUinfo().c_str());
  ImGui::Dummy(ImVec2(0.0f, 10.0f));

  if (ImGui::BeginTabBar("sys-graphics"))
  {
    if (ImGui::BeginTabItem("CPU"))
    {
      static float targetFPSCPU = 30.0f;
      static float yScaleCPU = 50.0f;
      static float frameCountCPU = 0.0f;
      static bool animGraph = true;

      frameCountCPU += ImGui::GetIO().DeltaTime;
      if (frameCountCPU >= 1.0f / targetFPSCPU && animGraph)
      {
        frameCountCPU = 0.0f;
        double cpuUtilization = getCurrentCpuUtil((1.0f / targetFPSCPU));
        cpuUtilizationHistory.push_back(cpuUtilization);
      }

      ImGui::Dummy(ImVec2(0.0f, 20.0f));

      ImGui::Checkbox("Play Animation", &animGraph);
      ImGui::SliderFloat("Target FPS", &targetFPSCPU, 1.0f, 60.0f, "%.1f FPS");
      ImGui::SliderFloat("Y Scale", &yScaleCPU, 10.0f, 100.0f, "%.1f scalar units");

      ImGui::Dummy(ImVec2(0.0f, 20.0f));
      ImGui::PlotLines("", cpuUtilizationHistory.data(), cpuUtilizationHistory.size(), 0, NULL, 0.0f, yScaleCPU, ImVec2(900, 250));

      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("FAN"))
    {
      ImGui::Text("N/A");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("THERMO"))
    {
      static float targetFPSTHERMO = 30.0f;
      static float yScaleTHERMO = 100.0f;
      static float frameCountTHERMO = 0.0f;
      static bool animGraph = true;

      frameCountTHERMO += ImGui::GetIO().DeltaTime;
      if (frameCountTHERMO >= 1.0f / targetFPSTHERMO && animGraph)
      {
        frameCountTHERMO = 0.0f;
        float cpuTemp = getCPUTemp();
        cpuTempHistory.push_back(cpuTemp);
      }

      ImGui::Dummy(ImVec2(0.0f, 20.0f));

      ImGui::Checkbox("Play Animation", &animGraph);
      ImGui::SliderFloat("Target FPS", &targetFPSTHERMO, 1.0f, 60.0f, "%.1f FPS");
      ImGui::SliderFloat("Y Scale", &yScaleTHERMO, 100.0f, 1000.0f, "%.1f scalar units");

      ImGui::Dummy(ImVec2(0.0f, 20.0f));
      ImGui::PlotLines("", cpuTempHistory.data(), cpuTempHistory.size(), 0, NULL, 0.0f, yScaleTHERMO, ImVec2(900, 250));

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  ImGui::End();
  ImGui::PopStyleColor(8);
}

vector<ProcessInfo> processes = parseProcessInfo();
std::vector<bool> selected(processes.size(), false);

// memoryProcessesWindow, display information for the memory and processes information
void memoryProcessesWindow(const char *id, ImVec2 size, ImVec2 position)
{
  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));

  ImGui::Begin(id);
  ImGui::SetWindowSize(id, size);
  ImGui::SetWindowPos(id, position);

  auto [totalMemory, usedMemory] = getMemUsage();
  float usedMemoryPercentage = static_cast<float>(usedMemory) / totalMemory;
  std::string memoryUsageText = std::to_string(usedMemory) + "/" + std::to_string(totalMemory) + "GB used";

  auto [totalSWAP, freeSWAP] = getSWAPUsage();
  float freeSWAPPercentage = static_cast<float>(freeSWAP) / totalSWAP;
  std::string SWAPUsageText = std::to_string(freeSWAP) + "/" + std::to_string(totalSWAP) + "GB free";

  auto [usedDisk, totalDisk] = parseDiskUsage();
  float diskUsagePercentage = static_cast<float>(usedDisk) / totalDisk;
  std::string diskUsageText = std::to_string(usedDisk) + "/" + std::to_string(totalDisk) + "GB used";

  ImGui::Text("Physical Mem (RAM)");
  ImVec2 barSize = ImVec2(690.0f, 30.0f);
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.4f, 0.2f, 0.7f));
  ImGui::ProgressBar(usedMemoryPercentage, barSize);
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::Text("%s", memoryUsageText.c_str());
  ImGui::Dummy(ImVec2(0.0f, 10.0f));

  ImGui::Text("Virtual Mem (SWAP)");
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.1f, 0.2f, 0.7f));
  ImGui::ProgressBar(freeSWAPPercentage, barSize);
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::Text("%s", SWAPUsageText.c_str());
  ImGui::Dummy(ImVec2(0.0f, 10.0f));

  ImGui::Text("Disk Usage");
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.1f, 0.5f, 0.7f));
  ImGui::ProgressBar(diskUsagePercentage, barSize);
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::Text("%s", diskUsageText.c_str());

  static char filterCMD[255] = "";
  ImGui::Dummy(ImVec2(0.0f, 10.0f));

  if (ImGui::CollapsingHeader("Proc Table"))
  {
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::InputTextWithHint("##cmd", "filter procs by cmd", filterCMD, sizeof(filterCMD));
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    if (ImGui::BeginTable("proc-table", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable))
    {
      // Setting up headers
      ImGui::TableSetupColumn("PID");
      ImGui::TableSetupColumn("cmd");
      ImGui::TableSetupColumn("state");
      ImGui::TableSetupColumn("cpu%");
      ImGui::TableSetupColumn("mem%");
      ImGui::TableHeadersRow();

      for (size_t i = 0; i < processes.size(); ++i)
      {
        const auto &process = processes[i];

        if (strlen(filterCMD) > 0 && process.name.find(filterCMD) == std::string::npos)
        {
          continue;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (ImGui::Selectable(std::to_string(process.pid).c_str(), selected[i], ImGuiSelectableFlags_SpanAllColumns))
        {
          selected[i] = !selected[i];
        }

        if (selected[i])
        {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(0.7f, 0.0f, 0.0f, 1.0f)));
        }

        ImGui::TableNextColumn();

        ImGui::Text("%s", process.name.c_str());
        ImGui::TableNextColumn();

        ImGui::Text("%s", process.state.c_str());
        ImGui::TableNextColumn();

        ImGui::Text("%.2f%%", process.cpuUsage);
        ImGui::TableNextColumn();

        ImGui::Text("%.2f%%", process.memoryUsage);
      }

      ImGui::EndTable();
    }
  }

  ImGui::End();
  ImGui::PopStyleColor(7);
}

// network, display information network information
void networkWindow(const char *id, ImVec2 size, ImVec2 position)
{
  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));

  ImGui::Begin(id);
  ImGui::SetWindowSize(id, size);
  ImGui::SetWindowPos(id, position);

  ImGui::Text("IPv4 Interfaces from ARP table");
  ImGui::Dummy(ImVec2(0.0f, 5.0f));
  for (const auto &ipv4 : GetIPv4Addrs())
  {
    ImGui::Text("%s ---------------> %s", ipv4.name.c_str(), ipv4.addressBuffer);
  }
  ImGui::Dummy(ImVec2(0.0f, 10.0f));

  vector<NetInterface> interfaces = ParseNetInterfaces();
  vector<PresentableNetInterface> pinterfacesRX = SimplifyNetDataRX();
  vector<PresentableNetInterface> pinterfacesTX = SimplifyNetDataTX();

  if (ImGui::CollapsingHeader("net tables"))
  {
    if (ImGui::CollapsingHeader("RX Table"))
    {
      if (ImGui::BeginTable("rx-table", 9, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable))
      {
        ImGui::TableSetupColumn("interface");
        ImGui::TableSetupColumn("bytes");
        ImGui::TableSetupColumn("packets");
        ImGui::TableSetupColumn("errs");
        ImGui::TableSetupColumn("drop");
        ImGui::TableSetupColumn("fifo");
        ImGui::TableSetupColumn("frame");
        ImGui::TableSetupColumn("compressed");
        ImGui::TableSetupColumn("multicast");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < interfaces.size(); i++)
        {
          const NetInterface interface = interfaces[i];
          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Text(interface.Name.c_str());
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.bytes);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.packets);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.errs);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.drop);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.fifo);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.frame);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.compressed);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.recvStats.multicast);
          ImGui::TableNextColumn();
        }

        ImGui::EndTable();
      }
    }

    if (ImGui::CollapsingHeader("TX Table"))
    {
      if (ImGui::BeginTable("tx-table", 9, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable))
      {
        ImGui::TableSetupColumn("interface");
        ImGui::TableSetupColumn("bytes");
        ImGui::TableSetupColumn("packets");
        ImGui::TableSetupColumn("errs");
        ImGui::TableSetupColumn("drop");
        ImGui::TableSetupColumn("fifo");
        ImGui::TableSetupColumn("colls");
        ImGui::TableSetupColumn("carrier");
        ImGui::TableSetupColumn("compressed");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < interfaces.size(); i++)
        {
          const NetInterface interface = interfaces[i];
          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Text(interface.Name.c_str());
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.bytes);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.packets);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.errs);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.drop);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.fifo);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.colls);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.carrier);
          ImGui::TableNextColumn();

          ImGui::Text("%ld", interface.transStats.compressed);
          ImGui::TableNextColumn();
        }

        ImGui::EndTable();
      }
    }
  }

  ImGui::Dummy(ImVec2(0.0f, 20.0f));

  if (ImGui::BeginTabBar("net-graphs"))
  {

    if (ImGui::BeginTabItem("recv (RX)"))
    {
      for (const auto &pinterface : pinterfacesRX)
      {
        ImGui::Text("%s", ltrim(pinterface.Name).c_str());
        double progress = static_cast<double>(pinterface.data.first) / 2147483648.0;
        ImGui::Text("0.0 GB");
        ImGui::SameLine();
        ImGui::ProgressBar(progress, ImVec2(1390.0f, 30.0f), pinterface.data.second.c_str());
        ImGui::SameLine();
        ImGui::Text("2.0 GB");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
      }

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("trans (TX)"))
    {
      for (const auto &pinterface : pinterfacesTX)
      {
        ImGui::Text("%s", ltrim(pinterface.Name).c_str());
        double progress = static_cast<double>(pinterface.data.first) / 2147483648.0;
        ImGui::Text("0.0 GB");
        ImGui::SameLine();
        ImGui::ProgressBar(progress, ImVec2(1390.0f, 30.0f), pinterface.data.second.c_str());
        ImGui::SameLine();
        ImGui::Text("2.0 GB");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
      }

      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
  ImGui::PopStyleColor(7);
}

// Main code
int main(int, char **)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
  {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Window *window = SDL_CreateWindow("sys-dog", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
  bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
  bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
  bool err = false;
  glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
  bool err = false;
  glbinding::initialize([](const char *name)
                        { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
  bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
  if (err)
  {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // render bindings
  ImGuiIO &io = ImGui::GetIO();

  // Setup Dear ImGui style
  ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.0f);

  io.Fonts->AddFontFromFileTTF("assets/csfont.ttf", 20.7f);
  io.FontDefault = io.Fonts->Fonts.back();

  bool done = false;
  while (!done)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    {
      ImVec2 mainDisplay = io.DisplaySize;
      memoryProcessesWindow("mem/proc",
                            ImVec2((mainDisplay.x / 2) - 20, (mainDisplay.y / 2) + 30),
                            ImVec2((mainDisplay.x / 2) + 10, 10));
      // --------------------------------------
      systemWindow("sys-info",
                   ImVec2((mainDisplay.x / 2) - 10, (mainDisplay.y / 2) + 30),
                   ImVec2(10, 10));
      // --------------------------------------
      networkWindow("net-dev",
                    ImVec2(mainDisplay.x - 20, (mainDisplay.y / 2) - 60),
                    ImVec2(10, (mainDisplay.y / 2) + 50));
    }

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
