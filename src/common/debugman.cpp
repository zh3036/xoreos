/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

// Inspired by ScummVM's debug channels

/** @file
 *  The debug manager, managing debug channels.
 */

#include "src/version/version.h"

#include "src/common/maths.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/filepath.h"
#include "src/common/debugman.h"
#include "src/common/configman.h"
#include "src/common/datetime.h"

DECLARE_SINGLETON(Common::DebugManager)

namespace Common {

static const char * const kDebugNames[kDebugChannelCount] = {
	"GGraphics", "GSound", "GVideo", "GEvents", "GScripts",
	"GGLAPI", "GGLWindow", "GGLShader", "GGL3rd", "GGLApp", "GGLOther",
	"EGraphics", "ESound", "EVideo", "EEvents", "EScripts", "ELogic"
};

static const char * const kDebugDescriptions[kDebugChannelCount] = {
	"Global graphics debug channel",
	"Global sound debug channel",
	"Global video (movies) debug channel",
	"Global events debug channel",
	"Global scripts debug channel",
	"OpenGL debug message generated by the GL",
	"OpenGL debug message generated by the windowing system",
	"OpenGL debug message generated by the shader compiler",
	"OpenGL debug message generated by third party middleware",
	"OpenGL debug message generated by the application",
	"OpenGL debug message generated by other sources",
	"Engine graphics debug channel",
	"Engine sound debug channel",
	"Engine video debug channel",
	"Engine events debug channel",
	"Engine scripts debug channel",
	"Engine game logic debug channel"
};

static const char * const kDebugGLTypes[kDebugGLTypeMAX] = {
	"Error", "Deprecated", "Undefined", "Portability", "Performance", "Other"
};

DebugManager::DebugManager() : _logFileStartLine(false), _changedConfig(false) {
	for (size_t i = 0; i < kDebugChannelCount; i++) {
		_channels[i].name        = kDebugNames[i];
		_channels[i].description = kDebugDescriptions[i];
		_channels[i].level       = 0;

		for (size_t j = 0; j < kDebugGLTypeMAX; j++)
			_channels[i].glTypeIDs[j] = 0;

		_channelMap[kDebugNames[i]] = (DebugChannel) i;
	}

	_channelMap["all"] = kDebugChannelAll;
}

DebugManager::~DebugManager() {
	closeLogFile();
}

void DebugManager::getDebugChannels(std::vector<UString> &names, std::vector<UString> &descriptions) const {
	names.resize(kDebugChannelCount);
	descriptions.resize(kDebugChannelCount);

	for (size_t i = 0; i < kDebugChannelCount; i++) {
		names[i]        = _channels[i].name;
		descriptions[i] = _channels[i].description;
	}
}

void DebugManager::setVerbosityLevel(DebugChannel channel, uint32 level) {
	if (channel == kDebugChannelAll) {
		for (size_t i = 0; i < kDebugChannelCount; i++)
			setVerbosityLevel((DebugChannel) i, level);

		return;
	}

	if (channel >= kDebugChannelCount)
		return;

	_channels[channel].level = MIN<uint32>(level, kMaxVerbosityLevel);

	for (size_t i = 0; i < kDebugGLTypeMAX; i++)
		_channels[channel].glTypeIDs[i] = 0;

	_changedConfig = true;
}

void DebugManager::setVerbosityLevel(const UString &channel, uint32 level) {
	ChannelMap::iterator c = _channelMap.find(channel);
	if (c == _channelMap.end())
		return;

	setVerbosityLevel(c->second, level);
}

uint32 DebugManager::getVerbosityLevel(DebugChannel channel) const {
	if (channel >= kDebugChannelCount)
		return 0;

	return _channels[channel].level;
}

uint32 DebugManager::getVerbosityLevel(const UString &channel) const {
	ChannelMap::const_iterator c = _channelMap.find(channel);
	if (c == _channelMap.end())
		return 0;

	return getVerbosityLevel(c->second);
}

bool DebugManager::isEnabled(DebugChannel channel, uint32 level) const {
	return getVerbosityLevel(channel) >= MIN<uint32>(level, kMaxVerbosityLevel);
}

bool DebugManager::isEnabled(const UString &channel, uint32 level) const {
	return getVerbosityLevel(channel) >= MIN<uint32>(level, kMaxVerbosityLevel);
}

void DebugManager::setVerbosityLevelsFromConfig() {
	setVerbosityLevel(kDebugChannelAll, 0);

	std::vector<UString> debug;
	UString::split(ConfigMan.getString("debug"), ',', debug);

	for (std::vector<UString>::const_iterator d = debug.begin(); d != debug.end(); ++d) {
		std::vector<UString> config;
		UString::split(*d, ':', config);

		if ((config.size() != 2) || config[0].empty())
			continue;

		config[0].trim();
		config[1].trim();

		uint32 level = 0;
		try {
			parseString(config[1], level);
		} catch (...) {
		}

		setVerbosityLevel(config[0], level);
	}

	_changedConfig = false;
}

void DebugManager::setConfigToVerbosityLevels() {
	if (!_changedConfig)
		return;

	UString debug;

	for (size_t i = 0; i < kDebugChannelCount; i++) {
		if (_channels[i].level == 0)
			continue;

		if (!debug.empty())
			debug += ',';

		debug += _channels[i].name + ":" + composeString(_channels[i].level);
	}

	ConfigMan.setString("debug", debug, true);
}

static bool isOpenGLDebugChannel(DebugChannel channel) {
	return (channel >= kDebugGLAPI) && (channel <= kDebugGLOther);
}

void DebugManager::logDebugGL(DebugChannel channel, uint32 level, DebugGLType type, uint32 id,
                              const char *msg) {

	if (!isOpenGLDebugChannel(channel) || !isEnabled(channel, level))
		return;

	if (((uint32) type) >= kDebugGLTypeMAX)
		type = kDebugGLTypeOther;

	// Suppress duplicates
	if (_channels[channel].glTypeIDs[type] == id)
		return;

	_channels[channel].glTypeIDs[type] = id;

	status("%s<%s,%u,%u>: %s", kDebugNames[channel], kDebugGLTypes[type], level, id, msg);
}

bool DebugManager::openLogFile(const UString &file) {
	closeLogFile();

	_logFileStartLine = true;

	// Create the directories in the path, if necessary
	UString path = FilePath::canonicalize(file);

	try {
		FilePath::createDirectories(FilePath::getDirectory(path));
	} catch (...) {
		return false;
	}

	if (!_logFile.open(path))
		return false;

	logString(Version::getProjectNameVersionFull());
	logString("\n");

	return true;
}

void DebugManager::closeLogFile() {
	_logFile.close();
}

void DebugManager::logString(const UString &str) {
	if (!_logFile.isOpen())
		return;

	// If we're at the start of a new line, write the timestamp
	if (_logFileStartLine) {
		UString tstamp;

		try {
			tstamp = "[" + DateTime(DateTime::kUTC).formatDateTimeISO('T', '-', ':') + "] ";
		} catch (...) {
			tstamp = "[0000-00-00T00:00:00] ";
		}

		_logFile.writeString(tstamp);
	}

	_logFile.writeString(str);

	// Find out whether we just started a new line. If this fails, force one
	try {
		_logFileStartLine = !str.empty() && (*--str.end() == '\n');
	} catch (...) {
		_logFile.writeString("\n");
		_logFileStartLine = true;
	}

	if (_logFileStartLine)
		_logFile.flush();
}

void DebugManager::logCommandLine(const std::vector<UString> &argv) {
	logString("Full command line:");
	for (std::vector<UString>::const_iterator arg = argv.begin(); arg != argv.end(); ++arg) {
		logString(" ");
		logString(*arg);
	}
	logString("\n");
}

UString DebugManager::getDefaultLogFile() {
	// By default, put the log file into the user data directory
	return FilePath::getUserDataDirectory() + "/xoreos.log";
}

} // End of namespace Common
