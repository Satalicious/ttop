
#include "catch_amalgamated.hpp"

#include "../mainframe.hpp"
#include <curl/curl.h>
#include <curl/easy.h>
#include <wx/generic/grid.h>
#include <wx/protocol/http.h>
#include <wx/url.h>
#include <wx/wx.h>

#include <codecvt>
#include <jsoncpp/json/json.h>
#include <locale>
class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        // This method is intentionally left empty.
        return true;
    }
};

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
TEST_CASE("GUI Test - FetchWelcomingText", "[wxWidgets]") {
    int argc = 0;
    char **argv = nullptr;
    wxApp::SetInstance(new MyApp());
    wxInitialize(argc, argv);

    SECTION("Test FetchWelcomingText") {
        MainFrame frame("Test");
        std::string result = frame.FetchWelcomingText();
    }

    wxUninitialize();
}