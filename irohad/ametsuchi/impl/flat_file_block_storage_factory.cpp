/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage_factory.hpp"

#include "ametsuchi/impl/flat_file_block_storage.hpp"

using namespace iroha::ametsuchi;

FlatFileBlockStorageFactory::FlatFileBlockStorageFactory(
    std::shared_ptr<KeyValueStorage> block_store,
    std::shared_ptr<shared_model::interface::BlockJsonConverter>
        json_block_converter,
    logger::LoggerManagerTreePtr log_manager)
    : block_store_(std::move(block_store)),
      json_block_converter_(std::move(json_block_converter)),
      log_manager_(std::move(log_manager)) {}

std::unique_ptr<BlockStorage> FlatFileBlockStorageFactory::create() {
  return std::make_unique<FlatFileBlockStorage>(
      block_store_,
      json_block_converter_,
      log_manager_->getChild("FlatFileBlockFactory")->getLogger());
}
