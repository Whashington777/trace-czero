#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "helpers/fonts.h"
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
extern char** environ;
namespace {
std::filesystem::path executable_directory() {
    std::array<char, 4096> path{};
    auto size = readlink("/proc/self/exe", path.data(), path.size() - 1);
    return size > 0 ? std::filesystem::path(std::string(path.data(), size)).parent_path()
                    : std::filesystem::current_path();
}
pid_t launch(const std::filesystem::path& build, const std::filesystem::path& directory, std::string& message) {
    const auto binary = directory / "hl_linux";
    const auto library = build / "libtracecs16.so";
    if (!std::filesystem::exists(binary) || !std::filesystem::exists(library)) {
        message = "Не найден hl_linux или библиотека оверлея.";
        return -1;
    }
    std::vector<std::string> variables;
    for (char** entry = environ; *entry; ++entry) {
        std::string value = *entry;
        if (value.starts_with("LD_PRELOAD=") || value.starts_with("LD_LIBRARY_PATH=") ||
            value.starts_with("TRACE_") || value.starts_with("SteamAppId=") || value.starts_with("SteamGameId=")) continue;
        variables.push_back(std::move(value));
    }
    variables.push_back("LD_PRELOAD=" + library.string());
    std::string search = "LD_LIBRARY_PATH=" + (build / "gui").string() + ":" +
        (build / "runtime").string() + ":/usr/lib32:" + directory.string();
    if (const char* existing = std::getenv("LD_LIBRARY_PATH"); existing && *existing) search += std::string(":") + existing;
    variables.push_back(search);
    variables.push_back("SteamAppId=80");
    variables.push_back("SteamGameId=80");
    std::vector<char*> environment;
    for (auto& value : variables) environment.push_back(value.data());
    environment.push_back(nullptr);
    std::string executable = binary.string();
    char mod[] = "-game", name[] = "czero";
    char* arguments[] = {executable.data(), mod, name, nullptr};
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addchdir_np(&actions, directory.c_str());
    auto log = build / "launcher-game.log";
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);
    pid_t process = -1;
    int error = posix_spawn(&process, binary.c_str(), &actions, nullptr, arguments, environment.data());
    posix_spawn_file_actions_destroy(&actions);
    if (error) { message = std::string("Не удалось запустить: ") + std::strerror(error); return -1; }
    message = "Игра запущена. Карту и ботов выбираешь в игре.";
    return process;
}
}
int main() {
    const auto build = executable_directory();
    const char* home = std::getenv("HOME");
    std::array<char, 2048> directory{};
    std::snprintf(directory.data(), directory.size(), "%s/.local/share/Steam/steamapps/common/Half-Life", home ? home : "");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return 1;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    auto window = SDL_CreateWindow("TRACE / Condition Zero", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   580, 340, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) { SDL_Quit(); return 1; }
    auto context = SDL_GL_CreateContext(window);
    if (!context) { SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    SDL_GL_MakeCurrent(window, context);
    SDL_GL_SetSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().LogFilename = nullptr;
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowPadding = {28, 24};
    style.FramePadding = {12, 10};
    style.FrameRounding = 7;
    style.ItemSpacing = {10, 12};
    style.Colors[ImGuiCol_WindowBg] = {.065f, .067f, .085f, 1};
    style.Colors[ImGuiCol_Button] = {.49f, .34f, .77f, 1};
    style.Colors[ImGuiCol_ButtonHovered] = {.58f, .42f, .88f, 1};
    style.Colors[ImGuiCol_ButtonActive] = {.43f, .28f, .69f, 1};
    if (!ImGui_ImplSDL2_InitForOpenGL(window, context)) { ImGui::DestroyContext(); SDL_GL_DeleteContext(context); SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    if (!ImGui_ImplOpenGL3_Init("#version 120")) { ImGui_ImplSDL2_Shutdown(); ImGui::DestroyContext(); SDL_GL_DeleteContext(context); SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    fonts::init();
    bool done = false;
    pid_t child = -1;
    std::string message;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)) done = true;
        }
        if (done) break;
        if (child > 0) {
            int status;
            if (waitpid(child, &status, WNOHANG) == child) {
                child = -1;
                message = WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "Игра закрыта." : "Игра завершилась. Подробности: build/launcher-game.log";
            }
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) { SDL_Delay(50); continue; }
        fonts::sync_ui_scale(1.f);
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplSDL2_NewFrame(); ImGui::NewFrame();
        auto display = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos({0, 0}); ImGui::SetNextWindowSize(display);
        ImGui::Begin("TRACE launcher", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
        auto dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled({28, 25}, {32, 54}, IM_COL32(169, 128, 255, 255), 2.f);
        auto font = fonts::inter(13.f, true); if (!font) font = ImGui::GetFont();
        dl->AddText(font, 25.f, {44, 24}, IM_COL32(239, 239, 247, 255), "TRACE");
        ImGui::SetCursorPos({28, 72});
        ImGui::TextColored({.67f, .68f, .75f, 1}, "Counter-Strike: Condition Zero");
        ImGui::Spacing();
        ImGui::TextUnformatted("Папка игры");
        ImGui::SetNextItemWidth(-1);
        ImGui::BeginDisabled(child > 0);
        ImGui::InputText("##game_folder", directory.data(), directory.size());
        ImGui::Spacing();
        if (ImGui::Button(child > 0 ? "Игра запущена" : "Запустить Condition Zero", {-1, 46})) child = launch(build, directory.data(), message);
        ImGui::EndDisabled();
        if (!message.empty()) ImGui::TextWrapped("%s", message.c_str());
        else ImGui::TextColored({.64f, .65f, .72f, 1}, "Карту и ботов выбираешь в игре.");
        ImGui::End(); ImGui::Render();
        int width, height; SDL_GL_GetDrawableSize(window, &width, &height);
        glViewport(0, 0, width, height); glClearColor(.06f, .06f, .08f, 1); glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); SDL_GL_SwapWindow(window);
    }
    
    fonts::shutdown(); ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplSDL2_Shutdown(); ImGui::DestroyContext();
    SDL_GL_DeleteContext(context); SDL_DestroyWindow(window); SDL_Quit();
}
