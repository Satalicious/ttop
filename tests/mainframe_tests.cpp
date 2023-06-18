
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

TEST_CASE("API is reachable") {
    int argc = 0;
    char **argv = nullptr;
    wxApp::SetInstance(new MyApp());
    wxInitialize(argc, argv);

    SECTION("Test FetchWelcomingText") {
        MainFrame frame("Test");
        std::string result = frame.FetchWelcomingText();
        REQUIRE(!result.empty());
    }

    wxUninitialize();
}

TEST_CASE("Test WriteCallback function", "[WriteCallback]") {
    //testing the scenario when the function is given a non-empty string,
    //and we are checking if it correctly appends it

    SECTION("Appends non-empty content to string") {
        char content[] = "hello, world!";
        size_t size = 1;
        size_t nmemb = sizeof(content) - 1;
        std::string userString;

        size_t bytesWritten = MainFrame::WriteCallback(content, size, nmemb, &userString); // <-- Modify this line

        REQUIRE(userString == "hello, world!");
        REQUIRE(bytesWritten == sizeof(content) - 1);
    }

    SECTION("Appends empty content to string") {
        // checking if it leaves unchanged when empty
        char content[] = "";
        size_t size = 1;
        size_t nmemb = sizeof(content) - 1;
        std::string userString = "original";

        size_t bytesWritten = MainFrame::WriteCallback(content, size, nmemb, &userString); // <-- Modify this line

        REQUIRE(userString == "original");
        REQUIRE(bytesWritten == 0);
    }
}
