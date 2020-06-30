#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include "ImGuiExt.h"
#include "TextEditor.h"
#include "Sources.h"
#include "MarkdownHelper.h"
#include "HyperlinkHelper.h"
#include <fplus/fplus.hpp>

static std::string gImGuiRepoUrl = "https://github.com/pthom/imgui/blob/DemoCode/";

TextEditor *gEditorImGuiDemo = nullptr;
void implImGuiDemoCallbackDemoCallback(int line_number)
{
    int cursorLineOnPage = 3;
    gEditorImGuiDemo->SetCursorPosition({line_number, 0}, cursorLineOnPage);
}


void menuEditorTheme(const std::vector<TextEditor *> editors)
{
    if (ImGui::BeginMenu("Editor"))
    {
        if (ImGui::MenuItem("Dark palette"))
            for (auto editor: editors)
                editor->SetPalette(TextEditor::GetDarkPalette());
        if (ImGui::MenuItem("Light palette"))
            for (auto editor: editors)
                editor->SetPalette(TextEditor::GetLightPalette());
        if (ImGui::MenuItem("Retro blue palette"))
            for (auto editor: editors)
                editor->SetPalette(TextEditor::GetRetroBluePalette());
        ImGui::EndMenu();
    }
}


void setEditorAnnotatedSource(TextEditor & editor, const Sources::AnnotatedSource &annotatedSource)
{
    editor.SetText(annotatedSource.source.sourceCode);
    std::unordered_set<int> lineNumbers;
    for (auto line : annotatedSource.linesWithTags)
        lineNumbers.insert(line.lineNumber);
    editor.SetBreakpoints(lineNumbers);
}


TextEditor factorCppEditor()
{
    TextEditor editor;
    editor.SetPalette(TextEditor::GetLightPalette());
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    return editor;
}


class LibrariesCodeBrowser
{
public:
    LibrariesCodeBrowser(
            const std::vector<Sources::Library>& librarySources,
            std::string currentSourcePath
            ) :
              mLibrarySources(librarySources)
            , mCurrentSource(Sources::ReadSource(currentSourcePath))
            , mEditor(factorCppEditor())
    {
        mEditor.SetText(mCurrentSource.sourceCode);
    }

    void gui()
    {
        if (guiSelectLibrarySource())
            mEditor.SetText(mCurrentSource.sourceCode);

        if (fplus::is_suffix_of(std::string(".md"), mCurrentSource.sourcePath))
            MarkdownHelper::Markdown(mCurrentSource.sourceCode);
        else
            mEditor.Render(mCurrentSource.sourcePath.c_str());
    }
private:
    bool guiSelectLibrarySource()
    {
        bool changed = false;
        for (const auto & librarySource: mLibrarySources)
        {
            ImGui::Text("%s", librarySource.name.c_str());
            ImGui::SameLine(ImGui::GetWindowSize().x - 350.f );
            ImGuiExt::Hyperlink(librarySource.url);
            for (const auto & source: librarySource.sourcePaths)
            {
                std::string currentSourcePath = librarySource.path + "/" + source;
                bool isSelected = (currentSourcePath == mCurrentSource.sourcePath);
                std::string buttonLabel = source + "##" + librarySource.path;
                if (isSelected)
                    ImGui::TextDisabled("%s", source.c_str());
                else if (ImGui::Button(buttonLabel.c_str()))
                {
                    mCurrentSource = Sources::ReadSource(currentSourcePath);
                    changed = true;
                }
                ImGuiExt::SameLine_IfPossible(80.f);
            }
            ImGui::NewLine();
            ImGui::Separator();
        }
        return changed;
    }

private:
    std::vector<Sources::Library> mLibrarySources;
    Sources::Source mCurrentSource;
    TextEditor mEditor;
};


class ImGuiDemoBrowser
{
public:
    ImGuiDemoBrowser() :
          mAnnotatedSource(Sources::ReadImGuiDemoCode("imgui/imgui_demo.cpp"))
        , mEditor(factorCppEditor())
    {
        _setEditorAnnotatedSource();
    }

    void gui()
    {
        std::string help =
"This is the code of imgui_demo.cpp. It is the best way to learn about Dear ImGui! \n"
"On the left, you can see a demo that showcases all the widgets and features of ImGui: "
"Click on the \"Code\" buttons to see their code and learn about them. \n"
"Alternatively, you can also search for some features (try searching for \"widgets\", \"layout\", \"drag\", etc)";
        ImGui::TextWrapped("%s", help.c_str());
        guiDemoCodeTags();
        guiGithubButton();
        mEditor.Render("imgui_demo.cpp");
    }

    TextEditor * _GetTextEditorPtr() {
        return &mEditor;
    }
private:
    void guiGithubButton()
    {
        if (ImGui::Button("View on github at this line"))
        {
            std::string url = gImGuiRepoUrl + "imgui_demo.cpp#L"
                              + std::to_string(mEditor.GetCursorPosition().mLine);
            HyperlinkHelper::OpenUrl(url);
        }
    }


    void guiDemoCodeTags()
    {
        bool showTooltip = false;
        ImGui::Text("Search demos"); ImGui::SameLine();
        if (ImGui::IsItemHovered())
            showTooltip = true;
        ImGui::TextDisabled("?"); ImGui::SameLine();
        if (ImGui::IsItemHovered())
            showTooltip = true;
        if (showTooltip)
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Filter usage:\n"
                                   "  \"\"         display all\n"
                                   "  \"xxx\"      display items containing \"xxx\"\n"
                                   "  \"xxx,yyy\"  display items containing \"xxx\" or \"yyy\"\n"
                                   "  \"-xxx\"     hide items containing \"xxx\"");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }


        static ImGuiTextFilter filter;
        filter.Draw();
        if (strlen(filter.InputBuf) >= 3)
        {
            for (const auto & lineWithNote : mAnnotatedSource.linesWithTags)
            {
                if (filter.PassFilter(lineWithNote.tag.c_str()))
                {
                    if (ImGui::Button(lineWithNote.tag.c_str()))
                        mEditor.SetCursorPosition({lineWithNote.lineNumber, 0}, 3);
                    ImGuiExt::SameLine_IfPossible();
                }
            }
            ImGui::NewLine();
        }
    }

    void _setEditorAnnotatedSource()
    {
        setEditorAnnotatedSource(mEditor, mAnnotatedSource);
    }


    Sources::AnnotatedSource mAnnotatedSource;
    TextEditor mEditor;
};


// Show ImGui Readme.md
class ImGuiReadmeBrowser
{
public:
    ImGuiReadmeBrowser() : mSource(Sources::ReadSource("imgui/README.md")) {}
    void gui() {
        MarkdownHelper::Markdown(mSource.sourceCode);
    }
private:
    Sources::Source mSource;
};

// Show doc in imgui.cpp
class ImGuiCppDocBrowser
{
public:
    ImGuiCppDocBrowser()
        : mAnnotatedSource(Sources::ReadImGuiCppDoc("imgui/imgui.cpp"))
        , mEditor(factorCppEditor())
    {
        setEditorAnnotatedSource(mEditor, mAnnotatedSource);
    }
    void gui()
    {
        ImGui::Text("The doc for Dear ImGui is simply stored inside imgui.cpp");
        guiTags();
        guiGithubButton();
        mEditor.Render("imgui.cpp");
    }
    TextEditor *_GetTextEditorPtr() {
        return &mEditor;
    }
private:
    void guiTags()
    {
        for (auto lineWithTag : mAnnotatedSource.linesWithTags)
        {
            // tags are of type H1 or H2, and begin with "H1 " or "H2 " (3 characters)
            std::string title = fplus::drop(3, lineWithTag.tag);
            bool isHeader1 = (fplus::take(3, lineWithTag.tag) == "H1 ");
            if (isHeader1)
            {
                if (ImGuiExt::ClickableText(title.c_str()))
                    mEditor.SetCursorPosition({lineWithTag.lineNumber, 0}, 3);
            }
        }
    }
    void guiGithubButton()
    {
        if (ImGui::Button("View on github at this line"))
        {
            std::string url = gImGuiRepoUrl + "imgui.cpp#L"
                              + std::to_string(mEditor.GetCursorPosition().mLine);
            HyperlinkHelper::OpenUrl(url);
        }
    }


    Sources::AnnotatedSource mAnnotatedSource;
    TextEditor mEditor;
};

struct AppState
{
    std::map<std::string, LibrariesCodeBrowser> librariesCodeBrowsers =
        {
            { "imguiSources", {Sources::imguiLibrary(),            "imgui/imgui.cpp"} },
            { "thisDemoSources", {Sources::thisDemoLibraries(),    "imgui_hellodemo/Readme.md"} },
            { "otherLibrariesSources", {Sources::otherLibraries(), ""} },
        };
};



int main(int, char **)
{
    ImGuiDemoBrowser imGuiDemoBrowser;
    ImGuiCppDocBrowser imGuiCppDocBrowser;

    gEditorImGuiDemo = imGuiDemoBrowser._GetTextEditorPtr();

    std::vector<TextEditor *> allEditors {
        imGuiDemoBrowser._GetTextEditorPtr(),
        imGuiCppDocBrowser._GetTextEditorPtr()
    };

    HelloImGui::RunnerParams runnerParams;

    // App window params
    runnerParams.appWindowParams.windowTitle = "implot demo";
    runnerParams.appWindowParams.windowSize = { 1200, 800};

    // ImGui window params
    runnerParams.imGuiWindowParams.defaultImGuiWindowType =
            HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
    runnerParams.imGuiWindowParams.showMenuBar = true;
    runnerParams.imGuiWindowParams.showStatusBar = true;

    // Split the screen in two parts
    runnerParams.dockingParams.dockingSplits = {
        { "MainDockSpace", "CodeSpace", ImGuiDir_Right, 0.65f },
    };

    // Dockable windows definitions
    HelloImGui::DockableWindow dock_imguiDemoWindow;
    {
        dock_imguiDemoWindow.label = "Dear ImGui Demo";
        dock_imguiDemoWindow.dockSpaceName = "MainDockSpace";
        dock_imguiDemoWindow.GuiFonction = [&dock_imguiDemoWindow] {
            if (dock_imguiDemoWindow.isVisible)
                ImGui::ShowDemoWindow(nullptr);
        };
        dock_imguiDemoWindow.callBeginEnd = false;
    };

    HelloImGui::DockableWindow dock_imguiDemoCode;
    {
        dock_imguiDemoCode.label = "imgui_demo code";
        dock_imguiDemoCode.dockSpaceName = "CodeSpace";
        dock_imguiDemoCode.GuiFonction = [&imGuiDemoBrowser]{ imGuiDemoBrowser.gui(); };
    };

    HelloImGui::DockableWindow dock_imguiReadme;
    {
        dock_imguiReadme.label = "ImGui - Readme";
        dock_imguiReadme.dockSpaceName = "CodeSpace";
        dock_imguiReadme.GuiFonction = []{
            static ImGuiReadmeBrowser w;
            w.gui();
        };
    };

    HelloImGui::DockableWindow dock_imGuiCppDocBrowser;
    {
        dock_imGuiCppDocBrowser.label = "ImGui - Doc";
        dock_imGuiCppDocBrowser.dockSpaceName = "CodeSpace";
        dock_imGuiCppDocBrowser.GuiFonction = [&imGuiCppDocBrowser]{ imGuiCppDocBrowser.gui(); };
    };


    // Menu
    runnerParams.callbacks.ShowMenus = [&allEditors]() {
        menuEditorTheme(allEditors);
    };

    // Fonts
    runnerParams.callbacks.LoadAdditionalFonts = MarkdownHelper::LoadFonts;

    // Set app dockable windows
    runnerParams.dockingParams.dockableWindows = {
            dock_imguiDemoCode,
            dock_imguiDemoWindow,
            dock_imguiReadme,
            dock_imGuiCppDocBrowser
            //dock_code,
    };

    gImGuiDemoCallback = implImGuiDemoCallbackDemoCallback;

    HelloImGui::Run(runnerParams);
    return 0;
}