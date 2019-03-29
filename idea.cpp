#include <stdio>
#include <shlobj_core.h>

time_t  lastPing;
time_t  now;
int     port            = 0;

bool    isRunning       = false;

#define APP_NAME        "TS3SSE"
#define APP_DISPLAY     "Teamspeak 3"
#define APP_DEVELOPER   "Niko Hujanen"
#define MSG_HEARTBEAT   "{\"game\": \"%s\"}"
#define MSG_REGISTER    "{\"game\": \"%s\", \"game_display_name\": \"%s\", \"developer\": \"%s\"}"
#define MSG_BIND        "{\"game\": \"%s\", \"event\": \"%s\", \"min_value\": %d, \"max_value\": %d, \"value_optional\": false}"

void initialize()
{
    readProps();
    ping();
}

void readProps()
{
    int     result;
    FILE    *handle;

    handle = fopen(getPropsPath(), "r");
    
    if (handle != NULL) {
        fscanf(handle, "127.0.0.1:%d", &result);
        if (result > 0) {
            isRunning   = true;
            port        = result;
        }
    }
}

// @TODO Fix this method - types are 110% frakked up, should be TCHAR or wchar_t or something (tm)
char *getPropsPath()
{
    char path[128];
    if (SUCCEEDED(SHGetFolderPath(
        NULL,
        CSIDL_PERSONAL|CSIDL_FLAG_CREAT,
        NULL,
        0,
        path
    ))) {
        PathAppend(path, "SteelSeries/SteelSeries Engine 3/coreProps.json");
    }

    return path;
}

void ping(bool forced = false)
{
    if (!isRunning) return;

    time(&now);

    if (!forced) {
        if ((now - lastPing) < 12)) return;
    }

    char *message = new char[64];
    sprintf(
        message, 
        MSG_HEARTBEAT, 
        APP_NAME
    );
    sendHttp("/game_heartbeat", message);
    time(&lastPing);
    
    // Needed?
    delete message;
}

void register()
{
    if (!isRunning) return;

    char *message = new char[64];
    sprintf(
        message, 
        MSG_REGISTER, 
        APP_NAME, 
        APP_DISPLAY, 
        APP_DEVELOPER
    );
    sendHttp("/game_metadata", message);
    // Needed?
    delete message;
}

void bind()
{
    if (!isRunning) return;

    char *message = new char[64];
    sprintf(
        message, 
        MSG_BIND,
        APP_NAME,
        "MUTED",
        0,
        1
    );

    sendHttp("/bind_game_event", message);
    
    // Needed?
    delete message;
}

void sendHttp(char *endpoint, char *message)
{
    char    request[4096];
    int     handle;
    struct  sockaddr_in address;

    handle = socket(AF_INET, SOCK_STREAM, 0);
    if (handle < 0) return;

    memset(&address, '0', sizeof(address));

    address.sin_family      = AF_INET;
    address.sin_port        = htons(port);
    address.sin_addr.s_addr = inetr_addr("127.0.0.1");

    if (connect(handle, (struct sockaddr *)&address, sizeof(address)) < 0} return;

    snprintf(
        request, 
        1024,
            "POST %s HTTP/1.0\r\n"
            "Host: 127.0.0.1\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n",
            "%s",
        endpoint,
        (int)strlen(message),
        message
    );

    write(handle, request, strlen(request));

    // Clean up, needed?
    close(handle);
    delete request;
}
