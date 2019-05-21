/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/files.hpp"

#include <ciso646>

#include <boost/filesystem.hpp>
#include "logger/logger.hpp"

namespace {
  bool has_err(const boost::system::error_code &error_code,
               const logger::LoggerPtr &log) {
    if (error_code != boost::system::errc::success) {
      log->error("{}", error_code.message());
      return true;
    }
    return false;
  }
}  // namespace

void iroha::remove_dir_contents(const std::string &dir,
                                const logger::LoggerPtr &log) {
  boost::system::error_code error_code;

  bool exists = boost::filesystem::exists(dir, error_code);
  if (has_err(error_code, log))
    return;
  if (not exists) {
    log->error("Directory does not exist {}", dir);
    return;
  }

  bool is_dir = boost::filesystem::is_directory(dir, error_code);
  if (has_err(error_code, log))
    return;
  if (not is_dir) {
    log->error("{} is not a directory", dir);
    return;
  }

  for (auto entry : boost::filesystem::directory_iterator(dir)) {
    boost::filesystem::remove_all(entry.path(), error_code);
    if (error_code != boost::system::errc::success)
      log->error("{}", error_code.message());
  }
}

bool iroha::remove_file(const std::string &path, const logger::LoggerPtr &log) {
  boost::system::error_code error_code;

  bool exists = boost::filesystem::exists(path, error_code);
  if (has_err(error_code, log))
    return false;
  if (not exists) {
    log->error("The file does not exists {}", path);
    return false;
  }

  bool is_dir = boost::filesystem::is_directory(path, error_code);
  if (has_err(error_code, log))
    return false;
  if (is_dir) {
    log->error("{} is a directory, not a file", path);
    return false;
  }

  bool removed = boost::filesystem::remove(path, error_code);
  if (has_err(error_code, log))
    return false;
  if (not removed) {
    log->error("The file was not removed {}", path);
    return false;
  }
  return true;
}
