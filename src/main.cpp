
#include "streaming_processor.hpp"
#include "config_parser.hpp"
#include "csv_reader.hpp"
#include "median_calculator.hpp"

#include <boost/program_options.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include <queue>
#include <chrono>
#include <windows.h>
#include "buffer_reader.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;
using csv_median_calculator::config;
using csv_median_calculator::record;
using csv_median_calculator::median_calculator;
using csv_median_calculator::parse_config;
using csv_median_calculator::collect_csv_files;


int main(int argc, char* argv[]) {

    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)) {
        spdlog::warn("Failed to set process priority class. Run as administrator for best results.");
    }
    else {
        spdlog::info("Process priority set to HIGH_PRIORITY_CLASS");
    }

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    spdlog::info("Starting csv_median_calculator v1.0.0");

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(), "path to config file")
        ("cfg", po::value<std::string>(), "path to config file (alternative)");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (const po::error& e) {
        spdlog::error("Command line error: {}", e.what());
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    std::string config_path;
    if (vm.count("config")) {
        config_path = vm["config"].as<std::string>();
    }
    else if (vm.count("cfg")) {
        config_path = vm["cfg"].as<std::string>();
    }
    else {
        fs::path exe_dir = fs::canonical(fs::path(argv[0])).parent_path();
        fs::path candidates[] = {
            exe_dir / "config.toml",
            fs::current_path() / "config.toml"
        };
        bool found = false;
        for (const auto& cand : candidates) {
            if (fs::exists(cand)) {
                config_path = cand.string();
                found = true;
                break;
            }
        }
        if (!found) {
            spdlog::error("Could not find config.toml neither in exe directory nor in current working directory.");
            return 1;
        }
    }
    spdlog::info("Using config file: {}", config_path);

    config cfg;
    try {
        cfg = parse_config(config_path);
    }
    catch (const std::exception& e) {
        spdlog::error("Config parsing failed: {}", e.what());
        return 1;
    }

    // Преобразование относительных путей в абсолютные относительно директории конфига 
    fs::path config_dir = fs::path(config_path).parent_path();
    auto make_absolute = [&config_dir](std::string& path_str) {
        fs::path p(path_str);
        if (p.is_relative()) {
            p = config_dir / p;
            path_str = p.lexically_normal().string();
        }
        };
    make_absolute(cfg.input_dir);
    make_absolute(cfg.output_dir);

    spdlog::info("Input directory: {}", cfg.input_dir);
    spdlog::info("Output directory: {}", cfg.output_dir);
    if (cfg.filename_masks.empty()) {
        spdlog::info("Filename masks: none (all CSV files)");
    }
    else {
        std::string masks;
        for (const auto& m : cfg.filename_masks) {
            masks += m + " ";
        }
        spdlog::info("Filename masks: {}", masks);
    }

    // --- Сбор списка подходящих CSV-файлов (без чтения) ---
    spdlog::info("Collecting CSV files...");
    auto candidate_files = collect_csv_files(cfg.input_dir, cfg.filename_masks);
    if (candidate_files.empty()) {
        spdlog::warn("No CSV files match the masks in input directory.");
        return 0;
    }
 
    csv_median_calculator::StreamingProcessor processor(cfg, candidate_files);
    return processor.run();
}