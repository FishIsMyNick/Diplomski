enum mCmd {
    none = 0,
    rotateCW,
    rotateCCW,
    getV,
    getI,
    getP,
    quit
};

float absoluteValue(float);
std::string to_string_with_precision(float, int);

void sendResponse(const std::string&);
int serverLoop();

std::string enumToString(mCmd);

std::tuple<mCmd, float, float> parseCommand(std::string);
void commandProcessingLoop();
void commandExecutionLoop();

void StartProgram();
