/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_FLAT_FILE_BLOCK_STORAGE_FACTORY_HPP
#define IROHA_FLAT_FILE_BLOCK_STORAGE_FACTORY_HPP

#include "ametsuchi/block_storage_factory.hpp"

#include "ametsuchi/key_value_storage.hpp"
#include "interfaces/iroha_internal/block_json_converter.hpp"
#include "logger/logger_manager.hpp"

namespace iroha {
  namespace ametsuchi {
    class FlatFileBlockStorageFactory : public BlockStorageFactory {
     public:
      FlatFileBlockStorageFactory(
          std::shared_ptr<KeyValueStorage> block_store,
          std::shared_ptr<shared_model::interface::BlockJsonConverter>
              json_block_converter,
          logger::LoggerManagerTreePtr log_manager);
      std::unique_ptr<BlockStorage> create() override;

     private:
      std::shared_ptr<KeyValueStorage> block_store_;
      std::shared_ptr<shared_model::interface::BlockJsonConverter>
          json_block_converter_;
      logger::LoggerManagerTreePtr log_manager_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_FLAT_FILE_BLOCK_STORAGE_FACTORY_HPP
