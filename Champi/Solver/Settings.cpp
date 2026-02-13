#include "Settings.h"

Settings Settings::_instance = Settings();

Settings::Settings()
{
    if (!readSettingsFile()) {
		writeSettingsFile();
    }
}

Settings::~Settings()
{
    //dtor
}

bool Settings::readSettingsFile() {
    ifstream settingsFile(_settingsFileName, ios::in);
    if (!settingsFile) {
        cout << "Couldn't open " << _settingsFileName << " for reading." << endl;
        return false;
    }
    
    map<string, string> settings;

    while (!settingsFile.eof()) {
        string line;
        getline(settingsFile, line);
        if (line.size() == 0) continue;

        stringstream ss;
        ss.str(line);
        string key, value;
        ss >> key >> value;
        settings[key] = value;
    }

    settingsFile.close();

    if (settings.contains("MaxParallelWorkers")) maxParallelWorkers = stoi(settings["MaxParallelWorkers"]);
    if (settings.contains("MinMateriaRatio")) minMateriaRatio = stof(settings["MinMateriaRatio"]);

	controlSettings();

	return true;
}

void Settings::writeSettingsFile() {
    ofstream settingsFile(_settingsFileName, ios::out);
    if (!settingsFile) {
        cout << "Couldn't open " << _settingsFileName << " for writing." << endl;
        return;
    }

    settingsFile << "MaxParallelWorkers " << maxParallelWorkers << "\n";
    settingsFile << "MinMateriaRatio " << minMateriaRatio << "\n";
}

void Settings::controlSettings() {
	auto prevMaxParallelWorkers = maxParallelWorkers;
	auto prevMinMateriaRatio = minMateriaRatio;

    maxParallelWorkers = max(1, min(maxParallelWorkers, (int)thread::hardware_concurrency()));
    minMateriaRatio = max(0.0f, min(minMateriaRatio, 1.0f));

    if (prevMaxParallelWorkers != maxParallelWorkers || prevMinMateriaRatio != minMateriaRatio) {
        writeSettingsFile();
    }
}