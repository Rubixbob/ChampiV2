#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <thread>

using namespace std;

class Settings
{
public:
	Settings();
	virtual ~Settings();
	static Settings& Instance() { return _instance; }

	int maxParallelWorkers = thread::hardware_concurrency();
	float minMateriaRatio = 0.5f;

	bool readSettingsFile();
	void writeSettingsFile();

private:
	static Settings _instance;
	const string _settingsFileName = "Config/Settings.ini";

	void controlSettings();
};