/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage.hpp"

#include <boost/filesystem.hpp>

#include "backend/protobuf/block.hpp"
#include "common/byteutils.hpp"
#include "logger/logger.hpp"

using namespace iroha::ametsuchi;

FlatFileBlockStorage::FlatFileBlockStorage(
    std::shared_ptr<KeyValueStorage> block_store,
    std::shared_ptr<shared_model::interface::BlockJsonConverter> json_converter,
    logger::LoggerPtr log)
    : block_store_(std::move(block_store)),
      json_converter_(std::move(json_converter)),
      height_(block_store_->last_id()),
      log_(std::move(log)) {}

FlatFileBlockStorage::~FlatFileBlockStorage() {
  for (FlatFile::Identifier uncommitted = block_store_->last_id();
       uncommitted > height_;
       --uncommitted) {
    log_->debug("Remove uncommitted block {}", uncommitted);
    block_store_->remove(uncommitted);
  }
}

bool FlatFileBlockStorage::insert(
    std::shared_ptr<const shared_model::interface::Block> block) {
  return json_converter_->serialize(*block).match(
      [&](const auto &block_json) {
        return block_store_->add(block->height(),
                                 stringToBytes(block_json.value));
      },
      [this](const auto &error) {
        log_->warn("Error while block serialization: {}", error.error);
        return false;
      });
}

boost::optional<std::shared_ptr<const shared_model::interface::Block>>
FlatFileBlockStorage::fetch(
    shared_model::interface::types::HeightType height) const {
  auto storage_block = block_store_->get(height);
  if (not storage_block) {
    return boost::none;
  }

  return json_converter_->deserialize(bytesToString(*storage_block))
      .match(
          [&](auto &&block) {
            return boost::make_optional<
                std::shared_ptr<const shared_model::interface::Block>>(
                std::move(block.value));
          },
          [&](const auto &error)
              -> boost::optional<
                  std::shared_ptr<const shared_model::interface::Block>> {
            log_->warn("Error while block deserialization: {}", error.error);
            return boost::none;
          });
}

size_t FlatFileBlockStorage::size() const {
  return block_store_->last_id() - height_;
}

void FlatFileBlockStorage::clear() {
  block_store_->dropAll();
}

void FlatFileBlockStorage::commit(FunctionType function) {
  auto last_id = block_store_->last_id();
  for (auto block_id = std::max(1u, height_); block_id <= last_id; ++block_id) {
    auto block = fetch(block_id);
    BOOST_ASSERT(block);
    function(*block);
  }
  height_ = last_id;
}
