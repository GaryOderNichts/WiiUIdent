#pragma once

#include "Screen.hpp"

class SubmitScreen : public Screen
{
public:
    SubmitScreen();
    virtual ~SubmitScreen();

    void Draw();

    bool Update(VPADStatus& input);

private:
    enum State {
        STATE_INFO,
        STATE_SUBMITTING,
        STATE_SUBMITTED,
    };
    State state = STATE_INFO;

    static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    void SubmitSystemData();

    std::string error;
    std::string response;
};
