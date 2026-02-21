/*
    updatemd.cpp - update project  metadata.


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2026  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "misc1.h"
#include "bprocess.h"
#include "benv.h"
#include "free.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <exception>
#include <optional>
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>


namespace fs = std::filesystem;
using ClocMap_t = std::map<std::string, int>;
using ClocKeysMap_t = std::map<const char *, const char *>;

struct MetaDataUpdateConfig
{
    std::string metaDataPath;
    std::string clangTidyConfigPath;
    std::string jsonKey;
    bool isEstimateLinesOfCode{};
};

static int readMetadata(const std::string &path, Json::Value &metaData)
{
    std::ifstream ifs;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    ifs.open(path);
    if (!ifs.is_open())
    {
      return EXIT_SUCCESS;
    }

    try
    {
        if (!parseFromStream(builder, ifs, &metaData, &errs))
        {
          std::cerr << "*** Error reading '" << path << "': " << errs << ".\n";
          return EXIT_FAILURE;
        }
    }
    catch(std::exception &ex)
    {
          std::cerr << "*** Exception reading '" << path << "': " <<
              ex.what() << ".\n";
          return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int writeMetadata(const std::string &path, const Json::Value &metaData)
{
    std::ofstream ofs;

    ofs.open(path);
    if (!ofs.is_open())
    {
      std::cout << "*** Warning: File '" << path << "' not updated.\n"
          "    File or directory is read-only.\n";
      return EXIT_SUCCESS;
    }

    try
    {
        Json::StreamWriterBuilder builder;

        builder["indentation"] = "    ";
        builder["commentStyle"] = "None";
        builder["enableYAMLCompatibility"] = true;
        const std::unique_ptr<Json::StreamWriter>
            writer(builder.newStreamWriter());
        writer->write(metaData, &ofs);
        ofs << "\n";
    }
    catch(std::exception &ex)
    {
          std::cerr << "*** Exception writing '" << path << "': " <<
              ex.what() << ".\n";
          return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Parsing clang-tidy config file to count the number of applied
// rules. The file has to follow the following restrictions, to
// successfully parse the rules count:
// - The file has to be YAML format.
// - The line beginning with "Checks:" must not define a positive rule.
// - The "Checks:" line defines the negative default: '-*'
// - Per line exactly one positive rule is defined.
// - Do not use globs for rule definitions.
static int getClangTidyRulesCount(const std::string &path, int &count)
{
    count = 0;
    std::ifstream ifs;
    bool isChecksActive = false;
    std::string line;

    ifs.open(path);
    if (!ifs.is_open())
    {
        std::cerr << "*** Error opening '"<< path << "' for reading.\n";
        return EXIT_FAILURE;
    }

    std::getline(ifs, line);
    while (ifs.good())
    {
        if (isChecksActive)
        {
            static std::regex regexRule(R"([a-z][a-zA-Z0-9.-]+\s*["',]?)");
            static std::regex regexKey("[a-zA-Z]+:\\s+.*");
            static std::regex regexMisc("[\"'*-]+\\s*,?");

            line = flx::trim(std::move(line));
            if (!line.empty() && !std::regex_match(line, regexMisc))
            {
                if (std::regex_match(line, regexRule))
                {
                    ++count;
                }
                else
                {
                    if (!std::regex_match(line, regexKey))
                    {
                        std::cerr << "*** Error parsing '"<< path << "'.\n"
                            "Unidentified string: '" << line << "'.\n";
                        return EXIT_FAILURE;
                    }

                    return EXIT_SUCCESS;
                }
            }
        }
        else
        {
            if (line.find("Checks:") != std::string::npos)
            {
                isChecksActive = true;
            }
        }

        std::getline(ifs, line);
    }

    return EXIT_SUCCESS;
}

static int which(const std::string &executable)
{
    std::vector<std::string> arguments{{
        "-c", "which " + executable + " >/dev/null 2>&1"}};
    BProcess process("sh", "", arguments);

    process.Start();
    return process.Wait();
}

static ClocMap_t cloc(const ClocKeysMap_t &keysMap,
        const char *sumKey, const std::string &jsonKey)
{
    std::string command{"ls options.txt >/dev/null 2>&1"};
    std::vector<std::string> arguments{{ "-c", command }};
    BProcess lsProcess("sh", "", arguments);
    lsProcess.Start();
    auto exitCode = lsProcess.Wait();
    if (exitCode != EXIT_SUCCESS)
    {
        std::cerr << "*** Error: File options.txt not found in current "
            "directory.\n";
        return {};
    }

    ClocMap_t resultMap;
    std::string stdoutFilename{std::string("_cloc_stdout_") + jsonKey + ".txt"};
    std::string stdoutTemp = fs::temp_directory_path() / stdoutFilename;
    command = "cloc --config options.txt . >" + stdoutTemp;
    arguments = {{ "-c", command }};
    BProcess clocProcess("sh", "", arguments);

    clocProcess.Start();
    exitCode = clocProcess.Wait();

    if (exitCode == EXIT_SUCCESS)
    {
        static std::regex regex(R"(([a-zA-Z+/ ]+)\d+\s+\d+\s+\d+\s+(\d+))");
        std::smatch match;
        std::ifstream ifs(stdoutTemp);
        std::string line;

        if (ifs.is_open())
        {
            int sum = 0;

            while (std::getline(ifs, line))
            {
                int value = 0;

                if (std::regex_match(line, match, regex) && match.size() == 3)
                {
                    for (const auto &[clocKey, resultKey] : keysMap)
                    {
                        if (flx::trim(match[1]) == clocKey)
                        {
                            value = std::stoi(match[2]);
                            resultMap[resultKey] = value;
                            sum += value;
                        }
                    }
                }
            }

            if (sum != 0 && sumKey != nullptr)
            {
                resultMap[sumKey] = sum;
            }
        }
    }

    if (resultMap.empty())
    {
        for (const auto &[clocKey, resultKey] : keysMap)
        {
            resultMap[resultKey] = 0;
        }
        resultMap[sumKey] = 0;
    }

    fs::remove(stdoutTemp);

    return resultMap;
}

static Json::Value createMetaData(
        const std::optional<int> &optClangTidyRules,
        const std::string &jsonKey,
        ClocMap_t &resultMap)
{
    Json::Value result;

    if (optClangTidyRules.has_value())
    {
        result["clangTidyRules"] = optClangTidyRules.value();
    }

    if (!resultMap.empty())
    {
        for (const auto &[key, value] : resultMap)
        {
            if (jsonKey.empty())
            {
                result["linesOfCode"][key] = value;
            }
            else
            {
                result["linesOfCode"][jsonKey][key] = value;
            }
        }
    }

    return result;
}

static void version()
{
    flx::print_versions(std::cout, "updatemd");
}

static void usage()
{
    std::cout <<
        "Update metadata in a C++ source code project. It updates the "
        "number of\nclang-tidy checks and/or the lines of code for C++ source "
        "and header files\nwithin a specific directory.\n\n"
        "Usage: updatemd -l [-k <key>][-f <metadata-path>]\n"
        "Usage: updatemd -c <clang-tidy-path> [-f <metadata-path>]\n"
        "Usage: updatemd -V\n"
        "Usage: updatemd -h\n\n"
        "Parameters:\n"
        "  -c: <clang-tidy-path> Path of clang-tidy config file\n"
        "  -f: <metadata-path> Path of metadata.json to be created\n"
        "  -h: Print this help and exit\n"
        "  -k: <key> The key under which the lines of code statistics is\n"
        "      places in json file. Default: No key\n"
        "      A key starts with a character followed by characters or digits\n"
        "  -l: parse directory for lines of code\n"
        "  -V: Print version and exit\n";
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static int createMetaDataConfig(int argc, char *argv[],
        MetaDataUpdateConfig &config)
{
    const std::string optstr{"c:f:k:lhV"};
    int result = 0;

    config.jsonKey.clear();
    config.clangTidyConfigPath.clear();
    config.metaDataPath = "metadata.json";
    config.isEstimateLinesOfCode = false;

    while ((result = getopt(argc, argv, optstr.c_str())) != -1)
    {
        switch (result)
        {
            case 'k': config.jsonKey = optarg;
                      break;

            case 'c': config.clangTidyConfigPath = optarg;
                      break;

            case 'f': config.metaDataPath = optarg;
                      break;

            case 'l': config.isEstimateLinesOfCode = true;
                      break;

            case 'V': version();
                      exit(0);

            case 'h': usage();
                      exit(0);
            case '?':
                      if (optopt != 'X' && !isprint(optopt))
                      {
                          std::cerr << "Unknown option character '\\x" <<
                                       std::hex << optopt << "'.\n";
                      }
                      return EXIT_FAILURE;
            default:  return EXIT_FAILURE;
        }
    }

    static std::regex regexKey("[a-zA-Z][a-zA-Z0-9]*");

    if (!config.jsonKey.empty() && !std::regex_match(config.jsonKey, regexKey))
    {
        std::cerr << "*** Error: Invalid json key '" << config.jsonKey <<
            "'.\n";
        return EXIT_FAILURE;
    }

    if (!config.clangTidyConfigPath.empty() &&
            !fs::exists(config.clangTidyConfigPath))
    {
        std::cerr << "*** Error: clang-tidy config file path does not exist: '"
            << config.clangTidyConfigPath << "'.\n";
        return EXIT_FAILURE;
    }

    if (!config.isEstimateLinesOfCode && !config.jsonKey.empty())
    {
        std::cerr << "*** Error: Parameter -k can only be used together with "
            "-l.\n";
        return EXIT_FAILURE;
    }

    if (!config.isEstimateLinesOfCode && config.clangTidyConfigPath.empty())
    {
        std::cerr << "*** Error: Either parameter -l or -c has to be "
            "specified.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int clangTidyRules = 0;
    std::string envval;
    Json::Value metaDataIn;
    std::map<std::string, ClocMap_t> resultMap;
    MetaDataUpdateConfig cfg;
    std::optional<int> optClangTidyRules;
    ClocMap_t clocResultMap;

    try
    {
        // The metadata update only should run on the flexemu build machine
        // which has this environment variable set.
        // If not set abort early with exit state 0.
        if (!BEnvironment::GetValue("FLEXEMU_UPDATE_METADATA", envval))
        {
            return EXIT_SUCCESS;
        }
        auto exitCode = createMetaDataConfig(argc, argv, cfg);
        if (exitCode != EXIT_SUCCESS)
        {
            return exitCode;
        }

        if (cfg.isEstimateLinesOfCode)
        {
            exitCode = which("cloc");
            if (exitCode != EXIT_SUCCESS)
            {
                return exitCode;
            }
        }

        if (!cfg.clangTidyConfigPath.empty())
        {
            exitCode = getClangTidyRulesCount(cfg.clangTidyConfigPath,
                    clangTidyRules);

            if (exitCode != EXIT_SUCCESS)
            {
                return exitCode;
            }

            optClangTidyRules = clangTidyRules;
        }

        exitCode = readMetadata(cfg.metaDataPath, metaDataIn);
        if (exitCode != EXIT_SUCCESS)
        {
            return exitCode;
        }

        if (cfg.isEstimateLinesOfCode)
        {
            // Mapping of key used in cloc output to key used in json file.
            const ClocKeysMap_t keysMap{{
                { "C++", "cpp" },
                { "C/C++ Header", "cheader" }
            }};

            clocResultMap = cloc(keysMap, "cppsum", cfg.jsonKey);
            if (clocResultMap.empty())
            {
                return EXIT_FAILURE;
            }
        }

        auto metaDataOut = createMetaData(optClangTidyRules, cfg.jsonKey,
                clocResultMap);
        if (metaDataIn != metaDataOut)
        {
            std::cout << "updatemd: Updating file " << cfg.jsonKey <<
                "/" << cfg.metaDataPath << " ...\n";
        }

        return writeMetadata(cfg.metaDataPath, metaDataOut);
    }
    catch (std::exception &ex)
    {
        std::cerr << "*** Got unrecoverable exception: " <<
            ex.what() << " - aborted.\n";
    }
}
